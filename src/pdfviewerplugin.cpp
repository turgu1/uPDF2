#include "pdfviewerplugin.h"
#include "pdfviewer.h"

// Currently not in use...

PDFViewerPlugin::PDFViewerPlugin(QObject * parent) : QObject(parent), initialized(false)
{

}

void PDFViewerPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
  if (initialized) return;

  initialized = true;
}

bool PDFViewerPlugin::isInitialized() const
{
  return initialized;
}

QWidget * PDFViewerPlugin::createWidget(QWidget *parent)
{
  return new PDFViewer(parent);
}

QString PDFViewerPlugin::name() const
{
  return "PDFViewer";
}

QString PDFViewerPlugin::group() const
{
  return "Display Widgets";
}

QIcon PDFViewerPlugin::icon() const
{
    return QIcon();
}

QString PDFViewerPlugin::toolTip() const
{
    return "";
}

QString PDFViewerPlugin::whatsThis() const
{
    return "";
}

bool PDFViewerPlugin::isContainer() const
{
    return false;
}

QString PDFViewerPlugin::domXml() const
{
    return "<ui language=\"c++\">\n"
           " <widget class=\"PDFViewer\" name=\"pdfViewer\">\n"
           "  <property name=\"geometry\">\n"
           "   <rect>\n"
           "    <x>0</x>\n"
           "    <y>0</y>\n"
           "    <width>100</width>\n"
           "    <height>100</height>\n"
           "   </rect>\n"
           "  </property>\n"
           "  <property name=\"toolTip\" >\n"
           "   <string>PDF Document Viewer</string>\n"
           "  </property>\n"
           "  <property name=\"whatsThis\" >\n"
           "   <string>The PDF Document viewer allow for the display of PDF documents using multi-columns in a single frame.</string>\n"
           "  </property>\n"
           " </widget>\n"
           "</ui>\n";
}

QString PDFViewerPlugin::includeFile() const
{
    return "pdfviewer.h";
}
