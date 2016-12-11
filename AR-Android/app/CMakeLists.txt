# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)
add_definitions(-DSL_GUI_JAVA)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/main/jniInclude/include)

#
# SLProject
#
file(GLOB_RECURSE SLPROJECT_SOURCES     ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/math/SLCurveBezier.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/math/SLPlane.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SL.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLAssimpImporter.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLFileSystem.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLImage.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLImporter.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLInterface.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLTexFont.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SL/SLTimer.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLOculus.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLOculusFB.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLProgram.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLShader.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLState.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLTexture.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLVertexArray.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLVertexArrayExt.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/GL/SLGLVertexBuffer.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/CV/SLCVCapture.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/CV/SLCVCalibration.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/CV/SLCVTrackerAruco.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/CV/SLCVTrackerChessboard.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLAABBox.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLAnimation.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLAnimManager.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLAnimPlayback.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLAnimTrack.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLBackground.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLBox.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLButton.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLCamera.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLCone.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLCompactGrid.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLCoordAxis.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLCylinder.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLDisk.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLGrid.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLInputDevice.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLInputManager.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLJoint.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLKeyframe.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLLens.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLLight.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLLightRect.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLLightSpot.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLLightDirect.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLMaterial.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLMesh.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLNode.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLPathtracer.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLPolygon.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLRay.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLRaytracer.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLRectangle.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLRevolver.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLSamples2D.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLScene.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLSceneView.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLScene_onLoad.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLSkeleton.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLSpheric.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/SLText.cpp 
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLProject/source/CV/SLCVTracker.cpp)


file(GLOB_RECURSE SLEXTERNAL_SOURCES
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glew/src/glew.c
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/src/context.c
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/src/init.c
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/src/input.c
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/src/monitor.c
                                        #${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/src/window.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jutils.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jquant2.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jquant1.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jmemnobs.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jmemmgr.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jidctint.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jidctfst.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jidctflt.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jfdctint.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jfdctfst.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jfdctflt.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jerror.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdtrans.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdsample.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdpostct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdmerge.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdmaster.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdmarker.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdmainct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdinput.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdhuff.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jddctmgr.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdcolor.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdcoefct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdatasrc.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdatadst.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdarith.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdapistd.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jdapimin.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jctrans.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcsample.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcprepct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcparam.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcomapi.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcmaster.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcmarker.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcmainct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcinit.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jchuff.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcdctmgr.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jccolor.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jccoefct.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcarith.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcapistd.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jcapimin.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8/jaricom.c
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/sobol.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/ranrotw.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/ranrotb.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/random.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/mother.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc/mersenne.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/Shoemake/EulerAngles.cpp
                                        ${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/Shoemake/Decompose.cpp)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/jpeg-8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/randomc)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/png)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/nvwa)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/zlib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/assimp/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/assimp/code)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glew/include)
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/glfw3/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib-SLExternal/half/include)

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
             ${SLPROJECT_SOURCES}
             ${SLEXTERNAL_SOURCES}
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
