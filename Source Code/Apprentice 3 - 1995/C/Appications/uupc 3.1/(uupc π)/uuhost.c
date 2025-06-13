/*		uuhost.c

*/
#define	MAIN	dcpmain
#define CWDSPOOL

/*** #include	"host.c" ***/
/*** host.c file included below:: ***/
/*		mac.c

		macintosh host 

			Portions Copyright © Gary Morris, 1991.  All Rights Reserved Worldwide.

			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/

#include <stdio.h>
/* #include "host.h" */
#include "dcp.h"

#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <errno.h>

#include <Sound.h>
#include <ShutDown.h>
#include <Connections.h>
#include <CommResources.h>
#include <GestaltEqu.h>

# ifdef THINK_C
# include <console.h>
# include <pascal.h>
#endif THINK_C

/* about dialog id */
#define Duplicate_CTB_Id (1014)
#define Delete_CTB_Id	(1013)
#define Name_CTB_Id		(1012)
#define Debug_Level_Id  (1011)
#define About_Dlog_Id	(1010)
#define Cmd_Dlog_Id		(1000)

typedef enum {
	OK_Button=1, Cancel_Button,
	Slave_RButton, Master_RButton,
	Call_Any_RButton, Call_All_RButton, Call_One_RButton, Call_Schedule,
	Call_Need, Call_Autopilot,
#ifdef DEF_FRAME
	Site_Name_Text, Debug_Level_Text, Debug_Level_Title, D_Default
#else DEF_FRAME
	Site_Name_Text, Debug_Level_Text
#endif DEF_FRAME
} Cmd_Dlog_Items;

typedef enum {
	OK_Button2=1, Cancel_Button2,
	Debug_Level_Item, Button0, Button1, Button5, Debug_Default_Item } Debug_Level_Items;

/* menu id numbers */
#define N_Menus 5
enum Menu_Ids {Apple_Menu_Id=1, File_Menu_Id=1000, Edit_Menu_Id, Call_Menu_Id, CTB_Menu_Id};
				
/* menu item list numbers */		 
enum Menu_Indexes {Apple_Menu_Index, File_Menu_Index, Edit_Menu_Index, Call_Menu_Index,
                   CTB_Menu_Index};
enum File_Menu_Items {Open_And_Call_Item=1, Call_System_Item, Cancel_Call_Item, Separator1, 
					  Debug_Item, Separator2, Abort_Program_Item,
					  Separator3, Quit_Item};
enum Edit_Menu_Items {Undo_Item=1, Cut_Item=3, Copy_Item, Paste_Item, Clear_Item};
enum Call_Menu_Items {All_Sites=1, Any_Site=2, Per_Schedule=3, Need=4, Slave_Mode=6, Autopilot=7, 
                      Specific_Site=9};
enum CTB_Menu_Items {New_Config=1};

static MenuHandle My_Menus[N_Menus];				/* indexed by Menu_Indexes */

#define N_Specific_Sites 256

static char **Sitenames[N_Specific_Sites];
static int siteCount = 0;



char *curdir;
char * getcwd();
/*
FILE * FOPEN();

int CHDIR();
*/

int	debuglevel;		/* debugging level */

int Main_State = Idle_System;
int Background = FALSE;

long int connectionManagerGestalt = 0;

#ifdef Upgrade
long int systimeout = 0;
#endif Upgrade

char *n_argv[20];
char line[128];

SysEnvRec SysEnv;
int settingsFileVRefNum;
long int settingsFileDirID;

#ifdef STANDALONE
# define DMAILBOX	"mailbox"
# define	DNAME		""
# define DHOME		"/usr/home/guest"
# define	DDOMAIN		"mac.uucp"
# define	DMAILDIR	"/usr/spool/mail"
# define	DCONFDIR	"/usr/lib/uucp"
# define	DSPOOLDIR	"/usr/spool/uucp"
# define DPUBDIR		"/usr/spool/uucppublic"
# define	DMAILSERVICE	"crash"
# define	DNODENAME	"shappy"
# define	DDEVICE		"modem"
# define	DSPEED		"2400"
# define	DTEMPDIR	"/usr/tmp"

#include "uuhost.proto.h"

