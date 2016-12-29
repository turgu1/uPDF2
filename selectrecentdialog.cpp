#include "selectrecentdialog.h"
#include "ui_selectrecentdialog.h"

#include <QFileInfo>

SelectRecentDialog::SelectRecentDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SelectRecentDialog)
{
  ui->setupUi(this);
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
