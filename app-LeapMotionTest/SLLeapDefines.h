//#############################################################################
//  File:      SLLeapDefines.h
//  Author:    Marc Wacker
//  Date:      February 2015
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: 2002-2015 Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLLEAPDEFINES_H
#define SLLEAPDEFINES_H

/// @todo   It would be advisable to switch to a higher resolution unit, cm or mm for SLProject too.
///         We've run into the lower float epsilone quite a few times now.
///         It would be a good idea to implement this somewhere in the core and let the user choose, but 
///         set the default to cm or mm.
#define LM_TO_SL_UNITS 0.001f;  // convert's lm units (millimeters) to SLunits (currently meters)


//-----------------------------------------------------------------------------
//! Finger types
enum SLFingerType
{
    FT_Thumb,
    FT_Index,
    FT_Middle,
    FT_Ring,
    FT_Pinky
};
//-----------------------------------------------------------------------------
//! Finger bone types
enum SLFingerBone
{
    FB_Metacarpal = 0,   // bone connected to the wrist inside the palm
    FB_Proximal = 1,     // bone connecting to the palm
    FB_Intermediate = 2, // bone between the tip and the base
    FB_Distal = 3,       // bone at the tip of the finger
};

#endif
