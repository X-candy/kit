#include "ui_mainwindow.h"

#include "mpi/hi_comm_sys.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 测试加入海思图像处理库编译是否通过
    MPP_SYS_CONF_S stSysConf = { 0 };

    QMainWindow w;
    Ui::MainWindow ui;
    ui.setupUi(&w);
    w.show();

    return a.exec();
}
