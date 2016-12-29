/*
Copyright (C) 2017 Guy Turcotte

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

#ifndef PDFFILE_H
#define PDFFILE_H

#include <QObject>

#include "updf.h"

struct CachedPage {
  u8  * data;
  u32   size;
  u32   uncompressed;

  u32   w, h;
  u16   left, right, top, bottom;

  bool  ready;
};

class PDFFile : public QObject
{
    Q_OBJECT
  private:
    bool         valid;
    bool         loaded;
    bool         loading;

  public:
    explicit PDFFile(QObject *parent = 0);
    ~PDFFile();

    QString      filename;
    CachedPage * cache;
    PDFDoc     * pdf;
    u32          maxW, maxH;
    u32          pages;
    u32          firstVisible;
    u32          lastVisible;

    void setLoading(bool val);
    void setLoaded(bool val);
    void setValid(bool val);
    bool isValid() { return valid; }

  signals:
    void fileIsLoading();
    void fileLoadCompleted();
    void fileIsValid();
    void pageLoadCompleted();

  public slots:
    void pageCompleted();
};

#endif // PDFFILE_H
