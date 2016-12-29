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

#include "pdfloader.h"
#include "pdfpageworker.h"

PDFLoader::PDFLoader(PDFFile & pdfFile) :
  aborting(false),
  pdfFile(pdfFile)
{
  if (!globalParams) {
    globalParams = new GlobalParams;
  }
  threadPool = QThreadPool::globalInstance();
}

static bool notdone(const bool arr[], const u32 num) {
  u32 i;
  for (i = 0; i <= num; i++) {
    if (!arr[i]) return true;
  }
  return false;
}

void PDFLoader::abort()
{
  aborting = true;
}

void PDFLoader::refreshRequest()
{
  emit refresh();
}

void PDFLoader::run()
{
  // Optional timing
  struct timeval start, end;
  gettimeofday(&start, NULL);

  const u32 chunksize = QThread::idealThreadCount() * 3;

  if (pdfFile.pages < chunksize) {
    for (u32 i = 1; i < pdfFile.pages; i++) {
      PDFPageWorker * pw = new PDFPageWorker(pdfFile, i);
      connect(pw, SIGNAL(refresh()), this, SLOT(refreshRequest()));
      threadPool->start(pw);
    }
    threadPool->waitForDone(10000);
  } else {
    // With a lot of pages, the user may want to go far before things
    // are fully loaded, say to page 500. Render in chunks and adapt.

    u32 c;
    const u32 chunks    = pdfFile.pages / chunksize;
    const u32 remainder = pdfFile.pages % chunksize;

    bool done[chunks + 1];
    memset(done, 0, sizeof(bool) * (chunks + 1));

    while (notdone(done, chunks)) {
      for (c = 0; c <= chunks; c++) {

        if (aborting) return;

        // Did the user skip around?
        const u32 first = __sync_fetch_and_add(&pdfFile.firstVisible, 0);
        if (first) {
          const u32 tmp = pdfFile.firstVisible / chunksize;
          if (tmp <= chunks && !done[tmp]) c = tmp;
        }

        if (done[c]) continue;

        u32 max = (c + 1) * chunksize;
        if (c == chunks) max -= (chunksize - remainder);

        for (u32 i = c * chunksize; i < max; i++) {
          PDFPageWorker * pw = new PDFPageWorker(pdfFile, i);
          connect(pw, SIGNAL(refresh()), this, SLOT(refreshRequest()));
          threadPool->start(pw);
        }
        threadPool->waitForDone(10000);
        done[c] = true;
      }
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

  u32 maxW = 0, maxH = 0;
  for (u32 i = 0; i < pdfFile.pages; i++) {
    if (pdfFile.cache[i].w > maxW)
      maxW = pdfFile.cache[i].w;
    if (pdfFile.cache[i].h > maxH)
      maxH = pdfFile.cache[i].h;
  }
  pdfFile.maxW = maxW;
  pdfFile.maxH = maxH;

  // Set normal cursor
  pdfFile.setLoaded(true);

  return;
}
