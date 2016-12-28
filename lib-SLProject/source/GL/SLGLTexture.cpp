//#############################################################################
//  File:      SLGLTexture.cpp
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

#include <SLGLTexture.h>
#include <SLScene.h>

//-----------------------------------------------------------------------------
//! Default path for texture files used when only filename is passed in load.
SLstring SLGLTexture::defaultPath = "../_data/images/textures/";
//! maxAnisotropy=-1 show that GL_EXT_texture_filter_anisotropic is not checked
SLfloat SLGLTexture::maxAnisotropy = -1.0f;
//! NO. of texture byte allocated on GPU
SLuint SLGLTexture::numBytesInTextures = 0;
//-----------------------------------------------------------------------------
//! default ctor for fonts
SLGLTexture::SLGLTexture()
{  
    _stateGL      = SLGLState::getInstance();
    _texName      = 0;
    _texType      = TT_unknown;
    _min_filter   = GL_NEAREST;
    _mag_filter   = GL_NEAREST;
    _wrap_s       = GL_REPEAT;
    _wrap_t       = GL_REPEAT;
    _target       = GL_TEXTURE_2D;
    _bumpScale    = 1.0f;
    _resizeToPow2 = false;
    _autoCalcTM3D = false;
    _bytesOnGPU   = 0;
}
//-----------------------------------------------------------------------------
//! ctor 2D textures with internal image allocation
SLGLTexture::SLGLTexture(SLstring  filename,
                         SLint     min_filter,
                         SLint     mag_filter,
                         SLTextureType type,
                         SLint     wrapS,
                         SLint     wrapT) : 
            SLObject(SLUtils::getFileName(filename), filename)
{  
    assert(filename!="");
    _stateGL = SLGLState::getInstance();
    _texType = type==TT_unknown ? detectType(filename) : type;

    load(filename);
   
    _min_filter   = min_filter;
    _mag_filter   = mag_filter;
    _wrap_s       = wrapS;
    _wrap_t       = wrapT;
    _target       = GL_TEXTURE_2D;
    _texName      = 0;
    _bumpScale    = 1.0f;
    _resizeToPow2 = false;
    _autoCalcTM3D = false;
    _needsUpdate  = false;
    _bytesOnGPU   = 0;
   
    // Add pointer to the global resource vectors for deallocation
    SLScene::current->textures().push_back(this);
}
//-----------------------------------------------------------------------------
//! ctor for 3D texture
SLGLTexture::SLGLTexture(SLVstring files,
                         SLint     min_filter,
                         SLint     mag_filter)
{
    assert(files.size() > 1);
    _stateGL = SLGLState::getInstance();
    _texType = TT_color;

    for (auto filename : files)
        load(filename);

    _min_filter   = min_filter;
    _mag_filter   = mag_filter;
    _wrap_s       = GL_REPEAT;
    _wrap_t       = GL_REPEAT;
    #ifdef SL_GLES2
    _target       = GL_TEXTURE_2D;
    #else
    _target       = GL_TEXTURE_3D;
    #endif
    _texName      = 0;
    _bumpScale    = 1.0f;
    _resizeToPow2 = false;
    _autoCalcTM3D = true;
    _needsUpdate  = false;
    _bytesOnGPU   = 0;

    // Add pointer to the global resource vectors for deallocation
    SLScene::current->textures().push_back(this);
}
//-----------------------------------------------------------------------------
//! ctor for cube mapping with internal image allocation
SLGLTexture::SLGLTexture(SLstring  filenameXPos,
                         SLstring  filenameXNeg,
                         SLstring  filenameYPos,
                         SLstring  filenameYNeg,
                         SLstring  filenameZPos,
                         SLstring  filenameZNeg,
                         SLint     min_filter,
                         SLint     mag_filter,
                         SLTextureType type) : SLObject(filenameXPos)
{  
    _stateGL = SLGLState::getInstance();
    _texType = type==TT_unknown ? detectType(filenameXPos) : type;
   
    assert(filenameXPos!=""); load(filenameXPos);
    assert(filenameXNeg!=""); load(filenameXNeg);
    assert(filenameYPos!=""); load(filenameYPos);
    assert(filenameYNeg!=""); load(filenameYNeg);
    assert(filenameZPos!=""); load(filenameZPos);
    assert(filenameZNeg!=""); load(filenameZNeg);
             
    _min_filter  = min_filter;
    _mag_filter  = mag_filter;
    _wrap_s      = GL_REPEAT;
    _wrap_t      = GL_REPEAT;
    _target      = GL_TEXTURE_CUBE_MAP;
    _texName     = 0;
    _bumpScale   = 1.0f;
    _resizeToPow2 = false;
    _autoCalcTM3D = false;
    _needsUpdate  = false;
    _bytesOnGPU   = 0;

    SLScene::current->textures().push_back(this);
}
//-----------------------------------------------------------------------------
SLGLTexture::~SLGLTexture()
{  
    //SL_LOG("~SLGLTexture(%s)\n", name().c_str());
    clearData();
}
//-----------------------------------------------------------------------------
void SLGLTexture::clearData()
{
    glDeleteTextures(1, &_texName);

    numBytesInTextures -= _bytesOnGPU;

    for (SLint i=0; i<_images.size(); ++i)
    {   delete _images[i];
        _images[i] = 0;
    }
    _images.clear();

    _texName = 0;
    _bytesOnGPU = 0;
    _vaoSprite.clearAttribs();
}
//-----------------------------------------------------------------------------
//! Loads the texture, converts color depth & applies the mirroring
void SLGLTexture::load(SLstring filename)
{
    // Load the file directly
    if (!SLFileSystem::fileExists(filename))
    {   filename = defaultPath + filename;
        if (!SLFileSystem::fileExists(filename))
        {   SLstring msg = "SLGLTexture: File not found: " + filename;
            SL_EXIT_MSG(msg.c_str());
        }
    }
    
    _images.push_back(new SLImage(filename));
}
//-----------------------------------------------------------------------------
void SLGLTexture::setVideoImage(SLstring videoImageFile)
{
     load(videoImageFile);
    _min_filter = GL_LINEAR;
    _mag_filter = GL_LINEAR;
    _needsUpdate = false;
}
//-----------------------------------------------------------------------------
//! Copies the image data from a video camera into the current video image
/*!
\brief SLGLTexture::copyVideoImage
\param camWidth Width in pixels of the camera image
\param camHeight Height in pixels of the camera image
\param srcFormat Pixel format according to the OpenGL pixel formats
\param data Pointer to the first byte of the first pixel
\param isTopLeft Flag if the data pointer points to the top left pixel
\return Returns true if the texture was rebuilt
It is important that passed pixel format is either PF_LUMINANCE, RGB or RGBA.
otherwise an expensive conversion must be done.
*/
SLbool SLGLTexture::copyVideoImage(SLint camWidth,
                                   SLint camHeight,
                                   SLPixelFormat srcFormat,
                                   SLuchar* data,
                                   SLbool isContinuous,
                                   SLbool isTopLeft)
{                         
    bool needsRebuild = _images[0]->load(camWidth,
                                         camHeight,
                                         srcFormat,
                                         PF_rgb,
                                         data,
                                         isContinuous,
                                         isTopLeft);
    //_images[0]->savePNG("Test.png");

    // OpenGL ES 2 only can resize non-power-of-two texture with clamp to edge
    _wrap_s = GL_CLAMP_TO_EDGE;
    _wrap_t = GL_CLAMP_TO_EDGE;
    
    if (needsRebuild)
        build();
    
    _needsUpdate = true;
    return needsRebuild;
}
//-----------------------------------------------------------------------------
/*! 
Builds an OpenGL texture object with the according OpenGL commands.
This texture creation must be done only once when a valid OpenGL rendering
context is present. This function is called the first time within the enable
method which is called by object that uses the texture.
*/
void SLGLTexture::build(SLint texID)
{  
    assert(texID>=0 && texID<32);
    
    if (_images.size()==0) 
        SL_EXIT_MSG("No images loaded in SLGLTexture::build");
  
    // delete texture name if it already exits
    if (_texName) 
    {   glDeleteTextures(1, &_texName);
        _texName = 0;
    }
    
    // get max texture size
    SLint texMaxSize=0;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texMaxSize);        

    // check if texture has to be resized
    if (_resizeToPow2)
    {   SLuint w2 = closestPowerOf2(_images[0]->width());
        SLuint h2 = closestPowerOf2(_images[0]->height());
        if (w2==0) SL_EXIT_MSG("Image can not be rescaled: width=0");
        if (h2==0) SL_EXIT_MSG("Image can not be rescaled: height=0");
        if (w2!=_images[0]->width() || h2!=_images[0]->height())
            _images[0]->resize(w2, h2);
    }
    
    // check 2D size
    if (_target == GL_TEXTURE_2D)
    {   if (_images[0]->width()  > (SLuint)texMaxSize)
            SL_EXIT_MSG("SLGLTexture::build: Texture width is too big.");
        if (_images[0]->height() > (SLuint)texMaxSize)
            SL_EXIT_MSG("SLGLTexture::build: Texture height is too big.");
    }
    
    #ifndef SL_GLES2
    // check 3D size
    if (_target == GL_TEXTURE_3D)
    {   SLint texMax3DSize=0;
        glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &texMax3DSize);
        for (auto img : _images)
        {   if (img->width()  > (SLuint)texMax3DSize)
                SL_EXIT_MSG("SLGLTexture::build: 3D Texture width is too big.");
            if (img->height() > (SLuint)texMax3DSize)
                SL_EXIT_MSG("SLGLTexture::build: 3D Texture height is too big.");
            if (img->width()  != _images[0]->width() ||
                img->height() != _images[0]->height())
                SL_EXIT_MSG("SLGLTexture::build: Not all images of the 3D texture have the same size.");
        }
    }
    #endif
      
    // check cube mapping capability & max. cube map size
    if (_target==GL_TEXTURE_CUBE_MAP)
    {   SLint texMaxCubeSize;  
        glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &texMaxCubeSize);
        if (_images[0]->width()  > (SLuint)texMaxCubeSize)
            SL_EXIT_MSG("SLGLTexture::build: Cube Texture width is too big.");
        if (_images.size() != 6)
            SL_EXIT_MSG("SLGLTexture::build: Not six images provided for cube map texture.");
    }
      
    // Generate texture names
    glGenTextures(1, &_texName);
      
    _stateGL->activeTexture(GL_TEXTURE0+texID);

    // create binding and apply texture properties
    _stateGL->bindTexture(_target, _texName);
   
    // check if anisotropic texture filter extension is available      
    if (maxAnisotropy < 0.0f)
    {   if (_stateGL->hasExtension("GL_EXT_texture_filter_anisotropic"))
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        else 
        {   maxAnisotropy = 0.0f;
            cout << "GL_EXT_texture_filter_anisotropic not available.\n";
        }
    }
      
   // apply anisotropic or minification filter
   SLfloat anisotropy = 1.0f; // = off
   if (_min_filter > GL_LINEAR_MIPMAP_LINEAR)
   {    if (_min_filter == SL_ANISOTROPY_MAX)  
             anisotropy = maxAnisotropy;
        else anisotropy = min((SLfloat)(_min_filter-GL_LINEAR_MIPMAP_LINEAR), 
                              maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
   } else glTexParameteri(_target, GL_TEXTURE_MIN_FILTER, _min_filter);
      
    // apply magnification filter only GL_NEAREST & GL_LINEAR is allowed
    glTexParameteri(_target, GL_TEXTURE_MAG_FILTER, _mag_filter);
      
    // apply texture wrapping modes
    glTexParameteri(_target, GL_TEXTURE_WRAP_S, _wrap_s);
    glTexParameteri(_target, GL_TEXTURE_WRAP_T, _wrap_t);
      
    // Build textures
    if (_target == GL_TEXTURE_2D)
    {
        //////////////////////////////////////////
        glTexImage2D(GL_TEXTURE_2D,
                     0, 
                     _images[0]->format(),
                     _images[0]->width(),
                     _images[0]->height(),
                     0,
                     _images[0]->format(),
                     GL_UNSIGNED_BYTE, 
                     (GLvoid*)_images[0]->data());
        //////////////////////////////////////////

        _bytesOnGPU += _images[0]->bytesPerImage();
        
        if (_min_filter>=GL_NEAREST_MIPMAP_NEAREST)
        {   if (_stateGL->glIsES2() || 
                _stateGL->glIsES3() || 
                _stateGL->glVersionNOf() >= 3.0)
                glGenerateMipmap(GL_TEXTURE_2D);
            else
                build2DMipmaps(GL_TEXTURE_2D, 0);

            // Mipmaps use 1/3 more memory on GPU
            _bytesOnGPU = (SLuint)((SLfloat)_bytesOnGPU * 1.333333333f);
        }

        numBytesInTextures += _bytesOnGPU;
    } 
    else if (_target == GL_TEXTURE_CUBE_MAP)
    {
        for (SLint i=0; i<6; i++)
        {
            //////////////////////////////////////////////
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,
                         0, 
                         _images[i]->format(),
                         _images[i]->width(),
                         _images[i]->height(),
                         0,
                         _images[i]->format(),
                         GL_UNSIGNED_BYTE,
                         (GLvoid*)_images[i]->data());
            //////////////////////////////////////////////

            _bytesOnGPU += _images[0]->bytesPerImage();
        }

        numBytesInTextures += _bytesOnGPU;
        if (_min_filter>=GL_NEAREST_MIPMAP_NEAREST)
        {
            glGenerateMipmap(GL_TEXTURE_2D);

            // Mipmaps use 1/3 more memory on GPU
            _bytesOnGPU = (SLuint)((SLfloat)_bytesOnGPU * 1.333333333f);
        }
    }
    #ifndef SL_GLES2
    else if (_target == GL_TEXTURE_3D)
    {
        // temporary buffer for 3D image data
        SLVuchar buffer(_images[0]->bytesPerImage() * _images.size());
        SLuchar* imageData = &buffer[0];

        // copy each image data into temp. buffer
        for (SLImage* img : _images)
        {   memcpy(imageData, img->data(), img->bytesPerImage());
            imageData += img->bytesPerImage();
            _bytesOnGPU += _images[0]->bytesPerImage();
        }
        
        /////////////////////////////////////////////////////
        glTexImage3D(GL_TEXTURE_3D,
                     0,                     //Mipmap level,
                     _images[0]->format(),  //Internal format
                     _images[0]->width(),
                     _images[0]->height(),
                     (SLsizei)_images.size(),
                     0,                     //Border
                     _images[0]->format(),  //Format
                     GL_UNSIGNED_BYTE,      //Data type
                     &buffer[0]);
        /////////////////////////////////////////////////////
        
        numBytesInTextures += _bytesOnGPU;
    }
    #endif

    GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
