#include "pdfviewer.h"

#include <QString>
#include <QRect>
#include <QSizePolicy>
#include <QVBoxLayout>

PDFViewer::PDFViewer(QWidget * parent) : QWidget(parent),
  myParent(*parent)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update();
  updateGeometry();
  //myParent.setLayout(new QVBoxLayout);
}

void PDFViewer::setPDFFile(PDFFile * f)
{
  pdfFile = f;
  update();
}

void PDFViewer::mouseMoveEvent(QMouseEvent * event)
{
  QWidget::mouseMoveEvent(event);
}

void PDFViewer::mousePressEvent(QMouseEvent * event)
{
  QWidget::mousePressEvent(event);
}

void PDFViewer::paintEvent(QPaintEvent * event)
{
  const QRect & g = geometry();
  const QRect & gp = myParent.geometry();

  debug(QString("Geometry: %1 %2 %3 %4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height()));
  debug(QString("Parent Geometry: %1 %2 %3 %4").arg(gp.x()).arg(gp.y()).arg(gp.width()).arg(gp.height()));
  QWidget::paintEvent(event);
}

QSize PDFViewer::sizeHint() const
{
  return QSize(0,0);
}

