//#############################################################################
//  File:      SL.cpp
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
#include <cstdarg>

//-----------------------------------------------------------------------------
//! Default path for configuration files
SLstring SL::configPath = "../_data/config/";
//-----------------------------------------------------------------------------
// Init global test variables from command line parameters
SLint SL::testDurationSec = 0;
SLint SL::testFactor = 1;
SLCommand SL::testScene = (SLCommand)-1;
SLCommand SL::testSceneAll = C_sceneMinimal;
SLLogVerbosity SL::testLogVerbosity = LV_quiet;
SLuint SL::testFrameCounter = 0;
const SLVstring SL::testSceneNames = 
{   "SceneAll               ",
    "SceneMinimal           ",
    "SceneFigure            ", 
    "SceneMeshLoad          ",
    "SceneVRSizeTest        ",
    "SceneLargeModel        ",
    "SceneChristoffel       ",
    "SceneRevolver          ",
    "SceneTextureFilter     ",
    "SceneTextureBlend      ",
    "SceneTextureVideo      ",
    "SceneFrustumCull1      ",
    "ScenePerVertexBlinn    ",
    "ScenePerPixelBlinn     ",
    "ScenePerVertexWave     ",
    "SceneWater             ",
    "SceneBumpNormal        ",
    "SceneBumpParallax      ",
    "SceneEarth             ",
    "SceneMassAnimation     ",
    "SceneTerrain           ",
    "SceneSkeletalAnimation ",
    "SceneNodeAnimation     ",
    "SceneAstroboyArmyGPU   ",
    "SceneAstroboyArmyCPU   ",
    "SceneRTMuttenzerBox    ",
    "SceneRTSpheres         ",
    "SceneRTSoftShadows     ",
    "SceneRTDoF             ",
    "SceneRTLens            ",
    "SceneRTTest            "
};
//-----------------------------------------------------------------------------
//! SL::log
void SL::log(const char* format, ...)
{
    char log[4096];
    va_list argptr;
    va_start(argptr, format);
    vsprintf(log, format, argptr);
    va_end(argptr);
    cout << log;

    /*
    // In case we don't have a console
    static ofstream logFile;
    if (!logFile.is_open())
        logFile.open ("Users/Shared/SLProjectLog.txt");

    logFile << log;
    */
}
//-----------------------------------------------------------------------------
//! SL::Exit terminates the application with a message. No leak cheching.
void SL::exitMsg(const SLchar* msg, const SLint line, const SLchar* file)
{  
    #if defined(SL_OS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "SLProject", 
                        "Exit %s at line %d in %s\n", msg, line, file);
    #else
    SL::log("Exit %s at line %d in %s\n", msg, line, file);
    #endif
   
    #ifdef SL_MEMLEAKDETECT       // set in SL.h for debug config only
    // turn off leak checks on forced exit
    nvwa::new_autocheck_flag = false;
    #endif
   
    exit(-1);
}
//-----------------------------------------------------------------------------
//! SL::Warn message output
void SL::warnMsg(const SLchar* msg, const SLint line, const SLchar* file)
{  
    #if defined(SL_OS_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "SLProject", 
                        "Warning: %s at line %d in %s\n", msg, line, file);
    #else
    SL::log("Warning %s at line %d in %s\n", msg, line, file);
    #endif
}
//-----------------------------------------------------------------------------
/*! SL::maxThreads returns in release config the max. NO. of threads and in 
debug config 1. Try to avoid multithreading in the debug configuration. 
*/
SLuint SL::maxThreads()
{  
    #ifdef _DEBUG
    return 1;
    #else
    return SL_max(thread::hardware_concurrency(), 1U);
    #endif
}
//------------------------------------------------------------------------------
//! Parses the command line arguments
void SL::parseCmdLineArgs(SLVstring& cmdLineArgs)
{   // Default values
    SL::testScene = (SLCommand)-1;
    SL::testDurationSec = -1;

    SLVstring argComponents;
    for (SLstring arg : cmdLineArgs)
    {
        SLUtils::split(arg, '=', argComponents);
        if (argComponents.size()==2)
        {   
            if(argComponents[0] ==  "testScene")
            {   SLint iScene = atoi(argComponents[1].c_str());
                if (iScene >= C_sceneAll && iScene <= C_sceneRTTest)
                {   SL::testScene = (SLCommand)iScene;
                    SL::testLogVerbosity = LV_normal;
                    if (SL::testScene == C_sceneAll)
                        SL::testSceneAll = C_sceneMinimal;
                    if (SL::testDurationSec == -1)
                        SL::testDurationSec = 5;
                }
            }
            if(argComponents[0] ==  "durationSec")
            {   SLint sec = atoi(argComponents[1].c_str());
                if (sec > 0) SL::testDurationSec = sec;
            }
            if(argComponents[0] ==  "testFactor")
            {   SLint factor = atoi(argComponents[1].c_str());
                if (factor > 0) SL::testFactor = factor;
            }
        }
        argComponents.clear();
    }
}
//-----------------------------------------------------------------------------
