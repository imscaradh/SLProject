//#############################################################################
//  File:      SLCVTracker.cpp
//  Author:    Michael G�ttlicher
//  Date:      Spring 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch, Michael G�ttlicher
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCVTRACKER_H
#define SLCVTRACKER_H

#include <stdafx.h>
#include <SLNode.h>
#include <SLSceneView.h>
#include <SLCV.h>
#include <SLCVCalibration.h>
#include <opencv2/aruco.hpp>

//-----------------------------------------------------------------------------
//! SLCVTracker is the pure virtual base class for tracking features in video.
/*! The SLScene instance holds a vector of SLCVTrackers that are tracked in 
scenes that require a live video image from the device camera. A tracker is
bound to a scene node. If the node is the camera node the tracker calculates
the relative position of the camera to the tracker. This is the standard 
aumented reality case. If the camera is a normal scene node, the tracker 
calculates the object matrix relative to the scene camera.
*/
class SLCVTracker
{
    public:
                     SLCVTracker    (SLNode* node = nullptr): 
                                     _node(node), _isVisible(false){;}
        virtual     ~SLCVTracker    (){;}

        virtual SLbool track        (SLCVMat imageGray,
                                     SLCVCalibration& calib,
                                     SLSceneView* sv) = 0;

        SLMat4f     createGLMatrix  (const SLCVMat& tVec, 
                                     const SLCVMat& rVec);
        SLMat4f     calcObjectMatrix(const SLMat4f& cameraObjectMat, 
                                     const SLMat4f& objectViewMat);

    protected:
        SLNode*     _node;          //!< Connected node
        SLbool      _isVisible;     //!< Flag if marker is visible
        SLMat4f     _objectViewMat; //!< view transformation matrix
};
//-----------------------------------------------------------------------------
#endif
