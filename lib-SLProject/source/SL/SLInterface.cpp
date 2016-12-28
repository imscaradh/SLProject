//#############################################################################
//  File:      SL/SLInterface.cpp
//  Purpose:   Implementation of the main Scene Library C-Interface.
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>
#include <SLInterface.h>
#include <SLScene.h>
#include <SLSceneView.h>
#include <SLAssimpImporter.h>
#include <SLInputManager.h>
#include <SLCVCapture.h>

//! \file SLInterface.cpp SLProject C-functions interface implementation.
/*! \file SLInterface.cpp
The SLInterface.cpp has all implementations of the SLProject C-Interface.
Only these functions should called by the OS-dependent GUI applications.
These functions can be called from any C, C++ or ObjectiveC GUI framework or
by a native API such as Java Native Interface (JNI).
*/

//-----------------------------------------------------------------------------
//! global flag that determines if the application should be closed
bool gShouldClose = false;

//-----------------------------------------------------------------------------
/*! Global creation function for a SLScene instance. This function should be
called only once per application. The SLScene constructor call is delayed until
the first SLSceneView is created to guarantee, that the OpenGL context is
present.<br>
/param cmdLineArgs Command line arguments
/param shaderPath Path to the shader files (readonly)
/param modelPath Path to the 3D model files (readonly)
/param texturePath Path to the texture image files (readonly)
/param calibrationPath Path to the calibration ini files (readonly)
/param configPath Path where the config files are stored (read-write)
<br>
See examples usages in:
  - app-Demo-GLFW: glfwMain.cpp in function main()
  - app-Demo-Qt: qtGLWidget::initializeGL()
  - app-Viewer-Qt: qtGLWidget::initializeGL()
  - app-Demo-Android: Java_ch_fhnw_comgRT_glES2Lib_onInit()
  - app-Demo-iOS: ViewController.m in method viewDidLoad()
*/
void slCreateScene(SLVstring& cmdLineArgs,
                   SLstring shaderPath,
                   SLstring modelPath,
                   SLstring texturePath,
                   SLstring calibrationPath,
                   SLstring configPath)
{
    assert(SLScene::current==nullptr && "SLScene is already created!");
   
    // Default paths for all loaded resources
    SLGLProgram::defaultPath      = shaderPath;
    SLGLTexture::defaultPath      = texturePath;
    SLAssimpImporter::defaultPath = modelPath;
    #ifdef SL_HAS_OPENCV
    SLCVCalibration::calibIniPath  = calibrationPath;
    #endif
    SL::configPath                = configPath;

    SLGLState* stateGL            = SLGLState::getInstance();

    SL::parseCmdLineArgs(cmdLineArgs);
    
    SL_LOG("Path to Models  : %s\n", modelPath.c_str());
    SL_LOG("Path to Shaders : %s\n", shaderPath.c_str());
    SL_LOG("Path to Textures: %s\n", texturePath.c_str());
    SL_LOG("Path to Calibr. : %s\n", calibrationPath.c_str());
    SL_LOG("Path to Config. : %s\n", configPath.c_str());
    #ifdef SL_HAS_OPENCV
    SL_LOG("OpenCV Version  : %d.%d.%d\n", CV_MAJOR_VERSION,
                                           CV_MINOR_VERSION,
                                           CV_VERSION_REVISION);
    #else
    SL_LOG("OpenCV Version  : Not installed\n");
    #endif
    SL_LOG("OpenGL Version  : %s\n", stateGL->glVersion().c_str());
    SL_LOG("Vendor          : %s\n", stateGL->glVendor().c_str());
    SL_LOG("Renderer        : %s\n", stateGL->glRenderer().c_str());
    SL_LOG("GLSL Version    : %s (%s) \n", stateGL->glSLVersion().c_str(),
                                           stateGL->getSLVersionNO().c_str());
    SL_LOG("------------------------------------------------------------------\n");

    SLScene::current = new SLScene("");
}
//-----------------------------------------------------------------------------
/*! Global creation function for a SLSceneview instance returning the index of 
the sceneview. It creates the new SLSceneView instance by calling the callback
function slNewSceneView. If you have a custom SLSceneView inherited class you 
have to provide a similar function and pass it function pointer to 
slCreateSceneView. You can create multiple sceneview per application.<br>
<br>
See examples usages in:
  - app-Demo-GLFW: glfwMain.cpp in function main()
  - app-Demo-Qt: qtGLWidget::initializeGL()
  - app-Viewer-Qt: qtGLWidget::initializeGL()
  - app-Demo-Android: Java_ch_fhnw_comgRT_glES2Lib_onInit()
  - app-Demo-iOS: ViewController.m in method viewDidLoad()
*/
int slCreateSceneView(int screenWidth,
                      int screenHeight,
                      int dotsPerInch,
                      SLCommand initScene,
                      void* onWndUpdateCallback,
                      void* onSelectNodeMeshCallback,
                      void* onNewSceneViewCallback,
                      void* onShowSystemCursorCallback)
{
    assert(SLScene::current && "No SLScene::current!");

    // Use our own sceneview creator callback or the the passed one.
    cbOnNewSceneView newSVCallback;
    if (onNewSceneViewCallback==nullptr)
         newSVCallback = &slNewSceneView;
    else newSVCallback = (cbOnNewSceneView)onNewSceneViewCallback;

    // Create the sceneview & get the pointer with the sceneview index
    SLuint index = newSVCallback();
    SLSceneView* sv = SLScene::current->sv(index);

    sv->init("SceneView",
             screenWidth, 
             screenHeight, 
             dotsPerInch, 
             onWndUpdateCallback,
             onSelectNodeMeshCallback,
             onShowSystemCursorCallback);

    // Set active sceneview and load scene. This is done for the first sceneview
    if (!SLScene::current->root3D())
    {   SLScene::current->onLoad(sv, initScene);
    } else
        sv->onInitialize();
   
    // return the identifier index
    return sv->index();
}
//-----------------------------------------------------------------------------
/*! Global sceneview construction function returning the index of the created
sceneview instance. If you have a custom SLSceneView inherited class you 
have to provide a similar function and pass it function pointer to 
slCreateSceneView.
*/
int slNewSceneView()
{
    SLSceneView* sv = new SLSceneView();
    return sv->index();
}
//-----------------------------------------------------------------------------
/*! Global closing function that deallocates the sceneview and scene instances.
All the scenegraph deallocation is started from here and has to be done before
the GUI app terminates.
*/
bool slShouldClose()
{
    return gShouldClose;
}
//-----------------------------------------------------------------------------
/*! Global closing function that sets our global running flag. This lets
the windowing system know that we want to terminate.
*/
void slShouldClose(bool val)
{
    gShouldClose = val;
}
//-----------------------------------------------------------------------------
/*! Global closing function that deallocates the sceneview and scene instances.
All the scenegraph deallocation is started from here and has to be done before
the GUI app terminates.
*/
void slTerminate()
{    
    // Deletes all remaining sceneviews the current scene instance  
    delete SLScene::current;
    SLScene::current = 0;
}
//-----------------------------------------------------------------------------
/*! Global rendering function that first updates the scene due to user or
device inputs and due to active animations. This happens only if all sceneviews
where finished with rendering. After the update sceneviews onPaint routine is
called to initiate the rendering of the frame. If either the onUpdate or onPaint
returned true a new frame should be drawn.
*/
bool slUpdateAndPaint(int sceneViewIndex)
{  
    SLSceneView* sv = SLScene::current->sv(sceneViewIndex);
    bool sceneGotUpdated = SLScene::current->onUpdate();
    bool viewNeedsUpdate =  sv->onPaint();
    return sceneGotUpdated || viewNeedsUpdate;
}
//-----------------------------------------------------------------------------
/*! Global resize function that must be called whenever the OpenGL frame
changes it's size.
*/
void slResize(int sceneViewIndex, int width, int height)
{
    SLResizeEvent* e = new SLResizeEvent;
    e->svIndex = sceneViewIndex;
    e->width = width;
    e->height = height;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse button down events. 
*/
void slMouseDown(int sceneViewIndex, SLMouseButton button, 
                 int xpos, int ypos, SLKey modifier) 
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::MouseDown);
    e->svIndex = sceneViewIndex;
    e->button = button;
    e->x = xpos;
    e->y = ypos;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse move events.
