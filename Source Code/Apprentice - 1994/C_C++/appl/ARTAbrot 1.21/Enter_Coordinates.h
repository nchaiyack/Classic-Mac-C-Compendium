/* Enter_Coordinates */

/* File name: Enter_Coordinat.h */
/* Function: Handle a modeless dialog */
/*            This dialog operates like a window, it is not modal. */
/*           Opened by:     */
/*           Closed by:      */
/*          Purpose:       
History: 8/18/93 Original by George Warner
   */



/* ======================================================= */
/* Initialize the flag for this dialog, for other routines to use */
void Init_Enter_Coordinat(void);

/* We were moved on the screen, or to another screen. */
void Moved_Enter_Coordinat(WindowPtr theWindow);

/* Update the display of this dialog, when an area is uncovered. */
void Update_Enter_Coordinat(WindowPtr theWindow);

/* Open the dialog and display it on the screen. */
void Open_Enter_Coordinat(void);

/* Handle any events for this dialog, when it is active. */
void Do_Enter_Coordinat(EventRecord *theEvent, WindowPtr theWindow, short itemHit);

/* Close this dialog and release it from memory. */
void Close_Enter_Coordinat(WindowPtr theWindow);

/* ======================================================= */


