/*
Copyright (C) 2017 Guy Turcotte
Portion Copyright (C) 2015 Lauri Kasanen

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

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
