QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++0x

TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    ros.cpp

HEADERS += \
    mainwindow.h \
    ros.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


## OpenCv
INCLUDEPATH += /usr/local/include \
               /usr/local/include/opencv \
               /usr/local/include/opencv2

LIBS += -L/usr/local/lib/ -lopencv_highgui
LIBS += -L/usr/local/lib/ -lopencv_core
LIBS += -L/usr/local/lib/ -lopencv_imgproc
LIBS += -L/usr/local/lib/ -lopencv_imgcodecs
LIBS += -L/usr/local/lib/ -lopencv_ml
LIBS += -L/usr/local/lib/ -lopencv_videoio


## ROS

INCLUDEPATH += -I /opt/ros/kinetic/include
DEPENDPATH += /opt/ros/kinetic/include

#--add ros libs
#unix:!macx: LIBS += -L /opt/ros/kinetic/lib/ -lroscpp
LIBS += -L /opt/ros/kinetic/lib/ -lroscpp
LIBS += -L /opt/ros/kinetic/lib/ -lroslib
LIBS += -L /opt/ros/kinetic/lib/ -lpthread
LIBS += -L /opt/ros/kinetic/lib/ -lroscpp_serialization
LIBS += -L /opt/ros/kinetic/lib/ -lrostime
LIBS += -L /opt/ros/kinetic/lib/ -lrosconsole
LIBS += -L /opt/ros/kinetic/lib/ -lrosconsole_log4cxx
LIBS += -L /opt/ros/kinetic/lib/ -lrosconsole_backend_interface
LIBS += -L /opt/ros/kinetic/lib/ -lxmlrpcpp
