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

#include <QSettings>
#include <QString>
#include <QRect>
#include <QFileInfo>

#include "config.h"

#define CONFIG_VERSION "1.0"

FileViewParameters *fileViewParameters = NULL;

void clearFileViewParameters()
{
  FileViewParameters * curr = fileViewParameters;
  FileViewParameters * next;

  while (curr) {
    next = curr->next;
    clearSingles(curr->customTrim);
    delete curr;
    curr = next;
  }

  fileViewParameters = NULL;
}

void saveToConfig(FileViewParameters & params)
{
  FileViewParameters * prev = NULL;
  FileViewParameters * rf   = fileViewParameters;

  while ((rf != NULL) && (rf->filename.compare(params.filename) != 0)) {
    prev = rf;
    rf   = rf->next;
  }

  if (rf) {
    clearSingles(rf->customTrim);
    FileViewParameters * next = rf->next;

    *rf = params; // This will override rf->next...
    rf->next = next;

    copySingles(params.customTrim, rf->customTrim);

    // prev is NULL if it's already the first entry in the list
    // if not, we put this file as the first in the list
    if (prev) {
      prev->next          = rf->next;
      rf->next            = fileViewParameters;
      fileViewParameters  = rf;
    }
  }
  else {
    rf = new FileViewParameters;

    *rf = params;
    copySingles(params.customTrim, rf->customTrim);

    rf->next = fileViewParameters;
    fileViewParameters  = rf;
  }
}

void loadConfig()
{
  QSettings cfg;

  QString version = cfg.value("Version", "0.0").toString();

  if (version != CONFIG_VERSION) return;

  cfg.beginGroup("preferences");

  preferences.fullScreenAtStartup    = cfg.value("fullScreenAtStartup",             false).toBool();
  preferences.hideControlsAtStartup  = cfg.value("hideControlsAtStartup",           false).toBool();
  preferences.viewClipboardSelection = cfg.value("viewClipboardSelection",          false).toBool();
  preferences.keepRecent             = cfg.value("keepRecent",                       true).toBool();
  preferences.recentGeometry         = cfg.value("recentGeometry",                   true).toBool();
  preferences.showLoadMetrics        = cfg.value("showLoadMetrics",                 false).toBool();
  preferences.logTrace               = cfg.value("logTrace",                        false).toBool();
  preferences.logFilename            = cfg.value("logFilename",           "/tmp/updf.log").toString();

  cfg.endGroup();

  int cnt = cfg.beginReadArray("fileViewParameters");

  FileViewParameters *prev = NULL;

  for (int i = 0; i < cnt; i++) {

    cfg.setArrayIndex(i);

    QString filename = cfg.value("filename", "").toString();
    QFileInfo fi(filename);

    if (!fi.exists()) continue;

    FileViewParameters *rf = new FileViewParameters;

    rf->filename           = cfg.value("filename"      ,                  "None").toString();
    rf->columns            = cfg.value("columns"       ,                       1).toInt();
    rf->titlePageCount     = cfg.value("titlePageCount",                       0).toInt();
    rf->xOff               = cfg.value("xOff"          ,                    0.0f).toFloat();
    rf->yOff               = cfg.value("yOff"          ,                    0.0f).toFloat();
    rf->viewZoom           = cfg.value("viewZoom"      ,                    0.5f).toFloat();
    rf->viewMode  = ViewMode(cfg.value("viewMode"      ,                 VM_PAGE).toInt());
    rf->winGeometry        = cfg.value("winGeometry"   , QRect(50, 50, 600, 600)).toRect();
    rf->fullscreen         = cfg.value("fullscreen"    ,                   false).toBool();

    cfg.beginGroup("customTrim");

    rf->customTrim.initialized = cfg.value("initialized",             false).toBool();
    rf->customTrim.similar     = cfg.value("similar"    ,             false).toBool();
    rf->customTrim.odd         = cfg.value("odd"        , QRect(0, 0, 0, 0)).toRect();
    rf->customTrim.even        = cfg.value("even"       , QRect(0, 0, 0, 0)).toRect();

    rf->customTrim.singles = NULL;

    SinglePageTrim * sPrev = NULL,
                   * sCurr = NULL;

    int sCnt = cfg.beginReadArray("singles");

    for (int j = 0; j < sCnt; j++) {
      cfg.setArrayIndex(j);
      sCurr = (SinglePageTrim *) xmalloc(sizeof(SinglePageTrim));

      sCurr->page     = cfg.value("page",                     0).toInt();
      sCurr->pageTrim = cfg.value("pageTrim", QRect(0, 0, 0, 0)).toRect();

      if (sPrev == NULL) {
        rf->customTrim.singles = sCurr;
      }
      else {
        sPrev->next = sCurr;
      }
      sPrev = sCurr;
      sCurr->next = NULL;
    }

    cfg.endArray();
    cfg.endGroup();

    if (prev == NULL) {
      prev = rf;
      fileViewParameters = rf;
    }
    else {
      prev->next = rf;
      prev = rf;
    }
    rf->next = NULL;
  }

  cfg.endArray();
}

void saveConfig()
{
  QSettings cfg;

  cfg.clear();

  cfg.setValue("Version", CONFIG_VERSION);

  cfg.beginGroup("preferences");

  cfg.setValue("fullScreenAtStartup",    preferences.fullScreenAtStartup   );
  cfg.setValue("hideControlsAtStartup",  preferences.hideControlsAtStartup );
  cfg.setValue("viewClipboardSelection", preferences.viewClipboardSelection);
  cfg.setValue("keepRecent",             preferences.keepRecent            );
  cfg.setValue("recentGeometry",         preferences.recentGeometry        );
  cfg.setValue("showLoadMetrics",        preferences.showLoadMetrics       );
  cfg.setValue("logTrace",               preferences.logTrace              );
  cfg.setValue("logFilename",            preferences.logFilename           );

  cfg.endGroup();

  cfg.beginWriteArray("fileViewParameters");

  FileViewParameters *rf = fileViewParameters;

  int cnt = 0;
  while ((rf != NULL) && (cnt < 30)) {
    cfg.setArrayIndex(cnt);
    cfg.setValue("filename",       rf->filename      );
    cfg.setValue("columns",        rf->columns       );
    cfg.setValue("titlePageCount", rf->titlePageCount);
    cfg.setValue("xOff",           rf->xOff          );
    cfg.setValue("yOff",           rf->yOff          );
    cfg.setValue("viewZoom",       rf->viewZoom      );
    cfg.setValue("viewMode",       rf->viewMode      );
    cfg.setValue("winGeometry",    rf->winGeometry   );
    cfg.setValue("fullscreen",     rf->fullscreen    );

    cfg.beginGroup("customTrim");

    cfg.setValue("initialized", rf->customTrim.initialized);
    cfg.setValue("similar",     rf->customTrim.similar);

    if (rf->customTrim.initialized) {
      cfg.setValue("odd",  rf->customTrim.odd);
      cfg.setValue("even", rf->customTrim.even);
    }

    if (rf->customTrim.singles) {
      cfg.beginWriteArray("singles");
      SinglePageTrim * curr = rf->customTrim.singles;

      int idx = 0;
      while (curr) {
        cfg.setArrayIndex(idx);
        cfg.setValue("page", curr->page);
        cfg.setValue("pageTrim", curr->pageTrim);

        curr = curr->next;
        idx += 1;
      }

      cfg.endArray();
    }

    cfg.endGroup();

    rf = rf->next;
    cnt += 1;
  }

  cfg.endArray();
}
