QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++14 console

INCLUDEPATH += ../../opencv/windows/vc16/debug/include

LIBS += -L"../../opencv/windows/vc16/debug/x64/lib" -lopencv_core451d -lopencv_tracking451d -lopencv_video451d -lopencv_videoio451d -lopencv_imgcodecs451d -lopencv_imgproc451d

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    ../../createrectdialog.cpp \
    ../../main.cpp \
    ../../mainwindow.cpp \
    ../../startupdialog.cpp

HEADERS += \
    ../../createrectdialog.h \
    ../../mainwindow.h \
    ../../startupdialog.h

FORMS += \
    ../../createrectdialog.ui \
    ../../mainwindow.ui \
    ../../startupdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    ../../deep_tag.qrc

DISTFILES += \
    ../../left_arrow.png \
    ../../shutter.wav
