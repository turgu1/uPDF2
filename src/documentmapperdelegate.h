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

#ifndef DOCUMENTMAPPERDELEGATE_H
#define DOCUMENTMAPPERDELEGATE_H

#include <QObject>
#include <QSqlRelationalDelegate>

#include "updf.h"

class DocumentMapperDelegate : public QSqlRelationalDelegate
{
public:
    DocumentMapperDelegate(QObject * parent = 0);
        void setEditorData(QWidget * editor, const QModelIndex & index) const;
         void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;
};

#endif // DOCUMENTMAPPERDELEGATE_H