FILE *fe;

void
sgenv(char **thename, char *envname, char *dflt) {
	char line[BUFSIZ];

	fprintf( stderr, "genv: %s %s\n", envname, dflt ); /**/
	
	if (fgets( line, BUFSIZ, fe ) == NULL) {
		fprintf( stderr, "genv: %s not found, using %s\n", envname, dflt ); /* */
		strcpy( *thename, dflt);
	}
	else {
		strcpy( *thename, line);
		fprintf( stderr, "genv: %s %s\n", envname, *thename ); /**/
	}
}

void sloadenv(void)
{
	fprintf( stderr, "sloadenv\n" );
	if ( (fe = FOPEN( "/usr/lib/uucp/defaults", "r" )) == (FILE *) NULL ) {
		fprintf( stderr, "can't open /usr/lib/uucp/defaults  %d\n", errno );
		exit( -1 );
	}
	fprintf( stderr, "sloadenv - fopened\n" );
	/* get environment var's */
	sgenv( &name, NAME, DNAME );
	sgenv( &mailbox, MAILBOX, DMAILBOX );
	sgenv( &nodename, NODENAME, DNODENAME );
	sgenv( &home, HOME, DHOME );
	sgenv( &domain, DOMAIN, DDOMAIN );
	sgenv( &maildir, MAILDIR, DMAILDIR );
	sgenv( &confdir, CONFDIR, DCONFDIR );
	sgenv( &spooldir, SPOOLDIR, DSPOOLDIR );
	sgenv( &pubdir, PUBDIR, DPUBDIR );
	sgenv( &mailserv, MAILSERVICE, DMAILSERVICE );
	sgenv( &device, DEVICE, DDEVICE );
	sgenv( &speed, SPEED, DSPEED );
	sgenv( &tempdir, TEMPDIR, DTEMPDIR );
	
	fclose(fe);
}
#endif

#ifdef DEF_FRAME
pascal void Draw_Ring(WindowPtr W, int Item);

pascal void Draw_Ring(WindowPtr W, int Item)
{
	int Item_Type;
	Handle H;
	Rect R;
	
	/* set the graphport */
	SetPort((GrafPtr)W);

	GetDItem(W, Item, &Item_Type, &H, &R);

	PenNormal();
	PenSize(3, 3);
	FrameRoundRect(&R, 16, 16);
	PenNormal();
}
#endif DEF_FRAME

char *gets();

void Set_Main_State(int New_State);

void Set_Main_State(int New_State)
{
	switch (Main_State) {
	  case Cancel_Call:
	  	if (New_State == Call_Systems) return;
	  	break;
	  case Quit_Program:
	  	/* once in this state, don't change, wait for the program to end */
	  	if (New_State != Idle_System && New_State != Abort_Program) 
	  		return;
	  	break;
	  case Abort_Program:
	  	/* once in this state, don't change, wait for the program to end */
	  	break;
	}
	
	switch (New_State) {
		case Startup_Program:
		case Idle_System:
			EnableItem(My_Menus[File_Menu_Index], Open_And_Call_Item);
			EnableItem(My_Menus[File_Menu_Index], Call_System_Item);
			DisableItem(My_Menus[File_Menu_Index], Cancel_Call_Item);
			DisableItem(My_Menus[File_Menu_Index], Abort_Program_Item);
			EnableItem(My_Menus[Call_Menu_Index], 0);
			if ((connectionManagerGestalt & (1L << gestaltConnMgrPresent)) == 0) {
				DisableItem(My_Menus[CTB_Menu_Index], 0);
			} else {
				EnableItem(My_Menus[CTB_Menu_Index], 0);
			}
    	    HiliteMenu(0);
   			DrawMenuBar();
			printmsg(0, (New_State == Startup_Program) ? "Startup" : "Idle");
			break;
		
		case Call_Systems:
			DisableItem(My_Menus[File_Menu_Index], Open_And_Call_Item);
			DisableItem(My_Menus[File_Menu_Index], Call_System_Item);
			EnableItem(My_Menus[File_Menu_Index], Cancel_Call_Item);
			EnableItem(My_Menus[File_Menu_Index], Abort_Program_Item);
			DisableItem(My_Menus[Call_Menu_Index], 0);
			DisableItem(My_Menus[CTB_Menu_Index], 0);
    	    HiliteMenu(0);
   			DrawMenuBar();
			break;
		
		case Cancel_Call:
			DisableItem(My_Menus[File_Menu_Index], Cancel_Call_Item);
			if (currentConnection) {
				(*currentConnection->Interrupt)();
			}
			printmsg(0, "Cancellation requested");
			PlayNamed("\pCancel");
			break;
		
		case Abort_Program:
			PlayNamed("\pAbort");
			break;

		case Quit_Program:
			if (currentConnection) {
				(*currentConnection->Interrupt) ();
			}
			break;
	}
	
	Main_State = New_State;
}

