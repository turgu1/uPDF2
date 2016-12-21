#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>

#include "updf.h"
#include "pdfloader.h"

PDFLoader::PDFLoader(PDFFile & pdfFile) :
  aborting(false),
  details(1),
  pdfFile(pdfFile)
{
  if (!globalParams) {
    globalParams = new GlobalParams;
  }
}

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

void store(SplashBitmap * const bm, CachedPage & cache)
{
  const u32 w          = bm->getWidth();
  const u32 h          = bm->getHeight();
  const u32 rowsize    = bm->getRowSize();
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

void PDFLoader::dopage(const u32 page)
{
  struct timeval start, end;
  gettimeofday(&start, NULL);

  //debug(QString("do page %1").arg(page));

  SplashColor       white  = { 255, 255, 255 };
  SplashOutputDev * splash = new SplashOutputDev(splashModeXBGR8, 4, false, white);
  splash->startDoc(pdfFile.pdf);

  pdfFile.pdf->displayPage(splash, page + 1, 144, 144, 0, true, false, false);

  gettimeofday(&end, NULL);
  if (details > 1) {
    printf("%u: rendering %u us\n", page, usecs(start, end));
    start = end;
  }

  SplashBitmap * const bm = splash->takeBitmap();

  store(bm, pdfFile.cache[page]);

  gettimeofday(&end, NULL);
  if (details > 1) {
    printf("%u: storing %u us\n", page, usecs(start, end));
    start = end;
  }

  delete bm;
  delete splash;

  __sync_bool_compare_and_swap(&pdfFile.cache[page].ready, 0, 1);

  // If this page was visible, tell the app to refresh
  const u32 first = __sync_fetch_and_add(&pdfFile.firstVisible, 0);
  const u32 last  = __sync_fetch_and_add(&pdfFile.lastVisible,  0);
  if (page >= first && page <= last) {
    emit refresh();
  }
}

static bool notdone(const bool arr[], const u32 num) {
  u32 i;
  for (i = 0; i < num; i++) {
    if (!arr[i]) return true;
  }
  return false;
}

void PDFLoader::abort()
{
  aborting = true;
}

void PDFLoader::run()
{
  // Optional timing
  struct timeval start, end;
  gettimeofday(&start, NULL);

  const u32 chunksize = omp_get_num_procs() * 3;

  if (pdfFile.pages < chunksize) {
    #pragma omp parallel for schedule(guided)
    for (u32 i = 1; i < pdfFile.pages; i++) {
      dopage(i);
    }
  } else {
    // With a lot of pages, the user may want to go far before things
    // are fully loaded, say to page 500. Render in chunks and adapt.

    u32 c;
    const u32 chunks    = pdfFile.pages / chunksize;
    const u32 remainder = pdfFile.pages % chunksize;

    bool done[chunks];
    memset(done, 0, sizeof(bool) * chunks);

    while (notdone(done, chunks)) {
      for (c = 0; c < chunks; c++) {

        if (aborting) return;

        // Did the user skip around?
        const u32 first = __sync_fetch_and_add(&pdfFile.firstVisible, 0);
        if (first) {
          const u32 tmp = pdfFile.firstVisible / chunksize;
          if (tmp < chunks && !done[tmp]) c = tmp;
        }

        const u32 max = (c + 1) * chunksize;
        if (done[c]) continue;
        #pragma omp parallel for schedule(guided)
        for (u32 i = c * chunksize; i < max; i++) {
          dopage(i);
        }
        done[c] = true;
      }
    }

    #pragma omp parallel for schedule(guided)
    for (c = 0; c < remainder; c++) {
      dopage(c + chunks * chunksize);
    }
  }

  // Print stats
  if (details) {
    u32 total = 0, totalcomp = 0;
    for (u32 i = 0; i < pdfFile.pages; i++) {
      total += pdfFile.cache[i].uncompressed;
      totalcomp += pdfFile.cache[i].size;
    }

    info(QString(tr("Compressed mem usage %1mb, compressed to %2%"))
      .arg(totalcomp / 1024 / 1024.0f, 0, 'f', 2)
      .arg(100 * totalcomp / (float) total, 0, 'f', 2));

    gettimeofday(&end, NULL);
    const u32 us = usecs(start, end);

    info(QString(tr("Processing the file took %1 us (%2 s)"))
      .arg(us)
      .arg(us / 1000000.0f, 0, 'f', 2));
  }

  u32 maxw = 0, maxh = 0;
  for (u32 i = 0; i < pdfFile.pages; i++) {
    if (pdfFile.cache[i].w > maxw)
      maxw = pdfFile.cache[i].w;
    if (pdfFile.cache[i].h > maxh)
      maxh = pdfFile.cache[i].h;
  }
  pdfFile.maxw = maxw;
  pdfFile.maxh = maxh;

  // Set normal cursor
  pdfFile.setLoaded(true);

  return;
}
