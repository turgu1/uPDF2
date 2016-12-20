
#include <ErrorCodes.h>
#include <GlobalParams.h>
#include <SplashOutputDev.h>

#include "updf.h"

#include "pdfloader.h"
#include "loadfile.h"

PDFLoader::PDFLoader(LoadFile * loadFile) :
  aborting(false),
  details(0),
  file(loadFile)
{

}

static bool notdone(const bool arr[], const u32 num) {
  u32 i;
  for (i = 0; i < num; i++) {
    if (!arr[i])
      return true;
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

  if (file->pages < chunksize) {
    #pragma omp parallel for schedule(guided)
    for (u32 i = 1; i < file->pages; i++) {
      file->dopage(i);
    }
  } else {
    // With a lot of pages, the user may want to go far before things
    // are fully loaded, say to page 500. Render in chunks and adapt.

    u32 c;
    const u32 chunks = file->pages / chunksize;
    const u32 remainder = file->pages % chunksize;

    bool done[chunks];
    memset(done, 0, sizeof(bool) * chunks);

    while (notdone(done, chunks)) {
      for (c = 0; c < chunks; c++) {

        if (aborting) return;

        // Did the user skip around?
        const u32 first = __sync_fetch_and_add(&file->first_visible, 0);
        if (first) {
          const u32 tmp = file->first_visible / chunksize;
          if (tmp < chunks && !done[tmp])
            c = tmp;
        }

        const u32 max = (c + 1) * chunksize;
        if (done[c]) continue;
        #pragma omp parallel for schedule(guided)
        for (u32 i = c * chunksize; i < max; i++) {
          file->dopage(i);
        }
        done[c] = true;
      }
    }

    #pragma omp parallel for schedule(guided)
    for (c = 0; c < remainder; c++) {
      file->dopage(c + chunks * chunksize);
    }
  }

  // Print stats
  if (details) {
    u32 total = 0, totalcomp = 0;
    for (u32 i = 0; i < file->pages; i++) {
      total += file->cache[i].uncompressed;
      totalcomp += file->cache[i].size;
    }

    info(QString(tr("Compressed mem usage %1.2fmb, compressed to %2.2f%%"))
      .arg(totalcomp / 1024 / 1024.0f)
      .arg(100 * totalcomp / (float) total));

    gettimeofday(&end, NULL);
    const u32 us = usecs(start, end);

    info(QString(tr("Processing the file took %1u us (%2.2f s)"))
      .arg(us).arg(us / 1000000.0f));
  }

  u32 maxw = 0, maxh = 0;
  for (u32 i = 0; i < file->pages; i++) {
    if (file->cache[i].w > maxw)
      maxw = file->cache[i].w;
    if (file->cache[i].h > maxh)
      maxh = file->cache[i].h;
  }
  file->maxw = maxw;
  file->maxh = maxh;

  // Set normal cursor
  emit resultReady();

  return;
}
