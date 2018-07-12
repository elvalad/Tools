#include "orbbec_main_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle("AstraPortal");
    w.setMinimumSize(1280, 640);
    w.showMaximized();

    return a.exec();
}
