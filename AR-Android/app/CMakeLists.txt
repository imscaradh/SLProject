# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)
add_definitions(-DSL_GUI_JAVA)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/include)

#
# SLProject
#
file(GLOB_RECURSE sl_SOURCES "../../*.cpp")
file(GLOB_RECURSE sl_HEADERS "../../*.h" "../../*.hpp")

set (sl_INCLUDE_DIRS "")
foreach (_headerFile ${sl_HEADERS})
    get_filename_component(_dir ${_headerFile} PATH)
    list (APPEND sl_INCLUDE_DIRS ${_dir})
endforeach()
list(REMOVE_DUPLICATES sl_INCLUDE_DIRS)

include_directories(${sl_INCLUDE_DIRS})

# Buggy
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal)

#
# OpenCV
#
add_library(lib_opencv SHARED IMPORTED)
set_target_properties(lib_opencv PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java3.so)


add_library( # Sets the name of the library.
             native-lib
             # Sets the library as a shared library.
             SHARED
             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/native-lib.cpp
             ${sl_SOURCES}
             )

find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries(native-lib
                        lib_opencv
                        ${OPENGL_LIBRARIES}
                        ${log-lib}
                     )
