project (TriLogger)
cmake_minimum_required (VERSION 2.6.0)

include_directories (${CMAKE_CURRENT_BINARY_DIR})

set (TRILOGGER_INCLUDE_DIRS
	${PROJECT_SOURCE_DIR}
	CACHE string "Paths to include library headers."
)

# LIBRARY_OUTPUT_PATH - path to contain the build output binary library,
#                       this path is cached and useful in the future to build the /examples.
# LIB_TARGET - target name of result output binary library file,
#                       this name of the library is stored and cached for the build of /examples.
# All depends on whether DEBUG or RELEASE.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/debug
        CACHE string "Path to output library binaries."
    )
    set(TRILOGGER_LIBRARY "triloggerd" CACHE string "Name of the output library binaries.")
else ()
    set (LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/release
        CACHE string "Path to output library binaries."
    )
    set (TRILOGGER_LIBRARY "trilogger" CACHE string "Name of the output library binaries.")
endif ()

set (TRILOGGER_SRCS tri_logger.cpp)
set (TRILOGGER_HDRS tri_logger.hpp nullstream.hpp)

# Set shared/static build library.
if (BUILD_SHARED)
    add_library (${TRILOGGER_LIBRARY} SHARED ${TRILOGGER_SRCS} ${TRILOGGER_HDRS})
else ()
    add_library (${TRILOGGER_LIBRARY} STATIC ${TRILOGGER_SRCS} ${TRILOGGER_HDRS})
endif ()
