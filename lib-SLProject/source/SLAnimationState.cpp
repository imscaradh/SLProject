
#include <stdafx.h>
#include "SLAnimation.h"
#include "SLAnimationState.h"

SLAnimationState::SLAnimationState(SLAnimation* parent, SLfloat weight)
: _parentAnim(parent), 
_localTime(0.0f),
_playbackRate(1.0f),
_playbackDir(1),
_weight(weight),
_loop(true),
_enabled(false),
_easing(EC_linear)
{ }

void SLAnimationState::advanceTime(SLfloat delta)
{
    if (!_enabled)
        return;

    _linearLocalTime += delta * _playbackRate * _playbackDir;

    // fix invalid inputs
    if (_linearLocalTime > _parentAnim->length())
    {
        // wrap around on loop, else just stay on last frame
        if (_loop)
            _linearLocalTime = fmod(_linearLocalTime, _parentAnim->length());
        else
            _linearLocalTime = _parentAnim->length();
    }
    // fix negative inputs, playback rate could be negative
    else if (_linearLocalTime < 0.0f)
    {
        if (_loop)
            _linearLocalTime = fmod(_localTime, _parentAnim->length()) + _parentAnim->length();
        else
            _linearLocalTime = 0.0f;
    }     

    _localTime = calcEasingTime(_linearLocalTime);
}



void SLAnimationState::playForward()
{
    _enabled = true;
    _playbackDir = 1;
}
void SLAnimationState::playBackward()
{
    _enabled = true;
    _playbackDir = -1;
}
void SLAnimationState::pause()
{
    // @todo is a paused animation disabled OR is it enabled but just not advancing time?
    //       currently we set the direction multiplier to 0
    _enabled = true;
    _playbackDir = 0;
}
void SLAnimationState::skipToNextKeyframe()
{
    SLfloat time = _parentAnim->nextKeyframeTime(_localTime);
    localTime(time);
}
void SLAnimationState::skipToPrevKeyframe()
{
    SLfloat time = _parentAnim->prevKeyframeTime(_localTime);
    localTime(time);
}
void SLAnimationState::skipToStart()
{
    localTime(0.0f);
}
void SLAnimationState::skipToEnd()
{
    localTime(_parentAnim->length());
}


//-----------------------------------------------------------------------------
//! Applies the easing time curve to the input time.
/*! See also the declaration of the SLEasingCurve enumeration for the different
easing curve type that are taken from Qt QAnimation and QEasingCurve class. 
See http://qt-project.org/doc/qt-4.8/qeasingcurve.html#Type-enum
*/
SLfloat SLAnimationState::calcEasingTime(SLfloat time) const
{
    SLfloat x = time / _parentAnim->length();
    SLfloat y = 0.0f;

    switch (_easing)
    {
        case EC_linear:      y = x; break;

        case EC_inQuad:      y =                  pow(x     ,2.0f);        break;
        case EC_outQuad:     y =                 -pow(x-1.0f,2.0f) + 1.0f; break;
        case EC_inOutQuad:   y =  (x<0.5f) ? 2.0f*pow(x     ,2.0f) : 
                                        -2.0f*pow(x-1.0f,2.0f) + 1.0f; break;
        case EC_outInQuad:   y =  (x<0.5f) ?-2.0f*pow(x-0.5f,2.0f) + 0.5f : 
                                          2.0f*pow(x-0.5f,2.0f) + 0.5f; break;
   
        case EC_inCubic:     y =                  pow(x     ,3.0f);        break;
        case EC_outCubic:    y =                  pow(x-1.0f,3.0f) + 1.0f; break;
        case EC_inOutCubic:  y =  (x<0.5f) ? 4.0f*pow(x     ,3.0f) : 
                                          4.0f*pow(x-1.0f,3.0f) + 1.0f; break;
        case EC_outInCubic:  y =             4.0f*pow(x-0.5f,3.0f) + 0.5f; break;

        case EC_inQuart:     y =                  pow(x     ,4.0f);        break;
        case EC_outQuart:    y =                 -pow(x-1.0f,4.0f) + 1.0f; break;
        case EC_inOutQuart:  y =  (x<0.5f) ? 8.0f*pow(x     ,4.0f) : 
                                         -8.0f*pow(x-1.0f,4.0f) + 1.0f; break;
        case EC_outInQuart:  y =  (x<0.5f) ?-8.0f*pow(x-0.5f,4.0f) + 0.5f : 
                                          8.0f*pow(x-0.5f,4.0f) + 0.5f; break;
   
        case EC_inQuint:     y =                  pow(x     ,5.0f);        break;
        case EC_outQuint:    y =                  pow(x-1.0f,5.0f) + 1.0f; break;
        case EC_inOutQuint:  y =  (x<0.5f) ?16.0f*pow(x     ,5.0f) : 
                                         16.0f*pow(x-1.0f,5.0f) + 1.0f; break;
        case EC_outInQuint:  y =            16.0f*pow(x-0.5f,5.0f) + 0.5f; break;

        case EC_inSine:      y =      sin(x*SL_PI*0.5f- SL_PI*0.5f)+ 1.0f; break;
        case EC_outSine:     y =      sin(x*SL_PI*0.5f);                   break;
        case EC_inOutSine:   y = 0.5f*sin(x*SL_PI - SL_PI*0.5f) + 0.5f;    break;
        case EC_outInSine:   y = (x<0.5f) ? 
                            0.5f*sin(x*SL_PI) : 
                            0.5f*sin(x*SL_PI - SL_PI) + 1.0f;         break;                                  
        default: y = x; 
    }

    SLfloat newTime = y * _parentAnim->length();
    //printf("time: %5.3f, easing: %5.3f\n", time, newTime);

    return newTime;
}

