/**********************************************************************

    Segment: Main.c

***********************************************************************/

/*
    Argus Text Editor
    (Written using THINK C 6.0)

    Argus Starter used as program template.
    Uses Argus Function Libraries
*/


/********** Includes */
#include <Types.h>
#include <Quickdraw.h>
#include <Controls.h>
#include <Desk.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Editions.h>
#include <EPPC.h>
#include <Events.h>        
#include <Fonts.h>
#include <GestaltEqu.h>
#include <Lists.h>
#include <Menus.h>
#include <OSEvents.h>
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Traps.h>
#include <AppleEvents.h>   
#include <Balloons.h>      

#include "Fn_Prototypes.h"

/********** Defines */
#define BASE_RES             400
#define NIL_PTR              0L
#define MOVE_TO_FRONT        -1L
#define LEAVE_IT             FALSE
#define DRAG_THRESHOLD       30
#define REMOVE_ALL_EVENTS    0
#define MIN_SLEEP            60L
#define NIL_MOUSE_REGION     0L
#define WNE_TRAP_NUM         0x60
#define UNIMPL_TRAP_NUM      0x9F
#define NIL_STR              "\p"
#define MIN_SYS_VERSION      0x700  // identify min sys req
#define HELP_MENU            "\pArgus TE Help..."

/********** General */
#define ZERO             0
#define FIRST            1
#define SECOND           2
#define THIRD            3
#define FOURTH           4

#define LINE_FEED        10
#define RETURN           13
#define SPACE            32
#define TAB              9

/********** Window definitions */
#define WIND_LEFT        5
#define WIND_TOP         45
#define WIND_OFFSET      20

/********** Menus */
#define APPLE_MENU     BASE_RES
#define FILE_MENU      BASE_RES + 1
#define EDIT_MENU      BASE_RES + 2

/********** Error Strings */
#define GENERAL_ERR    900
#define BAD_SYS        901
#define N0_RESOURCE    902

/********** Apple Menu Stuff */
#define ABOUT          1

/********** File Menu Stuff */
#define NEW            1
#define OPEN           2
#define CLOSE          3
#define SAVE           5
#define SAVEAS         6
#define PAGESETUP      8
#define PRINT          9
#define QUIT           11

/********** Edit Menu Stuff */
#define UNDO           1
#define CUT            3
#define COPY           4
#define PASTE          5
#define CLEAR          6
#define SELECTALL      8

/********** Variables */
Boolean             gDone;
EventRecord         gTheEvent;
MenuHandle          gAppleMenu;
MenuHandle          gFileMenu;
MenuHandle          gEditMenu;
Rect                gDragRect;
short               gAppResourceFile;
struct SysConfigRec gSysConfig;
int                 gNewWindowLeft = WIND_LEFT;
int                 gNewWindowTop = WIND_TOP;

WindowPtr           gWindow;        // current window
TEHandle            gTEH;           // current TE record
ControlHandle       gSBH;           // current scroll bar
Boolean             gDirty;         // current TE dirty status
Boolean             gSaved;         // current TE saved status
Str255              gFilename;      // current filename
short               gVRef;          // current file VRef
short               gRefNum;        // current file RefNum
THPrint             gPrintH = NULL; // current print record

extern struct WindowData
{
    TEHandle      wTEH;
    ControlHandle wSBH;
    Boolean       wDirty;
    Boolean       wSaved;
    Str255        wFilename;
    short         wVRef;
    short         wRefNum;
    THPrint       wPrintH;
};


/********** Structures */
struct SysConfigRec
{
    Boolean hasGestalt;
    Boolean hasWNE;
    Boolean hasColorQD;
    Boolean hasAppleEvents;
    Boolean hasEditionMgr;
    Boolean hasHelpMgr;

    long    sysVersion;
};


/********** Prototypes */
void ToolBoxInit_( void );
Boolean TrapAvailable_( short tNumber, TrapType tType );
static void GetSysConfig_( void );
void MenuBarInit_( void );
void SetUpDragRect_( void );
void MainLoop_( void );
void HandleEvent_( void );
void HandleMouseDown_( void );
void AdjustMenus_( void );
int  IsDAWindow_( WindowPtr whichWindow );
void HandleMenuChoice_( long int menuChoice );
void HandleAppleChoice_( int theItem );
void HandleFileChoice_( int theItem );
void HandleEditChoice_( int theItem );
void MySetGlobals( WindowPtr w );


