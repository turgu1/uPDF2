#ifndef DOCUMENTTAB_H
#define DOCUMENTTAB_H

#include <QWidget>

#include "updf.h"

class PDFViewer;
class PDFFile;

class DocumentTab : public QWidget
{
    Q_OBJECT
public:
    explicit DocumentTab(QWidget *parent = nullptr);
    ~DocumentTab();

    PDFViewer * getPdfViewer() { return pdfViewer;     }
    QString      getFilename();
    void            loadFile(QString filename);
    void            setFocus();

private:
    PDFViewer      * pdfViewer;
    PDFFile        * file;

signals:

};

#endif // DOCUMENTTAB_H