#ifdef MULTIFINDER
void Do_Command(long Menu_Command, int modifiers);
void Do_Mouse_Action(EventRecord *Cur_Event);


void Set_RButton(DialogPtr Dlog,int Item,int Min,int Max);
void Hilite_Buttons(DialogPtr Dlog,int Min,int Max, int Value);
int Get_Command_Options(void);

void Hilite_Buttons(DialogPtr Dlog, int Min, int Max, int Value)
{
	int i;
	int Item_Type;
	Handle H;
	Rect R;
	
	for (i=Min; i<=Max; i++) {
		GetDItem(Dlog, i, &Item_Type, &H, &R);
		HiliteControl(H, (Value==1) ? 0 : 255);
	}
}

void Set_RButton(DialogPtr Dlog, int Item, int Min, int Max)
{
	int i;
	int Item_Type;
	Handle H;
	Rect R;
	
	for (i=Min; i<=Max; i++) {
		GetDItem(Dlog, i, &Item_Type, &H, &R);
		SetCtlValue(H, (i==Item ? 1 : 0));
	}
}

int Get_Debug_Level(void);

int Get_Debug_Level(void)
{
	int Item_Hit = 0;
	DialogPtr DL_Dlog;
	int Debug_Level = debuglevel;
	int Item_Type;
	Handle H;
	Rect R;
	
	/* open debug level dialog */
	DL_Dlog = GetNewDialog(Debug_Level_Id, (Ptr) 0, (WindowPtr) -1);

#ifdef DEF_FRAME
	/* hilite the default (D_Default) placeholder */
	GetDItem(DL_Dlog, Debug_Default_Item,  &Item_Type, &H, &R);
	InsetRect(&R, -4, -4);
	SetDItem(DL_Dlog, Debug_Default_Item, userItem, Draw_Ring, &R);
#endif DEF_FRAME
	
	GetDItem(DL_Dlog, Debug_Level_Item, &Item_Type, &H, &R);
	sprintf(line, "%d", Debug_Level);
	CtoPstr(line);
	SetIText(H, line);
	SelIText(DL_Dlog, Debug_Level_Item, 0, 32767);
	
	Item_Hit = 0;
	while (Item_Hit != OK_Button && Item_Hit != Cancel_Button) {
		ModalDialog((ProcPtr) 0, &Item_Hit);
	
		switch (Item_Hit) {
			case Button0:
				Debug_Level = 0;
				break;
			
			case Button1:
				Debug_Level = 1;
				break;
			
			case Button5:
				Debug_Level = 5;
				break;
			
			case OK_Button:
				GetIText(H, line);
				PtoCstr(line);
				sscanf(line, "%d", &Debug_Level);
				debuglevel = Debug_Level;
				if (debuglevel < 0) debuglevel = 0;
				if (debuglevel >10) debuglevel = 10;
				break;
		}
		
		if (Item_Hit >= Button0 && Item_Hit <= Button5) {
			sprintf(line, "%d", Debug_Level);
			CtoPstr(line);
			SetIText(H, line);
		}
	}
		
	DisposDialog(DL_Dlog);
}
	
