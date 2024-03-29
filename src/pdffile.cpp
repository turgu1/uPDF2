/*
Copyright (C) 2017 Guy Turcotte

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

#include "updf.h"
#include "pdffile.h"
#include "loadpdffile.h"

PDFFile::PDFFile(QObject * parent) : QObject(parent),
  valid(false),
  loaded(false),
  loading(false),
  viewerCount(0),
  cache(NULL),
  pdf(NULL),
  pages(0),
  firstVisible(0),
  lastVisible(0),
  totalSize(0),
  totalSizeCompressed(0),
  loadTime(0)
{
}

PDFFile::~PDFFile()
{
  if (loadedPDFFile) {
    if (!loaded)  emit fileLoadCompleted();
    delete loadedPDFFile;
    loadedPDFFile = nullptr;
  }
}

void PDFFile::setValid(bool val) {
  valid = val;
  if (valid) emit fileIsValid();
}

void PDFFile::setLoading(bool val)
{
  loading = val;
  if (loading) emit fileIsLoading();
}

void PDFFile::load(QString filename, int atPage)
{
  firstVisible = lastVisible = atPage;
  loadedPDFFile = new LoadPDFFile(filename, *this);
}

void PDFFile::setLoaded(bool val)
{
  loaded = val;
  if (loaded)  emit fileLoadCompleted();
}

void PDFFile::pageCompleted()
{
  emit pageLoadCompleted();
}
