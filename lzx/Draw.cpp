#include "Draw.h"
#include "zlx.h"

#ifndef LIBXENON
#include "draw_shader.h"
#else
#include "draw_c_p_psu.h"
#include "draw_t_p_psu.h"
#include "draw_v_vsu.h"
#endif

namespace ZLX {
    static ZLXVertexShader * m_pVertexShader;
    static ZLXPixelShader * m_pPixelShader;
	static ZLXPixelShader * m_pPixelTexturedShader;

    static int isShaderLoaded = FALSE;

    typedef struct DrawVerticeFormats {
        float x, y; //z/w remove it ...
        float u, v;
        ZLXColor color;
    } DrawVerticeFormats;


#ifndef LIBXENON
    LPDIRECT3DVERTEXDECLARATION9 m_pVertexDecl;
#endif

    static int CreateShader();

	void Draw::DrawGradientRect(float x, float y, float w, float h, ZLXColor TopColor, ZLXColor BackColor){
		 if (isShaderLoaded == FALSE) {
            CreateShader();
        }

        DrawVerticeFormats Rect[6];

        // Uv
        float bottom = 0.0f;
        float top = 1.0f;
        float left = 0.0f;
        float right = 1.0f;

        // Bottom left
        Rect[0].x = x;
        Rect[0].y = y;
        Rect[0].u = bottom;
        Rect[0].v = left;
        Rect[0].color = BackColor;

        // top left
        Rect[1].x = x;
        Rect[1].y = y + h;
        Rect[1].u = top;
        Rect[1].v = left;
        Rect[1].color = TopColor;

        // bottom right
        Rect[2].x = x + w;
        Rect[2].y = y;
        Rect[2].u = bottom;
        Rect[2].v = right;
        Rect[2].color = BackColor;

        // bottom right
        Rect[3].x = x + w;
        Rect[3].y = y;
        Rect[3].u = bottom;
        Rect[3].v = right;
        Rect[3].color = BackColor;

        // top left
        Rect[4].x = x;
        Rect[4].y = y + h;
        Rect[4].u = top;
        Rect[4].v = left;
        Rect[4].color = TopColor;

        // top right
        Rect[5].x = x + w;
        Rect[5].y = y + h;
        Rect[5].u = top;
        Rect[5].v = right;
        Rect[5].color = TopColor;

        int len = 6 * sizeof (DrawVerticeFormats);
#ifndef LIBXENON
        IDirect3DVertexBuffer9 * vb;
        void *v;
        g_pVideoDevice->CreateVertexBuffer(len, 0, 0, D3DPOOL_DEFAULT, &vb, 0);
        vb->Lock(0, len, (void**) &v, D3DLOCK_NO_DIRTY_UPDATE);

        memcpy(v, Rect, len);

        vb->Unlock();

        g_pVideoDevice->SetTexture(0, NULL);
        g_pVideoDevice->SetPixelShader(m_pPixelShader);
        g_pVideoDevice->SetVertexShader(m_pVertexShader);
        g_pVideoDevice->SetVertexDeclaration(m_pVertexDecl);
        g_pVideoDevice->SetStreamSource(0, vb, 0, sizeof (DrawVerticeFormats));

        g_pVideoDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
#else
        //DrawVerticeFormats *v = (DrawVerticeFormats *) malloc(len);

        Xe_VBBegin(g_pVideoDevice, sizeof (DrawVerticeFormats));
        //memcpy(v, Rect, len);
        Xe_VBPut(g_pVideoDevice, Rect, len);

        XenosVertexBuffer *vb = Xe_VBEnd(g_pVideoDevice);
        Xe_VBPoolAdd(g_pVideoDevice, vb);

        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));
        Xe_SetTexture(g_pVideoDevice, 0, NULL);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pPixelShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pVertexShader, 0);
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
#endif
	}

	void Draw::DrawTexturedRect(float x, float y, float w, float h, ZLXTexture * p_texture){
		if (isShaderLoaded == FALSE) {
            CreateShader();
        }

		DrawVerticeFormats Rect[6];

        // Uv
        float bottom = 1.0f;
        float top = 0.0f;
        float left = 0.0f;
        float right = 1.0f;

        // Bottom left
        Rect[0].x = x;
        Rect[0].y = y;
        Rect[0].u = left;
        Rect[0].v = bottom;

        // top left
        Rect[1].x = x;
        Rect[1].y = y + h;
        Rect[1].u = left;
        Rect[1].v = top;

        // bottom right
        Rect[2].x = x + w;
        Rect[2].y = y;
        Rect[2].u = right;
        Rect[2].v = bottom;

        // bottom right
        Rect[3].x = x + w;
        Rect[3].y = y;
        Rect[3].u = right;
        Rect[3].v = bottom;

        // top left
        Rect[4].x = x;
        Rect[4].y = y + h;
        Rect[4].u = left;
        Rect[4].v = top;

        // top right
        Rect[5].x = x + w;
        Rect[5].y = y + h;
        Rect[5].u = right;
        Rect[5].v = top;
        
        int len = 6 * sizeof (DrawVerticeFormats);
#ifndef LIBXENON
        IDirect3DVertexBuffer9 * vb;
        void *v;
        g_pVideoDevice->CreateVertexBuffer(len, 0, 0, D3DPOOL_DEFAULT, &vb, 0);
        vb->Lock(0, len, (void**) &v, D3DLOCK_NO_DIRTY_UPDATE);

        memcpy(v, Rect, len);

        vb->Unlock();

        g_pVideoDevice->SetTexture(0, p_texture);
		g_pVideoDevice->SetPixelShader(m_pPixelTexturedShader);
        g_pVideoDevice->SetVertexShader(m_pVertexShader);
        g_pVideoDevice->SetVertexDeclaration(m_pVertexDecl);
        g_pVideoDevice->SetStreamSource(0, vb, 0, sizeof (DrawVerticeFormats));

        g_pVideoDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
#else
        //DrawVerticeFormats *v = (DrawVerticeFormats *) malloc(len);

        Xe_VBBegin(g_pVideoDevice, sizeof (DrawVerticeFormats));
        //memcpy(v, Rect, len);
        Xe_VBPut(g_pVideoDevice, Rect, len);

        XenosVertexBuffer *vb = Xe_VBEnd(g_pVideoDevice);
        Xe_VBPoolAdd(g_pVideoDevice, vb);

        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));
        Xe_SetTexture(g_pVideoDevice, 0, p_texture);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pPixelTexturedShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pVertexShader, 0);
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
#endif
	}

        void Draw::DrawClipTexturedRect(float x, float y, float w, float h, float top, float bottom, float right, float left, ZLXTexture * p_texture){
	if (isShaderLoaded == FALSE) {
            CreateShader();
        }

        DrawVerticeFormats Rect[6];

        x = (x/(1280/2))-1;
        y = (y/(720/2))-1;
        
        float ratiow = (right-left)/(w/2);
        float ratioh = (top-bottom)/(h/2);

                    // Uv
        bottom = bottom/ h;
        top = top / h;
        left = left / w;
        right = right / w;
        //float bottom = 1.0f;
        //float top = 0.0f;
        //float left = 0.0f;
        //float right = 1.0f;

        // Bottom left
        Rect[0].x = x;
        Rect[0].y = y;
        Rect[0].u = left;
        Rect[0].v = bottom;

        // top left
        Rect[1].x = x;
        Rect[1].y = y + ratioh;
        Rect[1].u = left;
        Rect[1].v = top;

        // bottom right
        Rect[2].x = x + ratiow;
        Rect[2].y = y;
        Rect[2].u = right;
        Rect[2].v = bottom;

        // bottom right
        Rect[3].x = x + ratiow;
        Rect[3].y = y;
        Rect[3].u = right;
        Rect[3].v = bottom;

        // top left
        Rect[4].x = x;
        Rect[4].y = y + ratioh;
        Rect[4].u = left;
        Rect[4].v = top;

        // top right
        Rect[5].x = x + ratiow;
        Rect[5].y = y + ratioh;
        Rect[5].u = right;
        Rect[5].v = top;
        
        int len = 6 * sizeof (DrawVerticeFormats);
#ifndef LIBXENON
        IDirect3DVertexBuffer9 * vb;
        void *v;
        g_pVideoDevice->CreateVertexBuffer(len, 0, 0, D3DPOOL_DEFAULT, &vb, 0);
        vb->Lock(0, len, (void**) &v, D3DLOCK_NO_DIRTY_UPDATE);

        memcpy(v, Rect, len);

        vb->Unlock();

        g_pVideoDevice->SetTexture(0, p_texture);
		g_pVideoDevice->SetPixelShader(m_pPixelTexturedShader);
        g_pVideoDevice->SetVertexShader(m_pVertexShader);
        g_pVideoDevice->SetVertexDeclaration(m_pVertexDecl);
        g_pVideoDevice->SetStreamSource(0, vb, 0, sizeof (DrawVerticeFormats));

        g_pVideoDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
#else
        //DrawVerticeFormats *v = (DrawVerticeFormats *) malloc(len);

        Xe_VBBegin(g_pVideoDevice, sizeof (DrawVerticeFormats));
        //memcpy(v, Rect, len);
        Xe_VBPut(g_pVideoDevice, Rect, len);

        XenosVertexBuffer *vb = Xe_VBEnd(g_pVideoDevice);
        Xe_VBPoolAdd(g_pVideoDevice, vb);
        Xe_SetClearColor(g_pVideoDevice, 0x00000000);

        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));
        Xe_SetTexture(g_pVideoDevice, 0, p_texture);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pPixelTexturedShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pVertexShader, 0);
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
#endif
	}
        
    void Draw::DrawColoredRect(float x, float y, float w, float h, ZLXColor color) {

        if (isShaderLoaded == FALSE) {
            CreateShader();
        }

        DrawVerticeFormats Rect[6];

        // Uv
        float bottom = 0.0f;
        float top = 1.0f;
        float left = 0.0f;
        float right = 1.0f;

        // Bottom left
        Rect[0].x = x;
        Rect[0].y = y;
        Rect[0].u = bottom;
        Rect[0].v = left;
        Rect[0].color = color;

        // top left
        Rect[1].x = x;
        Rect[1].y = y + h;
        Rect[1].u = top;
        Rect[1].v = left;
        Rect[1].color = color;

        // bottom right
        Rect[2].x = x + w;
        Rect[2].y = y;
        Rect[2].u = bottom;
        Rect[2].v = right;
        Rect[2].color = color;

        // bottom right
        Rect[3].x = x + w;
        Rect[3].y = y;
        Rect[3].u = bottom;
        Rect[3].v = right;
        Rect[3].color = color;

        // top left
        Rect[4].x = x;
        Rect[4].y = y + h;
        Rect[4].u = top;
        Rect[4].v = left;
        Rect[4].color = color;

        // top right
        Rect[5].x = x + w;
        Rect[5].y = y + h;
        Rect[5].u = top;
        Rect[5].v = right;
        Rect[5].color = color;

        int len = 6 * sizeof (DrawVerticeFormats);
#ifndef LIBXENON
        IDirect3DVertexBuffer9 * vb;
        void *v;
        g_pVideoDevice->CreateVertexBuffer(len, 0, 0, D3DPOOL_DEFAULT, &vb, 0);
        vb->Lock(0, len, (void**) &v, D3DLOCK_NO_DIRTY_UPDATE);

        memcpy(v, Rect, len);

        vb->Unlock();

        g_pVideoDevice->SetTexture(0, NULL);
        g_pVideoDevice->SetPixelShader(m_pPixelShader);
        g_pVideoDevice->SetVertexShader(m_pVertexShader);
        g_pVideoDevice->SetVertexDeclaration(m_pVertexDecl);
        g_pVideoDevice->SetStreamSource(0, vb, 0, sizeof (DrawVerticeFormats));

        g_pVideoDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2);
