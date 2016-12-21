#include "updf.h"
#include "pdffile.h"

PDFFile::PDFFile(QObject * parent) : QObject(parent),
  cache(NULL),
  pdf(NULL),
  loaded(false),
  loading(false)
{
}

PDFFile::~PDFFile()
{
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
