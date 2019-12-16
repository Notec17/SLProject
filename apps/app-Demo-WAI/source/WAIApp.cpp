#include <WAIApp.h>

#include <SLApplication.h>
#include <SLInterface.h>
#include <SLKeyframeCamera.h>
#include <CVCapture.h>
#include <Utils.h>
#include <AverageTiming.h>

#include <WAIModeOrbSlam2.h>
#include <WAIMapStorage.h>
#include <WAICalibration.h>
#include <AppWAIScene.h>

#include <AppDemoGuiInfosDialog.h>
#include <AppDemoGuiAbout.h>
#include <AppDemoGuiInfosFrameworks.h>
#include <AppDemoGuiInfosMapNodeTransform.h>
#include <AppDemoGuiInfosScene.h>
#include <AppDemoGuiInfosSensors.h>
#include <AppDemoGuiInfosTracking.h>
#include <AppDemoGuiProperties.h>
#include <AppDemoGuiSceneGraph.h>
#include <AppDemoGuiStatsDebugTiming.h>
#include <AppDemoGuiStatsTiming.h>
#include <AppDemoGuiStatsVideo.h>
#include <AppDemoGuiTrackedMapping.h>
#include <AppDemoGuiTransform.h>
#include <AppDemoGuiUIPrefs.h>
#include <AppDemoGuiVideoControls.h>
#include <AppDemoGuiVideoStorage.h>
#include <AppDemoGuiSlamLoad.h>
#include <AppDemoGuiTestOpen.h>
#include <AppDemoGuiTestWrite.h>
#include <AppDemoGuiSlamParam.h>
#include <AppDemoGuiError.h>

#include <AppDirectories.h>
#include <AppWAISlamParamHelper.h>
#include <FtpUtils.h>
#include <GUIPreferences.h>

#include <SLSceneView.h>
#include <SLPoints.h>
#include <SLQuat4.h>
#include <SLPolyline.h>

//move
#include <SLAssimpImporter.h>

//basic information
//-the sceen has a fixed size on android and also a fixed aspect ratio on desktop to simulate android behaviour on desktop
//-the viewport gets adjusted according to the target video aspect ratio (for live video WAIApp::liveVideoTargetWidth and WAIApp::_liveVideoTargetHeight are used and for video file the video frame size is used)
//-the live video gets cropped according to the viewport aspect ratio on android and according to WAIApp::videoFrameWdivH on desktop (which is also used to define the viewport aspect ratio)
//-the calibration gets adjusted according to the video (live and file)
//-the live video gets cropped to the aspect ratio that is defined by the transferred values in load(..) and assigned to liveVideoTargetWidth and liveVideoTargetHeight

//-----------------------------------------------------------------------------
WAIApp::WAIApp()
  : SLInputEventInterface(SLApplication::inputManager) //todo: local input manager
{
}
//-----------------------------------------------------------------------------
WAIApp::~WAIApp()
{
    //todo: destructor is not called on android (at the right position)
    if (_videoWriter)
        delete _videoWriter;
    if (_videoWriterInfo)
        delete _videoWriterInfo;
}

//-----------------------------------------------------------------------------
int WAIApp::load(int liveVideoTargetW, int liveVideoTargetH, int scrWidth, int scrHeight, float scr2fbX, float scr2fbY, int dpi, AppDirectories directories)
{
    _liveVideoTargetWidth  = liveVideoTargetW;
    _liveVideoTargetHeight = liveVideoTargetH;

    _dirs = directories;

    SLApplication::devRot.isUsed(true);
    SLApplication::devLoc.isUsed(true);

    videoDir       = _dirs.writableDir + "erleb-AR/locations/";
    calibDir       = _dirs.writableDir + "calibrations/";
    mapDir         = _dirs.writableDir + "maps/";
    vocDir         = _dirs.writableDir + "voc/";
    experimentsDir = _dirs.writableDir + "experiments/";

    _waiScene        = std::make_unique<AppWAIScene>();
    _videoWriter     = new cv::VideoWriter();
    _videoWriterInfo = new cv::VideoWriter();
    _loaded          = true;

    setupDefaultErlebARDir();
    int svIndex = initSLProject(scrWidth, scrHeight, scr2fbX, scr2fbY, dpi);
    return svIndex;
}

