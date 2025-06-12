/*
*********************************************************************
*	
*	Menus.c
*	Menu bar handling
*		
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright © 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "menus.h"
#include "pstr.h"
#include "util.h"
#include "window.h"


/*
********************************* Global variables *******************
*/

extern VolInfo		gCurrentCD;					/* information about current CD			*/
extern MenuHandle	gMenu[MENUNR];				/* menu handles	        					*/
extern Prefs		gPrefs;
extern DBInfo		gDBInfo[DB_NUM];

/**************************************
*	Adjust Other menu item in Creators submenu to show the name of the
*	creator application.
*	Return value:	none
*/

void UpdateOtherCreator()
{
	Str255	title;
	
	GetIndString(title,OTHERS,OTHERCREATOR);
	pstrcat(title,"\p (");
	pstrcat(title,gPrefs.creatorName);
	pstrcat(title,"\p)É");
	
	SetItem(gMenu[CREATOR],OTHERS_I,title);
}

/**************************************
*	Adjust menu bar
*	Return value:	none
*/

void MaintainMenus()
{
	register short	i,n;
	WindowPtr		topWindow=FrontWindow();
	WDPtr				wdp;
	Str255			title,str;
	
	/* Default status */
	
	EnableItem(gMenu[FILE],NEW_I);
	EnableItem(gMenu[FILE],OPEN_I);
	EnableItem(gMenu[FILE],LOAD_I);
	EnableItem(gMenu[FILE],CLOSE_I);
	DisableItem(gMenu[FILE],SAVE_I);
	DisableItem(gMenu[FILE],SAVEAS_I);
	DisableItem(gMenu[FILE],EXPORTSEL_I);
	EnableItem(gMenu[FILE],PAGESETUP_I);
	EnableItem(gMenu[FILE],PRINT_I);
	EnableItem(gMenu[FILE],QUIT_I);
	
	for(i=UNDO_I;i <= DUPLICATE_I;DisableItem(gMenu[EDIT],i++));

	EnableItem(gMenu[OSTUFF],INFO_I);
	DisableItem(gMenu[OSTUFF],XREF_I);
	
	for(i=FORMAT_I;i<=GENERAL_I;EnableItem(gMenu[PREFS],i++));
	EnableItem(gMenu[PREFS],SAVEPREFS_I);
	DisableItem(gMenu[WINDOWS],ROTATE_I);
	
	if(IsDAWindow(topWindow)) {
		DisableItem(gMenu[FILE],NEW_I);
		DisableItem(gMenu[FILE],OPEN_I);
		DisableItem(gMenu[FILE],LOAD_I);
		DisableItem(gMenu[FILE],CLOSE_I);
		DisableItem(gMenu[FILE],PAGESETUP_I);
		DisableItem(gMenu[FILE],PRINT_I);
		DisableItem(gMenu[FILE],QUIT_I);
		
		EnableItem(gMenu[EDIT],UNDO_I);
		for(i=CUT_I;i <= CLEAR_I;EnableItem(gMenu[EDIT],i++));
		for(i=FORMAT_I;i<= SAVEPREFS_I;DisableItem(gMenu[PREFS],i++));
	}
	else {
		/*
		*	Window-independent menu options
		*/

		if(*(gCurrentCD.volName) == EOS) {
			DisableItem(gMenu[OSTUFF],INFO_I);
		}
		
		if(GetFreeWindow() == -1) {
			DisableItem(gMenu[FILE],OPEN_I);
			DisableItem(gMenu[FILE],LOAD_I);
			DisableItem(gMenu[FILE],NEW_I);
		}
		
		/*
		*	Window-dependent menu options
		*/
		if( topWindow == NULL ) {		/* no windows open */
			EnableItem(gMenu[FILE],		OPEN_I);
			DisableItem(gMenu[FILE],	CLOSE_I);
			DisableItem(gMenu[FILE],	PRINT_I);
		}
		else {
			EnableItem(gMenu[WINDOWS], ROTATE_I);					
			n=CountMItems(gMenu[WINDOWS]);
			GetWTitle(topWindow,title);
			for(i=FIRSTWIN_I;i <= n;++i) {
				CheckItem(gMenu[WINDOWS],i,FALSE);
				GetItem(gMenu[WINDOWS],i,str);
				if(!pstrcmp(str,title))
					CheckItem(gMenu[WINDOWS],i,TRUE);
			}


			wdp = FindMyWindow(topWindow);
			if(wdp) {
				if(wdp->dirty)
					EnableItem(gMenu[FILE],	SAVE_I);
				EnableItem(gMenu[FILE],	SAVEAS_I);
				
				/*
				*	Window kind dependent options
				*/
				switch( ((WindowPeek)wdp)->windowKind) {
					case queryW:
						EnableItem(gMenu[EDIT], CUT_I);
						EnableItem(gMenu[EDIT], COPY_I);
						EnableItem(gMenu[EDIT], PASTE_I);
						EnableItem(gMenu[EDIT], CLEAR_I);
						EnableItem(gMenu[EDIT], DUPLICATE_I);
						DisableItem(gMenu[FILE],	PRINT_I);
						break;
					case seqW:
						if( (**(SeqRecHdl)(wdp->userHandle)).firstSel >= 0) {
							EnableItem(gMenu[EDIT], COPY_I);
							EnableItem(gMenu[EDIT], SELNONE_I);
							if( (**(SeqRecHdl)(wdp->userHandle)).firstSel > 0 ||
								 (**(SeqRecHdl)(wdp->userHandle)).lastSel <
							 	         (**(SeqRecHdl)(wdp->userHandle)).nlines -1 )
							 	EnableItem(gMenu[EDIT], SELALL_I);
						 }
						else 
							EnableItem(gMenu[EDIT],SELALL_I);
						
						if ((**(SeqRecHdl)(wdp->userHandle)).xrefHdl != NULL &&
								GetFreeWindow() != -1 )
							EnableItem(gMenu[OSTUFF],XREF_I);
						break;	
					case resW:
						if( (**(ResultHdl)(wdp->userHandle)).nsel > 0 ) {
							EnableItem(gMenu[FILE], EXPORTSEL_I);
							EnableItem(gMenu[EDIT], COPY_I);
							EnableItem(gMenu[EDIT], SELNONE_I);
							if( (**(ResultHdl)(wdp->userHandle)).nsel != 
							    (**(ResultHdl)(wdp->userHandle)).nhits )
							    EnableItem(gMenu[EDIT], SELALL_I);
						}
						else
							EnableItem(gMenu[EDIT],SELALL_I);
						break;
				}		
			}
		}
	}
}