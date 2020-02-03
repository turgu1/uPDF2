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

#ifndef BOOKMARKSBROWSER_H
#define BOOKMARKSBROWSER_H

#include "updf.h"
#include "documentmodel.h"

#include <QDialog>

class QWidget;
class QSqlRelationalTableModel;
class QSqlTableModel;
class QDataWidgetMapper;
class QItemSelection;
class QGraphicsScene;

namespace Ui {
class BookmarksBrowser;
}

class BookmarksBrowser : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarksBrowser(QWidget * parent = nullptr);
    ~BookmarksBrowser();

private slots:
    void changeDocumentsFilter();
    void   changeEntriesFilter();
    void        changeDocument(const QModelIndex & index);
    void           changeEntry(const QModelIndex & index);
    void           addDocument();
    void        removeDocument();
    void              addEntry();
    void           removeEntry();
    void          saveDocument();
    void        cancelDocument();
    void             saveEntry();
    void           cancelEntry();
    void            selectFile();
    void           loadFromCSV();
    void              nextPage();
    void          previousPage();
    void               setPage(int page);
    void       setPageFromEdit();
    void            changePage();

private:
    QPixmap                documentPixmap;
    QImage                 entryImage;
    int                    imagePageNbr;
    int                    pageCount;
    QString                currentFilename;
    Ui::BookmarksBrowser * ui;
    DocumentModel        * documentsModel;
    QSqlTableModel       * entriesModel;
    QDataWidgetMapper    * documentMapper;
    QDataWidgetMapper    * entryMapper;

    void      showThumbnail(QImage & thumbnail);
    bool  getEntryPageImage(int pageNbr, bool toShowFromPDFOnly = false);
    void saveEntryThumbnail(const QModelIndex & index);

protected:
    void         paintEvent(QPaintEvent * event)  Q_DECL_OVERRIDE;
};

#endif // BOOKMARKSBROWSER_H
