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

#include <QFileDialog>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QThread>

#include "updf.h"
#include "config.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loadpdffile.h"
#include "selectrecentdialog.h"

// Parameters at startup

u32     details = 0;
QString filenameAtStartup;


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

  connect(ui->showToolbarButton,   SIGNAL(clicked()),     this,      SLOT(showToolbar()) );
  connect(ui->hideToollbarButton,  SIGNAL(clicked()),     this,      SLOT(hideToolbar()) );
  connect(ui->openFileButton,      SIGNAL(clicked()),     this,      SLOT(openFile())    );
  connect(ui->fullScreenButton,    SIGNAL(clicked()),     this,      SLOT(onFullScreen()));
  connect(ui->exitButton,          SIGNAL(clicked()),     this,      SLOT(closeApp())    );
  connect(ui->aboutButton,         SIGNAL(clicked()),     this,      SLOT(aboutBox())    );
  connect(ui->openRecentButton,    SIGNAL(clicked()),     this,      SLOT(openRecent())  );

  connect(ui->beginDocumentButton, SIGNAL(clicked()),     pdfViewer, SLOT(top())         );
  connect(ui->endDocumentButton,   SIGNAL(clicked()),     pdfViewer, SLOT(bottom())      );
  connect(ui->previousPageButton,  SIGNAL(clicked()),     pdfViewer, SLOT(pageUp())      );
  connect(ui->nextPageButton,      SIGNAL(clicked()),     pdfViewer, SLOT(pageDown())    );
  connect(ui->zoomInButton,        SIGNAL(clicked()),     pdfViewer, SLOT(zoomIn())      );
  connect(ui->zoomOutButton,       SIGNAL(clicked()),     pdfViewer, SLOT(zoomOut())     );

  connect(ui->editTrimButton,      SIGNAL(clicked(bool)), pdfViewer, SLOT(trimZoneSelect(bool))   );
  connect(ui->evenOddButton,       SIGNAL(clicked(bool)), pdfViewer, SLOT(trimZoneDifferent(bool)));
  connect(ui->thisPageButton,      SIGNAL(clicked(bool)), pdfViewer, SLOT(thisPageTrim(bool))     );
  connect(ui->clearTrimsButton,    SIGNAL(clicked()),     pdfViewer, SLOT(clearAllSingleTrims())  );
  connect(ui->selectTextButton,    SIGNAL(clicked(bool)), pdfViewer, SLOT(textSelect(bool))       );
  connect(ui->copyButton,          SIGNAL(clicked()),     pdfViewer, SLOT(copyToClipboard())      );

  connect(ui->viewModeCombo,       static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setViewMode(ViewMode(index)); });

  connect(ui->columnCountCombo,    static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setColumnCount(index + 1); });

  connect(ui->titlePageCountCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
          [=](int index) { pdfViewer->setTitlePageCount(index); });

  connect(ui->currentPageEdit,     static_cast<void(QLineEdit::*)()>(&QLineEdit::editingFinished),
          [=](){ pdfViewer->gotoPage(ui->currentPageEdit->text().toInt() - 1); });

  connect(ui->trimPageList,        static_cast<void(QListWidget::*)(QListWidgetItem *)>(&QListWidget::itemClicked),
          [=](QListWidgetItem * item) { pdfViewer->gotoPage(item->text().toInt() - 1); });

  connect(pdfViewer,               static_cast<void(PDFViewer::*)(ViewState &)>(&PDFViewer::stateUpdated),
          [=](ViewState & state) { this->updateButtons(state); });

  ui->busyLabel->setMovie(new QMovie(":/agif/img/busy.gif"));

  currentState.page          = 0;
  currentState.pageCount     = 0;
  currentState.viewMode      = VM_PAGE;
  currentState.viewZoom      = 0.05f;
  currentState.fileLoading   = false;
  currentState.validDocument = false;

  updateButtons(currentState);
  update();
  updateGeometry();

  loadConfig();
  if (!filenameAtStartup.isEmpty()) {
    loadFile(filenameAtStartup);
  }
  else {
    if (fileViewParameters) loadRecentFile(*fileViewParameters);
  }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
  switch (event->key()) {
    case Qt::Key_F8:
      if (toolbarVisible) hideToolbar(); else showToolbar();
      break;

    case Qt::Key_Escape:
      closeApp();
      break;

    default:
      QWidget::keyPressEvent(event);
  }

  pdfViewer->setFocus();
}

void MainWindow::closeEvent()
{
  closeApp();
}

