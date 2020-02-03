#include "documentmodel.h"

DocumentModel::DocumentModel(QObject * parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{

}

QVariant DocumentModel::data(
        const QModelIndex &index,
        int role) const
{
    // The following code is working well, but the effect on screen is not nice.
    // ToDo: Get better presentation using books front page.

//    if (role == Qt::DecorationRole) {
//        QByteArray imageData = this->index(index.row(), Document_Thumbnail).data(Qt::EditRole).toByteArray();
//        QPixmap pixmap;
//        pixmap.loadFromData(imageData);
//        return pixmap.scaled(70, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation);
//    }
//    else {
        return QSqlTableModel::data(index, role);
//    }
}
