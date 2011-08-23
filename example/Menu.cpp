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
#else
#include "posix\dirent.h"
#endif
#include <stdio.h>

#include <math.h>
using namespace ZLX;

#define MAX_PATH 512
#define MAX_FILES 1000


class MyConsole : public ZLX::Console {
public:

	
    void Run() {
        Init();

        // Disable auto render on format
        SetAutoRender(0);

        Format("test Lzx sample");

        float i = 0.0f;

		ScanDir();

        while (1) {
            // Begin to draw
            Begin();

            // Draw some text using font
            m_font.Begin();
            // It use screen coord

            i += 0.01f;
			
			m_font.Begin();
			for(int k =0;k<entrycount;k++){
				// Format("[%s]",entries[i]);
				m_font.DrawText(entries[k], 0xFFFFFFFF, 0, m_font.GetHeight()*k);
			}
			m_font.End();


			// Draw all text + form
            Render();

            // Draw is finished
            End();
        }
    }
private:
	char currentPath[MAX_PATH];
	char entries[MAX_FILES][MAX_PATH];

	// Number of file found
	int entrycount;

	void ScanDir(){
		entrycount = 0;
		DIR * rep = opendir(".");
    
		if (rep != NULL)
		{
			struct dirent * ent;
        
			while ((ent = readdir(rep)) != NULL)
			{
				entrycount++;
				// printf("%s\n", ent->d_name);
				strcpy(entries[entrycount],ent->d_name);
			}
        
			closedir(rep);
		}
     
		return;
	}


    void Init() {
		entrycount = 0;
#ifdef WIN32
        Create("font\\font", 0xFF000000, 0xFFFFFF00);
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
