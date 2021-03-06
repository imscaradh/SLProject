##############################################################################
#  File:      app-NodeManipulationDemo.pro
#  Purpose:   QMake project definition file for demo application with GLFW
#  Author:    Marcus Hudritsch
#  Date:      February 2014
#  Copyright: Marcus Hudritsch, Switzerland
#             THIS SOFTWARE IS PROVIDED FOR EDUCATIONAL PURPOSE ONLY AND
#             WITHOUT ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED.
##############################################################################

TEMPLATE = app
TARGET = app-NewNodeTesting

CONFIG += console
CONFIG -= qt
CONFIG += glfw
CONFIG += warn_off
CONFIG += app_bundle

DEFINES += "SL_STARTSCENE=C_sceneMeshLoad"

include(../SLProjectCommon.pro)

DESTDIR     = ../_bin-$$CONFIGURATION-$$PLATFORM
OBJECTS_DIR = ../intermediate/$$TARGET/$$CONFIGURATION/$$PLATFORM

LIBS += -L$$PWD/../_lib/$$CONFIGURATION/$$PLATFORM -llib-SLProject
LIBS += -L$$PWD/../_lib/$$CONFIGURATION/$$PLATFORM -llib-SLExternal
LIBS += -L$$PWD/../_lib/$$CONFIGURATION/$$PLATFORM -llib-assimp
win32 {LIBS += -L../_lib/$$CONFIGURATION/$$PLATFORM -llib-ovr}

win32 {POST_TARGETDEPS += $$PWD/../_lib/$$CONFIGURATION/$$PLATFORM/lib-SLProject.lib}
else  {POST_TARGETDEPS += $$PWD/../_lib/$$CONFIGURATION/$$PLATFORM/liblib-SLProject.a}
   
SOURCES += \
   glfwMain.cpp \
   NewNodeSceneView.cpp
	   
HEADERS += \
    NewNodeSceneView.h

include(../SLProjectCommonLibraries.pro)
include(../SLProjectDeploy.pro)
