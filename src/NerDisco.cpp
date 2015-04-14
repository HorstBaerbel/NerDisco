#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("NerDisco");
    app.setOrganizationName("HorstBaerbel Inc.");
    MainWindow mainwindow;
    mainwindow.show();
    return app.exec();
}
