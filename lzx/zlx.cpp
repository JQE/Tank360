#include "zlx.h"

#ifdef LIBXENON
    struct XenosDevice _xe;
    struct XenosSurface *fb = NULL;
#endif

namespace ZLX {

    ZLXVideoDevice * g_pVideoDevice = NULL;
#ifdef WIN32
    HWND hWnd;

	void W32Update(){
		ShowWindow(hWnd, SW_SHOWDEFAULT);
        UpdateWindow(hWnd);

		// Enter the message loop
        MSG msg;
        ZeroMemory( &msg, sizeof( msg ) );
        //while( msg.message != WM_QUIT )
        {
            if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
	}
#endif

}