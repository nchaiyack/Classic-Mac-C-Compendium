//	GW-INIT4.2.1.h

#include	<A4Stuff.h>
#include	<SetupA4.h>
#include	<QDOffscreen.h>
#include	<LowMem.h>
#include	"GW-Common4.2.1.h"

#define		userState	(**(WStateData**)((WindowPeek)wp)->dataHandle).userState
#define		stdState	(**(WStateData**)((WindowPeek)wp)->dataHandle).stdState

#define		strucRect	(**((WindowPeek)wp)->strucRgn).rgnBBox
#define		contRect	(**((WindowPeek)wp)->contRgn).rgnBBox

pascal Boolean	(*trackGoAwayProc)	(WindowPtr wp, Point mp);
pascal void		(*dragWindowProc)	(WindowPtr wp, Point mp, Rect *r);
pascal long		(*growWindowProc)	(WindowPtr wp, Point mp, Rect *r);

int	abs(int);

void			main(void);
Boolean			isSystem7(void);
Boolean			trapAvailable(short theTrap);
TrapType		getTrapType(short theTrap);
short			numToolboxTrap(void);
pascal Handle	get1Resource(OSType rsrcType, short id);
pascal CursHandle getCursor(short id);
void			setupINIT(void);
void			drawIcon(short iconID);
void			setSelectorFunc(SelectorFunctionUPP selectorFuncAddress);
pascal OSErr	gestaltSelector(OSType selector, long *response);
initDataHandle	loadSetting(void);
initDataHandle	load420Data(void);
initDataHandle	load412Data(void);
initDataHandle	load401Data(void);
initDataHandle	setDefaultData(void);
OSErr			openPrefsFile(prefsFileSpec *pfSpecP);
void			saveSetting(initDataHandle dH);
OSErr			createPrefsFile(void);
pascal Boolean	trackGoAway	(WindowPtr wp, Point mp);
pascal void		dragWindow	(WindowPtr wp, Point mp, Rect *r);
pascal long		growWindow	(WindowPtr wp, Point mp, Rect *r);
Boolean			checkKeys(Boolean *key);
pascal void		pushWindow	(WindowPtr wp, Point mp, Rect *r);
pascal void		popWindow	(WindowPtr wp, Point mp, Rect *r);
pascal Boolean	customGoAway(WindowPtr wp, Point mp);
void			popFunc		(WindowPtr wp, Rect *r);
pascal void		customDrag	(WindowPtr wp, Point mp, Rect *r);
pascal void		strongGlue	(WindowPtr wp, Point mp, Rect *r);
pascal void		directDrag	(WindowPtr wp, Point mp, Rect *r);
pascal long		customGrow	(WindowPtr wp, Point mp, Rect *r);
void			drawMarquee	(RgnHandle rh, short globalRight, short globalBottom,
																	short gapTop);
void			frameRgn	(RgnHandle rh, short wLeft, short wTop);
void 			frameRect	(Rect *rect);
Boolean			rectInRgn	(Rect *rect, RgnHandle vRgn);
void			paintRgn	(RgnHandle strucVisRgnH);
Boolean			titleInRect	(WindowPeek wPeek, short h, short v, Rect *r);
void			getGlobal	(WindowPeek wPeek, Point *pt);
