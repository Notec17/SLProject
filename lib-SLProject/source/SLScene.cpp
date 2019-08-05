//#############################################################################
//  File:      SLScene.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h> // Must be the 1st include followed by  an empty line

#ifdef SL_MEMLEAKDETECT    // set in SL.h for debug config only
#    include <debug_new.h> // memory leak detector
#endif

#include <Utils.h>
#include <SLApplication.h>
#include <SLAssimpImporter.h>
#include <SLCVCapture.h>
#include <SLCVTracked.h>
#include <SLCVTrackedAruco.h>
#include <SLDeviceLocation.h>
#include <SLInputManager.h>
#include <SLLightDirect.h>
#include <SLScene.h>
#include <SLSceneView.h>
#include <SLText.h>
#include <SLKeyframeCamera.h>

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
  - PerPixCookTorance.vert, PerPixCookTorance.frag
  - PerPixCookToranceTex.vert, PerPixCookToranceTex.frag
  - BumpNormal.vert, BumpNormal.frag
  - BumpNormal.vert, BumpNormalParallax.frag
  - FontTex.vert, FontTex.frag
  - StereoOculus.vert, StereoOculus.frag
  - StereoOculusDistortionMesh.vert, StereoOculusDistortionMesh.frag

There will be only one scene for an application and it gets constructed in
the C-interface function slCreateScene in SLInterface.cpp that is called by the
platform and UI-toolkit dependent window initialization.
As examples you can see it in:
  - app-Demo-SLProject/GLFW: glfwMain.cpp in function main()
  - app-Demo-SLProject/android: Java_ch_fhnw_comgRT_glES2Lib_onInit()
  - app-Demo-SLProject/iOS: ViewController.m in method viewDidLoad()
  - _old/app-Demo-Qt: qtGLWidget::initializeGL()
  - _old/app-Viewer-Qt: qtGLWidget::initializeGL()
