/**********************************************************************

    Segment: SampleDialog.c

***********************************************************************/

/*
    A sample case demonstrating a movable model dialog box.  Need 
    System 7 for pop-up menu functionality.
*/

/********** Includes */
#include <Dialogs.h>
#include "Fn_Prototypes.h"

/********** Defines */
#define SAMPLE_DLOG    500
#define NIL_PTR        0L
#define ALLOCATE_MEM   0
#define IN_FRONT       (WindowPtr)-1L
#define RETURN_KEY     13
#define ENTER_KEY      3
#define ESCAPE_KEY     27
#define PERIOD_KEY     46
#define VISUAL_DELAY   8  // standard is 8 ticks

#define OK_BUTTON      1
#define CANCEL_BUTTON  2
#define NAME           5
#define PHONE          6
#define MALE_RADIO     7
#define FEMALE_RADIO   8
#define MARRIED_BOX    10
#define DB             11


/********** Prototypes */
Boolean        MySampleDialog(  void );
pascal Boolean MyEventFilter(   DialogPtr    theDialog,
                                EventRecord  *theEvent,
                                short        *itemHit );

/********** SampleDialog */

Boolean MySampleDialog( void )
{
    WindowPtr   docWindow;
    DialogPtr   dialog;
    Boolean     done;
    Boolean     result;
    short       itemHit;
    short       itemType;
    Handle      itemHandle;
    Rect        itemRect;
    EventRecord theEvent;
    
    Boolean     isMale;
    Boolean     isFemale;
    Boolean     isMarried;
    int         phoneDB;
    Str255      name;
    Str255      phone;

    result = FALSE;

    docWindow = FrontWindow();
    if( docWindow != NIL_PTR )
        MyDoDeactivateWindow( docWindow );

    dialog = GetNewDialog( SAMPLE_DLOG, ALLOCATE_MEM, IN_FRONT );

    if( dialog == NIL_PTR )
        return( result );

    /* AdjustMenus_(); */
    ShowWindow( dialog );
    FnMisc_FrameButton( dialog, OK_BUTTON );

    done = FALSE;
    while( done == FALSE )
    {
        ModalDialog( &MyEventFilter, &itemHit );

        switch( itemHit )
        {
            case OK_BUTTON:
                result = TRUE;
                done = TRUE;
                break;
            case CANCEL_BUTTON:
                done = TRUE;
                break;
            case MALE_RADIO:
                GetDItem( dialog,
                          MALE_RADIO,
                          &itemType,
                          &itemHandle,
                          &itemRect );
                SetCtlValue( (ControlHandle)itemHandle, TRUE );
                GetDItem( dialog,
                          FEMALE_RADIO,
                          &itemType,
                          &itemHandle,
                          &itemRect );
                SetCtlValue( (ControlHandle)itemHandle, FALSE );
                break;
            case FEMALE_RADIO:
                GetDItem( dialog,
                          FEMALE_RADIO,
                          &itemType,
                          &itemHandle,
                          &itemRect );
                SetCtlValue( (ControlHandle)itemHandle, TRUE );
                GetDItem( dialog,
                          MALE_RADIO,
                          &itemType,
                          &itemHandle,
                          &itemRect );
                SetCtlValue( (ControlHandle)itemHandle, FALSE );
                break;
            case MARRIED_BOX:
                GetDItem( dialog,
                          MARRIED_BOX,
                          &itemType,
                          &itemHandle,
                          &itemRect );
                if( GetCtlValue( (ControlHandle)itemHandle ) )
                    SetCtlValue( (ControlHandle)itemHandle, FALSE );
                else
                    SetCtlValue( (ControlHandle)itemHandle, TRUE );
                break;
        }
     }

    GetDItem(dialog,MALE_RADIO,&itemType,&itemHandle,&itemRect);
    isMale = GetCtlValue((ControlHandle)itemHandle);
    GetDItem(dialog,FEMALE_RADIO,&itemType,&itemHandle,&itemRect);
    isFemale = GetCtlValue((ControlHandle)itemHandle);
    GetDItem(dialog,MARRIED_BOX,&itemType,&itemHandle,&itemRect);
    isMarried = GetCtlValue((ControlHandle)itemHandle);

    GetDItem(dialog,NAME,&itemType,&itemHandle,&itemRect);
    GetIText(itemHandle,name);
    GetDItem(dialog,PHONE,&itemType,&itemHandle,&itemRect);
    GetIText(itemHandle,phone);

    GetDItem(dialog,DB,&itemType,&itemHandle,&itemRect);
    phoneDB = GetCtlValue((ControlHandle)itemHandle);

    DisposDialog( dialog );

    return( result );
}


/********** PASCAL MyEventFilter */

pascal Boolean MyEventFilter( DialogPtr    theDialog,
                              EventRecord  *theEvent,
                              short        *itemHit )
{
    short          thePart;
    char           key;
    short          itemType;
    Handle         itemHandle;
    Rect           itemRect;
    long           finalTicks;
    Rect           dragRect;
    Boolean        result;
    WindowPtr      theWindow;
    
    result = FALSE;
    dragRect = screenBits.bounds;

        switch( (*theEvent).what )
        {
            case mouseDown:
                thePart = FindWindow( (*theEvent).where, &theWindow );
                if( theWindow == theDialog )
                {
                switch( thePart )
                    {
                        case inDrag:
                            DragWindow( theDialog,
                                        (*theEvent).where,
                                        &dragRect );
                            result = TRUE;
                            break;
                    }
                }
                break;
            case keyDown:
            case autoKey:
                key = (*theEvent).message & charCodeMask;
                if( (key == RETURN_KEY) || (key == ENTER_KEY) )
                {
                    *itemHit = OK_BUTTON;
                    GetDItem( theDialog,
                              OK_BUTTON,
                              &itemType,
                              &itemHandle,
                              &itemRect );
                    HiliteControl( (ControlHandle)itemHandle,
                                   inButton );
                    Delay( VISUAL_DELAY, &finalTicks );
                    HiliteControl( (ControlHandle)itemHandle, 0 );
                    result = TRUE;
                }
                if( (key == ESCAPE_KEY) ||
                    (((*theEvent).modifiers & cmdKey) &&
                    (key == PERIOD_KEY)) )
                {
                    *itemHit = CANCEL_BUTTON;
                    GetDItem( theDialog,
                              CANCEL_BUTTON,
                              &itemType,
                              &itemHandle,
                              &itemRect );
                    HiliteControl( (ControlHandle)itemHandle,
                                   inButton );
                    Delay( VISUAL_DELAY, &finalTicks );
                    HiliteControl( (ControlHandle)itemHandle, 0 );
                    result = TRUE;
                }
                /* Handle other keyboard equivalents here */
                break;
            case updateEvt:
                if( (WindowPtr)(*theEvent).message != theDialog )
                {
                    MyDoUpdateWindow( (WindowPtr)(*theEvent).message );
                }
                else
                {
                    FnMisc_FrameButton( theDialog, OK_BUTTON );
                }
                break;
            case activateEvt:
                if( (WindowPtr)(*theEvent).message != theDialog )
                {
                    /*
                    DoActivate_( (WindowPtr)(*theEvent).message,
                                 ((*theEvent).modifiers & activeFlag),
                                 *theEvent );
                    */
                 }
                 break;
        }
        
    return( result );
}

// End of File