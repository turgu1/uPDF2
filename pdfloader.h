#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QObject>
#include <QThread>
#include <QThreadPool>

#include "updf.h"
#include "pdffile.h"

class PDFLoader : public QThread
{
    Q_OBJECT

  public:
    PDFLoader(PDFFile & pdfFile);
    void run() Q_DECL_OVERRIDE;

  signals:
    void loadCompleted();
    void refresh();

  public slots:
    void abort();
    void refreshRequest();

  private:
    bool          aborting;
    u32           details;
    PDFFile     & pdfFile;
    QThreadPool * threadPool;
};

#endif // PDFLOADER_H