void MainWindow::aboutBox()
{
  QMessageBox aboutBox;

  aboutBox.setIconPixmap(QPixmap(":/icons/48/img/48x48/updf.png"));
  aboutBox.setText(QString(tr("\nuPDF (micro PDF) Version %1")).arg(UPDF_VERSION));
  aboutBox.setDetailedText(tr("Written by:\n\n(c) 2017 - Guy Turcotte\n(c) 2015 - Lauri Kasanen\n\nGNU General Public License V3.0"));
  aboutBox.setWindowTitle(tr("About uPDF"));

  aboutBox.setStandardButtons(QMessageBox::Ok);
  aboutBox.setDefaultButton(QMessageBox::Ok);
  aboutBox.setEscapeButton(QMessageBox::Ok);

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

void MainWindow::updateTrimButtons(ViewState & state)
{
  if (ui->editTrimButton->isChecked()) {
    ui->evenOddButton->show();
    ui->thisPageButton->show();
    ui->trimPageList->show();
    ui->clearTrimsButton->show();

    ui->selectTextButton->hide();
    ui->zoomFrame->hide();

    ui->evenOddButton->setChecked(!state.trimSimilar);
    ui->thisPageButton->setChecked(state.thisPageTrim);
    ui->trimPageList->clear();
    ui->trimPageList->addItems(pdfViewer->getSinglePageTrims());
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
      updateTrimButtons(state);
    }
    else {
      ui->trimFrame->hide();
      ui->zoomFrame->show();
    }

    ui->selectTextButton->setChecked(state.textSelection);

    if (state.fileLoading) {
      ui->busyLabel->movie()->start();
      ui->busyLabel->show();
    }
    else {
      ui->busyLabel->movie()->stop();
      ui->busyLabel->hide();
    }

    if (state.someClipText) {
      ui->copyButton->show();
    }
    else {
      ui->copyButton->hide();
    }
    ui->columnCountCombo->setCurrentIndex(state.columnCount - 1);
    ui->titlePageCountCombo->setCurrentIndex(state.titlePageCount);

    ui->columnCountCombo->setEnabled(state.validDocument);
    ui->titlePageCountCombo->setEnabled(state.validDocument);
    ui->selectTextButton->setEnabled(state.validDocument);
    ui->zoomInButton->setEnabled(state.validDocument);
    ui->zoomOutButton->setEnabled(state.validDocument);
    ui->viewModeCombo->setEnabled(state.validDocument);
    ui->currentPageEdit->setEnabled(state.validDocument);
    ui->beginDocumentButton->setEnabled(state.validDocument);
    ui->endDocumentButton->setEnabled(state.validDocument);
    ui->previousPageButton->setEnabled(state.validDocument);
    ui->nextPageButton->setEnabled(state.validDocument);

    ui->openRecentButton->setEnabled(fileViewParameters != NULL);
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

void MainWindow::loadFile(QString filename)
{
  if (loadedPDFFile) {
    loadedPDFFile->clean();
    delete loadedPDFFile;
  }

  pdfViewer->reset();
  loadedPDFFile = new LoadPDFFile(filename, file);

  QFileInfo fi(filename);
  setWindowTitle(QString("%1 - uPDF").arg(fi.fileName()));

  pdfViewer->setFocus();
}

void MainWindow::loadRecentFile(FileViewParameters & params)
{
  loadFile(params.filename);

  if (params.fullscreen) {
    if (!isFullScreen()) {
      ui->fullScreenButton->setIcon(*iconRestore);
      ui->fullScreenButton->setToolTip(tr("Normal View"));
      showFullScreen();
    }
  }
  else {
    if (isFullScreen()) {
      ui->fullScreenButton->setIcon(*iconFullScreen);
      ui->fullScreenButton->setToolTip(tr("Full Screen View"));
      showNormal();
      QApplication::processEvents();
    }
    setGeometry(params.winGeometry);
  }

  pdfViewer->setFileViewParameters(params);
  update();
  updateGeometry();
}

void MainWindow::saveFileParameters()
{
  if (file.isValid()) {
    FileViewParameters params;

    params.customTrim.singles = NULL;
    params.winGeometry = geometry();
    params.fullscreen  = isFullScreen();

    pdfViewer->getFileViewParameters(params);
    saveToConfig(params);
  }
}

void MainWindow::openFile()
{
  saveFileParameters();

  QString filename = QFileDialog::getOpenFileName(this, tr("Open Document"), NULL, tr("PDF (*.pdf)"));

  if (filename.isEmpty()) return;

  loadFile(filename);
}

void MainWindow::closeApp()
{
  saveFileParameters();
  saveConfig();
  close();
}

void MainWindow::openRecent()
{
  SelectRecentDialog * recentDialog = new SelectRecentDialog(this);

  recentDialog->setContent();
  FileViewParameters * params = recentDialog->run();

  ui->openRecentButton->setEnabled(fileViewParameters != NULL);

  if (params) loadRecentFile(*params);
}
