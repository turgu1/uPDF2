#include "bookmarksbrowser.h"
#include "ui_bookmarksbrowser.h"
#include "bookmarksdb.h"

#include <QtGui>
#include <QtSql>
#include <QDataWidgetMapper>
#include <QFileDialog>
#include <QMessageBox>

#include "pagenbrdelegate.h"
#include "documentmapperdelegate.h"
#include "documentmodel.h"

BookmarksBrowser::BookmarksBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookmarksBrowser)
{
    ui->setupUi(this);

    entriesModel = &bookmarksDB->getEntriesModel();
    entriesModel->setEditStrategy(QSqlTableModel::OnRowChange);
    entriesModel->setFilter("");
    entriesModel->setSort(Entry_Caption, Qt::AscendingOrder);

    documentsModel = &bookmarksDB->getDocumentsModel();
    documentsModel->setEditStrategy(QSqlTableModel::OnRowChange);
    documentsModel->setFilter("");
    documentsModel->setSort(Document_Name, Qt::AscendingOrder);

    ui->entriesView->setModel(entriesModel);
    ui->entriesView->hideColumn(Entry_Id);
    ui->entriesView->hideColumn(Entry_Document_Id);
    ui->entriesView->hideColumn(Entry_Thumbnail);

    ui->entriesView->horizontalHeader()->resizeSection(3, 55);
    ui->entriesView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);

    ui->entriesView->setItemDelegate(new PageNbrDelegate(Entry_Page_Nbr));

    ui->documentsView->setModel(documentsModel);
    ui->documentsView->setModelColumn(Document_Name);

    ui->splitter->setSizes({1000, 3000});
    ui->splitter->setStretchFactor(0, 1);
    ui->splitter->setStretchFactor(1, 3);

    ui->entriesSplitter->setSizes({2000, 3000});
    ui->entriesSplitter->setStretchFactor(0, 2);
    ui->entriesSplitter->setStretchFactor(1, 3);

    documentMapper = new QDataWidgetMapper(this);
    documentMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    documentMapper->setModel(documentsModel);
    documentMapper->setItemDelegate(new DocumentMapperDelegate(this));
    documentMapper->addMapping(ui->documentNameEdit, Document_Name);
    documentMapper->addMapping(ui->documentFilenameEdit, Document_Filename);
    documentMapper->addMapping(ui->documentThumbnailView, Document_Thumbnail);

    entryMapper = new QDataWidgetMapper(this);
    entryMapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    entryMapper->setModel(entriesModel);
    entryMapper->setItemDelegate(new QSqlRelationalDelegate(this));
    entryMapper->addMapping(ui->entryCaptionEdit, Entry_Caption);
    entryMapper->addMapping(ui->entryPageNbrEdit, Entry_Page_Nbr);

    ui->pageNbrEdit->setValidator(new QIntValidator(1, 9999, this));

    connect(ui->documentsView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(changeDocument(QModelIndex)));
    connect(ui->entriesView->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(changeEntry(QModelIndex)));

    connect(ui->filterDocumentsButton,  SIGNAL(pressed()), this, SLOT(changeDocumentsFilter()));
    connect(ui->filterEntriesButton,    SIGNAL(pressed()), this, SLOT(  changeEntriesFilter()));
    connect(ui->doneButton,             SIGNAL(pressed()), this, SLOT(               accept()));

    connect(ui->addDocumentButton,      SIGNAL(pressed()), this, SLOT(          addDocument()));
    connect(ui->removeDocumentButton,   SIGNAL(pressed()), this, SLOT(       removeDocument()));
    connect(ui->addEntryButton,         SIGNAL(pressed()), this, SLOT(             addEntry()));
    connect(ui->removeEntryButton,      SIGNAL(pressed()), this, SLOT(          removeEntry()));

    connect(ui->documentFilenameButton, SIGNAL(pressed()), this, SLOT(           selectFile()));
    connect(ui->saveDocumentButton,     SIGNAL(pressed()), this, SLOT(         saveDocument()));
    connect(ui->cancelDocumentButton,   SIGNAL(pressed()), this, SLOT(       cancelDocument()));

    connect(ui->saveEntryButton,        SIGNAL(pressed()), this, SLOT(            saveEntry()));
    connect(ui->cancelEntryButton,      SIGNAL(pressed()), this, SLOT(          cancelEntry()));

    connect(ui->csvButton,              SIGNAL(pressed()), this, SLOT(          loadFromCSV()));

    connect(ui->previousButton,         SIGNAL(pressed()), this, SLOT(         previousPage()));
    connect(ui->nextButton,             SIGNAL(pressed()), this, SLOT(             nextPage()));

    connect(ui->documentSlider,         SIGNAL(   valueChanged(int)), this, SLOT(     setPage(int)));
    connect(ui->pageNbrEdit,            SIGNAL(editingFinished()),    this, SLOT(setPageFromEdit()));
    connect(ui->setButton,              SIGNAL(        pressed()),    this, SLOT(     changePage()));

    imagePageNbr = -1;

    documentsModel->select();

    const QModelIndex & idx = documentsModel->index(0, Document_Name);
    if (idx.isValid()) {
        ui->documentsView->setCurrentIndex(idx);
    }
}

