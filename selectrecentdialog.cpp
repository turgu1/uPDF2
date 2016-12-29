/*
Copyright (C) 2017 Guy Turcotte

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

#include "selectrecentdialog.h"
#include "ui_selectrecentdialog.h"

#include <QFileInfo>
#include <QMessageBox>

#include "pdfviewer.h"

SelectRecentDialog::SelectRecentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SelectRecentDialog)
{
  ui->setupUi(this);

  connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearRecentsList()));
}

SelectRecentDialog::~SelectRecentDialog()
{
  delete ui;
}

void SelectRecentDialog::setContent()
{
  FileViewParameters * curr = fileViewParameters;

  ui->recentFileList->clear();
  while (curr) {
    QFileInfo fi(curr->filename);

    ui->recentFileList->addItem(fi.fileName());
    curr = curr->next;
  }
}

FileViewParameters * SelectRecentDialog::run()
{
  if (exec() == QDialog::Accepted) {
    int row = ui->recentFileList->currentRow();
    FileViewParameters * curr = fileViewParameters;
    while (curr && row--) curr = curr->next;
    return curr;
  }
  else {
    return NULL;
  }
}

void SelectRecentDialog::clearRecentsList()
{
  QMessageBox msgBox;
  msgBox.setText("Are you sure you want to\ndelete the Recent File List content?");
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Cancel);


  if (msgBox.exec() == QMessageBox::Ok) {
    clearFileViewParameters();
    reject();
  }
}
