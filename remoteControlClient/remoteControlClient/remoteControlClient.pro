QT       += core gui
QT       += serialport   #加入串口模块
QT       += network  #添加网络支持的模块
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    dataprocessing.cpp \
    main.cpp \
    mainwidget.cpp \
    udpThread.cpp

HEADERS += \
    dataprocessing.h \
    mainwidget.h \
    udpThread.h

FORMS += \
    mainwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
