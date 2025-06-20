/*****
 * ShellInterfaceExec.c
 *
 *	Interface routines for the adaexec program
 *
 *****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */



#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "config.h"

#include "ExecMain.h"
#include "ExecShellGlobals.h"
#include "ExecShellInterface.h"

#include "DrawTextColumn.h"
#include "MacAdalib.h"
#include "AdaFileTypes.h"
#include "TermEmulationIntf.h"
#include "FileMgr.h"
#include "VersStr.h"

enum	{
	kRes_Menu_Apple = 128,
	kRes_Menu_File,
	kRes_Menu_Edit
	};

enum	{
	pauseItem = 1,
	resumeItem = 2,
	quitItem = 4,
	
	aboutItem = 1
	};

static void enable (MenuHandle menu, short item, short ok);


/****
 * SetUpMenus()
 *
 *	Set up the menus. Normally, we�d use a resource file, but
 *	for this example we�ll supply �hardwired� strings.
 *
 ****/
// no, no! We use resources! :-) [Fabrizio Oddone]

void SetUpMenus(void)

{
	{
	register Handle menuBar;
	
	SetMenuBar(menuBar = GetNewMBar(kRes_Menu_Apple));
	DisposeHandle(menuBar);
	}

AppendResMenu(gAppleMenu = GetMenuHandle(kRes_Menu_Apple),'DRVR');
gFileMenu = GetMenuHandle(kRes_Menu_File);
gEditMenu = GetMenuHandle(kRes_Menu_Edit);
DrawMenuBar();
/*
InsertMenu(gAppleMenu = NewMenu(appleID, "\p\024"), 0);
InsertMenu(gFileMenu = NewMenu(fileID, "\pFile"), 0);
AppendMenu(gAppleMenu, "\pAbout adaexec");
AddResMenu(gAppleMenu, 'DRVR');
AppendMenu(gFileMenu, "\pPause/P;Resume/R;(-;Quit/Q");
DrawMenuBar();
*/
}
/* end SetUpMenus */


/****
 *  AdjustMenus()
 ****/

void AdjustMenus(void)
{
	enable(gFileMenu, quitItem, 1);
	
	switch (ApplicationState()) {
		case applPaused:
			enable(gFileMenu, resumeItem, 1);
			enable(gFileMenu, pauseItem, 0);
			break;
		case applRunning:
			enable(gFileMenu, resumeItem, 0);
			enable(gFileMenu, pauseItem, 1);
			break;
		case applFinished:
			enable(gFileMenu, resumeItem, 0);
			enable(gFileMenu, pauseItem, 0);
			break;
	}
}


static void enable(MenuHandle menu, short item, short ok)
{
	if (ok)
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
}


/*****
 * HandleMenu(mSelect)
 *
 *	Handle the menu selection. mSelect is what MenuSelect() and
 *	MenuKey() return: the high word is the menu ID, the low word
 *	is the menu item
 *
 *****/

void HandleMenu (long mSelect)

{
short		menuID = HiWord(mSelect);
short		menuItem = LoWord(mSelect);
Str255		name;
GrafPtr		savePort;

switch (menuID) {

	case kRes_Menu_Apple:
		switch (menuItem) {
			case aboutItem:
				Alert(20100, NULL);
				break;

			default:
				GetPort(&savePort);
				GetMenuItemText(gAppleMenu, menuItem, name);
				OpenDeskAcc(name);
				SetPort(savePort);
				break;
		}
		break;

	case kRes_Menu_File:
		switch (menuItem) {
			case pauseItem:
				PauseApplication();
				break;
			case resumeItem:
				ResumeApplication();
				break;
			case quitItem:
				QuitApplication();
				break;
		}
		break;
	case	kRes_Menu_Edit:
		(void) SystemEdit(menuItem - 1);
		break;
	}
HiliteMenu(0);	// this is the right place for it [Fabrizio Oddone]
}
/* end HandleMenu */


/****
 * SetUpWindow()
 *
 *	Create the window, and open it.
 *
 ****/


void SetUpWindow(void)
{
	Str255 file;
	FILE *fp;

	gShellWindow = scrCreateWindow(monaco, 9/*, 25, 80*/);

	FileType('TEXT');
	GetConsoleOutputFile(file);
	fp = fopen(PtoCstr(file), "w");
	if (fp) {
		char *s;
		time_t bintim;
		char *day, *month, *date, *clock, *year;
		Str255 version, unit;

		time(&bintim);
		s = ctime(&bintim);
		day = s;
		s += 3;
		*s++ = '\0';
		month = s;
		s += 3;
		*s++ = '\0';
		date = s;
		s += 2;
		*s++ = '\0';
		clock = s;
		s += 8;
		*s++ = '\0';
		year = s;
		s += 4;
		*s = '\0';

		GetUnitName(unit);
		GetVersStr(version);
		fprintf(fp, "GWU/NYU Adaexec %#s %s  %s  %s %s %s  %s\n",
			version, OP_SYS, day, date, month, year, clock);
		fprintf(fp, "Console Output from \"%#s\"\n\n", unit);
	}


	scrCopyToFile(fp);
}

/****
 *	FinishWindow()
 *
 ****/

void FinishWindow(void)
{
scrCopyToFile(NULL);
}


/*****
 * DrawWContents()
 *****/
 
void DrawWContents(short /*active*/)
{
scrDrawScreen();
}


/* InternalError() - display alert message, write error results
 * to results file, and call the longjmp.  This routine never
 * returns!
 */

void InternalError(ErrMsgs msgs, int errCode, char *file, int line)
{
	Str255 filename;
	FILE *fp;
	Str255 str[4];
	short i;

	// Setup parameters
	for (i = 0; i < 4; i++) {
		if (msgs[i].id > 0)
			GetIndString(str[i], 128, msgs[i].id);

		else if (msgs[i].msg == NULL)
			str[i][0] = 0;			// NULL Pascal string

		else {
			short len = strlen(msgs[i].msg);
			BlockMoveData(msgs[i].msg, &str[i][1], len);
			str[i][0] = len;
		}			
	}

	// Show error message.
	ParamText(str[0], str[1], str[2], str[3]);
	InitCursor();
	StopAlert(128, NULL);


	// Now handle writting the errors to the return file
	GetReturnFile(filename);
	PtoCstr(filename);

	fp = fopen((char *)&filename, "w");
	if (fp) {
		fprintf(fp, "Chaos: in %s, line %d\n", file, line);
		fprintf(fp, "Error: %ld\n", (long)errCode);
		fclose(fp);
	}

	// jump back to the main program passing whatever return
	// code we have here!
	longjmp(gJumpEnv, errCode+10);		// add 10, see note in ShellMain.c
}

