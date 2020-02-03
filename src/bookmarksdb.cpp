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

#include "bookmarksdb.h"

#include <QtGui>
#include <QtSql>
#include <QMessageBox>

const QString DRIVER("QSQLITE");

BookmarksDB::BookmarksDB(QString dbFile)
{
  if (QSqlDatabase::isDriverAvailable(DRIVER)) {
      db = QSqlDatabase::addDatabase(DRIVER);
      db.setDatabaseName(dbFile);

      if (!db.open()) {
          QMessageBox::critical(
              nullptr,
              QObject::tr("Cannot open/create database"),
              QObject::tr("Unable to establish a database connection.\n"
                          "Database error: %1\n\n"
                          "Click Cancel to exit.").arg(db.lastError().text()),
              QMessageBox::Cancel);
      }
      else {
          QSqlQuery query(db);
          query.exec("PRAGMA foreign_keys = ON;");
          if (!query.isActive()) {
              QMessageBox::critical(
                  nullptr,
                  QObject::tr("Cannot turn on foreign keys"),
                  QObject::tr("Unable to allow foreign keys functionality.\n"
                              "Database error: %1\n\n"
                              "Click Cancel to exit.").arg(db.lastError().text()),
                  QMessageBox::Cancel);
          }
          else {
              if (!createDB()) {
                  QMessageBox::critical(
                      nullptr,
                      QObject::tr("Cannot complete database creation"),
                      QObject::tr("Unable to complete the database creation properly.\n"
                                  "Please correct the problem before retrying.\n\n"
                                  "Click Cancel to exit."),
                      QMessageBox::Cancel);
              }
              else {
                  entriesDBModel = new QSqlTableModel(nullptr, db);
                  entriesDBModel->setTable("entries");
                  //entriesDBModel->setRelation(1, QSqlRelation("documents", "id", "name"));
                  entriesDBModel->select();
                  entriesDBModel->setHeaderData(Entry_Id,           Qt::Horizontal, "Id");
                  entriesDBModel->setHeaderData(Entry_Document_Id,  Qt::Horizontal, "Document");
                  entriesDBModel->setHeaderData(Entry_Caption,      Qt::Horizontal, "Caption");
                  entriesDBModel->setHeaderData(Entry_Page_Nbr,     Qt::Horizontal, "Page #");
                  entriesDBModel->setHeaderData(Entry_Thumbnail,    Qt::Horizontal, "Thumbnail");

                  documentsDBModel = new DocumentModel(nullptr, db);
                  documentsDBModel->setTable("documents");
                  documentsDBModel->setHeaderData(Document_Id,        Qt::Horizontal, "Id");
                  documentsDBModel->setHeaderData(Document_Name,      Qt::Horizontal, "Name");
                  documentsDBModel->setHeaderData(Document_Filename,  Qt::Horizontal, "Filename");
                  documentsDBModel->setHeaderData(Document_Thumbnail, Qt::Horizontal, "Thumbnail");
              }
          }
      }

  }
}

BookmarksDB::~BookmarksDB()
{
  QString name = db.databaseName();

  db.close();
  QSqlDatabase::removeDatabase(name);
}

bool BookmarksDB::createDB()
{
    QSqlQuery query(db);

    query.exec("CREATE TABLE IF NOT EXISTS documents ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "name VARCHAR(50), "
                 "filename VARCHAR(200), "
                 "thumbnail BLOB"
               ");");
    if (!query.isActive()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Cannot create table DOCUMENTS"),
            QObject::tr("Unable to create database table Document.\n"
                        "Database error: %1\n\n"
                        "Click Cancel to exit.").arg(db.lastError().text()),
            QMessageBox::Cancel);

        return false;
    }

    query.exec("CREATE TABLE IF NOT EXISTS entries ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "document_id INTEGER, "
                 "caption VARCHAR(50), "
                 "page_nbr INTEGER, "
                 "thumbnail BLOB, "
                 "FOREIGN KEY(document_id) REFERENCES documents(id) "
                   "ON DELETE CASCADE ON UPDATE CASCADE"
               ");");
    if (!query.isActive()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Cannot create table ENTRIES"),
            QObject::tr("Unable to create database table Entries.\n"
                        "Database error: %1\n\n"
                        "Click Cancel to exit.").arg(query.lastError().text()),
            QMessageBox::Cancel);

        return false;
    }

    query.exec("CREATE TABLE IF NOT EXISTS authors ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "first_name VARCHAR(20), "
                 "last_name VARCHAR(20)"
               ");");
    if (!query.isActive()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Cannot create table AUTHORS"),
            QObject::tr("Unable to create database table Authors.\n"
                        "Database error: %1\n\n"
                        "Click Cancel to exit.").arg(db.lastError().text()),
            QMessageBox::Cancel);

        return false;
    }

    query.exec("CREATE TABLE IF NOT EXISTS author_entry ("
                 "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                 "author_id INTEGER, "
                 "entry_id INTEGER, "
                 "FOREIGN KEY(author_id) REFERENCES authors(id) "
                   "ON DELETE CASCADE ON UPDATE CASCADE, "
                 "FOREIGN KEY(entry_id) REFERENCES entries(id) "
                   "ON DELETE CASCADE ON UPDATE CASCADE"
               ");");
    if (!query.isActive()) {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Cannot create table AUTHOR_ENTRY"),
            QObject::tr("Unable to create database table Author_Entry.\n"
                        "Database error: %1\n\n"
                        "Click Cancel to exit.").arg(db.lastError().text()),
            QMessageBox::Cancel);

        return false;
    }

    return true;
}

