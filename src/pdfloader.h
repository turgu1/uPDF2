/*
Copyright (C) 2015 Lauri Kasanen
Modifications Copyright (C) 2017 Guy Turcotte

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

#ifndef PDFLOADER_H
#define PDFLOADER_H

#include <QtGlobal>
#include <QObject>
#include <QThread>
#include <QThreadPool>

#include "updf.h"
#include "pdffile.h"

class PDFLoader : public QThread
{
    Q_OBJECT

  public:
    PDFLoader(PDFFile & pdfFile);
    void run() Q_DECL_OVERRIDE;

  signals:
    void loadCompleted();
    void refresh();

  public slots:
    void abort();
    void refreshRequest();

  private:
    bool          aborting;
    PDFFile     & pdfFile;
    QThreadPool * threadPool;
};

#endif // PDFLOADER_H