int Get_Command_Options(void)
{
	int Item_Hit = 0;
	DialogPtr Cmd_Dlog;
	int Master = 1;
	int Callee = Call_Schedule;
	char Callee_Str[16];
	int Debug_Level = debuglevel;
	int Item_Type;
	Handle H;
	Rect R;
	
	/* open simulation dialog */
	Cmd_Dlog = GetNewDialog(Cmd_Dlog_Id, (Ptr) 0, (WindowPtr) -1);
	
	Set_RButton(Cmd_Dlog, Master_RButton, Slave_RButton, Master_RButton);
	Set_RButton(Cmd_Dlog, Call_Schedule,  Call_Any_RButton, Call_Autopilot);
	
	GetDItem(Cmd_Dlog, Debug_Level_Text, &Item_Type, &H, &R);
	sprintf(line, "%d", Debug_Level);
	CtoPstr(line);
	SetIText(H, line);
	
	GetDItem(Cmd_Dlog, Site_Name_Text, &Item_Type, &H, &R);
	strcpy(Callee_Str, mailserv);
	CtoPstr(Callee_Str);
	SetIText(H, Callee_Str);
	SelIText(Cmd_Dlog, Site_Name_Text, 0, 32767);

#ifdef DEF_FRAME
	/* hilite the default (D_Default) placeholder */
	GetDItem(Cmd_Dlog, D_Default,  &Item_Type, &H, &R);
	InsetRect(&R, -4, -4);
	SetDItem(Cmd_Dlog, D_Default, userItem, Draw_Ring, &R);
#endif DEF_FRAME
	
	Item_Hit = 0;
	while (Item_Hit != OK_Button && Item_Hit != Cancel_Button) {
		ModalDialog((ProcPtr) 0, &Item_Hit);
	
		switch (Item_Hit) {
			case Master_RButton:
			case Slave_RButton:
				Set_RButton(Cmd_Dlog, Item_Hit, Slave_RButton, Master_RButton);
				Master = (Item_Hit == Master_RButton) ? 1 : 0;
				Hilite_Buttons(Cmd_Dlog, Call_Any_RButton, Call_Autopilot, Master);
				break;
				
			case Call_Any_RButton:
			case Call_All_RButton:
			case Call_One_RButton:
			case Call_Schedule:
			case Call_Need:
			case Call_Autopilot:
				Callee = Item_Hit;
				Set_RButton(Cmd_Dlog, Callee, Call_Any_RButton, Call_Autopilot);
				break;
			case OK_Button:
				Set_Main_State(Call_Systems);
				break;
			case Cancel_Button:
				Set_Main_State(Idle_System);
				break;
			case Site_Name_Text:
				Callee = Call_One_RButton;
				Set_RButton(Cmd_Dlog, Callee, Call_Any_RButton, Call_Autopilot);
				break;
		}
	}

	if (Main_State == Call_Systems) {
		/* extract debug level from dialog box */
		GetDItem(Cmd_Dlog, Debug_Level_Text, &Item_Type, &H, &R);
		GetIText(H, line);
		PtoCstr(line);
		sscanf(line,"%d",&Debug_Level);
	    
	    switch (Callee) {
			case Call_Any_RButton:
				strcpy(Callee_Str, "any");
				break;
			case Call_All_RButton:
				strcpy(Callee_Str, "all");
				break;
			case Call_One_RButton:
				GetDItem(Cmd_Dlog, Site_Name_Text, &Item_Type, &H, &R);
				GetIText(H, Callee_Str);
				PtoCstr(Callee_Str);
				break;
			case Call_Schedule:
				strcpy(Callee_Str, "cron");
				break;
			case Call_Need:
				strcpy(Callee_Str, "need");
				break;
			case Call_Autopilot:
				strcpy(Callee_Str, "auto");
				break;
		}
		sprintf(line, "-r%d -x%d -s%s", Master, Debug_Level, Callee_Str); 
	}
		
	DisposDialog(Cmd_Dlog);
		
}

