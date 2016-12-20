#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QObject>
#include <QThread>

#include "updf.h"

class LoadFile;

class PDFLoader: public QThread
{
    Q_OBJECT

  public:
    PDFLoader(LoadFile * loadFile);
    void run() Q_DECL_OVERRIDE;

  signals:
    void resultReady();

  public slots:
    void abort();

  private:
    bool aborting;
    u32  details;
    LoadFile * file;

};

#endif // PDFLOADER_H
