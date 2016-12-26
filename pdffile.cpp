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