void Do_Command(long Menu_Command, int modifiers)
{
    int The_Menu;	/* the menu selected */
    int The_Item;	/* the item in the menu */
    Str255 Name;	/* name of DA selected */
    int Ref_Num;	/* reference number of the DA */
    int siteIndex;	/* index number of site to call */
    long dticks;
	Point sfBox;
	SFReply sfReply;
	SFTypeList myType;
	    
    The_Menu = HiWord(Menu_Command);
    The_Item = LoWord(Menu_Command);
    
    switch (The_Menu) {
    	case Apple_Menu_Id:
    	    /* open DA with item's name */
    	    if (The_Item == 1) {
    	    	/* show about box */
				int Item_Hit;
				DialogPtr About_Dlog;
				
				/* open about dialog */
				About_Dlog = GetNewDialog(About_Dlog_Id, (Ptr) 0, (WindowPtr) -1);
				ModalDialog((ProcPtr) 0, &Item_Hit);
				DisposDialog(About_Dlog);
    	    } else {
    	    	GetItem(My_Menus[Apple_Menu_Index],The_Item,Name);
    	    	Ref_Num = OpenDeskAcc(Name);
    	    }
    	    break;
    	    
    	case File_Menu_Id:
    		switch (The_Item) {
    			case Open_And_Call_Item:
					sfBox.h = 100;
					sfBox.v = 50;
					myType[0] = (OSType) 'CALL';
					SFGetFile(sfBox, (unsigned char *) "\p", (ProcPtr) NULL, -1, &myType, (ProcPtr) NULL, &sfReply);
					if (sfReply.good) {
						PtoCstr((char *) sfReply.fName);
						if (strncmp((char *) sfReply.fName, "slave", 5) == 0) {
							sprintf(line, "-r0 -s%s", sfReply.fName);
						} else {
							sprintf(line, "-r1 -s%s", sfReply.fName);
						}
						Set_Main_State(Call_Systems);
					}
    				break;
   				case Call_System_Item:
					/* call which system?  put up dialog */
					Set_Main_State(Call_Systems);
					Get_Command_Options();
					break;
				
				case Cancel_Call_Item:
					Set_Main_State(Cancel_Call);
    	    		Delay(15, &dticks);
					break;
					
				case Debug_Item:
					Get_Debug_Level();
					break;
					
				case Abort_Program_Item:
					Set_Main_State(Abort_Program);
    	    		Delay(15, &dticks);
					break;
					
    			case Quit_Item:
    				Set_Main_State(Quit_Program);	/* exit at next opportunity */
    	    		Delay(15, &dticks);
    				break;
    	    }
			HiliteMenu(0);		/* unhilite menu bar */
    	    break;
    	    
    	case Edit_Menu_Id:
    	    /* process edit command if not System's */
    	    if (!SystemEdit(The_Item-1)) {
    	    	Delay(15, &dticks);
    	    	switch (The_Item) {
    	    	    case Undo_Item:
    	    	    	/* UNDO */
    					break;
    	    	    case Cut_Item: 
    	    	    	/* CUT */
    					break;
    	    	    case Copy_Item:
    	    	    	/* COPY */
    					break;
   	    	    	case Paste_Item:
    	    	    	/* PASTE */
    					break;
    	    	    case Clear_Item:
    	    	    	/* CLEAR */
    					break;
    	    	}
    	    }
    	    break;
    	    
    	case Call_Menu_Id:
			Set_Main_State(Call_Systems);
    		switch(The_Item) {
    			case All_Sites:
					sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, "all");
					break;
    			case Any_Site:
					sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, "any");
					break;
    			case Per_Schedule:
					sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, "cron");
					break;
    			case Slave_Mode:
					sprintf(line, "-r%d -x%d -s%s", 0, debuglevel, "any");
					break;
    			case Need:
					sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, "need");
					break;
    			case Autopilot:
					sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, "auto");
					break;
				default:
					siteIndex = The_Item - (int) Specific_Site;
					if (siteIndex >= 0 && siteIndex < siteCount) {
						HLock(Sitenames[siteIndex]);
						sprintf(line, "-r%d -x%d -s%s", 1, debuglevel, *Sitenames[siteIndex]);
						HUnlock(Sitenames[siteIndex]);
					} else {
						Set_Main_State(Idle_System);
						SysBeep(10);
					}
					break;
			}
			break;
		case CTB_Menu_Id:
			CTB_Setup(The_Item, modifiers);
			break;				
    }
}

