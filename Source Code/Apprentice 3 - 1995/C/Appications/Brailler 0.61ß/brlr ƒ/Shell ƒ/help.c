#include "help.h"
#include "environment.h"
#include "popup.h"
#include "util.h"
#include "buttons.h"
#include "text twiddling.h"
#include "program globals.h"
#include "window layer.h"
#include "graphics.h"

#define DEAD_SPACE_TOP		10
#define DEAD_SPACE_LEFT		10
#define DEAD_SPACE_BOTTOM	5
#define DEAD_SPACE_RIGHT	10
#define	TEXT_RECT_WIDTH		405
#define	TEXT_RECT_HEIGHT	250
#define	BUTTON_WIDTH		80
#define	BUTTON_HEIGHT		17
#define BUTTON_GAP_H		15
#define BUTTON_GAP_V		5

#define MAX_MAIN_TOPICS		5
#define	MAX_SUB_TOPICS		6

#define MAIN_TOPIC_ID		600
#define POPUP_MENU_ID		100

short			gMainTopicShowing;		/* saved in prefs file */
short			gSubTopicShowing;		/* saved in prefs file */

static	short			gNumMainTopics;
static	short			gNumSubTopics[MAX_MAIN_TOPICS];
static	Str31			gMainTopicTitle[MAX_MAIN_TOPICS];
static	Rect			gMainTopicRect[MAX_MAIN_TOPICS];
static	Str31			gSubTopicTitle[MAX_MAIN_TOPICS][MAX_SUB_TOPICS];
static	short			gSubTopicID[MAX_MAIN_TOPICS][MAX_SUB_TOPICS];
static	Rect			gTextRect;
static	Handle			gTheText;
static	Handle			gTheStyle;

static	Boolean			gSetupDone=FALSE;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for help.c                                                         */

static	short ParseRawTitle(Str255 theTitle);
static	void GoToPage(WindowPtr theWindow, short mainTopic, short subTopic,
				Boolean updateNow);
static	void GetTextResources(short mainTopic, short subTopic);
static	void DisposeTextResources(void);

void SetupTheHelpWindow(WindowPtr theWindow)
{
	short			i,j;
	unsigned char	*titleStr="\pHelp";
	Handle			temp;
	short			strID;
	Point			thePoint;
	
	SetWindowMaxDepth(theWindow, 8);
	SetWindowDepth(theWindow, 8);
	SetWindowWidth(theWindow, DEAD_SPACE_LEFT+TEXT_RECT_WIDTH+DEAD_SPACE_RIGHT);
	SetWindowHeight(theWindow, BUTTON_GAP_V+DEAD_SPACE_TOP+BUTTON_HEIGHT+TEXT_RECT_HEIGHT+
		DEAD_SPACE_BOTTOM);
	SetWindowAttributes(theWindow, kHasCloseBoxMask+kHasDocumentTitlebarMask);
	thePoint.v=50;
	thePoint.h=6;
	SetWindowTopLeft(theWindow, thePoint);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowTitle(theWindow, titleStr);
	SetWindowAutoCenter(theWindow, FALSE);
	
	if (gSetupDone)
		return;
	
	temp=GetResource('STR#', MAIN_TOPIC_ID);
	gNumMainTopics=**((short**)temp);
	ReleaseResource(temp);
	for (i=0; i<gNumMainTopics; i++)
	{
		GetIndString(gMainTopicTitle[i], MAIN_TOPIC_ID, i+1);
		strID=ParseRawTitle(gMainTopicTitle[i]);
		
		for (j=0; j<5; j++)
			gMainTopicTitle[i][++gMainTopicTitle[i][0]]=' ';
		
		SetRect(	&gMainTopicRect[i],
					DEAD_SPACE_LEFT+(BUTTON_WIDTH+BUTTON_GAP_H)*i,
					DEAD_SPACE_TOP,
					DEAD_SPACE_LEFT+(BUTTON_WIDTH+BUTTON_GAP_H)*i+BUTTON_WIDTH,
					DEAD_SPACE_TOP+BUTTON_HEIGHT);
		
		temp=GetResource('STR#', strID);
		gNumSubTopics[i]=**((short**)temp);
		ReleaseResource(temp);
		
		for (j=0; j<gNumSubTopics[i]; j++)
		{
			GetIndString(gSubTopicTitle[i][j], strID, j+1);
			gSubTopicID[i][j]=ParseRawTitle(gSubTopicTitle[i][j]);
		}
	}
	
	gTheText=0L;
	gTheStyle=0L;
	
	SetRect(&gTextRect, DEAD_SPACE_LEFT+4, DEAD_SPACE_TOP+BUTTON_HEIGHT+BUTTON_GAP_V+4,
		DEAD_SPACE_LEFT+TEXT_RECT_WIDTH-4,
		DEAD_SPACE_TOP+BUTTON_HEIGHT+BUTTON_GAP_V+TEXT_RECT_HEIGHT-4);
	
	gSetupDone=TRUE;
	gNeedToOpenWindow=FALSE;
}

void OpenTheHelpWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	Rect			destRect, viewRect;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
	{
		destRect=gTextRect;
		viewRect=destRect;
		hTE=TEStyleNew(&destRect, &viewRect);
		SetWindowTE(theWindow, hTE);
		TESetSelect(0, 0, hTE);
		TEAutoView(TRUE, hTE);
		TESelView(hTE);
		GoToPage(theWindow, gMainTopicShowing, gSubTopicShowing, FALSE);
	}
}

void ShutDownTheHelpWindow(void)
{
	DisposeTextResources();
}

void DisposeTheHelpWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
}

