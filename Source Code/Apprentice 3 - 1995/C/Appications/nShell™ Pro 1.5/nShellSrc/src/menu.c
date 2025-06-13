/* ==========================================

	menu.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "nsh.h"
#include "menu.h"
#include "multi.h"
#include "resource.h"

#include "sfget.proto.h"
#include "file.proto.h"
#include "find.proto.h"
#include "menu.proto.h"
#include "multi.proto.h"
#include "nsh.proto.h"
#include "utility.proto.h"
#include "wind.proto.h"

extern Boolean	gHasNewStdFile;		// true if the new SF calls are there

MenuHandle		myMenus[MAX_MENU];

int				had_one_a_minute_ago;

void menu_init(void)
{
	int		i;

#if FULL_APP
	#if PROFESSIONAL
	myMenus[appleM] = GetMenu(appleIDPro);
	#else
	myMenus[appleM] = GetMenu(appleID);
	#endif
#else
	myMenus[appleM] = GetMenu(appleID);
#endif

	AddResMenu(myMenus[appleM], 'DRVR');

	myMenus[fileM] = GetMenu(fileID);
	myMenus[editM] = GetMenu(editID);
	myMenus[searchM] = GetMenu(searchID);
	
	for (i = 0; i < MAX_MENU; i++ )
		InsertMenu(myMenus[i],0);
	
	DrawMenuBar();
	
	had_one_a_minute_ago = 1;	// a flag to keep from DrawMenuBar all the time
}

void menu_command(long mResult)
{
	int		theItem;
	Str255	name;
	GrafPtr	SavPort;
	
	theItem = LoWord(mResult);
	switch (HiWord(mResult)) {
		case appleID:
			if (theItem == amAbout) {
				#if PROFESSIONAL
				SimpleModal(AboutProDialog);
				#else
				SimpleModal(AboutDialog);
				#endif
				}
			else {
				GetItem(myMenus[appleM], theItem, &name);
				GetPort(&SavPort);
				OpenDeskAcc(name);
				SetPort(SavPort);
				}
			break;

		case fileID: 
			menu_file(theItem);
			break;

		case editID: 
			if (SystemEdit(theItem-1) == 0)
				menu_edit(theItem);
			break;

		case searchID: 
			menu_search(theItem);
			break;

	}
	HiliteMenu(0);
}

void menu_file (int item)
{
#if FULL_APP
	ShellH	shell;
	int 	vRef, refNum, io;
	Str255	fn;

	switch (item) {
		case fmNew: 
			mf_new();
			break;

		case fmOpen:
			mf_open();
			break;

		case fmClose:
			multi_close(multi_front());
			break;
		case fmSave:
			if ( shell = multi_front() )
				if ( multi_stopped(shell) )
					mf_save(shell);
			break;
		case fmSaveAs:
			if ( shell = multi_front() )
				if ( multi_stopped(shell) )
					mf_save_as(shell);
			break;
		case fmRevert:
			if ( shell = multi_front() )
				if ( multi_stopped(shell) )
					mf_revert(shell);
			break;
		case fmQuit: 
			nsh_quit();
			break;
	}
#else
	if (item == fmQuit)
		nsh_quit();
#endif
}

void menu_edit(int theItem)
{
	ShellH	shell;
	Str255	insert_path;

	shell = multi_front();
	
	if ( shell ) {
		switch (theItem) {
			case emCut:
				if (multi_can_insert(shell)) {
					TECut((**shell).Text);
					ZeroScrap();
					TEToScrap();
					(**shell).Changed = 1;
					}
				break;
	
			case emCopy:
				TECopy((**shell).Text);
				ZeroScrap();
				TEToScrap();
				break;
		
			case emPaste:
				multi_discard_text( shell, TEGetScrapLen() );
				multi_force_insert( shell );
				TEFromScrap();
				TEPaste( (**shell).Text );
				(**shell).Changed = 1;
				break;
		
			case emClear:
				if (multi_can_insert(shell)) {
					TEDelete((**shell).Text);
					(**shell).Changed = 1;
					}
				break;
				
#if FULL_APP
			case emInsert:
				if (sfget(insert_path)) {
					multi_discard_text( shell, TEGetScrapLen() );
					multi_force_insert( shell );
					TEInsert( &insert_path[1], insert_path[0], (**shell).Text );
					(**shell).Changed = 1;
					}
				break;
#endif	
			}
				
		wind_show_sel(shell);
		
		}
}

void menu_search (int item)
{
	switch (item) {
		case smFind:
			find(); 
			break;
		case smFindNext:
			find_next(1);
			break;
		case smFindPrev:
			find_next(-1); 
			break;
		case smEnterSel:
			find_set();
			break;
		}
}

void menu_hilite(void)
{
	ShellH	shell;

	shell = multi_front();
	
	if ( shell ) {
		menu_hilite_shell(shell);
		had_one_a_minute_ago = 1;
		}
	else {
		if (had_one_a_minute_ago)
			menu_hilite_noshell();
		had_one_a_minute_ago = 0;
		}
}

void menu_hilite_shell(ShellH shell)
{	
	int	insert_f,select_f;
	
	if (find_ready()) {
		EnableItem(myMenus[searchM], smFindNext);
		EnableItem(myMenus[searchM], smFindPrev);
		}
	else {
		DisableItem(myMenus[searchM], smFindNext);
		DisableItem(myMenus[searchM], smFindPrev);
		}

#if FULL_APP

	if ((**shell).Changed) {
		EnableItem(myMenus[fileM], fmSave);
		if ((**shell).shell_fss.vRefNum)
			EnableItem(myMenus[fileM], fmRevert);
		else
			DisableItem(myMenus[fileM], fmRevert);
		}
	else {
		DisableItem(myMenus[fileM], fmRevert);
		DisableItem(myMenus[fileM], fmSave);
		}
		
#endif

	EnableItem(myMenus[editM], emPaste);
	
	select_f = (**(**shell).Text).selStart!=(**(**shell).Text).selEnd;
	
	if (select_f) {
		EnableItem(myMenus[editM], emCopy);
		EnableItem(myMenus[searchM], smEnterSel);
		}
	else {
		DisableItem(myMenus[editM], emCopy);
		DisableItem(myMenus[searchM], smEnterSel);
		}
		
	insert_f = multi_can_insert(shell);
		
	if (insert_f && select_f) {
		EnableItem(myMenus[editM], emCut);
		EnableItem(myMenus[editM], emClear);
		}
	else {
		DisableItem(myMenus[editM], emCut);
		DisableItem(myMenus[editM], emClear);
		}
		
	if (!had_one_a_minute_ago) {
#if FULL_APP
		EnableItem(myMenus[fileM], fmClose);
		EnableItem(myMenus[fileM], fmSaveAs);
		if ( gHasNewStdFile )
			EnableItem(myMenus[editM], emInsert);
#endif
		EnableItem(myMenus[searchM], smFind);
		EnableItem(myMenus[editM], 0);
		EnableItem(myMenus[searchM], 0);
		DrawMenuBar();
		}
}

void menu_hilite_noshell(void)
{
#if FULL_APP
	DisableItem(myMenus[fileM], fmClose);
	DisableItem(myMenus[fileM], fmSave);
	DisableItem(myMenus[fileM], fmSaveAs);
	DisableItem(myMenus[fileM], fmRevert);
	DisableItem(myMenus[editM], emInsert);
#endif
	DisableItem(myMenus[editM], 0);
	DisableItem(myMenus[editM], emCut);
	DisableItem(myMenus[editM], emCopy);
	DisableItem(myMenus[editM], emPaste);
	DisableItem(myMenus[editM], emClear);
	DisableItem(myMenus[searchM], 0);
	DisableItem(myMenus[searchM], smFind);
	DisableItem(myMenus[searchM], smFindNext);
	DisableItem(myMenus[searchM], smFindPrev);
	DisableItem(myMenus[searchM], smEnterSel);
	DrawMenuBar();
}


