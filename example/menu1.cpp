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
#include "posix\input.h"
#endif
#include <stdio.h>
#include <algorithm>
#include <vector>
#include <string>
#include <math.h>
using namespace ZLX;

#define MAX_PATH 512
#define MAX_FILES 1000
#define MAX_DISPLAYED_ENTRIES 20

#define MAX(a,b) (((a)>(b))?(a):(b))
#define MIN(a,b) (((a)<(b))?(a):(b))
//#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define CLAMP(x,low,high) {if(x<low)x=low;if(x>high)x=high;}

#ifdef LIBXENON
#define FILE_TYPE		8
#define DIR_TYPE        4
#endif

void append_dir_to_path(char * path,char * dir){
  if (!strcmp(dir,"..")){
    int i=strlen(path);
    int delimcount=0;

    while(i>=0 && delimcount<2){
      if (path[i]=='/'){
        ++delimcount;

        if(delimcount>1){
          path[i+1]='\0';
        }
      }
      --i;
    }
  }else if (!strcmp(dir,".")){
    return;
  }else{
    strcat(path,dir);
    strcat(path,"/");
  }
}

void ActionBootTFTP(void * unused){
	// boot TFTP
	printf("boot tftp\r\n");
}

void ActionShutdown(void * unused){
	// shutdown
	printf("Shutdown ...\r\n");
}

void ActionRestart(void * unused){
	// restart
}

void ActionLaunchElf(const char * filename){
	printf("launch %s",filename);
}

/*
void ActionDumpNand(const char * dest){
	printf("dump nand %s",dest);
}
*/
void ActionDumpNand(void * unused);

struct ActionEntry{
	std::string name;
	void * param;
	void (*action)(void *param);
};

struct FileEntry{
	std::string name;
	int type;
};

bool vEntrySort (FileEntry i,FileEntry j) { 
	if(i.type==j.type){
		return i.name<j.name;
	}
	else
	{
		return (i.type>j.type);
	}
}

enum{
	PANEL_FILE_LIST=0,
	PANEL_ACTION,
	PANEL_PROGRESS
};

class MyConsole : public ZLX::Console {
private:
	static const unsigned int DefaultColor =  0x88FFFFFF;
	static const unsigned int SelectedColor = 0xFFFFFFFF;
	static const unsigned int FocusColor = 0xFFFFFFFF;
	static const unsigned int BlurColor =  0x88FFFFFF;


	ZLXTexture * bg;

	char currentPath[MAX_PATH];

	// Selected
	int entrySelected;
	int actionSelected;
	int panelSelected;

	std::vector<FileEntry> vEntry;
	std::vector<ActionEntry> vAction;

	float progressPct;

	void RenderProgress(){
		
		float posX=-0.5f;
		float posY=-0.15f;

		float w = 1.f;
		float h = 0.3f;

		float borderSize = 0.03f;

		// white border
		Draw::DrawColoredRect(posX,posY,w,h,0xFFFFFFFF);
		// black bg
		Draw::DrawColoredRect(
			posX+(borderSize/2),
			posY+(borderSize/2),
			w-borderSize,
			h-borderSize,
			0xFF000000
		);

		int alpha = 0xFF000000;

		int PctColor = 0xFF00FF00;

		// progress things ...
		Draw::DrawColoredRect(
			posX+(borderSize/2),
			posY+(borderSize/2),
			(w-borderSize)*progressPct,
			h-borderSize,
			PctColor
		);
	}

	void RenderFileInfoPanel(){
		if(vEntry.size()==0)
			return;

		unsigned int color = DefaultColor;
		float leftOffset = -1.f;
		float topOffset = 0.65f;

		if(panelSelected==PANEL_FILE_LIST){
			color&=FocusColor;
		}
		else{
			color&=BlurColor;
		}

		char bottomInfo[256];
		sprintf(bottomInfo,"File %d/%d",(entrySelected+1),vEntry.size());
		m_font.DrawTextF(bottomInfo, color, leftOffset, topOffset);		
		m_font.DrawTextF("Some Info here ...", color, leftOffset, topOffset+0.08f);
	}

	void RenderActionPanel(){
		int start,end;
		float topOffset = -0.65f;
		float leftOffset = 0.2f;
		float lineHeight = 0.07f;
		
		for(int i=0;i<vAction.size();i++){
			float posY = (topOffset)+(lineHeight*(i));// + topOffset;
			unsigned int color = DefaultColor;
			if(actionSelected==i)
			{
				color=SelectedColor;
			}

			if(panelSelected==PANEL_ACTION){
				color&=FocusColor;
			}
			else{
				color&=BlurColor;
			}

			m_font.DrawTextF(vAction.at(i).name.c_str(), color, leftOffset, posY);			
		}
	}


	void RenderTopPanel(){
		// m_font.DrawTextF("Everyday i'm browsin'", 0xFFFFFFFF, -1.f, -1.f);		
		m_font.Scale(0.8f);
		m_font.DrawTextF(currentPath, 0xFFFFFFFF, -1.f, -0.83f);
		m_font.Scale(1.0f);
	}


	void RenderFileListPanel(){
		int start,end;
		float topOffset = -0.75f;
		float leftOffset = -1.f;
		float lineHeight = 0.07f;
				
		int size = vEntry.size();

		if(size==0)
			return;

		start=entrySelected-MAX_DISPLAYED_ENTRIES/2;
		if(start<0)
			start=0;

		end=MIN(MAX_DISPLAYED_ENTRIES,vEntry.size()-start)+start;
			
		if(end>size)
			end=size;

		for(int k=start;k<end;k++){
				
			float posY = (topOffset)+(lineHeight*(k-start));// + topOffset;
			unsigned int color = DefaultColor;
			if(entrySelected==k)
			{
				color=SelectedColor;
			}
			
			if(panelSelected==PANEL_FILE_LIST){
				color&=FocusColor;
			}
			else{
				color&=BlurColor;
			}


			if(vEntry.at(k).type==DIR_TYPE){
				m_font.DrawTextF("[Dir]", color, -0.2f, posY);
			}
			m_font.DrawTextF(vEntry.at(k).name.c_str(), color, leftOffset, posY);			
		}

	}