/********** main */

main()
{
    ToolBoxInit_();
    GetSysConfig_();
    
    if( gSysConfig.hasAppleEvents )
        FnAE_InitAE();
    
    MenuBarInit_();
    SetUpDragRect_();
    
    if( !gSysConfig.hasAppleEvents )
        /* put OpenApp procedure here */
        MyCreateWindow( BASE_RES, NIL_PTR, (WindowPtr)MOVE_TO_FRONT,
            WIND_TOP, WIND_LEFT, WIND_OFFSET, N0_RESOURCE );

    MainLoop_();
}


/********** ToolBoxInit */

void ToolBoxInit_( void )
/*
    Standard initialization procedure per IM:Overview p4-75
*/
{
    MaxApplZone();
    MoreMasters();
    
    InitGraf( &thePort );
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs( NIL_PTR );
    
    FlushEvents( everyEvent, REMOVE_ALL_EVENTS );    
    InitCursor();
}


/********** TrapAvailable */

Boolean TrapAvailable_( short tNumber, TrapType tType )
{
    return( NGetTrapAddress( tNumber, tType ) 
    != GetTrapAddress( _Unimplemented ) );
}


/********** GetSysConfig */

static void GetSysConfig_( void )
{   
    OSErr        ignoreError;
    long         tempLong;
    SysEnvRec    environs;
    short        myBit;


    gAppResourceFile = CurResFile();  /* set app resource fork ID */

    /* Check to see if Gestalt Manager is supported */
    gSysConfig.hasGestalt = TrapAvailable_( _Gestalt, ToolTrap );
    if( !gSysConfig.hasGestalt )
        /* Something has got to be wrong */
        FnErr_DisplayStrID( BAD_SYS, TRUE );
    else
    {
        /* Determine various system configuration checks */

        /* Check to see if WaitNextEvent is supported */
        gSysConfig.hasWNE = TrapAvailable_( _WaitNextEvent, ToolTrap );
        
        /* Check for Color Capability */
        ignoreError = Gestalt( gestaltQuickdrawVersion, &tempLong );
        gSysConfig.hasColorQD = ( tempLong != gestaltOriginalQD );
        
        /* Check to see if AppleEvents are supported in OS */
        gSysConfig.hasAppleEvents = ( Gestalt( gestaltAppleEventsAttr,
            &tempLong ) == noErr );

        /* Check for Edition Manager */
        gSysConfig.hasEditionMgr = ( Gestalt( gestaltEditionMgrAttr,
            &tempLong ) == noErr );
        if( gSysConfig.hasEditionMgr )
            if( InitEditionPack() != noErr )
                gSysConfig.hasEditionMgr = false;
                
        /* Check for Help Manager */
        ignoreError = Gestalt( gestaltHelpMgrAttr, &tempLong );
        myBit = gestaltHelpMgrPresent;
        gSysConfig.hasHelpMgr =
            BitTst( &tempLong, 31 - myBit );
            
        /* Determine and Check OS Version */
        gSysConfig.sysVersion = 0.0;
        ignoreError = Gestalt( gestaltSystemVersion, &tempLong );
        gSysConfig.sysVersion = tempLong;
        if( MIN_SYS_VERSION > gSysConfig.sysVersion )
            FnErr_DisplayStrID( BAD_SYS, TRUE );
    }
}


/********** MenuBarInit    */

