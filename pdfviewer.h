#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QObject>
#include <QWidget>

#include "updf.h"
#include "loadfile.h"

class PDFViewer : QWidget
{
  public:
    PDFViewer(QWidget * parent = 0);
    void setLoadFile(LoadFile *lf);

  private:
    LoadFile * loadFile;
};

#endif // PDFVIEWER_H
