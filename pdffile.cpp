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

PDFFile::PDFFile(QObject * parent) : QObject(parent),
  valid(false),
  loaded(false),
  loading(false),
  cache(NULL),
  pdf(NULL),
  pages(0)
{
}

PDFFile::~PDFFile()
{
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

void PDFFile::setLoaded(bool val)
{
  loaded = val;
  if (loaded)  emit fileLoadCompleted();
}

void PDFFile::pageCompleted()
{
  emit pageLoadCompleted();
}
