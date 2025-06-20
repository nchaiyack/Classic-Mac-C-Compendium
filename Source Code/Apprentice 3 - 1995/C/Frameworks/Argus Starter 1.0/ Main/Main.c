/**********************************************************************

    Segment: Main.c

***********************************************************************/

/*
    Starter C Application
    (Written using THINK C 6.0)
    
    Requires following libraries:
        FnAE_Library
        FnIO_Library
        FnTE_Library
        FnMisc_Library
        FnErr_Library
    
    Assumes inclusion of <MacHeaders> file and MacTraps libraries.
    
    DEFINES     All define variables are capitalized per standard
                'C' practice.
                        
    gGlobal     Global variables are preceded with a small 'g'.
                     
    Funct_()    All functions and sub-routines in Main.c are identified
                with an underscore '_' at the end of the function name.
                This hopefully helps to identify program functions 
                from Toolbox calls.

    Fn_Lib()    Functions from external libraries start with "Fn".
                You must make sure all identified libraries are
                included in project file.
                
    MyFunct()   All other functions are identified by "My" at the
                beginning of the function name.

*/


/********** Includes */
/*
    Many of these aren't required for this program, but are used in
    a lot of other programs and are included here in an 'all but the 
    kitchen sink' approach so that nearly everything is available if 
    you need it.  The addition to your code is minimal.
*/
#include <Types.h>
#include <Quickdraw.h>
#include <Controls.h>
#include <Desk.h>
#include <Dialogs.h>
#include <DiskInit.h>
#include <Editions.h>
#include <EPPC.h>
#include <Events.h>        // needed
#include <Fonts.h>
#include <GestaltEqu.h>
#include <Lists.h>
#include <Menus.h>
#include <OSEvents.h>
#include <TextEdit.h>
#include <ToolUtils.h>
#include <Traps.h>
#include <AppleEvents.h>   // needed
#include <Balloons.h>      // needed

#include "Fn_Prototypes.h" // Prototypes for all Fn functions

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
#define HELP_MENU            "\pApplication Help..."

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

/*
    The following define statements cross-reference
    names with resource ID numbers.
*/

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
#define CLOSE          2
#define QUIT           3

/********** Edit Menu Stuff */
#define UNDO           1
#define CUT            3
#define COPY           4
#define PASTE          5
#define CLEAR          6

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
    
    if ( gSysConfig.hasWNE )
        WaitNextEvent( everyEvent, &gTheEvent, MIN_SLEEP,
            NIL_MOUSE_REGION );
    else
    {
        SystemTask();
        GetNextEvent( everyEvent, &gTheEvent );
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
                /* Handle text from keyboard */
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
                    MyDoActivateWindow((WindowPtr)gTheEvent.message);
                }
                else
                {
                    /* Handle deactivate event. */
                    MyDoDeactivateWindow((WindowPtr)gTheEvent.message);
                }
            }
        case osEvt:
            // this application doesn't support operating sys events
            break;
        case nullEvent:
            // ignore
            break;
        case kHighLevelEvent:
            // need to #include <Events.h> to define kHighLevelEvent
            FnAE_DoHighLevelEvent( &gTheEvent );
            break;
    }
}


/********** HandleMouseDown */

void HandleMouseDown_( void )
{
    WindowPtr   whichWindow;
    short int   thePart;
    long int    menuChoice;    
    Point       theLocation;
    
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
                /* Handle click in window content */ 
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
            // not used in this application
            break;
        case inGoAway:
            theLocation = gTheEvent.where;
            GlobalToLocal( &theLocation );
            if( TrackGoAway( whichWindow, theLocation ) )
                DisposeWindow( whichWindow );
            break;
        case inZoomIn:
        case inZoomOut:
            // not used in this application
            break;
    }
}


/********** AdjustMenus */

void AdjustMenus_( void )
{
    WindowPtr currentWindow;

    if (IsDAWindow_( FrontWindow() ) )
    {
        EnableItem( gEditMenu, UNDO );
        EnableItem( gEditMenu, CUT );
        EnableItem( gEditMenu, COPY );
        EnableItem( gEditMenu, PASTE );
        EnableItem( gEditMenu, CLEAR );
    }
    else
    {
        DisableItem( gEditMenu, UNDO );
        DisableItem( gEditMenu, CUT );
        DisableItem( gEditMenu, COPY );
        DisableItem( gEditMenu, PASTE );
        DisableItem( gEditMenu, CLEAR );
    }
        
    if ( ( currentWindow = FrontWindow() ) == NIL_PTR )
        DisableItem( gFileMenu, CLOSE );
    else
        EnableItem( gFileMenu, CLOSE );
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
    WindowPtr   whichWindow;

    switch ( theItem )
    {
        case NEW :
            MyCreateWindow( BASE_RES, NIL_PTR, (WindowPtr)MOVE_TO_FRONT,
                WIND_TOP, WIND_LEFT, WIND_OFFSET, N0_RESOURCE );
            break;
        case CLOSE :
            if ( ( whichWindow = FrontWindow() ) != NIL_PTR )
                DisposeWindow( whichWindow );
            break;
        case QUIT :
            gDone = TRUE;
            break;
    }
}


/********** HandleEditChoice */

void HandleEditChoice_( int theItem )
{
    if( SystemEdit( theItem - 1 ) == 0 )
    {
        /* Add Edit menu switch statement here */
    }
}

// End of File