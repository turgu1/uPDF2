#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

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
  void run();

private:
  Ui::PreferencesDialog *ui;

private slots:
  void clearRecentsList();
  void selectLogFile();
};

#endif // PREFERENCESDIALOG_H
