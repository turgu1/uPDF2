#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QMessageBox>
#include <QFileDialog>

#include "bookmarksbrowser.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PreferencesDialog)
{
  ui->setupUi(this);

  this->setStyleSheet("QLabel#preferencesTitle { font-size: 15px; font-weight: bold }");

  connect(ui->clearRecentsButton,        SIGNAL(clicked()), this, SLOT(         clearRecentsList()));
  connect(ui->logFileButton,             SIGNAL(clicked()), this, SLOT(            selectLogFile()));
  connect(ui->setViewButton,             SIGNAL(clicked()), this, SLOT(           setDefaultView()));
  connect(ui->bookmarksDbFilenameButton, SIGNAL(clicked()), this, SLOT(selectBookmarksDbFilename()));
  connect(ui->pdfFolderPrefixButton,     SIGNAL(clicked()), this, SLOT(    selectPdfFolderPrefix()));

  connect(ui->bookmarksDbEnabledCB,      SIGNAL(clicked()), this, SLOT(       bookmarksDbEnabled()));
  connect(ui->bookmarksBrowserButton,    SIGNAL(clicked()), this, SLOT(     showBookmarksBrowser()));
}

PreferencesDialog::~PreferencesDialog()
{
  delete ui;
}

void PreferencesDialog::setContent()
{
  ui->     fullScreenCB->setChecked(preferences.fullScreenAtStartup   );
  ui->   hideControlsCB->setChecked(preferences.hideControlsAtStartup );
  ui->      clipboardCB->setChecked(preferences.viewClipboardSelection);
  ui->         recentCB->setChecked(preferences.keepRecent            );
  ui->       geometryCB->setChecked(preferences.recentGeometry        );
  ui->        metricsCB->setChecked(preferences.showLoadMetrics       );
  ui->            logCB->setChecked(preferences.logTrace              );
  ui->      logFileEdit->   setText(preferences.logFilename           );
  ui->horizontalPadding->  setValue(preferences.horizontalPadding     );
  ui->  verticalPadding->  setValue(preferences.verticalPadding       );
  ui-> doubleClickSpeed->  setValue(preferences.doubleClickSpeed      );

  ui->   bookmarksDbEnabledCB->setChecked(preferences.bookmarksParameters.bookmarksDbEnabled);
  ui->bookmarksDbFilenameEdit->   setText(preferences.bookmarksParameters.bookmarksDbFilename);
  ui->    pdfFolderPrefixEdit->   setText(preferences.bookmarksParameters.pdfFolderPrefix);

  ui->bookmarksBrowserButton->setEnabled(ui->bookmarksDbEnabledCB->isChecked());

  defaultView = preferences.defaultView;
}

void PreferencesDialog::bookmarksDbEnabled()
{
   ui->bookmarksBrowserButton->setEnabled(ui->bookmarksDbEnabledCB->isChecked());
}

void PreferencesDialog::saveContent()
{
  preferences.fullScreenAtStartup     = ui->     fullScreenCB->isChecked();
  preferences.hideControlsAtStartup   = ui->   hideControlsCB->isChecked();
  preferences.viewClipboardSelection  = ui->      clipboardCB->isChecked();
  preferences.keepRecent              = ui->         recentCB->isChecked();
  preferences.recentGeometry          = ui->       geometryCB->isChecked();
  preferences.showLoadMetrics         = ui->        metricsCB->isChecked();
  preferences.logTrace                = ui->            logCB->isChecked();
  preferences.logFilename             = ui->      logFileEdit->text();
  preferences.horizontalPadding       = ui->horizontalPadding->value();
  preferences.verticalPadding         = ui->  verticalPadding->value();
  preferences.doubleClickSpeed        = ui-> doubleClickSpeed->value();

  preferences.bookmarksParameters.bookmarksDbEnabled  = ui->   bookmarksDbEnabledCB->isChecked();
  preferences.bookmarksParameters.bookmarksDbFilename = ui->bookmarksDbFilenameEdit->text();
  preferences.bookmarksParameters.pdfFolderPrefix     = ui->    pdfFolderPrefixEdit->text();

  preferences.defaultView              = defaultView;
}

void PreferencesDialog::run(FileViewParameters & current)
{
  currentView = current;
  setContent();
  if (exec() == QDialog::Accepted) {
    saveContent();
  }
}

void PreferencesDialog::clearRecentsList()
{
  QMessageBox msgBox;
  msgBox.setText(tr("Are you sure you want to\n"
                    "delete the Recent File List content?"));
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);

  if (msgBox.exec() == QMessageBox::Ok) {
    clearFileViewParameters();
  }
}

void PreferencesDialog::selectLogFile()
{
  QString fname = ui->logFileEdit->text();
  QString filename = QFileDialog::getSaveFileName(
              this,
              tr("Select Log File"),
              fname,
              tr("LOG (*.log);;All Files (*)"));

  if (filename.isEmpty()) return;

  ui->logFileEdit->setText(filename);
}

void PreferencesDialog::selectBookmarksDbFilename()
{
  QString fname = ui->bookmarksDbFilenameEdit->text();
  QString filename = QFileDialog::getOpenFileName(
              this,
              tr("Select Bookmarks DB File"),
              fname,
              tr("DB (*.db);;All Files (*)"));

  if (filename.isEmpty()) return;

  ui->bookmarksDbFilenameEdit->setText(filename);
}

void PreferencesDialog::selectPdfFolderPrefix()
{
  QString fname = ui->pdfFolderPrefixEdit->text();
  QString foldername = QFileDialog::getExistingDirectory(
              this,
              tr("Select PDF Folder Prefix"),
              fname,
              QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (foldername.isEmpty()) return;

  ui->pdfFolderPrefixEdit->setText(foldername);
}

void PreferencesDialog::setDefaultView()
{
  defaultView = currentView;
}

void PreferencesDialog::showBookmarksBrowser()
{
    if ((bookmarksDB != nullptr) && ui->bookmarksDbEnabledCB->isChecked()) {
        BookmarksBrowser * bookmarksBrowser = new BookmarksBrowser(this);

        bookmarksBrowser->exec();
    }
}
