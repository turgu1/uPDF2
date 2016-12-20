#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    toolbarVisible(true)
{
    ui->setupUi(this);
    updateButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::aboutBox()
{
  QMessageBox aboutBox;

  aboutBox.setIconPixmap(QPixmap(":/icons/48/img/48x48/updf.png"));
  aboutBox.setText("\nuPDF (micro PDF) Version 1.0");
  aboutBox.setDetailedText("Written by:\n\n(c) 2017 - Guy Turcotte\n(c) 2015 - Lauri Kasanen\n\nGNU General Public License 3.0");
  aboutBox.setWindowTitle("About uPDF");
  aboutBox.exec();
}

void MainWindow::onFullScreen()
{
  if (isFullScreen()) {
    showNormal();
    ui->fullScreenButton->setIcon(QIcon::QIcon(":/icons/32/img/32x32/view-fullscreen.png"));
    ui->fullScreenButton->setToolTip("Full Screen View");
  }
  else {
    showFullScreen();
    ui->fullScreenButton->setIcon(QIcon::QIcon(":/icons/32/img/32x32/view-restore.png"));
    ui->fullScreenButton->setToolTip("Normal View");
  }
}

void MainWindow::updateTrimButtons()
{
  if (ui->editTrimButton->isChecked()) {
    ui->evenOddButton->show();
    ui->thisPageButton->show();
    ui->trimPageList->show();
    ui->clearTrimsButton->show();

    ui->selectTextButton->hide();
    ui->zoomFrame->hide();
  }
  else {
    ui->evenOddButton->hide();
    ui->thisPageButton->hide();
    ui->trimPageList->hide();
    ui->clearTrimsButton->hide();

    ui->selectTextButton->show();
    ui->zoomFrame->show();
  }
}

void MainWindow::updateButtons()
{
  if (toolbarVisible) {
    ui->showToolbarFrame->hide();
    ui->toolbarFrame->show();
    if (ui->viewModeCombo->currentIndex() == 4) {
      ui->trimFrame->show();
      updateTrimButtons();
    }
    else {
      ui->trimFrame->hide();
      ui->zoomFrame->show();
    }
  }
  else {
    ui->showToolbarFrame->show();
    ui->toolbarFrame->hide();
  }

}

void MainWindow::showToolbar()
{
  toolbarVisible = true;
  updateButtons();
}

void MainWindow::hideToolbar()
{
  toolbarVisible = false;
  updateButtons();
}
