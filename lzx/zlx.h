#pragma once

#ifndef _XBOX
#ifndef WIN32
#define LIBXENON
#endif
#endif

#ifdef _XBOX
// XBOX XDK INCLUDE
#include <xtl.h>
#endif
#ifdef LIBXENON
// XBOX LIBXENON INCLUDE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <xenos/xe.h>
#include <xenos/xenos.h>
#include <xenos/edram.h>
#include <console/console.h>
#endif
#ifdef WIN32
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#endif

#include <stdio.h>
#include <stdlib.h>

typedef unsigned int ZLXColor;

#ifdef WIN32
#undef DrawText
#endif

#ifndef LIBXENON
typedef IDirect3DDevice9 ZLXVideoDevice;
typedef IDirect3DTexture9 ZLXTexture;
typedef IDirect3DVertexShader9 ZLXVertexShader;
typedef IDirect3DPixelShader9 ZLXPixelShader;
#else
// LIBXENON INCLUDE
typedef struct XenosDevice ZLXVideoDevice;
typedef struct XenosSurface ZLXTexture;
typedef struct XenosShader ZLXVertexShader;
typedef struct XenosShader ZLXPixelShader;
#endif


#ifdef LIBXENON
#define DIRECTORY_SEPARATOR "/"
#else
#define DIRECTORY_SEPARATOR "\\"
#endif


// Error code

enum {
    // Simple error code
    X_OK = 0,
    X_FAIL = 1,
    // File error code
    X_FILE_NOT_FOUND = 100,
    // Memory error code
    X_OUT_OF_MEMORY = 200,

};

#ifdef LIBXENON
    extern struct XenosDevice _xe;
    extern struct XenosSurface *fb;
#endif

namespace ZLX {
    // Global video device
    extern ZLXVideoDevice * g_pVideoDevice;
#ifdef WIN32
    extern HWND hWnd;
#endif

#ifdef LIBXENON
    // Fatal error ... display line and wait ...
#define FatalError(x, ...) printf("[file %s, line %d]\r\n",__FILE__, __LINE__);printf(x);
#else
#define FatalError(x, ...) printf("[file %s, line %d]"x,__FILE__, __LINE__,__VA_ARGS__);while(1){;};
#endif
#ifdef WIN32
	void W32Update();
#endif
}