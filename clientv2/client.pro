win32: LIBS += -lws2_32

QT += core gui websockets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    base64.cpp \
    clientwebsocket.cpp \
    imagebox.cpp \
    main.cpp \
    mainwindow.cpp \
    msgbox.cpp \
    qaesencryption.cpp

HEADERS += \
    base64.h \
    clientwebsocket.h \
    header.h \
    imagebox.h \
    mainwindow.h \
    msgbox.h \
    qaesencryption.h

FORMS += \
    imagebox.ui \
    mainwindow.ui \
    msgbox.ui

TRANSLATIONS +=
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
win32: target.path = $$OUT_PWD/build
!isEmpty(target.path): INSTALLS += target
