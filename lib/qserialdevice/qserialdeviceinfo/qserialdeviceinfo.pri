#

HEADERS         += $$PWD/serialdeviceinfo.h  \
                  $$PWD/serialdeviceinfo_p.h

SOURCES         += $$PWD/serialdeviceinfo.cpp

win32 {
    include(../qserialdeviceinfo/winregkeyreader.pri)
    SOURCES += $$PWD/serialdeviceinfo_win.cpp
}

unix {
    include(../qserialdevice/ttylocker.pri)
    SOURCES += $$PWD/serialdeviceinfo_unix.cpp
    LIBS        += -ludev
}

INCLUDEPATH += $$PWD
