#include "ui_mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QMainWindow w;
    Ui::MainWindow ui;
    ui.setupUi(&w);
    w.show();

    return a.exec();
}
