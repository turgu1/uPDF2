#ifndef LOADFILE_H
#define LOADFILE_H

#include <QObject>
#include <QThread>

#include <PDFDoc.h>
#include <splash/SplashBitmap.h>

#include "pdfloader.h"

struct cachedpage {
  u8  * data;
  u32   size;
  u32   uncompressed;

  u32   w, h;
  u16   left, right, top, bottom;

  bool  ready;
};

class LoadFile : public QObject
{
    Q_OBJECT

  private:
    QString      filename;
    bool         loaded;
    bool         loading;
    cachedpage * cache;
    PDFDoc     * pdf;
    u32          maxw, maxh;

    u32          pages;

    u32          first_visible;
    u32          last_visible;
    u32          details;

    PDFLoader  * pdfLoader;

    void dopage(const u32 page);
    void store(SplashBitmap * const bm, cachedpage & cache);
    friend void PDFLoader::run();

  public:
    LoadFile(const QString & fname);
    ~LoadFile();
    void clean();

  public slots:
    void handleResults();

  signals:
    void refresh();
    void resultReady();
};

#endif // LOADFILE_H