#else
        //DrawVerticeFormats *v = (DrawVerticeFormats *) malloc(len);

        Xe_VBBegin(g_pVideoDevice, sizeof (DrawVerticeFormats));
        //memcpy(v, Rect, len);
        Xe_VBPut(g_pVideoDevice, Rect, len);

        XenosVertexBuffer *vb = Xe_VBEnd(g_pVideoDevice);
        Xe_VBPoolAdd(g_pVideoDevice, vb);

        Xe_SetStreamSource(g_pVideoDevice, 0, vb, 0, sizeof (DrawVerticeFormats));
        Xe_SetTexture(g_pVideoDevice, 0, NULL);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_PIXEL, m_pPixelShader, 0);
        Xe_SetShader(g_pVideoDevice, SHADER_TYPE_VERTEX, m_pVertexShader, 0);
        Xe_DrawPrimitive(g_pVideoDevice, XE_PRIMTYPE_TRIANGLELIST, 0, 2);
#endif
    };

    int CreateShader() {
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

        m_pPixelShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) draw_c_p_psu);
        Xe_InstantiateShader(g_pVideoDevice, m_pPixelShader, 0);

		m_pPixelTexturedShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) draw_t_p_psu);
        Xe_InstantiateShader(g_pVideoDevice, m_pPixelTexturedShader, 0);

        m_pVertexShader = Xe_LoadShaderFromMemory(g_pVideoDevice, (void*) draw_v_vsu);
        Xe_InstantiateShader(g_pVideoDevice, m_pVertexShader, 0);
        Xe_ShaderApplyVFetchPatches(g_pVideoDevice, m_pVertexShader, 0, &vbf);
		        
        isShaderLoaded = TRUE;
        
        return 0;