//-----------------------------------------------------------------------------
bool WAIApp::update()
{
    AVERAGE_TIMING_START("WAIAppUpdate");
    //resetAllTexture();
    if (_mode && _loaded)
    {
        if (CVCapture::instance()->lastFrame.empty() ||
            CVCapture::instance()->lastFrame.cols == 0 && CVCapture::instance()->lastFrame.rows == 0)
        {
            //this only has an influence on desktop or video file
            CVCapture::instance()->grabAndAdjustForSL(_videoFrameWdivH);
            return false;
        }

        bool iKnowWhereIAm = (_mode->getTrackingState() == WAI::TrackingState_TrackingOK);
        while (videoCursorMoveIndex < 0)
        {
            //this only has an influence on desktop or video file
            CVCapture::instance()->moveCapturePosition(-2);
            CVCapture::instance()->grabAndAdjustForSL(_videoFrameWdivH);
            iKnowWhereIAm = updateTracking();

            videoCursorMoveIndex++;
        }

        while (videoCursorMoveIndex > 0)
        {
            //this only has an influence on desktop or video file
            CVCapture::instance()->grabAndAdjustForSL(_videoFrameWdivH);
            iKnowWhereIAm = updateTracking();

            videoCursorMoveIndex--;
        }

        if (CVCapture::instance()->videoType() != VT_NONE)
        {
            //this only has an influence on desktop or video file
            if (CVCapture::instance()->videoType() != VT_FILE || !pauseVideo)
            {
                CVCapture::instance()->grabAndAdjustForSL(_videoFrameWdivH);
                iKnowWhereIAm = updateTracking();
            }
        }

        //update tracking infos visualization
        updateTrackingVisualization(iKnowWhereIAm);

        if (iKnowWhereIAm)
        {
            _lastKnowPoseQuaternion = SLApplication::devRot.quaternion();
            _IMUQuaternion          = SLQuat4f(0, 0, 0, 1);

            // TODO(dgj1): maybe make this API cleaner
            cv::Mat pose = cv::Mat(4, 4, CV_32F);
            if (!_mode->getPose(&pose))
            {
                return false;
            }

            // update camera node position
            cv::Mat Rwc(3, 3, CV_32F);
            cv::Mat twc(3, 1, CV_32F);

            Rwc = (pose.rowRange(0, 3).colRange(0, 3)).t();
            twc = -Rwc * pose.rowRange(0, 3).col(3);

            cv::Mat PoseInv = cv::Mat::eye(4, 4, CV_32F);

            Rwc.copyTo(PoseInv.colRange(0, 3).rowRange(0, 3));
            twc.copyTo(PoseInv.rowRange(0, 3).col(3));
            SLMat4f om;

            om.setMatrix(PoseInv.at<float>(0, 0),
                         -PoseInv.at<float>(0, 1),
                         -PoseInv.at<float>(0, 2),
                         PoseInv.at<float>(0, 3),
                         PoseInv.at<float>(1, 0),
                         -PoseInv.at<float>(1, 1),
                         -PoseInv.at<float>(1, 2),
                         PoseInv.at<float>(1, 3),
                         PoseInv.at<float>(2, 0),
                         -PoseInv.at<float>(2, 1),
                         -PoseInv.at<float>(2, 2),
                         PoseInv.at<float>(2, 3),
                         PoseInv.at<float>(3, 0),
                         -PoseInv.at<float>(3, 1),
                         -PoseInv.at<float>(3, 2),
                         PoseInv.at<float>(3, 3));

            _waiScene->cameraNode->om(om);
        }
        else
        {
            SLQuat4f q1 = _lastKnowPoseQuaternion;
            SLQuat4f q2 = SLApplication::devRot.quaternion();
            q1.invert();
            SLQuat4f q              = q1 * q2;
            _IMUQuaternion          = SLQuat4f(q.y(), -q.x(), -q.z(), -q.w());
            SLMat4f imuRot          = _IMUQuaternion.toMat4();
            _lastKnowPoseQuaternion = q2;

            SLMat4f cameraMat = _waiScene->cameraNode->om();
            _waiScene->cameraNode->om(cameraMat * imuRot);
        }

        AVERAGE_TIMING_STOP("WAIAppUpdate");
    }

    //update scene (before it was slUpdateScene)
    SLApplication::scene->onUpdate();
    return updateSceneViews();
}

//-----------------------------------------------------------------------------
void WAIApp::close()
{
    // Deletes all remaining sceneviews the current scene instance
    SLApplication::deleteAppAndScene();
}

