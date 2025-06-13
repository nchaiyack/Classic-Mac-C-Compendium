/**********************************************************************

    Segment: ArgusAbout.c

***********************************************************************/

/*
    Standard Argus Software About dialog box.
*/

/********** Includes */
#include <Dialogs.h>
#include "Fn_Prototypes.h"

/********** Defines */
#define ABOUT_DLOG     599
#define NIL_PTR        0L
#define ALLOCATE_MEM   0
#define IN_FRONT       (WindowPtr)-1L

#define OK_BUTTON      1
#define HELP_BUTTON    2


/********** Prototypes */
Boolean MyAboutDialog( void );

/********** SampleDialog */

Boolean MyAboutDialog( void )
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

    result = FALSE;

    docWindow = FrontWindow();
    if( docWindow != NIL_PTR )
        MyDoDeactivateWindow( docWindow );

    dialog = GetNewDialog( ABOUT_DLOG, ALLOCATE_MEM, IN_FRONT );

    if( dialog == NIL_PTR )
        return( result );

    /* AdjustMenus_(); */
    ShowWindow( dialog );
    FnMisc_FrameButton( dialog, OK_BUTTON );

    done = FALSE;
    while( done == FALSE )
    {
        ModalDialog( nil, &itemHit );

        switch( itemHit )
        {
            case OK_BUTTON:
                result = TRUE;
                done = TRUE;
                break;
            case HELP_BUTTON:
                DisposeDialog( dialog );
                MyHelpDialog();
                done = TRUE;
                return( result );
                break;
        }
     }
    DisposDialog( dialog );
    return( result );
}

// End of File