 /*
   GraphicsModule_main.c

   Apple and Think C Implementation of 'ADgm' screen saver code resource.

   This file provides a generic interface for writing a After Darkª graphics module.
   The function "main" is called by After Dark and passed four parameters:

   storage:     A Handle to memory you have allocated.
   blankRgn:    A region covering all screens.
   message:     A value indicating which function to call.
   params:              A pointer to a structure containing useful information.

   To use it, write the five routines:

   OSErr DoInitialize(Handle *storage, RgnHandle blankRgn, GMParamBlockPtr params);
   OSErr DoClose(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
   OSErr DoBlank(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);
   OSErr DoDrawFrame(Handle storage, RgnHandle blankRgn, GMParamBlockPtr params);

   For more information, consult the programmer's section of the manual.

   By Patrick Beard and Bruce Burkhalter and Colin Glassey

   ©1989,1990,1991 Berkeley Systems, Inc.
 */

#include <QuickDraw.h>
#include "AfterDarkTypes.h"
#ifdef THINK_C
/* allows use of Global vars with Think C */
#include <SetUpA4.h>
#define OpenGlobals()   RememberA0(); SetUpA4();
#define CloseGlobals()  RestoreA4();
#endif

/* entry point into graphics module */

pascal OSErr
main(storage, blankRgn, message, params)
Handle *storage;                /* storage allocated by graphics module. */
RgnHandle blankRgn;             /* region to do all drawing in. */
short message;                  /* the action to take */
GMParamBlockPtr params;         /* parameters & services for controlling graphics module */
{
    OSErr err = noErr;

#ifdef THINK_C
    /* set up globals for strings etc. only for THINK C */
    OpenGlobals();
#endif
    /* dispatch message to appropriate routine. */
    switch (message)
    {
    case Initialize:
        err = DoInitialize(storage, blankRgn, params);
        break;
    case Close:
        err = DoClose(*storage, blankRgn, params);
        break;
    case Blank:
        err = DoBlank(*storage, blankRgn, params);
        break;
    case DrawFrame:
        err = DoDrawFrame(*storage, blankRgn, params);
        break;
    case DoAbout:
        err = DoHelp(blankRgn, params);
        break;
        
        /* Not handled by our module.
    case ModuleSelected:
        err = DoSelected(params);
        break;
        */

    default:
        break;
    }                           /* end switch */
#ifdef THINK_C
    CloseGlobals();
#endif
    return err;
}