BookmarksBrowser::~BookmarksBrowser()
{
    delete ui;
}

void BookmarksBrowser::changeEntriesFilter()
{
    const QModelIndex & idx = documentsModel->index(ui->documentsView->currentIndex().row(), Document_Name);
    changeDocument(idx);
}

void BookmarksBrowser::changeDocumentsFilter()
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

void BookmarksBrowser::changeDocument(const QModelIndex & index)
{
    if (index.isValid()) {
        currentFilename = absoluteFilename(documentsModel->index(ui->documentsView->currentIndex().row(), Document_Filename).data().toString());
        pageCount = getPageCount(currentFilename);
        ui->pageNbrEdit->setValidator(new QIntValidator(1, pageCount, this));

        documentMapper->setCurrentIndex(index.row());
        QString filter = "document_id = '" + index.model()->index(index.row(), Document_Id).data().toString() + '\'';
        if (!ui->entriesFilterEdit->text().isEmpty()) {
            filter += " and lower(caption) like lower('%" + ui->entriesFilterEdit->text() + "%\')";
        }
        entriesModel->setFilter(filter);
    }
    else {
        documentMapper->setCurrentIndex(-1);
        entryMapper->setCurrentIndex(-1);
        currentFilename = "";
    }
}

bool BookmarksBrowser::getEntryPageImage(int pageNbr, bool toShowFromPDFOnly)
{
    imagePageNbr = pageNbr;

    bool result;

    QModelIndex index = entriesModel->index(entryMapper->currentIndex(), Entry_Thumbnail);
    if (!toShowFromPDFOnly && index.isValid() && !entriesModel->data(index).isNull()) {
        QByteArray imageData = entriesModel->data(index).toByteArray();
        result = entryImage.loadFromData(imageData, "PNG");
    }
    else {
        result = getPageImage(currentFilename, entryImage, 144, imagePageNbr);
        if (!toShowFromPDFOnly && result && index.isValid()) {
            bookmarksDB->saveEntryThumbnail(index, entryImage);
        }
    }
    if (result) {
        ui->pageNbrEdit->blockSignals(true);
        ui->pageNbrEdit->setText(QString("%1").arg(pageNbr));
        ui->pageNbrEdit->blockSignals(false);

        ui->documentSlider->blockSignals(true);
        ui->documentSlider->setMaximum(pageCount);
        ui->documentSlider->setValue(pageNbr);
        ui->documentSlider->blockSignals(false);
    }

    return result;
}

void BookmarksBrowser::changeEntry(const QModelIndex & index)
{
    if (index.isValid()) {
        entryMapper->setCurrentIndex(index.row());

        int entryId = entriesModel->index(ui->entriesView->currentIndex().row(), Entry_Id).data().toInt();
        ui->entryAuthorsEdit->setText(bookmarksDB->getAuthorsList(entryId));

        if (QFileInfo(currentFilename).exists()) {
            if (getEntryPageImage(entriesModel->index(ui->entriesView->currentIndex().row(), Entry_Page_Nbr).data().toInt(), false)) {
                return;
            }
        }
        currentFilename = "";
    }
}

void BookmarksBrowser::paintEvent(QPaintEvent * event)
{
    if (ui->entriesView->currentIndex().isValid()) {
        QPixmap pixmap;
        pixmap.convertFromImage(entryImage);
        ui->pageImageLabel->setPixmap(
           pixmap.scaled(QSize(ui->pageImageLabel->width()  - 2,
                               ui->pageImageLabel->height() - 2),
                         Qt::KeepAspectRatio,
                         Qt::SmoothTransformation));
    }
    else {
        ui->pageImageLabel->clear();
        ui->pageImageLabel->setText(tr("Entry Image"));

        ui->pageNbrEdit->blockSignals(true);
        ui->pageNbrEdit->setText("");
        ui->pageNbrEdit->blockSignals(false);

        ui->documentSlider->blockSignals(true);
        ui->documentSlider->setValue(1);
        ui->documentSlider->blockSignals(false);

        imagePageNbr = -1;
    }

    QDialog::paintEvent(event);
}

void BookmarksBrowser::addDocument()
{
    documentMapper->submit();
    documentsModel->insertRow(0);
    ui->documentsView->setCurrentIndex(documentsModel->index(0, Document_Name));
    ui->documentNameEdit->setFocus();
}

void BookmarksBrowser::removeDocument()
{
    int row = documentMapper->currentIndex();
    //documentMapper->submit();
    documentsModel->removeRow(row);
    documentsModel->select();
    ui->documentsView->setCurrentIndex(documentsModel->index(qMin(row, documentsModel->rowCount() - 1), Document_Name));
}

