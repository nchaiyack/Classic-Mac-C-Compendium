/* this file contains code from two TextEdit-based little editors: */

/*********************************************************************

	MiniEdit.c

	The sample application from Inside Macintosh (RoadMap p.
	15-17)
	beefed up a bit by Stephen Z. Stein, Symantec Corp.

*********************************************************************/
/*------------------------------------------------------------------------------
#
#	Apple Macintosh Developer Technical Support
#
#	MultiFinder-Aware TextEdit Sample Application
#
#	TESample.c
#
#	Copyright � 1989 Apple Computer, Inc.
#	All rights reserved.
#
------------------------------------------------------------------------------*/

/* the integration of the two environments is by us: */

/*******************************************************************************\

windows initialization module

suntar, �1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

#include "windows.h"

#define REQUIREDSTACK 16384


void my_init(void);
void crea_menu_font(void);
void prefs_da_risorsa(void);
void set_marks_size(void);
void set_marks_style(void);

extern MenuHandle		myMenus[4],fontM[3];
extern short quit_selection,n_about_entries;
extern short current_font,current_size,current_style,curr_font_index;
extern WindowPtr foreign_window[n_max_foreign];
extern upd_proc update_handler[n_max_foreign],activate_handler[n_max_foreign];
extern struct acur{
	short n_cursors;
	short delay;
	Ptr cursor_ID_or_Ptr[1000];	/* the true size depends on the size of the resource */
	} *rot_cursor;
extern SysEnvRec	gMac;				/* set up by Initialize */


/* kSysEnvironsVersion is passed to SysEnvirons to tell it which version of the
   SysEnvRec we understand. */
#define	kSysEnvironsVersion		1
#define _WaitNextEvent	0xA860
#define _AliasDispatch	0xA823
#define _SCSIDispatch	0xA815
#define _SCSIAtomic		0xA089	/* SCSI manager 4.3, CAM */
#define _Gestalt		0xA1AD
#define _KeyTrans		0xA9C3

#define _InitGraf		0xA86E
#define _Unimplemented	0xA89F

#define TrapType Trap_Type
typedef unsigned char Trap_Type;		/* changed name, to avoid a conflict with
										newer standard headers which might have that
										declaration */


#ifdef SUNTAR
extern Boolean		gHasResolveAlias;
extern Boolean		gHasFindFolder;
extern Boolean		gHasCustomPutFile;
extern Boolean		gHasVirtualMemory;
extern Boolean		gHasSCSIManager;
#endif
extern Boolean	gHasKeyTrans;
extern Boolean		gHasWaitNextEvent;	/* set up by Initialize */


static void SetUpMenus()
{
	short	i;

	myMenus[appleM] = GetMenu( appleID );
	if(myMenus[appleM]==NULL){
		SysBeep(5);
		SysBeep(5);
		SysBeep(5);
		ExitToShell();
		}
	n_about_entries=CountMItems(myMenus[appleM]);
	if(n_about_entries>0)
		AppendMenu(myMenus[appleM], "\p(-");
	AddResMenu( myMenus[appleM], 'DRVR' );

	myMenus[fileM] = GetMenu(fileID);
	quit_selection=CountMItems(myMenus[fileM]);

#ifdef SUNTAR
	prefs_da_risorsa();		/* one of the MENU resources has "checks" which
							may be reloaded from a preferences file */
#endif
	myMenus[editM] = GetMenu(editID);
	myMenus[prefM] = GetMenu(prefID);
	
	for ( (i=appleM); (i<sizeof(myMenus)/sizeof(MenuHandle)); i++ ) 
		InsertMenu(myMenus[i], 0) ;
	DrawMenuBar();
	if(my_add_menus!=NULL)
		(*my_add_menus)();

	crea_menu_font();	/* it takes a lot of time, and it's more pleasant
					to do that after the Menu bar was drawn, at least it
					looks like suntar is a lot faster to load */
	DrawMenuBar();
}