void MenuBarInit_( void )
{
    Handle     myMenuBar;
    MenuHandle helpMenu;
    OSErr      err;

    myMenuBar = GetNewMBar( BASE_RES ) ;
    if ( myMenuBar == NIL_PTR )
        FnErr_DisplayStrID( N0_RESOURCE, TRUE );
    SetMenuBar( myMenuBar );
    
    if ( ( gAppleMenu = GetMHandle( APPLE_MENU ) ) == NIL_PTR )
        FnErr_DisplayStrID( N0_RESOURCE, TRUE );
    if ( ( gFileMenu = GetMHandle( FILE_MENU ) ) == NIL_PTR )
        FnErr_DisplayStrID( N0_RESOURCE, TRUE );
    if ( ( gEditMenu = GetMHandle( EDIT_MENU ) ) == NIL_PTR )
        FnErr_DisplayStrID( N0_RESOURCE, TRUE );
        
    AddResMenu( gAppleMenu, 'DRVR' );
    DrawMenuBar();
    
    if( gSysConfig.hasHelpMgr )
    {
        err = HMGetHelpMenuHandle( &helpMenu );
        if( err == noErr )
        {
            if( helpMenu != nil )
            {
                 AppendMenu( helpMenu, HELP_MENU );   
            }
        }
    }
}


/********** SetUpDragRect */

void SetUpDragRect_( void )
{
    gDragRect = screenBits.bounds;
    gDragRect.left += DRAG_THRESHOLD;
    gDragRect.right -= DRAG_THRESHOLD;
    gDragRect.bottom -= DRAG_THRESHOLD;
}


/********** MainLoop */

void MainLoop_( void )
{
    gDone = FALSE;
    while( gDone == FALSE )
    {
        HandleEvent_();
    }
}


/********** HandleEvent */

void HandleEvent_( void )
{
    char    theChar;
    GrafPtr oldPort;
    Point   displayPoint;
    struct  WindowData *wd;

    if ( gSysConfig.hasWNE )
        WaitNextEvent( everyEvent, &gTheEvent, MIN_SLEEP,
            NIL_MOUSE_REGION );
    else
    {
        SystemTask();
        GetNextEvent( everyEvent, &gTheEvent );
    }

    if((gWindow != 0) && (gWindow == FrontWindow()))
    {
        FnTE_MaintainCursor( gWindow, &gTEH );
        TEIdle( gTEH );
    }

    switch ( gTheEvent.what )
    {
        case mouseDown: 
            HandleMouseDown_();
            break;
        case mouseUp:
            // this application doesn't use mouseUp events
            break;
        case keyDown:
        case autoKey:
            theChar = gTheEvent.message & charCodeMask;
            if (( gTheEvent.modifiers & cmdKey ) != 0)
            {
                AdjustMenus_(); 
                HandleMenuChoice_( MenuKey( theChar ) );
            }
            else
            {
                if((gWindow == FrontWindow()) && (gWindow != 0) )
                {
                    wd = (struct WindowData *)GetWRefCon(gWindow);
                    FnTE_DoKeyDown(
                        theChar,
                        &gTEH,
                        &gSBH,
                        (char *)&gDirty );
                    wd->wDirty = gDirty;
                }
            }
            break;
        case updateEvt:
            if ( !IsDAWindow_( (WindowPtr)gTheEvent.message ) )
            {
                /* Handle update event */
                MyDoUpdateWindow( (WindowPtr)gTheEvent.message );
             }
            break;
        case diskEvt:
            // most applications don't need to worry about diskEvt's
             break;
        case activateEvt:
            if ( !IsDAWindow_( (WindowPtr)gTheEvent.message ) )
            {
                if ( gTheEvent.modifiers & activeFlag )
                {
                    /* Handle activate event. */
                    MyDoActivateWindow((WindowPtr)gTheEvent.message,
                        TRUE );
                }
                else
                {
                    /* Handle deactivate event. */
                    MyDoDeactivateWindow((WindowPtr)gTheEvent.message,
                        FALSE );
                }
            }
        case osEvt:
            // this application doesn't support operating sys events
            break;
        case nullEvent:
            // ignore
            break;
        case kHighLevelEvent:
            FnAE_DoHighLevelEvent( &gTheEvent );
            break;
    }
}


/********** HandleMouseDown */

