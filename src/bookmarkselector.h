/*
Copyright (C) 2020 Guy Turcotte

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

#ifndef BOOKMARKSELECTOR_H
#define BOOKMARKSELECTOR_H

#include <QDialog>

#include "updf.h"

class QSqlTableModel;

namespace Ui {
class BookmarkSelector;
}

struct Selection {
    QString filename;
    QString caption;
    int pageNbr;
};

class BookmarkSelector : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkSelector(QWidget * parent = nullptr);
    ~BookmarkSelector();

    bool select(Selection & selection, const QString & currentFilename, int page);

private slots:
    void documentSelect(const QModelIndex & index);
    void    entrySelect(const QModelIndex & index);
    void changeDocument(const QModelIndex & index);
    void    changeEntry(const QModelIndex & index);

    void changeDocumentsFilter();
    void   changeEntriesFilter();
    void           selectEntry();

private:
    Ui::BookmarkSelector * ui;
    Selection            * sel;
    QSqlTableModel       * documentsModel;
    QSqlTableModel       * entriesModel;
    QImage                 entryImage;

    void saveEntryThumbnail(const QModelIndex & index);

protected:
    void paintEvent(QPaintEvent * event)  Q_DECL_OVERRIDE;
};

#endif // BOOKMARKSELECTOR_H
