#include "bookmarkselector.h"
#include "ui_bookmarkselector.h"

#include <QSqlTableModel>
#include <QSqlRelationalTableModel>
#include <QDebug>
#include <QFileInfo>
#include <QBuffer>

BookmarkSelector::BookmarkSelector(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookmarkSelector)
{
    ui->setupUi(this);

    entriesModel = &bookmarksDB->getEntriesModel();
    entriesModel->setSort(Entry_Page_Nbr, Qt::AscendingOrder);
    entriesModel->setFilter("");

    documentsModel = &bookmarksDB->getDocumentsModel();
    documentsModel->setSort(Document_Name, Qt::AscendingOrder);
    documentsModel->setFilter("");

    ui->entriesView->setModel(entriesModel);
    ui->entriesView->setModelColumn(Entry_Caption);

    ui->documentsView->setModel(documentsModel);
    ui->documentsView->setModelColumn(Document_Name);

    ui->imageSplitter->setSizes({ 1000, 2000 });

    connect(ui->documentsView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(changeDocument(QModelIndex)));
    connect(ui->entriesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(changeEntry(QModelIndex)));

    connect(ui->filterDocumentsButton,  SIGNAL(pressed()),                  this, SLOT(    changeDocumentsFilter()));
    connect(ui->filterEntriesButton,    SIGNAL(pressed()),                  this, SLOT(      changeEntriesFilter()));

    connect(ui->entriesView,            SIGNAL(doubleClicked(QModelIndex)), this, SLOT(   entrySelect(QModelIndex)));
    connect(ui->documentsView,          SIGNAL(doubleClicked(QModelIndex)), this, SLOT(documentSelect(QModelIndex)));

    connect(ui->cancelButton,           SIGNAL(clicked()),                  this, SLOT(                   reject()));
    connect(ui->selectButton,           SIGNAL(clicked()),                  this, SLOT(              selectEntry()));

    documentsModel->select();

    const QModelIndex & idx = documentsModel->index(0, Document_Name);
    if (idx.isValid()) {
        ui->documentsView->setCurrentIndex(idx);
    }
}

BookmarkSelector::~BookmarkSelector()
{
    delete ui;
}

void BookmarkSelector::documentSelect(const QModelIndex & index)
{
    if (index.isValid()) {
        // qDebug() << "Index Selection: " << index.data().toString();
        sel->filename = documentsModel->index(ui->documentsView->currentIndex().row(), Document_Filename).data().toString();
        sel->pageNbr  = 1;
        accept();
    }
    else {
        reject();
    }
}

void BookmarkSelector::entrySelect(const QModelIndex & index)
{
    if (index.isValid()) {
        // qDebug() << "Index Selection: " << index.data().toString();
        sel->filename = documentsModel->index(ui->documentsView->currentIndex().row(), Document_Filename).data().toString();
        sel->pageNbr  = entriesModel->index(ui->entriesView->currentIndex().row(), Entry_Page_Nbr).data().toInt();
        accept();
    }
    else {
        reject();
    }
}

void BookmarkSelector::selectEntry()
{
    QModelIndex index = entriesModel->index(ui->entriesView->currentIndex().row(), Entry_Caption);

    if (index.isValid()) {
        entrySelect(index);
        accept();
    }
    else {
        reject();
    }
}

bool BookmarkSelector::select(Selection & selection, const QString & currentFilename, int page)
{
    sel = &selection;

    QString filename = relativeFilename(currentFilename);
    int row = 0;

    while (row < documentsModel->rowCount()) {
        if (documentsModel->index(row, Document_Filename).data().toString().compare(filename, Qt::CaseInsensitive) == 0) {
            break;
        }
        row += 1;
    }

    if (row < documentsModel->rowCount()) {
        qDebug() << "Found file at row: " << row;
        ui->documentsView->setCurrentIndex(documentsModel->index(row, Document_Name));

        row = 0;
        while (row < entriesModel->rowCount()) {
            if (entriesModel->index(row, Entry_Page_Nbr).data().toInt() > page) {
                break;
            }
            row += 1;
        }
        row -= 1;
        if ((row >= 0) && (row < entriesModel->rowCount())) {
            ui->entriesView->setCurrentIndex(entriesModel->index(row, Entry_Caption));
        }
    }
    else {
        qDebug() << "Filename not found ( "<< filename << ")";
        ui->documentsView->setCurrentIndex(documentsModel->index(0, Document_Name));
        ui->entriesView->setCurrentIndex(entriesModel->index(0, Entry_Caption));
    }

    if (exec() == QDialog::Accepted) {
        return true;
    }
    return false;
}

void BookmarkSelector::changeDocument(const QModelIndex & index)
{
    if (index.isValid()) {
        QString filter = "document_id = '" + documentsModel->index(index.row(), Document_Id).data().toString() + '\'';
        if (!ui->entriesFilterEdit->text().isEmpty()) {
            filter += " and lower(caption) like lower('%" + ui->entriesFilterEdit->text() + "%\')";
        }
        entriesModel->setFilter(filter);
    }
}

void BookmarkSelector::changeEntry(const QModelIndex & index)
{
     bool result;

     QModelIndex idx = entriesModel->index(index.row(), Entry_Thumbnail);

     if (idx.isValid() && !entriesModel->data(idx).isNull()) {
         QByteArray imageData = entriesModel->data(idx).toByteArray();
         entryImage.loadFromData(imageData, "PNG");
     }
     else {
         QString filename = absoluteFilename(documentsModel->index(ui->documentsView->currentIndex().row(), Document_Filename).data().toString());
         int page = entriesModel->index(index.row(), Entry_Page_Nbr).data().toInt();
         result = getPageImage(filename, entryImage, 144, page);
         if (result && idx.isValid()) {
             bookmarksDB->saveEntryThumbnail(idx, entryImage);
         }
     }

}

void BookmarkSelector::paintEvent(QPaintEvent * event)
{
    if (ui->entriesView->currentIndex().isValid()) {
        QPixmap pixmap;
        pixmap.convertFromImage(entryImage);
        ui->pageImageLabel->setPixmap(
           pixmap.scaled(QSize(ui->pageImageLabel->width()  - 2,
                               ui->pageImageLabel->height() - 2),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation));
        ui->selectButton->setEnabled(true);
    }
    else {
        ui->pageImageLabel->clear();
        ui->pageImageLabel->setText(tr("Entry Image"));
        ui->selectButton->setEnabled(false);
    }

    QDialog::paintEvent(event);
}

void BookmarkSelector::changeEntriesFilter()
{
    const QModelIndex & idx = documentsModel->index(ui->documentsView->currentIndex().row(), Document_Name);
    changeDocument(idx);
}

void BookmarkSelector::changeDocumentsFilter()
{
    QString filter = ui->documentsFilterEdit->text().isEmpty() ? "" : "name like '%" + ui->documentsFilterEdit->text() + "%\'";
    documentsModel->setFilter(filter);
    const QModelIndex & idx = documentsModel->index(0, Document_Name);
    if (idx.isValid()) {
        ui->documentsView->setCurrentIndex(idx);
        changeDocument(idx);
    }
    else {
        entriesModel->setFilter("name = '00000000'");
    }
}