//-----------------------------------------------------------------------------
/*
videoFile: path to a video or empty if live video should be used
calibrationFile: path to a calibration or empty if calibration should be searched automatically
mapFile: path to a map or empty if no map should be used
*/
OrbSlamStartResult WAIApp::startOrbSlam(SlamParams* slamParams)
{
    OrbSlamStartResult result = {};
    //TODO:find other solution
    _gui->uiPrefs->showError = false;

    std::string               videoFile       = "";
    std::string               calibrationFile = "";
    std::string               mapFile         = "";
    std::string               vocFile         = "";
    std::string               markerFile      = "";
    WAI::ModeOrbSlam2::Params params;

    if (slamParams)
    {
        videoFile       = slamParams->videoFile;
        calibrationFile = slamParams->calibrationFile;
        mapFile         = slamParams->mapFile;
        vocFile         = slamParams->vocabularyFile;
        markerFile      = slamParams->markerFile;
        params          = slamParams->params;
    }

    bool useVideoFile             = !videoFile.empty();
    bool detectCalibAutomatically = calibrationFile.empty();
    bool useMapFile               = !mapFile.empty();

    // reset stuff
    if (_mode)
    {
        _mode->requestStateIdle();
        while (!_mode->hasStateIdle())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        delete _mode;
        _mode = nullptr;
    }

    // Check that files exist
    if (useVideoFile && !Utils::fileExists(videoFile))
    {
        result.errorString = "Video file " + videoFile + " does not exist.";
        return result;
    }

    // determine correct calibration file
    std::string calibrationFileName;
    if (detectCalibAutomatically)
    {
        std::string computerInfo;

        if (useVideoFile)
        {
            SlamVideoInfos slamVideoInfos;
            std::string    videoFileName = Utils::getFileNameWOExt(videoFile);

            if (!extractSlamVideoInfosFromFileName(videoFileName, &slamVideoInfos))
            {
                result.errorString = "Could not extract computer infos from video filename.";
                return result;
            }

            computerInfo = slamVideoInfos.deviceString;
        }
        else
        {
            computerInfo = SLApplication::getComputerInfos();
        }

        calibrationFileName = "camCalib_" + computerInfo + "_main.xml";
        calibrationFile     = calibDir + calibrationFileName;
    }
    else
    {
        calibrationFileName = Utils::getFileName(calibrationFile);
    }

    if (!Utils::fileExists(calibrationFile))
    {
        result.errorString = "Calibration file " + calibrationFile + " does not exist.";
        return result;
    }

    if (!checkCalibration(calibDir, calibrationFileName))
    {
        result.errorString = "Calibration file " + calibrationFile + " is incorrect.";
        return result;
    }

    if (vocFile.empty())
    {
        vocFile = vocDir + "ORBvoc.bin";
    }

    if (!Utils::fileExists(vocFile))
    {
        result.errorString = "Vocabulary file does not exist: " + vocFile;
        return result;
    }

    if (useMapFile && !Utils::fileExists(mapFile))
    {
        result.errorString = "Map file " + mapFile + " does not exist.";
        return result;
    }

    CVCapture* cap = CVCapture::instance();

    // 1. Initialize CVCapture with either video file or live video
    if (useVideoFile)
    {
        cap->videoType(VT_FILE);
        cap->videoFilename = videoFile;
        cap->videoLoops    = true;
        _videoFrameSize    = cap->openFile();
    }
    else
    {
        cap->videoType(VT_MAIN);
        //open(0) only has an effect on desktop. On Android it just returns {0,0}
        cap->open(0);
        _videoFrameSize = cv::Size2i(_liveVideoTargetWidth, _liveVideoTargetHeight);
    }
    _videoFrameWdivH = (float)_videoFrameSize.width / (float)_videoFrameSize.height;

    // 2. Load Calibration
    if (!cap->activeCamera->calibration.load(calibDir, Utils::getFileName(calibrationFile)))
    {
        result.errorString = "Error when loading calibration from file: " +
                             calibrationFile;
        return result;
    }

    if (cap->activeCamera->calibration.imageSize() != _videoFrameSize)
    {
        cap->activeCamera->calibration.adaptForNewResolution(_videoFrameSize);
    }

    // 3. Adjust FOV of camera node according to new calibration (fov is used in projection->prespective _mode)
    _waiScene->cameraNode->fov(cap->activeCamera->calibration.cameraFovVDeg());
    // Set camera intrinsics for scene camera frustum. (used in projection->intrinsics mode)
    cv::Mat scMat = cap->activeCamera->calibration.cameraMatUndistorted();
    std::cout << "scMat: " << scMat << std::endl;
    _waiScene->cameraNode->intrinsics(scMat.at<double>(0, 0),
                                      scMat.at<double>(1, 1),
                                      scMat.at<double>(0, 2),
                                      scMat.at<double>(1, 2));
    //enable projection -> intrinsics mode
    _waiScene->cameraNode->projection(P_monoIntrinsic);
    //enable image undistortion
    cap->activeCamera->showUndistorted(true);

    // 4. Create new mode ORBSlam
    if (!markerFile.empty())
    {
        params.cullRedundantPerc = 0.99f;
    }

    _mode = new WAI::ModeOrbSlam2(cap->activeCamera->calibration.cameraMat(),
                                  cap->activeCamera->calibration.distortion(),
                                  params,
                                  vocFile,
                                  markerFile);

    // 5. Load map data
    if (useMapFile)
    {
        _mode->requestStateIdle();
        while (!_mode->hasStateIdle())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        _mode->reset();

        // TODO(dgj1): extract feature type
        // TODO(dgj1): check that map feature type matches with _mode feature type
        bool mapLoadingSuccess = WAIMapStorage::loadMap(_mode->getMap(),
                                                        _mode->getKfDB(),
                                                        _waiScene->mapNode,
                                                        mapFile,
                                                        _mode->retainImage(),
                                                        params.fixOldKfs);

        if (!mapLoadingSuccess)
        {
            delete _mode;
            _mode = nullptr;

            result.errorString = "Could not load map from file " + mapFile;
            return result;
        }

        _mode->resume();
        _mode->setInitialized(true);
    }

    // 6. save current params
    _currentSlamParams.calibrationFile  = calibrationFile;
    _currentSlamParams.mapFile          = mapFile;
    _currentSlamParams.params.retainImg = params.retainImg;
    _currentSlamParams.videoFile        = videoFile;
    _currentSlamParams.vocabularyFile   = vocFile;

    result.wasSuccessful = true;

    _sv->setViewportFromRatio(SLVec2i(_videoFrameSize.width, _videoFrameSize.height), SLViewportAlign::VA_center, true);
    //_resizeWindow = true;

    return result;
}

