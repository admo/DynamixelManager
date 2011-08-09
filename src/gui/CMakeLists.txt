project (DynamixelManager)
cmake_minimum_required (VERSION 2.6.0)

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
include(${QT_USE_FILE})

find_package (Boost 1.36.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${CMAKE_CURRENT_BINARY_DIR}
	${QT_INCLUDES}
	${QSERIALDEVICE_INCLUDE_DIRS}
	${TRILOGGER_INCLUDE_DIRS}
)

set (APP_TARGET "DynamixelManager")

set(QT_DEFINITIONS
    -DUNICODE
    -DQT_NO_DEBUG
    -DQT_THREAD_SUPPORT
)
add_definitions (${QT_DEFINITIONS})

# Applicalion sources files.
set (APP_SRCS
	dynamixelbus.cpp
	searchservosdialog.cpp
	selectserialport.cpp
	dynamixelmanager.cpp
	main.cpp
)

# Applicalion headers files.
set (APP_HDRS
	dynamixelbus.h
	searchservosdialog.h
	selectserialport.h
	dynamixelmanager.h
)

# Applicalion ui files.
set (APP_UIS_H
	searchservosdialog.ui
	selectserialport.ui
	dynamixelmanager.ui
)

# Application resources files.
set (APP_QRC_H
	icons.qrc
)

# Wrap Qt classes inherited from QObject.
qt4_wrap_cpp (APP_MOCS ${APP_HDRS})

# Wrap Qt UI files.
qt4_wrap_ui (APP_UIS ${APP_UIS_H})

# Wrap Qt resources files.
qt4_add_resources(APP_QRC ${APP_QRC_H})

# LIBRARY_OUTPUT_PATH - link directory in which to store the output binary library
link_directories (${LIBRARY_OUTPUT_PATH})

add_executable (${APP_TARGET} ${APP_SRCS} ${APP_MOCS} ${APP_QRC} ${APP_UIS})

target_link_libraries (${APP_TARGET}
	${QT_LIBRARIES}
	${TRILOGGER_LIBRARY}
	${QDYNAMIXEL_LIBRARY}
	${QSERIALDEVICE_LIBRARY}
)
