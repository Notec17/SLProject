//#############################################################################
//  File:      AppDemoGuiTestBenchOpen.h
//  Author:    Luc Girod
//  Date:      September 2019
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SL_IMGUI_TEST_OPEN_H
#define SL_IMGUI_TEST_OPEN_H

#include <opencv2/core.hpp>
#include <AppDemoGuiInfosDialog.h>

#include <SLMat4.h>
#include <SLNode.h>
#include <WAICalibration.h>
#include <WAI.h>

//-----------------------------------------------------------------------------
class AppDemoGuiTestOpen : public AppDemoGuiInfosDialog
{
    public:
    AppDemoGuiTestOpen(const std::string& name, std::string saveDir, WAI::WAI* wai, WAICalibration* wc, SLNode* mapNode, bool* activator);

    void buildInfos(SLScene* s, SLSceneView* sv) override;

    private:
    struct TestInfo
    {
        bool        open;
        std::string name;
        std::string testScene;
        std::string lighting;
        std::string features;
        std::string date;
        std::string calPath;
        std::string vidPath;
        std::string mapPath;
    };

    TestInfo openTestSettings(std::string path);
    void     loadTestSettings(TestInfo* infos);

    std::vector<TestInfo> _infos;
    SLNode*               _mapNode;
    int                   _currentItem;

    std::string     _saveDir;
    std::string     _settingsDir;
    WAICalibration* _wc;
    WAI::WAI*       _wai;
    std::string     _statusMsg = "nothing loaded";
};

#endif