//-----------------------------------------------------------------------------
void WAIApp::showErrorMsg(std::string msg)
{
    assert(_errorDial && "errorDial is not initialized");

    _errorDial->setErrorMsg(msg);
    _gui->uiPrefs->showError = true;
}

//-----------------------------------------------------------------------------
std::string WAIApp::name()
{
    return SLApplication::name;
}

//-----------------------------------------------------------------------------
void WAIApp::setDeviceParameter(const std::string& parameter,
                                std::string        value)
{
    SLApplication::deviceParameter[parameter] = std::move(value);
}
//-----------------------------------------------------------------------------
void WAIApp::setRotationQuat(float quatX,
                             float quatY,
                             float quatZ,
                             float quatW)
{
    //todo: replace
    SLApplication::devRot.onRotationQUAT(quatX, quatY, quatZ, quatW);
}

//-----------------------------------------------------------------------------
bool WAIApp::usesRotationSensor()
{
    //todo: replace
    return SLApplication::devRot.isUsed();
}

//-----------------------------------------------------------------------------
void WAIApp::setLocationLLA(float latitudeDEG, float longitudeDEG, float altitudeM, float accuracyM)
{
    //todo: replace
    SLApplication::devLoc.onLocationLLA(latitudeDEG,
                                        longitudeDEG,
                                        altitudeM,
                                        accuracyM);
}
//-----------------------------------------------------------------------------
bool WAIApp::usesLocationSensor()
{
    //todo: replace
    return SLApplication::devLoc.isUsed();
}

//-----------------------------------------------------------------------------
void WAIApp::initExternalDataDirectory(std::string path)
{
    if (Utils::dirExists(path))
    {
        Utils::log("External directory: %s\n", path.c_str());
        SLApplication::externalPath = path;
    }
    else
    {
        Utils::log("ERROR: external directory does not exists: %s\n", path.c_str());
    }
}

//-----------------------------------------------------------------------------
bool WAIApp::updateTracking()
{
    bool iKnowWhereIAm = false;

    CVCapture* cap = CVCapture::instance();
    if (cap->videoType() != VT_NONE && !cap->lastFrame.empty())
    {
        if (_videoWriter->isOpened())
        {
            _videoWriter->write(cap->lastFrame);
        }

        iKnowWhereIAm = _mode->update(cap->lastFrameGray,
                                      cap->lastFrame);

        if (_videoWriterInfo->isOpened())
        {
            _videoWriterInfo->write(cap->lastFrame);
        }

        if (_gpsDataStream.is_open())
        {
            if (SLApplication::devLoc.isUsed())
            {
                SLVec3d v = SLApplication::devLoc.locLLA();
                _gpsDataStream << SLApplication::devLoc.locAccuracyM();
                _gpsDataStream << std::to_string(v.x) + " " + std::to_string(v.y) + " " + std::to_string(v.z);
                _gpsDataStream << std::to_string(SLApplication::devRot.yawRAD());
                _gpsDataStream << std::to_string(SLApplication::devRot.pitchRAD());
                _gpsDataStream << std::to_string(SLApplication::devRot.rollRAD());
            }
        }
    }

    return iKnowWhereIAm;
}

