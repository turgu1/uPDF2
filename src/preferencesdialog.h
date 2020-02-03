/*
Copyright (C) 2017, 2020 Guy Turcotte

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
  void selectBookmarksDbFilename();
  void selectPdfFolderPrefix();
  void setDefaultView();
  void showBookmarksBrowser();
  void bookmarksDbEnabled();
};

#endif // PREFERENCESDIALOG_H
