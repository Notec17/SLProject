//#############################################################################
//  File:      SLCVTrackerAruco.cpp
//  Author:    Michael Goettlicher, Marcus Hudritsch
//  Date:      Winter 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch, Michael Goettlicher
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCVTrackerFeatures_H
#define SLCVTrackerFeatures_H

/*
The OpenCV library version 3.1 with extra module must be present.
If the application captures the live video stream with OpenCV you have
to define in addition the constant SL_USES_CVCAPTURE.
All classes that use OpenCV begin with SLCV.
See also the class docs for SLCVCapture, SLCVCalibration and SLCVTracker
for a good top down information.
*/

#include <SLCV.h>
#include <SLCVTracker.h>
#include <SLNode.h>
#include <SLCVRaulMurOrb.h>
using namespace cv;


class SLCVTrackerFeatures : public SLCVTracker
{
    public:
        SLCVTrackerFeatures         (SLNode* node);
        ~SLCVTrackerFeatures        () {;}
        SLbool  track               (SLCVMat imageGray,
                                     SLCVMat image,
                                     SLCVCalibration* calib,
                                     SLSceneView* sv);
    private:
        static SLVMat4f         objectViewMats; //!< object view matrices
        Ptr<DescriptorMatcher>  _matcher;
        SLfloat                 _fx, _fy, _cx, _cy;
        SLMat4f                 _pose;
        SLCVCalibration         *_calib;

        struct map {
            vector<Point3f>         model;
            SLCVMat                 frameGray;
            SLCVVKeyPoint           keypoints;
            Mat                     descriptors;
        } _map;

        inline void loadModelPoints();
        inline SLCVVKeyPoint detectFeatures(const Mat &imageGray);
        inline Mat describeFeatures(const Mat &imageGray, SLCVVKeyPoint &keypoints);
        inline vector<DMatch> matchFeatures(const Mat &descriptors);
        inline bool calculatePose(const Mat &image, const SLCVVKeyPoint &keypoints, const vector<DMatch> &matches, Mat &rvec, Mat &tvec);
        inline void draw2DPoints(Mat image, const vector<Point2f> &list_points, Scalar color);
        inline Point2f backproject3DPoint(const Point3f &point3d);
};
//-----------------------------------------------------------------------------
#endif // SLCVTrackerFeatures_H
