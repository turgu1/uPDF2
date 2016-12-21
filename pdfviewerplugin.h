#ifndef PDFVIEWERPLUGIN_H
#define PDFVIEWERPLUGIN_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QIcon>
#include <QDesignerCustomWidgetInterface>

class PDFViewerPlugin : public QObject, public QDesignerCustomWidgetInterface
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
  Q_INTERFACES(QDesignerCustomWidgetInterface)

  public:
    explicit  PDFViewerPlugin(QObject * parent = 0);
    QWidget * createWidget(QWidget * parent) Q_DECL_OVERRIDE;
    void      initialize(QDesignerFormEditorInterface * core) Q_DECL_OVERRIDE;

    bool      isInitialized() const Q_DECL_OVERRIDE;
    QString   name()          const Q_DECL_OVERRIDE;
    QString   group()         const Q_DECL_OVERRIDE;
    QIcon     icon()          const Q_DECL_OVERRIDE;
    QString   toolTip()       const Q_DECL_OVERRIDE;
    QString   whatsThis()     const Q_DECL_OVERRIDE;
    bool      isContainer()   const Q_DECL_OVERRIDE;
    QString   domXml()        const Q_DECL_OVERRIDE;
    QString   includeFile()   const Q_DECL_OVERRIDE;

  protected:

  signals:

  public slots:

  private:
    bool initialized;
};

#endif // PDFVIEWERPLUGIN_H