void HandleMouseDown_( void )
{
    WindowPtr         whichWindow;
    WindowPtr         w;
    short int         thePart;
    long int          menuChoice;    
    Point             theLocation;
    GrafPtr           savePort;
    struct WindowData *wd;
    int               SBarWidth = 15;
    Rect              oldHorizBar;
    
    thePart = FindWindow( gTheEvent.where, &whichWindow );
    switch ( thePart )
    {
        case inMenuBar:
            AdjustMenus_();
            menuChoice = MenuSelect( gTheEvent.where );
            HandleMenuChoice_( menuChoice );
            break;
        case inSysWindow: 
            SystemClick( &gTheEvent, whichWindow );
            break;
        case inContent:
            if( whichWindow != FrontWindow() )
                SelectWindow( whichWindow );
            else if( !IsDAWindow_( (WindowPtr)gTheEvent.message ) )
                MyDoContent( whichWindow, &gTheEvent );
            break;
        case inDrag:
            if( whichWindow != FrontWindow() )
            {
                SelectWindow( whichWindow );
                DragWindow( whichWindow, gTheEvent.where, &gDragRect );
            }
            else
                DragWindow( whichWindow, gTheEvent.where, &gDragRect );
            break;
        case inGrow:
            MyDoGrow( whichWindow, gTheEvent.where );
            break;
        case inGoAway:
            theLocation = gTheEvent.where;
            GlobalToLocal( &theLocation );
            if( TrackGoAway( whichWindow, theLocation ) )
                DisposeWindow( whichWindow );
            break;
        case inZoomIn:
        case inZoomOut:
            // only supports zoomout
            w = whichWindow;
            if( w == gWindow )
            {
                if( TrackBox( w,gTheEvent.where,thePart ) )
                {
		            wd = (struct WindowData *)GetWRefCon(gWindow);
					//HideWindow( w );
					
					
						
					GetPort(&savePort);
					SetPort(w);
					    EraseRect(&w->portRect);
					    MoveWindow( w, WIND_LEFT, WIND_TOP, FALSE );
					    SizeWindow( w, screenBits.bounds.right - 10, 
					        screenBits.bounds.bottom - WIND_TOP - 5,
					        FALSE );
					    
					    oldHorizBar = w->portRect;
					    oldHorizBar.top = oldHorizBar.bottom - 
					        (SBarWidth+1);
					    //InvalRect(&w->portRect);
					    //EraseRect(&oldHorizBar);
					    
					    MoveControl(wd->wSBH, w->portRect.right - 
					        SBarWidth, w->portRect.top-1);
					    SizeControl(wd->wSBH, SBarWidth+1,
					        w->portRect.bottom - w->portRect.top -
					        (SBarWidth-2));
					    //ValidRect(&((**wd->wSBH).contrlRect));
					    //BringToFront( w );
					    //ShowWindow( w );
					    
					    // reset dest and view rects
					    (**(wd->wTEH)).viewRect = w->portRect;
					    (**(wd->wTEH)).viewRect.right -= SBarWidth;
					    (**(wd->wTEH)).viewRect.bottom -= SBarWidth;
					    InsetRect(
					        &(**(wd->wTEH)).viewRect,
					        3,   // inset pixels
					        3 ); // inset pixels
					    (**(wd->wTEH)).destRect = 
					        (**(wd->wTEH)).viewRect;
					        
                        TECalText( wd->wTEH );
					    FnTE_DetSBarIncr( &(wd->wTEH), &(wd->wSBH) );
	                    FnTE_AdjustText( &(wd->wTEH), &(wd->wSBH) );
	                    DrawGrowIcon( w );
	                    TEUpdate( &w->portRect, wd->wTEH );
					SetPort(savePort);
				}
			}
            break;
    }
}


/********** AdjustMenus */

