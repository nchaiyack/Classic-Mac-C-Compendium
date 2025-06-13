#include <Gestalt.h>
#include <pascal.h>
#include <Menus.h>
#include <Files.h>
#include <Folders.h>
#include <Fonts.h>
#include <Dialogs.h>
#include <Memory.h>
#include <SegLoad.h>
#include <BDC.h>
#include <OSUtils.h>
#include "ctc.h"

/* Tells the Dialog Manager that there is an edit line in this dialog, and */ 
/* it should track and change to an I-Beam cursor when over the edit line */
pascal OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks)
	= { 0x303C, 0x0306, 0xAA68 };


void InitMacintosh()
{
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitCursor();
	InitDialogs(nil);
}	/* End of () */