*/
void slMouseMove(int sceneViewIndex, int x, int y)
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::MouseMove);
    e->svIndex = sceneViewIndex;
    e->x = x;
    e->y = y;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse button up events.
*/
void slMouseUp(int sceneViewIndex, SLMouseButton button, 
               int xpos, int ypos, SLKey modifier) 
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::MouseUp);
    e->svIndex = sceneViewIndex;
    e->button = button;
    e->x = xpos;
    e->y = ypos;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for double click events.
*/
void slDoubleClick(int sceneViewIndex, SLMouseButton button, 
                   int xpos, int ypos, SLKey modifier) 
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::MouseDoubleClick);
    e->svIndex = sceneViewIndex;
    e->button = button;
    e->x = xpos;
    e->y = ypos;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for long touches
*/
void slLongTouch(int sceneViewIndex, int xpos, int ypos) 
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::LongTouch);
    e->svIndex = sceneViewIndex;
    e->x = xpos;
    e->y = ypos;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger touch down events of touchscreen 
devices.
*/
void slTouch2Down(int sceneViewIndex, int xpos1, int ypos1, int xpos2, int ypos2) 
{  
    SLTouchEvent* e = new SLTouchEvent(SLInputEvent::Touch2Down);
    e->svIndex = sceneViewIndex;
    e->x1 = xpos1;
    e->y1 = ypos1;
    e->x2 = xpos2;
    e->y2 = ypos2;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger move events of touchscreen devices. 
*/
void slTouch2Move(int sceneViewIndex, int xpos1, int ypos1, int xpos2, int ypos2) 
{  
    SLTouchEvent* e = new SLTouchEvent(SLInputEvent::Touch2Move);
    e->svIndex = sceneViewIndex;
    e->x1 = xpos1;
    e->y1 = ypos1;
    e->x2 = xpos2;
    e->y2 = ypos2;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for the two finger touch up events of touchscreen 
devices. 
*/
void slTouch2Up(int sceneViewIndex, int xpos1, int ypos1, int xpos2, int ypos2) 
{
    SLTouchEvent* e = new SLTouchEvent(SLInputEvent::Touch2Up);
    e->svIndex = sceneViewIndex;
    e->x1 = xpos1;
    e->y1 = ypos1;
    e->x2 = xpos2;
    e->y2 = ypos2;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for mouse wheel events. 
*/
void slMouseWheel(int sceneViewIndex, int pos, SLKey modifier)
{  
    SLMouseEvent* e = new SLMouseEvent(SLInputEvent::MouseWheel);
    e->svIndex = sceneViewIndex;
    e->y = pos;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key press events. 
*/
void slKeyPress(int sceneViewIndex, SLKey key, SLKey modifier) 
{  
    SLKeyEvent* e = new SLKeyEvent(SLInputEvent::KeyDown);
    e->svIndex = sceneViewIndex;
    e->key = key;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key release events. 
*/
void slKeyRelease(int sceneViewIndex, SLKey key, SLKey modifier) 
{  
    SLKeyEvent* e = new SLKeyEvent(SLInputEvent::KeyUp);
    e->svIndex = sceneViewIndex;
    e->key = key;
    e->modifier = modifier;

    SLInputManager::instance().queueEvent(e);
}

//-----------------------------------------------------------------------------
/*! Global event handler for unicode character input.
*/
void slCharInput(int sceneViewIndex, unsigned int character)
{
    SLCharInputEvent* e = new SLCharInputEvent();
    e->svIndex = sceneViewIndex;
    e->character = character;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for keyboard key release events. 
*/
void slCommand(int sceneViewIndex, SLCommand command) 
{  
    SLCommandEvent* e = new SLCommandEvent;
    e->svIndex = sceneViewIndex;
    e->cmd = command;
    
    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
bool slUsesRotation(int sceneViewIndex)
{
    SLSceneView* sv = SLScene::current->sv(sceneViewIndex);
    return sv->usesRotation();
}
//-----------------------------------------------------------------------------
/*! Global event handler for device rotation change with Euler angles pitch
yaw and roll. With the parameter zeroYawAfterSec sets the time in seconds after
which the yaw angle is set to zero by subtracting the average yaw in this time.
*/
void slRotationPYR(int sceneViewIndex, 
                   float pitchRAD, float yawRAD, float rollRAD)
{
    SLRotationEvent* e = new SLRotationEvent(SLInputEvent::DeviceRotationPYR);
    e->svIndex = sceneViewIndex;
    e->x = pitchRAD;
    e->y = yawRAD;
    e->z = rollRAD;
    e->w = 3.0f;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global event handler for device rotation change with angle & and axis. 
*/
void slRotationQUAT(int sceneViewIndex, 
                    float quatX, float quatY, float quatZ, float quatW)
{
    SLRotationEvent* e = new SLRotationEvent(SLInputEvent::DeviceRotationPYR);
    e->svIndex = sceneViewIndex;
    e->x = quatX;
    e->y = quatY;
    e->z = quatZ;
    e->w = quatW;

    SLInputManager::instance().queueEvent(e);
}
//-----------------------------------------------------------------------------
/*! Global function to retrieve a window title text generated by the scene
library. 
*/
string slGetWindowTitle(int sceneViewIndex) 
{  
    SLSceneView* sv = SLScene::current->sv(sceneViewIndex);
    return sv->windowTitle();
}
//-----------------------------------------------------------------------------
/*! Global function to copy a new camera image to the SLScene::_videoTexture.
An application can grab the live video image with OpenCV via slGrabCopyVideoImage
or with another OS dependent framework.
*/
void slCopyVideoImage(SLint width,
                      SLint height,
                      SLPixelFormat format,
                      SLuchar* data,
                      SLbool isContinuous,
                      SLbool isTopLeft)
{
    #ifdef SL_HAS_OPENCV
    SLCVCapture::loadIntoLastFrame(width,
                                   height,
                                   format,
                                   data,
                                   isContinuous,
                                   isTopLeft);
    SLCVCapture::adjustForSL();
    #endif
}
//-----------------------------------------------------------------------------
/*! Global function returns true if SL wants a live video images
*/
bool slUsesVideo()
{
    #ifdef SL_HAS_OPENCV
    return SLScene::current->usesVideo();
    #else
    return false;
    #endif
}
//-----------------------------------------------------------------------------

