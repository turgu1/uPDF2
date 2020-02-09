#include "filescache.h"
#include "pdffile.h"

FilesCache::FilesCache(QObject *parent) : QObject(parent),
    loadingCount(0)
{

}

PDFFile * FilesCache::getFile(QString filename)
{
    int i;

    for (i = 0; i < files.count(); i++) {
        if (files[i]->filename == filename) break;
    }

    if (i >= files.count()) {
        PDFFile * f = new PDFFile;
        files.append(f);

        connect(f, SIGNAL(    fileIsLoading()), this, SLOT(    fileIsLoading()));
        connect(f, SIGNAL(fileLoadCompleted()), this, SLOT(fileLoadCompleted()));

        f->load(filename);
        return f;
    }
    else {
        PDFFile * f = files[i];
        f->setViewerCount(f->getViewerCount() + 1);
        return f;
    }
}

void FilesCache::releaseFile(PDFFile * f)
{
    int index = files.indexOf(f);

    if (index >= 0) {
        f->setViewerCount(f->getViewerCount() - 1);
        if (f->getViewerCount() == 0) {
            delete f;
            files.removeAt(index);
        }
    }
}

void FilesCache::fileIsLoading()
{
    loadingCount += 1;
    if (loadingCount == 1) {
        emit busy(true);
    }
}

void FilesCache::fileLoadCompleted()
{
    loadingCount -= 1;
    if (loadingCount <= 0) {
        loadingCount = 0;
        emit busy(false);
    }
}
