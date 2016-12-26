#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLineEdit>
#include <QMovie>

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
    void  keyPressEvent(QKeyEvent * event) Q_DECL_OVERRIDE;

  private slots:
    void showToolbar();
    void hideToolbar();
    void updateButtons(ViewState & state);
    void updateTrimButtons();
    void onFullScreen();
    void aboutBox();
    void openFile();

  private:
    ViewState        currentState;

    Ui::MainWindow * ui;
    PDFViewer      * pdfViewer;
    QIcon          * iconFullScreen;
    QIcon          * iconRestore;
    bool             toolbarVisible;
    PDFFile          file;
    LoadPDFFile    * loadedPDFFile;
    QMovie         * busyMovie;
};

#endif // MAINWINDOW_H
