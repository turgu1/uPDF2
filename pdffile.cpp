#include "updf.h"
#include "pdffile.h"

PDFFile::PDFFile(QObject * parent) : QObject(parent),
  valid(false),
  loaded(false),
  loading(false),
  cache(NULL),
  pdf(NULL)
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
  if (loading) emit fileLoading();
}

void PDFFile::setLoaded(bool val)
{
  loaded = val;
  if (loaded)  emit fileLoaded();
}

void PDFFile::pageCompleted(u32 pageNbr)
{
  emit pageLoadCompleted(pageNbr);
}
