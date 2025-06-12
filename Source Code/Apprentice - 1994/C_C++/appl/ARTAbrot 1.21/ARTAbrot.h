/* ======================================================= */
/* Initialize us so all our routines can be activated */
void Init_ARTAbrot(void);

/* Close our window */
void Close_ARTAbrot(WindowPtr whichWindow);

/* Handle resizing scrollbars */
void Resized_ARTAbrot(WindowPtr whichWindow);

/* Our window was moved */
void Moved_ARTAbrot(WindowPtr whichWindow);

/* Update our window, someone uncovered a part of us */
void Update_ARTAbrot(WindowPtr whichWindow);

/* Open our window and draw everything */
void Open_ARTAbrot(void);

/* Handle activation of our window */
void Activate_ARTAbrot(WindowPtr whichWindow, Boolean Do_An_Activate);

/* Handle action to our window, like controls */
void Do_ARTAbrot(EventRecord *myEvent);

/* ======================================================= */


