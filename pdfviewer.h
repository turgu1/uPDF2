#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QObject>
#include <QWidget>
#include <QPixmap>

#include "updf.h"
#include "pdffile.h"
#include "loadpdffile.h"

#define CACHE_MAX            30
#define PAGES_ON_SCREEN_MAX 100
#define MAX_COLUMNS_COUNT     5
#define MARGIN               36
#define MARGINHALF           18


// Used to keep drawing postion of displayed pages to
// help in the identification of the selection zone.
// Used by the endOffSelection method.
struct PagePos {
  float zoom, ratioX, ratioY;
  u32 page;
  int X0, Y0, W0, H0, X, Y, W, H;
};

enum TrimZoneLoc {
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

enum ViewMode {
  VM_TRIM = 0,
  VM_WIDTH,
  VM_PAGE,
  VM_PGTRIM,
  VM_CUSTOMTRIM,
  VM_ZOOMFACTOR
};

class PDFViewer : public QWidget
{
  public:
    PDFViewer(QWidget * parent = 0);
    void setPDFFile(PDFFile * f);
    void mouseMoveEvent(QMouseEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void paintEvent(QPaintEvent * event);
    QSize sizeHint() const;

  private:
    PDFFile * pdfFile;

    ViewMode  viewMode;
    float     viewZoom;
    float     xOff, yOff;
    u32       columns, titlePages;

    PagePos   pagePosOnScreen[PAGES_ON_SCREEN_MAX];
    u32       pagePosCount;

    u32       cachedSize;
    u8      * cache[CACHE_MAX];
    u16       cachedPage[CACHE_MAX];
    QPixmap   pix[CACHE_MAX];

    void    computeScreenSize();
    QPixmap doCache(const u32 page);
    u32     pageH(u32 page) const;
    u32     pageW(u32 page) const;
    u32     fullH(u32 page) const;
    u32     fullW(u32 page) const;
    bool    hasMargins(const u32 page) const;
    float   lineZoomFactor(const u32 firstPage, u32 &retWidth, u32 &retHeight) const;
    void    updateVisible() const;

  public slots:
    void validFilePresent();
};

#endif // PDFVIEWER_H
