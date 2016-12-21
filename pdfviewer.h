#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QObject>
#include <QWidget>

#include "updf.h"
#include "pdffile.h"
#include "loadpdffile.h"

class PDFViewer : public QWidget
{
  public:
    PDFViewer(QWidget * parent = 0);
    void setPDFFile(PDFFile * f);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void paintEvent(QPaintEvent * event);

  private:
    PDFFile * pdfFile;
    QWidget & myParent;
};

#endif // PDFVIEWER_H
