// lzx.cpp : Defines the entry point for the application.
//

#include "main.h"

#define STICK_THRESHOLD 25000

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
#define CLAMP(x,low,high) {if(x<low)x=low;if(x>high)x=high;}

char rootpath[256] = {0};

int time() {
  return (int)mftb();
}
    

void MyConsole::RenderApp() {
    // Begin to draw
    Begin();
    unsigned int color = 0xFF616161;
    char* message1;
    message1 = (char*)malloc(256);
    char* message2;
    message2 = (char*)malloc(256);
    
    sprintf(message1, "Play1: %d", P1.score);
    sprintf(message2, "play2: %d", P2.score);
    

    Draw::DrawColoredRect(-1.0f, 0.8f, 2.0f, 0.20f, color);
    m_font.Begin();

    m_font.DrawTextF(message1, SelectedColor, -1.0f, -1.0f);
    m_font.DrawTextF(message2, SelectedColor, 0.5f, -1.0f);

    m_font.End();
    
    
    P1.draw();
    P2.draw();
    // Draw all text + form
    Render();

    // Draw is finished
    End();
}

    
void MyConsole::Run() {
    Init();
    
    // Disable auto render on format
    SetAutoRender(0);
    
    while (1) {
        RenderApp();
        Update();
        P1.update();
        P2.update();
        P1.Collision(&P2);
        P2.Collision(&P1);
        P1.Finalize();
        P2.Finalize();
        if (P1.score >= 5) {
            Alert("Player 1 Wins");
            P1.setTexture(bg);
            P2.setTexture(bg);
        } else if (P2.score >= 5) {
            Alert("Player 2 Wins");
            P1.setTexture(bg);
            P2.setTexture(bg);   
        }
        mdelay(30);
    }
}


void MyConsole::Update() {

    usb_do_poll();
       
    get_controller_data(&ctrl, 0);
    
    if (ctrl.up) {
        P1.setDirection(u);
    } else if (ctrl.down) {
        P1.setDirection(d);
    } else if (ctrl.left) {
        P1.setDirection(l);
    } else if (ctrl.right) {
        P1.setDirection(r);
    } else if (ctrl.s1_x > STICK_THRESHOLD) {
        P1.setDirection(r);
    } else if (ctrl.s1_x < -STICK_THRESHOLD) {
        P1.setDirection(l);
    } else if (ctrl.s1_y > STICK_THRESHOLD) {
        P1.setDirection(u);
    } else if (ctrl.s1_y < -STICK_THRESHOLD) {
        P1.setDirection(d);
    }
     
    if (ctrl.a && !old_ctrl.a) {
        P1.shoot();
    }  
    
    if (ctrl.x && !old_ctrl.x) {
        if (Warning("Reset Game stats?")) {
            P1.setTexture(bg);
            P2.setTexture(bg);
        }
    }
    
    
    get_controller_data(&ctrl1, 1);
    
    if (ctrl1.up) {
        P2.setDirection(u);
    } else if (ctrl1.down) {
        P2.setDirection(d);
    } else if (ctrl1.left) {
        P2.setDirection(l);
    } else if (ctrl1.right) {
        P2.setDirection(r);
    } else if (ctrl1.s1_x > STICK_THRESHOLD) {
        P2.setDirection(r);
    } else if (ctrl1.s1_x < -STICK_THRESHOLD) {
        P2.setDirection(l);
    } else if (ctrl1.s1_y > STICK_THRESHOLD) {
        P2.setDirection(u);
    } else if (ctrl1.s1_y < -STICK_THRESHOLD) {
        P2.setDirection(d);
    }
     
    if (ctrl1.a && !old_ctrl.a) {
        P2.shoot();
    }
    if (ctrl1.x && !old_ctrl1.x) {
        if (Warning("Reset Game stats?")) {
            P1.setTexture(bg);
            P1.spawn(P1.sPoint[P1.sCount].x, P1.sPoint[P1.sCount].y, P1.sPoint[P1.sCount].direction);
            P2.setTexture(bg);
            P2.sCount = rand()%4;
            while (P1.sCount == P2.sCount) {
                P2.sCount = rand()%4;
            }
            P2.spawn(P2.sPoint[P2.sCount].x, P2.sPoint[P2.sCount].y, P2.sPoint[P2.sCount].direction);
        }
    }
    
    old_ctrl = ctrl;
    old_ctrl1 = ctrl1;
    memset(&ctrl, 0, sizeof(struct controller_data_s));
    memset(&ctrl1, 0, sizeof(struct controller_data_s));
}

