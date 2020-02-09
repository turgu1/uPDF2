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

#ifndef UPDF_H
#define UPDF_H

#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <sys/time.h>
#include <lzo/lzo1x.h>

#include <QObject>
#include <QRect>
#include <PDFDoc.h>

class BookmarksDB;
class FilesCache;

#define DEBUGGING 1

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

typedef int64_t  s64;
typedef int32_t  s32;
typedef int16_t  s16;
typedef int8_t   s8;

enum ViewMode {
  VM_TRIM = 0,
  VM_WIDTH,
  VM_PAGE,
  VM_PGTRIM,
  VM_CUSTOMTRIM,
  VM_ZOOMFACTOR
};

struct SinglePageTrim {
  int              page;
  QRect            pageTrim;
  SinglePageTrim * next;
};

struct CustomTrim {
  QRect            odd, even;
  SinglePageTrim * singles;
  bool             initialized;  // true if the struct contains valid data
  bool             similar;      // true if even and odd are the same
};

struct FileViewParameters {
    QString       filename;
    int           columns;
    int           titlePageCount;
    float         xOff;
    float         yOff;
    float         viewZoom;
    QRect         winGeometry;
    ViewMode      viewMode;
    CustomTrim    customTrim;

    FileViewParameters * next;
};

struct BookmarksParameters {
    bool          bookmarksDbEnabled;
    QString       bookmarksDbFilename;
    QString       pdfFolderPrefix;
};

struct Preferences {
  bool fullScreenAtStartup;
  bool hideControlsAtStartup;
  bool viewClipboardSelection;
  bool keepRecent;
  bool recentGeometry;
  bool showLoadMetrics;
  bool logTrace;
  int  horizontalPadding;
  int  verticalPadding;
  int  doubleClickSpeed;
  QString logFilename;
  FileViewParameters defaultView;
  BookmarksParameters bookmarksParameters;
};

class BookmarksDB;

// They are instantiated at the beginning of mainwindow.cpp
extern u32           details;
extern QString       filenameAtStartup;
extern Preferences   preferences;
extern BookmarksDB * bookmarksDB;
extern FilesCache  * filesCache;

#include "utils.h"

#endif // UPDF_H
