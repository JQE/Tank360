#include "Font.h"
#include "Utils.h"
#ifdef LIBXENON
//typedef unsigned char u8;
//typedef unsigned int u32;
#include "font_p_psu.h"
#include "font_v_vsu.h"
#else
#include "font_str.h"
#endif
//
// These two structures are mapped to data loaded from disk.
// DO NOT ALTER ANY ENTRIES OR YOU WILL BREAK 
// COMPATIBILITY WITH THE FONT FILE
//
#define ATGCALCFONTFILEHEADERSIZE(x) ( sizeof(unsigned int) + (sizeof(float)*4) + sizeof(unsigned short) + (sizeof(unsigned short)*(x)) )
#define ATGFONTFILEVERSION 5

#define MAXCHAR 1024

typedef struct FontFileHeaderImage_t {
    unsigned int m_dwFileVersion; // Version of the font file (Must match FONTFILEVERSION)
    float m_fFontHeight; // Height of the font strike in pixels
    float m_fFontTopPadding; // Padding above the strike zone
    float m_fFontBottomPadding; // Padding below the strike zone
    float m_fFontYAdvance; // Number of pixels to move the cursor for a line feed
    unsigned short m_cMaxGlyph; // Number of font characters (Should be an odd number to maintain DWORD Alignment)
    unsigned short m_TranslatorTable[1]; // ASCII to Glyph lookup table, NOTE: It's m_cMaxGlyph+1 in size.
    // Entry 0 maps to the "Unknown" glyph.
} FontFileHeaderImage_t;

// Font strike array. Immediately follows the FontFileHeaderImage_t
// structure image

typedef struct FontFileStrikesImage_t {
    unsigned int m_dwNumGlyphs; // Size of font strike array (First entry is the unknown glyph)
    GLYPH_ATTR m_Glyphs[1]; // Array of font strike uv's etc... NOTE: It's m_dwNumGlyphs in size
} FontFileStrikesImage_t;

#ifdef WIN32 // Declare some bitswap operation

inline float _byteswap_float(float p_source) {
    float ret;
    *(unsigned int*) &ret = _byteswap_ulong(*(const unsigned int*) &p_source);
    return ret;
}

void bswap_glyphs(GLYPH_ATTR * p_gly) {
    p_gly->tu1 = _byteswap_ushort(p_gly->tu1);
    p_gly->tu2 = _byteswap_ushort(p_gly->tu2);
    p_gly->tv1 = _byteswap_ushort(p_gly->tv1);
    p_gly->tv2 = _byteswap_ushort(p_gly->tv2);
    p_gly->wMask = _byteswap_ushort(p_gly->wMask);

    p_gly->wAdvance = _byteswap_ushort(p_gly->wAdvance);
    p_gly->wOffset = _byteswap_ushort(p_gly->wOffset);
    p_gly->wWidth = _byteswap_ushort(p_gly->wWidth);
}

#endif

static inline unsigned int unsplat32(unsigned int v) {
    // Splat the 4 bit per pixels from 0x1234 to 0x01020304
    v = ((v & 0xF000) << (24 - 12)) | ((v & 0xF00) << (16 - 8)) |
            ((v & 0xF0) << (8 - 4)) | (v & 0xF);

    // Perform a vectorized multiply to make 0x01020304 into 0x11223344
    v *= 0x11;
    return v;
}

namespace ZLX {

    Font::Font(void) {
        m_XScaleFactor = 1.0f;
        m_YScaleFactor = 1.0f;
    }

    Font::~Font(void) {
    }

    /**
     * Create the font shader
     * @return 
     */
    int Font::CreateFontShader() {
#ifdef LIBXENON 
        // Load already compiled shader
        static const struct XenosVBFFormat vbf = {
            3,
            {
                {XE_USAGE_POSITION, 0, XE_TYPE_FLOAT2},
                {XE_USAGE_TEXCOORD, 0, XE_TYPE_FLOAT2},
                {XE_USAGE_COLOR, 0, XE_TYPE_UBYTE4},
            }
        };

        m_pFontPixelShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) font_p_psu);
        Xe_InstantiateShader(g_pVideoDevice, m_pFontPixelShader, 0);

        m_pFontVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) font_v_vsu);
        Xe_InstantiateShader(g_pVideoDevice, m_pFontVertexShader, 0);
        Xe_ShaderApplyVFetchPatches(g_pVideoDevice, m_pFontVertexShader, 0, &vbf);

        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pFontPixelShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pFontVertexShader, 0);
