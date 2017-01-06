#include "preferencesdialog.h"
#include "ui_preferencesdialog.h"

#include <QMessageBox>
#include <QFileDialog>

#include "updf.h"
#include "config.h"

PreferencesDialog::PreferencesDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::PreferencesDialog)
{
  ui->setupUi(this);

  this->setStyleSheet("QLabel#preferencesTitle { font-size: 15px; font-weight: bold }");

  connect(ui->clearRecentsButton, SIGNAL(clicked()), this, SLOT(clearRecentsList()));
  connect(ui->logFileButton,      SIGNAL(clicked()), this, SLOT(   selectLogFile()));
}

PreferencesDialog::~PreferencesDialog()
{
  delete ui;
}

void PreferencesDialog::setContent()
{
  ui->  fullScreenCB->setChecked(preferences.fullScreenAtStartup   );
  ui->hideControlsCB->setChecked(preferences.hideControlsAtStartup );
  ui->   clipboardCB->setChecked(preferences.viewClipboardSelection);
  ui->      recentCB->setChecked(preferences.keepRecent            );
  ui->    geometryCB->setChecked(preferences.recentGeometry        );
  ui->     metricsCB->setChecked(preferences.showLoadMetrics       );
  ui->         logCB->setChecked(preferences.logTrace              );
  ui->   logFileEdit->   setText(preferences.logFilename           );
}

void PreferencesDialog::saveContent()
{
  preferences.fullScreenAtStartup    = ui->  fullScreenCB->isChecked();
  preferences.hideControlsAtStartup  = ui->hideControlsCB->isChecked();
  preferences.viewClipboardSelection = ui->   clipboardCB->isChecked();
  preferences.keepRecent             = ui->      recentCB->isChecked();
  preferences.recentGeometry         = ui->    geometryCB->isChecked();
  preferences.showLoadMetrics        = ui->     metricsCB->isChecked();
  preferences.logTrace               = ui->         logCB->isChecked();
  preferences.logFilename            = ui->   logFileEdit->text();
}

void PreferencesDialog::run()
{
  setContent();
  if (exec() == QDialog::Accepted) {
    saveContent();
  }
}

void PreferencesDialog::clearRecentsList()
{
  QMessageBox msgBox;
  msgBox.setText(tr("Are you sure you want to\ndelete the Recent File List content?"));
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);

  if (msgBox.exec() == QMessageBox::Ok) {
    clearFileViewParameters();
  }
}

void PreferencesDialog::selectLogFile()
{
  QString fname = ui->logFileEdit->text();
  QString filename = QFileDialog::getSaveFileName(this, tr("Select Log File"), fname, tr("LOG (*.log);;All Files (*)"));

  if (filename.isEmpty()) return;

  ui->logFileEdit->setText(filename);
}
