/* BBcdev class definition */

struct BBcdev : cdev {

//	instance variables
	
	short		sleepNowRect,
				neverSleepRect;
	Handle		pictHandles[8];
	Rect		smallSNRect[4],
				smallNSRect[4],
				smIdleRect,
				smRefrRect;
	Boolean		mustRemember;

//	methods + functions
		
	void	Init(void);					// "initDev"
	void	Idle(void);					// "nulDev"
	void	Close(void);				// "closeDev"
	void	Key(short);					// "keyEvtDev"
	void	ItemHit(short);				// "hitDev"
	void	RememberValues(void);
};



/* Picture resource IDs */

#define topLeftPict		129
#define topRightPict	130
#define botLeftPict		131
#define botRightPict	132
#define noArrowPict		133
#define upArrowPict		134
#define dnArrowPict		135
#define grArrowPict		136


/* Rectangle numbers */

#define topLeftRect		0
#define topRightRect	1
#define botRightRect	2
#define botLeftRect		3
#define noArrow			4
#define upArrow			5
#define dnArrow			6
#define grArrow			7


/* Dialog item numbers */

#define screenOn	1			// "On / Off" check box
#define startupIcon	2			// "Startup Icon" check box
#define clockOn		3			// "clock on" check box
#define fadeWhite	4			// "Fade To White" check box
#define zeroMenu	5			// "Zero Menubar" check box
#define sleepNow	6			// "Sleep Now" picture
#define neverSleep	7			// "Never Sleep" picture
#define idleMins	8			// "idle time" edit text item
#define refreshSecs	9			// "refresh time" edit text item
#define idleRect	10			// idle time arrows picture
#define refrRect	11			// refresh time arrows picture
#define eraseRect	12			// "EraseRect" check box
#define eraseOval	13			// "EraseOval" check box
#define eraseRgn	14			// "EraseRgn" check box
#define drawMenu	15			// "DrawMenuBar" check box
#define initCurs	16			// "InitCursor" check box
