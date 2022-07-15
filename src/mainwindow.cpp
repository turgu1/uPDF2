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

#include <QFileDialog>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QThread>
#include <QDebug>
#include <QInputDialog>
#include <QSqlRelationalTableModel>

#include "updf.h"
#include "config.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loadpdffile.h"
#include "selectrecentdialog.h"
#include "preferencesdialog.h"
#include "bookmarksbrowser.h"
#include "bookmarkselector.h"
#include "newbookmarkdialog.h"
#include "documenttab.h"
#include "filescache.h"

// Parameters at startup

u32           details = 0;
QString       filenameAtStartup;
Preferences   preferences;
BookmarksDB * bookmarksDB = nullptr;
FilesCache  * filesCache = nullptr;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    toolbarVisible(true),
    currentDocumentTab(nullptr)
{
  loadConfig();

  ui->setupUi(this);

  if (preferences.bookmarksParameters.bookmarksDbEnabled) {
      bookmarksDB = new BookmarksDB(preferences.bookmarksParameters.bookmarksDbFilename);
  }

  filesCache = new FilesCache;

  ui->trimParametersLabel->setStyleSheet("QLabel#trimParametersLabel { font: 9px }");
  ui->viewer->setStyleSheet("QTabBar::tab { height: 20px; font: 10px }");

  iconFullScreen = new QIcon(":/icons/svg/24x24/maximize-2.svg");
  iconRestore    = new QIcon(":/icons/svg/24x24/minimize-2.svg");

  ui->currentPageEdit->setValidator(new QIntValidator(1, 9999, this));

  ui->viewer->removeTab(0);

  connect(ui->showToolbarButton,   SIGNAL(clicked()),     this,      SLOT(         showToolbar()));
  connect(ui->hideToollbarButton,  SIGNAL(clicked()),     this,      SLOT(         hideToolbar()));
  connect(ui->openFileButton,      SIGNAL(clicked()),     this,      SLOT(            openFile()));
  connect(ui->fullScreenButton,    SIGNAL(clicked()),     this,      SLOT(        onFullScreen()));
  connect(ui->exitButton,          SIGNAL(clicked()),     this,      SLOT(            closeApp()));
  connect(ui->aboutButton,         SIGNAL(clicked()),     this,      SLOT(            aboutBox()));
  connect(ui->openRecentButton,    SIGNAL(clicked()),     this,      SLOT(          openRecent()));
  connect(ui->preferencesButton,   SIGNAL(clicked()),     this,      SLOT(      askPreferences()));

  connect(ui->bookmarksButton,     SIGNAL(clicked()),    this,       SLOT(showBookmarkSelector()));
  connect(ui->addBookmarkButton,   SIGNAL(clicked()),    this,       SLOT(         addBookmark()));

  connect(ui->viewer,              SIGNAL(currentChanged(int)),    this, SLOT(     tabChange(int)));
  connect(ui->viewer,              SIGNAL(tabCloseRequested(int)), this, SLOT(      closeTab(int)));

  connect(filesCache, SIGNAL(busy(bool)), this, SLOT(fileIsLoading(bool)));

  ui->busyLabel->setMovie(new QMovie(":/agif/img/busy.gif"));

  if (preferences.hideControlsAtStartup) hideToolbar();
  if (preferences.fullScreenAtStartup) {
    showFullScreen();
    ui->fullScreenButton->setIcon(*iconRestore);
    ui->fullScreenButton->setToolTip(tr("Normal View"));
  }

  currentState.page           = 0;
  currentState.pageCount      = 0;
  currentState.viewMode       = VM_PAGE;
  currentState.viewZoom       = 0.05f;
  currentState.validDocument  = false;
  currentState.textSelection  = false;
  currentState.someClipText   = false;
  currentState.columnCount    = 1;
  currentState.titlePageCount = 0;

  updateButtons(currentState);
  update();
  updateGeometry();

  if (!filenameAtStartup.isEmpty()) {
    loadFile(filenameAtStartup, QFileInfo(filenameAtStartup).fileName());
    setFileViewParameters(preferences.defaultView, false);
  }
  else {
    if (fileViewParameters) loadRecentFile(*fileViewParameters);
  }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::tabChange(int index)
{
    PDFViewer * pdfViewer;

    disconnect(ui->beginDocumentButton, SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->endDocumentButton,   SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->previousPageButton,  SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->nextPageButton,      SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->zoomInButton,        SIGNAL(clicked()), nullptr, nullptr);
    disconnect(ui->zoomOutButton,       SIGNAL(clicked()), nullptr, nullptr);

    disconnect(ui->editTrimButton,      SIGNAL(clicked(bool)), nullptr, nullptr);
    disconnect(ui->evenOddButton,       SIGNAL(clicked(bool)), nullptr, nullptr);
    disconnect(ui->thisPageButton,      SIGNAL(clicked(bool)), nullptr, nullptr);
    disconnect(ui->clearTrimsButton,    SIGNAL(clicked()),     nullptr, nullptr);
    disconnect(ui->selectTextButton,    SIGNAL(clicked(bool)), nullptr, nullptr);
    disconnect(ui->copyButton,          SIGNAL(clicked()),     nullptr, nullptr);

    disconnect(ui->viewModeCombo,       SIGNAL(currentIndexChanged(int)), nullptr, nullptr);
    disconnect(ui->columnCountCombo,    SIGNAL(currentIndexChanged(int)), nullptr, nullptr);
    disconnect(ui->titlePageCountCombo, SIGNAL(currentIndexChanged(int)), nullptr, nullptr);
    disconnect(ui->currentPageEdit,     SIGNAL(    editingFinished()),    nullptr, nullptr);
    disconnect(ui->trimPageList,        SIGNAL(        itemClicked(QListWidgetItem *)), nullptr, nullptr);

    if (currentDocumentTab != nullptr) {
        pdfViewer = currentDocumentTab->getPdfViewer();
        disconnect(pdfViewer, SIGNAL(stateUpdated(ViewState &)));
    }

    currentDocumentTab = (DocumentTab *) ui->viewer->currentWidget();

    if (currentDocumentTab != nullptr) {
        pdfViewer = currentDocumentTab->getPdfViewer();

        connect(ui->beginDocumentButton, SIGNAL(clicked()),     pdfViewer, SLOT(                  top()));
        connect(ui->endDocumentButton,   SIGNAL(clicked()),     pdfViewer, SLOT(               bottom()));
        connect(ui->previousPageButton,  SIGNAL(clicked()),     pdfViewer, SLOT(               pageUp()));
        connect(ui->nextPageButton,      SIGNAL(clicked()),     pdfViewer, SLOT(             pageDown()));
        connect(ui->zoomInButton,        SIGNAL(clicked()),     pdfViewer, SLOT(               zoomIn()));
        connect(ui->zoomOutButton,       SIGNAL(clicked()),     pdfViewer, SLOT(              zoomOut()));

        connect(ui->editTrimButton,      SIGNAL(clicked(bool)), pdfViewer, SLOT(   trimZoneSelect(bool)));
        connect(ui->evenOddButton,       SIGNAL(clicked(bool)), pdfViewer, SLOT(trimZoneDifferent(bool)));
        connect(ui->thisPageButton,      SIGNAL(clicked(bool)), pdfViewer, SLOT(     thisPageTrim(bool)));
        connect(ui->clearTrimsButton,    SIGNAL(clicked()),     pdfViewer, SLOT(  clearAllSingleTrims()));
        connect(ui->selectTextButton,    SIGNAL(clicked(bool)), pdfViewer, SLOT(       textSelect(bool)));
        connect(ui->copyButton,          SIGNAL(clicked()),     pdfViewer, SLOT(      copyToClipboard()));

        connect(ui->currentPageEdit,
                static_cast<void(QLineEdit::*)()>(&QLineEdit::editingFinished),
                pdfViewer,
                [=](){ currentDocumentTab->getPdfViewer()->gotoPage(ui->currentPageEdit->text().toInt() - 1); });

        connect(ui->trimPageList,
                static_cast<void(QListWidget::*)(QListWidgetItem *)>(&QListWidget::itemClicked),
                pdfViewer,
                [=](QListWidgetItem * item) { currentDocumentTab->getPdfViewer()->gotoPage(item->text().toInt() - 1); });

        connect(ui->viewModeCombo,       SIGNAL(currentIndexChanged(int)), pdfViewer, SLOT(            setViewMode(int)));
        connect(ui->columnCountCombo,    SIGNAL(currentIndexChanged(int)), pdfViewer, SLOT(setColumnCountFromIndex(int)));
        connect(ui->titlePageCountCombo, SIGNAL(currentIndexChanged(int)), pdfViewer, SLOT(      setTitlePageCount(int)));
        connect(pdfViewer,               SIGNAL(stateUpdated(ViewState&)), this,      SLOT(          updateButtons(ViewState&)));

        pdfViewer->sendState();
        currentDocumentTab->setFocus();
    }
}

void MainWindow::closeTab(int index)
{
    DocumentTab * docTab = (DocumentTab *) ui->viewer->widget(index);

    ui->viewer->removeTab(index);

    delete docTab;
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
//    qDebug() << "Key event: " << event->key() << " modifier: " << event->modifiers();

    if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_N)) {
        addBookmark();
    }
    else {
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
    }

    if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::closeEvent()
{
  closeApp();
}

