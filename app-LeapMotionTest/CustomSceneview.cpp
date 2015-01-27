

#include <random>

#include "SLRectangle.h"
#include "SLBox.h"
#include "SLSphere.h"
#include "SLAnimation.h"
#include "SLLightSphere.h"
#include "SLText.h"
#include "SLTexFont.h"
#include "SLAssImpImporter.h"

#include "SLInputManager.h"
#include "CustomSceneView.h"

using namespace Leap; // dont use using!

void drawXZGrid(const SLMat4f& mat)
{
    // for now we don't want to update the mesh implementation
    // or the buffer implementation, so we don't have vertex color support
    static bool         initialized = false;
    static SLGLBuffer   grid;
    static SLint        indexX;
    static SLint        indexZ;
    static SLint        indexGrid;
    static SLint        numXVerts;
    static SLint        numZVerts;
    static SLint        numGridVerts;

    if (!initialized)
    {
        vector<SLVec3f>  gridVert;

        SLint gridLineNum = 21;
        gridLineNum += gridLineNum%2 - 1; // make sure grid is odd
        SLint gridHalf = gridLineNum / 2;
        SLfloat gridSize = 2;
        SLfloat gridMax = (SLfloat)gridHalf/(gridLineNum-1) * gridSize;
        SLfloat gridMin = -gridMax;

        
        // x
        gridVert.push_back(SLVec3f(gridMin, 0, 0));
        gridVert.push_back(SLVec3f(gridMax, 0, 0));
        // z
        gridVert.push_back(SLVec3f(0, 0, gridMin));
        gridVert.push_back(SLVec3f(0, 0, gridMax));

        indexX = 0;
        indexZ = 2;
        indexGrid = 4;
        numXVerts = 2;
        numZVerts = 2;
        numGridVerts = (gridLineNum-1)*4;

        for (int i = 0; i < gridLineNum; ++i) 
        {
            SLfloat offset = (SLfloat)(i - gridHalf);
            offset /= (SLfloat)(gridLineNum-1);
            offset *= gridSize;
            
            // we're at the center
            if (offset != 0) 
            {
                // horizontal lines
                gridVert.push_back(SLVec3f(gridMin, 0, offset));
                gridVert.push_back(SLVec3f(gridMax, 0, offset));
                // vertical lines
                gridVert.push_back(SLVec3f(offset, 0, gridMin));
                gridVert.push_back(SLVec3f(offset, 0, gridMax));
            }
        }
        grid.generate(&gridVert[0], gridVert.size(), 3);

        initialized = true;
    }

    
    SLGLState* state = SLGLState::getInstance();
    state->pushModelViewMatrix();
    state->modelViewMatrix = mat;

    grid.drawArrayAsConstantColorLines(SLCol3f::RED,   1.0f, indexX, numXVerts);
    grid.drawArrayAsConstantColorLines(SLCol3f::BLUE, 1.0f, indexZ, numZVerts);
    grid.drawArrayAsConstantColorLines(SLCol3f(0.45f, 0.45f, 0.45f),  0.8f, indexGrid, numGridVerts);
    
    state->popModelViewMatrix();
}

















void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);
}

void SampleListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

void SampleListener::onFrame(const Controller& controller) {
    // Get the most recent frame and report some basic information
    const Frame frame = controller.frame();


    HandList hands = frame.hands();
    for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
        // Get the first hand
        const Hand hand = *hl;
        SLint index = 0;
        std::string handType = hand.isLeft() ? "Left hand" : "Right hand";
        Vector pos = hand.palmPosition();
        SLVec3f slPos(pos.x, pos.y, pos.z);
        if (hand.isLeft())
            positionsLeft[index++] = slPos;
        else
            positionsRight[index++] = slPos;
        

        
    
        FingerList fingers = hand.fingers();
        for(Leap::FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); fl++) {
            Bone bone;
            Bone::Type boneType;
            for (int b = 0; b < 4; ++b) {
                boneType = static_cast<Bone::Type>(b);
                bone = (*fl).bone(boneType);

                SLVec3f prevPos(bone.prevJoint().x, bone.prevJoint().y, bone.prevJoint().z);
                SLVec3f nextPos(bone.nextJoint().x, bone.nextJoint().y, bone.nextJoint().z);

                if (hand.isLeft()) {
                    positionsLeft[index++] = prevPos;
                    if (boneType == Bone::Type::TYPE_DISTAL)
                        positionsLeft[index++] = nextPos;
                }
                else {
                    positionsRight[index++] = prevPos;
                    if (boneType == Bone::Type::TYPE_DISTAL)
                        positionsRight[index++] = nextPos;
                }
            }
        }
    }
}

void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}


