//-----------------------------------------------------------------------------
bool WAIApp::initSLProject(int scrWidth, int scrHeight, float scr2fbX, float scr2fbY, int dpi)
{
    assert(SLApplication::scene == nullptr && "SLScene is already created!");

    // Default paths for all loaded resources
    SLGLProgram::defaultPath      = _dirs.slDataRoot + "/shaders/";
    SLGLTexture::defaultPath      = _dirs.slDataRoot + "/images/textures/";
    SLGLTexture::defaultPathFonts = _dirs.slDataRoot + "/images/fonts/";
    SLAssimpImporter::defaultPath = _dirs.slDataRoot + "/models/";
    SLApplication::configPath     = _dirs.writableDir;

    SLApplication::name  = "WAI Demo App";
    SLApplication::scene = new SLScene("WAI Demo App", nullptr);

    int screenWidth  = (int)(scrWidth * scr2fbX);
    int screenHeight = (int)(scrHeight * scr2fbY);
    assert(SLApplication::scene && "No SLApplication::scene!");

    setupGUI(SLApplication::name, SLApplication::configPath, dpi);
    // Set default font sizes depending on the dpi no matter if ImGui is used
    //todo: is this still needed?
    if (!SLApplication::dpi)
        SLApplication::dpi = dpi;

    _sv = new SLSceneView();
    _sv->init("SceneView",
              screenWidth,
              screenHeight,
              nullptr,
              nullptr,
              _gui.get());

    onLoadWAISceneView(SLApplication::scene, _sv);
    return (SLint)_sv->index();
}

//-----------------------------------------------------------------------------
void WAIApp::onLoadWAISceneView(SLScene* s, SLSceneView* sv)
{
    s->init();
    _waiScene->rebuild();

    // Set scene name and info string
    s->name("Track Keyframe based Features");
    s->info("Example for loading an existing pose graph with map points.");

    // Save no energy
    sv->doWaitOnIdle(false); //for constant video feed
    sv->camera(_waiScene->cameraNode);

    _videoImage = new SLGLTexture("LiveVideoError.png", GL_LINEAR, GL_LINEAR);
    //_testTexture = new SLGLTexture("LiveVideoError.png", GL_LINEAR, GL_LINEAR);
    _waiScene->cameraNode->background().texture(_videoImage);

    s->root3D(_waiScene->rootNode);

    sv->onInitialize();
    sv->doWaitOnIdle(false);

    /*OrbSlamStartResult orbSlamStartResult = startOrbSlam();

    if (!orbSlamStartResult.wasSuccessful)
    {
        errorDial->setErrorMsg(orbSlamStartResult.errorString);
        _gui->uiPrefs->showError = true;
    }*/

    sv->setViewportFromRatio(SLVec2i(_liveVideoTargetWidth, _liveVideoTargetHeight), SLViewportAlign::VA_center, true);

    //do once an onResize in update loop so that everything is aligned correctly
    //_resizeWindow = true;
}

//-----------------------------------------------------------------------------
void WAIApp::setupGUI(std::string appName, std::string configDir, int dotsPerInch)
{
    _gui = std::make_unique<AppDemoWaiGui>(SLApplication::name, SLApplication::configPath, dotsPerInch);
    //aboutDial = new AppDemoGuiAbout("about", cpvrLogo, &uiPrefs.showAbout);
    _gui->addInfoDialog(new AppDemoGuiInfosFrameworks("frameworks", &_gui->uiPrefs->showInfosFrameworks));
    _gui->addInfoDialog(new AppDemoGuiInfosMapNodeTransform("map node",
                                                            _waiScene->mapNode,
                                                            &_gui->uiPrefs->showInfosMapNodeTransform));

    _gui->addInfoDialog(new AppDemoGuiInfosScene("scene", &_gui->uiPrefs->showInfosScene));
    _gui->addInfoDialog(new AppDemoGuiInfosSensors("sensors", &_gui->uiPrefs->showInfosSensors));
    _gui->addInfoDialog(new AppDemoGuiInfosTracking("tracking", *_gui->uiPrefs.get(), *this));
    _gui->addInfoDialog(new AppDemoGuiSlamLoad("slam load",
                                               _dirs.writableDir + "erleb-AR/locations/",
                                               _dirs.writableDir + "calibrations/",
                                               _dirs.writableDir + "voc/",
                                               _waiScene->mapNode,
                                               &_gui->uiPrefs->showSlamLoad,
                                               *this,
                                               this->_currentSlamParams));

    _gui->addInfoDialog(new AppDemoGuiProperties("properties", &_gui->uiPrefs->showProperties));
    _gui->addInfoDialog(new AppDemoGuiSceneGraph("scene graph", &_gui->uiPrefs->showSceneGraph));
    _gui->addInfoDialog(new AppDemoGuiStatsDebugTiming("debug timing", &_gui->uiPrefs->showStatsDebugTiming));
    _gui->addInfoDialog(new AppDemoGuiStatsTiming("timing", &_gui->uiPrefs->showStatsTiming));
    _gui->addInfoDialog(new AppDemoGuiStatsVideo("video", &CVCapture::instance()->activeCamera->calibration, &_gui->uiPrefs->showStatsVideo));
    _gui->addInfoDialog(new AppDemoGuiTrackedMapping("tracked mapping", &_gui->uiPrefs->showTrackedMapping, *this));

    _gui->addInfoDialog(new AppDemoGuiTransform("transform", &_gui->uiPrefs->showTransform));
    _gui->addInfoDialog(new AppDemoGuiUIPrefs("prefs", _gui->uiPrefs.get(), &_gui->uiPrefs->showUIPrefs));

    _gui->addInfoDialog(new AppDemoGuiVideoStorage("video storage", _videoWriter, _videoWriterInfo, &_gpsDataStream, &_gui->uiPrefs->showVideoStorage, *this));
    _gui->addInfoDialog(new AppDemoGuiVideoControls("video load", &_gui->uiPrefs->showVideoControls, *this));

    _gui->addInfoDialog(new AppDemoGuiTestOpen("Tests Settings",
                                               _waiScene->mapNode,
                                               &_gui->uiPrefs->showTestSettings,
                                               *this));

    _gui->addInfoDialog(new AppDemoGuiTestWrite("Test Writer",
                                                &CVCapture::instance()->activeCamera->calibration,
                                                _waiScene->mapNode,
                                                _videoWriter,
                                                _videoWriterInfo,
                                                &_gpsDataStream,
                                                &_gui->uiPrefs->showTestWriter,
                                                *this));

    _gui->addInfoDialog(new AppDemoGuiSlamParam("Slam Param", &_gui->uiPrefs->showSlamParam, *this));

    _errorDial = new AppDemoGuiError("Error", &_gui->uiPrefs->showError);
    _gui->addInfoDialog(_errorDial);
    //TODO: AppDemoGuiInfosDialog are never deleted. Why not use smart pointer when the reponsibility for an object is not clear?
}

