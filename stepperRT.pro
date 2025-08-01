QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += $$PWD/inc \
               $$PWD/inc/ui \
               $$PWD/inc/motor \
               $$PWD/inc/serial \
               $$PWD/inc/external

SOURCES += \
    main.cpp \
    $$files($$PWD/src/ui/*.cpp) \
    $$files($$PWD/src/motor/*.cpp) \
    $$files($$PWD/src/serial/*.cpp) \
    $$files($$PWD/src/external/*.cpp)

HEADERS += \
    $$files($$PWD/inc/ui/*.h) \
    $$files($$PWD/inc/motor/*.h) \
    $$files($$PWD/inc/serial/*.h) \
    $$files($$PWD/inc/external/*.h)

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


RESOURCES += \
    images.qrc

QMAKE_CXXFLAGS += -Wno-clazy-connect-by-name
