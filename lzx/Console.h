#pragma once

#include "zlx.h"
#include "Utils.h"
#include "Font.h"

namespace ZLX {

    class Console {
    public:
        Console(void);
        ~Console(void);

        // Initilisation
        int Create(const char * pFontPath, ZLXColor backColor, ZLXColor textColor);
        void Destroy();

        // Clear the screen
        void Clear();

        // Safe Area
        void SetSafeArea(unsigned int left, unsigned int top);

        // Output
        void Format(const char * format, ...);

		void Begin();
		void End();

		// Set auto Render ( do begin+render+end in format)
		void SetAutoRender(int s){
			autoRender = s;
		};

        // Render
        void Render();

        // Scroll
        void ScrollUp(int nline);

    protected:
        // Color 
        ZLXColor m_backColor;
        ZLXColor m_textColor;

		
		int autoRender;

        // Line
        unsigned int m_lineOffset;
        unsigned int m_currentLine;
        unsigned int n_lines; // Number of line
        float m_lineHeight;

        // Column
        unsigned int m_colOffset;
        unsigned int m_currentCol;
        unsigned int n_col; // Number of columns

        void newLine();

        // Charactere buffer
        char ** m_pLines;
        char * m_pBuffer;

        // Scroll
        unsigned int m_scrollOffset;

        // Add a character to the current line
        void Add(char c);

        // Safe area
        float m_safeAreaWidth;
        float m_safeAreaHeight;
        float m_safeAreaTopMargin;
        float m_safeAreaLeftMargin;

        // Video Things ...
        ZLXVideoDevice * m_pVideoDevice;

        // Font
        Font m_font;
    };

}