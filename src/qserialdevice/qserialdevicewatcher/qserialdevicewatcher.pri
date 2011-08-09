#

HEADERS         += $$PWD/serialdevicewatcher.h  \
                  $$PWD/serialdevicewatcher_p.h

SOURCES         += $$PWD/serialdevicewatcher.cpp

win32 {
    include(../qserialdeviceinfo/winregkeyreader.pri)
    SOURCES     += $$PWD/serialdevicewatcher_p_win.cpp
}

macx {
    SOURCES	+= $$PWD/serialdevicewatcher_p_mac.cpp
}

unix {
    !macx:SOURCES += $$PWD/serialdevicewatcher_p_unix.cpp
}

INCLUDEPATH += $$PWD


