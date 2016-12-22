#include "pdfviewer.h"

#include <QString>
#include <QRect>
#include <QSizePolicy>
#include <QVBoxLayout>
#include <QPainter>

PDFViewer::PDFViewer(QWidget * parent) : QWidget(parent),
  viewMode(VM_PAGE),
  viewZoom(0.5f),
  xOff(0.0f),
  yOff(0.0f),
  columns(1),
  titlePages(0),
  cachedSize(7 * 1024 * 1024)
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  update();
  updateGeometry();

  // Initialize the uncompressed images cache
  srand(time(NULL));

  for (u32 i = 0; i < CACHE_MAX; i++) {
    cache[i]      = (u8 *) xcalloc(cachedSize, 1);
    cachedPage[i] = USHRT_MAX;
    pix[i]        = QPixmap();
  }
}

void PDFViewer::setPDFFile(PDFFile * f)
{
  pdfFile = f;

  connect(f, &PDFFile::fileIsValid, this, &PDFViewer::validFilePresent);
  update();
}

void PDFViewer::validFilePresent()
{
  update();
}

void PDFViewer::mouseMoveEvent(QMouseEvent * event)
{
  QWidget::mouseMoveEvent(event);
}

void PDFViewer::mousePressEvent(QMouseEvent * event)
{
  QWidget::mousePressEvent(event);
}

u32 PDFViewer::pageH(u32 page) const
{
  if (!pdfFile->cache[page].ready) page = 0;

  s32 h;

  if (viewMode == VM_TRIM || viewMode == VM_PGTRIM) {
    h = pdfFile->cache[page].h;
  }
  else if (viewMode == VM_CUSTOMTRIM && !trim_zone_selection) {
    if (my_trim.initialized) {
      const trim_struct * the_trim = get_trimming_for_page(page);
      h = the_trim->H;
    }
    else {
      return pdfFile->cache[page].h;
    }
  }
  else { // VM_PAGE || VM_WIDTH || VM_ZOOMFACTOR || (VM_CUSTOMTRIM && trim_zone_selection)
    return  pdfFile->cache[page].h +
        pdfFile->cache[page].top   +
        pdfFile->cache[page].bottom;
  }

  return h < 2*MARGIN ? h + MARGIN : h;
}

