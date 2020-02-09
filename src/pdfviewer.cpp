/*
Copyright (C) 2017 Guy Turcotte
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

#include "pdfviewer.h"

#include <QString>
#include <QRect>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <TextOutputDev.h>
#include <QDebug>
#include <QMessageBox>
#include <QApplication>

#define CTRL_PRESSED event->modifiers().testFlag(Qt::ControlModifier)
#define LEFT_BUTTON  (event->button() == Qt::LeftButton)
#define RIGHT_BUTTON (event->button() == Qt::RightButton)

PDFViewer::PDFViewer(QWidget * parent) : QWidget(parent),
                  pdfFile(nullptr),
                 viewMode(VM_PAGE),
                 viewZoom(0.5f),
                     xOff(0.0f),
                     yOff(0.0f),
                  columns(1),
               titlePages(0),
   leftDClickColumnsCount(2),
  rightDClickColumnsCount(5),
                   silent(false),
            zoneSelection(false),
        trimZoneSelection(false),
            textSelection(false),
             pagePosCount(0),
                     selX(0),
                     selY(0),
                    selX2(0),
                    selY2(0),
                   savedX(0),
                   savedY(0),
                    lastX(0),
                    lastY(0),
                 someDrag(false),
                 selector(NULL),
                 clipText(""),
      wasMouseDoubleClick(false),
               cachedSize(7 * 1024 * 1024),
           singlePageTrim(false),
            fileIsLoading(false)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update();
  updateGeometry();

  customTrim.initialized = false;
  customTrim.similar     = true;
  customTrim.singles     = NULL;

  // Initialize the uncompressed images cache
  srand(time(NULL));

  for (u32 i = 0; i < CACHE_MAX; i++) {
    cache[i]      = (u8 *) xcalloc(cachedSize, 1);
    cachedPage[i] = USHRT_MAX;
    pix[i]        = QPixmap();
  }

  setFocusPolicy(Qt::StrongFocus);

  QApplication::setDoubleClickInterval(preferences.doubleClickSpeed);

  singleClickTimer = new QTimer(this);
  singleClickTimer->setSingleShot(true);
  connect(singleClickTimer, SIGNAL(timeout()), this, SLOT(singleMouseClick()));
}

PDFViewer::~PDFViewer()
{
  if (singleClickTimer) {
    singleClickTimer->stop();
    delete singleClickTimer;
  }
}

void PDFViewer::setPDFFile(PDFFile * f)
{
  pdfFile = f;

  connect(pdfFile, SIGNAL(pageLoadCompleted()), this, SLOT(refreshView()));

  update();
}

void PDFViewer::keyPressEvent(QKeyEvent * event)
{
  resetSelection();
  switch (event->key()) {
    case Qt::Key_J:    if (CTRL_PRESSED) bottom(); else pageDownWithOverlap(); break;
    case Qt::Key_K:    if (CTRL_PRESSED) top();    else pageUpWithOverlap();   break;
    case Qt::Key_Home: if (CTRL_PRESSED) top();    else beginningOfPage();     break;
    case Qt::Key_End:  if (CTRL_PRESSED) bottom(); else endOfPage();           break;

    case Qt::Key_Up:       up();              break;
    case Qt::Key_Down:     down();            break;
    case Qt::Key_PageDown: pageDown();        break;
    case Qt::Key_PageUp:   pageUp();          break;
    case Qt::Key_Right:    pageDown();        break;
    case Qt::Key_Left:     pageUp();          break;

    default:
      if (event->matches(QKeySequence::Copy)) {
        copyToClipboard();
      }
      else {
        QWidget::keyPressEvent(event);
      }
  }
}

void PDFViewer::wheelEvent(QWheelEvent * event)
{
  int numPixels  = - event->angleDelta().y() / 30;

  if (event->inverted()) numPixels = -numPixels;

  if (CTRL_PRESSED) {
    viewMode = VM_ZOOMFACTOR;
    if (numPixels > 0){
      viewZoom *= 0.833333f;
      if (viewZoom < 0.1f) viewZoom = 0.1f;
    }
    else{
      viewZoom *= 1.2f;
      if (viewZoom > 10.0f) viewZoom = 10.0f;
    }
  }
  else{
    adjustYOff(SMALL_MOVE * numPixels);
  }
  pageChanged();
  event->accept();
}

void PDFViewer::mouseMoveEvent(QMouseEvent * event)
{
  const int mY = event->y();
  const int mX = event->x();

  if ((mX < 0) || (mY < 0)) return;

  if (dragging) {

    const int movedY = mY - lastY;
    const int movedX = mX - lastX;

    someDrag = true;
    if (zoneSelection) {

      switch (zoneLoc) {
        case TZL_NE: 	selX2 = mX; selY  = mY; break;
        case TZL_NW:  selX  = mX; selY  = mY; break;
        case TZL_SE:  selX2 = mX; selY2 = mY; break;
        case TZL_SW:  selX  = mX; selY2 = mY; break;
        case TZL_E:  	selX2 = mX;             break;
        case TZL_W:   selX  = mX;             break;
        case TZL_N:   selY  = mY;             break;
        case TZL_S:   selY2 = mY;             break;
        default:      selX2 = mX; selY2 = mY; break;
      }

      int x, y, w, h;
      if (selX < selX2) {
        x = selX;
        w = selX2 - selX;
      }
      else {
        x = selX2;
        w = selX - selX2;
      }
      if (selY < selY2) {
        y = selY;
        h = selY2 - selY;
      }
      else {
        y = selY2;
        h = selY - selY2;
      }

      if ((x != 0) && (y != 0) && (w != 0) && (h != 0)) {
        if (!selector) selector = new QRubberBand(QRubberBand::Rectangle, this);
        selector->setGeometry(QRect(x, y, w, h));
        if (!selector->isVisible()) selector->show();
      }
//    update();
    }
    else {
      setCursor(Qt::OpenHandCursor);

      if (pdfFile->maxH) {
        adjustYOff(-((movedY / viewZoom) / pdfFile->maxH));
      }
      else {
        adjustYOff(-((movedY / (float) height()) / viewZoom));
      }

      if (viewMode == VM_ZOOMFACTOR) {
        const u32 lineWidth  = fullW(yOff);
        xOff += ((float) movedX / viewZoom) / lineWidth;
        float maxchg = (((((float) width() / viewZoom) + lineWidth) / 2) / lineWidth) - 0.1f;
        if (xOff < -maxchg) {
          xOff = -maxchg;
        }
        if (xOff > maxchg) {
          xOff = maxchg;
        }
      }

      lastY = mY;
      lastX = mX;
      pageChanged();
    }
  }
  else {

    // Set the cursor appropriately
    if (zoneSelection) {
      ZoneLoc zl = getZoneLoc(event->x(), event->y());
      switch (zl) {
        case TZL_NW:
        case TZL_SE: setCursor(Qt::SizeAllCursor); break;
        case TZL_NE:
        case TZL_SW: setCursor(Qt::SizeAllCursor); break;
        case TZL_E:
        case TZL_W:  setCursor(Qt::SizeHorCursor); break;
        case TZL_N:
        case TZL_S:  setCursor(Qt::SizeVerCursor); break;

        default:     setCursor(Qt::CrossCursor);   break;
      }
    }
    else {
      setCursor(Qt::ArrowCursor);
    }
  }
  event->accept();
}

void PDFViewer::mousePressEvent(QMouseEvent * event)
{
  lastX = event->x();
  lastY = event->y();

  if (LEFT_BUTTON) {
    dragging = true;
    if (zoneSelection) {
      savedX = selX;
      savedY = selY;

      zoneLoc = getZoneLoc(event->x(), event->y());
      if (zoneLoc == TZL_NONE) {
        selX   = lastX;
        selY   = lastY;
      }
    }
  }

  event->accept();
}

// This is called at the end of a mouseReleaseEvent, if no text or trim zone selection
// was being processed.
void PDFViewer::mouseClickEvent(QMouseEvent * event)
{
  if (!someDrag) {
    theMouseKey = (LEFT_BUTTON) ? 1 : (RIGHT_BUTTON) ? 2 : 0;
    singleClickTimer->start(preferences.doubleClickSpeed + 50);
  }
  someDrag = false;
  event->accept();
}

void PDFViewer::singleMouseClick()
{
  if (theMouseKey == 1) {
    pageDownWithOverlap();
  }
  else if (theMouseKey == 2) {
    pageUpWithOverlap();
  }
}

void PDFViewer::mouseDoubleClickEvent(QMouseEvent * event)
{
  singleClickTimer->stop();
  wasMouseDoubleClick = true;

  lastX = event->x();
  lastY = event->y();

  if (LEFT_BUTTON) {
    someDrag = false;
    dragging = false;
    selectPageAt(lastX, lastY, CTRL_PRESSED);
  }
  else if (RIGHT_BUTTON) {
    selectPageAt(lastX, lastY, true);
  }

  event->accept();
}

void PDFViewer::mouseReleaseEvent(QMouseEvent * event)
{
  dragging = false;
  setCursor(Qt::ArrowCursor);

  if (zoneSelection) {
    if (LEFT_BUTTON && selX && selY && selX2 && selY2) {

      if ((selX != selX2) && (selY != selY2) && someDrag) {

        if (trimZoneSelection) {
          s32 tmp;
          if (selX > selX2) {
            tmp   = selX;
            selX  = selX2;
            selX2 = tmp;
          }
          if (selY > selY2) {
            tmp   = selY;
            selY  = selY2;
            selY2 = tmp;
          }
        }
        endOfSelection();
      }
      if (trimZoneSelection && !someDrag) {
        selX = savedX;
        selY = savedY;
      }
    }
  }
  else {
    if (!wasMouseDoubleClick) mouseClickEvent(event);
    wasMouseDoubleClick = false;
  }
}

QSize PDFViewer::sizeHint() const
{
  return QSize(0,0);
}

QStringList PDFViewer::getSinglePageTrims()
{
  SinglePageTrim * pt = customTrim.singles;
  QStringList list;
  while (pt) {
    list.append(QString("%1").arg(pt->page + 1));
    pt = pt->next;
  }
  return list;
}

bool PDFViewer::getFileViewParameters(FileViewParameters & params)
{
  if (!pdfFile->isValid()) return false;

  params.filename       = pdfFile->filename;
  params.columns        = columns          ;
  params.titlePageCount = titlePages       ;
  params.xOff           = xOff             ;
  params.yOff           = yOff             ;
  params.viewMode       = viewMode         ;
  params.viewZoom       = viewZoom         ;
  params.customTrim     = customTrim       ;

  if (customTrim.initialized) {
    copySingles(customTrim, params.customTrim);
  }
  else {
    params.customTrim.singles = NULL;
  }

  return true;
}

void PDFViewer::setFileViewParameters(FileViewParameters & params)
{
  silent = true;

  if (customTrim.initialized) clearSingles(customTrim);

  customTrim = params.customTrim;

  if (customTrim.initialized) {
    copySingles(params.customTrim, customTrim);
  }

  setColumnCount(params.columns);
  setTitlePageCount(params.titlePageCount);

  xOff        = params.xOff;
  yOff        = params.yOff;
  viewZoom    = params.viewZoom;
  setViewMode(params.viewMode);

  silent = false;
  pageChanged();
}

// Called when a new document is to be loaded
void PDFViewer::reset()
{
  xOff = yOff = 0.0f;
  //adjustYOff(0.0f);
  resetSelection();

  for (u32 i = 0; i < CACHE_MAX; i++) {
    cachedPage[i] = USHRT_MAX;
  }
}

void PDFViewer::sendState()
{
  ViewState state;
  u32 page = yOff;

  state.validDocument  = pdfFile->isValid();
  state.page           = page;
  state.pageCount      = pdfFile->pages;
  state.columnCount    = columns;
  state.titlePageCount = titlePages;
  state.viewMode       = viewMode;
  state.viewZoom       = viewZoom;
  state.trimSelection  = trimZoneSelection;
  state.textSelection  = textSelection;
  state.trimSimilar    = customTrim.similar;
  state.thisPageTrim   = singlePageTrim;
  state.someClipText   = clipText.size() > 0;

  if (pdfFile->totalSize > 0) {
    state.metrics = QString(tr("Mem %1MB\nRatio %2%\nTime %3s"))
        .arg((float)pdfFile->totalSizeCompressed / 1000000.0f, 0, 'f', 2)
        .arg((float)(pdfFile->totalSizeCompressed) / pdfFile->totalSize * 100.0, 0, 'f', 1)
        .arg((float)pdfFile->loadTime / 1000000.0f, 0, 'f', 2);
  }
  else {
    state.metrics = "";
  }

  emit stateUpdated(state);
}

void copySingles(CustomTrim &from, CustomTrim &to)
{
  SinglePageTrim *curr, *curr_i, *prev;

  if (!from.initialized) {
    to.singles = NULL;
    return;
  }

  curr_i = from.singles;
  prev = NULL;

  while (curr_i) {
    curr = (SinglePageTrim *) xmalloc(sizeof(SinglePageTrim));
    *curr = *curr_i;
    if (prev) {
      prev->next = curr;
    }
    else {
      to.singles = curr;
    }
    prev = curr;
    curr->next = NULL;
    curr_i = curr_i->next;
  }
}

void clearSingles(CustomTrim & customTrim)
{
  SinglePageTrim *curr, *next;

  if (customTrim.initialized) {
    curr = customTrim.singles;
    while (curr) {
      next = curr->next;
      free(curr);
      curr = next;
    }
  }
  customTrim.singles = NULL;
}

void PDFViewer::endOfSelection()
{
  s32 X, Y, W, H;
  if (selX < selX2) {
    X = selX;
    W = selX2 - X;
  }
  else {
    X = selX2;
    W = selX - X;
  }

  if (selY < selY2) {
    Y = selY;
    H = selY2 - Y;
  }
  else {
    Y = selY2;
    H = selY - Y;
  }

  // Search for the page caracteristics saved before with
  // the draw method.
  u32 idx = 0;
  PagePos *pp = pagePosOnScreen;

  while (idx < pagePosCount) {
    if ((X >= pp->X0)         &&
        (Y >= pp->Y0)         &&
        (X < (pp->X + pp->W)) &&
        (Y < (pp->Y + pp->H))) {
      break;
    }
    idx++;
    pp++;
  }
  if (idx >= pagePosCount) {
    update();
    return; // Not found
  }

  if (textSelection) {
    // Adjust the selection rectangle to be inside the real page data
    if ((X + W) > (pp->X + pp->W)) {
      W = pp->X + pp->W - X;
    }
    if ((Y + H) > (pp->Y + pp->H)) {
      H = pp->Y + pp->H - Y;
    }
    if (((X + W) < pp->X) ||
      ((Y + H) < pp->Y)) {
      update();
      return; // Out of the printed area
    }
    if (X < pp->X) {
      W -= (pp->X - X);
      X = pp->X;
    }
    if (Y < pp->Y) {
      H -= (pp->Y - Y);
      Y = pp->Y;
    }
  }
  else if (trimZoneSelection) {
    // Adjust the selection rectangle to be inside the extended page data
    if ((X + W) > (pp->X0 + pp->W0)) {
      W = pp->X0 + pp->W0 - X;
    }
    if ((Y + H) > (pp->Y0 + pp->H0)) {
      H = pp->Y0 + pp->H0 - Y;
    }
    if (((X + W) < pp->X0) || ((Y + H) < pp->Y0)) {
      update();
      return; // Out of the printed area
    }
    if (X < pp->X0) {
      W -= (pp->X0 - X);
      X = pp->X0;
    }
    if (Y < pp->Y0) {
      H -= (pp->Y0 - Y);
      Y = pp->Y0;
    }
  }

  // Convert to page coords
  if (viewMode == VM_TRIM   ||
      viewMode == VM_PGTRIM ||
     (viewMode == VM_CUSTOMTRIM && !trimZoneSelection)) {
    X = X - pp->X0 + (pp->zoom * pdfFile->cache[pp->page].left);
    Y = Y - pp->Y0 + (pp->zoom * pdfFile->cache[pp->page].top ) ;

    if (hasMargins(pp->page)) {
      X -= (pp->X - pp->X0);
      Y -= (pp->Y - pp->Y0);
    }
  }
  else {
    X -= pp->X0;
    Y -= pp->Y0;
  }

  // Convert to original page resolution
  X /= (pp->zoom * pp->ratioX);
  Y /= (pp->zoom * pp->ratioY);
  W /= (pp->zoom * pp->ratioX);
  H /= (pp->zoom * pp->ratioY);

  if (textSelection) {
    TextOutputDev * const dev = new TextOutputDev(NULL, true, 0, false, false);
    pdfFile->pdf->displayPage(dev, pp->page + 1, 144, 144, 0, true, false, false);
    GooString *str = dev->getText(X, Y, X + W, Y + H);
//  const char * const cstr = str->getCString();
    const char * const cstr = str->c_str();

    // Save it for clipboard retrieval
    clipText = cstr;

    if (preferences.viewClipboardSelection) {
      QMessageBox msgBox;
      QPushButton * copyButton = NULL;
      if (clipText.size() > 0) {
        copyButton = msgBox.addButton(tr("Copy to Clipboard"), QMessageBox::ActionRole);
      }
      QPushButton * abortButton = msgBox.addButton(QMessageBox::Close);

      msgBox.setWindowTitle("Selected Text");
      msgBox.setText(clipText.size() > 0 ? clipText : "[No Text Selected]");
      msgBox.exec();

      if ((msgBox.clickedButton() != NULL) &&
          (msgBox.clickedButton() == (QAbstractButton *) copyButton)) {
        copyToClipboard();
      }
    }

    delete str;
    delete dev;

    sendState();
  }
  else if (trimZoneSelection) {
    if (singlePageTrim) {
      addSinglePageTrim(pp->page, QRect(X, Y, W, H));
    }
    else {
      if (customTrim.similar) {
        customTrim.odd = customTrim.even = QRect(X, Y, W, H);
      }
      else if (pp->page & 1) {
        customTrim.even = QRect(X, Y, W, H);
      }
      else {
        customTrim.odd = QRect(X, Y, W, H);
      }
      customTrim.initialized = true;
    }
  }
}

ZoneLoc PDFViewer::getZoneLoc(s32 x, s32 y) const
{
  if (x > (selX - 5) && x < (selX + 5)) {
    if      (y > (selY  - 5) && y < (selY  + 5)) return TZL_NW;
    else if (y > (selY2 - 5) && y < (selY2 + 5)) return TZL_SW;
    else if (y > (selY  - 5) && y < (selY2 + 5)) return TZL_W;
    else                                         return TZL_NONE;
  }
  else if (x > (selX2 - 5) && x < (selX2 + 5)) {
    if      (y > (selY  - 5) && y < (selY  + 5)) return TZL_NE;
    else if (y > (selY2 - 5) && y < (selY2 + 5)) return TZL_SE;
    else if (y > (selY  - 5) && y < (selY2 + 5)) return TZL_E;
    else                                         return TZL_NONE;
  }
  else if (y > (selY - 5) && y < (selY + 5)) {
    if      (x > (selX - 5) && x < (selX2 + 5))  return TZL_N;
    else                                         return TZL_NONE;
  }
  else if (y > (selY2 - 5) && y < (selY2 + 5)) {
    if      (x > (selX - 5) && x < (selX2 + 5))  return TZL_S;
    else                                         return TZL_NONE;
  }
  else                                           return TZL_NONE;
}

QRect PDFViewer::getTrimmingForPage(s32 page) const
{
  SinglePageTrim * curr = customTrim.singles;
  QRect result;

  while (curr && (curr->page < page)) curr = curr->next;

  if (curr && (curr->page == page)) {
    result = curr->pageTrim;
  }
  else {
    if (page & 1) {
      result = customTrim.even;
    }
    else {
      result = customTrim.odd;
    }
  }

  return result;
}

// Compute the height of a page
u32 PDFViewer::pageH(u32 page) const
{
  if (!pdfFile->cache[page].ready) page = 0;

  s32 h;

  if (viewMode == VM_TRIM || viewMode == VM_PGTRIM) {
    h = pdfFile->cache[page].h;
  }
  else if (viewMode == VM_CUSTOMTRIM && !trimZoneSelection) {
    if (customTrim.initialized) {
      QRect theTrim = getTrimmingForPage(page);
      h = theTrim.height();
    }
    else {
      return pdfFile->cache[page].h;
    }
  }
  else { // VM_PAGE || VM_WIDTH || VM_ZOOMFACTOR || (VM_CUSTOMTRIM && trimZoneSelection)
    return  pdfFile->cache[page].h +
        pdfFile->cache[page].top   +
        pdfFile->cache[page].bottom;
  }

  return h < 2*MARGIN ? h + MARGIN : h;
}

// Compute the width of a page
u32 PDFViewer::pageW(u32 page) const
{
  if (!pdfFile->cache[page].ready) page = 0;

  if (viewMode == VM_TRIM || viewMode == VM_PGTRIM) {
    return pdfFile->cache[page].w;
  }
  else if (viewMode == VM_CUSTOMTRIM && !trimZoneSelection) {
    if (customTrim.initialized) {
      QRect theTrim = getTrimmingForPage(page);
      return  theTrim.width();
    }
    else {
      return pdfFile->cache[page].w;
    }
  }
  else { // VM_PAGE || VM_WIDTH || VM_ZOOMFACTOR || (VM_CUSTOMTRIM && trimZoneSelection)
    return pdfFile->cache[page].w    +
           pdfFile->cache[page].left +
           pdfFile->cache[page].right;
  }
}

// Compute the height of a line of pages, selecting the page that is the
// largest in height.
u32 PDFViewer::fullH(u32 page) const
{
  if (!pdfFile->cache[page].ready) page = 0;

  u32 fh = 0;
  u32 h;
  u32 i, limit;

  if ((titlePages > 0) && (titlePages < columns) && (page < titlePages)) {
    limit = titlePages;
  }
  else {
    limit = page + columns;
  }

  if (limit > pdfFile->pages) limit = pdfFile->pages;

  for (i = page; i < limit; i++) {
    h = pageH(i);
    if (h > fh) fh = h;
  }

  return fh;
}

// Compute the width of a line of pages. The page number is the one that is
// the first on the left of the line.
u32 PDFViewer::fullW(u32 page) const
{
  u32 fw = 0;
  u32 i, limit;

  if ((titlePages > 0) && (titlePages < columns) && (page < titlePages)) {
    limit = titlePages;
  }
  else {
    limit = page + columns;
  }

  if (limit > pdfFile->pages) limit = pdfFile->pages;

  for (i = page; i < (page + columns); i++) {
    fw += pageW(i < limit ? i : 0);
  }

  // Add the padding between columns
  return fw + (columns - 1) * preferences.horizontalPadding;
}

bool PDFViewer::hasMargins(const u32 page) const
{
  if (!pdfFile->cache[page].ready) {
    return
      pdfFile->cache[0].left   > MARGIN ||
      pdfFile->cache[0].right  > MARGIN ||
      pdfFile->cache[0].top    > MARGIN ||
      pdfFile->cache[0].bottom > MARGIN;
  }

  return
    pdfFile->cache[page].left   > MARGIN ||
    pdfFile->cache[page].right  > MARGIN ||
    pdfFile->cache[page].top    > MARGIN ||
    pdfFile->cache[page].bottom > MARGIN;
}

// Compute the required zoom factor to fit the line of pages on the screen,
// according to the zoom mode parameter if not a custom zoom.
float PDFViewer::lineZoomFactor(const u32 firstPage, u32 &retWidth, u32 &retHeight) const
{
  const u32 lineWidth  = fullW(firstPage);
  const u32 lineHeight = fullH(firstPage);

  float zoomFactor;

  switch (viewMode) {
    case VM_TRIM:
    case VM_WIDTH:
      zoomFactor = (float)(width()) / lineWidth;
      break;
    case VM_PAGE:
    case VM_PGTRIM:
    case VM_CUSTOMTRIM:
      if (((float)lineWidth / lineHeight) > ((float)(width()) / height())) {
        zoomFactor = (float)(width()) / lineWidth;
      }
      else {
        zoomFactor = (float)(height()) / lineHeight;
      }
      break;
    default:
      zoomFactor = viewZoom;
      break;
  }

  retWidth  = lineWidth;
  retHeight = lineHeight;

  return zoomFactor;
}

// From the current zoom mode and view offset, update the visible page info
// Will adjust the following parameters:
//
// - pdfFile->firstVisible
// - pdfFile->lastVisible
//
// This method as been extensively modified to take into account multicolumns
// and the fact that no page will be expected to be of the same size as the others,
// both for vertical and horizontal limits. Because of these constraints, the
// zoom factor cannot be applied to the whole screen but for each "line" of
// pages.
//
// Those are very very conservative numbers to compute last_visible.
// The end of the draw process will adjust precisely the value

void PDFViewer::updateVisible() const
{

  static const u32 maxLinesPerScreen[MAX_COLUMNS_COUNT] = { 2, 3, 4, 5, 6 };

  // Adjust file->first_visible

  pdfFile->firstVisible = yOff < 0.0f ? 0 : yOff;
  if (pdfFile->firstVisible > pdfFile->pages - 1) {
    pdfFile->firstVisible = pdfFile->pages - 1;
  }

  // Adjust file->last_visible

  u32 newLastVisible = pdfFile->firstVisible + maxLinesPerScreen[columns - 1] * columns;
  if (newLastVisible >= pdfFile->pages) {
    pdfFile->lastVisible = pdfFile->pages - 1;
  }
  else {
    pdfFile->lastVisible = newLastVisible;
  }
}

// Put an uncompressed page Pixmap version in the cache if not already
// available. Return the uncompressed page.
QPixmap PDFViewer::getPage(const u32 page)
{
  u32 i;
  for (i = 0; i < CACHE_MAX; i++) {
    if (cachedPage[i] == page) return pix[i]; // Already there
  }

  CachedPage * const cur = &pdfFile->cache[page];

  // Adjust cache to be big enough if required
  if (cur->uncompressed > cachedSize) {
    qDebug() <<  "Cache size update...";
    cachedSize = cur->uncompressed;

    for (i = 0; i < CACHE_MAX; i++) {
      // Reallocation render Pixmaps inconsistent. Seems that
      // Pixmaps keep pointer on image data...
      cachedPage[i] = USHRT_MAX;
      cache[i] = (u8 *) realloc(cache[i], cachedSize);
      if (cache[i] == nullptr) {
          QMessageBox::critical(nullptr, "Memory Allocation Error", "Memory Allocation Error");
          exit(1);
      }
    }
  }

  // Be safe
  if (!cur->ready) return QPixmap();

  // Insert it in the cache. Pick the slot at random.
  const u32 dst = rand() % CACHE_MAX;

  // qDebug() << "Page: " << page << ", Size: " << cur->size;

  lzo_uint dstSize = cachedSize;
  const int ret = lzo1x_decompress(cur->data,
          cur->size,
          cache[dst],
          &dstSize,
          NULL);

  if (ret != LZO_E_OK || dstSize != cur->uncompressed) {
    qCritical() << tr("Fatal: Error decompressing") << endl;
    exit(1);
  }

  cachedPage[dst] = page;

  // Create the Pixmap
  //qDebug() << "cur->w: " << cur->w << ", cur->h: " << cur->h;

  QImage img(cache[dst], cur->w, cur->h, QImage::Format_RGB32);

  if (!pix[dst].convertFromImage(img)) {
    qCritical() << tr("Fatal: QPixmap::loadFromData failed") << endl;
    exit(1);
  }

  return pix[dst];
}

void PDFViewer::paintEvent(QPaintEvent * event)
{
  QPainter painter(this);

  // If nothing is visible on screen, nothing to do
  if (!isVisible()) return;

  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  // Paint background
  painter.fillRect(geometry(), QColor("gray"));

  if (!pdfFile->isValid()) return;

  if (!pdfFile->cache) return;

  updateVisible();

  const QColor pageColor("white");

  int X, Y, W, H;
  int Xs, Ys, Ws, Hs; // Saved values

  CachedPage * cur = &pdfFile->cache[pdfFile->firstVisible];

  if (!cur->ready) return;

  if (viewMode != VM_ZOOMFACTOR) xOff = 0.0f;

  // As the variables are used for calculations, reset them to full widget dimensions.
  // (doesn´t seems to be needed... they are adjusted to something else in the code
  //  that follow...)

  s32 currentScreenVPos = 0; // Current vertical position in the screen space
  u32 firstPageInLine = pdfFile->firstVisible;

  bool firstLine = true;
  bool firstPage = true;

  const float invisibleY = yOff - floorf(yOff);

  // pp will hold all topological information required to identify the selection
  // made by the user with mouse movements
  PagePos * pp = pagePosOnScreen;
  pagePosCount = 0;

  u32 page = pdfFile->firstVisible;

  // Do the following for each line of pages
  while ((currentScreenVPos < height()) && (firstPageInLine < pdfFile->pages)) {

    float zoom;
    u32   lineWidth, lineHeight;

    zoom = lineZoomFactor(
      /* in  */ firstPageInLine,
      /* out */ lineWidth,
      /* out */ lineHeight);

    if (firstLine) {
        viewZoom = zoom;
    }

    const int zoomedMargin     = zoom * MARGIN;
    const int zoomedMarginHalf = zoomedMargin / 2;

    H = lineHeight * zoom; // Line of pages height in screen pixels

    if (firstLine) {
      Y = y() - invisibleY * H;
    }

    X = x() +
        width() / 2 -
        zoom * lineWidth / 2 +
        (zoom * xOff * lineWidth);

    // Do the following for each page in the line

    s32 column = 0;
    s32 limit = columns;
    page = firstPageInLine;

    if ((titlePages > 0) && (titlePages < columns) && (page == 0))
      limit = titlePages;

    // Do the following for each page in the line
    while ((column < limit) && (page < pdfFile->pages)) {

      cur = &pdfFile->cache[page];
      if (!cur->ready)
        break;

      H = pageH(page) * zoom;
      W = pageW(page) * zoom;

      // Paint the page backgroud rectangle, save coordinates for next loop
      painter.fillRect(QRect(Xs = X, Ys = Y, Ws = W, Hs = H), pageColor);

      #if DEBUGGING && 0
        if (firstPage) {
          qDebug("Zoom factor: %f\n", zoom);
          qDebug("Page data: Left: %d Right: %d Top: %d Bottom: %d W: %d H: %d\n",
            cur->left, cur->right, cur->top, cur-> bottom, cur->w, cur->h);
          qDebug("Page screen rectangle: X: %d Y: %d W: %d H: %d\n", X, Y, W, H);
        }
      #endif

      const bool margins = hasMargins(page);
      const bool trimmed =
        (margins && ((viewMode == VM_TRIM) || (viewMode == VM_PGTRIM)));

      float ratioX = 1.0f;
      float ratioY = 1.0f;

      if ((viewMode == VM_CUSTOMTRIM) &&
          !trimZoneSelection &&
          customTrim.initialized) {

        QRect theTrim = getTrimmingForPage(page);

        s32 w, h, th;
        float zw, zh;

        th = theTrim.height() < 2*MARGIN ? theTrim.height() + MARGIN : theTrim.height();

        painter.setClipRect(
          Xs + zoomedMarginHalf,
          Ys + zoomedMarginHalf,
          w = zoom * theTrim.width() - zoomedMargin,
          h = zoom * th - zoomedMargin);

        ratioX = w / (float)(w + zoomedMargin);
        ratioY = h / (float)(h + zoomedMargin);

        //qDebug("Ratio: %f %f (%d %d)\n", ratioX, ratioY, w, h);

        zw = ratioX * zoom;
        zh = ratioY * zoom;

        X -= zw * (theTrim.x() - cur->left) - zoomedMarginHalf;
        Y -= zh * (theTrim.y() - cur->top ) - zoomedMarginHalf;

        W = zw * cur->w;
        H = zh * cur->h;

        #if DEBUGGING && 0
          if (firstPage) {
            qDebug("My Trim: X: %d Y: %d W: %d H: %d\n",
              theTrim.x(),
              theTrim.y(),
              theTrim.width(),
              theTrim.height());
            qDebug("Clipping: X: %d, Y: %d, W: %d, H: %d\n", Xs, Ys, w, h);
          }
        #endif
      }
      else if (trimmed) {
        // If the page was trimmed, have the real one a bit smaller
        X += zoomedMarginHalf;
        Y += zoomedMarginHalf;
        W -= zoomedMargin;
        H -= zoomedMargin;

        // These changes in size have an impact on the zoom factor
        // previously used. We need to keep track of these changes
        // to help into the selection processes.
        ratioX = W / (float)(W + zoomedMargin);
        ratioY = H / (float)(H + zoomedMargin);
      }
      else if (margins) {
        // Restore the full size with empty borders
        X +=  cur->left                * zoom;
        Y +=  cur->top                 * zoom;
        W -= (cur->left + cur->right ) * zoom;
        H -= (cur->top  + cur->bottom) * zoom;
      }

      // qDebug() << "Page: " << page;
      QPixmap img = getPage(page);

      // Render real content
//      if (firstPage) {
//         qDebug("Drawing page %d: X: %d, Y: %d, W: %d, H: %d (%d)",
//                 page, X, Y, W, H, img.size());
//      }

      // Do render the page on the canvas
      painter.drawPixmap(QRect(X, Y, W, H), img);

      if (painter.hasClipping()) painter.setClipping(false);

      if (zoneSelection) {
        if (!selector) selector = new QRubberBand(QRubberBand::Rectangle, this);
        if (!dragging && firstPage && (viewMode == VM_CUSTOMTRIM) && trimZoneSelection) {

          if (customTrim.initialized) {

            s32 w, h;

            QRect theTrim = getTrimmingForPage(page);

            selector->setGeometry(
              selX = Xs + (zoom * theTrim.x()     ),
              selY = Ys + (zoom * theTrim.y()     ),
              w    =      (zoom * theTrim.width() ),
              h    =      (zoom * theTrim.height()));

            if (!selector->isVisible()) selector->show();

            selX2 = selX + w;
            selY2 = selY + h;
          }
        }
        else if (!((viewMode == VM_CUSTOMTRIM) && trimZoneSelection)) {
          if (selector && selector->isVisible()) {
            if ((selX == 0) && (selY == 0) && (selX2 == 0) && (selY2 == 0)) {
              selector->hide();
            }
          }
        }
      }
      else {
        if (selector && selector->isVisible()) selector->hide();
      }

      // For each displayed page, we keep those parameters
      // to permit the localization of the page on screen when
      // a selection is made to retrieve the text underneath
      if (pagePosCount < PAGES_ON_SCREEN_MAX) {

        pp->page    = page;
        pp->X0      = Xs;  // Page output coor
        pp->Y0      = Ys;
        pp->W0      = Ws;
        pp->H0      = Hs;
        pp->X       = X;   // inked portion of the page coor
        pp->Y       = Y;
        pp->W       = W;
        pp->H       = H;
        pp->zoom    = zoom;
        pp->ratioX  = ratioX; // margin ratio
        pp->ratioY  = ratioY;

        // if (pagePosCount == 0)
        //  printf("Copy X0:%d Y0:%d X:%d Y:%d W:%d H:%d Zoom: %6.3f page: %d\n",
        //      pp->X0, pp->Y0, pp->X, pp->Y, pp->W, pp->H, pp->zoom, pp->page + 1);

        pagePosCount++;
        pp++;
      }

      // Restore page coordinates for next loop.
      X = Xs;
      Y = Ys;
      W = Ws;
      H = Hs;

      X += preferences.horizontalPadding + (pageW(page) * zoom);
      page++; column++;

      firstPage = false;
    }

    // Prepare for next line of pages

    Y += preferences.verticalPadding + (lineHeight * zoom);

    firstPageInLine += limit;
    currentScreenVPos = Y - y();

    firstLine = false;
  }

  pdfFile->lastVisible = page;
}