static void crea_menu_font()
{
short i,font_n;
	Str255 fontName;
/* gestione men� dei font */
/* ispirata al dimostrativo in Pascal di Carlo Rogialli, SP Computer aprile 1989
-- inspired to the demo program in Pascal by Carlo Rogialli, in an italian magazine 
 */
	fontM[0]=GetMenu(fontID);
	AddResMenu( fontM[0], 'FONT' );
	fontM[1]=GetMenu(sizeID);
	fontM[2]=GetMenu(styleID);
	for(i=0;i<3;i++)
		InsertMenu(fontM[i], -1);

	curr_font_index=1+CountMItems(fontM[0]);
	do{
		curr_font_index--;
		GetItem(fontM[0],curr_font_index,&fontName);
		GetFNum(fontName,&font_n);
		/*printf("font %P %d\n",fontName,font_n);*/
		}
	while(font_n!=current_font &&curr_font_index>0);	/* se il font monaco non � presente 
												si comporta stranamente ma almeno non crasha */		
	SetItemMark (fontM[0], curr_font_index, '�');

	for(i=CountMItems(fontM[1]);i>0;i--){	/* if a size is checked, accept it; otherwise,
						use the default value (9) */
		short markChar;
		GetItemMark (fontM[1],i,&markChar);
		if(markChar==checkMark){
			char sizestr[16];
			GetItem(fontM[1],i,sizestr);
			current_size=pstrtoi(sizestr);
			break;
			}
		}
	set_marks_size();
	set_marks_style();
}

void SetUpCursors(void);
static void SetUpCursors()
{
	CursHandle	hCurs;
	short i;

	hCurs = GetCursor(1);
	editCursor = **hCurs;
	hCurs = GetCursor(watchCursor);
	waitCursor = **hCurs;

	ResrvMem ((Size)200);
	hCurs=GetResource('acur',128);
	HLock(hCurs);
	rot_cursor= *hCurs;
	if(rot_cursor->delay==0) rot_cursor->delay=10;

	for(i=0;i<rot_cursor->n_cursors;i++){
		ResrvMem ((Size)sizeof(Cursor));
		hCurs=GetCursor(hiword(rot_cursor->cursor_ID_or_Ptr[i]));
		HLock(hCurs);
		rot_cursor->cursor_ID_or_Ptr[i]= *hCurs;
		}
}

#if 0
/*  from TESample.c */
/*	Check to see if a given trap is implemented. This is only used by the
	Initialize routine in this program, so we put it in the Initialize segment.
	The recommended approach to see if a trap is implemented is to see if
	the address of the trap routine is the same as the address of the
	Unimplemented trap. */
/*	1.02 - Needs to be called after call to SysEnvirons so that it can check
	if a ToolTrap is out of range of a pre-MacII ROM. */

Boolean TrapAvailable(tNumber,tType)
	short		tNumber;
	Trap_Type	tType;
{
	if ( ( tType == (unsigned char) ToolTrap ) &&
		( gMac.machineType > envMachUnknown ) &&
		( gMac.machineType < envMacII ) ) {		/* it's a 512KE, Plus, or SE */
		tNumber = tNumber & 0x03FF;
		if ( tNumber > 0x01FF )					/* which means the tool traps */
			tNumber = _Unimplemented;			/* only go to 0x01FF */
	}
	return NGetTrapAddress(tNumber, tType) != GetTrapAddress(_Unimplemented);
} /*TrapAvailable*/
#else
/* this one is suggested by Inside Mac volume VI, hence probably it's better;
but we did not want to have three function to do what a single one would do,
and a #define for _GetTrapType is evaluated at compile time... */

#define _GetTrapType(theTrap) ((theTrap & 0x0800) !=0 )
#define TrapAvailable(theTrap) _TrapAvailable(theTrap,_GetTrapType(theTrap) )


Boolean _TrapAvailable(short,TrapType);

