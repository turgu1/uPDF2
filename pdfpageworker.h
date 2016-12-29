#ifndef PDFPAGEWORKER_H
#define PDFPAGEWORKER_H

#include <QObject>
#include <QRunnable>

#include "updf.h"
#include "pdffile.h"

class PDFPageWorker : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    PDFPageWorker(PDFFile & file, const u32 pageNbr);
    void run();

  private:
    PDFFile & pdfFile;
    u32 page;

  signals:
    void refresh();
};

#endif // PDFPAGEWORKER_H
