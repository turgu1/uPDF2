/*
Copyright (C) 2017, 2020 Guy Turcotte
Portion Copyright (C) 2015 Lauri Kasanen

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

#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QtGlobal>
#include <QWidget>
#include <QPixmap>
#include <QRubberBand>
#include <QTimer>

#include "updf.h"
#include "pdffile.h"
#include "loadpdffile.h"

#define CACHE_MAX            30
#define PAGES_ON_SCREEN_MAX 100
#define MAX_COLUMNS_COUNT     5
#define MARGIN               36
#define MARGINHALF           18
#define SMALL_MOVE         0.05f

// Used to keep drawing postion of displayed pages to
// help in the identification of the selection zone.
// Used by the endOffSelection method.
struct PagePos {
  float zoom, ratioX, ratioY;
  u32   page;
  int   X0, Y0, W0, H0, X, Y, W, H;
};

enum ZoneLoc {
  TZL_N = 0,
  TZL_S,
  TZL_E,
  TZL_W,
  TZL_NE,
  TZL_NW,
  TZL_SE,
  TZL_SW,
  TZL_NONE
};

struct ViewState {
    bool     validDocument;
    int      page;
    int      pageCount;
    int      titlePageCount;
    int      columnCount;
    ViewMode viewMode;
    float    viewZoom;
    bool     trimSelection;
    bool     textSelection;
    bool     trimSimilar;
    bool     thisPageTrim;
    bool     someClipText;
    QString  metrics;
};

class PDFViewer : public QWidget
{
    Q_OBJECT

  public:
    explicit PDFViewer(QWidget * parent = 0);
    ~PDFViewer();

    void                setPDFFile(PDFFile     * f);
    void             keyPressEvent(QKeyEvent   * event)  Q_DECL_OVERRIDE;
    void                wheelEvent(QWheelEvent * event)  Q_DECL_OVERRIDE;
    void            mouseMoveEvent(QMouseEvent * event)  Q_DECL_OVERRIDE;
    void           mousePressEvent(QMouseEvent * event)  Q_DECL_OVERRIDE;
    void           mouseClickEvent(QMouseEvent * event); // My own event generated from mouseReleaseEvent...
    void     mouseDoubleClickEvent(QMouseEvent * event)  Q_DECL_OVERRIDE;
    void         mouseReleaseEvent(QMouseEvent * event)  Q_DECL_OVERRIDE;
    void                paintEvent(QPaintEvent * event)  Q_DECL_OVERRIDE;
    QSize                 sizeHint() const               Q_DECL_OVERRIDE;
    void             rubberBanding(bool show = true);
    QStringList getSinglePageTrims();
    bool     getFileViewParameters(FileViewParameters & params);
    void     setFileViewParameters(FileViewParameters & params);
    void                     reset();
    void                 sendState();

  private:
    PDFFile     * pdfFile;

    ViewMode      viewMode;
    float         viewZoom;
    float         xOff, yOff;
    u32           columns, titlePages;
    u32           leftDClickColumnsCount;
    u32           rightDClickColumnsCount;
    bool          silent;

    // pages positioning parameters for mouse selection decoding
    bool          zoneSelection; // Encompass both trimZoneSelection and textSelection
    bool          trimZoneSelection;
    bool          textSelection;
    ZoneLoc       zoneLoc;
    PagePos       pagePosOnScreen[PAGES_ON_SCREEN_MAX];
    u32           pagePosCount;
    u16           selX, selY, selX2, selY2, savedX, savedY;
    u16           lastX, lastY;
    bool          someDrag;
    bool          dragging;
    QRubberBand * selector;
    QString       clipText;
    QTimer      * singleClickTimer;
    u32           theMouseKey;
    bool          wasMouseDoubleClick;

    // caching
    //u32           cachedSize;
    //u8          * cache[CACHE_MAX];
    u16           cachedPage[CACHE_MAX];
    QPixmap       pix[CACHE_MAX];

    // custom trimming management (VM_CUSTOMTRIM)
    CustomTrim    customTrim;
    bool          singlePageTrim;

    bool          fileIsLoading;

    // internal processing support methods
    void            endOfSelection();
    ZoneLoc             getZoneLoc(s32 x, s32 y) const;
    void         computeScreenSize();
    QPixmap                getPage(const u32 page);
    u32                      pageH(u32 page) const;
    u32                      pageW(u32 page) const;
    u32                      fullH(u32 page) const;
    u32                      fullW(u32 page) const;
    bool                hasMargins(const u32 page) const;
    float           lineZoomFactor(const u32 firstPage, u32 &retWidth, u32 &retHeight) const;
    void             updateVisible() const;
    QRect       getTrimmingForPage(s32 page) const;
    void               pageChanged();
    float                  maxYOff() const;
    void                adjustYOff(float offset);
    void           adjustFloorYOff(float offset);
    void            resetSelection(bool anyway = false);
    u32                      pxrel(u32 page) const;

  public slots:
    void           textSelect(bool doSelect);
    void      copyToClipboard();
    void    trimZoneDifferent(bool diff);
    void         thisPageTrim(bool thisPage);
    void removeSinglePageTrim(s32 page);
    void    addSinglePageTrim(s32 page, QRect trim);
    void  clearAllSingleTrims();
    void       trimZoneSelect(bool doSelect);
    void         selectPageAt(s32 X, s32 Y, bool rightDClick);
    void             gotoPage(const int page);
    void       setColumnCount(int count);
    void setColumnCountFromIndex(int index);
    void    setTitlePageCount(int count);
    void                   up();
    void                 down();
    void                  top();
    void               bottom();
    void      beginningOfPage();
    void            endOfPage();
    void               pageUp();
    void             pageDown();
    void    pageUpWithOverlap();
    void  pageDownWithOverlap();
    void               zoomIn();
    void              zoomOut();
    void          setViewMode(int newViewMode);
    void        setZoomFactor(float zoomFactor);
    void          refreshView();
    void     singleMouseClick();

  signals:
    void stateUpdated(ViewState & state);
};

extern void clearSingles(CustomTrim & customTrim);
extern void  copySingles(CustomTrim &from, CustomTrim &to);

#endif // PDFVIEWER_H