void AdjustMenus_( void )
{
    WindowPtr currentWindow;

    if (IsDAWindow_( FrontWindow() ) )
    {   // DA Window
        EnableItem( gEditMenu, UNDO );
        EnableItem( gEditMenu, CUT );
        EnableItem( gEditMenu, COPY );
        EnableItem( gEditMenu, PASTE );
        EnableItem( gEditMenu, CLEAR );
        DisableItem( gEditMenu, SELECTALL );
    }
    else
    {
        if ( ( currentWindow = FrontWindow() ) == NIL_PTR )
        {   // no window currently open
            DisableItem( gFileMenu, CLOSE );
            DisableItem( gFileMenu, SAVE );
            DisableItem( gFileMenu, SAVEAS );
            DisableItem( gFileMenu, PAGESETUP );
            DisableItem( gFileMenu, PRINT );

            DisableItem( gEditMenu, UNDO );
            DisableItem( gEditMenu, CUT );
            DisableItem( gEditMenu, COPY );
            DisableItem( gEditMenu, PASTE );
            DisableItem( gEditMenu, CLEAR );
            DisableItem( gEditMenu, SELECTALL );
        }
        else
        {   // window open
            EnableItem( gFileMenu, CLOSE );
            if( gDirty ) // text updated since last saved
            {
                if( *gFilename != 0 )
                    EnableItem( gFileMenu, SAVE );
            }
            else
                DisableItem( gFileMenu, SAVE );
            EnableItem( gFileMenu, SAVEAS );
            EnableItem( gFileMenu, PAGESETUP );
            EnableItem( gFileMenu, PRINT );

            DisableItem( gEditMenu, UNDO );
            if((**gTEH).selStart==(**gTEH).selEnd)
            {   // no text selected
                DisableItem( gEditMenu, CUT );
                DisableItem( gEditMenu, COPY );
                if(TEGetScrapLen() > 0)
                    EnableItem( gEditMenu, PASTE );
                else
                    DisableItem( gEditMenu, PASTE );
                DisableItem( gEditMenu, CLEAR );
                if((**gTEH).teLength > 0)
                    EnableItem( gEditMenu, SELECTALL );
                else
                    DisableItem( gEditMenu, SELECTALL );
            }
            else
            {   // some text selected
                EnableItem( gEditMenu, CUT );
                EnableItem( gEditMenu, COPY );
                if(TEGetScrapLen() > 0)
                    EnableItem( gEditMenu, PASTE );
                else
                    DisableItem( gEditMenu, PASTE );
                EnableItem( gEditMenu, CLEAR );
                if((**gTEH).teLength > 0)
                    EnableItem( gEditMenu, SELECTALL );
                else
                    DisableItem( gEditMenu, SELECTALL );
            }
        }
    }
}


/********** IsDAWindow */

int IsDAWindow_( WindowPtr whichWindow )
{
    if ( whichWindow == NIL_PTR )
        return( FALSE );
    else /* DA windows have negative windowKinds */
        return( ( (WindowPeek)whichWindow )->windowKind < 0 );
}


/********** HandleMenuChoice */

void HandleMenuChoice_( long int menuChoice )
{
    int    theMenu;
    int    theItem;
    
    if ( menuChoice != 0 )
    {
        theMenu = HiWord( menuChoice );
        theItem = LoWord( menuChoice );
        switch ( theMenu )
        {
            case APPLE_MENU :
                HandleAppleChoice_( theItem );
                break;
            case FILE_MENU :
                HandleFileChoice_( theItem );
                break;
            case EDIT_MENU :
                HandleEditChoice_( theItem );
                break;
            case kHMHelpMenuID :
                MyHelpDialog();
                break;
        }
        HiliteMenu( 0 );
    }
}


/********** HandleAppleChoice */

void HandleAppleChoice_( int theItem )
{
    Str255  accName;
    int     accNumber;
    
    switch ( theItem )
    {
        case ABOUT : 
            MyAboutDialog();
            break;
        default :
            GetItem( gAppleMenu, theItem, accName );
            accNumber = OpenDeskAcc( accName );
            break;
    }
}


/********** HandleFileChoice */

