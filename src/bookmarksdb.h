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

#ifndef BOOKMARKSDB_H
#define BOOKMARKSDB_H

#include <QSqlDatabase>
#include <QDebug>

#include "updf.h"
#include "documentmodel.h"

class QSqlTableModel;
class QString;
class DocumentModel;

enum {
    Document_Id,
    Document_Name,
    Document_Filename,
    Document_Thumbnail
};

enum {
    Entry_Id,
    Entry_Document_Id,
    Entry_Caption,
    Entry_Page_Nbr,
    Entry_Thumbnail
};

class BookmarksDB
{
private:
    QSqlDatabase     db;
    QSqlTableModel * entriesDBModel;
    DocumentModel  * documentsDBModel;

    QString check(QString str) { qDebug() << str; return str; }
public:
    BookmarksDB(QString dbFile);
    ~BookmarksDB();

    bool                           createDB();
    inline QSqlDatabase &             getDB() { return db; }
    QSqlTableModel      &   getEntriesModel() { return * entriesDBModel; }
    DocumentModel       & getDocumentsModel() { return * documentsDBModel; }
    bool                           addEntry(QString filename,
                                            QString caption,
                                            QString authors,                                             int pageNbr,
                                            QImage & image);
    bool                     readCSVEntries(QString & filename, int documentId);
    void                 saveEntryThumbnail(const QModelIndex & index, QImage & image);
    QString                  getAuthorsList(int entryId);
    bool                    saveAuthorsList(int entryId, const QString & list);
};

#endif // BOOKMARKSDB_H
