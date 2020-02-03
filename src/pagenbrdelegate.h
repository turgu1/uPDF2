/*
Copyright (C) 2017, 2020 Guy Turcotte

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

#ifndef PAGENBRDELEGATE_H
#define PAGENBRDELEGATE_H

#include <QObject>

#include <QItemDelegate>

class PageNbrDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    PageNbrDelegate(int column, QObject * parent = nullptr);
    void paint(QPainter * painter,
               const QStyleOptionViewItem & option,
               const QModelIndex & index) const;
private:
    int column;
};

#endif // PAGENBRDELEGATE_H
