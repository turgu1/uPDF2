/*
Copyright (C) 2015 Lauri Kasanen
Modifications Copyright (C) 2017 Guy Turcotte

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <QObject>
#include <QTextStream>
#include <QImage>

#include <PDFDoc.h>
#include <splash/SplashBitmap.h>
#include <SplashOutputDev.h>
//#include <QtPdf>
#include <QFileInfo>

#include "updf.h"

void *xcalloc(size_t nmemb, size_t size) {

  void *tmp = calloc(nmemb, size);
  if (!tmp) {
    qFatal("Out of memory\n");
  }
  return tmp;
}

void *xmalloc(size_t size) {

  void *tmp = malloc(size);
  if (!tmp) {
    qFatal("Out of memory\n");
  }
  return tmp;
}

u32 usecs(const timeval old, const timeval now) {

  u32 us = (now.tv_sec - old.tv_sec) * 1000 * 1000;
  us += now.tv_usec - old.tv_usec;

  return us;
}

u64 msec() {
  struct timeval t;
  gettimeofday(&t, NULL);

  u64 ms = t.tv_sec * 1000;
  ms += t.tv_usec / 1000;
  return ms;
}

//static QPdfDocument * pdf = nullptr;
//static QString pdfFileName;

static PDFDoc * pdf = nullptr;

int getPageCount(QString & filename)
{
//    QPdfDocument::DocumentError res = QPdfDocument::DocumentError::UnknownError;

//    if ((pdf == nullptr) || (pdfFileName != filename)) {
//        if (pdf != nullptr) { free(pdf); pdf = nullptr; }

//        pdf = new QPdfDocument();
//        res = pdf->load(filename);
//        if (res == QPdfDocument::DocumentError::NoError) {
//            pdfFileName = filename;
//        }
//        else {
//            delete pdf;
//            pdfFileName.clear();
//        }
//    }

//    return ((pdf == nullptr) || (!res)) ? 0 : pdf->pageCount();

    GooString * gfilename = new GooString(filename.toLatin1());
    if ((pdf == nullptr) || (pdf->getFileName() != gfilename)) {
        if (pdf != nullptr) { free(pdf); pdf = nullptr; }

        pdf = new PDFDoc(std::unique_ptr<GooString>(gfilename));
    }

    return ((pdf == nullptr) || (!pdf->isOk())) ? 0 : pdf->getNumPages();

}

bool getPageImage(QString & filename, QImage & img, int pixelsPerInch, int page) {

//    QPdfDocument::DocumentError res = QPdfDocument::DocumentError::UnknownError;

//    if ((pdf == nullptr) || (pdfFileName != filename)) {
//        if (pdf != nullptr) { free(pdf); pdf = nullptr; }

//        pdf = new QPdfDocument();
//        res = pdf->load(filename);
//        if (res == QPdfDocument::DocumentError::NoError) {
//            pdfFileName = filename;
//        }
//        else {
//            delete pdf;
//            pdfFileName.clear();
//        }
//    }

    GooString * gfilename = new GooString(filename.toLatin1());
    if ((pdf == nullptr) || (pdf->getFileName() != gfilename)) {
        if (pdf != nullptr) { free(pdf); pdf = nullptr; }

        pdf = new PDFDoc(std::unique_ptr<GooString>(gfilename));
    }

//    if ((pdf != nullptr) && !res) {

    if ((pdf != nullptr) && pdf->isOk()) {
        SplashColor       white  = { 255, 255, 255 };
        SplashOutputDev * splash = new SplashOutputDev(splashModeXBGR8, 4, false, white);
        splash->startDoc(pdf);

        pdf->displayPage(splash, page, pixelsPerInch, pixelsPerInch, 0, true, false, false);

        SplashBitmap * const bm = splash->takeBitmap();

        img = QImage(bm->getDataPtr(), bm->getWidth(), bm->getHeight(), QImage::Format_RGB32);

//        QSize size = pdf->pageSize(page).toSize();
//        size.setHeight(size.height() * pixelsPerInch / 72);
//        size.setWidth(size.width() * pixelsPerInch / 72);

//        img = pdf->render(page, size);
    }
    else {
        return false;
    }

    return true;
}

QString absoluteFilename(const QString & filename)
{
    QString & prefix = preferences.bookmarksParameters.pdfFolderPrefix;
    if (prefix.right(1) != "/") prefix += "/";

    return QFileInfo::exists(filename) ? filename : prefix + filename;
}

QString relativeFilename(const QString & filename)
{
    QString & prefix = preferences.bookmarksParameters.pdfFolderPrefix;
    if (prefix.right(1) != "/") prefix += "/";

    if (filename.length() > prefix.length()) {
        if (filename.left(prefix.length()).compare(prefix, Qt::CaseInsensitive) == 0) {
            return filename.right(filename.length() - prefix.length());
        }
    }

    return filename;
}

QString extractFilename(const QString & filename)
{
    QString name = QFileInfo(filename).fileName();
    if (name.right(4).compare(".pdf", Qt::CaseInsensitive) == 0) {
        return name.left(name.length() - 4);
    }
    return name;
}
