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

#ifndef ENTRYMAPPERDELEGATE_H
#define ENTRYMAPPERDELEGATE_H

#include <QObject>
#include <QSqlRelationalDelegate>

#include "updf.h"

class EntryMapperDelegate : public QSqlRelationalDelegate
{
public:
    EntryMapperDelegate(QObject * parent);
     void setEditorData(QWidget * editor, const QModelIndex & index) const;
      void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index ) const;
};

#endif // ENTRYMAPPERDELEGATE_H
