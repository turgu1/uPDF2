#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setWindowIcon(QIcon(":/icons/img/updf-512x512.png"));
    a.setApplicationName("uPDF");
    a.setOrganizationName("GTHomeComputing");
    a.setApplicationDisplayName("uPDF");
    a.setApplicationVersion("V1.0");
    a.setStyle(QStyleFactory::create("fusion"));

    MainWindow w;
    w.setWindowTitle("uPDF");
    w.show();

    return a.exec();
}
