#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include "updf.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    const struct option opts[] = {
      { "details", 0, NULL, 'd' },
      { "help",    0, NULL, 'h' },
      { "version", 0, NULL, 'v' },
      { NULL,      0, NULL,  0  }
    };

    while (1) {
      const int c = getopt_long(argc, argv, "dhv", opts, NULL);
      if (c == -1)
        break;

      switch (c) {
        case 'd':
          details++;
        break;
        case 'v':
          printf("%s\n", UPDF_VERSION);
          return 0;
        break;
        case 'h':
        default:
          printf("Usage: %s [options] file.pdf\n\n"
            "   -d --details   Print RAM, timing details (use twice for more)\n"
            "   -h --help      This help\n"
            "   -v --version   Print version\n",
            argv[0]);
          return 0;
        break;
      }
    }

    filenameAtStartup = optind < argc ? argv[optind] : "";

    a.setWindowIcon(QIcon(":/icons/img/updf-512x512.png"));
    a.setApplicationName("updf");
    a.setOrganizationName("updf");
    a.setApplicationDisplayName("uPDF");
    a.setApplicationVersion("V1.0");
    a.setStyle(QStyleFactory::create("fusion"));

    a.setStyleSheet("QWidget {font: 13px;}");

    MainWindow w;
    w.setWindowTitle("uPDF");
    w.show();

    return a.exec();
}
