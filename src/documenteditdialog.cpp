#include "documenteditdialog.h"
#include "ui_documenteditdialog.h"

DocumentEditDialog::DocumentEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DocumentEditDialog)
{
    ui->setupUi(this);
}

DocumentEditDialog::~DocumentEditDialog()
{
    delete ui;
}
