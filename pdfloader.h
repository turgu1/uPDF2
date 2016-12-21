#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QObject>
#include <QThread>

#include "updf.h"
#include "pdffile.h"

class PDFLoader : public QThread
{
    Q_OBJECT

  public:
    PDFLoader(PDFFile & pdfFile);
    void run() Q_DECL_OVERRIDE;
    void dopage(const u32 page);

  signals:
    void resultReady();
    void refresh();

  public slots:
    void abort();

  private:
    bool      aborting;
    u32       details;
    PDFFile & pdfFile;

};

#endif // PDFLOADER_H