#else

        static const D3DVERTEXELEMENT9 decl[] = {
            { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            { 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END()
        };

        g_pVideoDevice->CreateVertexDeclaration(decl, &m_pFontVertexDecl);


        ID3DXBuffer* pShaderCode;

        HRESULT Result = D3DXCompileShader(g_strFontShader, sizeof (g_strFontShader) - 1,
                NULL, NULL, "FontVertexShader", "vs.2.0", 0, &pShaderCode, NULL, NULL);
        if (SUCCEEDED(Result)) {
            Result = g_pVideoDevice->CreateVertexShader((DWORD*) pShaderCode->GetBufferPointer(),
                    &m_pFontVertexShader);
            // Release the compiled shader
            pShaderCode->Release();

            Result = D3DXCompileShader(g_strFontShader, sizeof (g_strFontShader) - 1,
                    NULL, NULL, "FontPixelShader", "ps.2.0", 0, &pShaderCode, NULL, NULL);
            if (SUCCEEDED(Result)) {
                Result = g_pVideoDevice->CreatePixelShader((DWORD*) pShaderCode->GetBufferPointer(),
                        &m_pFontPixelShader);
                // Release the compiled shader
                pShaderCode->Release();

                if (SUCCEEDED(Result)) {
                    Result = S_OK; // I'm good.
                    //
                }
            }

        }

#endif

        return X_OK;
    };

    float Font::GetXSafeArea() {
        return GetScreenWidth() / 20.f;
    };

    float Font::GetYSafeArea() {
        return GetScreenHeight() / 20.f;
    };

    static FontVerticeFormats FontVerticePool[MAXCHAR];
    static FontVerticeFormats ** p_fontVerticesBuffer = NULL;

    /**
     * Create a font instance
     * @param pFontTexture
     * @param pFontData
     * @return 
     */
    int Font::Create(ZLXTexture * pFontTexture, const void * pFontData) {

        CreateFontShader();

        // Save a pointer to the texture
        m_pFontTexture = pFontTexture;

        const unsigned char* pData = static_cast<const unsigned char*> (pFontData);

        // Check version of file (to make sure it matches up with the FontMaker tool)
        unsigned int dwFileVersion = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_dwFileVersion;

#ifdef WIN32
        //Bswap
        dwFileVersion = _byteswap_ulong(dwFileVersion);
#endif

        if (dwFileVersion == ATGFONTFILEVERSION) {
            m_fontHeight = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_fFontHeight;
            m_fontTopPadding = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_fFontTopPadding;
            m_fontBottomPadding = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_fFontBottomPadding;
            m_fontYAdvance = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_fFontYAdvance;

            // Point to the translator string which immediately follows the 4 floats
            m_cMaxGlyph = reinterpret_cast<const FontFileHeaderImage_t *> (pData)->m_cMaxGlyph;

            m_TranslatorTable = const_cast<FontFileHeaderImage_t*> (reinterpret_cast<const FontFileHeaderImage_t *> (pData))->m_TranslatorTable;

#ifdef WIN32
            // Bswap
            m_fontHeight = _byteswap_float(m_fontHeight);
            m_fontTopPadding = _byteswap_float(m_fontTopPadding);
            m_fontBottomPadding = _byteswap_float(m_fontBottomPadding);
            m_fontYAdvance = _byteswap_float(m_fontYAdvance);
            m_cMaxGlyph = _byteswap_ushort(m_cMaxGlyph); // ?

            // bswap translator table
            for (int i = 0; i < m_cMaxGlyph + 1; i++) {
                m_TranslatorTable[i] = _byteswap_ushort(m_TranslatorTable[i]);
            }
#endif
//            int offset = ATGCALCFONTFILEHEADERSIZE(m_cMaxGlyph + 1);
            pData += ATGCALCFONTFILEHEADERSIZE(m_cMaxGlyph + 1);

            // Read the glyph attributes from the file
            m_dwNumGlyphs = reinterpret_cast<const FontFileStrikesImage_t *> (pData)->m_dwNumGlyphs;

#ifdef WIN32
            // Bswap
            // bswap_glyphs(m_Glyphs);
            m_dwNumGlyphs = _byteswap_ulong(m_dwNumGlyphs);
#endif
            int len = m_dwNumGlyphs * sizeof (GLYPH_ATTR);
            m_Glyphs = (GLYPH_ATTR *) malloc(len);
            memcpy(m_Glyphs, reinterpret_cast<const FontFileStrikesImage_t *> (pData)->m_Glyphs, len);

#ifdef WIN32
            // bswap all data
            for (int i = 0; i < m_dwNumGlyphs; i++) {
                bswap_glyphs(&m_Glyphs[i]);
            }
#endif

        } else {
            printf("font fail ...");
            return X_FAIL;
        }

        // Init some array
        p_fontVerticesBuffer = (FontVerticeFormats**) malloc(MAXCHAR * sizeof (FontVerticeFormats*));
        for (int i = 0; i < MAXCHAR; i++) {
            p_fontVerticesBuffer[i] = (FontVerticeFormats*) malloc(6 * sizeof (FontVerticeFormats));
        }

        return X_OK;
    }

    /**
     * Draw some text
     * @param str
     * @param textColor
     * @param x
     * @param y
     */
    void Font::DrawTextF(const char * str, ZLXColor textColor, float x, float y) {
        x = (x + 1)*(GetScreenWidth() / 2.f);
        y = (y + 1)*(GetScreenHeight() / 2.f);

        DrawText(str, textColor, x, y);
    }

    /**
     * Draw some text
     * @param str
     * @param textColor
     * @param x (screen space coord)
     * @param y (screen space coord)
     */
    void Font::DrawText(const char * str, ZLXColor textColor, float x, float y) {
        if (str[0] == '\0')
            return;
        //Begin();

        // Set the color as a vertex shader constant
        float vColor[4];

        vColor[0] = ((textColor & 0x00ff0000) >> 16L) / 255.0F;
        vColor[1] = ((textColor & 0x0000ff00) >> 8L) / 255.0F;
        vColor[2] = ((textColor & 0x000000ff) >> 0L) / 255.0F;
        vColor[3] = ((textColor & 0xff000000) >> 24L) / 255.0F;

        //vColor[0]=vColor[1]=vColor[2]=vColor[3] = 255.0f;

#ifndef LIBXENON
        g_pVideoDevice->SetVertexShaderConstantF(1, vColor, 1);
        g_pVideoDevice->SetVertexShaderConstantF(2, vTexScale, 1);
        g_pVideoDevice->SetVertexShaderConstantF(3, vScreenScale, 1);
#else
        Xe_SetVertexShaderConstantF(g_pVideoDevice, 1, vColor, 1);
        Xe_SetVertexShaderConstantF(g_pVideoDevice, 2, vTexScale, 1);
        Xe_SetVertexShaderConstantF(g_pVideoDevice, 3, vScreenScale, 1);
#endif

        float XOrigin = -GetScreenWidth() / 2.0f + GetXSafeArea();
        float YOrigin = GetScreenHeight() / 2.0f - (m_fontYAdvance + GetYSafeArea());

        float m_CursorX = XOrigin + x; //-18.0f;
        float m_CursorY = YOrigin - y;
        float m_SlantFactor = 0.0f;

        // Get the number of character to draw
        int iNbChar = 0;

        // Draw all letter like a rectangle
        unsigned int letter;
        while ((letter = *str) != 0) {
            ++str;

            // Eof
            if (letter == '\0')
                break;

            if (letter == '\r') {
                //
                continue;
            }

            // New line
            if (letter == '\n') {
                m_CursorX = XOrigin + x;
                m_CursorY -= m_fontYAdvance;
                continue;
            }

            if (letter > m_cMaxGlyph)
                letter = 0;
            else
                letter = m_TranslatorTable[letter];

            GLYPH_ATTR * pGlyph = (GLYPH_ATTR *) & m_Glyphs[letter];

            float Offset = m_XScaleFactor * (float) pGlyph->wOffset;
            float Advance = m_XScaleFactor * (float) pGlyph->wAdvance;
            float Width = m_XScaleFactor * (float) pGlyph->wWidth;
            float Height = m_YScaleFactor * m_fontHeight;

            m_CursorX += Offset;

            float X4 = m_CursorX;
            float X1 = X4 + m_SlantFactor;
            float X3 = X4 + Width;
            float X2 = X1 + Width;
            float Y1 = m_CursorY;
            float Y3 = Y1 + Height;
            float Y2 = Y1;
            float Y4 = Y3;

            unsigned short tu1 = pGlyph->tu1;
            unsigned short tv1 = pGlyph->tv1;
            unsigned short tu2 = pGlyph->tu2;
            unsigned short tv2 = pGlyph->tv2;

            unsigned int channelSelector = unsplat32(pGlyph->wMask);

            p_fontVerticesBuffer[iNbChar][0].color = channelSelector;
            p_fontVerticesBuffer[iNbChar][1].color = channelSelector;
            p_fontVerticesBuffer[iNbChar][2].color = channelSelector;
            p_fontVerticesBuffer[iNbChar][3].color = channelSelector;
            p_fontVerticesBuffer[iNbChar][4].color = channelSelector;
            p_fontVerticesBuffer[iNbChar][5].color = channelSelector;

            //bottom left
            p_fontVerticesBuffer[iNbChar][0].x = X1;
            p_fontVerticesBuffer[iNbChar][0].y = Y1;
            p_fontVerticesBuffer[iNbChar][0].u = tu1;
            p_fontVerticesBuffer[iNbChar][0].v = tv2;

            // top left
            p_fontVerticesBuffer[iNbChar][1].x = X4;
            p_fontVerticesBuffer[iNbChar][1].y = Y4;
            p_fontVerticesBuffer[iNbChar][1].u = tu1;
            p_fontVerticesBuffer[iNbChar][1].v = tv1;

            p_fontVerticesBuffer[iNbChar][4].x = X4;
            p_fontVerticesBuffer[iNbChar][4].y = Y4;
            p_fontVerticesBuffer[iNbChar][4].u = tu1;
            p_fontVerticesBuffer[iNbChar][4].v = tv1;

            // bottom right
            p_fontVerticesBuffer[iNbChar][2].x = X2;
            p_fontVerticesBuffer[iNbChar][2].y = Y2;
            p_fontVerticesBuffer[iNbChar][2].u = tu2;
            p_fontVerticesBuffer[iNbChar][2].v = tv2;

            p_fontVerticesBuffer[iNbChar][3].x = X2;
            p_fontVerticesBuffer[iNbChar][3].y = Y2;
            p_fontVerticesBuffer[iNbChar][3].u = tu2;
            p_fontVerticesBuffer[iNbChar][3].v = tv2;

            // top right
            p_fontVerticesBuffer[iNbChar][5].x = X3;
            p_fontVerticesBuffer[iNbChar][5].y = Y3;
            p_fontVerticesBuffer[iNbChar][5].u = tu2;
            p_fontVerticesBuffer[iNbChar][5].v = tv1;

            m_CursorX += Advance;
            iNbChar++;
        }

#ifndef LIBXENON
        int len = iNbChar * 6 * sizeof (FontVerticeFormats);

        IDirect3DVertexBuffer9 * vb;
        g_pVideoDevice->CreateVertexBuffer(len, 0, 0, D3DPOOL_DEFAULT, &vb, 0);

        FontVerticeFormats *v;

        vb->Lock(0, len, (void**) &v, D3DLOCK_NO_DIRTY_UPDATE);

        int k = 0;
        for (int i = 0; i < iNbChar; i++) {
            for (int j = 0; j < 6; j++) {
                memcpy(&v[k], &p_fontVerticesBuffer[i][j], sizeof (FontVerticeFormats));
                k++;
            }
        }

        vb->Unlock();

        g_pVideoDevice->SetStreamSource(0, vb, 0, sizeof (FontVerticeFormats));
        g_pVideoDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, iNbChar * 2);

        vb->Release();

#else
        int len = iNbChar * 6 * sizeof (FontVerticeFormats);

        Xe_VBBegin(g_pVideoDevice, sizeof (FontVerticeFormats));

        int k = 0;
        for (int i = 0; i < iNbChar; i++) {
            for (int j = 0; j < 6; j++) {
                memcpy(&FontVerticePool[k], &p_fontVerticesBuffer[i][j], sizeof (FontVerticeFormats));
                k++;
            }
        }
        Xe_VBPut(g_pVideoDevice, FontVerticePool, len);

        XenosVertexBuffer *vb = Xe_VBEnd(g_pVideoDevice);
        Xe_VBPoolAdd(g_pVideoDevice, vb);

        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (FontVerticeFormats));
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, iNbChar * 2);

