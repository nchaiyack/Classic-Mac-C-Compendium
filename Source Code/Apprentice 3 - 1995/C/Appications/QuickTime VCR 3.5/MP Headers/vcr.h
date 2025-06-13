#pragma once

#define UP 1
#define DOWN 0

#define BASE_ICON	700
#define ICON_COUNT	7

#define GO_START_ICON	700
#define RW_ICON	701
#define STOP_ICON 702
#define PLAY_ICON	703
#define FF_ICON	704
#define GO_END_ICON	705
#define LAST_ICON 706
#define VOLUME_UP_CICN	800
#define VOLUME_DOWN_CICN	801
#define CLOSE_CICN	802

#define GO_START_BUTTON	1
#define RW_BUTTON	2
#define STOP_BUTTON 3
#define PLAY_BUTTON	4
#define FF_BUTTON	5
#define GO_END_BUTTON	6
#define LAST_BUTTON 7

#define STATUS_BOX	7
#define CLOCK_BOX	8
#define TAPE_SLOT	9

#define VOLUME_DOWN	10
#define VOLUME_UP	11
#define CLOSE_VCR_BOX	12
#define VCR_BOX		13

#define PROGRESS_1	14
#define PROGRESS_2	15
#define PROGRESS_3	16
#define PROGRESS_4	17


enum {stopped = 0,playing,fastforward,re_wind,gotoend,gotostart};

typedef struct buttons {
	Boolean button[LAST_BUTTON];
	int buttonRNum[LAST_BUTTON];
} Buttons;

class VCR : public Dlog {
	public:
		Buttons myButtons;
		short vcr_mode;
		
		VCR(void);
		~VCR(void);
		
		virtual void loadResource( int number);	// loads resource and sets number
		virtual int HandleDialogItem(int itemHit, Point thePoint, int thePart);
		virtual void RedrawDialog(void);
		virtual void RedrawButtons(void);
		virtual void ButtonsAndValues(int itemHit);
		virtual void RedrawStatus(void);
		virtual void RedrawTime(void);
		virtual void UpdateProgress(void);
		virtual void DrawAllProgress(int color);
		virtual void ClearAllButtons(void);
		virtual void FlashButton( int id);
	
	private:
		virtual void Draw2Rects2Colors(Rect r1, int color1, Rect r2, int color2);
		

};