void PDFViewer::rubberBanding(bool show)
{
  if (show) {
    int x, y, w, h;
    if (selX < selX2) {
      x = selX;
      w = selX2 - selX;
    }
    else {
      x = selX2;
      w = selX - selX2;
    }
    if (selY < selY2) {
      y = selY;
      h = selY2 - selY;
    }
    else {
      y = selY2;
      h = selY - selY2;
    }

    if ((x != 0) && (y != 0) && (w != 0) && (h != 0)) {
      if (!selector) selector = new QRubberBand(QRubberBand::Rectangle, this);
      selector->setGeometry(QRect(x, y, w, h));
      if (!selector->isVisible()) selector->show();
    }
    else {
      if (selector && selector->isVisible()) selector->hide();
    }
  }
  else {
    if (selector && selector->isVisible()) selector->hide();
  }
}

void PDFViewer::pageChanged()
{
  if (silent) return;
  if ((pdfFile == nullptr) || !pdfFile->isValid()) return;

  s32 page = yOff;

  if (trimZoneSelection) {
    SinglePageTrim * curr = customTrim.singles;
    while (curr && (curr->page < page)) curr = curr->next;

    singlePageTrim = curr && (curr->page == page);
  }

  resetSelection();
  sendState();
  update();
}

// Compute the maximum yoff value, taking care of the number of
// columns displayed, title pages and the screen size. Start at the end of the
// document, getting up to the point where the line of pages will be
// out of reach on screen.
//
// Pages = 8, columns = 3, title pages = 1
// last = 7 - (7 % 3) - (columns - title_pages) = 7 - 1 - 2 = 4 + 3 = 7

