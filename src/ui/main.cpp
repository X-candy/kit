/******************************************************************************

  Copyright (C), 2018, Vision Soft. Co., Ltd.

 ******************************************************************************
  File Name     : hi_md.h
  Author        : Hisilicon multimedia software (IVE) group
  Created       : 2018/01/09
  Author		: backup4mark@gmail.com
******************************************************************************/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include "log.h"
#include "ui_mainwindow.h"
#include "hi_mpi.h"

static void *HISI_Init_Thread(void *arg)
{
    int ret;

    //初始化海思资源
    ret = hi_sys_init();
    if (ret)
        pr_err("hisi sys init failed");

    ret = hi_vi_init();
    if (ret)
        pr_err("hisi vi init failed");

    ret = hi_vo_init();
    if (ret)
        pr_err("hisi vo init failed");

    ret = vo_bind_vi();
    if (ret)
        pr_err("hisi bind vi vo failed");

    fb_alloc();

    //释放信号量锁
    sem_post((sem_t *)arg);

    return NULL;
}

int HISI_Init(void)
{
    int err;
    pthread_t tid;
    sem_t semDetec;

    if (sem_init(&semDetec, 0, 0) < 0) {
        pr_err("sem_init failed!");
        return -1;
    }

    err = pthread_create(&tid, NULL, HISI_Init_Thread, &semDetec);
    if (0 != err) {
        pr_err("pthread_create failed!");
        return -1;
    }

    sem_wait(&semDetec);
    sem_destroy(&semDetec);

    return 0;
}

int main(int argc, char *argv[])
{
    pr_build_info("Kit");

    //初始化海思平台
    if (HISI_Init() < 0) {
        pr_err("HISI_Init failed!");
        exit(1);
    }

    //设置Qt界面
    QApplication a(argc, argv);

    QMainWindow w;
    Ui::MainWindow ui;
    ui.setupUi(&w);
    w.show();

    return a.exec();
}