// @todo add all the inverse versions
SLfloat SLAnimationState::calcEasingTimeInv(SLfloat time) const
{
    SLfloat x = time / _parentAnim->length();
    SLfloat y = 0.0f;

    switch (_easing)
    {
        case EC_linear:      y = x; break;

        case EC_inQuad:      y =                  sqrt(x);        break;
        case EC_outQuad:     y =                 -pow(x-1.0f,2.0f) + 1.0f; break;
        case EC_inOutQuad:   y =  (x<0.5f) ? 2.0f*pow(x     ,2.0f) : 
                                        -2.0f*pow(x-1.0f,2.0f) + 1.0f; break;
        case EC_outInQuad:   y =  (x<0.5f) ?-2.0f*pow(x-0.5f,2.0f) + 0.5f : 
                                          2.0f*pow(x-0.5f,2.0f) + 0.5f; break;
   
        case EC_inCubic:     y =                  pow(x     ,3.0f);        break;
        case EC_outCubic:    y =                  pow(x-1.0f,3.0f) + 1.0f; break;
        case EC_inOutCubic:  y =  (x<0.5f) ? 4.0f*pow(x     ,3.0f) : 
                                          4.0f*pow(x-1.0f,3.0f) + 1.0f; break;
        case EC_outInCubic:  y =             4.0f*pow(x-0.5f,3.0f) + 0.5f; break;

        case EC_inQuart:     y =                  pow(x     ,4.0f);        break;
        case EC_outQuart:    y =                 -pow(x-1.0f,4.0f) + 1.0f; break;
        case EC_inOutQuart:  y =  (x<0.5f) ? 8.0f*pow(x     ,4.0f) : 
                                         -8.0f*pow(x-1.0f,4.0f) + 1.0f; break;
        case EC_outInQuart:  y =  (x<0.5f) ?-8.0f*pow(x-0.5f,4.0f) + 0.5f : 
                                          8.0f*pow(x-0.5f,4.0f) + 0.5f; break;
   
        case EC_inQuint:     y =                  pow(x     ,5.0f);        break;
        case EC_outQuint:    y =                  pow(x-1.0f,5.0f) + 1.0f; break;
        case EC_inOutQuint:  y =  (x<0.5f) ?16.0f*pow(x     ,5.0f) : 
                                         16.0f*pow(x-1.0f,5.0f) + 1.0f; break;
        case EC_outInQuint:  y =            16.0f*pow(x-0.5f,5.0f) + 0.5f; break;

        case EC_inSine:      y =      sin(x*SL_PI*0.5f- SL_PI*0.5f)+ 1.0f; break;
        case EC_outSine:     y =      sin(x*SL_PI*0.5f);                   break;
        case EC_inOutSine:   y = 0.5f*sin(x*SL_PI - SL_PI*0.5f) + 0.5f;    break;
        case EC_outInSine:   y = (x<0.5f) ? 
                            0.5f*sin(x*SL_PI) : 
                            0.5f*sin(x*SL_PI - SL_PI) + 1.0f;         break;                                  
        default: y = x; 
    }

    SLfloat newTime = y * _parentAnim->length();
    printf("time: %5.3f, easing: %5.3f\n", time, newTime);

    return newTime;
}