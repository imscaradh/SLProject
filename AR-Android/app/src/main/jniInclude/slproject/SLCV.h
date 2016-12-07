//#############################################################################
//  File:      SLCV.h
//  Author:    Marcus Hudritsch
//  Date:      Autumn 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLCV_H
#define SLCV_H

#include <stdafx.h>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
//-----------------------------------------------------------------------------
typedef cv::Mat                     SLCVMat;
typedef cv::Rect                    SLCVRect;
typedef cv::Rect2f                  SLCVRect2f;
typedef cv::Point                   SLCVPoint;
typedef cv::Point2f                 SLCVPoint2f;
typedef cv::Point3f                 SLCVPoint3f;
typedef cv::Size                    SLCVSize;
typedef cv::Size2f                  SLCVSize2f;

// 1D STL vectors
typedef vector<cv::Mat>             SLCVVMat;
typedef vector<cv::Point>           SLCVVPoint;
typedef vector<cv::Point2f>         SLCVVPoint2f;
typedef vector<cv::Point2d>         SLCVVPoint2d;
typedef vector<cv::Point3f>         SLCVVPoint3f;
typedef vector<cv::Point3d>         SLCVVPoint3d;

// 2D STL vectors 
typedef vector<vector<cv::Point>>   SLCVVVPoint;
typedef vector<vector<cv::Point2f>> SLCVVVPoint2f;
typedef vector<vector<cv::Point3f>> SLCVVVPoint3f;
//-----------------------------------------------------------------------------
#endif