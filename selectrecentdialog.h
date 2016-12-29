#ifndef SELECTRECENTDIALOG_H
#define SELECTRECENTDIALOG_H

#include <QDialog>

#include "config.h"

namespace Ui {
  class SelectRecentDialog;
}

class SelectRecentDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit SelectRecentDialog(QWidget *parent = 0);
    ~SelectRecentDialog();
    void setContent();
    FileViewParameters * run();

  private:
    Ui::SelectRecentDialog *ui;
};

#endif // SELECTRECENTDIALOG_H