void CTB_Setup(int item, int modifiers)
{
	ConnHandle connHandle;
	Str255 menuItemName;
	Point where;
	Handle configHandle;
	OSErr errCode;
	Ptr configString;
	int got, duplicate;
	DialogPtr nameDialog;
	Handle rItem;
	int rItemType, itemHit;
	Rect rRect;
	const char nullString[] = "\p";
	if (item == 1) {
		connHandle = CTBLoadConfig("\pDefault");
	} else {
		GetItem(My_Menus[CTB_Menu_Index], item, menuItemName);
		connHandle = CTBLoadConfig((char *) menuItemName);
	}
	if (!connHandle) {
		SysBeep(10);
		SysBeep(10);
		SysBeep(10);
		return;
	}
	if (item != 1 && (modifiers & optionKey)) {
		ParamText(menuItemName, nullString, nullString, nullString);
		itemHit = CautionAlert(Delete_CTB_Id, (ProcPtr) NULL);
		if (itemHit == 1) {
			CTBPurgeConfig((char *) menuItemName);
			DisableItem(My_Menus[CTB_Menu_Index], item);
		}
		CMDispose(connHandle);
		return;
	}	
	where.h = where.v = 40;
	errCode = CMChoose( &connHandle, where, NULL);
	if (	errCode == chooseDisaster || errCode == chooseFailed ||
			errCode == chooseCancel)
	{
		if (errCode != chooseDisaster) {
			CMDispose( connHandle);
		}
	} else {
		if (item == 1 || (modifiers & shiftKey)) {
ask:		strcpy((char *) menuItemName, "Untitled");
			CtoPstr((char *) menuItemName);
			nameDialog = GetNewDialog(Name_CTB_Id, (char *) NULL, (WindowPtr) -1);
			ShowWindow(nameDialog);
			GetDItem(nameDialog, 4, &rItemType, &rItem, &rRect);
			SetIText(rItem, menuItemName);
			itemHit = 0;
			do {
				ModalDialog((ProcPtr) NULL, &itemHit);
			} while (itemHit != 1 /* OK */ && itemHit != Cancel);
			GetIText(rItem, menuItemName);
			DisposDialog(nameDialog);
			if (itemHit != Cancel) {
				if (CTBIsNamedConfig((char *) menuItemName)) {
					ParamText(menuItemName, nullString, nullString, nullString);
					NoteAlert(Duplicate_CTB_Id, (ProcPtr) NULL);
					ParamText(nullString, nullString, nullString, nullString);
					goto ask;
				}
	   			AppendMenu(My_Menus[CTB_Menu_Index], menuItemName);
				CTBSaveConfig((char *) menuItemName, connHandle);
	   		}
		} else {
			CTBPurgeConfig((char *) menuItemName);
			CTBSaveConfig((char *) menuItemName, connHandle);
		}
		CMDispose( connHandle );
	}
}

void Do_Mouse_Action(EventRecord *Cur_Event)
{
    WindowPeek Which_Window;
    Rect dragRect;
    int code = FindWindow(Cur_Event->where,&Which_Window);
    
    switch (code) {
        case inDesk:
            SysBeep(10);
            break;
    	case inMenuBar:
    	    Do_Command(MenuSelect(Cur_Event->where), Cur_Event->modifiers);
    	    HiliteMenu(0);
            break;
    	case inSysWindow:
    	    SystemClick(Cur_Event,Which_Window);
            break;
    	case inContent:
    	    /* should already be handled by DialogSelect */
            break;
    	case inDrag:
			dragRect = screenBits.bounds;
			DragWindow(Which_Window, Cur_Event->where, &dragRect);
            break;
    	case inGrow:
    	    /* no action, main window has no grow box */
            break;
    	case inGoAway:
			/* we don't have any window to make go away */
            break;
		}
}

long Last_Check_Event = 0;

