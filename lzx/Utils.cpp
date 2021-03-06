#include "zlx.h"
#include "Utils.h"
#ifdef LIBXENON
#include "usb/usbmain.h"
#endif
#include <stdio.h>
#include <stdlib.h>

#ifdef LIBXENON
#include <png.h>
#include <pnginfo.h>
#endif

namespace ZLX {

#ifdef LIBXENON
    int fd = 0;

    struct file_buffer_t {
        char name[256];
        unsigned char *data;
        long length;
        long offset;
    };

    struct pngMem {
        unsigned char *png_end;
        unsigned char *data;
        int size;
        int offset; //pour le parcours
    };

    int offset = 0;

    static void png_mem_read(png_structp png_ptr, png_bytep data, png_size_t length) {
        struct file_buffer_t *src = (struct file_buffer_t *) png_get_io_ptr(png_ptr);
        /* Copy data from image buffer */
        memcpy(data, src->data + src->offset, length);
        /* Advance in the file */
        src->offset += length;
    }

    //Lits un fichier png en mémoire

    struct XenosSurface *loadPNGFromMemory(unsigned char *PNGdata) {

        printf("Loading PNG from memory");
//        unsigned char header[8]; // 8 is the maximum size that can be checked
        int y;

        int width, height;
        png_byte color_type;
        png_byte bit_depth;

        png_structp png_ptr;
        png_infop info_ptr;
        int number_of_passes;
        png_bytep * row_pointers;

        offset = 0;

        struct file_buffer_t *file;
        file = (struct file_buffer_t *) malloc(sizeof (struct file_buffer_t));
        file->length = 1024 * 1024 * 5;
        file->data = (unsigned char *) malloc(file->length); //5mo ...
        file->offset = 0;
        memcpy(file->data, PNGdata, file->length);

        /* initialize stuff */
        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr) {
            printf("[read_png_file] png_create_read_struct failed\n");
            return 0;
        }

        info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr) {
            printf("[read_png_file] png_create_info_struct failed\n");
            return 0;
        }

        png_set_read_fn(png_ptr, (png_voidp *) file, png_mem_read); //permet de lire à  partir de pngfile buff

        //png_set_sig_bytes(png_ptr, 8);//on avance de 8 ?

        png_read_info(png_ptr, info_ptr);

        width = info_ptr->width;
        height = info_ptr->height;
        color_type = info_ptr->color_type;
        bit_depth = info_ptr->bit_depth;

        number_of_passes = png_set_interlace_handling(png_ptr);
        

        if (color_type != PNG_COLOR_TYPE_RGB && color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
            printf("no support :(\n");
            return 0;
        }

        if (color_type == PNG_COLOR_TYPE_RGB)
            png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);

        png_set_swap_alpha(png_ptr);

        png_read_update_info(png_ptr, info_ptr);
        
        //On créer la surface
        struct XenosSurface *surface = Xe_CreateTexture(g_pVideoDevice, width, height, 1, XE_FMT_8888 | XE_FMT_ARGB, 0);

        uint8_t *data = (uint8_t*) Xe_Surface_LockRect(g_pVideoDevice, surface, 0, 0, 0, 0, XE_LOCK_WRITE);

        row_pointers = (png_bytep*) malloc(sizeof (png_bytep) * height);
        for (y = 0; y < height; y++)
            row_pointers[y] = data + surface->wpitch * y;

        png_read_image(png_ptr, row_pointers);

        Xe_Surface_Unlock(g_pVideoDevice, surface);

        free(row_pointers);

        return surface;
    }

#endif

    int GetScreenWidth() {
#ifdef WIN32
        //return 850;
        return 1280;
#else
        return 1280;
#endif
    };

    int GetScreenHeight() {
#ifdef WIN32
        // return 480;
        return 720;
#else
        return 720;
#endif
    };

    int LoadTextureFromFile(ZLXVideoDevice * pDevice, char * pSrcFile, ZLXTexture**ppTexture) {
#ifndef LIBXENON
        if (SUCCEEDED(D3DXCreateTextureFromFile(pDevice, pSrcFile, ppTexture))) {
            return X_OK;
        }
        return E_FAIL;
#else
        unsigned char * PNGdata = NULL;
        unsigned int size = 0;
        LoadFile(pSrcFile, (void**) &PNGdata, &size);
        if (PNGdata != NULL) {
            *ppTexture = loadPNGFromMemory(PNGdata);
            printf("Successfully loaded PNG");
            return X_OK;
        }
        free(PNGdata);
        printf("Can't find %s\r\n", pSrcFile);
        return X_FAIL;
#endif
    }


    //--------------------------------------------------------------------------------------
    // Name: LoadFile()
    // Desc: Helper function to load a file
    //--------------------------------------------------------------------------------------

    int LoadFile(const char* strFileName, void** ppFileData, unsigned int * pdwFileSize) {
        if (pdwFileSize)
            *pdwFileSize = 0L;
#ifdef LIBXENON
        usb_do_poll();
#endif
        printf("LoadFile %s\r\n", strFileName);

        // Open the file for reading
        FILE * fd = fopen(strFileName, "rb");

        if (fd == NULL)
            return X_FILE_NOT_FOUND;

        // Seek to end to get filesize
        fseek(fd, 0, SEEK_END);

        // Get the filesize
        unsigned int dwFileSize = ftell(fd);

        // Seek to start
        fseek(fd, 0, SEEK_SET);

        void * pFileData = malloc(dwFileSize);

        if (pFileData == NULL) {
            fclose(fd);
            return X_OUT_OF_MEMORY;
        }
        unsigned int dwBytesRead = 0;
        dwBytesRead = fread(pFileData, dwFileSize, 1, fd);

        // Finished reading file
        fclose(fd);

        if (pdwFileSize)
            *pdwFileSize = dwFileSize;

        *ppFileData = pFileData;
        return X_OK;
    }

    /*

     */
    void UnloadFile(void* pFileData) {
        free(pFileData);
    }