//-----------------------------------------------------------------------------
void WAIApp::setupDefaultErlebARDir()
{
    std::string dir = Utils::unifySlashes(_dirs.writableDir);
    if (!Utils::dirExists(dir))
    {
        Utils::makeDir(dir);
    }

    dir += "erleb-AR/";
    if (!Utils::dirExists(dir))
    {
        Utils::makeDir(dir);
    }

    dir += "locations/";
    if (!Utils::dirExists(dir))
    {
        Utils::makeDir(dir);
    }

    dir += "default/";
    if (!Utils::dirExists(dir))
    {
        Utils::makeDir(dir);
    }

    dir += "default/";
    if (!Utils::dirExists(dir))
    {
        Utils::makeDir(dir);
    }
}

//-----------------------------------------------------------------------------
bool WAIApp::checkCalibration(const std::string& calibDir, const std::string& calibFileName)
{
    CVCalibration testCalib(CVCameraType::FRONTFACING, "");
    testCalib.load(calibDir, calibFileName);
    if (testCalib.cameraMat().empty() || testCalib.distortion().empty()) //app will crash if distortion is empty
        return false;
    if (testCalib.numCapturedImgs() == 0) //if this is 0 then the calibration is automatically invalidated in load()
        return false;
    if (testCalib.imageSize() == cv::Size(0, 0))
        return false;

    return true;
}

//-----------------------------------------------------------------------------
bool WAIApp::updateSceneViews()
{
    bool needUpdate = false;

    for (auto sv : SLApplication::scene->sceneViews())
        if (sv->onPaint() && !needUpdate)
            needUpdate = true;

    return needUpdate;
}

