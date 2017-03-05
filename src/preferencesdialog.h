#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

#include "updf.h"
#include "config.h"

namespace Ui {
  class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit PreferencesDialog(QWidget *parent = 0);
  ~PreferencesDialog();
  void setContent();
  void saveContent();
  void run(FileViewParameters & current);

private:
  Ui::PreferencesDialog *ui;
  FileViewParameters defaultView, currentView;

private slots:
  void clearRecentsList();
  void selectLogFile();
  void setDefaultView();
};

#endif // PREFERENCESDIALOG_H
