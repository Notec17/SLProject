//#############################################################################
//  File:      AppDemoGuiTestBenchOpen.h
//  Author:    Luc Girod
//  Date:      September 2019
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SL_IMGUI_SLAM_PARAM_H
#define SL_IMGUI_SLAM_PARAM_H

#include <opencv2/core.hpp>
#include <AppDemoGuiInfosDialog.h>

#include <SLMat4.h>
#include <SLNode.h>
#include <AppWAI.h>
#include <WAICalibration.h>
#include <vector>

//-----------------------------------------------------------------------------
class AppDemoGuiSlamParam : public AppDemoGuiInfosDialog
{
public:
    AppDemoGuiSlamParam(const std::string& name,
                        bool*              activator);

    void buildInfos(SLScene* s, SLSceneView* sv) override;

private:
    KPextractor* orbExtractor(int nf);
    KPextractor* surfExtractor(int th);
    KPextractor* glslExtractor();
    KPextractor* kpExtractor(int id);

    KPextractor* _current;
    KPextractor* _iniCurrent;

    int _currentId;
    int _iniCurrentId;

    std::vector<std::string> _extractors;
};

#endif