void MyConsole::Init() {

    char *pngPath = (char*)malloc(256);
    char *ressourcesPath = (char*)malloc(256);
    sprintf(ressourcesPath,"%s/ressources/",rootpath);
    sprintf(pngPath, "%stank.png", ressourcesPath);
    
    Create(ressourcesPath, 0xFF0000FF, 0xFFFFFF00);
    LoadTextureFromFile(g_pVideoDevice, pngPath, &bg);
    srand(time());  
    P1.sCount = rand()%4;
    
    P1.sPoint[0].x = 100;
    P1.sPoint[0].y = 100;
    P1.sPoint[0].direction = r;
    P1.sPoint[1].x = 1080;
    P1.sPoint[1].y = 600;
    P1.sPoint[1].direction = l;
    P1.sPoint[2].x = 1080;
    P1.sPoint[2].y = 100;
    P1.sPoint[2].direction = l;
    P1.sPoint[3].x = 100;
    P1.sPoint[3].y = 600;
    P1.sPoint[3].direction = r;
    
    P2.sPoint[0].x = 100;
    P2.sPoint[0].y = 100;
    P2.sPoint[0].direction = r;
    P2.sPoint[1].x = 1080;
    P2.sPoint[1].y = 600;
    P2.sPoint[1].direction = l;
    P2.sPoint[2].x = 1080;
    P2.sPoint[2].y = 100;
    P2.sPoint[2].direction = l;
    P2.sPoint[3].x = 100;
    P2.sPoint[3].y = 600;
    P2.sPoint[3].direction = r;
    
    P1.setTexture(bg);
    P1.spawn(P1.sPoint[P1.sCount].x, P1.sPoint[P1.sCount].y, P1.sPoint[P1.sCount].direction);
    P2.setTexture(bg);
    P2.sCount = rand()%4;
    while (P1.sCount == P2.sCount) {
        P2.sCount = rand()%4;
    }
    P2.spawn(P2.sPoint[P2.sCount].x, P2.sPoint[P2.sCount].y, P2.sPoint[P2.sCount].direction);

};

BOOL MyConsole::Warning(const char *message) {
    unsigned int color = 0xFF6f641a;
    old_ctrl = ctrl;
    memset(&ctrl, 0, sizeof (struct controller_data_s));
    old_ctrl1 = ctrl1;
    memset(&ctrl1, 0, sizeof (struct controller_data_s));
    while (1) {

        // Begin to draw
        Begin();

        Draw::DrawColoredRect(-0.5f, -0.5f, 1.0f, 1.0f, color);

        m_font.Begin();

        m_font.DrawTextF(message, SelectedColor, -0.55f, -0.5f);

        m_font.DrawTextF("Press X to continue", SelectedColor, -0.55f, 0.3f);
        m_font.DrawTextF("Press B to cancel", SelectedColor, 0.00f, 0.3f);

        m_font.End();

        // Draw all text + form
        Render();

        // Draw is finished
        End();
        // Update code ...
        usb_do_poll();
        get_controller_data(&ctrl, 0);

        if (ctrl.b & !old_ctrl.b) {
            return FALSE;
        }
        if (ctrl.x & !old_ctrl.x) {
            return TRUE;
        }
        get_controller_data(&ctrl1, 1);

        if (ctrl1.b & !old_ctrl1.b) {
            return FALSE;
        }
        if (ctrl1.x & !old_ctrl1.x) {
            return TRUE;
        }
        // Save the old value
        old_ctrl = ctrl;
        memset(&ctrl, 0, sizeof (struct controller_data_s));
        old_ctrl1 = ctrl1;
        memset(&ctrl1, 0, sizeof (struct controller_data_s));
    }
    return FALSE;
}

void MyConsole::Alert(const char *message) {
    unsigned int color = 0xFF1212b4;

    old_ctrl = ctrl;
    memset(&ctrl, 0, sizeof (struct controller_data_s));
    old_ctrl1 = ctrl1;
    memset(&ctrl1, 0, sizeof (struct controller_data_s));
        
    while (1) {
        // Begin to draw
        Begin();
        Draw::DrawColoredRect(-0.5f, -0.5f, 1.0f, 1.0f, color);

        m_font.Begin();

        m_font.DrawTextF(message, SelectedColor, -0.55f, -0.5f);
        m_font.DrawTextF("Press B to continue", SelectedColor, -0.3f, 0.3f);

        m_font.End();

        // Draw all text + form
        Render();

        // Draw is finished
        End();
        // Update code ...
        usb_do_poll();
        get_controller_data(&ctrl, 0);

        if (ctrl.b & !old_ctrl.b) {
            return;
        }
        get_controller_data(&ctrl1, 1);

        if (ctrl1.b & !old_ctrl1.b) {
            return;
        }
        // Save the old value
        old_ctrl = ctrl;
        memset(&ctrl, 0, sizeof (struct controller_data_s));
        old_ctrl1 = ctrl1;
        memset(&ctrl1, 0, sizeof (struct controller_data_s));
    }
}

MyConsole cApp;
int main(int argc, char **argv) {
#ifdef LIBXENON
    xenos_init(VIDEO_MODE_AUTO);
    Hw::SystemInit(INIT_USB|INIT_ATA|INIT_ATAPI|INIT_FILESYSTEM);
    usb_do_poll();

    xenon_smc_start_bootanim();

#endif

    if(argc != 0 && argv[0]) {
        char *tmp = argv_GetFilepath(argv[0]);
        strcpy(rootpath,tmp);
    } else {
        strcpy(rootpath,"uda0:");
    }

    cApp.Run();
    
    while (1) {
    }

    return 0;
}
