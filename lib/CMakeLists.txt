if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/debug/lib
        CACHE string "Path to output library binaries."
    )
else ()
    set (LIBRARY_OUTPUT_PATH 
        ${PROJECT_BINARY_DIR}/release/lib
        CACHE string "Path to output library binaries."
    )
endif ()

add_definitions(-DETLOG)

add_subdirectory (trilogger)
add_subdirectory (qserialdevice)
add_subdirectory (qdynamixel)