void BookmarksBrowser::addEntry()
{
    if (ui->documentsView->currentIndex().isValid()) {
        entryMapper->submit();
        entriesModel->insertRow(0);
        ui->entriesView->setCurrentIndex(
                    entriesModel->index(0, Entry_Caption));
        ui->entryCaptionEdit->setFocus();
    }
}

void BookmarksBrowser::removeEntry()
{
    int row = entryMapper->currentIndex();
    //entryMapper->submit();
    entriesModel->removeRow(row);
    entriesModel->select();
    ui->entriesView->setCurrentIndex(entriesModel->index(qMin(row, entriesModel->rowCount() - 1), Entry_Caption));
}

void BookmarksBrowser::saveDocument()
{
    documentMapper->submit();
}

void BookmarksBrowser::cancelDocument()
{
    documentMapper->revert();
}

void BookmarksBrowser::saveEntry()
{
    QModelIndex id  = documentsModel->index(documentMapper->currentIndex(), Document_Id);
    QModelIndex idx = entriesModel->index(entryMapper->currentIndex(), Entry_Document_Id);

    if (id.isValid() && idx.isValid()) {
        if (entriesModel->setData(idx, id.data().toString(), Qt::EditRole)) {
            if (entryMapper->submit()) {
                int entryId = entriesModel->index(entryMapper->currentIndex(), Entry_Id).data().toInt();
                if (!bookmarksDB->saveAuthorsList(
                            entryId,
                            ui->entryAuthorsEdit->text())) {
                    QMessageBox::critical(this,
                                          "Authors Save Error",
                                          "Unable to save this authors' list.");
                }
            }
            else {
                QMessageBox::critical(this,
                                      "Entry Save Error",
                                      "Unable to save this entry.\n\n"
                                      "Message: " + entriesModel->lastError().text());
            }
        }
        else {
            QMessageBox::critical(this,
                                  "Unable to set data",
                                  "Unable to save document Id.");
        }
    }
    else {
        QMessageBox::critical(this,
                              "Internal Error",
                              "Some non-valid Qt Index, please advise the developper...");
    }
}

void BookmarksBrowser::cancelEntry()
{
    entryMapper->revert();
}

void BookmarksBrowser::selectFile()
{
  QString prefix = preferences.bookmarksParameters.pdfFolderPrefix;
  QString filename = QFileDialog::getOpenFileName(
              this,
              tr("Select Document"),
              prefix,
              tr("PDF (*.pdf);;All Files (*)"));

  if (!filename.isEmpty()) {
      ui->documentFilenameEdit->setText(relativeFilename(filename));

      if (ui->documentNameEdit->text().isEmpty()) {
          ui->documentNameEdit->setText(extractFilename(filename));
      }

      QImage thumbnail;

      if (getPageImage(filename, thumbnail)) {
          showThumbnail(thumbnail);
      }
  }

}

void BookmarksBrowser::showThumbnail(QImage & thumbnail)
{
    documentPixmap.convertFromImage(thumbnail);
    ui->documentThumbnailView->setPixmap(
       documentPixmap.scaled(QSize(ui->documentThumbnailView->width()  - 2,
                                   ui->documentThumbnailView->height() - 2)));
}

void BookmarksBrowser::loadFromCSV()
{
    QModelIndex idx = documentsModel->index(ui->documentsView->currentIndex().row(), Document_Id);

    if (idx.isValid()) {
        QString filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Select a CSV File"),
                    "",
                    tr("CSV (*.csv);;All Files (*)"));

        if (!filename.isEmpty() && QFileInfo(filename).exists()) {
            if (bookmarksDB->readCSVEntries(filename, idx.data().toInt())) {
                entriesModel->select();
                QMessageBox::information(
                    nullptr,
                    QObject::tr("Completed"),
                    QObject::tr("CSV retrieval completed."),
                    QMessageBox::Ok);

            }
            else {
                QMessageBox::critical(
                    nullptr,
                    QObject::tr("Not completed"),
                    QObject::tr("CVS read has not been completed.\n\n"
                                "Click Cancel to exit."),
                    QMessageBox::Cancel);
            }
        }
    }
    else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("No document"),
            QObject::tr("You must first define and select a document to insert CSV data.\n\n"
                        "Click Cancel to exit."),
            QMessageBox::Cancel);

    }

}

void BookmarksBrowser::nextPage()
{
    if ((imagePageNbr > 0) && (imagePageNbr < pageCount)) {
        getEntryPageImage(++imagePageNbr, true);
    }
}

void BookmarksBrowser::previousPage()
{
    if (imagePageNbr > 1) {
        getEntryPageImage(--imagePageNbr, true);
    }
}

void BookmarksBrowser::setPage(int page)
{
    getEntryPageImage(page, true);
}

void BookmarksBrowser::setPageFromEdit()
{
    getEntryPageImage(ui->pageNbrEdit->text().toInt(), true);
}

void BookmarksBrowser::changePage()
{
    ui->entryPageNbrEdit->setText(QString("%1").arg(imagePageNbr));
    bookmarksDB->saveEntryThumbnail(entriesModel->index(entryMapper->currentIndex(), Entry_Thumbnail), entryImage);
}
