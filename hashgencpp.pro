QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += gnu++14

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

TARGET = hashgen

INCLUDEPATH += include/

SOURCES += \
    src/BinaryCheck.cpp \
    src/HashgenIniConfig.cpp \
    src/ProcessWorker.cpp \
    src/ProgressDialog.cpp \
    src/main.cpp \
    src/appwindow.cpp \
    src/aboutdialog.cpp

HEADERS += \
    include/BinaryCheck.h \
    include/HashgenIniConfig.h \
    include/ProcessWorker.h \
    include/ProgressDialog.h \
    include/appwindow.h \
    include/aboutdialog.h

FORMS += \
    res/aboutdialog.ui \
    res/appwindow.ui

TRANSLATIONS += \
    hashgencpp_pt_BR.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res/resources.qrc
