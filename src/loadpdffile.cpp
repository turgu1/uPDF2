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

#include <ErrorCodes.h>
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
    pdfLoader = NULL;
  }

  if (file.cache) {
    u32 i;
    const u32 max = file.pages;
    for (i = 0; i < max; i++) {
      if (file.cache[i].ready) {
        free(file.cache[i].data);
      }
    }
    free(file.cache);
    file.cache = NULL;
  }

  file.filename = "";
  if (file.pdf) {
    free(file.pdf);
    file.pdf = NULL;
  }
  file.setLoaded(false);
  file.setLoading(false);
}

LoadPDFFile::LoadPDFFile(const QString & fname, PDFFile & pdfFile) :
  file(pdfFile),
  details(0),
  pdfLoader(NULL)
{
  // Parse info

  QByteArray ba = fname.toLatin1(); // This maybe not appropriate for non-latin languages
  GooString gooname(ba.data());

  qDebug() << "Opening File: " << fname << endl;

  PDFDoc *pdfDoc = new PDFDoc(&gooname);
  if (!pdfDoc->isOk()) {
    const int err = pdfDoc->getErrorCode();
    QString msg = tr("Unknown");

    switch (err) {
      case errOpenFile:
      case errFileIO:
        msg = tr("Couldn't open file");
      break;
      case errBadCatalog:
      case errDamaged:
      case errPermission:
        msg = tr("Damaged PDF file");
      break;
    }

    qCritical() << err << ", " << msg << endl;

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
    qCritical() << QString(tr("Couldn't open ")) << fname << QString(tr("perhaps it's corrupted?")) << endl;
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
  qDebug() << tr("Document Load complete!") << endl;

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
