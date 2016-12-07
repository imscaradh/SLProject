# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)
add_definitions(-DSL_GUI_JAVA)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/slproject)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/Shoemake/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/half/include)
#
# SLProject
#
add_library(lib_slexternal SHARED IMPORTED)
set_target_properties(lib_slexternal PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/liblib-SLExternal.a)

add_library(lib_slproject SHARED IMPORTED)
set_target_properties(lib_slproject PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/liblib-SLProject.a)

add_library(lib_assimp SHARED IMPORTED)
set_target_properties(lib_assimp PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/liblib-assimp.a)

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
             )

find_library( # Sets the name of the path variable.
              log-lib
              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log)


# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( native-lib ${lib_slproject})

target_link_libraries(  native-lib
                        lib_slproject
                        lib_slexternal
                        lib_assimp
                        lib_opencv
                        ${OPENGL_LIBRARIES}
                        ${log-lib}
                     )