//-----------------------------------------------------------------------------
void WAIApp::updateTrackingVisualization(const bool iKnowWhereIAm)
{
    CVCapture* cap = CVCapture::instance();
    //undistort image and copy image to video texture
    if (_videoImage && cap->activeCamera)
    {
        //decorate distorted image with distorted keypoints
        if (_gui->uiPrefs->showKeyPoints)
            _mode->decorateVideoWithKeyPoints(cap->lastFrame);
        if (_gui->uiPrefs->showKeyPointsMatched)
            _mode->decorateVideoWithKeyPointMatches(cap->lastFrame);

        CVMat undistortedLastFrame;
        if (cap->activeCamera->calibration.state() == CS_calibrated && cap->activeCamera->showUndistorted())
        {
            cap->activeCamera->calibration.remap(cap->lastFrame, undistortedLastFrame);
        }
        else
        {
            undistortedLastFrame = cap->lastFrame;
        }

        _videoImage->copyVideoImage(undistortedLastFrame.cols,
                                    undistortedLastFrame.rows,
                                    cap->format,
                                    undistortedLastFrame.data,
                                    undistortedLastFrame.isContinuous(),
                                    true);
    }

    //update map point visualization:
    //if we still want to visualize the point cloud
    if (_gui->uiPrefs->showMapPC)
    {
        //get new points and add them
        renderMapPoints("MapPoints",
                        _mode->getMapPoints(),
                        _waiScene->mapPC,
                        _waiScene->mappointsMesh,
                        _waiScene->redMat);

        //get new points and add them
        renderMapPoints("MarkerCornerMapPoints",
                        _mode->getMarkerCornerMapPoints(),
                        _waiScene->mapMarkerCornerPC,
                        _waiScene->mappointsMarkerCornerMesh,
                        _waiScene->blueMat);
    }
    else
    {
        if (_waiScene->mappointsMesh)
        {
            //delete mesh if we do not want do visualize it anymore
            _waiScene->mapPC->deleteMesh(_waiScene->mappointsMesh);
        }
        if (_waiScene->mappointsMarkerCornerMesh)
        {
            //delete mesh if we do not want do visualize it anymore
            _waiScene->mapMarkerCornerPC->deleteMesh(_waiScene->mappointsMarkerCornerMesh);
        }
    }

    //update visualization of local map points:
    //only update them with a valid pose from WAI
    if (_gui->uiPrefs->showLocalMapPC && iKnowWhereIAm)
    {
        renderMapPoints("LocalMapPoints",
                        _mode->getLocalMapPoints(),
                        _waiScene->mapLocalPC,
                        _waiScene->mappointsLocalMesh,
                        _waiScene->blueMat);
    }
    else if (_waiScene->mappointsLocalMesh)
    {
        //delete mesh if we do not want do visualize it anymore
        _waiScene->mapLocalPC->deleteMesh(_waiScene->mappointsLocalMesh);
    }

    //update visualization of matched map points
    //only update them with a valid pose from WAI
    if (_gui->uiPrefs->showMatchesPC && iKnowWhereIAm)
    {
        renderMapPoints("MatchedMapPoints",
                        _mode->getMatchedMapPoints(),
                        _waiScene->mapMatchedPC,
                        _waiScene->mappointsMatchedMesh,
                        _waiScene->greenMat);
    }
    else if (_waiScene->mappointsMatchedMesh)
    {
        //delete mesh if we do not want do visualize it anymore
        _waiScene->mapMatchedPC->deleteMesh(_waiScene->mappointsMatchedMesh);
    }

    //update keyframe visualization
    _waiScene->keyFrameNode->deleteChildren();
    if (_gui->uiPrefs->showKeyFrames)
    {
        renderKeyframes();
    }

    //update pose graph visualization
    renderGraphs();
}