u32 PDFViewer::pageW(u32 page) const
{
  if (!pdfFile->cache[page].ready) page = 0;

  if (viewMode == VM_TRIM || viewMode == VM_PGTRIM) {
    return pdfFile->cache[page].w;
  }
  else if (viewMode == VM_CUSTOMTRIM && !trim_zone_selection) {
    if (my_trim.initialized) {
      const trim_struct * the_trim = get_trimming_for_page(page);
      return  the_trim->W;
    }
    else {
      return pdfFile->cache[page].w;
    }
  }
  else { // VM__PAGE || VM__WIDTH || VM__ZOOMFACTOR || (VM_CUSTOMTRIM && trim_zone_selection)
    return pdfFile->cache[page].w +
        pdfFile->cache[page].left  +
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

  // if (view_mode == Z_TRIM || view_mode == Z_PGTRIM || (view_mode == Z_MYTRIM && !trim_zone_selection))
  //   for (i = page; i < limit; i++) {
  //     if (file->cache[i].ready && (file->cache[i].h > fh))
  //       fh = file->cache[i].h;
  //   }
  // else
  //   for (i = page; i < limit; i++) {
  //     if (file->cache[i].ready) {
  //       u32 h = file->cache[i].h +
  //           file->cache[i].top   +
  //           file->cache[i].bottom;
  //       if (h > fh)
  //         fh = h;
  //     }
  //   }

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

  if (limit > file->pages) limit = pdfFile->pages;

  for (i = page; i < (page + columns); i++) {
    fw += pageW(i < limit ? i : 0);
  }

  // Add the margins between columns
  return fw + (columns - 1) * MARGINHALF;
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
      zoomFactor = (float)width / lineWidth;
      break;
    case VM_PAGE:
    case VM_PGTRIM:
    case VM_CUSTOMTRIM:
      if (((float)lineWidth / lineHeight) > ((float)width / height)) {
        zoomFactor = (float)width / lineWidth;
      }
      else {
        zoomFactor = (float)height / lineHeight;
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
QPixmap PDFViewer::doCache(const u32 page)
{
  u32 i;
  for (i = 0; i < CACHE_MAX; i++) {
    if (cachedPage[i] == page) return pix[i]; // Already there
  }

  CachedPage * const cur = &pdfFile->cache[page];

  // Adjust cache to be big enough if required
  if (cur->uncompressed > cachedSize) {
    cachedSize = cur->uncompressed;

    for (i = 0; i < CACHE_MAX; i++) {
      cache[i] = (u8 *) realloc(cache[i], cachedSize);
    }
  }

  // Be safe
  if (!cur->ready) return QPixmap();

  // Insert it in the cache. Pick the slot at random.
  const u32 dst = rand() % CACHE_MAX;

  lzo_uint dstSize = cachedSize;
  const int ret = lzo1x_decompress(cur->data,
          cur->size,
          cache[dst],
          &dstSize,
          NULL);
  if (ret != LZO_E_OK || dstSize != cur->uncompressed) {
    die(QString(tr("Error decompressing")));
  }

  cachedPage[dst] = page;

  // Create the Pixmap
  QImage img(cache[dst], cur->w, cur->h, QImage::Format_RGB32);

  if (!pix[dst].convertFromImage(img)) {
    die(QString(tr("QPixmap::loadFromData failed")));
  }

  return pix[dst];
}

void PDFViewer::paintEvent(QPaintEvent * event)
{
  QPainter painter(this);

  // If nothing is visible on screen, nothing to do
  if (!visible) return;

  // Paint background
  painter.fillRect(geometry(), QColor("gray"));

  if (!pdfFile->isValid()) return;

#if 0
  QPixmap px = doCache(0);

  painter.drawPixmap(0, 0, px);
  debug("Ready to show...");

#else
  if (!pdfFile->cache) return;

  //?computeScreenSize();
  updateVisible();

  const QColor pageColor("white");

  int X, Y, W, H;
  int Xs, Ys, Ws, Hs; // Saved values

  //?fl_clip_box(screenX, screenY, screenWidth, screenHeight, X, Y, W, H);

  //?if (W == 0 || H == 0)
  //?  return;

  //--> fl_overlay_clear(); rubber banding...

  struct cachedPage *cur = &pdfFile->cache[pdfFile->firstVisible];

  if (!cur->ready) return;

  //? insure that nothing will be drawned outside the clipped area
  //?fl_push_clip(X, Y, W, H);

  if (viewMode != VM_ZOOMFACTOR) xOff = 0.0f;

  // As the variables are used for calculations, reset them to full widget dimensions.
  // (doesn´t seems to be needed... they are adjusted to something else in the code
  //  that follow...)

  s32 currentScreenVPos = 0; // Current vertical position in the screen space
  u32 firstPageInLine = pdfFile->firstVisible;

  bool firstLine = true;
  bool firstPage = true;

  const float invisibleY = yOff - floorf(yOff);

  // pp will acquire all topological information required to identify the selection
  // made by the user with mouse movements
  PagePos * pp = pagePosOnScreen;
  pagePosCount = 0;

  u32 page = pdfFile->firstVisible;

  // Do the following for each line of pages
  while ((currentScreenVPos < height) && (firstPageInLine < pdfFile->pages)) {

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
      Y = y - invisibleY * H;
    }

    X = x +
        width / 2 -
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
      //?fl_rectf(Xs = X, Ys = Y, Ws = W, Hs = H, pageColor);
      painter.fillRect(QRect(Xs = X, Ys = Y, Ws = W, Hs = H), pageColor);

      #if DEBUGGING && 0
        if (firstPage) {
          debug("Zoom factor: %f\n", zoom);
          debug("Page data: Left: %d Right: %d Top: %d Bottom: %d W: %d H: %d\n",
            cur->left, cur->right, cur->top, cur-> bottom, cur->w, cur->h);
          debug("Page screen rectangle: X: %d Y: %d W: %d H: %d\n", X, Y, W, H);
        }
      #endif

      const bool margins = hasMargins(page);
      const bool trimmed =
        (margins && ((viewMode == VM_TRIM) || (viewMode == VM_PGTRIM)));

      float ratioX = 1.0f;
      float ratioY = 1.0f;

      if ((viewMode == VM_CUSTOMTRIM) &&
          !trim_zone_selection &&
          my_trim.initialized) {

        const trim_struct * the_trim = get_trimming_for_page(page);

        s32 w, h, th;
        float zw, zh;

        th = the_trim->H < 2*MARGIN ? the_trim->H + MARGIN : the_trim->H;

        fl_push_clip(
          Xs + zoomedMarginHalf,
          Ys + zoomedMarginHalf,
          w = zoom * the_trim->W - zoomedMargin,
          h = zoom * th - zoomedMargin);

        ratioX = w / (float)(w + zoomedMargin);
        ratioY = h / (float)(h + zoomedMargin);

        //debug("Ratio: %f %f (%d %d)\n", ratioX, ratioY, w, h);

        zw = ratioX * zoom;
        zh = ratioY * zoom;

        X -= zw * (the_trim->X - cur->left) - zoomedMarginHalf;
        Y -= zh * (the_trim->Y - cur->top) - zoomedMarginHalf;

        W = zw * cur->w;
        H = zh * cur->h;

        #if DEBUGGING && 0
          if (firstPage) {
            debug("My Trim: X: %d Y: %d W: %d H: %d\n",
              the_trim->X,
              the_trim->Y,
              the_trim->W,
              the_trim->H);
            debug("Clipping: X: %d, Y: %d, W: %d, H: %d\n", Xs, Ys, w, h);
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

      // Render real content
      #if DEBUGGING && 0
        if (firstPage) {
          debug("Drawing page %d: X: %d, Y: %d, W: %d, H: %d\n", page, X, Y, W, H);
        }
      #endif

      content(page, X, Y, W, H);

//?      if (viewMode == VM_CUSTOMTRIM &&
//?          !trim_zone_selection &&
//?          my_trim.initialized) {
//?        fl_pop_clip();
//?      }

      if (firstPage && (view_mode == VM_CUSTOMTRIM) && trim_zone_selection) {

        if (my_trim.initialized) {

          s32 w, h;

          const trim_struct * the_trim = get_trimming_for_page(page);

          fl_overlay_rect(
            selx = Xs + (zoom * the_trim->X),
            sely = Ys + (zoom * the_trim->Y),
            w    = (zoom * the_trim->W),
            h    = (zoom * the_trim->H));

          selx2 = selx + w;
          sely2 = sely + h;

          //fl_message("selx: %d, sely: %d, w: %d, h: %d", selx, sely, w, h);
        }
      }

      // For each displayed page, we keep those parameters
      // to permit the localization of the page on screen when
      // a selection is made to retrieve the text underneath
      if (pagePosCount < PAGES_ON_SCREEN_MAX) {

        pp->page    = page;
        pp->X0      = Xs;
        pp->Y0      = Ys;
        pp->W0      = Ws;
        pp->H0      = Hs;
        pp->X       = X;
        pp->Y       = Y;
        pp->W       = W;
        pp->H       = H;
        pp->zoom    = zoom;
        pp->ratioX  = ratioX;
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

      X += zoomedMarginHalf + (pageW(page) * zoom);
      page++; column++;

      firstPage = false;
    }

    // Prepare for next line of pages

    Y += (lineHeight * zoom) + zoomedMarginHalf;

    firstPageInLine += limit;
    currentScreenVPos = Y - y;

    firstLine = false;
  }

  pdfFile->lastVisible = page;

  //?fl_pop_clip();
#endif
}

QSize PDFViewer::sizeHint() const
{
  return QSize(0,0);
}

