//#############################################################################
//  File:      SLBox.h
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/SLProject-Coding-Style
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLBOX_H
#define SLBOX_H

#include <SLEnums.h>
#include <SLMesh.h>

//-----------------------------------------------------------------------------
//! Axis aligned box mesh
/*!      
The SLBox node draws an axis aligned box from a minimal corner to a maximal
corner. If the minimal and maximal corner are swapped the normals will point
inside.
*/
class SLBox : public SLMesh
{
    public:
    SLBox(SLfloat     minx = 0,
          SLfloat     miny = 0,
          SLfloat     minz = 0,
          SLfloat     maxx = 1,
          SLfloat     maxy = 1,
          SLfloat     maxz = 1,
          SLstring    name = "box mesh",
          SLMaterial* mat  = nullptr);
    SLBox(SLVec3f     min,
          SLVec3f     max,
          SLstring    name = "box mesh",
          SLMaterial* mat  = nullptr);

    void buildMesh(SLMaterial* mat);

    private:
    SLVec3f _min; //!< minimal corner
    SLVec3f _max; //!< maximum corner
};
//-----------------------------------------------------------------------------
#endif
