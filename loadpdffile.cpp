/*
Copyright (C) 2015 Lauri Kasanen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

Modifications Copyright (C) 2016 Guy Turcotte
*/

#include <ErrorCodes.h>

#include "updf.h"
#include "loadpdffile.h"

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

  debug(QString(tr("Opening File: %1")).arg(fname));

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

    warn(QString("%1, %2").arg(err).arg(msg));

    return;
  }

  // Free the old one, clean all related parameters
  if (pdfLoader) clean();

  file.filename = fname;
  file.pdf      = pdfDoc;
  file.pages    = file.pdf->getNumPages();
  file.maxw     = file.maxh
                = file.firstVisible
                = file.lastVisible = 0;

  // Start threaded magic
  if (file.pages < 1) {
    warn(QString(tr("Couldn't open %1, perhaps it's corrupted?")).arg(fname));
    return;
  }

  file.cache = (CachedPage *) xcalloc(file.pages, sizeof(CachedPage));

  if (pdfLoader) delete pdfLoader;
  pdfLoader = new PDFLoader(file);

  connect(pdfLoader, &PDFLoader::resultReady, this, &LoadPDFFile::handleResults);

  pdfLoader->dopage(0);
  file.setLoading(true);
  pdfLoader->start();
}

void LoadPDFFile::handleResults()
{
  debug(tr("Document Load complete!"));

  emit resultReady();
}

LoadPDFFile::~LoadPDFFile()
{
  clean();
}
