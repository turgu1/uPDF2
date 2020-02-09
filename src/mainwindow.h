/*
Copyright (C) 2017, 2020 Guy Turcotte
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
#include "bookmarksdb.h"
#include "pdfviewer.h"

class DocumentTab;

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
    void     closeEvent();

  private slots:
    void            tabChange(int index);
    void             closeTab(int index);
    void          showToolbar();
    void          hideToolbar();
    void        updateButtons(ViewState & state);
    void    updateTrimButtons(ViewState & state);
    void         onFullScreen();
    void             aboutBox();
    void             openFile();
    void             closeApp();
    void           openRecent();
    void       askPreferences();
    void showBookmarkSelector();
    void          addBookmark();
    void        fileIsLoading(bool isLoading);

  private:
    ViewState        currentState;

    Ui::MainWindow * ui;
    QIcon          * iconFullScreen;
    QIcon          * iconRestore;
    bool             toolbarVisible;
    QMovie         * busyMovie;
    DocumentTab    * currentDocumentTab;

    void              loadFile(QString filename, QString title, int atPage = 0);
    void    saveFileParameters();
    void setFileViewParameters(FileViewParameters & params, bool recent);
    void        loadRecentFile(FileViewParameters & params);

};

#endif // MAINWINDOW_H