// Pages = 9, columns = 3, title pages = 2
// last = 8 - (8 % 3) - (columns - title_pages) = 8 - 2 - 1 = 5 + 3

// Pages = 13, columns = 4, title pages = 1
// last = 12 - (12 % 4) - (columns - title_pages) = 12 - 0 - 3 = 9
float PDFViewer::maxYOff() const
{
  float zoom, f;
  u32   lineWidth, lineHeight, h;

  if (pdfFile->pages == 0) return 0.0f;

  s32 pages = pdfFile->pages;
  s32 last  = pages - 1;

  last     -= (last % columns);
  if ((titlePages > 0) && (titlePages < columns)) {
    last -= (columns - titlePages);
    if (last < (pages - (s32)columns)) {
      last += columns;
    }
  }

  if (last < 0) last = 0;

  if (!pdfFile->cache[last].ready)
    f = last + 0.5f;
  else {
    s32 H = height();

    while (true) {
      zoom = lineZoomFactor(last, lineWidth, lineHeight);
      H   -= (h = zoom * (lineHeight + preferences.verticalPadding));

      if (H <= 0) {
        H += (preferences.verticalPadding * zoom);
        f = last + (float)(-H) / (zoom * lineHeight);
        break;
      }

      last -= columns;
      if (last < 0) {
        f = 0.0f;
        break;
      }
    }
  }
  return f;
}