static Boolean	_TrapAvailable(short theTrap,TrapType tType)
{
	/* TrapType tType;

	tType = GetTrapType(theTrap); */
	if (tType == ToolTrap) {
		theTrap &= 0x07FF;
		if (theTrap >= 
			(NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E,ToolTrap) ?
				0x0200 : 0x0400) )
			theTrap = _Unimplemented;
	}

	return NGetTrapAddress(theTrap, tType) != NGetTrapAddress(_Unimplemented, ToolTrap);
}

#endif



static void my_init()
{
register short i;
Handle h;


		/* load all the code resources, before allocating buffers, otherwise
		I might allocate all the memory for buffers before loading a necessary
		module. This strategy would have been silly on the old Mac 128k, but
		since suntar is a small program by today's standards, it's not
		a big waste of memory to load everything */
PurgeMem(maxSize);
SetResLoad(false);
i=0;
while( (h=Get1Resource('CODE',++i)) !=NULL){
	if(*h==NULL){
		ResrvMem(SizeResource(h));
		LoadResource(h);
		/*HLock(h);*/
		}
	}
SetResLoad(true);

	for(i=0;i<n_max_windows;i++)
		my_windows[i].used=false;
	SetUpCursors();
	for(i=0;i<n_max_foreign;i++)
		foreign_window[i]=NULL;
	SetUpMenus();
}


void InitConsole()
{	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents( everyEvent&~diskMask, 0 );
	InitWindows();
	InitMenus();
	MoreMasters();
	MoreMasters();	/* the typical heap of suntar has 100 used handles, obviously
					peek values may go beyond 128, hence three master blocks (one
					preallocated and two MoreMasters) avoid having a new nonrelocatable
					block placed among relocatable blocks */
	TEInit();
	InitDialogs(0L);
	InitCursor();
	if(ApplLimit> CurStackBase-(long)REQUIREDSTACK )
		SetApplLimit(CurStackBase-(long)REQUIREDSTACK);
	MaxApplZone();

	/*	Ignore the error returned from SysEnvirons; even if an error occurred,
		the SysEnvirons glue will fill in the SysEnvRec. You can save a redundant
		call to SysEnvirons by calling it after initializing AppleTalk. */

	SysEnvirons(kSysEnvironsVersion, &gMac);

	/* Make sure that the machine has at least 128K ROMs. If it doesn't, exit. */

	if (gMac.machineType < 0){SysBeep(5); ExitToShell();}

	gHasWaitNextEvent = TrapAvailable(_WaitNextEvent /*, ToolTrap*/);
	gHasResolveAlias  = TrapAvailable(_AliasDispatch /*, ToolTrap*/);
	gHasKeyTrans = TrapAvailable(_KeyTrans);
	gHasSCSIManager = TrapAvailable(_SCSIDispatch);
	if(gHasSCSIManager && TrapAvailable(_SCSIAtomic)) gHasSCSIManager=2;

	gHasCustomPutFile=false;
	gHasVirtualMemory=false;
	if(TrapAvailable(_Gestalt /*, ToolTrap*/)){
		long response;
		OSErr err;
		#ifdef HAS_GESTALT_GLUE
		err=Gestalt('stdf',&response);
		#else	/* the libraries of Think C 4 don't contain Gestalt... */
		asm{
			move.l #'stdf',D0
			dc.w _Gestalt
			move.l A0,response
			move.w D0,err
			}
		#endif
		gHasCustomPutFile= err==noErr && ((short)response&1);
		#ifdef HAS_GESTALT_GLUE
		err=Gestalt('vm  ',&response);
		#else
		asm{
			move.l #'vm  ',D0
			dc.w _Gestalt
			move.l A0,response
			move.w D0,err
			}
		#endif
		if(err) response=0;
		gHasVirtualMemory=response&1;

		#ifdef HAS_GESTALT_GLUE
		err=Gestalt('fold',&response);
		#else
		asm{
			move.l #'fold',D0
			dc.w _Gestalt
			move.l A0,response
			move.w D0,err
			}
		#endif
		if(err) response=0;
		gHasFindFolder=response&1;

		}

	my_init();
}


