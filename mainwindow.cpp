#include <QFileDialog>
#include <QSignalMapper>
#include <QKeyEvent>

#include "updf.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loadpdffile.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    toolbarVisible(true),
    loadedPDFFile(NULL)
{
  QVBoxLayout * layout = new QVBoxLayout;

  ui->setupUi(this);

  layout->setContentsMargins(0, 0, 0, 0);
  ui->viewer->setLayout(layout);

  pdfViewer = new PDFViewer(ui->viewer);
  pdfViewer->setPDFFile(&file);

  layout->addWidget(pdfViewer, 1);

  iconFullScreen = new QIcon(":/icons/32/img/32x32/view-fullscreen.png");
  iconRestore    = new QIcon(":/icons/32/img/32x32/view-restore.png"   );

  ui->currentPageEdit->setValidator(new QIntValidator(1, 9999, this));

  connect(ui->beginDocumentButton, SIGNAL(clicked()),     pdfViewer, SLOT(top()));
  connect(ui->endDocumentButton,   SIGNAL(clicked()),     pdfViewer, SLOT(bottom()));
  connect(ui->previousPageButton,  SIGNAL(clicked()),     pdfViewer, SLOT(pageUp()));
  connect(ui->nextPageButton,      SIGNAL(clicked()),     pdfViewer, SLOT(pageDown()));
  connect(ui->zoomInButton,        SIGNAL(clicked()),     pdfViewer, SLOT(zoomIn()));
  connect(ui->zoomOutButton,       SIGNAL(clicked()),     pdfViewer, SLOT(zoomOut()));
  connect(ui->editTrimButton,      SIGNAL(clicked(bool)), pdfViewer, SLOT(trimZoneSelect(bool)));

  connect(ui->viewModeCombo,       static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setViewMode(ViewMode(index)); });

  connect(ui->columnCountCombo,    static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setColumnCount(index + 1); });

  connect(ui->titlePageCountCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setTitlePageCount(index); });

  connect(ui->currentPageEdit,     static_cast<void(QLineEdit::*)()>(&QLineEdit::editingFinished),
          [=](){ pdfViewer->gotoPage(ui->currentPageEdit->text().toInt() - 1); });

  connect(pdfViewer,               static_cast<void(PDFViewer::*)(ViewState &)>(&PDFViewer::stateUpdated),
          [=](ViewState & state) { this->updateButtons(state); });

  ui->busyLabel->setMovie(new QMovie(":/agif/img/busy.gif"));

  currentState.page        = 1;
  currentState.pageCount   = 0;
  currentState.viewMode    = VM_PAGE;
  currentState.viewZoom    = 0.05f;
  currentState.fileLoading = false;

  updateButtons(currentState);
  update();
  updateGeometry();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
  if (event->key() == Qt::Key_F8) {
    if (toolbarVisible) hideToolbar(); else showToolbar();
  }
  else {
    QWidget::keyPressEvent(event);
  }

  pdfViewer->setFocus();
}

void MainWindow::aboutBox()
{
  QMessageBox aboutBox;

  aboutBox.setIconPixmap(QPixmap(":/icons/48/img/48x48/updf.png"));
  aboutBox.setText(tr("\nuPDF (micro PDF) Version 1.0"));
  aboutBox.setDetailedText(tr("Written by:\n\n(c) 2017 - Guy Turcotte\n(c) 2015 - Lauri Kasanen\n\nGNU General Public License V3.0"));
  aboutBox.setWindowTitle(tr("About uPDF"));
  aboutBox.exec();

  pdfViewer->setFocus();
}

void MainWindow::onFullScreen()
{
  if (isFullScreen()) {
    showNormal();
    ui->fullScreenButton->setIcon(*iconFullScreen);
    ui->fullScreenButton->setToolTip(tr("Full Screen View"));
  }
  else {
    showFullScreen();
    ui->fullScreenButton->setIcon(*iconRestore);
    ui->fullScreenButton->setToolTip(tr("Normal View"));
  }
  pdfViewer->setFocus();
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
  pdfViewer->setFocus();
}

void MainWindow::updateButtons(ViewState & state)
{
  currentState = state;

  if (toolbarVisible) {
    ui->viewModeCombo->setCurrentIndex(state.viewMode);
    if (state.viewMode == VM_ZOOMFACTOR) {
      ui->viewModeCombo->setCurrentText(QString("%1").arg(state.viewZoom, 0, 'f', 2));
    }
    ui->currentPageEdit->setText(QString("%1").arg(state.page + 1));
    ui->pageCountLabel->setText(QString("/ %1").arg(state.pageCount));
    ui->showToolbarFrame->hide();
    ui->toolbarFrame->show();
    if (ui->viewModeCombo->currentIndex() == VM_CUSTOMTRIM) {
      ui->trimFrame->show();
      updateTrimButtons();
    }
    else {
      ui->trimFrame->hide();
      ui->zoomFrame->show();
    }
    if (state.fileLoading) {
      ui->busyLabel->movie()->start();
      ui->busyLabel->show();
    }
    else {
      ui->busyLabel->movie()->stop();
      ui->busyLabel->hide();
    }
  }
  else {
    ui->showToolbarFrame->show();
    ui->toolbarFrame->hide();
  }
  pdfViewer->setFocus();
}

void MainWindow::showToolbar()
{
  toolbarVisible = true;
  updateButtons(currentState);
  pdfViewer->setFocus();
}

void MainWindow::hideToolbar()
{
  toolbarVisible = false;
  updateButtons(currentState);
  pdfViewer->setFocus();
}

void MainWindow::openFile()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Document"), NULL, tr("PDF (*.pdf)"));

  if (filename.isEmpty()) return;

  if (loadedPDFFile) {
    loadedPDFFile->clean();
    delete loadedPDFFile;
  }

  loadedPDFFile = new LoadPDFFile(filename, file);
  pdfViewer->setFocus();
}