// Advance the yoff position by an offset, taking into account the number
// of columns and the title_pages count. Yoff is adjusted to correspond to
// the first page offset to be disblayed on a line of pages. For examples:
//
//    columns   title_pages   offset    yoff    new_yoff
//
//       3           1          1         0        1
//       3           2          1         0        2
//       3           2          2         0        5
//       3           1         -1         3        1
void PDFViewer::adjustYOff(float offset)
{
  float newYOff = yOff;

  if (offset != 0.0f) {

    newYOff += offset;
    s32 diff = floorf(newYOff) - floorf(yOff); // Number of absolute pages of difference

    if ((diff != 0) && (newYOff >= 0.0f)) {
      // The displacement is to another line of pages
      newYOff += (float)diff * (columns - 1);
      if ((titlePages > 0) && (titlePages < columns)) {
        // There is title pages to take care of
        if (newYOff < 1.0f) {
          newYOff += (columns - titlePages);
        }
        else if (yOff < 1.0f) {
          newYOff -= (columns - titlePages);
        }
      }
    }

    //printf("diff: %d, yoff: %f new_yoff: %f\n", diff, yoff, new_yoff);

    //  float y = floorf(yoff);
    //  yoff += offset;
    //  float diff = floorf(yoff) - y; // diff is the number of "line of pages" to advance
    //  yoff = yoff + (diff * columns) - diff;
  }

  if (newYOff < 0.0f)
    newYOff = 0.0f;
  else {
    float y = maxYOff();

    if (newYOff > y) newYOff = y;
  }

  yOff = newYOff;
}

