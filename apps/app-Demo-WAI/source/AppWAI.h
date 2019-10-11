//#############################################################################
//  File:      WAISceneView.h
//  Purpose:   Node transform test application that demonstrates all transform
//             possibilities of SLNode
//  Author:    Marc Wacker
//  Date:      July 2015
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef APP_WAI_SCENE_VIEW
#define APP_WAI_SCENE_VIEW

#include "AppWAIScene.h"
#include <SLSceneView.h>
#include <SLPoints.h>
#include <SLPolyline.h>

#include <CVCalibration.h>
#include <WAIAutoCalibration.h>
#include <AppDirectories.h>
#include <AppDemoGuiPrefs.h>
#include <AppDemoGuiAbout.h>
#include <AppDemoGuiError.h>

struct OrbSlamStartResult
{
    bool        wasSuccessful;
    std::string errorString;
};

//-----------------------------------------------------------------------------
class WAIApp
{
    public:
    static int                load(int width, int height, float scr2fbX, float scr2fbY, int dpi, AppWAIDirectories* dirs);
    static void               close();
    static OrbSlamStartResult startOrbSlam(std::string videoFile       = "",
                                           std::string calibrationFile = "",
                                           std::string mapFile         = "",
                                           std::string vocFileName     = "ORBvoc.bin",
                                           bool        saveVideoFrames = false);

    static void onLoadWAISceneView(SLScene* s, SLSceneView* sv, SLSceneID sid);
    static bool update();
    static bool updateTracking();

    static void updateTrackingVisualization(const bool iKnowWhereIAm);

    static void renderMapPoints(std::string                      name,
                                const std::vector<WAIMapPoint*>& pts,
                                SLNode*&                         node,
                                SLPoints*&                       mesh,
                                SLMaterial*&                     material);

    static void renderKeyframes();
    static void renderGraphs();
    static void refreshTexture(cv::Mat* image);

    static void setupGUI();
    static void buildGUI(SLScene* s, SLSceneView* sv);
    static void openTest(std::string path);

    static AppDemoGuiAbout* aboutDial;
    static AppDemoGuiError* errorDial;

    static GUIPreferences     uiPrefs;
    static AppWAIDirectories* dirs;
    static WAICalibration*    wc;
    static int                scrWidth;
    static int                scrHeight;
    static int                defaultScrWidth;
    static int                defaultScrHeight;
    static float              scrWdivH;
    static cv::VideoWriter*   videoWriter;
    static cv::VideoWriter*   videoWriterInfo;
    static WAI::ModeOrbSlam2* mode;
    static AppWAIScene*       waiScene;
    static bool               loaded;
    static SLGLTexture*       cpvrLogo;
    static SLGLTexture*       videoImage;
    static SLGLTexture*       testTexture;
    static ofstream           gpsDataStream;

    static bool resizeWindow;

    static std::string videoDir;
    static std::string calibDir;
    static std::string mapDir;
    static std::string vocDir;
    static std::string experimentsDir;

    static bool pauseVideo; // pause video file
    static int  videoCursorMoveIndex;

    struct GLSLKP
    {
        GLuint yuv422Converter;
        GLuint RGBTexture;
        GLuint grayTexture;

        GLuint hLaplacianId;
        GLuint vLaplacianId;
        GLuint interFBO;
        GLuint interTexture;
        GLuint hLapVAO;
        GLuint vLapVAO;
        GLuint vbo;
        GLuint vboi;
        GLuint framebuffers[2];
        GLuint renderTexture[2];
        GLuint pbo[2];
        GLint hLapTexLoc;
        GLint vLapTexLoc;
        GLint hLapWLoc;
        GLint vLapWLoc;
        int curr;
        int ready;
    };

    static GLSLKP glslKP;

    static GLuint buildShaderFromSource(string source,
                                        GLenum shaderType);
    static void initTestProgram();
    static void gpu_kp();
    static void readResult();
    static unsigned char * outputTexture;
};

#endif
