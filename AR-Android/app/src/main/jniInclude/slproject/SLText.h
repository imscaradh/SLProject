//#############################################################################
//  File:      SLText.h
//  Author:    Marcus Hudritsch
//  Date:      July 2014
//  Codestyle: https://github.com/cpvrlab/SLProject/wiki/Coding-Style-Guidelines
//  Copyright: Marcus Hudritsch
//             This software is provide under the GNU General Public License
//             Please visit: http://opensource.org/licenses/GPL-3.0
//#############################################################################

#ifndef SLTEXT_H
#define SLTEXT_H

#include <stdafx.h>
#include <SLNode.h>
#include <SLAABBox.h>
#include <SLGLVertexArray.h>
#include <SLTexFont.h>

class SLSceneView;
class SLRay;

//-----------------------------------------------------------------------------
//! SLText creates a mesh using a textured font from SLTexFont
/*!
The text is passed as standard string that can contain line breaks (\\n).
Line breaks are only inserted if a maxWidth is defined. If the lineHeightFactor
is 1.0 the minimal line spacing is used.
*/
class SLText: public SLNode 
{  public:                     
                        SLText          (SLstring text,
                                         SLTexFont* font = SLTexFont::font09,
                                         SLCol4f txtCol = SLCol4f::WHITE,
                                         SLfloat maxWidth = 0.0f,
                                         SLfloat lineHeightFactor = 1.3f);

                       ~SLText          (){;}
            
            void        drawRec         (SLSceneView* sv);
            void        statsRec        (SLNodeStats &stats);
            SLAABBox&   updateAABBRec   ();
            SLbool      hitRec          (SLRay* ray){return false;}
    virtual void        drawMeshes      (SLSceneView* sv);
            
            void        preShade        (SLRay* ray){;}
            
            // Getters
            SLstring    text            (){return _text;}
            SLCol4f     color           (){return _color;}
            SLVec2f     size            (){return _font->calcTextSize(_text,
                                                                      _maxW,
                                                                      _lineH);}
            SLfloat     fontHeightPX    (){return(SLfloat)_font->charsHeight;}
            SLint       length          (){return (SLint)_text.length();}
            
   protected:    
            SLstring        _text;  //!< Text of the button
            SLTexFont*      _font;  //!< Font pointer of the preloaded font
            SLCol4f         _color; //!< RGBA-Color of the text
            SLfloat         _maxW;  //!< Max. width in pix. for wrapped text
            SLfloat         _lineH; //!< Line height factor for wrapped text
            SLGLVertexArray _vao;   //!< Vertex array for rendering
                        
};
//-----------------------------------------------------------------------------
#endif //SLSPHERE_H

