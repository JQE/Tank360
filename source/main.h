/* 
 * File:   menu.h
 * Author: cc
 *
 * Created on 22 juillet 2011, 20:39
 */

#ifndef MAIN_H
#define	MAIN_H

#include <zlx/Console.h>
#include <zlx/Draw.h>
#include <zlx/Hw.h>
#include <zlx/Utils.h>

#ifdef LIBXENON

#include <debug.h>
#include <xenos/xenos.h>

#include <diskio/ata.h>
extern "C" {
#include <input/input.h>
}
#include <console/console.h>
#include <diskio/disc_io.h>
#include <usb/usbmain.h>
#include <elf/elf.h>
#include <time/time.h>
#include <xenon_soc/xenon_power.h>
#include <xenon_smc/xenon_smc.h>
#include <ppc/timebase.h>
#else
#include "posix\input.h"
#endif
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>

#include "Tank.h"
#include "Tools.h"

using namespace ZLX;



class MyConsole : public ZLX::Console {
private:
    static const unsigned int DefaultColor = 0x88FFFFFF;
    static const unsigned int SelectedColor = 0xFFFFFFFF;
    static const unsigned int FocusColor = 0xFFFFFFFF;
    static const unsigned int BlurColor = 0x88FFFFFF;


    ZLXTexture * bg;
    
    cTank P1;
    cTank P2;
    xTools Tools;

    float progressPct;

    // handle to bdev
    int handle;
    int sCount;

    controller_data_s ctrl;
    controller_data_s old_ctrl;
    
    controller_data_s ctrl1;
    controller_data_s old_ctrl1;

    void RenderApp();
    void Update();
    void Init();
    
public:
    BOOL Warning(const char *message);
    void Alert(const char *message);
    void Run();
};
    
    

#endif	/* MENU_H */

