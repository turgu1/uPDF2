/*
Copyright (C) 2015 Lauri Kasanen
Modifications Copyright (C) 2017, 2020 Guy Turcotte

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

#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QBitmap>

extern void    * xcalloc(size_t nmemb, size_t size);
extern void    * xmalloc(size_t size);
extern u32         usecs(const timeval old, const timeval now);
extern bool getPageImage(QString & filename, QImage & img, int pixelsPerInch = 18, int page = 1);
extern int  getPageCount(QString & filename);

extern QString absoluteFilename(const QString & filename);
extern QString relativeFilename(const QString & filename);
extern QString  extractFilename(const QString & filename);

#endif // UTILS_H
