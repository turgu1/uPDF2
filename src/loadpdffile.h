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

#ifndef LOADPDFFILE_H
#define LOADPDFFILE_H

#include <QObject>
#include <QThread>

#include <splash/SplashBitmap.h>

#include "pdfloader.h"

class LoadPDFFile : public QObject
{
    Q_OBJECT

  private:
    PDFFile    & file;
    u32          details;

    PDFLoader  * pdfLoader;

//  friend void PDFLoader::run();

  public:
    LoadPDFFile(const QString & fname, PDFFile & pdfFile);
    ~LoadPDFFile();
    void clean();

  public slots:
    void       handleResults();
    void pageReadyForRefresh();

  signals:
    void       refresh();
    void loadCompleted();
};

#endif // LOADPDFFILE_H
