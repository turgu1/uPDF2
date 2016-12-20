#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QObject>

class LoadFile;

class PDFLoader: public QObject
{
    Q_OBJECT

  public:
    PDFLoader(LoadFile * loadFile);

  signals:
    void resultReady(const QString &result);

  public slots:
    void abort();
    void renderer();

  private:
    bool aborting;
    u32  details;
    LoadFile * file;

};

#endif // PDFLOADER_H