/*!
SLGLTexture::bindActive binds the active texture. This method must be called 
by the object that uses the texture every time BEFORE the its rendering. 
The texID is only used for multi texturing. Before the first time the texture
is passed to OpenGL.
*/
void SLGLTexture::bindActive(SLint texID)
{
    assert(texID>=0 && texID<32);
   
    // if texture not exists build it
    if (!_texName) build(texID);
   
    if (_texName)
    {   _stateGL->activeTexture(GL_TEXTURE0 + texID);
        _stateGL->bindTexture(_target, _texName);
        SLScene* s = SLScene::current;
        
        #ifdef SL_HAS_OPENCV
        if (this == s->videoTexture() &&
            s->usesVideo() &&
            _needsUpdate)
        {   fullUpdate();
            _needsUpdate = false;
        }
        #endif // SL_HAS_OPENCV
    }

    GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
/*!
Fully updates the OpenGL internal texture data by the image data 
*/
void SLGLTexture::fullUpdate()
{  
    if (_texName && _images.size() &&
        _images[0]->data() &&
        _target == GL_TEXTURE_2D)
    {   if (_min_filter==GL_NEAREST || _min_filter==GL_LINEAR)
        {   
            numBytesInTextures -= _bytesOnGPU;

            /////////////////////////////////////////////
            glTexSubImage2D(_target, 0, 0, 0,
                            _images[0]->width(),
                            _images[0]->height(),
                            _images[0]->format(),
                            GL_UNSIGNED_BYTE, 
                            (GLvoid*)_images[0]->data());
            /////////////////////////////////////////////
            
            _bytesOnGPU = _images[0]->bytesPerImage();
            numBytesInTextures += _bytesOnGPU;
        } 
    }
    GET_GL_ERROR;
}
//-----------------------------------------------------------------------------
//! Draws the texture as 2D sprite with OpenGL buffers
/*! Draws the texture as a flat 2D sprite with a height and a width on two
triangles with zero in the bottom left corner: <br>
          w
       +-----+
       |    /|
       |   / |
    h  |  /  |
       | /   |
       |/    |
     0 +-----+
       0
*/
void SLGLTexture::drawSprite(SLbool doUpdate)
{
    SLfloat w = (SLfloat)_images[0]->width();
    SLfloat h = (SLfloat)_images[0]->height();

    // build buffer object once
    if (!_vaoSprite.id())
    {
        // Vertex X & Y of corners
        SLVVec2f P = {{0.0f, h}, 
                      {0.0f, 0.0f}, 
                      {w,    h}, 
                      {w,    0.0f}};
      
        // Texture coords of corners
        SLVVec2f T = {{0.0f, 1.0f}, 
                      {0.0f, 0.0f}, 
                      {1.0f, 1.0f}, 
                      {1.0f, 0.0f}};
      
        // Indexes for a triangle strip
        SLVushort I = {0,1,2,3};
    
        SLGLProgram* sp = SLScene::current->programs(SP_TextureOnly);
        sp->useProgram();
        _vaoSprite.setAttrib(AT_position, sp->getAttribLocation("a_position"), &P);
        _vaoSprite.setAttrib(AT_texCoord, sp->getAttribLocation("a_texCoord"), &T);
        _vaoSprite.setIndices(&I);
        _vaoSprite.generate(4);
    }

    bindActive(0);              // Enable & build texture
    if (doUpdate) fullUpdate(); // Update the OpenGL texture on each draw
   
    // Draw the character triangles
    SLMat4f mvp(_stateGL->projectionMatrix * _stateGL->modelViewMatrix);
    SLGLProgram* sp = SLScene::current->programs(SP_TextureOnly);
    sp->useProgram();
    sp->uniformMatrix4fv("u_mvpMatrix", 1, (SLfloat*)&mvp);
    sp->uniform1i("u_texture0", 0);
 
    ///////////////////////////////////////////////
    _vaoSprite.drawElementsAs(PT_triangleStrip);
    ///////////////////////////////////////////////
}
//-----------------------------------------------------------------------------
/*!
getTexelf returns a pixel color with its s & t texture coordinates.
If the OpenGL filtering is set to GL_LINEAR a bilinear interpolated color out
of four neighboring pixels is return. Otherwise the nearest pixel is returned.
*/
SLCol4f SLGLTexture::getTexelf(SLfloat s, SLfloat t)
{     
    // transform tex coords with the texture matrix
    s = s * _tm.m(0) + _tm.m(12);
    t = t * _tm.m(5) + _tm.m(13); 

    // Bilinear interpolation
    if (_min_filter==GL_LINEAR || _mag_filter==GL_LINEAR)
         return _images[0]->getPixelf(s, t);
    else return _images[0]->getPixeli((SLint)(s*_images[0]->width()),
                                      (SLint)(t*_images[0]->height()));
}
//-----------------------------------------------------------------------------
/*! 
dsdt calculates the partial derivation (gray value slope) at s,t for bump
mapping either from a height map or a normal map
*/
SLVec2f SLGLTexture::dsdt(SLfloat s, SLfloat t)
{
    SLVec2f dsdt(0,0);
    SLfloat ds = 1.0f / _images[0]->width();
    SLfloat dt = 1.0f / _images[0]->height();
   
    if (_texType==TT_height)
    {   dsdt.x = (getTexelf(s+ds,t   ).x - getTexelf(s-ds,t   ).x) * -_bumpScale;
        dsdt.y = (getTexelf(s   ,t+dt).x - getTexelf(s   ,t-dt).x) * -_bumpScale;
    } else
    if (_texType==TT_normal)
    {   SLVec4f texel = getTexelf(s, t);
        dsdt.x = texel.r * 2.0f - 1.0f;
        dsdt.y = texel.g * 2.0f - 1.0f;
    }
    return dsdt;
}
//-----------------------------------------------------------------------------
//! Detects the texture type from the filename appendix (See SLTexType def.)
SLTextureType SLGLTexture::detectType(SLstring filename)
{
    SLstring name = SLUtils::getFileNameWOExt(filename);
    SLstring appendix = name.substr(name.length()-2, 2);
    if (appendix=="_C") return TT_color;
    if (appendix=="_N") return TT_normal;
    if (appendix=="_H") return TT_height;
    if (appendix=="_G") return TT_gloss;
    return TT_color;
}
//-----------------------------------------------------------------------------
//! Returns the closest power of 2 to a passed number.
SLuint SLGLTexture::closestPowerOf2(SLuint num)
{
    SLuint nextPow2 = 1;
    if (num <= 0) return 1;
   
    while (nextPow2 <= num) nextPow2 <<= 1;   
    SLint prevPow2 = nextPow2 >> 1;
   
    if (num-prevPow2 < nextPow2-num)
        return prevPow2; else return nextPow2;
}
//-----------------------------------------------------------------------------
//! Returns the next power of 2 to a passed number.
SLuint SLGLTexture::nextPowerOf2(SLuint num)
{
    SLuint nextPow2 = 1;
    if (num <= 0) return 1;
   
    while (nextPow2 <= num) nextPow2 <<= 1;   
    return nextPow2;
}
//-----------------------------------------------------------------------------
void SLGLTexture::build2DMipmaps(SLint target, SLuint index)
{  
    // Create the base level mipmap
    SLint level = 0;   
    glTexImage2D(target, 
                 level, 
                 _images[index]->bytesPerPixel(),
                 _images[index]->width(),
                 _images[index]->height(), 0,
                 _images[index]->format(),
                 GL_UNSIGNED_BYTE, 
                 (GLvoid*)_images[index]->data());
    GET_GL_ERROR;
    
    // working copy of the base mipmap   
    SLImage img2(*_images[index]);
   
    // create half sized sub level mipmaps
    while(img2.width() > 1 || img2.height() > 1 )
    {   level++;
        img2.resize(max(img2.width() >>1,(SLuint)1),
                    max(img2.height()>>1,(SLuint)1));
      
        //SLfloat gauss[9] = {1.0f, 2.0f, 1.0f,
        //                    2.0f, 4.0f, 2.0f,
        //                    1.0f, 2.0f, 1.0f};

        //img2.convolve3x3(gauss);

        // Debug output
        //SLchar filename[255];
        //sprintf(filename,"%s_L%d_%dx%d.png", _name.c_str(), level, img2.width(), img2.height());
        //img2.savePNG(filename);
      
        glTexImage2D(target, 
                     level, 
                     img2.bytesPerPixel(),
                     img2.width(), 
                     img2.height(), 0,
                     img2.format(),
                     GL_UNSIGNED_BYTE, 
                     (GLvoid*)img2.data());
        GET_GL_ERROR;
    }
}
//-----------------------------------------------------------------------------
//! Returns the texture type as string
SLstring SLGLTexture::typeName()
{
    switch(_texType)
    {
        case TT_unknown: return "Unknown type"; break;
        case TT_color:   return "color map"; break;
        case TT_normal:  return "normal map"; break;
        case TT_height:  return "hight map"; break;
        case TT_gloss:   return "gloss map"; break;
        case TT_font:    return "font map"; break;
        default: return "Unknown type";
    }
}
//-----------------------------------------------------------------------------
