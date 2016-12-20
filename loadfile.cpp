/*
Copyright (C) 2015 Lauri Kasanen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Modifications Copyright (C) 2016 Guy Turcotte
*/

#include "loadfile.h"

#include "utils.h"

//#include <omp.h>
#include <lzo/lzo1x.h>
#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>

static bool nonwhite(const u8 * const pixel) {

  return pixel[0] != 255 ||
    pixel[1] != 255 ||
    pixel[2] != 255;
}

static void getmargins(const u8 * const src, const u32 w, const u32 h,
      const u32 rowsize, u32 *minx, u32 *maxx,
      u32 *miny, u32 *maxy) {

  int i, j;

  bool found = false;
  for (i = 0; i < (int) w && !found; i++) {
    for (j = 0; j < (int) h && !found; j++) {
      const u8 * const pixel = src + j * rowsize + i * 4;
      if (nonwhite(pixel)) {
        found = true;
        *minx = i;
      }
    }
  }

  found = false;
  for (j = 0; j < (int) h && !found; j++) {
    for (i = *minx; i < (int) w && !found; i++) {
      const u8 * const pixel = src + j * rowsize + i * 4;
      if (nonwhite(pixel)) {
        found = true;
        *miny = j;
      }
    }
  }

  const int startx = *minx, starty = *miny;

  found = false;
  for (i = w - 1; i >= startx && !found; i--) {
    for (j = h - 1; j >= starty && !found; j--) {
      const u8 * const pixel = src + j * rowsize + i * 4;
      if (nonwhite(pixel)) {
        found = true;
        *maxx = i;
      }
    }
  }

  found = false;
  for (j = h - 1; j >= starty && !found; j--) {
    for (i = *maxx; i >= startx && !found; i--) {
      const u8 * const pixel = src + j * rowsize + i * 4;
      if (nonwhite(pixel)) {
        found = true;
        *maxy = j;
      }
    }
  }
}

void LoadFile::store(SplashBitmap * const bm, cachedpage & cache)
{
  const u32 w = bm->getWidth();
  const u32 h = bm->getHeight();
  const u32 rowsize = bm->getRowSize();

  const u8 * const src = bm->getDataPtr();

  u32 minx = 0,
      miny = 0,
      maxx = w - 1,
      maxy = h - 1;

  // Trim margins
  getmargins(src, w, h, rowsize, &minx, &maxx, &miny, &maxy);

  const u32 trimw = maxx - minx + 1;
  const u32 trimh = maxy - miny + 1;

  u8 * const trimmed = (u8 *) xcalloc(trimw * trimh * 4, 1);
  u32 j;
  for (j = miny; j <= maxy; j++) {
    const u32 destj = j - miny;
    memcpy(trimmed + destj * trimw * 4, src + j * rowsize + minx * 4, trimw * 4);
  }

  // Trimmed copy done, compress it

  u8 * const tmp = (u8 *) xcalloc(trimw * trimh * 4 * 1.08f, 1);
  u8 workmem[LZO1X_1_MEM_COMPRESS]; // 64kb, we can afford it
  lzo_uint outlen;
  int ret = lzo1x_1_compress(trimmed, trimw * trimh * 4, tmp, &outlen, workmem);
  if (ret != LZO_E_OK) {
    die("Compression failed");
  }

  free(trimmed);

  u8 * const dst = (u8 *) xcalloc(outlen, 1);
  memcpy(dst, tmp, outlen);
  free(tmp);

  // Store

  cache.uncompressed = trimw * trimh * 4;
  cache.w            = trimw;
  cache.h            = trimh;
  cache.left         = minx;
  cache.right        = w - maxx;
  cache.top          = miny;
  cache.bottom       = h -maxy;
  cache.size         = outlen;
  cache.data         = dst;
}

void LoadFile::dopage(const u32 page) {

  struct timeval start, end;
  gettimeofday(&start, NULL);

  SplashColor white = { 255, 255, 255 };
  SplashOutputDev * splash = new SplashOutputDev(splashModeXBGR8, 4, false, white);
  splash->startDoc(pdf);

  pdf->displayPage(splash, page + 1, 144, 144, 0, true, false, false);

  gettimeofday(&end, NULL);
  if (details > 1) {
    printf("%u: rendering %u us\n", page, usecs(start, end));
    start = end;
  }

  SplashBitmap * const bm = splash->takeBitmap();

  store(bm, cache[page]);

  gettimeofday(&end, NULL);
  if (details > 1) {
    printf("%u: storing %u us\n", page, usecs(start, end));
    start = end;
  }

  delete bm;
  delete splash;

  __sync_bool_compare_and_swap(&cache[page].ready, 0, 1);

  // If this page was visible, tell the app to refresh
  const u32 first = __sync_fetch_and_add(&first_visible, 0);
  const u32 last = __sync_fetch_and_add(&last_visible, 0);
  if (page >= first && page <= last) {
    emit refresh();
  }
}

LoadFile::LoadFile(const char *fname) :
  filename(NULL),
  loaded(false),
  loading(false),
  cache(NULL),
  pdf(NULL),
  details(0),
  pdfLoader(NULL)
{
  // Parse info
  GooString gooname(fname);

  PDFDoc *pdfDoc = new PDFDoc(&gooname);
  if (!pdfDoc->isOk()) {
    const int err = pdfDoc->getErrorCode();
    const char *msg = "Unknown";

    switch (err) {
      case errOpenFile:
      case errFileIO:
        msg = "Couldn't open file";
      break;
      case errBadCatalog:
      case errDamaged:
      case errPermission:
        msg = "Damaged PDF file";
      break;
    }

    warn("Warning %d, %s", err, msg);

    return;
  }

  if (pdfLoader) {
    // Free the old one
    //pthread_cancel(tid);
    pdfLoader->abort();
    loaderThread.wait();

    u32 i;
    const u32 max = pages;
    for (i = 0; i < max; i++) {
      if (cache[i].ready)
        free(cache[i].data);
    }
    free(cache);
    cache = NULL;
  }

  if (filename) free(filename);
  if (pdf) free(pdf);
  filename = (char *) xmalloc(strlen(fname) + 1);
  strcpy(filename,  fname);
  pdf = pdfDoc;
  pages = pdf->getNumPages();
  maxw = maxh = first_visible = last_visible = 0;

  // Start threaded magic
  if (pages < 1) {
    warn("Couldn't open %s, perhaps it's corrupted?", fname);
    return;
  }

  //fl_cursor(FL_CURSOR_WAIT);

  cache = (cachedpage *) xcalloc(pages, sizeof(cachedpage));

  if (!globalParams) {
    globalParams = new GlobalParams;
  }

  dopage(0);

  if (pdfLoader == NULL) {
    pdfLoader = new PDFLoader(this);
  }
  pdfLoader->moveToThread(&loaderThread);

//  connect(&loaderThread, &QThread::finished, pdfLoader, &QObject::deleteLater);
//  connect(this, &LoadFile::operate, pdfLoader, &PDFLoader::renderer);
//  connect(pdfLoader, &Worker::resultReady, this, &LoadFile::handleResults);

  loaderThread.start();

//  pthread_attr_t attr;
//  pthread_attr_init(&attr);
//  const struct sched_param nice = { 15 };
//  pthread_attr_setschedparam(&attr, &nice);

//  aborting = false;
//  pthread_create(tid, &attr, renderer, NULL);

//  loading = true;
}

LoadFile::~LoadFile()
{
  loaderThread.quit();
  loaderThread.wait();
}
