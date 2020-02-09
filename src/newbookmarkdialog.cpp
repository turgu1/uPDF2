#include "newbookmarkdialog.h"
#include "ui_newbookmarkdialog.h"

#include <QMessageBox>

#include "bookmarksdb.h"

NewBookmarkDialog::NewBookmarkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewBookmarkDialog)
{
    ui->setupUi(this);

    connect(ui->addButton,    SIGNAL(clicked()), this, SLOT(save()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

NewBookmarkDialog::~NewBookmarkDialog()
{
    delete ui;
}

bool NewBookmarkDialog::run(QString filename, int page)
{
    this->filename = filename;
    this->page = page;

    return exec() == QDialog::Accepted;
}

void NewBookmarkDialog::save()
{
    if (!ui->captionEdit->text().trimmed().isEmpty()) {
        QImage entryImage;
        getPageImage(filename, entryImage, 144, page);
        bool result = bookmarksDB->addEntry(
                    filename,
                    ui->captionEdit->text(),
                    ui->authorsEdit->text(),
                    page,
                    entryImage);
        if (!result) {
            QMessageBox::critical(this,
                                  "Bookmark Creation Error",
                                  "Unable to add a new Bookmark entry in the database.");
            reject();
        }
    }
    accept();
}