bool BookmarksDB::addEntry(QString filename,
                           QString caption,
                           QString authors,
                           int pageNbr,
                           QImage & image)
{
    QSqlTableModel * entriesModel = &getEntriesModel();
    QSqlTableModel * documentsModel = &getDocumentsModel();
    QString completeFilename = filename;

    int documentId;

    db.transaction();

    while (true) {
        documentsModel->setFilter("filename = \'" + relativeFilename(filename) + "\'");
        documentsModel->select();

        if (!documentsModel->index(0, Document_Id).isValid()) {
             QSqlRecord rec = documentsModel->record();
             rec.setValue(Document_Name,      extractFilename(filename));
             rec.setValue(Document_Filename, relativeFilename(filename));

             QImage thumbnail;
             if (getPageImage(completeFilename, thumbnail)) {
                 QBuffer buf;
                 buf.open(QIODevice::WriteOnly);
                 thumbnail.save(&buf, "PNG");
                 rec.setValue(Document_Thumbnail, buf.data());
             }
             else {
                 rec.setNull(Document_Thumbnail);
             }
             documentsModel->insertRecord(0, rec);
             if (!documentsModel->submitAll()) {
                 qDebug() << "Unable to insert new document record: " << db.lastError().text();
                 break;
             }
             documentId = documentsModel->query().lastInsertId().toInt();
             qDebug() << "Last document insert id: " << documentId;
        }
        else {
            documentId = documentsModel->index(0, Document_Id).data().toInt();
        }

        QSqlRecord rec = entriesModel->record();
        rec.setValue(Entry_Caption,     caption);
        rec.setValue(Entry_Page_Nbr,    pageNbr);

        rec.setValue(Entry_Document_Id, documentId);

        QBuffer buf;
        buf.open(QIODevice::WriteOnly);
        if (!image.isNull() && image.save(&buf, "PNG")) {
            rec.setValue(Entry_Thumbnail, buf.data());
        }

        entriesModel->insertRecord(-1, rec);
        if (!entriesModel->submitAll()) {
            qDebug() << "Unable to insert new index entry record: " << db.lastError().text();
            break;
        }

        if (saveAuthorsList(entriesModel->query().lastInsertId().toInt(), authors)) {
            db.commit();
            return true;
        }
    }
    db.rollback();
    return false;
}

bool BookmarksDB::readCSVEntries(QString & filename, int documentId)
{
    QFile file(filename);
    QSqlTableModel * entriesModel = &getEntriesModel();

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << file.errorString();
        return false;
    }

    while (!file.atEnd()) {
        QList<QByteArray> wordList;
        QByteArray line = file.readLine();
        wordList = line.split(',');

        if (wordList.size() != 3) {
            qDebug() << "CSV Format error.";
            file.close();
            return false;
        }

        qDebug() << "Title: " << QString(wordList[0]).trimmed();

        QSqlRecord rec = entriesModel->record();
        rec.setValue(Entry_Caption,     QString(wordList[0]).trimmed());
        rec.setValue(Entry_Page_Nbr,    QString(wordList[2]).trimmed().toInt());
        rec.setValue(Entry_Document_Id, documentId);
        entriesModel->insertRecord(-1, rec);
        if (!entriesModel->submitAll()) {
            qDebug() << "Unable to insert new index entry record: " << db.lastError().text();
            file.close();
            return false;
        }
    }

    qDebug() << "Completed";

    file.close();
    return true;
}

void BookmarksDB::saveEntryThumbnail(const QModelIndex & index, QImage & image)
{
    QBuffer buf;
    buf.open(QIODevice::WriteOnly);
    if (!image.isNull() && image.save(&buf, "PNG")) {
        if (index.isValid()) {
            entriesDBModel->setData(index, buf.data(), Qt::EditRole);
        }
    }
}