void HandleFileChoice_( int theItem )
{
    WindowPtr         whichWindow;
    struct WindowData *wd;
    Rect              tempViewRect;
    Rect              tempDestRect;
    Str255            fn;

    switch ( theItem )
    {
        case NEW:
            MyCreateWindow( BASE_RES, NIL_PTR, (WindowPtr)MOVE_TO_FRONT,
                WIND_TOP, WIND_LEFT, WIND_OFFSET, N0_RESOURCE );
            break;
        case OPEN:
            whichWindow = gWindow;
            MyCreateHiddenWindow( BASE_RES, NIL_PTR,
                (WindowPtr)MOVE_TO_FRONT, WIND_TOP, WIND_LEFT,
                WIND_OFFSET, N0_RESOURCE );
            if(FnIO_OpenTextFile(&gTEH,fn,&gVRef,&gRefNum))
            {
                SetWTitle(gWindow,fn);
                wd = (struct WindowData *)GetWRefCon(gWindow);
                wd->wVRef = gVRef;
                wd->wRefNum = gRefNum;
                wd->wSaved = TRUE;
                FnIO_pStrCopy( fn, wd->wFilename );
                MySetGlobals( gWindow );
                TECalText( gTEH );
                FnTE_UpdateWindow( gWindow, &gTEH, TRUE );
                FnTE_DetSBarIncr( &gTEH, &gSBH );
                
                BringToFront( gWindow );
                ShowWindow( gWindow );
                DrawControls( gWindow );
            }
            else
            {
                wd = (struct WindowData *)GetWRefCon(gWindow);
                TEDispose( gTEH );
                DisposeHandle((Handle)wd->wTEH);
                free(wd);
                DisposeWindow(gWindow);
                MySetGlobals( whichWindow );
            }
            break;
        case CLOSE:
            whichWindow = FrontWindow();
            if((gWindow == whichWindow) && (whichWindow != NIL_PTR))
            {
                wd = (struct WindowData *)GetWRefCon(gWindow);
                TEDispose( gTEH );
                DisposeHandle((Handle)wd->wTEH);
                free(wd);
                DisposeWindow(gWindow);
                gWindow = FrontWindow();
                if( gWindow != NIL_PTR )
                {
                    MySetGlobals( whichWindow );
                }
            }
            break;
        case SAVE:
            wd = (struct WindowData *)GetWRefCon(gWindow);
            if( gSaved )
            { // save with existing filename
                if( FnIO_SaveTextFile(&(wd->wTEH),wd->wFilename,
                    &(wd->wVRef),&(wd->wRefNum) ) )
                {
                    wd->wDirty = FALSE;
                    wd->wSaved = TRUE;
                    MySetGlobals(gWindow);
                }
            }
            else
            { // saveas
                if( FnIO_SaveAsTextFile(&(wd->wTEH),wd->wFilename,
                    &(wd->wVRef),&(wd->wRefNum) ) )
                {
                    wd->wDirty = FALSE;
                    wd->wSaved = TRUE;
                    SetWTitle( gWindow, wd->wFilename );
                    MySetGlobals(gWindow);
                }
            }
            break;
        case SAVEAS:
            wd = (struct WindowData *)GetWRefCon(gWindow);
            if( FnIO_SaveAsTextFile(&(wd->wTEH),wd->wFilename,
                &(wd->wVRef),&(wd->wRefNum) ) )
            {
                wd->wDirty = FALSE;
                wd->wSaved = TRUE;
                SetWTitle( gWindow, wd->wFilename );
                MySetGlobals(gWindow);
            }
            break;
        case PAGESETUP:
            wd = (struct WindowData *)GetWRefCon(gWindow);
            FnIO_PageSetup(&(wd->wPrintH));
            MySetGlobals(gWindow);
            break;
        case PRINT:
            FnIO_PrintTERecord(&gTEH,&gPrintH);
            break;
        case QUIT:
            gDone = TRUE;
            break;
    }
}


/********** HandleEditChoice */

void HandleEditChoice_( int theItem )
{
    struct WindowData *wd;

    if( SystemEdit( theItem - 1 ) == 0 )
    {
        FnTE_DoEditMenu(
            theItem,
            &gTEH,
            &gSBH,
            (char *)&gDirty,
            CUT,
            COPY,
            PASTE,
            CLEAR,
            SELECTALL );
        wd = (struct WindowData *)GetWRefCon(gWindow);
        wd->wDirty = gDirty;
    }
}


/********** MySetGlobals */

void MySetGlobals( WindowPtr w )
{
    struct WindowData *wd;

    wd = (struct WindowData *)GetWRefCon(w);

    gWindow = w;
    gTEH = wd->wTEH;
    gSBH = wd->wSBH;
    gDirty = wd->wDirty;
    gSaved = wd->wSaved;
    FnIO_pStrCopy( wd->wFilename, gFilename );
    gVRef = wd->wVRef;
    gRefNum = wd->wRefNum;
    gPrintH = wd->wPrintH;
}

// End of File