int Check_Events(long Sleep_Time)
{
/* #define osEvt app4Evt */
#define suspend 1
#define mouseMoved 0xFA
#define resumeMask 1

    EventRecord Cur_Event;
    long dticks;
    int Event_Mask = keyDownMask | mDownMask | mUpMask | keyUpMask | autoKeyMask |
    				 updateMask | activMask | diskEvt;
    
    static int inited = 0;
    static RgnHandle Mouse_Rgn;
    static Point badMountPoint = {100, 100};

    
    if (inited == 0) {
    	Mouse_Rgn = NewRgn();
    	SetRectRgn(Mouse_Rgn,
    			   screenBits.bounds.left,
    			   screenBits.bounds.top,
    			   screenBits.bounds.right,
    			   screenBits.bounds.bottom);
    	inited = 1;
    }
    
	if (currentConnection) {
		(*currentConnection->Idle)();
	}

	(void) WaitNextEvent(Event_Mask, &Cur_Event, Sleep_Time, (Handle) NULL);
	
    if (currentConnection && (*currentConnection->Event)(&Cur_Event)) {
    	goto handled;
    }

    /* any events for us? */
	switch (Cur_Event.what) {
		case keyDown:
	    	if (Cur_Event.modifiers & cmdKey) {
	    		SetCursor(&arrow);
				Do_Command(MenuKey((char) Cur_Event.message & 0xFF), 0);
	    		Delay(15,&dticks);
	    		HiliteMenu(0);   /* unhilite after it's seen */
	    	} 
			break;	    
				    
		case mouseDown:
			SetCursor(&arrow);
			Do_Mouse_Action(&Cur_Event);
			break;
		
		case diskEvt:
			if ((Cur_Event.message >> 16) != noErr) {
				(void) DIBadMount(badMountPoint, Cur_Event.message);
			}
			break;
			
		case activateEvt:
			if (Cur_Event.modifiers & activeFlag) {
				/* set port to the window becoming activated */
				SetPort((WindowPtr) Cur_Event.message);
			}
			break;
	    	    	
		case updateEvt:
			break;
	    	    	
		case osEvt:
			if (((Cur_Event.message >> 24) & 0xFF) == suspend) {
				Background = (Cur_Event.message & resumeMask) ? FALSE : TRUE;
				/* switch to background or foreground */
			}
			break;
	}

handled:
	
	Last_Check_Event = Ticks;
	
	return (Main_State == Cancel_Call) || (Main_State == Quit_Program) || 
		   (Main_State == Abort_Program) || (Main_State == Shutdown_Program);
}
#endif

int Count_Appl_Files(void);
int Count_Appl_Files(void)
{
	int message, count;
	
	CountAppFiles(&message, &count);
	
	return count;
}

void AppendToCallMenu(void);

void AppendToCallMenu(void)
{
	char state;
	int i, j;
	char **t;
	mkfilename( line, confdir,  SYSTEMS );
	if (strchr(confdir, SEPCHAR) == NULL && strchr(confdir, DIRCHAR) == NULL)
		/* make it an absolute pathname */
		mkfilename(s_systems, curdir, line);
	else
		strcpy(s_systems, line);
	if (( fsys = FOPEN( s_systems, "r", 't' )) == (FILE *)NULL ) {
		printmsg(0, "Can't open %s file!", s_systems);
		return;
	}
	while (TRUE) {
		if (siteCount >= N_Specific_Sites) break;
		state = getsystem("list");
		if (state == 'A') break;
		if (strcmp(rmtname, "INCOMING") == SAME || strcmp(rmtname, "ANONYMOUS") == SAME) {
			goto scanNext;
		}
		for (i = 0; i < siteCount; i++) {
			if (strcmp(rmtname, *Sitenames[i]) == SAME) {
				goto scanNext;
			}
		}
		Sitenames[siteCount] = NewHandle(strlen(rmtname) + 1);
		MoveHHi(Sitenames[siteCount]);
		HLock(Sitenames[siteCount]);
		strcpy(*Sitenames[siteCount], rmtname);
		siteCount++;
scanNext: ;
	}
	fclose( fsys );
	fsys = (FILE *)NULL;
	for (i = 0; i < siteCount; i++) {
		for (j = i+1; j < siteCount; j++) {
			if (strcmp(*Sitenames[i], *Sitenames[j]) > 0) {
				t = Sitenames[i];
				Sitenames[i] = Sitenames[j];
				Sitenames[j] = t;
			}
		}
		strcpy(rmtname, *Sitenames[i]);
		HUnlock(Sitenames[i]);
		CtoPstr(rmtname);
		AppendMenu(My_Menus[Call_Menu_Index], rmtname);
	}
}
		