#else
        static const D3DVERTEXELEMENT9 decl[] = {
            { 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
            { 0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
            { 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
            D3DDECL_END()
        };

        g_pVideoDevice->CreateVertexDeclaration(decl, &m_pVertexDecl);

        ID3DXBuffer* pShaderCode;
        ID3DXBuffer* pErrorCode;

        HRESULT Result = D3DXCompileShader(g_strDrawVertexhader, strlen(g_strDrawVertexhader),
                NULL, NULL, "main", "vs.2.0", 0, &pShaderCode, &pErrorCode, NULL);
        if (SUCCEEDED(Result)) {
            Result = g_pVideoDevice->CreateVertexShader((DWORD*) pShaderCode->GetBufferPointer(),
                    &m_pVertexShader);
            // Release the compiled shader
            pShaderCode->Release();

            Result = D3DXCompileShader(g_strDrawPixelShader, strlen(g_strDrawPixelShader),
                    NULL, NULL, "main", "ps.2.0", 0, &pShaderCode, &pErrorCode, NULL);
            if (SUCCEEDED(Result)) {
                Result = g_pVideoDevice->CreatePixelShader((DWORD*) pShaderCode->GetBufferPointer(),
                        &m_pPixelShader);
                // Release the compiled shader
                pShaderCode->Release();

                if (SUCCEEDED(Result)) {
                    Result = S_OK; // I'm good.
                    //
					Result = D3DXCompileShader(g_strDrawPixelTexturedShader, strlen(g_strDrawPixelTexturedShader),
                    NULL, NULL, "main", "ps.2.0", 0, &pShaderCode, &pErrorCode, NULL);
					if (SUCCEEDED(Result)){

						 Result = g_pVideoDevice->CreatePixelShader((DWORD*) pShaderCode->GetBufferPointer(),
								&m_pPixelTexturedShader);
						// Release the compiled shader
						pShaderCode->Release();
						if (SUCCEEDED(Result)){
							isShaderLoaded = TRUE;
						}
					}
                    
                }
            }

        }
        if (pErrorCode)
            if (pErrorCode->GetBufferSize()) {
                printf("%s\r\n", (char*) pErrorCode->GetBufferPointer());
            }

        return Result;
#endif
    }
}