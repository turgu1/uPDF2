#include "pagenbrdelegate.h"

PageNbrDelegate::PageNbrDelegate(int column, QObject * parent)
    : QItemDelegate(parent)
{
    this->column = column;
}

void PageNbrDelegate::paint(QPainter * painter,
                            const QStyleOptionViewItem & option,
                            const QModelIndex & index) const
{

    QStyleOptionViewItem myOption = option;
    if (index.column() == this->column) {
        myOption.displayAlignment = Qt::AlignRight | Qt::AlignVCenter;
    }
    QItemDelegate::paint(painter, myOption, index);
}

