/* About_ARTAbrot */

/* File name: About_ARTAbrot.h */
/* Function: Handle a modeless dialog */
/*            This dialog operates like a window, it is not modal. */
/*           Opened by:     */
/*           Closed by:      */
/*          Purpose:       
History: 8/18/93 Original by George Warner
   */



/* ======================================================= */
/* Initialize the flag for this dialog, for other routines to use */
void Init_About_ARTAbrot(void);

/* We were moved on the screen, or to another screen. */
void Moved_About_ARTAbrot(WindowPtr theWindow);

/* Update the display of this dialog, when an area is uncovered. */
void Update_About_ARTAbrot(WindowPtr theWindow);

/* Open the dialog and display it on the screen. */
void Open_About_ARTAbrot(void);

/* Handle any events for this dialog, when it is active. */
void Do_About_ARTAbrot(EventRecord *theEvent, WindowPtr theWindow, short itemHit);

/* Close this dialog and release it from memory. */
void Close_About_ARTAbrot(WindowPtr theWindow);

/* ======================================================= */


