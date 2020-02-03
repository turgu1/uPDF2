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

#ifndef PDFPAGEWORKER_H
#define PDFPAGEWORKER_H

#include <QObject>
#include <QRunnable>

#include "updf.h"
#include "pdffile.h"

class PDFPageWorker : public QObject, public QRunnable
{
    Q_OBJECT

  public:
    PDFPageWorker(PDFFile & file, const u32 pageNbr);
    void run();

  private:
    PDFFile & pdfFile;
    u32 page;

  signals:
    void refresh();
};

#endif // PDFPAGEWORKER_H
