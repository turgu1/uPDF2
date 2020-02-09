#include "documenttab.h"
#include "pdfviewer.h"
#include "pdffile.h"
#include "filescache.h"

#include <QVBoxLayout>

DocumentTab::DocumentTab(QWidget *parent) :
    QWidget(parent)
{
    QVBoxLayout * layout = new QVBoxLayout;

    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    pdfViewer = new PDFViewer(this);

    layout->addWidget(pdfViewer, 1);
}

DocumentTab::~DocumentTab()
{
    filesCache->releaseFile(file);
    delete pdfViewer;
}

QString DocumentTab::getFilename()
{
    return (file == nullptr) ? QString("") : file->filename;
}

void DocumentTab::loadFile(QString filename)
{
    pdfViewer->reset();
    file = filesCache->getFile(filename);
    pdfViewer->setPDFFile(file);
}

void DocumentTab::setFocus()
{
    if (pdfViewer) pdfViewer->setFocus();
}
