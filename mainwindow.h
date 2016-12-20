#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

  private slots:
    void showToolbar();
    void hideToolbar();
    void updateButtons();
    void updateTrimButtons();
    void onFullScreen();
    void aboutBox();

  private:
    Ui::MainWindow *ui;

    bool toolbarVisible;
};

#endif // MAINWINDOW_H