void PDFViewer::adjustFloorYOff(float offset)
{
  float newYOff = floorf(yOff);

  // From here, almost same code as adjustYOff...'

  if (offset != 0.0f) {

    newYOff += offset;
    s32 diff = newYOff - floorf(yOff);

    if ((diff != 0) && (newYOff >= 0.0f)) {
      if ((titlePages > 0) && (titlePages < columns) && (yOff < 1.0f)) {
        newYOff += ((float)diff * (columns - 1)) - (columns - titlePages);
      }
      else {
        newYOff += (float)diff * (columns - 1);
      }
    }
  }

  if (newYOff < 0.0f) {
    newYOff = 0.0f;
  }
  else {
    float max = maxYOff();

    if (newYOff > max) newYOff = max;
  }

  yOff = newYOff;
}

void PDFViewer::resetSelection(bool anyway)
{
  if (anyway || !trimZoneSelection) {
    savedX = savedY = selX = selY = selX2 = selY2 = 0;
  }
}

// Compute the vertical screen size of a line of pages
u32 PDFViewer::pxrel(u32 page) const
{
  float zoom;
  u32 lineWidth, lineHeight;

  zoom = lineZoomFactor(page, lineWidth, lineHeight);
  return lineHeight * zoom;
}

//----- Slots ------

