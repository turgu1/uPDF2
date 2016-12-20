#ifndef LOADFILE_H
#define LOADFILE_H

#include <QObject>
#include <QThread>

#include <PDFDoc.h>
#include <splash/SplashBitmap.h>

#include "updf.h"
#include "pdfloader.h"

struct cachedpage {
  u8  * data;
  u32   size;
  u32   uncompressed;

  u32   w, h;
  u16   left, right, top, bottom;

  bool  ready;
};

enum msg {
  MSG_REFRESH = 0,
  MSG_READY
};

class LoadFile : public QObject
{
    Q_OBJECT

  private:
    char       * filename;
    bool         loaded;
    bool         loading;
    cachedpage * cache;
    PDFDoc     * pdf;
    u32          maxw, maxh;

    u32          pages;

    u32          first_visible;
    u32          last_visible;
    u32          details;

    QThread      loaderThread;

    PDFLoader  * pdfLoader;

    void dopage(const u32 page);
    void store(SplashBitmap * const bm, cachedpage & cache);
    friend void PDFLoader::renderer();

  public:
    LoadFile(const char * fname);
    ~LoadFile();

  signals:
    void refresh();
};

#endif // LOADFILE_H