QString BookmarksDB::getAuthorsList(int entryId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM authors a LEFT JOIN author_entry ae ON ae.author_id = a.id WHERE ae.entry_id = ?;");
    query.addBindValue(entryId);
    query.exec();

    QSqlRecord record = query.record();

    QString result = "";
    QString separator = "";
    while (query.next()) {
        QString first_name = query.value(record.indexOf("first_name")).toString();
        QString last_name = query.value(record.indexOf("last_name")).toString();
        if (first_name.isEmpty()) {
            result += separator + last_name;
        }
        else {
            result += separator + last_name + ", " + first_name;
        }
        separator = " & ";
    }
    query.finish();
    return result;
}

bool BookmarksDB::saveAuthorsList(int entryId, const QString & authorsList)
{
    // An Authors list must follow the following syntax:
    //
    //   name & name & ...
    //
    // where name may be presented as follow:
    //
    //   last_name, first_name
    // or
    //   anything without a comma  << last_name will contain everything
    //
    // Examples:
    //
    //   Turcotte, Guy & Doe, John
    //   Doe, John & Sting

    qDebug() << "Entry Id: " << entryId;

    QSqlQuery query;
    QStringList authors = authorsList.split("&", QString::SkipEmptyParts);
    QStringList author;
    QVector<bool> alreadyThere(authors.size(), false);

    int i;

    // 1. Insure proper syntax and normalize entries
    //    Allow for 0 or 1 comma

    for (i = 0; i < authors.size(); i++ ) {
        author = authors[i].split(",", QString::SkipEmptyParts);
        if (author.size() > 2) {
            return false;
        }
        QString last_name  = author[0].trimmed();
        QString first_name = author.size() == 2 ? author[1].trimmed() : "";
        authors[i] = last_name + ", " + first_name;
    }

    // 2. Create entries in authors table if they do not exists

    for (i = 0; i < authors.size(); i++) {
        author = authors[i].split(",", QString::SkipEmptyParts);

        QString last_name  = author[0].trimmed();
        QString first_name = author.size() == 2 ? author[1].trimmed() : "";

        query.prepare("SELECT * FROM authors WHERE last_name = ? AND first_name = ?;");
        query.addBindValue(last_name);
        query.addBindValue(first_name);
        if (!query.exec()) {
            qDebug() << "Select problem: " << db.lastError().text();
            return false;
        }

        if (!query.next()) {
            query.finish();
            query.prepare("INSERT INTO authors (last_name, first_name) VALUES (?, ?);");
            query.addBindValue(last_name);
            query.addBindValue(first_name);
            if (!query.exec()) {
                qDebug() << "Insert problem: " << db.lastError().text();
                return false;
            }
        }
    }

    // 3. Cleanup author_entry table of removed entries

    query.prepare("SELECT * FROM authors a LEFT JOIN author_entry ae ON ae.author_id = a.id WHERE ae.entry_id = ?;");
    query.addBindValue(entryId);
    if (!query.exec()) {
        qDebug() << "Select problem: " << db.lastError().text();
        return false;
    }

    QSqlRecord record = query.record();

    if (!query.isActive()) {
        qDebug() << "Select problem: " << db.lastError().text();
        return false;
    }

    while (query.next()) {
        QString author = query.value(record.indexOf("last_name")).toString() + ", " +
                         query.value(record.indexOf("first_name")).toString();

        bool found = false;
        for (i = 0; i < authors.size(); i++) {
            found = authors[i] == author;
            if (found) {
                alreadyThere[i] = true;
                break;
            }
        }

        if (!found) {
            QSqlQuery q;
            q.prepare("DELETE FROM author_entry WHERE author_id = ? AND entry_id = ?;");
            q.addBindValue(query.value(record.indexOf("author_id")).toInt());
            q.addBindValue(entryId);
            if (!q.exec()) {
                qDebug() << "Delete problem: " << db.lastError().text();
                return false;
            }
        }
    }
    query.finish();

    // 4. Add author_entry for the one that are not defined

    for (i = 0; i < authors.size(); i++) {

        if (!alreadyThere[i]) {

            author = authors[i].split(",", QString::SkipEmptyParts);

            QString last_name  = author[0].trimmed();
            QString first_name = author.size() == 2 ? author[1].trimmed() : "";

            query.prepare("SELECT * FROM authors WHERE last_name = ? AND first_name = ?;");
            query.addBindValue(last_name);
            query.addBindValue(first_name);
            if (!query.exec()) {
                qDebug() << "Select problem: " << db.lastError().text();
                return false;
            }

            if (!query.next()) {
                return false;
            }

            QSqlQuery q;
            q.prepare("INSERT INTO author_entry (author_id, entry_id) VALUES (?, ?);");
            q.addBindValue(query.value(0).toInt());
            q.addBindValue(entryId);
            if (!q.exec()) {
                qDebug() << "Insert problem: " << db.lastError().text();
                return false;
            }

            query.finish();
        }
    }

    return true;
}