main(int argc, char **argv)
{
	int Status;
	int i;
	int appFiles, appFileNo;
	AppFile appFile;
	Str255 apName;
	Handle apParam;
	
	SysEnvirons(1, &SysEnv);
	
	if (noErr != Gestalt(gestaltConnMgrAttr, &connectionManagerGestalt)) {
		connectionManagerGestalt = 0;
	}

	console_options.title = (unsigned char *) "\pUUPC";
	console_options.nrows = 24;		/* don't need that huge window, */
	console_options.ncols = 64;		/* especially when it's running in background */
	macinit();

    /***** setup menu bar *****/
    /* get handles to menu resources */
    My_Menus[Apple_Menu_Index]   = NewMenu(Apple_Menu_Id, "\p\024" );
    My_Menus[File_Menu_Index]    = GetMenu(File_Menu_Id);
    My_Menus[Edit_Menu_Index]    = GetMenu(Edit_Menu_Id);
    My_Menus[Call_Menu_Index]    = GetMenu(Call_Menu_Id);
    My_Menus[CTB_Menu_Index]    = GetMenu(CTB_Menu_Id);
    
    AppendMenu(My_Menus[Apple_Menu_Index],"\pAbout UUPC...");
    AppendMenu(My_Menus[Apple_Menu_Index],"\p(-");
    AddResMenu(My_Menus[Apple_Menu_Index],'DRVR');	/* get DA names */
    
    for (i=0; i<N_Menus; i++) {
        InsertMenu(My_Menus[i],0);		/* Add menu to menu bar */
    }

    DrawMenuBar();
    Set_Main_State(Startup_Program);
    
	/* mac specific prolog */
	
	loadenv();
	
	AppendToCallMenu();
    
    CTBAddToMenu(My_Menus[CTB_Menu_Index]);

	appFiles = Count_Appl_Files();
	appFileNo = 1;
	
	curdir = getcwd( NULL, 0 );
#ifdef CWDSPOOL
	CHDIR( spooldir );
#endif

	while (Main_State != Quit_Program && Main_State != Abort_Program && 
	       Main_State != Shutdown_Program) {
		
		switch (Main_State) {

			case Idle_System:
				Check_Events(50);
			case Startup_Program:
				if (appFileNo <= appFiles) {
					GetAppFiles(appFileNo, &appFile);
					appFileNo++;
					PtoCstr((char *) appFile.fName);
					if (strncmp((char *) appFile.fName, "slave", 5) == 0 &&
						!isalnum(appFile.fName[5])) {
						printmsg(0, "Launch request: %s", appFile.fName);
						sprintf(line, "-r0 -x0 -s%s", appFile.fName);
					} else {
						sprintf(line, "-r1 -x0 -s%s", appFile.fName);
						printmsg(0, "Launch request: call %s", appFile.fName);
					}
					Set_Main_State(Call_Systems);
				} else {
					if (Main_State == Startup_Program) {
						Set_Main_State(Idle_System);
					}
				}
						
				break;
				
			case Call_Systems:
				Check_Events(MF_DELAY);
				argc = getargs(line, n_argv);
				argv = n_argv;

				MAIN( argc, argv );
				
				if (Main_State == Call_Systems) Set_Main_State(Idle_System);
				break;
			
			case Cancel_Call:
				Check_Events(MF_DELAY);
				Set_Main_State(Idle_System);
				appFiles = 0;
				break;
				
			default:
				Check_Events(MF_DELAY);
				break;
		}
	}
	
	/* mac specific epilog */
	if (Main_State == Shutdown_Program) {
		ShutDwnPower();
	}
	chdir( curdir );
	exit(0);
}

void PlayNamed(char *theName)
{
	Handle h;
	long int flags;
	OSErr osErr;
	/*** this is the WRONG TEST but I can't figure out how to tell whether
	     SndPlay is present except by checking for the Apple Sound Chip!
	***/
	osErr = Gestalt('hdwr', &flags);
	if (osErr == noErr &&
	    (flags & 0x8) != 0 &&
	    (h = GetNamedResource('snd ', theName)) != NULL) {
		SndPlay(NULL, h, FALSE);
	}
}

