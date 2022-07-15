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

//#include <ErrorCodes.h>
#include <QThreadPool>
#include <QDebug>

#include "updf.h"
#include "loadpdffile.h"
#include "pdfpageworker.h"

void LoadPDFFile::clean()
{
  if (pdfLoader) {
    pdfLoader->abort();
    pdfLoader->wait();

    delete pdfLoader;
    pdfLoader = nullptr;
  }

  if (file.cache) {
    u32 i;
    const u32 max = file.pages;
    for (i = 0; i < max; i++) {
      if (file.cache[i].ready) {
        file.cache[i].data.clear();
        file.cache[i].ready = false;
      }
    }
    free(file.cache);
    file.cache = nullptr;
  }

  file.filename = "";
  if (file.pdf) {
    delete file.pdf;
    file.pdf = nullptr;
  }
  file.setLoaded(false);
  file.setLoading(false);
}

//GooString *QStringToUnicodeGooString(const QString &s)
//{
//    int len = s.length() * 2 + 2;
//    char *cstring = (char *)gmallocn(len, sizeof(char));
//    cstring[0] = 0xfe;
//    cstring[1] = 0xff;
//    for (int i = 0; i < s.length(); ++i)
//    {
//       cstring[2+i*2] = s.at(i).row();
//       cstring[3+i*2] = s.at(i).cell();
//    }
//    GooString *ret = new GooString(cstring, len);
//    gfree(cstring);
//    return ret;
//}

LoadPDFFile::LoadPDFFile(const QString & fname, PDFFile & pdfFile) :
  file(pdfFile),
  details(0),
  pdfLoader(nullptr)
{
  // Parse info
//  qDebug() << "Opening File: " << fname << Qt::endl;

//  QPdfDocument *pdfDoc = new QPdfDocument();
//  QPdfDocument::DocumentError res = pdfDoc->load(fname);
//  if (res != QPdfDocument::DocumentError::NoError) {
//    QString msg;

//    switch (res) {
//      case QPdfDocument::DocumentError::FileNotFoundError:
//        msg = tr("File Not Found.");
//        break;
//      case QPdfDocument::DocumentError::InvalidFileFormatError:
//        msg = tr("Invalid File Format.");
//        break;
//      case QPdfDocument::DocumentError::DataNotYetAvailableError:
//        msg = tr("Data Not Yet Available.");
//        break;
//      case QPdfDocument::DocumentError::IncorrectPasswordError:
//        msg = tr("Unsupported Password Protected Document.");
//        break;
//      case QPdfDocument::DocumentError::UnsupportedSecuritySchemeError:
//        msg = tr("Unsupported Security Scheme.");
//        break;
//      default:
//        msg = tr("Unknown");
//        break;
//    }

  QByteArray ba = fname.toLatin1(); // This maybe not appropriate for non-latin languages
  GooString gooname(ba.data());

  qDebug() << "Opening File: " << fname << Qt::endl;

  PDFDoc * pdfDoc = new PDFDoc(std::unique_ptr<GooString>(&gooname));
  if (!pdfDoc->isOk()) {
    const int err = pdfDoc->getErrorCode();
    QString msg = tr("Unknown.");

    switch (err) {
      case errOpenFile:
      case errFileIO:
        msg = tr("Couldn't open file.");
      break;
      case errBadCatalog:
      case errDamaged:
      case errPermission:
        msg = tr("Damaged PDF file.");
      break;
    }

    qCritical() << err << ", " << msg << Qt::endl;

    // qCritical() << "Not Able To Open File [" << fname << "]: " << msg << Qt::endl;

    return;
  }

  // Free the old one, clean all related parameters
  if (pdfLoader) clean();

  file.filename = fname;
  file.pdf      = pdfDoc;
  file.pages    = file.pdf->getNumPages();
  file.maxW     = file.maxH = 0;

  // Start threaded magic
  if (file.pages < 1) {
    qCritical() << QString(tr("Couldn't open ")) << fname << QString(tr("perhaps it's corrupted?")) << Qt::endl;
    return;
  }

  file.cache = (CachedPage *) xcalloc(file.pages, sizeof(CachedPage));

  if (pdfLoader) delete pdfLoader;
  pdfLoader = new PDFLoader(file);

  connect(pdfLoader, SIGNAL(loadCompleted()), this, SLOT(      handleResults()));
  connect(pdfLoader, SIGNAL(      refresh()), this, SLOT(pageReadyForRefresh()));

  // Do first page and wait for the result
  QThreadPool::globalInstance()->start(new PDFPageWorker(file, 0));
  QThreadPool::globalInstance()->waitForDone();

  //pdfLoader->dopage(0);

  file.setValid(true);
  file.setLoading(true);
  pdfLoader->start();   // Do the rest of the document
}

void LoadPDFFile::handleResults()
{
  qDebug() << tr("Document Load complete!") << Qt::endl;

  emit loadCompleted();
}

void LoadPDFFile::pageReadyForRefresh()
{
  file.pageCompleted();
}

LoadPDFFile::~LoadPDFFile()
{
  clean();
}