// User requested text selection to copy to clipboard (or not if do_select is false)
void PDFViewer::textSelect(bool doSelect)
{
  zoneSelection = textSelection = doSelect;
  trimZoneSelection = false;
  resetSelection();

  if (doSelect) {
    setMouseTracking(true);
    setCursor(Qt::CrossCursor);
  }
  else {
    setMouseTracking(false);
    setCursor(Qt::ArrowCursor);
  }

  update();
}

void PDFViewer::copyToClipboard()
{
  if (clipText.size() > 0) {
    QGuiApplication::clipboard()->setText(clipText);
  }
}

void PDFViewer::trimZoneDifferent(bool diff)
{
  bool wasSimilar = customTrim.similar;
  customTrim.similar = !diff;

  if (!wasSimilar && customTrim.similar) {
    s32 page = yOff;

    if (page & 1) {
      customTrim.odd = customTrim.even;
    }
    else {
      customTrim.even = customTrim.odd;
    }
  }
  update();
}

void PDFViewer::thisPageTrim(bool thisPage)
{
  s32 page = yOff;

  singlePageTrim = thisPage;

  if (singlePageTrim) {
    QRect & trim = (page & 1) ? customTrim.even : customTrim.odd;
    addSinglePageTrim(page, trim);
  }
  else {
    removeSinglePageTrim(page);
    pageChanged();
  }
}