// builds a custom scene with a grid where every node is animated
void SLScene::onLoad(SLSceneView* sv, SLCmd cmd)
{
    init();

    
    _backColor.set(0.1f,0.1f,0.1f);
    
    SLLightSphere* light1 = new SLLightSphere(0.1f);
    light1->position(2, -4, 3);

    SLLightSphere* light2 = new SLLightSphere(0.1f);
    light2->ambient(SLCol4f(0.2f, 0.1f, 0.0f, 1.0f));
    light2->diffuse(SLCol4f(2.0f, 0.9f, 0.5f, 1.0f));
    light2->position(2, 1, -3);
    
    SLLightSphere* light3 = new SLLightSphere(0.1f);
    light3->position(-5, 15, 10);

    SLMaterial* mat = new SLMaterial("floorMat", SLCol4f::GRAY, SLCol4f::GRAY);

    SLNode* scene = new SLNode;
    scene->addChild(light1);
    scene->addChild(light2);
    scene->addChild(light3);
    
    
    SLAssimpImporter importer("Importer.log");
    SLNode* meshDAE = importer.load("DAE/AstroBoy/AstroBoy.dae");
    importer.logFileVerbosity(LV_Detailed);
    SLNode* meshDAE2 = importer.load("DAE/Hands/rigged_hands.dae");

    
    for (SLint i = 0; i < importer.meshes().size(); ++i)
        importer.meshes()[i]->skinningMethod(SM_HardwareSkinning);

    
    // Scale to so that the AstroBoy is about 2 (meters) high.
    meshDAE->scale(10.0f);
    meshDAE->translate(0,-3.33f, 0, TS_Local);
    scene->addChild(meshDAE);

    meshDAE2->scale(10);
    meshDAE2->translate(0, 0, 5);
    scene->addChild(meshDAE2);

    SLCamera* cam1 = new SLCamera;
    cam1->position(-4, 3, 3);
    cam1->lookAt(0, 0, 1);
    cam1->focalDist(6);

    scene->addChild(cam1);
    
    _root3D = scene;

    sv->camera(cam1);
    sv->showMenu(false);
    sv->waitEvents(false);
    sv->onInitialize();
}

CustomSceneView::~CustomSceneView()
{
    _leapController.removeHandListener(&_slHandListener);
}

