#
#PROJECT         = Serial Device Liblary
TEMPLATE        = lib

CONFIG          += staticlib
#CONFIG          += dll
CONFIG          -= debug_and_release debug
QT              -= gui

OBJECTS_DIR     = ../build/lib/qserialdeviceinfo/obj
MOC_DIR         = ../build/lib/qserialdeviceinfo/moc

include(qserialdeviceinfo.pri)

#TODO: here in future replace
#contains( CONFIG, staticlib ) {
#    win32:DEFINES += QSERIALDEVICE_STATIC
#}

#TODO: here in future replace
contains( CONFIG, dll ) {
    win32:DEFINES += QSERIALDEVICE_SHARED
}

DESTDIR         = ../build/lib/qserialdeviceinfo/release
TARGET          = qserialdeviceinfo

VERSION         = 0.2.0

win32:RC_FILE   += version.rc
