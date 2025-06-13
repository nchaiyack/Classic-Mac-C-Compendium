/* menus.h */

#define k_aboutItem		"\pAbout DilView..."

enum{
	mFile = 129,	
		iOpen = 1,
 		iOpenMultiple = 2,
 		iPageSetup = 4,
		iPrintOne = 5,
		iPrintMarked = 6,
		iPrintAll = 7,
		iQuit = 9,
	mEdit = 130,
		iUndo = 1,
		iCut = 3,
		iCopy = 4,
		iPaste = 5,
		iClear = 6,
	mScale = 131,
		i50scale = 1,
		i75scale = 2,
		i100scale = 3,
	mSelect = 132,
		iPrevious = 1,
		iNext = 2
};

void myInitMenus( void );	
void myDisposeMenus( void );
pascal void myFileMenu( short item );
pascal void myScaleMenu( short item );
pascal void mySelectMenu( short item );