void CustomSceneView::postSceneLoad()
{
    //_leapController.addListener(_leapListener);
    //_leapController.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);
    _leapController.leapController().enableGesture(Leap::Gesture::TYPE_CIRCLE);
    _leapController.leapController().enableGesture(Leap::Gesture::TYPE_KEY_TAP);
    _leapController.leapController().enableGesture(Leap::Gesture::TYPE_SCREEN_TAP);
    _leapController.leapController().enableGesture(Leap::Gesture::TYPE_SWIPE);

    _leapController.registerHandListener(&_slHandListener);
    _leapController.registerHandListener(&_riggedListener);
    _leapController.registerToolListener(&_slToolListener);
    _leapController.registerGestureListener(&_gestureListener);
    _slHandListener.init();
    _slToolListener.init();
    _riggedListener.setSkeleton(SLScene::current->animManager().skeletons()[1]);
    
    //_riggedListener.setScaleCorrection(SLVec3f(10, 10, 10));

    _riggedListener.setLWrist("L_Wrist");
    _riggedListener.setRWrist("R_Wrist");
    
    _riggedListener.setRFingerJoint(0, 1, "R_ThumbA");
    _riggedListener.setRFingerJoint(0, 2, "R_ThumbB");
    _riggedListener.setRFingerJoint(0, 3, "R_ThumbC");
    
    _riggedListener.setRFingerJoint(1, 0, "R_IndexA");
    _riggedListener.setRFingerJoint(1, 1, "R_IndexB");
    _riggedListener.setRFingerJoint(1, 2, "R_IndexC");
    _riggedListener.setRFingerJoint(1, 3, "R_IndexD");
    
    _riggedListener.setRFingerJoint(2, 0, "R_MiddleA");
    _riggedListener.setRFingerJoint(2, 1, "R_MiddleB");
    _riggedListener.setRFingerJoint(2, 2, "R_MiddleC");
    _riggedListener.setRFingerJoint(2, 3, "R_MiddleD");
    
    _riggedListener.setRFingerJoint(3, 0, "R_RingA");
    _riggedListener.setRFingerJoint(3, 1, "R_RingB");
    _riggedListener.setRFingerJoint(3, 2, "R_RingC");
    _riggedListener.setRFingerJoint(3, 3, "R_RingD");
    
    _riggedListener.setRFingerJoint(4, 0, "R_PinkyA");
    _riggedListener.setRFingerJoint(4, 1, "R_PinkyB");
    _riggedListener.setRFingerJoint(4, 2, "R_PinkyC");
    _riggedListener.setRFingerJoint(4, 3, "R_PinkyD");
    
    
    _riggedListener.setLFingerJoint(0, 1, "L_ThumbA");
    _riggedListener.setLFingerJoint(0, 2, "L_ThumbB");
    _riggedListener.setLFingerJoint(0, 3, "L_ThumbC");
    
    _riggedListener.setLFingerJoint(1, 0, "L_IndexA");
    _riggedListener.setLFingerJoint(1, 1, "L_IndexB");
    _riggedListener.setLFingerJoint(1, 2, "L_IndexC");
    _riggedListener.setLFingerJoint(1, 3, "L_IndexD");
    
    _riggedListener.setLFingerJoint(2, 0, "L_MiddleA");
    _riggedListener.setLFingerJoint(2, 1, "L_MiddleB");
    _riggedListener.setLFingerJoint(2, 2, "L_MiddleC");
    _riggedListener.setLFingerJoint(2, 3, "L_MiddleD");
    
    _riggedListener.setLFingerJoint(3, 0, "L_RingA");
    _riggedListener.setLFingerJoint(3, 1, "L_RingB");
    _riggedListener.setLFingerJoint(3, 2, "L_RingC");
    _riggedListener.setLFingerJoint(3, 3, "L_RingD");
    
    _riggedListener.setLFingerJoint(4, 0, "L_PinkyA");
    _riggedListener.setLFingerJoint(4, 1, "L_PinkyB");
    _riggedListener.setLFingerJoint(4, 2, "L_PinkyC");
    _riggedListener.setLFingerJoint(4, 3, "L_PinkyD");
    /*
    _riggedListener.setLWrist("L_wrist");
    _riggedListener.setRWrist("R_wrist");
    // thumb
    //_riggedListener.setLFingerJoint(0, 1, "L_thumbOrient");
    _riggedListener.setLFingerJoint(0, 1, "L_thumb_01");
    _riggedListener.setLFingerJoint(0, 3, "L_thumb_02");
    _riggedListener.setRFingerJoint(0, 1, "R_thumb_01");
    _riggedListener.setRFingerJoint(0, 3, "R_thumb_02");
    // index
    _riggedListener.setLFingerJoint(1, 1, "L_index_01");
    _riggedListener.setLFingerJoint(1, 2, "L_index_02");
    _riggedListener.setRFingerJoint(1, 1, "R_index_01");
    _riggedListener.setRFingerJoint(1, 2, "R_index_02");
    // middle
    _riggedListener.setLFingerJoint(2, 1, "L_middle_01");
    _riggedListener.setLFingerJoint(2, 2, "L_middle_02");
    _riggedListener.setRFingerJoint(2, 1, "R_middle_01");
    _riggedListener.setRFingerJoint(2, 2, "R_middle_01");
    // pinky
    _riggedListener.setLFingerJoint(4, 1, "L_pinky_01");
    _riggedListener.setLFingerJoint(4, 2, "L_pinky_02");
    _riggedListener.setRFingerJoint(4, 1, "R_pinky_01");
    _riggedListener.setRFingerJoint(4, 2, "R_pinky_02");
    */
    /*
        Before continuing with the mapping above try and import one of the leap hand models and map the input to those (the ones used in the unity demos)
        And complete the todo (to check tutorials on how to bind kinect/leap on custom skeletons in unity/blender/maya...
    
    */

    _playbackCube = new SLNode(new SLBox(0.5f));
    SLScene::current->root3D()->addChild(_playbackCube);

    _root = SLScene::current->animManager().skeletons()[0]->getJoint("root");
}

void CustomSceneView::preDraw()
{
    //_root->rotate(1, 0, 1, 1); 
    // @note we need to force an update on the skeleton here
    //       since updateAnimations resets this dirty flag and the listener
    //       isn't guaranteed to come before draw but afte ranimation is called
    //SLScene::current->animManager().skeletons()[0]->changed(true);
    //SLScene::current->animManager().skeletons()[1]->changed(true);


    SLInputManager::instance().update();
}


void CustomSceneView::postDraw()
{
    drawXZGrid(_camera->updateAndGetVM());
}


// some basic manipulation for now
SLbool CustomSceneView::onKeyPress(const SLKey key, const SLKey mod)
{
    // temp test of leapmotion model axes
    SLSkeleton* skel = SLScene::current->animManager().skeletons()[1];
    SLJoint* indexJoint = skel->getJoint("R_IndexC");
    int dir = 1;
    if (indexJoint) {
        if (mod == KeyShift)
            dir = -1;

        switch (key)
        {
        case 'Y': indexJoint->rotate(dir*5, 1, 0, 0); break;
        case 'X': indexJoint->rotate(dir*5, 0, 1, 0); break;
        case 'C': indexJoint->rotate(dir*5, 0, 0, 1); break;
        }
    }

    return false;
}

SLbool CustomSceneView::onKeyRelease(const SLKey key, const SLKey mod)
{
    return false;
}