void PDFViewer::removeSinglePageTrim(s32 page)
{
  SinglePageTrim *prev, *curr, *next;

  prev = NULL;
  curr = customTrim.singles;
  while (curr && (curr->page < page)) {
    prev = curr;
    curr = curr->next;
  }

  if (curr && (curr->page == page)) {
    next = curr->next;
    free(curr);

    if (prev) {
      prev->next = next;
    }
    else {
      customTrim.singles = next;
    }
  }
}

void PDFViewer::addSinglePageTrim(s32 page, QRect trim)
{
  SinglePageTrim *prev, *curr, *s;

  prev = NULL;
  curr = customTrim.singles;
  while (curr && (curr->page < page)) {
    prev = curr;
    curr = curr->next;
  }

  if ((curr == NULL) || (curr->page != page)) {
    s = (SinglePageTrim *) xmalloc(sizeof(SinglePageTrim));
    s->next = curr;
    if (prev)
      prev->next = s;
    else
      customTrim.singles = s;
    curr = s;
  }

  curr->page     = page;
  curr->pageTrim = trim;
  sendState();
}

void PDFViewer::clearAllSingleTrims()
{
  clearSingles(customTrim);

  singlePageTrim = false;

  sendState();
}

// User requested trimming zone selection (or not if do_select is false).
// This is in support of the Z_MYTRIM view_mode
void PDFViewer::trimZoneSelect(bool doSelect)
{
  static float savedXOff;
  static float savedYOff;
  static u32   savedColumns;
  static bool  initialized = false;

  trimZoneSelection = (viewMode == VM_CUSTOMTRIM) && doSelect;

  if (viewMode == VM_CUSTOMTRIM) {

    if (trimZoneSelection) {

      zoneSelection = true;

      savedXOff     = xOff;
      savedYOff     = yOff;
      savedColumns  = columns;
      initialized   = true;
      columns       = 1;
      textSelection = false;

      yOff          = floorf(yOff);

      if (!customTrim.initialized) {
        s32 page = yOff;

        customTrim.initialized = true;
        customTrim.similar     = true;
        customTrim.singles     = NULL;

        // To insure that the initial trim zone rectangle will be visible on screen
        // We offset it off 25 pixels from the visible edges of the page

        if ((pdfFile->cache[page].w > 150) && (pdfFile->cache[page].h > 150)) {
          customTrim.odd = customTrim.even = QRect(
                pdfFile->cache[page].left + 25,
                pdfFile->cache[page].top  + 25,
                pdfFile->cache[page].w    - 50,
                pdfFile->cache[page].h    - 50);
        }
      }

      rubberBanding(true);
      setMouseTracking(true);
    }
    else {
      zoneSelection = false;

      if (initialized) {
        xOff    = savedXOff;
        yOff    = savedYOff;
        columns = savedColumns;
      }

      rubberBanding(false);
      setMouseTracking(false);
      setCursor(Qt::ArrowCursor);
    }

    resetSelection();
    pageChanged();
  }
}

