#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "updf.h"
#include "loadpdffile.h"
#include "pdffile.h"
#include "pdfviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget * parent = 0);
    ~MainWindow();

  private slots:
    void showToolbar();
    void hideToolbar();
    void updateButtons();
    void updateTrimButtons();
    void onFullScreen();
    void aboutBox();
    void openFile();

  private:
    Ui::MainWindow * ui;
    PDFViewer      * pdfViewer;
    QIcon          * iconFullScreen;
    QIcon          * iconRestore;
    bool             toolbarVisible;
    PDFFile          file;
    LoadPDFFile    * loadedPDFFile;
};

#endif // MAINWINDOW_H
