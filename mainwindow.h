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
    void  closeEvent();

  private slots:
    void showToolbar();
    void hideToolbar();
    void updateButtons(ViewState & state);
    void updateTrimButtons(ViewState & state);
    void onFullScreen();
    void aboutBox();
    void openFile();
    void closeApp();
    void openRecent();

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

    void loadFile(QString filename);
    void saveFileParameters();
    void loadRecentFile(FileViewParameters & params);
};

#endif // MAINWINDOW_H
