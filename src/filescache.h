#ifndef FILESCACHE_H
#define FILESCACHE_H

#include <QObject>
#include <QList>

#include "updf.h"

class PDFFile;

class FilesCache : public QObject
{
    Q_OBJECT
public:
    explicit   FilesCache(QObject * parent = nullptr);
    PDFFile *     getFile(QString   filename);
    void      releaseFile(PDFFile * f);
signals:
    void busy(bool isBusy);

private:
    QList<PDFFile *> files;
    int loadingCount;

private slots:
    void fileIsLoading();
    void fileLoadCompleted();
};

#endif // FILESCACHE_H