//-----------------------------------------------------------------------------
void WAIApp::renderMapPoints(std::string                      name,
                             const std::vector<WAIMapPoint*>& pts,
                             SLNode*&                         node,
                             SLPoints*&                       mesh,
                             SLMaterial*&                     material)
{
    //remove old mesh, if it exists
    if (mesh)
        node->deleteMesh(mesh);

    //instantiate and add new mesh
    if (pts.size())
    {
        //get points as Vec3f
        std::vector<SLVec3f> points, normals;
        for (auto mapPt : pts)
        {
            WAI::V3 wP = mapPt->worldPosVec();
            WAI::V3 wN = mapPt->normalVec();
            points.push_back(SLVec3f(wP.x, wP.y, wP.z));
            normals.push_back(SLVec3f(wN.x, wN.y, wN.z));
        }

        mesh = new SLPoints(points, normals, name, material);
        node->addMesh(mesh);
        node->updateAABBRec();
    }
}
//-----------------------------------------------------------------------------
void WAIApp::renderKeyframes()
{
    std::vector<WAIKeyFrame*> keyframes = _mode->getKeyFrames();

    // TODO(jan): delete keyframe textures
    for (WAIKeyFrame* kf : keyframes)
    {
        if (kf->isBad())
            continue;

        SLKeyframeCamera* cam = new SLKeyframeCamera("KeyFrame " + std::to_string(kf->mnId));
        //set background
        if (kf->getTexturePath().size())
        {
            // TODO(jan): textures are saved in a global textures vector (scene->textures)
            // and should be deleted from there. Otherwise we have a yuuuuge memory leak.
#if 0
        SLGLTexture* texture = new SLGLTexture(kf->getTexturePath());
        _kfTextures.push_back(texture);
        cam->background().texture(texture);
#endif
        }

        cv::Mat Twc = kf->getObjectMatrix();

        SLMat4f om;
        om.setMatrix(Twc.at<float>(0, 0),
                     -Twc.at<float>(0, 1),
                     -Twc.at<float>(0, 2),
                     Twc.at<float>(0, 3),
                     Twc.at<float>(1, 0),
                     -Twc.at<float>(1, 1),
                     -Twc.at<float>(1, 2),
                     Twc.at<float>(1, 3),
                     Twc.at<float>(2, 0),
                     -Twc.at<float>(2, 1),
                     -Twc.at<float>(2, 2),
                     Twc.at<float>(2, 3),
                     Twc.at<float>(3, 0),
                     -Twc.at<float>(3, 1),
                     -Twc.at<float>(3, 2),
                     Twc.at<float>(3, 3));
        //om.rotate(180, 1, 0, 0);

        cam->om(om);

        //calculate vertical field of view
        SLfloat fy     = (SLfloat)kf->fy;
        SLfloat cy     = (SLfloat)kf->cy;
        SLfloat fovDeg = 2 * (SLfloat)atan2(cy, fy) * Utils::RAD2DEG;
        cam->fov(fovDeg);
        cam->focalDist(0.11f);
        cam->clipNear(0.1f);
        cam->clipFar(1000.0f);

        _waiScene->keyFrameNode->addChild(cam);
    }
}
//-----------------------------------------------------------------------------
void WAIApp::renderGraphs()
{
    std::vector<WAIKeyFrame*> kfs = _mode->getKeyFrames();

    SLVVec3f covisGraphPts;
    SLVVec3f spanningTreePts;
    SLVVec3f loopEdgesPts;
    for (auto* kf : kfs)
    {
        cv::Mat Ow = kf->GetCameraCenter();

        //covisibility graph
        const std::vector<WAIKeyFrame*> vCovKFs = kf->GetBestCovisibilityKeyFrames(_gui->uiPrefs->minNumOfCovisibles);

        if (!vCovKFs.empty())
        {
            for (vector<WAIKeyFrame*>::const_iterator vit = vCovKFs.begin(), vend = vCovKFs.end(); vit != vend; vit++)
            {
                if ((*vit)->mnId < kf->mnId)
                    continue;
                cv::Mat Ow2 = (*vit)->GetCameraCenter();

                covisGraphPts.push_back(SLVec3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2)));
                covisGraphPts.push_back(SLVec3f(Ow2.at<float>(0), Ow2.at<float>(1), Ow2.at<float>(2)));
            }
        }

        //spanning tree
        WAIKeyFrame* parent = kf->GetParent();
        if (parent)
        {
            cv::Mat Owp = parent->GetCameraCenter();
            spanningTreePts.push_back(SLVec3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2)));
            spanningTreePts.push_back(SLVec3f(Owp.at<float>(0), Owp.at<float>(1), Owp.at<float>(2)));
        }

        //loop edges
        std::set<WAIKeyFrame*> loopKFs = kf->GetLoopEdges();
        for (set<WAIKeyFrame*>::iterator sit = loopKFs.begin(), send = loopKFs.end(); sit != send; sit++)
        {
            if ((*sit)->mnId < kf->mnId)
                continue;
            cv::Mat Owl = (*sit)->GetCameraCenter();
            loopEdgesPts.push_back(SLVec3f(Ow.at<float>(0), Ow.at<float>(1), Ow.at<float>(2)));
            loopEdgesPts.push_back(SLVec3f(Owl.at<float>(0), Owl.at<float>(1), Owl.at<float>(2)));
        }
    }

    if (_waiScene->covisibilityGraphMesh)
        _waiScene->covisibilityGraph->deleteMesh(_waiScene->covisibilityGraphMesh);

    if (covisGraphPts.size() && _gui->uiPrefs->showCovisibilityGraph)
    {
        _waiScene->covisibilityGraphMesh = new SLPolyline(covisGraphPts, false, "CovisibilityGraph", _waiScene->covisibilityGraphMat);
        _waiScene->covisibilityGraph->addMesh(_waiScene->covisibilityGraphMesh);
        _waiScene->covisibilityGraph->updateAABBRec();
    }

    if (_waiScene->spanningTreeMesh)
        _waiScene->spanningTree->deleteMesh(_waiScene->spanningTreeMesh);

    if (spanningTreePts.size() && _gui->uiPrefs->showSpanningTree)
    {
        _waiScene->spanningTreeMesh = new SLPolyline(spanningTreePts, false, "SpanningTree", _waiScene->spanningTreeMat);
        _waiScene->spanningTree->addMesh(_waiScene->spanningTreeMesh);
        _waiScene->spanningTree->updateAABBRec();
    }

    if (_waiScene->loopEdgesMesh)
        _waiScene->loopEdges->deleteMesh(_waiScene->loopEdgesMesh);

    if (loopEdgesPts.size() && _gui->uiPrefs->showLoopEdges)
    {
        _waiScene->loopEdgesMesh = new SLPolyline(loopEdgesPts, false, "LoopEdges", _waiScene->loopEdgesMat);
        _waiScene->loopEdges->addMesh(_waiScene->loopEdgesMesh);
        _waiScene->loopEdges->updateAABBRec();
    }
}