void PDFViewer::selectPageAt(s32 X, s32 Y, bool rightDClick)
{
  u32 idx = 0;
  PagePos *pp = pagePosOnScreen;

  while (idx < pagePosCount) {
    if ((X >= pp->X0)         &&
        (Y >= pp->Y0)         &&
        (X < (pp->X + pp->W)) &&
        (Y < (pp->Y + pp->H))) {
      break;
    }
    idx++;
    pp++;
  }
  if (idx >= pagePosCount) return; // Not found

  if (rightDClick) {
    if (columns != rightDClickColumnsCount) {
      leftDClickColumnsCount = columns;
    }
  }
  setColumnCount(rightDClick ?
                 rightDClickColumnsCount :
                (zoneSelection ? 1 : leftDClickColumnsCount));
  gotoPage(pp->page);
}

void PDFViewer::gotoPage(const int page)
{
  yOff = page;
  adjustYOff(0);
  resetSelection();
  pageChanged();
}

void PDFViewer::setColumnCount(int count)
{
  if ((count >= 1) && (count <= 5)) {
    columns = count;
    resetSelection();
    pageChanged();
  }
}

void PDFViewer::setTitlePageCount(int count)
{
  if (count <= 4) {
    titlePages = count;
    resetSelection();
    pageChanged();
  }
}

void PDFViewer::up()
{
  adjustYOff(-SMALL_MOVE);
  pageChanged();
}

void PDFViewer::down()
{
  adjustYOff(SMALL_MOVE);
  pageChanged();
}

void PDFViewer::beginningOfPage()
{
  yOff = floor(yOff);
  pageChanged();
}

void PDFViewer::endOfPage()
{
  u32 page = yOff;
  s32 sh   = height() * viewZoom;

  if (pdfFile->cache[page].ready) {
    const s32 hidden = sh - height();
    float tmp = floorf(yOff) + hidden / (float) sh;
    if (tmp > yOff) {
      yOff = tmp;
    }
    adjustYOff(0);
  }
  else {
    yOff = ceilf(yOff) - 0.4f;
  }
  pageChanged();
}

void PDFViewer::top()
{
  yOff = 0.0f;
  pageChanged();
}

void PDFViewer::bottom()
{
  yOff = maxYOff();
  pageChanged();
}

void PDFViewer::pageUp()
{
  adjustFloorYOff(floorf(yOff) == yOff ? -1.0f : 0.0f);
  pageChanged();
}

void PDFViewer::pageDown()
{
  adjustFloorYOff(1.0f);
  pageChanged();
}

void PDFViewer::pageUpWithOverlap()
{
  const u32 page = yOff;
  s32 sh;
  s32 shp = sh = pxrel(page);
  if (page >= columns) {
    shp = pxrel(page - columns);
  }
  if (height() >= sh) {
    pageUp();
  }
  else {
    /* scroll up a bit less than one screen height */
    float d = (height() - 2 * MARGIN) / (float) sh;
    if (((u32) yOff) != ((u32) (yOff - d))) {
      /* scrolling over page border */
      d -= (yOff - floorf(yOff));
      yOff = floorf(yOff);
      /* ratio of prev page can be different */
      d = d * (float) sh / (float) shp;
    }
    adjustYOff(-d);
    pageChanged();
  }
}

void PDFViewer::pageDownWithOverlap()
{
  const s32 page = yOff;
  s32 sh;
  s32 shn = sh = pxrel(page);
  if (page + columns <= (u32)(pdfFile->pages - 1)) {
    shn = pxrel(page + columns);
  }
  if (height() >= sh) {
    pageDown();
  }
  else {
    /* scroll down a bit less than one screen height */
    float d = (height() - 2 * MARGIN) / (float) sh;
    if (((u32) yOff) != ((u32) (yOff + d))) {
      /* scrolling over page border */
      d -= (ceilf(yOff) - yOff);
      yOff = floorf(yOff) + columns;
      /* ratio of next page can be different */
      d = d * (float) sh / (float) shn;
    }
    adjustYOff(d);
    pageChanged();
  }
}

void PDFViewer::zoomIn()
{
  viewZoom *= 1.2f;
  viewZoom  = viewZoom < 10.0f ? viewZoom : 10.0f;
  viewMode  = VM_ZOOMFACTOR;
  pageChanged();
}

void PDFViewer::zoomOut()
{
  viewZoom *= 0.833333;
  viewZoom  = viewZoom > 0.1f ? viewZoom : 0.1f;
  viewMode  = VM_ZOOMFACTOR;
  pageChanged();
}

void PDFViewer::setViewMode(ViewMode newViewMode)
{
  viewMode = newViewMode;
  pageChanged();
}

void PDFViewer::setZoomFactor(float zoomFactor)
{
  viewMode = VM_ZOOMFACTOR;
  viewZoom = zoomFactor > 0.1f ? (zoomFactor < 10.0f ? zoomFactor : 10.0f) : 0.1f;
  pageChanged();
}

void PDFViewer::refreshView()
{
  update();
}
