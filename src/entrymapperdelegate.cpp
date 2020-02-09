#include "entrymapperdelegate.h"
#include "bookmarksdb.h"

#include <QLineEdit>

EntryMapperDelegate::EntryMapperDelegate(QObject * parent) : QSqlRelationalDelegate(parent)
{

}

void EntryMapperDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    switch(index.column()) {
        case Entry_Id: {
            QLineEdit * line = qobject_cast<QLineEdit *>(editor);
            Q_ASSERT(line);
            if (line) {
//                QByteArray imageData = index.data(Qt::EditRole).toByteArray();
//                QPixmap pixmap;
//                if (pixmap.loadFromData(imageData)) {
//                    label->setPixmap(pixmap);
//                }
//                else {
//                    label->clear();
//                    label->setText(tr("Thumbnail"));
//                }
            }
            break;
        }
        default:
            QSqlRelationalDelegate::setEditorData(editor, index);
    }
}

void EntryMapperDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
    switch(index.column()) {
        case Entry_Id: {
            QLineEdit * line = qobject_cast<QLineEdit *>(editor);
            Q_ASSERT(line);
            if (line) {
//                QBuffer buf;
//                buf.open(QIODevice::WriteOnly);
//                if (label->pixmap() && label->pixmap()->save(&buf, "PNG")) {
//                    model->setData(index, buf.data(), Qt::EditRole);
//                }
            }
            break;
        }
        default:
            QSqlRelationalDelegate::setModelData(editor, model, index);
    }
}
