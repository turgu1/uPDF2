#ifndef PDFFILE_H
#define PDFFILE_H

#include <QObject>

#include "updf.h"

struct CachedPage {
  u8  * data;
  u32   size;
  u32   uncompressed;

  u32   w, h;
  u16   left, right, top, bottom;

  bool  ready;
};

class PDFFile : public QObject
{
  Q_OBJECT
private:
  bool         valid;
  bool         loaded;
  bool         loading;

public:
  explicit PDFFile(QObject *parent = 0);
  ~PDFFile();

  QString      filename;
  CachedPage * cache;
  PDFDoc     * pdf;
  u32          maxw, maxh;
  u32          pages;
  u32          firstVisible;
  u32          lastVisible;

  void setLoading(bool val);
  void setLoaded(bool val);
  void setValid(bool val);
  bool isValid() { return valid; }
  void pageCompleted(u32 pageNbr);


signals:
  void fileLoading();
  void fileLoaded();
  void fileIsValid();
  void pageLoadCompleted(u32 pageNbr);

public slots:
};

#endif // PDFFILE_H
