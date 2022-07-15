/*
Copyright (C) 2015 Lauri Kasanen
Modifications Copyright (C) 2017 Guy Turcotte

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>
#include <splash/SplashBitmap.h>
#include <QDebug>
#include <QElapsedTimer>
#include <QBuffer>

#include "pdfpageworker.h"

PDFPageWorker::PDFPageWorker(PDFFile & file, const u32 pageNbr) :
  pdfFile(file),
  page(pageNbr)
{

}

#define METRICS 1

#if 0

// This version is a little bit faster (about 33% faster) than the original algo,
// but maybe not good enough to be retained, considering the readability lost
// for the developper and the amount of time spent to do the calculation that is
// already minimal (~200us per page...) (GT)
//
// May *not* work on a big endian architecture if not running under gnu linux or OSX.
// It is also expected that the pixels will be stored at a 32 bit boundary address.

#if __gnu_linux__ || (__APPLE__ && __MACH__)
  #include <endian.h>
#endif

static bool nonwhite(const u32 pixel)
{
  return (pixel & le32toh(0x00FFFFFF)) != le32toh(0x00FFFFFF);
}

static void getmargins(const u8 * const src, const u32 w, const u32 h,
      const u32 rowsize,
      u32 *minx, u32 *maxx,
      u32 *miny, u32 *maxy)
{

  #if METRICS
    struct timeval start, end;
    static bool firstPage = true;

    if (firstPage) {
      gettimeofday(&start, NULL);
    }
  #endif

  int i, j;
  const u32 *pixel;
  const u32 *pixel2;

  bool found = false;
  pixel2 = (const u32 *) src;
  pixel = pixel2++;
  for (i = 0; i < (int) w && !found; i++) {
    for (j = 0; j < (int) h && !found; j++) {
      if (nonwhite(*pixel)) {
        found = true;
        *minx = i;
      }
      else {
        pixel += w;
      }
    }
    pixel = pixel2++;
  }

  found = false;
  pixel = (const u32 *)(src) + *minx;
  for (j = 0; j < (int) h && !found; j++) {
    for (i = *minx; i < (int) w && !found; i++) {
      if (nonwhite(*pixel++)) {
        found = true;
        *miny = j;
      }
    }
    pixel += *minx;
  }

  const int startx = *minx, starty = *miny;

  found = false;
  pixel2 = (const u32 *)(src) + (w - 1 + starty * w);
  pixel  = pixel2--;

  for (i = w - 1; i >= startx && !found; i--) {
    for (j = starty; j < h && !found; j++) {
      if (nonwhite(*pixel)) {
        found = true;
        *maxx = i;
      }
      else {
        pixel += w;
      }
    }
    pixel = pixel2--;
  }

  found = false;
  pixel = pixel2 = (const u32 *)(src) + ((h - 1) * w + startx);
  for (j = h - 1; j >= starty && !found; j--) {
    for (i = startx; i <= *maxx && !found; i++) {
      if (nonwhite(*pixel++)) {
        found = true;
        *maxy = j;
      }
    }
    pixel = (pixel2 -= w);
  }

  #if METRICS
    if (firstPage) {
      firstPage = false;

      gettimeofday(&end, NULL);
      const u32 us = usecs(start, end);

      qInfo() <<
        "Processing the first page took" <<
        us <<
        "us (" <<
        (us / 1000000.0f) <<
        " s)" << endl;
    }
  #endif
}

#else

// This is the original algorithm from Lauri Kasanen (GT)

static bool nonwhite(const u8 * const pixel)
{
  return
    pixel[0] != 255 ||
    pixel[1] != 255 ||
    pixel[2] != 255;
}

static void getmargins(
        const u8  * const src,
        const u32   w,
        const u32   h,
        const u32   rowsize,
              u32 * minx,
              u32 * maxx,
              u32 * miny,
              u32 * maxy,
        int pageNbr)
{
  #if METRICS
    struct timeval start, end;
    static bool firstPage = true;

    if (firstPage) {
      gettimeofday(&start, NULL);
    }
  #endif

  u32 i, j;

  bool found = false;
  for (i = 0; i < w && !found; i++) {
    for (j = 0; j < h && !found; j++) {
      const u8 * const pixel = src + (j * rowsize) + (i * 4);
      if (nonwhite(pixel)) {
        // if (pageNbr == 1) qDebug() << "Min x at " << j;
        found = true;
        *minx = i;
      }
    }
  }

  found = false;
  for (j = 0; j < h && !found; j++) {
    for (i = *minx; i < w && !found; i++) {
      const u8 * const pixel = src + (j * rowsize) + (i * 4);
      if (nonwhite(pixel)) {
        // if (pageNbr == 1) qDebug() << "Min y at " << i;
        found = true;
        *miny = j;
      }
    }
  }

  const u32 startx = *minx, starty = *miny;

  found = false;
  for (i = w - 1; i > startx && !found; i--) {
    for (j = h - 1; j > starty && !found; j--) {
      const u8 * const pixel = src + (j * rowsize) + (i * 4);
      if (nonwhite(pixel)) {
        // if (pageNbr == 1) qDebug() << "Max x at " << j;
        found = true;
        *maxx = i;
      }
    }
  }

  found = false;
  for (j = h - 1; j > starty && !found; j--) {
    for (i = *maxx; i > startx && !found; i--) {
      const u8 * const pixel = src + (j * rowsize) + (i * 4);
      if (nonwhite(pixel)) {
        // if (pageNbr == 1) qDebug() << "Max y at " << i;
        found = true;
        *maxy = j;
      }
    }
  }

  #if METRICS
    if (firstPage) {
      firstPage = false;

      gettimeofday(&end, NULL);
      const u32 us = usecs(start, end);

      qInfo() <<
        "Processing the first page took" <<
        us <<
        "us (" <<
        (us / 1000000.0f) <<
        " s)" << Qt::endl;
    }
  #endif
}

#endif

#undef METRICS

void store(SplashBitmap const & bm, CachedPage & cache, int pageNbr)
{
//  const u32 w          = pg.width();
//  const u32 h          = pg.height();
//  const u32 rowsize    = pg.bytesPerLine();
//  const u8 * const src = pg.constBits();

  const u32 w          = bm.getWidth();
  const u32 h          = bm.getHeight();
  const u32 rowsize    = bm.getRowSize();
  const u8 * const src = bm.getDataPtr();

  u32 minx = 0,
      miny = 0,
      maxx = 0,
      maxy = 0;

  // Trim margins
  getmargins(src, w, h, rowsize, &minx, &maxx, &miny, &maxy, pageNbr);

  const u32 trimw = maxx - minx + 1;
  const u32 trimh = maxy - miny + 1;

//  if (pageNbr == 1) {
//    qDebug() << "Values " << maxx << maxy << minx << miny;
//    qDebug() << "First pixel" << src[0] << src[1] << src[2] << src[3];
//  }

  u8 * const trimmed = (u8 *) xcalloc(trimw * trimh * 4, 1);
  for (u32 j = miny; j <= maxy; j++) {
    const u32 destj = j - miny;
    memcpy(trimmed + destj * trimw * 4, src + j * rowsize + minx * 4, trimw * 4);
  }

  // Trimmed copy done, compress it

  QImage img(trimmed, trimw, trimh, QImage::Format_RGB32);

  QBuffer buf(&cache.data);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, "PNG", 50);
  buf.close();

  qDebug() << "Page " << pageNbr << " size: " << cache.data.size() / 1024.0 << "KB";

  // Store

  cache.uncompressed = trimw * trimh * 4;
  cache.w            = trimw;
  cache.h            = trimh;
  cache.left         = minx;
  cache.right        = w - maxx;
  cache.top          = miny;
  cache.bottom       = h - maxy;
  //cache.size         = tmp.size();
  //cache.data         = dst;

  free(trimmed);
}

void PDFPageWorker::run()
{
  SplashColor       white  = { 255, 255, 255 };
  SplashOutputDev * splash = new SplashOutputDev(splashModeXBGR8, 4, false, white);
  splash->startDoc(pdfFile.pdf);

  pdfFile.pdf->displayPage(splash, page + 1, 144, 144, 0, true, false, false);

  SplashBitmap * const bm = splash->takeBitmap();

//  QSize size = pdfFile.pdf->pageSize(page).toSize();
//  size.setWidth(size.width() * 2);
//  size.setHeight(size.height() * 2);
//  QImage pg = pdfFile.pdf->render(page, size);
//  store(pg, pdfFile.cache[page], page);

  store(*bm, pdfFile.cache[page], page);

  delete bm;
  delete splash;

//  qDebug() << "Page "         << page
//           << ", Width "      << c.w
//           << ", Height "     << c.h
//           << ", Left "       << c.left
//           << ", Right "      << c.right
//           << ", Top "        << c.top
//           << ", Bottom "     << c.bottom
//           << ", Size "       << c.size
//           << ", Uncompress " << c.uncompressed;

  __sync_bool_compare_and_swap(&pdfFile.cache[page].ready, 0, 1);

  // If this page was visible, tell the app to refresh
  const u32 first = __sync_fetch_and_add(&pdfFile.firstVisible, 0);
  const u32 last  = __sync_fetch_and_add(&pdfFile.lastVisible,  0);
  if (page >= first && page <= last) {
    emit refresh();
  }
}
