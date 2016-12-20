#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

#include "loadfile.h"

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
    QIcon * iconFull;
    QIcon * iconRestore;

    bool toolbarVisible;

    LoadFile * loadedFile;
};

#endif // MAINWINDOW_H
