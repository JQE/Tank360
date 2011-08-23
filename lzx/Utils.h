#pragma once

#include "zlx.h"

namespace ZLX {

    void UnloadFile(void* pFileData);
    int LoadFile(const char* strFileName, void** ppFileData, unsigned int* pdwFileSize);

    /**
    Load a texture from a file (libxenon only png are loaded)
     **/
    int LoadTextureFromFile(ZLXVideoDevice * pDevice, char * pSrcFile, ZLXTexture**ppTexture);
    struct XenosSurface *CreateTextureFromTexture(XenosSurface* XS, int width, int height, int x, int y);
    int GetScreenWidth();
    int GetScreenHeight();

    /**
    Init video only 1 time
     **/
    int InitialiseVideo();
}