#endif

    }

    void Font::End() {
        // Restore some states ...
    }

    void Font::Begin() {
        // 2 =  coord -1, 1
        vScreenScale[0] = 2.0f / (GetScreenWidth());
        vScreenScale[1] = 2.0f / (GetScreenHeight());

#ifndef LIBXENON
        // Set Render states
        g_pVideoDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        g_pVideoDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        g_pVideoDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        g_pVideoDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);

        g_pVideoDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
        g_pVideoDevice->SetRenderState(D3DRS_ALPHAREF, 0x08);
        g_pVideoDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);

        g_pVideoDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        //	g_pVideoDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );

        g_pVideoDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
        g_pVideoDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_pVideoDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        //	g_pVideoDevice->SetRenderState( D3DRS_VIEWPORTENABLE, FALSE );

        g_pVideoDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        g_pVideoDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
        g_pVideoDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
        g_pVideoDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

        g_pVideoDevice->SetVertexDeclaration(m_pFontVertexDecl);
        g_pVideoDevice->SetVertexShader(m_pFontVertexShader);
        g_pVideoDevice->SetPixelShader(m_pFontPixelShader);

        // Set render state
        g_pVideoDevice->SetTexture(0, m_pFontTexture);

        // Set the texture scaling factor as a vertex shader constant
        D3DSURFACE_DESC TextureDesc;
        m_pFontTexture->GetLevelDesc(0, &TextureDesc); // Get the description

        // Read the TextureDesc here to ensure no load/hit/store from GetLevelDesc()
        vTexScale[0] = 1.0f / TextureDesc.Width; // LHS due to int->float conversion
        vTexScale[1] = 1.0f / TextureDesc.Height;

