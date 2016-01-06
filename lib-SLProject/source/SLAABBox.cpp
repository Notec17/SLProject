//#############################################################################
//  File:      SLAABBox.cpp
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>           // precompiled headers
#ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
#include <debug_new.h>        // memory leak detector
#endif

#include <SLAABBox.h>
#include <SLRay.h>
#include <SLGLProgram.h>
#include <SLMesh.h>
#include <SLScene.h>

//-----------------------------------------------------------------------------
//! Default contructor with default zero vector initialization
SLAABBox::SLAABBox()
{
    _minWS    = SLVec3f::ZERO;
    _maxWS    = SLVec3f::ZERO;
    _centerWS = SLVec3f::ZERO;
    _radiusWS = 0;
   
    _minOS    = SLVec3f::ZERO;
    _maxOS    = SLVec3f::ZERO;
    _centerOS = SLVec3f::ZERO;
    _radiusOS = 0;
   
    _sqrViewDist = 0;
    _axis0WS = SLVec3f::ZERO;
    _axisXWS = SLVec3f::ZERO;
    _axisYWS = SLVec3f::ZERO;
    _axisZWS = SLVec3f::ZERO;

    _hasTransp = false;
    _isVisible = true;
}
//-----------------------------------------------------------------------------
//! Recalculate min and max after transformation in world coords
void SLAABBox::fromOStoWS(const SLVec3f &minOS,     
                          const SLVec3f &maxOS, 
                          const SLMat4f &wm)
{  
    // Do not transform empty AABB (such as from the camera)
    if (minOS==SLVec3f::ZERO && maxOS==SLVec3f::ZERO)
        return;

    _minOS.set(minOS);
    _maxOS.set(maxOS);
    _minWS.set(minOS);
    _maxWS.set(maxOS);

    // we need to transform all 8 corners for a non-optimal bounding box
    SLVec3f vCorner[8];
    SLint i;

    vCorner[0].set(_minWS);
    vCorner[1].set(_maxWS.x, _minWS.y, _minWS.z); 
    vCorner[2].set(_maxWS.x, _minWS.y, _maxWS.z);  
    vCorner[3].set(_minWS.x, _minWS.y, _maxWS.z);  
    vCorner[4].set(_maxWS.x, _maxWS.y, _minWS.z);  
    vCorner[5].set(_minWS.x, _maxWS.y, _minWS.z);
    vCorner[6].set(_minWS.x, _maxWS.y, _maxWS.z);  
    vCorner[7].set(_maxWS);

    // apply world transform
    for (i=0; i<8; ++i) vCorner[i] = wm.multVec(vCorner[i]);

    //sets the minimum and maximum of the vertex components of the 8 corners
    _minWS.set(vCorner[0]);
    _maxWS.set(vCorner[0]);
    for (i=1; i<8; ++i)
    {   _minWS.setMin(vCorner[i]);
        _maxWS.setMax(vCorner[i]);
    }

    //set coordinate axis in world space
    _axis0WS = wm.multVec(SLVec3f::ZERO);
    _axisXWS = wm.multVec(SLVec3f::AXISX);
    _axisYWS = wm.multVec(SLVec3f::AXISY);
    _axisZWS = wm.multVec(SLVec3f::AXISZ);

    // Delete OpenGL vertex array
    if (_vao.id()) _vao.clearAttribs();
   
    setCenterAndRadius();
}
//-----------------------------------------------------------------------------
//! Recalculate min and max before transformation in object coords
void SLAABBox::fromWStoOS(const SLVec3f &minWS, 
                          const SLVec3f &maxWS, 
                          const SLMat4f &wmI)
{  
    _minOS.set(minWS);
    _maxOS.set(maxWS);
    _minWS.set(minWS);
    _maxWS.set(maxWS);

    // we need to transform all 8 corners for a non-optimal bounding box
    SLVec3f vCorner[8];
    SLint i;

    vCorner[0].set(_minOS);
    vCorner[1].set(_maxOS.x, _minOS.y, _minOS.z); 
    vCorner[2].set(_maxOS.x, _minOS.y, _maxOS.z);  
    vCorner[3].set(_minOS.x, _minOS.y, _maxOS.z);  
    vCorner[4].set(_maxOS.x, _maxOS.y, _minOS.z);  
    vCorner[5].set(_minOS.x, _maxOS.y, _minOS.z);
    vCorner[6].set(_minOS.x, _maxOS.y, _maxOS.z);  
    vCorner[7].set(_maxOS);

    // apply world transform
    for (i=0; i<8; ++i) vCorner[i] = wmI.multVec(vCorner[i]);

    //sets the minimum and maximum of the vertex components of the 8 corners
    _minOS.set(vCorner[0]);
    _maxOS.set(vCorner[0]);
    for (i=1; i<8; ++i)
    {   _minOS.setMin(vCorner[i]);
        _maxOS.setMax(vCorner[i]);
    }
   
    // Delete OpenGL vertex array
    if (_vao.id()) _vao.clearAttribs();
   
    setCenterAndRadius();
}
//-----------------------------------------------------------------------------
//! Updates the axis of the owning node
void SLAABBox::updateAxisWS(const SLMat4f &wm)
{  
    //set coordinate axis in world space
    _axis0WS = wm.multVec(SLVec3f::ZERO);
    _axisXWS = wm.multVec(SLVec3f::AXISX);
    _axisYWS = wm.multVec(SLVec3f::AXISY);
    _axisZWS = wm.multVec(SLVec3f::AXISZ);
    
    // Delete OpenGL vertex array
    if (_vao.id()) _vao.clearAttribs();
}
//-----------------------------------------------------------------------------
//! Updates joints axis and the bone line from the parent to us
/*! If the node has a skeleton assigned the method updates the axis and bone
visualization lines of the joint. Note that joints bone line is drawn by its
children. So the bone line in here is the bone from the parent to us.
If this parent bone direction is not along the parents Y axis we interpret the
connection not as a bone but as an offset displacement. Bones will be drawn in
SLAABBox::drawBoneWS in yellow and displacements in magenta.
If the joint has no parent (the root) no line is drawn.
*/
void SLAABBox::updateBoneWS(const SLMat4f &parentWM,
                            const SLbool isRoot,
                            const SLMat4f &nodeWM)
{
    // set coordinate axis centre point
    _axis0WS = nodeWM.multVec(SLVec3f::ZERO);

    // set scale factor for coordinate axis
    SLfloat axisScaleFactor = 0.03f;

    if (!isRoot)
    { 
        // build the parent pos in WM
        _parent0WS = parentWM.multVec(SLVec3f::ZERO);

        // set the axis scale factor depending on the length of the parent bone
        SLVec3f parentToMe = _axis0WS - _parent0WS;
        axisScaleFactor = SL_max(parentToMe.length() / 10.0f, axisScaleFactor);

        // check if the parent to me direction is parallel to the parents actual y-axis
        parentToMe.normalize();
        SLVec3f parentY = parentWM.axisY();
        parentY.normalize();
        _boneIsOffset = parentToMe.dot(parentY) < (1.0f - FLT_EPSILON);
    } else
    {   
        // for the root node don't draw a parent bone
        _parent0WS = _axis0WS;
        _boneIsOffset = false;
    }

    // set coordinate axis end points
    _axisXWS = nodeWM.multVec(SLVec3f::AXISX * axisScaleFactor);
    _axisYWS = nodeWM.multVec(SLVec3f::AXISY * axisScaleFactor);
    _axisZWS = nodeWM.multVec(SLVec3f::AXISZ * axisScaleFactor);
    
    // Delete OpenGL vertex array
    if (_vao.id()) _vao.clearAttribs();
}
//-----------------------------------------------------------------------------
//! Calculates center & radius of the bounding sphere around the AABB
void SLAABBox::setCenterAndRadius()
{  
    _centerWS =  _minWS;
    _centerWS += _maxWS;
    _centerWS *= 0.5f;
    SLVec3f ext(_maxWS-_centerWS);
    _radiusWS = ext.length();
   
    _centerOS =  _minOS;
    _centerOS += _maxOS;
    _centerOS *= 0.5f;
    ext.set(_maxOS-_centerOS);
    _radiusOS = ext.length();
}
//-----------------------------------------------------------------------------
//! Generates the vertex buffer for the line visualization
void SLAABBox::generateVAO()
{
    SLVec3f P[32];  // vertex positions (24 for aabb, 6 for axis, 3 for joint)

    P[ 0].set(_minWS.x, _minWS.y, _minWS.z); // lower rect
    P[ 1].set(_maxWS.x, _minWS.y, _minWS.z);
    P[ 2].set(_maxWS.x, _minWS.y, _minWS.z);
    P[ 3].set(_maxWS.x, _minWS.y, _maxWS.z);
    P[ 4].set(_maxWS.x, _minWS.y, _maxWS.z);
    P[ 5].set(_minWS.x, _minWS.y, _maxWS.z);
    P[ 6].set(_minWS.x, _minWS.y, _maxWS.z);
    P[ 7].set(_minWS.x, _minWS.y, _minWS.z);

    P[ 8].set(_minWS.x, _maxWS.y, _minWS.z); // upper rect
    P[ 9].set(_maxWS.x, _maxWS.y, _minWS.z);
    P[10].set(_maxWS.x, _maxWS.y, _minWS.z);
    P[11].set(_maxWS.x, _maxWS.y, _maxWS.z);
    P[12].set(_maxWS.x, _maxWS.y, _maxWS.z);
    P[13].set(_minWS.x, _maxWS.y, _maxWS.z);
    P[14].set(_minWS.x, _maxWS.y, _maxWS.z);

    P[15].set(_minWS.x, _maxWS.y, _minWS.z); // vertical lines
    P[16].set(_minWS.x, _minWS.y, _minWS.z);
    P[17].set(_minWS.x, _maxWS.y, _minWS.z);
    P[18].set(_maxWS.x, _minWS.y, _minWS.z);
    P[19].set(_maxWS.x, _maxWS.y, _minWS.z);
    P[20].set(_maxWS.x, _minWS.y, _maxWS.z);
    P[21].set(_maxWS.x, _maxWS.y, _maxWS.z);
    P[22].set(_minWS.x, _minWS.y, _maxWS.z);
    P[23].set(_minWS.x, _maxWS.y, _maxWS.z);

    P[24].set(_axis0WS.x, _axis0WS.y, _axis0WS.z); // x-axis
    P[25].set(_axisXWS.x, _axisXWS.y, _axisXWS.z);
    P[26].set(_axis0WS.x, _axis0WS.y, _axis0WS.z); // y-axis
    P[27].set(_axisYWS.x, _axisYWS.y, _axisYWS.z);
    P[28].set(_axis0WS.x, _axis0WS.y, _axis0WS.z); // z-axis
    P[29].set(_axisZWS.x, _axisZWS.y, _axisZWS.z);

    // Bone points
    P[30].set(_parent0WS.x, _parent0WS.y, _parent0WS.z);
    P[31].set(_axis0WS.x, _axis0WS.y, _axis0WS.z);

    _vao.generateLineVertices(32, 3, P);
}
//-----------------------------------------------------------------------------
//! Draws the AABB in world space with lines in a color
void SLAABBox::drawWS(const SLCol3f color)
{
    if (!_vao.id()) generateVAO();
    _vao.drawColorLines(color, 1.0f, 0, 24);
}
//-----------------------------------------------------------------------------
//! Draws the axis in world space with lines in a color
void SLAABBox::drawAxisWS()
{
    if (!_vao.id()) generateVAO();
    _vao.drawColorLines(SLCol3f::RED,   2.0f, 24, 2);
    _vao.drawColorLines(SLCol3f::GREEN, 2.0f, 26, 2);
    _vao.drawColorLines(SLCol3f::BLUE,  2.0f, 28, 2);
}
//-----------------------------------------------------------------------------
//! Draws the joint axis and the parent bone in world space
/*! The joints x-axis is drawn in red, the y-axis in green and the z-axis in 
blue. If the parent displacement is a bone it is drawn in yellow, if it is a
an offset displacement in magenta. See also SLAABBox::updateBoneWS.
*/
void SLAABBox::drawBoneWS()
{
    if (!_vao.id()) generateVAO();
    _vao.drawColorLines(SLCol3f::RED,     2.0f, 24, 2);
    _vao.drawColorLines(SLCol3f::GREEN,   2.0f, 26, 2);
    _vao.drawColorLines(SLCol3f::BLUE,    2.0f, 28, 2);

    // draw either an offset line or a bone line as the parent
    if (!_boneIsOffset)
         _vao.drawColorLines(SLCol3f::YELLOW,  1.0f, 30, 2);
    else _vao.drawColorLines(SLCol3f::MAGENTA, 1.0f, 30, 2);
}
//-----------------------------------------------------------------------------
//! SLAABBox::isHitInWS: Ray - AABB Intersection Test in object space
#define SL_RAY_AABB_FYFFE
SLbool SLAABBox::isHitInOS(SLRay* ray)
{     
    //See: "An Efficient and Robust Ray Box Intersection Algorithm"
    //by Amy L. Williams, Steve Barrus, R. Keith Morley, Peter Shirley
    //This test is about 10% faster than the test from Woo
    //It need the pre computed values invDir and sign in SLRay

    SLVec3f  params[2] = {_minOS, _maxOS};
    SLfloat  tymin, tymax, tzmin, tzmax;

    ray->tmin = (params[  ray->signOS[0]].x - ray->originOS.x) * ray->invDirOS.x;
    ray->tmax = (params[1-ray->signOS[0]].x - ray->originOS.x) * ray->invDirOS.x;
    tymin     = (params[  ray->signOS[1]].y - ray->originOS.y) * ray->invDirOS.y;
    tymax     = (params[1-ray->signOS[1]].y - ray->originOS.y) * ray->invDirOS.y;

    if ((ray->tmin > tymax) || (tymin > ray->tmax)) return false;
    if (tymin > ray->tmin) ray->tmin = tymin;
    if (tymax < ray->tmax) ray->tmax = tymax;

    tzmin = (params[  ray->signOS[2]].z - ray->originOS.z) * ray->invDirOS.z;
    tzmax = (params[1-ray->signOS[2]].z - ray->originOS.z) * ray->invDirOS.z;

    if ((ray->tmin > tzmax) || (tzmin > ray->tmax)) return false;
    if (tzmin > ray->tmin) ray->tmin = tzmin;
    if (tzmax < ray->tmax) ray->tmax = tzmax;
   
    return ((ray->tmin < ray->length) && (ray->tmax > 0));
 }