#ifdef WIN32
    //-----------------------------------------------------------------------------
    // Name: MsgProc()
    // Desc: The window's message handler
    //-----------------------------------------------------------------------------
#include "posix\input.h"

    controller_data_s tmp;

    LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_KEYDOWN:
                memset(&tmp, 0, sizeof (controller_data_s));
                switch (wParam) {
                    case VK_DOWN:
                        tmp.down = 1;
                        break;
                    case VK_UP:
                        tmp.up = 1;
                        break;
                    case VK_LEFT:
                        tmp.left = 1;
                        break;
                    case VK_RIGHT:
                        tmp.right = 1;
                        break;
                    case VK_SPACE:
                        tmp.start = 1;
                        break;
                }
                set_controller_data(0, &tmp);
                return 0;

            case WM_DESTROY:
                PostQuitMessage(0);
                exit(0);
                return 0;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
#endif

#ifndef LIBXENON
    static D3DPRESENT_PARAMETERS m_d3dpp;
#endif

    static int isVideoInitialised = 0;

    int InitialiseVideo() {
        if (isVideoInitialised == 0) {
#ifdef WIN32
            // Register the window class
            WNDCLASSEX wc = {
                sizeof ( WNDCLASSEX), CS_CLASSDC, MsgProc, 0L, 0L,
                GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                "LZx Libxenon Pc Apps", NULL
            };
            RegisterClassEx(&wc);

            // Create the application's window
            hWnd = CreateWindowA("LZx Libxenon Pc Apps", "LZx Libxenon Pc Apps",
                    WS_OVERLAPPEDWINDOW, 0, 0, GetScreenWidth(), GetScreenHeight(),
                    NULL, NULL, wc.hInstance, NULL);

            ShowWindow(hWnd, SW_SHOWDEFAULT);
            UpdateWindow(hWnd);
#endif

#ifndef LIBXENON
            memset(&m_d3dpp, 0, sizeof (D3DPRESENT_PARAMETERS));

            m_d3dpp.Windowed = FALSE;

            m_d3dpp.BackBufferCount = 1;
            m_d3dpp.BackBufferWidth = 1280;
            m_d3dpp.BackBufferHeight = 720;
            m_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
#ifndef WIN32
            m_d3dpp.FrontBufferFormat = D3DFMT_LE_X8R8G8B8;
#endif
            m_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
            m_d3dpp.MultiSampleQuality = 0;

            m_d3dpp.EnableAutoDepthStencil = TRUE;
            m_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

            m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
            m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

            // Create the d3d Device
            LPDIRECT3D9 pD3D = Direct3DCreate9(D3D_SDK_VERSION);
#ifdef WIN32
            m_d3dpp.Windowed = TRUE;

            HRESULT hr = pD3D->CreateDevice(0, D3DDEVTYPE_HAL, hWnd,
                    D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                    &m_d3dpp, (::IDirect3DDevice9**) & g_pVideoDevice);
#else
            HRESULT hr = pD3D->CreateDevice(0, D3DDEVTYPE_HAL, NULL,
                    D3DCREATE_HARDWARE_VERTEXPROCESSING,
                    &m_d3dpp, (::IDirect3DDevice9**) & g_pVideoDevice);
#endif
            if (FAILED(hr)) {
                printf("Could not create font.\n");
                DebugBreak();
            }
#endif

#ifdef LIBXENON
            xenos_init(VIDEO_MODE_AUTO);

            g_pVideoDevice = &_xe;

            // Init Gpu
            Xe_Init(g_pVideoDevice);

            fb = Xe_GetFramebufferSurface(g_pVideoDevice);
            Xe_SetRenderTarget(g_pVideoDevice, fb);
            edram_init(g_pVideoDevice);
#endif

            isVideoInitialised = 1;
        }

        return X_OK;
    };

}
