#pragma once

#include "zlx.h"

//--------------------------------------------------------------------------------------
// Name: GLYPH_ATTR
// Desc: Structure to hold information about one glyph (font character image)
//--------------------------------------------------------------------------------------

typedef struct GLYPH_ATTR {
    unsigned short tu1, tv1, tu2, tv2; // Texture coordinates for the image
    short wOffset; // Pixel offset for glyph start
    short wWidth; // Pixel width of the glyph
    short wAdvance; // Pixels to advance after the glyph
    unsigned short wMask; // Channel mask
} GLYPH_ATTR;

typedef struct FontVerticeFormats {
    float x, y;
    float u, v;
    //unsigned short u, v;
    //unsigned int u;
    ZLXColor color;
} FontVerticeFormats;

namespace ZLX {

    class Font {
    private:
        ZLXTexture * m_pFontTexture;

        int CreateFontShader();
        int ReleaseFontShader();

        // Font information
        float m_fontHeight;
        float m_fontTopPadding;
        float m_fontBottomPadding;
        float m_fontYAdvance;

        // Font display info
        float m_XScaleFactor;
        float m_YScaleFactor;

        unsigned int m_cMaxGlyph;
        unsigned short * m_TranslatorTable; // ASCII to glyph lookup table

        // Glyph data for the font
        unsigned int m_dwNumGlyphs; // Number of valid glyphs
        GLYPH_ATTR * m_Glyphs; // Array of glyphs

		 float vScreenScale[2];
		 float vTexScale[2];

        float GetXSafeArea();
        float GetYSafeArea();


		
    public:
        Font(void);
        ~Font(void);

		void Scale(float mul){
			m_XScaleFactor = mul;
			m_YScaleFactor = mul;
		}

        int Create(ZLXTexture * pFontTexture, const void * pFontData);
        void Destroy();

        // string size function
        float GetTextWidth(const char * str);
        // Returns the dimensions of a text string
        void GetTextExtent(const char* str, float* pWidth,
                float* pHeight, BOOL bFirstLineOnly = FALSE) const;

		float GetHeight(){
			return m_fontHeight;
		}

        // Create a texture with text on it
        ZLXTexture * CreateTexture(const char *str, ZLXColor backColor = 0x00000000, ZLXColor textColor = 0xFFFFFFFF);

#ifndef LIBXENON
        LPDIRECT3DVERTEXDECLARATION9 m_pFontVertexDecl; // Shared vertex buffer
#endif
        
        ZLXVertexShader * m_pFontVertexShader; // Created vertex shader
        ZLXPixelShader* m_pFontPixelShader; // Created pixel shader

        // Rendering stuff
        void Begin();
        void End();

        void DrawText(const char * str);
		// Use screen coord
        void DrawText(const char * str, ZLXColor textColor, float x, float y);
		// Use float coord
		void DrawTextF(const char * str, ZLXColor textColor, float x, float y);

        inline ZLXTexture * GetTexture() const {
            return m_pFontTexture;
        }
    };

}