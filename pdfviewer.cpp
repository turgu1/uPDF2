#include "pdfviewer.h"

PDFViewer::PDFViewer(QWidget * parent) : QWidget(parent)
{

}

void PDFViewer::setLoadFile(LoadFile *lf)
{
  loadFile = lf;
  update();
}

void PDFViewer::mouseMoveEvent(QMouseEvent *event)
{
  QWidget::mouseMoveEvent(event);
}

void PDFViewer::mousePressEvent(QMouseEvent *event)
{
  QWidget::mousePressEvent(event);
}

void PDFViewer::paintEvent(QPaintEvent *event)
{
  QWidget::paintEvent(event);
}
