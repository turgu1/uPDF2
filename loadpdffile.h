#ifndef LOADPDFFILE_H
#define LOADPDFFILE_H

#include <QObject>
#include <QThread>

#include <splash/SplashBitmap.h>

#include "pdfloader.h"

class LoadPDFFile : public QObject
{
    Q_OBJECT

  private:
    PDFFile    & file;
    u32          details;

    PDFLoader  * pdfLoader;

//  friend void PDFLoader::run();

  public:
    LoadPDFFile(const QString & fname, PDFFile & pdfFile);
    ~LoadPDFFile();
    void clean();

  public slots:
    void handleResults();

  signals:
    void refresh();
    void resultReady();
};

#endif // LOADPDFFILE_H
