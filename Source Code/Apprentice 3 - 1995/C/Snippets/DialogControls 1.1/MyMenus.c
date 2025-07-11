#include "MyMenus.h"
#include "Functions.h"

// I include this because of an archaic way I used to do things, and it needs
// to have a file named this with appropriate functions in it.  I've got too
// many programs dependent on this scheme that I don't want to change it yet.
// This also allows for having all menu related functions in one place.

#define		mApple			128
#define		mFile			129
#define		mEdit			130

#define		miAbout			1

#define		miOpen			1
#define		miClose			2
#define		miQuit			7


void	TwiddleMenus( void )
{
	// Here I usually check the state of many of my globals, and see which menus should be enabled.
	// There is no need for that here, so it's hard-coded.
	
	MenuEnable( mFile, 0, true );
	MenuEnable( mApple, 1, true );

	DrawMenuBar();
}

void	MovableModalMenus( void )
{
	MenuEnable( mFile, 0, false );
	MenuEnable( mApple, 1, false );
	
	DrawMenuBar();
}
