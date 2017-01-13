//#############################################################################
//  File:      SLCVTracker.cpp
//  Author:    Michael G�ttlicher
//  Date:      Spring 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch, Michael G�ttlicher
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCVCHESSBOARDTRACKER_H
#define SLCVCHESSBOARDTRACKER_H

#include <SLCV.h>
#include <SLCVTracker.h>

//-----------------------------------------------------------------------------
/*!
Chessboard tracking class
*/
class SLCVTrackerChessboard : public SLCVTracker
{
    public:
                SLCVTrackerChessboard   (SLNode* node);
               ~SLCVTrackerChessboard   () {;}
        bool    track                   (SLCVMat imageGray,
                                         SLCVCalibration& calib,
                                         SLSceneView* sv);
    private:
        SLfloat         _edgeLengthM;   //<! Length of chessboard square in meters
        SLCVVPoint3f    _boardPoints3D; //<! chessboard corners in world coordinate system
        SLCVSize        _boardSize;     //<! NO. of inner chessboard corners
        SLbool          _solved;        //<! Flag if last solvePnP was solved
        SLCVMat         _rVec;          //<! rotation angle vector from solvePnP
        SLCVMat         _tVec;          //<! translation vector from solvePnP
        
};
//-----------------------------------------------------------------------------

#endif // SLCVCHESSBOARDTRACKER_H
