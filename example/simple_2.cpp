// lzx.cpp : Defines the entry point for the application.
//


#include "Console.h"
#include "Draw.h"
#ifdef LIBXENON

#include <debug.h>
#include <xenos/xenos.h>

#include <diskio/ata.h>
#include <diskio/dvd.h>
extern "C" {
#include <input/input.h>
}
#include <console/console.h>
#include <diskio/diskio.h>
#include <usb/usbmain.h>
#include <time/time.h>
#include <xenon_soc/xenon_power.h>
#include <dirent.h>
#endif


#include <math.h>
using namespace ZLX;

class MyConsole : public ZLX::Console {
public:

    void Run() {
        Init();

        // Disable auto render on format
        SetAutoRender(0);

        Format("test Lzx sample");

		ZLXTexture * tt = NULL;
		LoadTextureFromFile(g_pVideoDevice,"uda:/t1.png",&tt);

        float i;

        while (1) {
            // Add a string to buffer
            Format("test");

            // Begin to draw
            Begin();

			ZLX::Draw::DrawGradientRect(-1, -1,2, 2, 0xFF00FF00,0xFF00FFFF);
			ZLX::Draw::DrawTexturedRect(-1, -1,1, 1, tt);
			ZLX::Draw::DrawColoredRect(cos(i) / 2, sin(i) / 2, 0.2f, 0.2f, 0xFFFFFF00);

			// Draw some text using font
            m_font.Begin();
            
			// It use screen coord
            m_font.Scale(2.0f);
            m_font.DrawText("Some text in white", 0xffffffff, 100, 18);
            m_font.Scale(1.0f);
            m_font.DrawText("Other in red", 0xffFF0000, 300, 200);
            
			// End of using
            m_font.End();

            // Draw all text + form
            Render();

			// Draw is finished
            End();

            i += 0.01f;

            //Sleep(16);
        }
    }
private:

    void Init() {
#ifdef WIN32
        Create(".\\font\\", 0xFF0000FF, 0xFFFFFF00);
#else
        Create("game:\\Verdana_16", 0xFF0000FF, 0xFFFFFF00);
#endif
    };
};

int main() {
#ifdef LIBXENON
    usb_init();
    usb_do_poll();
    xenon_ata_init();
    dvd_init();
#endif
    MyConsole cApp;

    cApp.Run();

    while (1) {
    }

    return 0;
}
