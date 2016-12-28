//#############################################################################
//  File:      SLScene.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>         // precompiled headers
#ifdef SL_MEMLEAKDETECT     // set in SL.h for debug config only
#include <debug_new.h>      // memory leak detector
#endif

#include <SLScene.h>
#include <SLSceneView.h>
#include <SLCamera.h>
#include <SLText.h>
#include <SLLight.h>
#include <SLTexFont.h>
#include <SLButton.h>
#include <SLAnimation.h>
#include <SLAnimManager.h>
#include <SLInputManager.h>
#include <SLCVCapture.h>
#include <SLAssimpImporter.h>
#include <SLBox.h>
#include <SLLightDirect.h>
#include <SLCVTracker.h>
#include <SLCVTrackerAruco.h>

//-----------------------------------------------------------------------------
/*! Global static scene pointer that can be used throughout the entire library
to access the current scene and its sceneviews. 
*/
SLScene* SLScene::current = nullptr;
//-----------------------------------------------------------------------------
/*! The constructor of the scene does all one time initialization such as 
loading the standard shader programs from which the pointers are stored in
the vector _shaderProgs. Custom shader programs that are loaded in a
scene must be deleted when the scene changes.
The following standard shaders are preloaded:
  - ColorAttribute.vert, Color.frag
  - ColorUniform.vert, Color.frag
  - DiffuseAttribute.vert, Diffuse.frag
  - PerVrtBlinn.vert, PerVrtBlinn.frag
  - PerVrtBlinnTex.vert, PerVrtBlinnTex.frag
  - TextureOnly.vert, TextureOnly.frag
  - PerPixBlinn.vert, PerPixBlinn.frag
  - PerPixBlinnTex.vert, PerPixBlinnTex.frag
  - BumpNormal.vert, BumpNormal.frag
  - BumpNormal.vert, BumpNormalParallax.frag
  - FontTex.vert, FontTex.frag
  - StereoOculus.vert, StereoOculus.frag
  - StereoOculusDistortionMesh.vert, StereoOculusDistortionMesh.frag

There will be only one scene for an application and it gets constructed in
the C-interface function slCreateScene in SLInterface.cpp that is called by the
platform and GUI-toolkit dependent window initialization.
As examples you can see it in:
  - app-Demo-GLFW: glfwMain.cpp in function main()
  - app-Demo-Qt: qtGLWidget::initializeGL()
  - app-Viewer-Qt: qtGLWidget::initializeGL()
  - app-Demo-Android: Java_ch_fhnw_comgRT_glES2Lib_onInit()
  - app-Demo-iOS: ViewController.m in method viewDidLoad()
*/
SLScene::SLScene(SLstring name) : SLObject(name)
{  
    current = this;

    _root3D         = nullptr;
    _menu2D         = nullptr;
    _menuGL         = nullptr;
    _menuRT         = nullptr;
    _menuPT         = nullptr;
    _info           = nullptr;
    _infoGL         = nullptr;
    _infoRT         = nullptr;
    _infoLoading    = nullptr;
    _btnHelp        = nullptr;
    _btnAbout       = nullptr;
    _btnCredits     = nullptr;
    _btnNoCalib     = nullptr;
    _selectedMesh   = nullptr;
    _selectedNode   = nullptr;
    _stopAnimations = false;

    _fps = 0;
    _elapsedTimeMS = 0;
    _lastUpdateTimeMS = 0;
    _frameTimesMS.init();
    _updateTimesMS.init();
    _cullTimesMS.init();
    _draw3DTimesMS.init();
    _draw2DTimesMS.init();
     
    // Load std. shader programs in order as defined in SLStdShaderProgs enum
    // In the constructor they are added the _shaderProgs vector
    SLGLProgram* p;
    p = new SLGLGenericProgram("ColorAttribute.vert","Color.frag");
    p = new SLGLGenericProgram("ColorUniform.vert","Color.frag");
    p = new SLGLGenericProgram("PerVrtBlinn.vert","PerVrtBlinn.frag");
    p = new SLGLGenericProgram("PerVrtBlinnColorAttrib.vert","PerVrtBlinn.frag");
    p = new SLGLGenericProgram("PerVrtBlinnTex.vert","PerVrtBlinnTex.frag");
    p = new SLGLGenericProgram("TextureOnly.vert","TextureOnly.frag");
    p = new SLGLGenericProgram("PerPixBlinn.vert","PerPixBlinn.frag");
    p = new SLGLGenericProgram("PerPixBlinnTex.vert","PerPixBlinnTex.frag");
    p = new SLGLGenericProgram("BumpNormal.vert","BumpNormal.frag");
    p = new SLGLGenericProgram("BumpNormal.vert","BumpNormalParallax.frag");
    p = new SLGLGenericProgram("FontTex.vert","FontTex.frag");
    p = new SLGLGenericProgram("StereoOculus.vert","StereoOculus.frag");
    p = new SLGLGenericProgram("StereoOculusDistortionMesh.vert","StereoOculusDistortionMesh.frag");
    _numProgsPreload = (SLint)_programs.size();
   
    // font and video texture are not added to the _textures vector
    SLTexFont::generateFonts();

    #ifdef SL_HAS_OPENCV
    // load default video image that is displayed when no live video is available
    _videoTexture.setVideoImage("LiveVideoError.png");

    // load opencv camera calibration
    _calibration.loadCamParams();
    _calibration.loadCalibParams();
    #endif // SL_HAS_OPENCV

    _oculus.init();

    _infoAbout_en =
"Welcome to the SLProject demo app (v2.0). It is developed at the \
Computer Science Department of the Bern University of Applied Sciences. \
The app shows what you can learn in one semester about 3D computer graphics \
in real time rendering and ray tracing. The framework is developed \
in C++ with OpenGL ES2 so that it can run also on mobile devices. \
Ray tracing provides in addition high quality transparencies, reflections and soft shadows. \
Click to close and use the menu to choose different scenes and view settings. \
For more information please visit: https://github.com/cpvrlab/SLProject";

    _infoCredits_en =
"Credits for external libraries: \\n\
- assimp: assimp.sourceforge.net \\n\
- glew: glew.sourceforge.net \\n\
- glfw: www.glfw.org \\n\
- jpeg: www.jpeg.org \\n\
- nvwa: sourceforge.net/projects/nvwa \\n\
- png: www.libpng.org \\n\
- Qt: www.qt-project.org \\n\
- randomc: www.agner.org/random \\n\
- zlib: zlib.net";

    _infoHelp_en =
"Help for mouse or finger control: \\n\
- Use mouse or your finger to rotate the scene \\n\
- Use mouse-wheel or pinch 2 fingers to go forward/backward \\n\
- Use CTRL-mouse or 2 fingers to move sidewards/up-down \\n\
- Double click or double tap to select object \\n\
- Screenshot: Use a screenshot tool,\\n\
on iOS: Quick hold down home & power button, \\n\
on Android: Quick hold down back & home button \\n\
on desktop: Use a screenshot tool";

    _infoNoCalib_en = 
"Your device camera is not yet or not anymore calibrated. \\n\
You are trying to use a scene that requires a calibrated live camera image. \\n\
To calibrate your camera please open the calibration scene with \\n\
Load Scene > Augmented Reality > Track Chessboard or Calibrate Camera. \\n\
It requires a chessboard image to be printed and glued on a flat board. \\n\
You can find the PDF with the chessboard image on: \\n\
https://github.com/cpvrlab/SLProject_data/tree/master/ \\n\
calibrations/CalibrationChessboard_8x5_A4.pdf \\n\\n\
For a calibration you have to take 20 images with detected inner \\n\
chessboard corners. To take an image you have to click with the mouse \\n\
or tap with finger into the screen.";

}
//-----------------------------------------------------------------------------
/*! The destructor does the final total deallocation of all global resources.
The destructor is called in slTerminate.
*/
SLScene::~SLScene()
{
    // Delete all remaining sceneviews
    for (auto sv : _sceneViews)
        if (sv != nullptr)
            delete sv;

    unInit();
   
    // delete global SLGLState instance
    SLGLState::deleteInstance();

    // clear light pointers
    _lights.clear();
   
    // delete materials 
    for (auto m : _materials) delete m;
    _materials.clear();
   
    // delete materials 
    for (auto m : _meshes) delete m;
    _meshes.clear();
   
    // delete textures
    for (auto t : _textures) delete t;
    _textures.clear();
   
    // delete shader programs
    for (auto p : _programs) delete p;
    _programs.clear();
        
    // delete AR tracker programs
    #ifdef SL_HAS_OPENCV
    for (auto t : _trackers) delete t;
    _trackers.clear();
    #endif
   
    // delete fonts   
    SLTexFont::deleteFonts();
   
    // delete menus & statistic texts
    deleteAllMenus();
   
    current = nullptr;


    #ifdef SL_USES_CVCAPTURE
    // release the capture device
    SLCVCapture::release();
    #endif

    SL_LOG("~SLScene\n");
    SL_LOG("------------------------------------------------------------------\n");
}
//-----------------------------------------------------------------------------
/*! The scene init is called whenever the scene is new loaded.
*/
void SLScene::init()
{     
    unInit();
   
    _background.colors(SLCol4f(0.6f,0.6f,0.6f), SLCol4f(0.3f,0.3f,0.3f));
    _globalAmbiLight.set(0.2f,0.2f,0.2f,0.0f);
    _selectedNode = 0;

    _timer.start();

    // load virtual cursor texture
    _texCursor = new SLGLTexture("cursor.tga");

    // load dummy live video texture
    #ifdef SL_HAS_OPENCV
    _usesVideo = false;
    #endif
}
//-----------------------------------------------------------------------------
/*! The scene uninitializing clears the scenegraph (_root3D) and all global
global resources such as materials, textures & custom shaders loaded with the 
scene. The standard shaders, the fonts and the 2D-GUI elements remain. They are
destructed at process end.
*/
void SLScene::unInit()
{  
    _selectedMesh = nullptr;
    _selectedNode = nullptr;

    // reset existing sceneviews
    for (auto sv : _sceneViews)
        if (sv != nullptr)
            sv->camera(sv->sceneViewCamera());

    // delete entire scene graph
    delete _root3D;
    _root3D = nullptr;

    // clear light pointers
    _lights.clear();

    // delete textures
    for (auto t : _textures) delete t;
    _textures.clear();
   
    // manually clear the default materials (it will get deleted below)
    SLMaterial::defaultGray(nullptr);
    SLMaterial::diffuseAttrib(nullptr);
    
    // delete materials 
    for (auto m : _materials) delete m;
    _materials.clear();

    // delete meshes 
    for (auto m : _meshes) delete m;
    _meshes.clear();
   
    SLMaterial::current = nullptr;
   
    // delete custom shader programs but not default shaders
    while (_programs.size() > _numProgsPreload)
    {   SLGLProgram* sp = _programs.back();
        delete sp;
        _programs.pop_back();
    }

    // delete trackers
    #ifdef SL_HAS_OPENCV
    for (auto t : _trackers) delete t;
    _trackers.clear();
    #endif
   
    // clear eventHandlers
    _eventHandlers.clear();

    _animManager.clear();

    // reset all states
    SLGLState::getInstance()->initAll();

    _currentSceneID = C_sceneEmpty;
}
//-----------------------------------------------------------------------------
//! Updates all animations in the scene after all views got painted.
/*! Updates different important updates in the scene after all views got painted:
\n
\n 1) Calculate frame time
\n 2) Update all animations
\n 3) Augmented Reality (AR) Tracking with the live camera
\n 4) Update AABBs
\n
A scene can be displayed in multiple views as demonstrated in the app-Viewer-Qt 
example. AR tracking is only handled on the first scene view.
\return true if really something got updated
*/
bool SLScene::onUpdate()
{
    // Return if not all sceneview got repainted
    for (auto sv : _sceneViews)
        if (sv != nullptr && !sv->gotPainted())
            return false;

    // Reset all _gotPainted flags
    for (auto sv : _sceneViews)
        if (sv != nullptr)
            sv->gotPainted(false);
    

    /////////////////////////////
    // 1) Calculate frame time //
    /////////////////////////////

    // Calculate the elapsed time for the animation
    // todo: If slowdown on idle is enabled the delta time will be wrong!
    _elapsedTimeMS = timeMilliSec() - _lastUpdateTimeMS;
    _lastUpdateTimeMS = timeMilliSec();
     
    // Sum up all timings of all sceneviews
    SLfloat sumCullTimeMS   = 0.0f;
    SLfloat sumDraw3DTimeMS = 0.0f;
    SLfloat sumDraw2DTimeMS = 0.0f;
    SLbool renderTypeIsRT = false;
    SLbool voxelsAreShown = false;
    for (auto sv : _sceneViews)
    {   if (sv != nullptr)
        {   sumCullTimeMS   += sv->cullTimeMS();
            sumDraw3DTimeMS += sv->draw3DTimeMS();
            sumDraw2DTimeMS += sv->draw2DTimeMS();
            if (!renderTypeIsRT && sv->renderType()==RT_rt)
                renderTypeIsRT = true;
            if (!voxelsAreShown && sv->drawBit(SL_DB_VOXELS))
                voxelsAreShown = true;
        }
    }
    _cullTimesMS.set(sumCullTimeMS);
    _draw3DTimesMS.set(sumDraw3DTimeMS);
    _draw2DTimesMS.set(sumDraw2DTimeMS);

    // Calculate the frames per second metric
    _frameTimesMS.set(_elapsedTimeMS);
    _fps = 1 / _frameTimesMS.average() * 1000.0f;
    if (_fps < 0.0f) _fps = 0.0f;


    //////////////////////////////
    // 2) Update all animations //
    //////////////////////////////

    SLfloat startUpdateMS = timeMilliSec();

    // reset the dirty flag on all skeletons
    for(auto skeleton : _animManager.skeletons())
        skeleton->changed(false);

    // Process queued up system events and poll custom input devices
    SLbool animatedOrChanged = SLInputManager::instance().pollEvents();

    animatedOrChanged |= !_stopAnimations && _animManager.update(elapsedTimeSec());
    
    // Do software skinning on all changed skeletons
    for (auto mesh : _meshes) 
    {   if (mesh->skeleton() && mesh->skeleton()->changed())
        {   mesh->transformSkin();
            animatedOrChanged = true;
        }

        // update any out of date acceleration structure for RT or if they're being rendered.
        if (renderTypeIsRT || voxelsAreShown)
            mesh->updateAccelStruct();
    }
    

    ////////////////////
    // 3) AR Tracking //
    ////////////////////
    
    #ifdef SL_HAS_OPENCV
    if (_usesVideo && !SLCVCapture::lastFrame.empty())
    {   
        // Invalidate calibration if camera input aspect doesn't match output
        SLfloat calibWdivH = _calibration.imageAspectRatio();
        SLbool aspectRatioDoesNotMatch = SL_abs(_sceneViews[0]->scrWdivH() - calibWdivH) > 0.01f;
        if (aspectRatioDoesNotMatch && _calibration.state() == CS_calibrated)
        {   _calibration.clear();
        }

        stringstream ss; // info line text

        if (_calibration.state() == CS_uncalibrated)
        {   
            menu2D(btnNoCalib());
            if (_currentSceneID == C_sceneTrackChessboard)
                _calibration.state(CS_calibrateStream);
        } 
        else
        if (_calibration.state() == CS_calibrateStream || 
            _calibration.state() == CS_calibrateGrab)
        {               
            _calibration.findChessboard(SLCVCapture::lastFrame,
                                        SLCVCapture::lastFrameGray,
                                        true);

            int imgsToCap = _calibration.numImgsToCapture();
            int imgsCaped = _calibration.numCapturedImgs();

            //update info line
            if(imgsCaped < imgsToCap)
                ss << "Click on the screen to create a calibration foto. Created " 
                   << imgsCaped << " of " << imgsToCap;
            else
            {   ss << "Calculating, please wait ...";
                _calibration.state(CS_startCalculating);
            }
            info(_sceneViews[0], ss.str());
        }
        else
        if (_calibration.state() == CS_startCalculating)
        {
            _calibration.calculate();
            _sceneViews[0]->camera()->fov(_calibration.cameraFovDeg());
        }
        else
        if (_calibration.state() == CS_calibrated)
        {
            SLCVTrackerAruco::trackAllOnce = true;
        
            // track all trackers in the first sceneview
            for (auto tracker : _trackers)
                tracker->track(SLCVCapture::lastFrameGray,
                               _calibration,
                               _sceneViews[0]);
            
            if (_currentSceneID == C_sceneTrackChessboard)
            {   ss << "Camera calibration: fov: " << _calibration.cameraFovDeg() << 
                      ", error: " << _calibration.reprojectionError();
                info(_sceneViews[0], ss.str());
            }
        }

        //copy image to video texture
        if(_calibration.state() == CS_calibrated && _calibration.showUndistorted())
        {
            SLCVMat undistorted;

            undistort(SLCVCapture::lastFrame,
                      undistorted,
                      _calibration.intrinsics(),
                      _calibration.distortion());

            _videoTexture.copyVideoImage(undistorted.cols,
                                         undistorted.rows,
                                         SLCVCapture::format,
                                         undistorted.data,
                                         undistorted.isContinuous(),
                                         true);
        } else
        {
            _videoTexture.copyVideoImage(SLCVCapture::lastFrame.cols,
                                         SLCVCapture::lastFrame.rows,
                                         SLCVCapture::format,
                                         SLCVCapture::lastFrame.data,
                                         SLCVCapture::lastFrame.isContinuous(),
                                         true);

        }
    }
    #endif // SL_HAS_OPENCV


    /////////////////////
    // 4) Update AABBs //
    /////////////////////

    // The updateAABBRec call won't generate any overhead if nothing changed
    SLGLState::getInstance()->modelViewMatrix.identity();
    if (_root3D)
        _root3D->updateAABBRec();


    _updateTimesMS.set(timeMilliSec()-startUpdateMS);
    
    return animatedOrChanged;
}
//-----------------------------------------------------------------------------
//! SLScene::onAfterLoad gets called after onLoad
void SLScene::onAfterLoad()
{
    #ifdef SL_USES_CVCAPTURE
    if (_usesVideo)
    {   if (!SLCVCapture::isOpened())
            SLCVCapture::open(0);
    }
    #endif
}
//-----------------------------------------------------------------------------
/*!
SLScene::info deletes previous info text and sets new one with a max. width 
*/
void SLScene::info(SLSceneView* sv, SLstring infoText, SLCol4f color)
{  
    delete _info;
   
    // Set font size depending on DPI
    SLTexFont* f = SLTexFont::getFont(1.5f, sv->dpi());

    SLfloat minX = 11 * sv->dpmm();
    _info = new SLText(infoText, f, color, 
                       sv->scrW()-minX-5.0f,
                       1.2f);

    _info->translate(minX, SLButton::minMenuPos.y, 0, TS_object);
}
//-----------------------------------------------------------------------------
/*! 
SLScene::info returns the info text. If null it creates an empty one
*/
SLText* SLScene::info(SLSceneView* sv)
{
    if (_info == nullptr) info(sv, "", SLCol4f::WHITE);
    return _info;
}
//-----------------------------------------------------------------------------
//! Sets the _selectedNode to the passed Node and flags it as selected
void SLScene::selectNode(SLNode* nodeToSelect)
{
    if (_selectedNode)
        _selectedNode->drawBits()->off(SL_DB_SELECTED);

    if (nodeToSelect)
    {  if (_selectedNode == nodeToSelect)
        {   _selectedNode = 0;
        } else
        {   _selectedNode = nodeToSelect;
            _selectedNode->drawBits()->on(SL_DB_SELECTED);
        }
    } else _selectedNode = 0;
}
//-----------------------------------------------------------------------------
//! Sets the _selectedNode and _selectedMesh and flags it as selected
void SLScene::selectNodeMesh(SLNode* nodeToSelect, SLMesh* meshToSelect)
{
    if (_selectedNode)
        _selectedNode->drawBits()->off(SL_DB_SELECTED);

    if (nodeToSelect)
    {  if (_selectedNode == nodeToSelect && _selectedMesh == meshToSelect)
        {   _selectedNode = 0;
            _selectedMesh = 0;
        } else
        {   _selectedNode = nodeToSelect;
            _selectedMesh = meshToSelect;
            _selectedNode->drawBits()->on(SL_DB_SELECTED);
        }
    } else 
    {   _selectedNode = 0;
        _selectedMesh = 0;
    }
}
//-----------------------------------------------------------------------------
//! Executes a command on all sceneview
SLbool SLScene::onCommandAllSV(const SLCommand cmd)
{
    SLbool result = false;
    for(auto sv : _sceneViews)
        if (sv != nullptr)
            result = sv->onCommand(cmd) ? true : result;

    return true;
}
//-----------------------------------------------------------------------------
//! Copies the image data from a video camera into image[0] of the video texture
void SLScene::copyVideoImage(SLint width,
                             SLint height,
                             SLPixelFormat srcPixelFormat,
                             SLuchar* data,
                             SLbool isContinuous,
                             SLbool isTopLeft)
{
    #ifdef SL_HAS_OPENCV
    _videoTexture.copyVideoImage(width, 
                                 height, 
                                 srcPixelFormat, 
                                 data, 
                                 isContinuous,
                                 isTopLeft);
    #endif
}
//-----------------------------------------------------------------------------
//! Deletes all menus and buttons objects
void SLScene::deleteAllMenus()
{                           _menu2D     = nullptr;
    delete _menuGL;         _menuGL     = nullptr;
    delete _menuRT;         _menuRT     = nullptr;
    delete _menuPT;         _menuPT     = nullptr;
    delete _info;           _info       = nullptr;
    delete _infoGL;         _infoGL     = nullptr;
    delete _infoRT;         _infoRT     = nullptr;
    delete _btnAbout;       _btnAbout   = nullptr;
    delete _btnHelp;        _btnHelp    = nullptr;
    delete _btnCredits;     _btnCredits = nullptr;
    delete _btnNoCalib;     _btnNoCalib = nullptr;
}
//-----------------------------------------------------------------------------
void SLScene::onLoadAsset(SLstring assetFile, 
                          SLuint processFlags)
{
    _currentSceneID = C_sceneFromFile;

    // Set scene name for new scenes
    if (!_root3D)
        name(SLUtils::getFileName(assetFile));

    // Try to load assed and add it to the scene root node
    SLAssimpImporter importer;

    //////////////////////////////////////////////////////////////
    SLNode* loaded = importer.load(assetFile, true, processFlags);
    //////////////////////////////////////////////////////////////

    // Add root node on empty scene
    if (!_root3D)
    {   SLNode* scene = new SLNode("Scene");
        _root3D = scene;
    }

    // Add loaded scene
    if (loaded) 
        _root3D->addChild(loaded);

    // Add directional light if no light was in loaded asset
    if (!_lights.size())
    {   SLAABBox boundingBox = _root3D->updateAABBRec();
        SLfloat arrowLength = boundingBox.radiusWS() > FLT_EPSILON ? 
                              boundingBox.radiusWS() * 0.1f : 0.5f;
        SLLightDirect* light = new SLLightDirect(0,0,0, 
                                                 arrowLength,
                                                 1.0f, 1.0f, 1.0f);
        SLVec3f pos = boundingBox.maxWS().isZero() ? 
                      SLVec3f(1,1,1) : boundingBox.maxWS() * 1.1f;
        light->translation(pos);
        light->lookAt(pos-SLVec3f(1,1,1));
        light->attenuation(1,0,0);
        _root3D->addChild(light);
        _root3D->aabb()->reset(); // rest aabb so that it is recalculated
    }

    // call onInitialize on all scene views
    for (auto sv : _sceneViews)
    {   if (sv != nullptr)
        {   sv->onInitialize();
            sv->showLoading(false);
        }
    }
}
//-----------------------------------------------------------------------------