void MainWindow::aboutBox()
{
  QMessageBox aboutBox;

  aboutBox.setIconPixmap(QPixmap(":/icons/img/updf-48x48.png"));
  aboutBox.setText(QString(tr("\nuPDF (micro PDF) Version %1")).arg(APP_VERSION));
  aboutBox.setDetailedText(tr(
       "Written by:\n"
       "\n"
       "(c) 2017, 2020 - Guy Turcotte\n"
       "(c) 2015 - Lauri Kasanen\n"
       "\n"
       "GNU General Public License V3.0"));
  aboutBox.setWindowTitle(tr("About uPDF"));

  aboutBox.setStandardButtons(QMessageBox::Ok);
  aboutBox.setDefaultButton(QMessageBox::Ok);
  aboutBox.setEscapeButton(QMessageBox::Ok);

  aboutBox.exec();

//  bookmarksDB->saveAuthorsList(1, QString("Doe, John & Turcotte, René & Turcotte, Guy"));
//  QString authors = bookmarksDB->getAuthorsList(1);

//  QMessageBox::information(this, tr("Test..."), authors, QMessageBox::Ok, QMessageBox::Ok);

  if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::showBookmarkSelector()
{
    if (bookmarksDB != nullptr) {

        Selection selection;
        BookmarkSelector * bookmarkSelector = new BookmarkSelector(this);

        QString filename = currentDocumentTab == nullptr ? "" : currentDocumentTab->getFilename();

        bool result = bookmarkSelector->select(selection, filename, ui->currentPageEdit->text().toInt());

        // qDebug() << "showBookmarkSelection Result: " << result;

        if (result) {
            // qDebug() << "Selected filename: " << selection.filename;
            // qDebug() << "Selected page nbr: " << selection.pageNbr;

            FileViewParameters params = preferences.defaultView;
            params.yOff = selection.pageNbr - 1;

            QString filename = absoluteFilename(selection.filename);
            if (QFileInfo(filename).exists()) {
//                if (file.filename != filename) {
                    loadFile(filename, selection.caption, selection.pageNbr - 1);
                    setFileViewParameters(params, false);
//                }
//                else {
//                    pdfViewer->gotoPage(selection.pageNbr - 1);
//                }
            }
        }
    }
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
  if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::updateTrimButtons(ViewState & state)
{
  if (ui->editTrimButton->isChecked()) {
    ui->   evenOddButton->show();
    ui->  thisPageButton->show();
    ui->    trimPageList->show();
    ui->clearTrimsButton->show();

    ui->selectTextButton->hide();
    ui->       zoomFrame->hide();

    ui->   evenOddButton->setChecked(!state.trimSimilar);
    ui->  thisPageButton->setChecked(state.thisPageTrim);
    ui->    trimPageList->clear();

    if (currentDocumentTab != nullptr) {
      ui->  trimPageList->addItems(currentDocumentTab->getPdfViewer()->getSinglePageTrims());
    }
  }
  else {
    ui->   evenOddButton->hide();
    ui->  thisPageButton->hide();
    ui->    trimPageList->hide();
    ui->clearTrimsButton->hide();

    ui->selectTextButton->show();
    ui->       zoomFrame->show();
  }

  if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
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
    ui-> pageCountLabel->setText(QString("/ %1").arg(state.pageCount));

    ui->showToolbarFrame->hide();
    ui->    toolbarFrame->show();

    if (ui->viewModeCombo->currentIndex() == VM_CUSTOMTRIM) {
      ui->trimFrame->show();
      updateTrimButtons(state);
    }
    else {
      ui->trimFrame->hide();
      ui->zoomFrame->show();
    }

    ui->selectTextButton->setChecked(state.textSelection);

    if (state.someClipText) {
      ui->copyButton->show();
    }
    else {
      ui->copyButton->hide();
    }
    ui->   columnCountCombo->setCurrentIndex(state.columnCount - 1);
    ui->titlePageCountCombo->setCurrentIndex(state.titlePageCount);

    ui->   columnCountCombo->setEnabled(state.validDocument);
    ui->titlePageCountCombo->setEnabled(state.validDocument);
    ui->   selectTextButton->setEnabled(state.validDocument);
    ui->       zoomInButton->setEnabled(state.validDocument);
    ui->      zoomOutButton->setEnabled(state.validDocument);
    ui->      viewModeCombo->setEnabled(state.validDocument);
    ui->    currentPageEdit->setEnabled(state.validDocument);
    ui->beginDocumentButton->setEnabled(state.validDocument);
    ui->  endDocumentButton->setEnabled(state.validDocument);
    ui-> previousPageButton->setEnabled(state.validDocument);
    ui->     nextPageButton->setEnabled(state.validDocument);

    ui->  addBookmarkButton->setEnabled(preferences.bookmarksParameters.bookmarksDbEnabled);
    ui->    bookmarksButton->setEnabled(preferences.bookmarksParameters.bookmarksDbEnabled);

    ui->   openRecentButton->setEnabled(fileViewParameters != NULL);
  }
  else {
    ui->showToolbarFrame->show();
    ui->    toolbarFrame->hide();
  }

  if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::showToolbar()
{
    toolbarVisible = true;
    updateButtons(currentState);
    if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::hideToolbar()
{
    toolbarVisible = false;
    updateButtons(currentState);
    if (currentDocumentTab != nullptr) currentDocumentTab->setFocus();
}

void MainWindow::loadFile(QString filename, QString title, int atPage)
{
    // qDebug() << "Loading file " << filename << " at Page " << atPage;

    currentDocumentTab = new DocumentTab(nullptr);
    currentDocumentTab->loadFile(filename, atPage);

    int index = ui->viewer->addTab(currentDocumentTab, title);
    ui->viewer->setCurrentIndex(index);

    currentDocumentTab->setFocus();
}

void MainWindow::setFileViewParameters(FileViewParameters & params, bool recent)
{
  if (!recent || preferences.recentGeometry) {
    if (currentDocumentTab != nullptr) {
        currentDocumentTab->getPdfViewer()->setFileViewParameters(params);
    }
  }
  update();
//  updateGeometry();
}

void MainWindow::loadRecentFile(FileViewParameters & params)
{
  QFileInfo fi(params.filename);
  loadFile(params.filename, fi.fileName(), params.yOff);

  setFileViewParameters(params, true);
}

void MainWindow::saveFileParameters()
{
  if (currentDocumentTab && preferences.keepRecent) {
    FileViewParameters params;

    params.customTrim.singles = NULL;
    params.winGeometry        = geometry();

    currentDocumentTab->getPdfViewer()->getFileViewParameters(params);
    saveToConfig(params);
  }
}

void MainWindow::openFile()
{
  saveFileParameters();

  QString filename = QFileDialog::getOpenFileName(this, tr("Open Document"), NULL, tr("PDF (*.pdf);;All Files (*)"));

  if (filename.isEmpty()) return;

  loadFile(filename, QFileInfo(filename).fileName());

  setFileViewParameters(preferences.defaultView, false);
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

  if (params) {
    saveFileParameters();
    loadRecentFile(*params);
  }
}

void MainWindow::askPreferences()
{
  PreferencesDialog * preferencesDialog = new PreferencesDialog(this);

  FileViewParameters currentView;

  currentView.customTrim.initialized = false;
  currentView.customTrim.singles     = NULL;
  currentView.winGeometry            = geometry();

  if (currentDocumentTab != nullptr) {
    currentDocumentTab->getPdfViewer()->getFileViewParameters(currentView);
  }
  preferencesDialog->run(currentView);

  updateButtons(currentState);

  if (preferences.bookmarksParameters.bookmarksDbEnabled) {
    if (bookmarksDB == nullptr) {
        bookmarksDB = new BookmarksDB(preferences.bookmarksParameters.bookmarksDbFilename);
    }
  }
  else if (bookmarksDB) {
      delete bookmarksDB;
      bookmarksDB = nullptr;
  }

}

void MainWindow::addBookmark()
{
    if ((bookmarksDB != nullptr) && (currentDocumentTab != nullptr)) {

        int page = ui->currentPageEdit->text().toInt();

        NewBookmarkDialog * dialog = new NewBookmarkDialog(nullptr);

        dialog->run(currentDocumentTab->getFilename(), page);

        delete dialog;
        currentDocumentTab->setFocus();
    }
}

void MainWindow::fileIsLoading(bool isLoading)
{
    if (isLoading) {
        ui->metricsLabel->hide();
        ui->busyLabel->movie()->start();
        ui->busyLabel->show();
    }
    else {
        ui->busyLabel->movie()->stop();
        ui->busyLabel->hide();
    }
//      if (preferences.showLoadMetrics) {
//        ui->metricsLabel->setText(state.metrics);
//        ui->metricsLabel->show();
//      }
}
