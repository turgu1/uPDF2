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

#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>

#include "updf.h"
#include "loadfile.h"

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

void LoadFile::dopage(const u32 page)
{
  struct timeval start, end;
  gettimeofday(&start, NULL);

  debug(QString("do page %1").arg(page));

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

void LoadFile::clean()
{
  if (pdfLoader) {
    pdfLoader->abort();
    pdfLoader->wait();

    delete pdfLoader;
    pdfLoader = NULL;
  }

  if (cache) {
    u32 i;
    const u32 max = pages;
    for (i = 0; i < max; i++) {
      if (cache[i].ready) {
        free(cache[i].data);
      }
    }
    free(cache);
    cache = NULL;
  }

  filename = "";
  if (pdf) {
    free(pdf);
    pdf = NULL;
  }
}

LoadFile::LoadFile(const QString & fname) :
  filename(""),
  loaded(false),
  loading(false),
  cache(NULL),
  pdf(NULL),
  details(0),
  pdfLoader(NULL)
{
  // Parse info

  QByteArray ba = fname.toLatin1(); // This maybe not appropriate for non-latin or english languages

  GooString gooname(ba.data());

  debug(QString(tr("Opening File: %1")).arg(fname));

  PDFDoc *pdfDoc = new PDFDoc(&gooname);
  if (!pdfDoc->isOk()) {
    const int err = pdfDoc->getErrorCode();
    QString msg = tr("Unknown");

    switch (err) {
      case errOpenFile:
      case errFileIO:
        msg = tr("Couldn't open file");
      break;
      case errBadCatalog:
      case errDamaged:
      case errPermission:
        msg = tr("Damaged PDF file");
      break;
    }

    warn(QString("%1, %2").arg(err).arg(msg));

    return;
  }

  if (pdfLoader) {
    // Free the old one, clean all related parameters
    clean();
  }

  filename = fname;
  pdf      = pdfDoc;
  pages    = pdf->getNumPages();
  maxw     = maxh
           = first_visible
           = last_visible = 0;

  // Start threaded magic
  if (pages < 1) {
    warn(QString(tr("Couldn't open %1, perhaps it's corrupted?")).arg(fname));
    return;
  }

  cache = (cachedpage *) xcalloc(pages, sizeof(cachedpage));

  if (!globalParams) {
    globalParams = new GlobalParams;
  }

  dopage(0);

  if (pdfLoader == NULL) {
    pdfLoader = new PDFLoader(this);
  }

  connect(pdfLoader, &PDFLoader::resultReady, this, &LoadFile::handleResults);

  pdfLoader->start();
}

void LoadFile::handleResults()
{
  debug(tr("Document Load complete!"));

  emit resultReady();
}

LoadFile::~LoadFile()
{
  clean();
}