#else
        vTexScale[0] = 1.0f / m_pFontTexture->width; // Png is set to
        vTexScale[1] = 1.0f / m_pFontTexture->height;

        Xe_SetTexture(g_pVideoDevice, 0, m_pFontTexture);

        Xe_SetBlendOp(g_pVideoDevice, XE_BLENDOP_ADD);
        Xe_SetSrcBlend(g_pVideoDevice, XE_BLEND_SRCALPHA);
        Xe_SetDestBlend(g_pVideoDevice, XE_BLEND_INVSRCALPHA);

        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pFontPixelShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pFontVertexShader, 0);

        Xe_SetCullMode(g_pVideoDevice, XE_CULL_NONE);

#endif
    }

    float Font::GetTextWidth(const char * str) {
        float width, height;
        GetTextExtent(str, &width, &height);
        return width;
    }

    void Font::GetTextExtent(const char* str, float* pWidth,
            float* pHeight, BOOL bFirstLineOnly) const {
        int iwidth = 0;
        float height = m_fontHeight;

        int x = 0;
        int fy = m_fontHeight;

        if (str != NULL) {
            unsigned int letter;

            while ((letter = *str) != 0) {
                ++str;

                if (letter == '\n') {
                    x = 0.0f;
                    // Add some height
                    fy += m_fontYAdvance;
                    if (fy > height) {
                        height = fy;
                    }
                    continue;
                }

                if (letter == '\r') {
                    continue;
                }

                if (letter > m_cMaxGlyph)
                    letter = 0;
                else
                    letter = m_TranslatorTable[letter];

                // get char size
                GLYPH_ATTR * pGlyph = (GLYPH_ATTR *) & m_Glyphs[letter];

                x += pGlyph->wOffset;
                x += pGlyph->wAdvance;

                // get the bigger
                if (x > iwidth)
                    iwidth = x;
            }

        }

        *pWidth = static_cast<float> (iwidth);
        *pHeight = m_fontHeight;

    }






}