//-----------------------------------------------------------------------------
//! SLAABBox::isHitInWS: Ray - AABB Intersection Test in world space
SLbool SLAABBox::isHitInWS(SLRay* ray)
{     
    //See: "An Efficient and Robust Ray Box Intersection Algorithm"
    //by Amy L. Williams, Steve Barrus, R. Keith Morley, Peter Shirley
    //This test is about 10% faster than the test from Woo
    //It need the pre computed values invDir and sign in SLRay
    SLVec3f  params[2] = {_minWS, _maxWS};
    SLfloat  tymin, tymax, tzmin, tzmax;

    ray->tmin = (params[  ray->sign[0]].x - ray->origin.x) * ray->invDir.x;
    ray->tmax = (params[1-ray->sign[0]].x - ray->origin.x) * ray->invDir.x;
    tymin     = (params[  ray->sign[1]].y - ray->origin.y) * ray->invDir.y;
    tymax     = (params[1-ray->sign[1]].y - ray->origin.y) * ray->invDir.y;

    if ((ray->tmin > tymax) || (tymin > ray->tmax)) return false;
    if (tymin > ray->tmin) ray->tmin = tymin;
    if (tymax < ray->tmax) ray->tmax = tymax;

    tzmin = (params[  ray->sign[2]].z - ray->origin.z) * ray->invDir.z;
    tzmax = (params[1-ray->sign[2]].z - ray->origin.z) * ray->invDir.z;

    if ((ray->tmin > tzmax) || (tzmin > ray->tmax)) return false;
    if (tzmin > ray->tmin) ray->tmin = tzmin;
    if (tzmax < ray->tmax) ray->tmax = tzmax;
   
    return ((ray->tmin < ray->length) && (ray->tmax > 0));
}

//-----------------------------------------------------------------------------
//! Merges the bounding box bb to this one by extending this one axis aligned
void SLAABBox::mergeWS(SLAABBox &bb)
{  
    if (bb.minWS()!=SLVec3f::ZERO && bb.maxWS()!=SLVec3f::ZERO)
    {  
        _minWS.setMin(bb.minWS());
        _maxWS.setMax(bb.maxWS());
    }
}
//-----------------------------------------------------------------------------