	void RenderApp(){
		// Begin to draw
        Begin();

		// Draw bg
		Draw::DrawTexturedRect(-1.0f,-1.0f,2.0f,2.0f,bg);

        // Draw some text using font
        m_font.Begin();
            
		// Top part
		RenderTopPanel();

		// Render file listing
		RenderFileListPanel();

		// Render bottom part
		RenderFileInfoPanel();	

		RenderActionPanel();

		if(panelSelected==PANEL_PROGRESS)
			RenderProgress();

		// Finished to draw font things ...
		m_font.End();
						
		// Draw all text + form
        Render();

        // Draw is finished
        End();
	}

public:	

	void SetProgressValue(float pct){
		if(pct>0.0f)
		{
			panelSelected=PANEL_PROGRESS;
			progressPct=pct;

			RenderApp();
		}
		else
		{
			panelSelected=PANEL_FILE_LIST;
		}
	}

    void Run() {
        Init();

        // Disable auto render on format
        SetAutoRender(0);
#ifdef WIN32
		strcpy(currentPath,"c:/");
#else
		strcpy(currentPath,"uda:/");
#endif
		ScanDir();

        while (1) {
            RenderApp();
			Update();
        }
    }
private:

	void ScanDir(){
		entrySelected = 0;

		DIR * rep = opendir(currentPath);

		vEntry.clear();
    
		if (rep != NULL)
		{
			struct dirent * ent;
        
			while ((ent = readdir(rep)) != NULL)
			{
				// never display hidden files
				if(ent->d_name[0]=='.')
					continue;

				FileEntry currentEntry;
				currentEntry.name = ent->d_name;
				currentEntry.type = (ent->d_type == DIR_TYPE)?1:0;
				vEntry.push_back(currentEntry);
			}
        
			closedir(rep);
		}
		// Sort array
		sort(vEntry.begin(),vEntry.end(),vEntrySort);
     
		return;
	}


	void Update(){
		#ifdef WIN32
			W32Update();
		#endif
		controller_data_s ctrl;

		if(get_controller_data(&ctrl,0)){
			if(ctrl.up){
				switch(panelSelected){
					case 0:
						entrySelected--;
						break;
					case 1:
						actionSelected--;
						break;
				}
			}
			
			if(ctrl.down){
				switch(panelSelected){
					case 0:
						entrySelected++;
						break;
					case 1:
						actionSelected++;
						break;
				}
			}
			if(ctrl.start){
				switch(panelSelected){
				case 0:{
							if(vEntry.size()==0)
								break;

							// entrySelected++;
							FileEntry currentEntry = vEntry.at(entrySelected);
							append_dir_to_path(currentPath,(char*)currentEntry.name.c_str());
							if(currentEntry.type==1)
								ScanDir();
							else
								ActionLaunchElf(currentPath);
							break;
					   }
				case 1:{
							if(vAction.size()==0)
								break;

							// Exec action ...
							ActionEntry currentAction = vAction.at(actionSelected);
							if(currentAction.action!=NULL){
								currentAction.action(currentAction.param);
							}
							break;
					   }
				}
			}

			if(ctrl.left){
				//if(panelSelected!=PANEL_PROGRESS)
					panelSelected--;
			}
			if(ctrl.right){
				//if(panelSelected!=PANEL_PROGRESS)
					panelSelected++;
			}

			// clamp ...
			CLAMP(entrySelected,0,vEntry.size()-1);
			CLAMP(actionSelected,0,vAction.size()-1);
			CLAMP(panelSelected,0,1);
		}

	
	}

	void InitActionEntry(){
		ActionEntry action;
		{
			// TFTP
			action.name = "Boot TFTP";
			action.action = ActionBootTFTP;
			action.param = NULL;
			vAction.push_back(action);
		}
		{
			// Shutdown
			action.name = "Shutdown";
			action.action = ActionShutdown;
			action.param = NULL;
			vAction.push_back(action);
		}
		{
			// Restart
			action.name = "Restart";
			action.action = ActionRestart;
			action.param = NULL;
			vAction.push_back(action);
		}
		{
			// Dump nand
			action.name = "Dump nand";
			action.action = ActionDumpNand;
			action.param = NULL;
			vAction.push_back(action);
		}
	}

    void Init() {
		entrySelected = 0;
		panelSelected= 0;
		actionSelected = 0;

		progressPct = 0.0f;

		InitActionEntry();

#ifdef WIN32
        Create("..\\ressources\\", 0xFF000000, 0xFFFFFF00);
		LoadTextureFromFile(g_pVideoDevice,"..\\ressources\\bg.png",&bg);
#else
        Create("uda:/ressources", 0xFF0000FF, 0xFFFFFF00);
		LoadTextureFromFile(g_pVideoDevice,"uda:/ressources/bg.png",&bg);
#endif
		
    };
};

MyConsole cApp;

void ActionDumpNand(void * unused){
	char * dest = (char*)unused;
	float pct=0.0f;
	while(1){
		pct+=0.001f;
		cApp.SetProgressValue(pct);
		if(pct>1.0f)
			break;
	}
}

int main() {
#ifdef LIBXENON
    usb_init();
    usb_do_poll();
    xenon_ata_init();
    dvd_init();
#endif
    

    cApp.Run();

    while (1) {
    }

    return 0;
}
