//#############################################################################
//  File:      SLCVCalibration.cpp
//  Author:    Michael G�ttlicher, Marcus Hudritsch
//  Date:      Winter 2016
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch, Michael G�ttlicher
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#include <stdafx.h>         // precompiled headers

/* 
If an application uses live video processing you have to define 
the preprocessor contant SL_HAS_OPENCV in the project settings.
The OpenCV library version 3.1 with extra module must be present.
If the application captures the live video stream with OpenCV you have
to define in addition the constant SL_USES_CVCAPTURE.
All classes that use OpenCV begin with SLCV.
See also the class docs for SLCVCapture, SLCVCalibration and SLCVTracker
for a good top down information.
*/
#ifdef SL_HAS_OPENCV
#include <SLCVCalibration.h>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>

using namespace cv;
using namespace std;

//-----------------------------------------------------------------------------
//! Default path for calibration files
SLstring SLCVCalibration::calibIniPath = "../_data/calibrations/";
//-----------------------------------------------------------------------------
SLCVCalibration::SLCVCalibration() :
    _cameraFovDeg(1.0f),
    _state(CS_uncalibrated),
    _calibFileName("cam_calibration.xml"),
    _calibParamsFileName("calib_in_params.yml"),
    _boardSize(0, 0),
    _boardSquareMM(0.0f),
    _numOfImgsToCapture(0),
    _numCaptured(0),
    _reprojectionError(-1.0f),
    _showUndistorted(false)
{
}
//-----------------------------------------------------------------------------
//! Resets the calibration to the uncalibrated state
void SLCVCalibration::clear()
{
    _numCaptured = 0;
    _reprojectionError = -1.0f;
    _imagePoints.clear();
    _state = CS_uncalibrated;
}
//-----------------------------------------------------------------------------
//! Loads the calibration information from the config file
bool SLCVCalibration::loadCamParams()
{
    //load camera parameter
    FileStorage fs;
    fs.open(SL::configPath + _calibFileName, FileStorage::READ);
    if (!fs.isOpened())
    {
        cout << "Could not open the calibration file: "
             << (SL::configPath + _calibFileName) << endl;
        _state = CS_uncalibrated;
        return false;
    }


    fs["camera_matrix"] >> _intrinsics;
    fs["distortion_coefficients"] >> _distortion;
    fs["avg_reprojection_error"] >> _reprojectionError;
    fs["image_width"] >> _imageSize.width;
    fs["image_height"] >> _imageSize.height;


    // close the input file
    fs.release();

    //calculate projection matrix
    calcCameraFOV();

    _state = CS_calibrated;

    return true;
}
//-----------------------------------------------------------------------------
//! Loads the chessboard calibration pattern parameters
bool SLCVCalibration::loadCalibParams()
{
    FileStorage fs;
    fs.open(calibIniPath + _calibParamsFileName, FileStorage::READ);
    if (!fs.isOpened())
    {   cout << "Could not open the calibration parameter file: "
             << (calibIniPath + _calibParamsFileName) << endl;
        _state = CS_uncalibrated;
        return false;
    }

    //assign paramters
    fs["numInnerCornersWidth"] >> _boardSize.width;
    fs["numInnerCornersHeight"] >> _boardSize.height;
    fs["squareSizeMM"] >> _boardSquareMM;
    fs["numOfImgsToCapture"] >> _numOfImgsToCapture;

    return true;
}
//-----------------------------------------------------------------------------
//! Calculates the vertical field of view angle in degrees
void SLCVCalibration::calcCameraFOV()
{
    //calculate vertical field of view
    SLfloat fy = (SLfloat)_intrinsics.at<double>(1,1);
    SLfloat cy = (SLfloat)_intrinsics.at<double>(1,2);
    SLfloat fovRad = 2 * (SLfloat)atan2(cy, fy);
    _cameraFovDeg = fovRad * SL_RAD2DEG;
}
//-----------------------------------------------------------------------------
//! Calculates the 3D positions of the chessboard corners
void SLCVCalibration::calcBoardCorners3D(SLCVSize boardSize, 
                                         SLfloat squareSize, 
                                         SLCVVPoint3f& objectPoints3D)
{
    // Because OpenCV image coords are top-left we define the according
    // 3D coords also top-left.
    objectPoints3D.clear();
    for(SLint y = boardSize.height-1; y >= 0 ; --y)
        for(SLint x = 0; x < boardSize.width; ++x)
            objectPoints3D.push_back(SLCVPoint3f(x*squareSize, 
                                                 y*squareSize, 
                                                 0));
}
//-----------------------------------------------------------------------------
//! Calculates the reprojection error of the calibration
static double calcReprojectionErrors(const SLCVVVPoint3f& objectPoints,
                                     const SLCVVVPoint2f& imagePoints,
                                     const SLCVVMat& rvecs,
                                     const SLCVVMat& tvecs,
                                     const SLCVMat& cameraMatrix ,
                                     const SLCVMat& distCoeffs,
                                     SLVfloat& perViewErrors)
{
    SLCVVPoint2f imagePoints2;
    size_t totalPoints = 0;
    double totalErr = 0, err;
    perViewErrors.resize(objectPoints.size());

    for(size_t i = 0; i < objectPoints.size(); ++i)
    {
        cv::projectPoints(objectPoints[i], 
                          rvecs[i], 
                          tvecs[i], 
                          cameraMatrix, 
                          distCoeffs, 
                          imagePoints2);

        err = norm(imagePoints[i], imagePoints2, NORM_L2);

        size_t n = objectPoints[i].size();
        perViewErrors[i] = (SLfloat) std::sqrt(err*err/n);
        totalErr        += err*err;
        totalPoints     += n;
    }

    return std::sqrt(totalErr/totalPoints);
}
//-----------------------------------------------------------------------------
//! Calculates the calibration with the given set of image points
static bool calcCalibration(SLCVSize& imageSize,
                            SLCVMat& cameraMatrix,
                            SLCVMat& distCoeffs,
                            SLCVVVPoint2f imagePoints,
                            SLCVVMat& rvecs, 
                            SLCVVMat& tvecs,
                            SLVfloat& reprojErrs,
                            double& totalAvgErr,
                            SLCVSize& boardSize,
                            SLfloat squareSize,
                            SLint flag)
{
    // fixed_aspect
    cameraMatrix = SLCVMat::eye(3, 3, CV_64F);

    //if 1, only fy is considered as a free parameter, 
    // the ratio fx/fy stays the same as in the input cameraMatrix.
    cameraMatrix.at<double>(0,0) = 1.0;

    // fixed_aspect
    distCoeffs = SLCVMat::zeros(8, 1, CV_64F);

    SLCVVVPoint3f objectPoints(1);

    SLCVCalibration::calcBoardCorners3D(boardSize, 
                                        squareSize, 
                                        objectPoints[0]);

    objectPoints.resize(imagePoints.size(),objectPoints[0]);

    ////////////////////////////////////////////////
    //Find intrinsic and extrinsic camera parameters
    double rms = cv::calibrateCamera(objectPoints,
                                     imagePoints,
                                     imageSize,
                                     cameraMatrix,
                                     distCoeffs,
                                     rvecs,
                                     tvecs,
                                     flag);
    ////////////////////////////////////////////////

    cout << "Re-projection error reported by calibrateCamera: "<< rms << endl;

    bool ok = cv::checkRange(cameraMatrix) && cv::checkRange(distCoeffs);

    totalAvgErr = calcReprojectionErrors(objectPoints,
                                         imagePoints,
                                         rvecs,
                                         tvecs,
                                         cameraMatrix,
                                         distCoeffs,
                                         reprojErrs);
    return ok;
}
//-----------------------------------------------------------------------------
//! Saves the camera calibration parameters to the config file
static void saveCameraParams(SLCVSize& imageSize,
                             SLCVMat& cameraMatrix, Mat& distCoeffs,
                             const SLCVVMat& rvecs,
                             const SLCVVMat& tvecs,
                             const SLVfloat& reprojErrs,
                             const SLCVVVPoint2f& imagePoints,
                             double totalAvgErr,
                             SLint flag,
                             SLstring filename,
                             SLCVSize& boardSize,
                             SLfloat squareSize)
{
    SLstring fullPathAndFilename = SL::configPath + filename;
    cout << "saveCameraParams: " << fullPathAndFilename << endl;
    
    cv::FileStorage fs(fullPathAndFilename, FileStorage::WRITE);
    
    if (!fs.isOpened())
    {   SL_EXIT_MSG("Failed to open file for writing!");
        return;
    }

    time_t tm;
    time(&tm);
    struct tm *t2 = localtime(&tm);
    char buf[1024];
    strftime(buf, sizeof(buf), "%c", t2);

    fs << "calibration_time" << buf;

    if(!rvecs.empty() || !reprojErrs.empty())
        fs << "nr_of_frames" << (int)std::max(rvecs.size(), reprojErrs.size());
    fs << "image_width"  << imageSize.width;
    fs << "image_height" << imageSize.height;
    fs << "board_width"  << boardSize.width;
    fs << "board_height" << boardSize.height;
    fs << "square_size"  << squareSize;

    fs << "fix_aspect_ratio" << 1;

    if (flag)
    {   sprintf(buf, "flags:%s%s%s%s",
                flag & CALIB_USE_INTRINSIC_GUESS ? " +use_intrinsic_guess" : "",
                flag & CALIB_FIX_ASPECT_RATIO ? " +fix_aspectRatio" : "",
                flag & CALIB_FIX_PRINCIPAL_POINT ? " +fix_principal_point" : "",
                flag & CALIB_ZERO_TANGENT_DIST ? " +zero_tangent_dist" : "");
        cvWriteComment(*fs, buf, 0);
    }

    fs << "flags" << flag;
    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;
    fs << "avg_reprojection_error" << totalAvgErr;
}
//-----------------------------------------------------------------------------
//!< Finds the inner chessboard corners in the given image
bool SLCVCalibration::findChessboard(SLCVMat imageColor,
                                     SLCVMat imageGray,
                                     bool drawCorners)
{   
    _imageSize = imageColor.size();

    SLCVVPoint2f corners2D;
    SLint flags = CALIB_CB_ADAPTIVE_THRESH | 
                  CALIB_CB_NORMALIZE_IMAGE | 
                  CALIB_CB_FAST_CHECK;

    bool found = cv::findChessboardCorners(imageGray,
                                           _boardSize,
                                           corners2D,
                                           flags);

    if(found && drawCorners)
        cv::drawChessboardCorners(imageColor,
                                  _boardSize,
                                  SLCVMat(corners2D),
                                  found);

    if (found && _state == CS_calibrateGrab)
    {
        cv::cornerSubPix(imageGray,
                         corners2D, 
                         SLCVSize(11,11),
                         SLCVSize(-1,-1), 
                         TermCriteria(TermCriteria::EPS+TermCriteria::COUNT, 
                         30, 
                         0.1));

        //debug save image
        //stringstream ss;
        //ss << "imageIn_" << _numCaptured << ".png";
        //cv::imwrite(ss.str(), image);

        //add detected points
        _imagePoints.push_back(corners2D);
        _numCaptured++;

        //simulate a snapshot
        cv::bitwise_not(imageColor, imageColor);

        _state = CS_calibrateStream;
    }
    return found;
}
//-----------------------------------------------------------------------------
//! Clears and flags the state for calibration
void SLCVCalibration::setCalibrationState()
{
    clear();
    _state = CS_calibrateStream;
}
//-----------------------------------------------------------------------------
//! Initiates the final calculation
void SLCVCalibration::calculate()
{
    _state = CS_startCalculating;

    SLCVVMat rvecs, tvecs;
    SLVfloat reprojErrs;
    double totalAvgErr = 0;

    SLint flag = 0;
    flag |= CALIB_FIX_PRINCIPAL_POINT;
    flag |= CALIB_ZERO_TANGENT_DIST;
    flag |= CALIB_FIX_ASPECT_RATIO;

    bool ok = calcCalibration(_imageSize, 
                              _intrinsics, 
                              _distortion, 
                              _imagePoints, 
                              rvecs, 
                              tvecs, 
                              reprojErrs,
                              totalAvgErr, 
                              _boardSize, 
                              _boardSquareMM, 
                              flag);

    cout << (ok ? "Calibration succeeded" : "Calibration failed")
         << ". avg re projection error = " << totalAvgErr << endl;

    if (ok)
    {   cout << "intrinsics" << _intrinsics << endl;
        cout << "distortion" << _distortion << endl;

        saveCameraParams(_imageSize, 
                         _intrinsics, 
                         _distortion, 
                         rvecs, 
                         tvecs, 
                         reprojErrs, 
                         _imagePoints,
                         totalAvgErr, 
                         flag, 
                         _calibFileName, 
                         _boardSize, 
                         _boardSquareMM);

        calcCameraFOV();
        _reprojectionError = (float)totalAvgErr;
        _state = CS_calibrated;
    }
}
//-----------------------------------------------------------------------------
#endif // SL_HAS_OPENCV
