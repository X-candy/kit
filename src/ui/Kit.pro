#-------------------------------------------------------------------------------

#
# Project created by QtCreator 2017-12-14
#
#-------------------------------------------------------------------------------

TEMPLATE    =   app

QT          +=  core gui

TARGET      =   kit

DEPENDPATH  +=  .

INCLUDEPATH +=  . ../include/ ../include/mpp

# 链接libmedia.a, dl库用于打开动态库
LIBS        +=  -ldl -L../../lib -lmpi -ljpeg -lupvqe -ldnvqe -lVoiceEngine

#-------------------------------------------------------------------------------

SOURCES     +=  main.cpp hi_mpi.c

FORMS       +=  mainwindow.ui

RESOURCES   +=  res.qrc

#RC_FILE     =   myicon.rc

#-------------------------------------------------------------------------------

