#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(imchenwen);
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    
    return a.exec();
}
