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

#ifndef NEWBOOKMARKDIALOG_H
#define NEWBOOKMARKDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class NewBookmarkDialog;
}

class NewBookmarkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewBookmarkDialog(QWidget * parent = nullptr);
    ~NewBookmarkDialog();

    bool run(QString filename, int page);

private slots:
    void save();

private:
    Ui::NewBookmarkDialog * ui;
    QString                 filename;
    int                     page;
};

#endif // NEWBOOKMARKDIALOG_H
