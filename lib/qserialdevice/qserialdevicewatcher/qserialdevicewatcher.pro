#
#PROJECT        = Serial Device Liblary
TEMPLATE        = lib

CONFIG          += staticlib
#CONFIG          += dll
CONFIG          -= debug_and_release debug
QT              -= gui

OBJECTS_DIR     = ../build/lib/qserialdevicewatcher/obj
MOC_DIR         = ../build/lib/qserialdevicewatcher/moc

include(qserialdevicewatcher.pri)

unix {
    ~macx:LIBS  += -ludev
    macx:LIBS   += -framework IOKit
}

#TODO: here in future replace
#contains( CONFIG, staticlib ) {
#    win32:DEFINES += QSERIALDEVICE_STATIC
#}

#TODO: here in future replace
contains( CONFIG, dll ) {
    win32:DEFINES += QSERIALDEVICE_SHARED
}

DESTDIR         = ../build/lib/qserialdevicewatcher/release
TARGET          = qserialdevicewatcher

VERSION         = 0.2.0

win32:RC_FILE   += version.rc