*/
SLScene::SLScene(SLstring      name,
                 cbOnSceneLoad onSceneLoadCallback)
  : SLObject(name),
    _frameTimesMS(60, 0.0f),
    _vsyncTimesMS(60, 0.0f),
    _captureTimesMS(200, 0.0f),
    _updateTimesMS(60, 0.0f),
    _cullTimesMS(60, 0.0f),
    _draw3DTimesMS(60, 0.0f),
    _draw2DTimesMS(60, 0.0f),
    _trackingTimesMS(60, 0.0f),
    _detectTimesMS(60, 0.0f),
    _detect1TimesMS(60, 0.0f),
    _detect2TimesMS(60, 0.0f),
    _matchTimesMS(60, 0.0f),
    _optFlowTimesMS(60, 0.0f),
    _poseTimesMS(60, 0.0f),
    _updateAABBTimesMS(60, 0.0f),
    _updateAnimTimesMS(60, 0.0f)
{
    SLApplication::scene = this;

    onLoad = onSceneLoadCallback;

    _root3D           = nullptr;
    _root2D           = nullptr;
    _info             = "";
    _selectedMesh     = nullptr;
    _selectedNode     = nullptr;
    _stopAnimations   = false;
    _fps              = 0;
    _frameTimeMS      = 0;
    _lastUpdateTimeMS = 0;

    // Load std. shader programs in order as defined in SLShaderProgs enum in SLenum
    // In the constructor they are added the _shaderProgs vector
    // If you add a new shader here you have to update the SLShaderProgs enum accordingly.
    SLGLProgram* p;
    p = new SLGLGenericProgram("ColorAttribute.vert", "Color.frag");
    p = new SLGLGenericProgram("ColorUniform.vert", "Color.frag");
    p = new SLGLGenericProgram("PerVrtBlinn.vert", "PerVrtBlinn.frag");
    p = new SLGLGenericProgram("PerVrtBlinnColorAttrib.vert", "PerVrtBlinn.frag");
    p = new SLGLGenericProgram("PerVrtBlinnTex.vert", "PerVrtBlinnTex.frag");
    p = new SLGLGenericProgram("TextureOnly.vert", "TextureOnly.frag");
    p = new SLGLGenericProgram("PerPixBlinn.vert", "PerPixBlinn.frag");
    p = new SLGLGenericProgram("PerPixBlinnTex.vert", "PerPixBlinnTex.frag");
    p = new SLGLGenericProgram("PerPixCookTorrance.vert", "PerPixCookTorrance.frag");
    p = new SLGLGenericProgram("PerPixCookTorranceTex.vert", "PerPixCookTorranceTex.frag");
    p = new SLGLGenericProgram("BumpNormal.vert", "BumpNormal.frag");
    p = new SLGLGenericProgram("BumpNormal.vert", "BumpNormalParallax.frag");
    p = new SLGLGenericProgram("FontTex.vert", "FontTex.frag");
    p = new SLGLGenericProgram("StereoOculus.vert", "StereoOculus.frag");
    p = new SLGLGenericProgram("StereoOculusDistortionMesh.vert", "StereoOculusDistortionMesh.frag");

    _numProgsPreload = (SLint)_programs.size();

    // font and video texture are not added to the _textures vector
    SLTexFont::generateFonts();

    // load default video image that is displayed when no live video is available
    _videoTexture.setVideoImage("LiveVideoError.png");
    _videoTextureErr.setVideoImage("LiveVideoError.png");

    // Set video type to none (this also sets the active calibration to the main calibration)
    videoType(VT_NONE);
    _showDetection = false;

    _oculus.init();
}
//-----------------------------------------------------------------------------
/*! The destructor does the final total deallocation of all global resources.
The destructor is called in slTerminate.
*/
SLScene::~SLScene()
{
    //// Delete all remaining sceneviews
    //for (auto sv : _sceneViews)
    //    if (sv != nullptr)
    //        delete sv;

    // delete AR tracker programs
    for (auto t : _trackers) delete t;
    _trackers.clear();

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

    // delete fonts
    SLTexFont::deleteFonts();

    // release the capture device
    SLCVCapture::release();

    SL_LOG("Destructor      : ~SLScene\n");
    SL_LOG("------------------------------------------------------------------\n");
}
//-----------------------------------------------------------------------------
/*! The scene init is called before a new scene is assembled.
*/
void SLScene::init()
{
    unInit();

    // reset all states
    SLGLState::getInstance()->initAll();

    _globalAmbiLight.set(0.2f, 0.2f, 0.2f, 0.0f);
    _selectedNode = nullptr;

    // Reset timing variables
    _timer.start();
    _vsyncTimesMS.init(60, 0.0f);
    _frameTimesMS.init(60, 0.0f);
    _captureTimesMS.init(200, 0.0f);
    _updateTimesMS.init(60, 0.0f);
    _cullTimesMS.init(60, 0.0f);
    _draw3DTimesMS.init(60, 0.0f);
    _draw2DTimesMS.init(60, 0.0f);
    _trackingTimesMS.init(60, 0.0f);
    _detectTimesMS.init(60, 0.0f);
    _detect1TimesMS.init(60, 0.0f);
    _detect2TimesMS.init(60, 0.0f);
    _matchTimesMS.init(60, 0.0f);
    _optFlowTimesMS.init(60, 0.0f);
    _poseTimesMS.init(60, 0.0f);
    _updateAnimTimesMS.init(60, 0.0f);
    _updateAABBTimesMS.init(60, 0.0f);

    // Reset calibration process at scene change
    if (SLApplication::activeCalib->state() != CS_calibrated &&
        SLApplication::activeCalib->state() != CS_uncalibrated)
        SLApplication::activeCalib->state(CS_uncalibrated);

    // Deactivate in general the device sensors
    SLApplication::devRot.isUsed(false);
    SLApplication::devLoc.isUsed(false);

    // Reset the video texture
    _videoTexture.setVideoImage("LiveVideoError.png");

    _selectedRect.setZero();
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

    //// reset existing sceneviews
    //for (auto sv : _sceneViews)
    //{
    //    if (sv != nullptr)
    //    {
    //        sv->camera(sv->sceneViewCamera());
    //        sv->skybox(nullptr);
    //    }
    //}

    // delete entire scene graph
    delete _root3D;
    _root3D = nullptr;
    delete _root2D;
    _root2D = nullptr;

    // clear light pointers
    _lights.clear();

    // delete textures that where allocated during scene construction.
    // The video & raytracing textures are not in this vector and are not dealocated
    for (auto t : _textures)
        delete t;
    _textures.clear();

    // manually clear the default materials (it will get deleted below)
    SLMaterial::defaultGray(nullptr);
    SLMaterial::diffuseAttrib(nullptr);

    // delete materials
    for (auto m : _materials)
        delete m;
    _materials.clear();

    // delete meshes
    for (auto m : _meshes)
        delete m;
    _meshes.clear();

    SLMaterial::current = nullptr;

    // delete custom shader programs but not default shaders
    while (_programs.size() > (SLuint)_numProgsPreload)
    {
        SLGLProgram* sp = _programs.back();
        delete sp;
        _programs.pop_back();
    }

    // delete trackers
    for (auto t : _trackers)
        delete t;
    _trackers.clear();

    // close video if running
    if (_videoType != VT_NONE)
    {
        SLCVCapture::release();
        _videoType = VT_NONE;
    }

    _eventHandlers.clear();

    _animManager.clear();
}
//-----------------------------------------------------------------------------
void SLScene::calculateFrameTime(SLSceneView* sv)
{
    ///////////////////////////////
    // 1) Calculate frame time //
    /////////////////////////////

    // Calculate the elapsed time for the animation
    // todo: If slowdown on idle is enabled the delta time will be wrong!
    _frameTimeMS      = timeMilliSec() - _lastUpdateTimeMS;
    _lastUpdateTimeMS = timeMilliSec();

    // Sum up all timings of all sceneviews
    _sumCullTimeMS   = 0.0f;
    _sumDraw3DTimeMS = 0.0f;
    _sumDraw2DTimeMS = 0.0f;
    //SLbool  renderTypeIsRT  = false;
    //SLbool  voxelsAreShown  = false;
    //for (auto sv : _sceneViews)
    //{
    if (sv != nullptr)
    {
        _sumCullTimeMS += sv->cullTimeMS();
        _sumDraw3DTimeMS += sv->draw3DTimeMS();
        _sumDraw2DTimeMS += sv->draw2DTimeMS();
        //if (!renderTypeIsRT && sv->renderType() == RT_rt)
        //    renderTypeIsRT = true;
        //if (!voxelsAreShown && sv->drawBit(SL_DB_VOXELS))
        //    voxelsAreShown = true;
    }
    //}
    _cullTimesMS.set(_sumCullTimeMS);
    _draw3DTimesMS.set(_sumDraw3DTimeMS);
    _draw2DTimesMS.set(_sumDraw2DTimeMS);

    // Calculate the frames per second metric
    _frameTimesMS.set(_frameTimeMS);
    _fps = 1 / _frameTimesMS.average() * 1000.0f;
    if (_fps < 0.0f) _fps = 0.0f;

    _startUpdateMS = timeMilliSec();
}
//-----------------------------------------------------------------------------
void SLScene::updateAnimations(SLbool sceneHasChanged,
                               SLbool renderTypeIsRT,
                               SLbool voxelsAreShown)
{
    SLfloat startAnimUpdateMS = timeMilliSec();

    if (_root3D)
        _root3D->update();

    // reset the dirty flag on all skeletons
    for (auto skeleton : _animManager.skeletons())
        skeleton->changed(false);

    sceneHasChanged |= !_stopAnimations && _animManager.update(elapsedTimeSec());

    // Do software skinning on all changed skeletons
    for (auto mesh : _meshes)
    {
        if (mesh->skeleton() && mesh->skeleton()->changed())
        {
            mesh->transformSkin();
            sceneHasChanged = true;
        }

        // update any out of date acceleration structure for RT or if they're being rendered.
        if (renderTypeIsRT || voxelsAreShown)
            mesh->updateAccelStruct();
    }

    _updateAnimTimesMS.set(timeMilliSec() - startAnimUpdateMS);
}
//-----------------------------------------------------------------------------
void SLScene::updateAABBs()
{
    // The updateAABBRec call won't generate any overhead if nothing changed
    SLfloat startAAABBUpdateMS = timeMilliSec();
    SLNode::numWMUpdates       = 0;
    SLGLState::getInstance()->modelViewMatrix.identity();
    if (_root3D)
        _root3D->updateAABBRec();
    if (_root2D)
        _root2D->updateAABBRec();
    _updateAABBTimesMS.set(timeMilliSec() - startAAABBUpdateMS);

    // Finish total update time
    SLfloat updateTimeMS = timeMilliSec() - _startUpdateMS;
    _updateTimesMS.set(updateTimeMS);

    // calculate vsync time as diff. of major part times to the frame time
    SLfloat totalMajorTime = captureTimesMS().average() +
                             updateTimeMS +
                             _sumCullTimeMS +
                             _sumDraw3DTimeMS +
                             _sumDraw2DTimeMS;
    SLfloat vsyncTime = _frameTimeMS > totalMajorTime ? _frameTimeMS - totalMajorTime : 0.0f;
    _vsyncTimesMS.set(vsyncTime);

    //SL_LOG("SLScene::onUpdate\n");
}
//-----------------------------------------------------------------------------
//! Processes all queued events and updates animations, AR trackers and AABBs
/*! Updates different updatables in the scene after all views got painted:
\n
\n 1) Calculate frame time
\n 2) Process queued events
\n 3) Update all animations
\n 4) Augmented Reality (AR) Tracking with the live camera
\n 5) Update AABBs
\n
A scene can be displayed in multiple views as demonstrated in the app-Viewer-Qt
example. AR tracking is only handled on the first scene view.
\return true if really something got updated
*/
bool SLScene::onUpdate()
{
    //// Return if not all sceneview got repainted: This check if necessary if
    //// this function is called for multiple SceneViews. In this way we only
    //// update the geometric representations if all SceneViews got painted once.

    //for (auto sv : _sceneViews)
    //    if (sv != nullptr && !sv->gotPainted())
    //        return false;

    //// Reset all _gotPainted flags
    //for (auto sv : _sceneViews)
    //    if (sv != nullptr)
    //        sv->gotPainted(false);

    ///////////////////////////////
    //// 1) Calculate frame time //
    ///////////////////////////////

    //// Calculate the elapsed time for the animation
    //// todo: If slowdown on idle is enabled the delta time will be wrong!
    //_frameTimeMS      = timeMilliSec() - _lastUpdateTimeMS;
    //_lastUpdateTimeMS = timeMilliSec();

    //// Sum up all timings of all sceneviews
    //SLfloat sumCullTimeMS   = 0.0f;
    //SLfloat sumDraw3DTimeMS = 0.0f;
    //SLfloat sumDraw2DTimeMS = 0.0f;
    //SLbool  renderTypeIsRT  = false;
    //SLbool  voxelsAreShown  = false;
    //for (auto sv : _sceneViews)
    //{
    //    if (sv != nullptr)
    //    {
    //        sumCullTimeMS += sv->cullTimeMS();
    //        sumDraw3DTimeMS += sv->draw3DTimeMS();
    //        sumDraw2DTimeMS += sv->draw2DTimeMS();
    //        if (!renderTypeIsRT && sv->renderType() == RT_rt)
    //            renderTypeIsRT = true;
    //        if (!voxelsAreShown && sv->drawBit(SL_DB_VOXELS))
    //            voxelsAreShown = true;
    //    }
    //}
    //_cullTimesMS.set(sumCullTimeMS);
    //_draw3DTimesMS.set(sumDraw3DTimeMS);
    //_draw2DTimesMS.set(sumDraw2DTimeMS);

    //// Calculate the frames per second metric
    //_frameTimesMS.set(_frameTimeMS);
    //_fps = 1 / _frameTimesMS.average() * 1000.0f;
    //if (_fps < 0.0f) _fps = 0.0f;

    //SLfloat startUpdateMS = timeMilliSec();

    ////////////////////////////////
    //// 2) Process queued events //
    ////////////////////////////////

    //// Process queued up system events and poll custom input devices
    //SLbool sceneHasChanged = SLApplication::inputManager.pollAndProcessEvents();

    ////////////////////////////////
    //// 3) Update all animations //
    ////////////////////////////////

    //SLfloat startAnimUpdateMS = timeMilliSec();

    //if (_root3D)
    //    _root3D->update();

    //// reset the dirty flag on all skeletons
    //for (auto skeleton : _animManager.skeletons())
    //    skeleton->changed(false);

    //sceneHasChanged |= !_stopAnimations && _animManager.update(elapsedTimeSec());

    //// Do software skinning on all changed skeletons
    //for (auto mesh : _meshes)
    //{
    //    if (mesh->skeleton() && mesh->skeleton()->changed())
    //    {
    //        mesh->transformSkin();
    //        sceneHasChanged = true;
    //    }

    //    // update any out of date acceleration structure for RT or if they're being rendered.
    //    if (renderTypeIsRT || voxelsAreShown)
    //        mesh->updateAccelStruct();
    //}

    //_updateAnimTimesMS.set(timeMilliSec() - startAnimUpdateMS);

    //////////////////////
    //// 4) AR Tracking //
    //////////////////////

    //if (_videoType != VT_NONE && !SLCVCapture::lastFrame.empty())
    //{
    //    SLfloat          trackingTimeStartMS = timeMilliSec();
    //    SLCVCalibration* ac                  = SLApplication::activeCalib;

    //    // Invalidate calibration if camera input aspect doesn't match output
    //    SLfloat calibWdivH              = ac->imageAspectRatio();
    //    SLbool  aspectRatioDoesNotMatch = SL_abs(_sceneViews[0]->scrWdivH() - calibWdivH) > 0.01f;
    //    if (aspectRatioDoesNotMatch && ac->state() == CS_calibrated)
    //    {
    //        ac->clear();
    //    }

    //    stringstream ss; // info line text

    //    //.....................................................................
    //    if (ac->state() == CS_uncalibrated)
    //    {
    //        if (SLApplication::sceneID == SID_VideoCalibrateMain ||
    //            SLApplication::sceneID == SID_VideoCalibrateScnd)
    //        {
    //            ac->state(CS_calibrateStream);
    //        }
    //        else
    //        { // Changes the state to CS_guessed
    //            ac->createFromGuessedFOV(SLCVCapture::lastFrame.cols,
    //                                     SLCVCapture::lastFrame.rows);
    //            _sceneViews[0]->camera()->fov(ac->cameraFovVDeg());
    //        }
    //    }
    //    else //..............................................................
    //      if (ac->state() == CS_calibrateStream || ac->state() == CS_calibrateGrab)
    //    {
    //        ac->findChessboard(SLCVCapture::lastFrame, SLCVCapture::lastFrameGray, true);
    //        int imgsToCap = ac->numImgsToCapture();
    //        int imgsCaped = ac->numCapturedImgs();

    //        //update info line
    //        if (imgsCaped < imgsToCap)
    //            ss << "Click on the screen to create a calibration photo. Created "
    //               << imgsCaped << " of " << imgsToCap;
    //        else
    //        {
    //            ss << "Calculating calibration, please wait ...";
    //            ac->state(CS_startCalculating);
    //        }
    //        _info = ss.str();
    //    }
    //    else //..............................................................
    //      if (ac->state() == CS_startCalculating)
    //    {
    //        if (ac->calculate())
    //        {
    //            _sceneViews[0]->camera()->fov(ac->cameraFovVDeg());
    //            if (SLApplication::sceneID == SID_VideoCalibrateMain)
    //                onLoad(this, _sceneViews[0], SID_VideoTrackChessMain);
    //            else
    //                onLoad(this, _sceneViews[0], SID_VideoTrackChessScnd);
    //        }
    //    }
    //    else if (ac->state() == CS_calibrated || ac->state() == CS_guessed) //......
    //    {
    //        SLCVTrackedAruco::trackAllOnce = true;

    //        // track all trackers in the first sceneview
    //        for (auto tracker : _trackers)
    //        {
    //            tracker->track(SLCVCapture::lastFrameGray,
    //                           SLCVCapture::lastFrame,
    //                           ac,
    //                           _showDetection,
    //                           _sceneViews[0]);
    //        }

    //        // Update info text only for chessboard scene
    //        if (SLApplication::sceneID == SID_VideoCalibrateMain ||
    //            SLApplication::sceneID == SID_VideoCalibrateScnd ||
    //            SLApplication::sceneID == SID_VideoTrackChessMain ||
    //            SLApplication::sceneID == SID_VideoTrackChessScnd)
    //        {
    //            SLfloat fovH = ac->cameraFovHDeg();
    //            SLfloat err = ac->reprojectionError();
    //            ss << "Tracking Chessboard on " << (_videoType == VT_MAIN ? "main " : "scnd. ") << "camera. ";
    //            if (ac->state() == CS_calibrated)
    //                ss << "FOVH: " << fovH << ", error: " << err;
    //            else
    //                ss << "Not calibrated. FOVH guessed: " << fovH << " degrees.";
    //            _info = ss.str();
    //        }
    //    } //...................................................................

    //    //copy image to video texture
    //    if (ac->state() == CS_calibrated && ac->showUndistorted())
    //    {
    //        SLCVMat undistorted;
    //        ac->remap(SLCVCapture::lastFrame, undistorted);

    //        _videoTexture.copyVideoImage(undistorted.cols,
    //                                     undistorted.rows,
    //                                     SLCVCapture::format,
    //                                     undistorted.data,
    //                                     undistorted.isContinuous(),
    //                                     true);
    //    }
    //    else
    //    {
    //        _videoTexture.copyVideoImage(SLCVCapture::lastFrame.cols,
    //                                     SLCVCapture::lastFrame.rows,
    //                                     SLCVCapture::format,
    //                                     SLCVCapture::lastFrame.data,
    //                                     SLCVCapture::lastFrame.isContinuous(),
    //                                     true);
    //    }

    //    _trackingTimesMS.set(timeMilliSec() - trackingTimeStartMS);
    //}

    ///////////////////////
    //// 5) Update AABBs //
    ///////////////////////

    //// The updateAABBRec call won't generate any overhead if nothing changed
    //SLfloat startAAABBUpdateMS = timeMilliSec();
    //SLNode::numWMUpdates       = 0;
    //SLGLState::getInstance()->modelViewMatrix.identity();
    //if (_root3D)
    //    _root3D->updateAABBRec();
    //if (_root2D)
    //    _root2D->updateAABBRec();
    //_updateAABBTimesMS.set(timeMilliSec() - startAAABBUpdateMS);

    //// Finish total update time
    //SLfloat updateTimeMS = timeMilliSec() - startUpdateMS;
    //_updateTimesMS.set(updateTimeMS);

    //// calculate vsync time as diff. of major part times to the frame time
    //SLfloat totalMajorTime = captureTimesMS().average() +
    //                         updateTimeMS +
    //                         sumCullTimeMS +
    //                         sumDraw3DTimeMS +
    //                         sumDraw2DTimeMS;
    //SLfloat vsyncTime = _frameTimeMS > totalMajorTime ? _frameTimeMS - totalMajorTime : 0.0f;
    //_vsyncTimesMS.set(vsyncTime);

    ////SL_LOG("SLScene::onUpdate\n");
    //return sceneHasChanged;

    return false;
}
//-----------------------------------------------------------------------------
//! SLScene::onAfterLoad gets called after onLoad
void SLScene::onAfterLoad(SLSceneView* sv)
{
#ifdef SL_USES_CVCAPTURE

    if (_videoType != VT_NONE)
    {
        if (!SLCVCapture::isOpened())
        {
            SLVec2i videoSize;
            if (_videoType == VT_FILE && SLCVCapture::videoFilename != "")
                videoSize = SLCVCapture::openFile();
            else
                videoSize = SLCVCapture::open(0);

            if (videoSize != SLVec2i::ZERO)
            {
                SLCVCapture::grabAndAdjustForSL(sv->scrWdivH());

                if (SLCVCapture::lastFrame.cols > 0 &&
                    SLCVCapture::lastFrame.rows > 0)
                {
                    _videoTexture.copyVideoImage(SLCVCapture::lastFrame.cols,
                                                 SLCVCapture::lastFrame.rows,
                                                 SLCVCapture::format,
                                                 SLCVCapture::lastFrame.data,
                                                 SLCVCapture::lastFrame.isContinuous(),
                                                 true);
                }
            }
        }
    }
#else
    if (_videoType == VT_FILE && SLCVCapture::videoFilename != "")
    {
        if (!SLCVCapture::isOpened())
        {
            SLVec2i videoSize = SLCVCapture::openFile();

            if (videoSize != SLVec2i::ZERO)
            {
                if (SLCVCapture::lastFrame.cols > 0 &&
                    SLCVCapture::lastFrame.rows > 0)
                {
                    _videoTexture.copyVideoImage(SLCVCapture::lastFrame.cols,
                                                 SLCVCapture::lastFrame.rows,
                                                 SLCVCapture::format,
                                                 SLCVCapture::lastFrame.data,
                                                 SLCVCapture::lastFrame.isContinuous(),
                                                 true);
                }
            }
        }
    }
#endif
}
//-----------------------------------------------------------------------------
//! Sets the _selectedNode to the passed node and flags it as selected
/*! If one node is selected a rectangle selection is reset to zero.
The drawing of the selection is done in SLMesh::draw and SLAABBox::drawWS.
*/
void SLScene::selectNode(SLNode* nodeToSelect)
{
    if (_selectedNode)
        _selectedNode->drawBits()->off(SL_DB_SELECTED);

    if (nodeToSelect)
    {
        if (_selectedNode == nodeToSelect)
        {
            _selectedNode = nullptr;
        }
        else
        {
            _selectedNode = nodeToSelect;
            _selectedNode->drawBits()->on(SL_DB_SELECTED);
        }
        _selectedRect.setZero();
    }
    else
        _selectedNode = nullptr;
    _selectedMesh = nullptr;
}
//-----------------------------------------------------------------------------
//! Sets the _selectedNode and _selectedMesh and flags it as selected
/*! If one node is selected a rectangle selection is reset to zero.
The drawing of the selection is done in SLMesh::draw and SLAABBox::drawWS.
*/
void SLScene::selectNodeMesh(SLNode* nodeToSelect,
                             SLMesh* meshToSelect)
{
    if (_selectedNode)
        _selectedNode->drawBits()->off(SL_DB_SELECTED);

    if (nodeToSelect)
    {
        if (_selectedNode == nodeToSelect && _selectedMesh == meshToSelect)
        {
            _selectedNode = nullptr;
            _selectedMesh = nullptr;
            _selectedRect.setZero();
        }
        else
        {
            _selectedNode = nodeToSelect;
            _selectedMesh = meshToSelect;
            _selectedNode->drawBits()->on(SL_DB_SELECTED);
        }
    }
    else
    {
        _selectedNode = nullptr;
        _selectedMesh = nullptr;
        _selectedRect.setZero();
    }
}
//-----------------------------------------------------------------------------
void SLScene::onLoadAsset(SLstring assetFile,
                          SLuint   processFlags)
{
    SLApplication::sceneID = SID_FromFile;

    // Set scene name for new scenes
    if (!_root3D)
        name(Utils::getFileName(assetFile));

    // Try to load assed and add it to the scene root node
    SLAssimpImporter importer;

    ///////////////////////////////////////////////////////////////////////
    SLNode* loaded = importer.load(assetFile, true, nullptr, processFlags);
    ///////////////////////////////////////////////////////////////////////

    // Add root node on empty scene
    if (!_root3D)
    {
        SLNode* scene = new SLNode("Scene");
        _root3D       = scene;
    }

    // Add loaded scene
    if (loaded)
        _root3D->addChild(loaded);

    // Add directional light if no light was in loaded asset
    if (!_lights.size())
    {
        SLAABBox boundingBox = _root3D->updateAABBRec();
        SLfloat  arrowLength = boundingBox.radiusWS() > FLT_EPSILON
                                ? boundingBox.radiusWS() * 0.1f
                                : 0.5f;
        SLLightDirect* light = new SLLightDirect(0, 0, 0, arrowLength, 1.0f, 1.0f, 1.0f);
        SLVec3f        pos   = boundingBox.maxWS().isZero()
                        ? SLVec3f(1, 1, 1)
                        : boundingBox.maxWS() * 1.1f;
        light->translation(pos);
        light->lookAt(pos - SLVec3f(1, 1, 1));
        light->attenuation(1, 0, 0);
        _root3D->addChild(light);
        _root3D->aabb()->reset(); // rest aabb so that it is recalculated
    }

    // call onInitialize on all scene views
    //for (auto sv : _sceneViews)
    //{
    //    if (sv != nullptr)
    //    {
    //        sv->onInitialize();
    //    }
    //}
}
//-----------------------------------------------------------------------------
//! Setter for video type also sets the active calibration
/*! The SLApplication instance has up to three video camera calibrations, one
for a main camera (SLApplication::calibMainCam), one for the selfie camera on
mobile devices (SLApplication::calibScndCam) and one for video file simulation
(SLApplication::calibVideoFile). The member SLApplication::activeCalib
references the active one.
*/
void SLScene::videoType(SLVideoType vt)
{
    _videoType = vt;

    if (vt == VT_SCND)
    {
        if (SLCVCapture::hasSecondaryCamera)
            SLApplication::activeCalib = &SLApplication::calibScndCam;
        else //fallback if there is no secondary camera we use main setup
        {
            _videoType                 = VT_MAIN;
            SLApplication::activeCalib = &SLApplication::calibMainCam;
        }
    }
    else if (vt == VT_FILE)
        SLApplication::activeCalib = &SLApplication::calibVideoFile;
    else //VT_MAIN and VT_NONE
        SLApplication::activeCalib = &SLApplication::calibMainCam;
}
//-----------------------------------------------------------------------------
//! Returns the number of camera nodes in the scene
SLint SLScene::numSceneCameras()
{
    if (!_root3D) return 0;
    vector<SLCamera*> cams = _root3D->findChildren<SLCamera>();
    return (SLint)cams.size();
}
//-----------------------------------------------------------------------------
//! Returns the next camera in the scene if there is one
SLCamera* SLScene::nextCameraInScene(SLSceneView* activeSV)
{
    if (!_root3D) return nullptr;

    vector<SLCamera*> cams = _root3D->findChildren<SLCamera>();

    if (cams.size() == 0) return nullptr;
    if (cams.size() == 1) return cams[0];

    SLint activeIndex = 0;
    for (SLuint i = 0; i < cams.size(); ++i)
    {
        if (cams[i] == activeSV->camera())
        {
            activeIndex = (SLint)i;
            break;
        }
    }

    //find next camera, that is not of type SLCVCamera if "allow SLCVCamera as
    //active camera" is deactivated
    do
    {
        activeIndex = activeIndex > cams.size() - 2 ? 0 : ++activeIndex;
    } while (dynamic_cast<SLKeyframeCamera*>(cams[(uint)activeIndex]) &&
             !dynamic_cast<SLKeyframeCamera*>(cams[(uint)activeIndex])->allowAsActiveCam());

    return cams[(uint)activeIndex];
}
//-----------------------------------------------------------------------------
/*! Removes the specified mesh from the meshes resource vector.
*/
bool SLScene::removeMesh(SLMesh* mesh)
{
    assert(mesh);
    for (SLuint i = 0; i < _meshes.size(); ++i)
    {
        if (_meshes[i] == mesh)
        {
            _meshes.erase(_meshes.begin() + i);
            return true;
        }
    }
    return false;
}
//-----------------------------------------------------------------------------
/*! Removes the specified texture from the textures resource vector.
*/
bool SLScene::deleteTexture(SLGLTexture* texture)
{
    assert(texture);
    for (SLuint i = 0; i < _textures.size(); ++i)
    {
        if (_textures[i] == texture)
        {
            delete _textures[i];
            _textures.erase(_textures.begin() + i);
            return true;
        }
    }
    return false;
}
//----------------------------------------------------------------------------