void KeyPressedInHelpWindow(WindowPtr theWindow, unsigned char keyPressed)
{
	ObscureCursor();
	
	switch (keyPressed)
	{
		case 0x1d:										/* right arrow */
			gSubTopicShowing++;
			if (gSubTopicShowing>=gNumSubTopics[gMainTopicShowing])
			{
				gSubTopicShowing=0;
				gMainTopicShowing++;
				if (gMainTopicShowing>=gNumMainTopics)
					gMainTopicShowing=0;
			}
			GoToPage(theWindow, gMainTopicShowing, gSubTopicShowing, TRUE);
			break;
		case 0x1c:										/* left arrow */
			gSubTopicShowing--;
			if (gSubTopicShowing<0)
			{
				gMainTopicShowing--;
				if (gMainTopicShowing<0)
					gMainTopicShowing=gNumMainTopics-1;
				gSubTopicShowing=gNumSubTopics[gMainTopicShowing]-1;
			}
			GoToPage(theWindow, gMainTopicShowing, gSubTopicShowing, TRUE);
			break;
	}
}

void MouseClickedInHelpWindow(WindowPtr theWindow, Point mouseLoc)
{
	short			i;
	short			newMain, newSub;
	MenuHandle		theMenu;
	Rect			menuRect;
	
	newMain=-1;
	
	for (i=0; i<gNumMainTopics; i++)
	{
		if (PtInRect(mouseLoc, &gMainTopicRect[i]))
		{
			newMain=i;
			i=gNumMainTopics;
		}
	}
	
	if (newMain!=-1)
	{
		Draw3DButton(&gMainTopicRect[newMain], gMainTopicTitle[newMain], 0L,
			GetWindowDepth(theWindow), TRUE, FALSE, TRUE);
		
		theMenu=NewMenu(POPUP_MENU_ID, "\p");
		for (i=0; i<gNumSubTopics[newMain]; i++)
		{
			AppendMenu(theMenu, gSubTopicTitle[newMain][i]);
			CheckItem(theMenu, i+1, ((newMain==gMainTopicShowing) && (i==gSubTopicShowing)));
		}
		
		menuRect.top=gMainTopicRect[newMain].bottom-1;
		menuRect.left=gMainTopicRect[newMain].left+1;
		newSub=-1;
		if (MouseInModelessPopUp(theMenu, &newSub, &menuRect, POPUP_MENU_ID))
		{
			GoToPage(theWindow, newMain, newSub-1, TRUE);
		}
		else
		{
			Draw3DButton(&gMainTopicRect[newMain], gMainTopicTitle[newMain], 0L,
				GetWindowDepth(theWindow), FALSE, FALSE, TRUE);
		}
		
		DisposeHandle((Handle)theMenu);
	}
}

void DrawTheHelpWindow(WindowPtr theWindow, short theDepth)
{
	GrafPtr			curPort;
	short			i;
	Rect			tempRect;
	
	curPort=(GrafPtr)theWindow;
	EraseRect(&(curPort->portRect));
	
	tempRect=gTextRect;
	InsetRect(&tempRect, -4, -4);
	tempRect.right+=4;
	DrawTheShadowBox(tempRect, TRUE);
	
	for (i=0; i<gNumMainTopics; i++)
	{
		Draw3DButton(&gMainTopicRect[i], gMainTopicTitle[i], 0L, theDepth, FALSE, FALSE, TRUE);
	}
	
}

/* ---------------------------------------- */
/* the rest of these are internal to help.c */

static	short ParseRawTitle(Str255 theTitle)
{
	Str255			numStr;
	long			result;
	
	numStr[0]=0x00;
	while ((numStr[numStr[0]]=theTitle[++numStr[0]])!=' ') {}
	theTitle[0]-=numStr[0];
	Mymemcpy((Ptr)&theTitle[1], (Ptr)&theTitle[numStr[0]+1], theTitle[0]);
	numStr[0]--;
	StringToNum(numStr, &result);
	
	return result;
}

static	void GoToPage(WindowPtr theWindow, short mainTopic, short subTopic,
						Boolean updateNow)
{
	StScrpHandle	styles;
	TEHandle		hTE;
	SignedByte		savedState;
	long			textLength;
	
	DisposeTextResources();
	GetTextResources(mainTopic, subTopic);
	styles=(StScrpHandle)gTheStyle;
	if (styles!=0L)
		savedState=HGetState((Handle)styles);
	hTE=GetWindowTE(theWindow);
	if (gTheText!=0L)
	{
		(**hTE).viewRect.bottom=(**hTE).viewRect.top;
		HLockHi(gTheText);
		textLength=GetHandleSize(gTheText);
		SetTheText(theWindow, *gTheText, textLength);
		HUnlock(gTheText);
		if (styles!=0L)
		{
			TEUseStyleScrap(0, GetHandleSize(gTheText), styles, TRUE, hTE);
			HSetState((Handle)styles, savedState);
		}
		(**hTE).destRect=(**hTE).viewRect=gTextRect;
		TECalText(hTE);
		InvalRect(&(theWindow->portRect));
	}
	gMainTopicShowing=mainTopic;
	gSubTopicShowing=subTopic;
	if (updateNow)
	{
		InvalRect(&(theWindow->portRect));
	}
}

static	void GetTextResources(short mainTopic, short subTopic)
{
	short			resID;
	
	DisposeTextResources();
	resID=gSubTopicID[mainTopic][subTopic];
	gTheText=GetResource('TEXT', resID);
	if (gTheText!=0L)
		DetachResource(gTheText);
	gTheStyle=GetResource('styl', resID);
	if (gTheStyle!=0L)
		DetachResource(gTheStyle);
}

static	void DisposeTextResources(void)
{
	if (gTheText!=0L)
		DisposeHandle(gTheText);
	if (gTheStyle!=0L)
		DisposeHandle(gTheStyle);
	gTheText=0L;
	gTheStyle=0L;
}
