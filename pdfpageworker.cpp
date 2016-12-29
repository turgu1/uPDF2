#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>

#include "pdfpageworker.h"

PDFPageWorker::PDFPageWorker(PDFFile & file, const u32 pageNbr) :
  pdfFile(file),
  page(pageNbr)
{

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
  for (u32 j = miny; j <= maxy; j++) {
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

void PDFPageWorker::run()
{
  SplashColor       white  = { 255, 255, 255 };
  SplashOutputDev * splash = new SplashOutputDev(splashModeXBGR8, 4, false, white);
  splash->startDoc(pdfFile.pdf);

  pdfFile.pdf->displayPage(splash, page + 1, 144, 144, 0, true, false, false);

  SplashBitmap * const bm = splash->takeBitmap();

  store(bm, pdfFile.cache[page]);

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
