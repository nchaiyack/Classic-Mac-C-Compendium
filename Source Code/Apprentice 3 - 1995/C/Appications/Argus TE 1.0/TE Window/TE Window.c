/**********************************************************************

    Segment: TE Window.c

***********************************************************************/

#include <stdlib.h>
#include "Fn_Prototypes.h"

#define DRAG_THRESHOLD 30
#define INSET          3

extern gNewWindowLeft;
extern gNewWindowTop;
extern TEHandle gTEH;
extern ControlHandle gSBH;
extern Boolean gDirty;
extern Boolean gSaved;
extern Str255 gFilename;
extern WindowPtr gWindow;
extern short gVRef;
extern short gRefNum;
extern THPrint gPrintH;

struct WindowData
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

/********** Prototypes */
void MyCreateWindow(       short       winID,
                           Ptr         winRec,
                           WindowPtr   behind,
                           int         top,
                           int         left,
                           int         offset,
                           int         errID );
void MyCreateHiddenWindow( short       winID,
                           Ptr         winRec,
                           WindowPtr   behind,
                           int         top,
                           int         left,
                           int         offset,
                           int         errID );
void MyDoContent(          WindowPtr   w,
                           EventRecord *e );
void MyDoUpdateWindow(     WindowPtr   w );
void MyDoActivateWindow(   WindowPtr   w );
void MyDoDeactivateWindow( WindowPtr   w );
void MyDoGrow(             WindowPtr   w, 
                           Point       clickLoc );

/********** CreateWindow */

void MyCreateWindow( short     winID,
                     Ptr       winRec,
                     WindowPtr behind,
                     int       top,
                     int       left,
                     int       offset,
                     int       errID )
{
    WindowPtr         w;
    ControlHandle     c;
    struct WindowData *wd;
    Str255            wTitle = "\pUntitled";
    Str255            tempFilename = "\p";
    
    if ( ( w = GetNewWindow( winID, winRec, behind ) ) == nil )
        FnErr_DisplayStrID( errID, TRUE );
    if (((screenBits.bounds.right - gNewWindowLeft) < DRAG_THRESHOLD) ||
        ((screenBits.bounds.bottom - gNewWindowTop) < DRAG_THRESHOLD))
    {
        gNewWindowLeft = left;
        gNewWindowTop = top;
    }
    
    MoveWindow( w, gNewWindowLeft, gNewWindowTop, FALSE );
    gNewWindowLeft += offset;
    gNewWindowTop += offset;
    
    FnTE_SetUpTEWindow( w,         // window ptr
                        TRUE,      // hasGrowIcon
                        TRUE,      // wordWrap
                        monaco,    // font
                        9,         // textSize
                        INSET,     // textInsetPixels
                        &gTEH,     // *te (result)
                        &gSBH );   // *vScroll (result)

    // store reference to TE and scroll bar in window record
    wd = (struct WindowData *)malloc(sizeof(struct WindowData));
    wd->wTEH = gTEH;
    wd->wSBH = gSBH;
    wd->wDirty = 0;
    wd->wSaved = 0;
    wd->wVRef = 0;
    wd->wRefNum = 0;
    wd->wPrintH = NULL;
    FnIO_pStrCopy( wTitle, wd->wFilename );
    SetWRefCon(w,(long)wd);
    
    // set global values
    gDirty = 0;
    gSaved = 0;
    FnIO_pStrCopy( tempFilename, gFilename );
    gWindow = w;
    gVRef = 0;
    gRefNum = 0;
    
    SetWTitle(w,wTitle);

    ShowWindow( w );
    DrawControls( w );
}


/********** DoContent */

void MyDoContent( WindowPtr w, EventRecord *e )
{
    struct WindowData *wd;

    wd = (struct WindowData *)GetWRefCon(w);
    FnTE_DoContent(
        w,
        e,
        &(wd->wTEH),
        &(wd->wSBH) );
}


/********** MyDoUpdateWindow */

void MyDoUpdateWindow( WindowPtr w )
{
    struct WindowData *wd;

    wd = (struct WindowData *)GetWRefCon(w);
    FnTE_UpdateWindow( 
        w,
        &(wd->wTEH),
        TRUE );
}


/********** MyDoActivateWindow */

void MyDoActivateWindow( WindowPtr w )
{
    struct WindowData *wd;

    // set globals
    wd = (struct WindowData *)GetWRefCon(w);
    gTEH = wd->wTEH;
    gSBH = wd->wSBH;
    gDirty = wd->wDirty;
    gSaved = wd->wSaved;
    FnIO_pStrCopy( wd->wFilename, gFilename );
    gWindow = w;

    FnTE_DoActivate(
        w,
        &(wd->wTEH),
        &(wd->wSBH),
        TRUE,
        TRUE );
}


/********** MyDoDeactivateWindow */

void MyDoDeactivateWindow( WindowPtr w )
{
    struct WindowData *wd;

    wd = (struct WindowData *)GetWRefCon(w);
    FnTE_DoActivate(
        w,
        &(wd->wTEH),
        &(wd->wSBH), 
        TRUE,
        FALSE );
}


/********** MyDoGrow */

void MyDoGrow( WindowPtr w, Point clickLoc)
{
    struct WindowData *wd;

    wd = (struct WindowData *)GetWRefCon(w);
    FnTE_GrowWindow(
        w,
        &(wd->wTEH),
        &(wd->wSBH), 
        INSET,
        clickLoc );
}


/********** CreateWindow */

void MyCreateHiddenWindow( short     winID,
                          Ptr       winRec,
                          WindowPtr behind,
                          int       top,
                          int       left,
                          int       offset,
                          int       errID )
// Same as CreateWindow except for commented out lines
{
    WindowPtr         w;
    ControlHandle     c;
    struct WindowData *wd;
    Str255            wTitle = "\pUntitled";
    Str255            tempFilename = "\p";
    
    if ( ( w = GetNewWindow( winID, winRec, behind ) ) == nil )
        FnErr_DisplayStrID( errID, TRUE );
    if (((screenBits.bounds.right - gNewWindowLeft) < DRAG_THRESHOLD) ||
        ((screenBits.bounds.bottom - gNewWindowTop) < DRAG_THRESHOLD))
    {
        gNewWindowLeft = left;
        gNewWindowTop = top;
    }
    
    MoveWindow( w, gNewWindowLeft, gNewWindowTop, FALSE );
    gNewWindowLeft += offset;
    gNewWindowTop += offset;
    
    FnTE_SetUpTEWindow( w,         // window ptr
                        TRUE,      // hasGrowIcon
                        TRUE,      // wordWrap
                        monaco,    // font
                        9,         // textSize
                        INSET,     // textInsetPixels
                        &gTEH,     // *te (result)
                        &gSBH );   // *vScroll (result)

    // store reference to TE and scroll bar in window record
    wd = (struct WindowData *)malloc(sizeof(struct WindowData));
    wd->wTEH = gTEH;
    wd->wSBH = gSBH;
    wd->wDirty = 0;
    wd->wSaved = 0;
    wd->wVRef = 0;
    wd->wRefNum = 0;
    wd->wPrintH = NULL;
    FnIO_pStrCopy( wTitle, wd->wFilename );
    SetWRefCon(w,(long)wd);
    
    // set global values
    gDirty = 0;
    gSaved = 0;
    FnIO_pStrCopy( tempFilename, gFilename );
    gWindow = w;
    gVRef = 0;
    gRefNum = 0;
    
    SetWTitle(w,wTitle);

    //ShowWindow( w );
    //DrawControls( w );
}

// End of File