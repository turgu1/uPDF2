#include "documentmapperdelegate.h"
#include "bookmarksdb.h"

#include <QLabel>
#include <QBuffer>

DocumentMapperDelegate::DocumentMapperDelegate(QObject * parent) : QSqlRelationalDelegate(parent)
{

}

void DocumentMapperDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    switch(index.column()) {
        case Document_Thumbnail: {
            QLabel * label = qobject_cast<QLabel *>(editor);
            Q_ASSERT(label);
            if (label) {
                QByteArray imageData = index.data(Qt::EditRole).toByteArray();
                QPixmap pixmap;
                if (pixmap.loadFromData(imageData)) {
                    label->setPixmap(pixmap.scaled(label->width(), label->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                }
                else {
                    label->clear();
                    label->setText(tr("Thumbnail"));
                }
            }
            break;
        }
        default:
            QSqlRelationalDelegate::setEditorData(editor, index);
    }
}

void DocumentMapperDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const
{
    switch(index.column()) {
        case Document_Thumbnail: {
            QLabel * label = qobject_cast<QLabel *>(editor);
            Q_ASSERT(label);
            if (label) {
                QBuffer buf;
                buf.open(QIODevice::WriteOnly);
                if (label->pixmap() && label->pixmap()->save(&buf, "PNG")) {
                    model->setData(index, buf.data(), Qt::EditRole);
                }
            }
            break;
        }
        default:
            QSqlRelationalDelegate::setModelData(editor, model, index);
    }
}
