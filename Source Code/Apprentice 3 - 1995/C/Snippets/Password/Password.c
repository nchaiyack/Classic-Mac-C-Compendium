/****************************************************************/
/*																*/
/* Password.c													*/
/*																*/
/****************************************************************/

#include "Password.h"

/****************************************************************/
/*																*/
/* SetPassword													*/
/*																*/
/* <-> password			On entry, contains the old password		*/
/*						which is required to change the 		*/
/*						password, or an empty string.			*/
/*						On exit, contains the new password		*/
/*						if the function returns 1.				*/
/*																*/
/* Returns:  0 User pressed Cancel.								*/
/*			 1 The password is successfully changed.			*/
/*			-1 The old password was not verified correctly.		*/
/*			-2 The new password was not typed the same twice.	*/
/*																*/
/****************************************************************/

pascal short SetPassword(Str32 password)
{
	Str255			oldpw;
	Str255			newpw1;
	Str255			newpw2;
	WindowPtr		theWindow;
	Rect			theRect;
	TEHandle		oldpwTEa;
	TEHandle		oldpwTEb;
	TEHandle		newpwTE1a;
	TEHandle		newpwTE1b;
	TEHandle		newpwTE2a;
	TEHandle		newpwTE2b;
	TEHandle		activeTEa;
	TEHandle		activeTEb;
	short			passwordFont;
	char			c;
	EventRecord		theEvent;
	WindowPtr		whichWindow;
	GrafPort		tempPort;
	GrafPtr			remPort;
	short			whichPart;
	ControlHandle	okButton;
	ControlHandle	cancelButton;
	ControlHandle	whichControl;
	short			dialogResult = 0;
	short			theResult = 0;
	
	/* calcualte sign on window size and location */
	GetPort(&remPort);
	OpenPort(&tempPort);
	theRect.left = ((tempPort.portRect.right - tempPort.portRect.left) / 2) - 145;
	theRect.right = theRect.left + 290;
	theRect.top = ((tempPort.portRect.bottom - tempPort.portRect.top) / 2) - 85;
	theRect.bottom = theRect.top + 170;
	ClosePort(&tempPort);
	
	/* create new window and prepare to draw there*/
	theWindow = NewWindow (NULL, &theRect, "\p", TRUE, dBoxProc, (WindowPtr)-1, FALSE, 0);
	SetPort(theWindow);

	/* set up ok button */
	theRect.top = 138;
	theRect.bottom = 158;
	theRect.left = 164;
	theRect.right = 222;
	okButton = NewControl(theWindow, &theRect, "\pOK", TRUE, 0, 0, 0, pushButProc, 0);

	/* set up cancel button */
	theRect.top = 138;
	theRect.bottom = 158;
	theRect.left = 57;
	theRect.right = 115;
	cancelButton = NewControl(theWindow, &theRect, "\pCancel", TRUE, 0, 0, 0, pushButProc, 0);

	/* set up old password textedit */
	if (password[0] == 0) {
		oldpwTEa = NULL; 
		oldpwTEb = NULL;
	}
	else {
		/* visible */
		theRect.top = 35;
		theRect.bottom = 51;
		theRect.left = 160;
		theRect.right = 270;
		oldpwTEa = TENew(&theRect, &theRect);
		(**oldpwTEa).txFont = systemFont;
		(**oldpwTEa).txSize = 12;

		/* hidden */
		theRect.top = 35;
		theRect.bottom = 51;
		theRect.left = 460;
		theRect.right = 270;
		oldpwTEb = TENew(&theRect, &theRect);
		(**oldpwTEb).txFont = systemFont;
		(**oldpwTEb).txSize = 12;
	}
	
	/* set up new "visible" password textedit */
	theRect.top = 66;
	theRect.bottom = 82;
	theRect.left = 160;
	theRect.right = 270;
	newpwTE1a = TENew(&theRect, &theRect);
	(**newpwTE1a).txFont = systemFont;
	(**newpwTE1a).txSize = 12;

	/* set up new "hidden" password textedit */
	theRect.top = 66;
	theRect.bottom = 82;
	theRect.left = 460;
	theRect.right = 270;
	newpwTE1b = TENew(&theRect, &theRect);
	(**newpwTE1b).txFont = systemFont;
	(**newpwTE1b).txSize = 12;

	/* set up new "visible" password textedit */
	theRect.top = 97;
	theRect.bottom = 113;
	theRect.left = 160;
	theRect.right = 270;
	newpwTE2a = TENew(&theRect, &theRect);
	(**newpwTE2a).txFont = systemFont;
	(**newpwTE2a).txSize = 12;

	/* set up new "hidden" password textedit */
	theRect.top = 97;
	theRect.bottom = 113;
	theRect.left = 460;
	theRect.right = 270;
	newpwTE2b = TENew(&theRect, &theRect);
	(**newpwTE2b).txFont = systemFont;
	(**newpwTE2b).txSize = 12;

	/* activate appropriate textedit */
	if (oldpwTEa != NULL) {
		activeTEa = oldpwTEa;
		activeTEb = oldpwTEb;
	}
	else {
		activeTEa = newpwTE1a;
		activeTEb = newpwTE1b;
	}
	TESetSelect(0, 0, activeTEa);
	TEActivate(activeTEa);

	do {
		TEIdle(activeTEa);
		if (WaitNextEvent(everyEvent, &theEvent, 0xFFFFFFFF, 0)) {
			switch (theEvent.what) {
				case mouseDown:
					switch (FindWindow(theEvent.where, &whichWindow)) {
						case inContent:
							if (whichWindow != theWindow) break;
							GlobalToLocal(&(theEvent.where));
							switch (FindControl(theEvent.where, theWindow, &whichControl)) {
								case inButton:
									if (TrackControl(whichControl, theEvent.where, NULL)) {
										if (whichControl == okButton)
											dialogResult = 1;
										else if (whichControl == cancelButton)
											dialogResult = 2;
									}
									break;
								default:
									if ((oldpwTEa != NULL) && (PtInRect(theEvent.where, &((**oldpwTEa).viewRect)))) {
										if (activeTEa != oldpwTEa) {
											TEDeactivate(activeTEa);
											activeTEa = oldpwTEa;
											activeTEb = oldpwTEb;
											TEActivate(activeTEa);
										}
										TEClick(theEvent.where, (theEvent.modifiers & shiftKey) == shiftKey, activeTEa);
										(**activeTEb).selStart = (**activeTEa).selStart;
										(**activeTEb).selEnd = (**activeTEa).selEnd;
									}
									else if (PtInRect(theEvent.where, &((**newpwTE1a).viewRect))) {
										if (activeTEa != newpwTE1a) {
											TEDeactivate(activeTEa);
											activeTEa = newpwTE1a;
											activeTEb = newpwTE1b;
											TEActivate(activeTEa);
										}
										TEClick(theEvent.where, (theEvent.modifiers & shiftKey) == shiftKey, activeTEa);
										(**activeTEb).selStart = (**activeTEa).selStart;
										(**activeTEb).selEnd = (**activeTEa).selEnd;
									}
									else if (PtInRect(theEvent.where, &((**newpwTE2a).viewRect))) {
										if (activeTEa != newpwTE2a) {
											TEDeactivate(activeTEa);
											activeTEa = newpwTE2a;
											activeTEb = newpwTE2b;
											TEActivate(activeTEa);
										}
										TEClick(theEvent.where, (theEvent.modifiers & shiftKey) == shiftKey, activeTEa);
										(**activeTEb).selStart = (**activeTEa).selStart;
										(**activeTEb).selEnd = (**activeTEa).selEnd;
									}
									break;
							}
							break;
						default:
							SysBeep(1);
							break;
					}
					break;
				
				case keyDown:
				case autoKey:
					c = (theEvent.message & charCodeMask);
					switch (c) {
						case 0x03:	/* enter */
						case 0x0D:	/* return */
							dialogResult = 1;
							break;
							
						case 0x09:	/* tab */
							TEDeactivate(activeTEa);
							if (activeTEa == newpwTE1a) {
								activeTEa = newpwTE2a;
								activeTEb = newpwTE2b;
							}
							else if ((activeTEa == oldpwTEa) | (oldpwTEa == NULL)) {
								activeTEa = newpwTE1a;
								activeTEb = newpwTE1b;
							}
							else {
								activeTEa = oldpwTEa;
								activeTEb = oldpwTEb;
							}				
							TESetSelect(0, 32000, activeTEa);
							TESetSelect(0, 32000, activeTEb);
							TEActivate(activeTEa);
							break;
							
						case 0x08:	/* backspace */
							TEKey(c, activeTEa);
							TEKey(c, activeTEb);
							break;
							
						default:
							TEKey('¥', activeTEa);
							TEKey(c, activeTEb);
							break;
					}
					break;
					
				case updateEvt:
					if ((WindowPtr)(theEvent.message) != theWindow) break;
					
					BeginUpdate(theWindow);
					
					EraseRect(&(theWindow->portRect));
			
					TextFont(systemFont);
					MoveTo(((290 - StringWidth("\pSet Password")) / 2),15);
					DrawString("\pSet Password");
		
					/* label the fields */
					if (oldpwTEa != NULL) {
						MoveTo(5,48);
						DrawString("\pOld password:");
					}
					MoveTo(5,79);
					DrawString("\pNew password:");
					MoveTo(5,110);
					DrawString("\pVerify new password:");
					
					/* draw and frame the fields */
					if (oldpwTEa != NULL) {
						TEUpdate(&(theWindow->portRect), oldpwTEa);
						theRect = (**oldpwTEa).viewRect;
						InsetRect(&theRect, -3, -3);
						FrameRect(&theRect);
					}
					TEUpdate(&(theWindow->portRect), newpwTE1a);
					theRect = (**newpwTE1a).viewRect;
					InsetRect(&theRect, -3, -3);
					FrameRect(&theRect);
					TEUpdate(&(theWindow->portRect), newpwTE2a);
					theRect = (**newpwTE2a).viewRect;
					InsetRect(&theRect, -3, -3);
					FrameRect(&theRect);

					DrawControls(theWindow);
					
					/* frame deafult button */
					PenSize(3,3);
					theRect = (**okButton).contrlRect;
					InsetRect(&theRect, -4, -4);
					FrameRoundRect(&theRect, 16, 16);
					PenSize(1, 1);

					EndUpdate(theWindow);
					
					break;
					
			} /* switch */
		} /* WaitNextEvent */
	} while (dialogResult == 0);

	switch (dialogResult) {
		case 1: /* ok button */
		
			/* verify old password */
			if (oldpwTEa != NULL) {
				GetIText((Handle)(**oldpwTEb).hText, oldpw);
				if ((EqualString (oldpw, password, FALSE, TRUE)) == FALSE) {
					theResult = -1;
					break;
				}
			}
			
			/* get the new password */
			GetIText((Handle)(**newpwTE1b).hText, newpw1);
			GetIText((Handle)(**newpwTE2b).hText, newpw2);
			if ((EqualString (newpw1, newpw2, FALSE, TRUE)) == FALSE) {
				theResult = -2;
				break;
			}
			BlockMove(newpw1, password, 33);
			if (password[0] > 32)
				password[0] = 32;

			theResult = 1;
			break;
			
		case 2: /* cancel button */
			break;
	}
	
	DisposeControl(okButton);
	DisposeControl(cancelButton);
	if (oldpwTEa != NULL) {
		TEDispose(oldpwTEa);
		TEDispose(oldpwTEb);
	}
	TEDispose(newpwTE1a);
	TEDispose(newpwTE1b);
	TEDispose(newpwTE2a);
	TEDispose(newpwTE2b);
	DisposeWindow(theWindow);
	SetPort(remPort);
	
	return (theResult);

} /* SetPassword */


/****************************************************************/
/*																*/
/* AskPassword													*/
/*																*/
/*  -> password			Contains the password.					*/
/*																*/
/* Returns:  0 User pressed Cancel.								*/
/*			 1 The password is successfully entered.			*/
/*			-1 The old password was not valid.					*/
/*																*/
/****************************************************************/

pascal short AskPassword(Str32 password)
{
	Str255			pw;
	WindowPtr		theWindow;
	Rect			theRect;
	TEHandle		activeTEa;
	TEHandle		activeTEb;
	short			passwordFont;
	char			c;
	EventRecord		theEvent;
	WindowPtr		whichWindow;
	GrafPort		tempPort;
	GrafPtr			remPort;
	short			whichPart;
	ControlHandle	okButton;
	ControlHandle	cancelButton;
	ControlHandle	whichControl;
	short			dialogResult = 0;
	short			theResult = 0;
	
	/* calcualte sign on window size and location */
	GetPort(&remPort);
	OpenPort(&tempPort);
	theRect.left = ((tempPort.portRect.right - tempPort.portRect.left) / 2) - 118;
	theRect.right = theRect.left + 236;
	theRect.top = ((tempPort.portRect.bottom - tempPort.portRect.top) / 2) - 53;
	theRect.bottom = theRect.top + 106;
	ClosePort(&tempPort);
	
	/* create new window and prepare to draw there*/
	theWindow = NewWindow (NULL, &theRect, "\p", TRUE, dBoxProc, (WindowPtr)-1, FALSE, 0);
	SetPort(theWindow);

	/* set up ok button */
	theRect.top = 72;
	theRect.bottom = 92;
	theRect.left = 134;
	theRect.right = 192;
	okButton = NewControl(theWindow, &theRect, "\pOK", TRUE, 0, 0, 0, pushButProc, 0);

	/* set up cancel button */
	theRect.top = 72;
	theRect.bottom = 92;
	theRect.left = 32;
	theRect.right = 90;
	cancelButton = NewControl(theWindow, &theRect, "\pCancel", TRUE, 0, 0, 0, pushButProc, 0);

	/* set up "visible" password textedit */
	theRect.top = 35;
	theRect.bottom = 51;
	theRect.left = 105;
	theRect.right = 215;
	activeTEa = TENew(&theRect, &theRect);
	(**activeTEa).txFont = systemFont;
	(**activeTEa).txSize = 12;

	/* set up "hidden" password textedit */
	theRect.top = 35;
	theRect.bottom = 51;
	theRect.left = 305;
	theRect.right = 215;
	activeTEb = TENew(&theRect, &theRect);
	(**activeTEb).txFont = systemFont;
	(**activeTEb).txSize = 12;
	
	/* activate appropriate textedit */
	TESetSelect(0, 0, activeTEa);
	TESetSelect(0, 0, activeTEb);
	TEActivate(activeTEa);

	do {
		TEIdle(activeTEa);
		if (WaitNextEvent(everyEvent, &theEvent, 0xFFFFFFFF, 0)) {
			switch (theEvent.what) {
				case mouseDown:
					switch (FindWindow(theEvent.where, &whichWindow)) {
						case inContent:
							if (whichWindow != theWindow) break;
							GlobalToLocal(&(theEvent.where));
							switch (FindControl(theEvent.where, theWindow, &whichControl)) {
								case inButton:
									if (TrackControl(whichControl, theEvent.where, NULL)) {
										if (whichControl == okButton)
											dialogResult = 1;
										else if (whichControl == cancelButton)
											dialogResult = 2;
									}
									break;
								default:
									if (PtInRect(theEvent.where, &((**activeTEa).viewRect))) {
										TEClick(theEvent.where, (theEvent.modifiers & shiftKey) == shiftKey, activeTEa);
										(**activeTEb).selStart = (**activeTEa).selStart;
										(**activeTEb).selEnd = (**activeTEa).selEnd;
									}
									break;
							}
							break;
						default:
							SysBeep(1);
							break;
					}
					break;
				
				case keyDown:
				case autoKey:
					c = (theEvent.message & charCodeMask);
					switch (c) {
						case 0x03:	/* enter */
						case 0x0D:	/* return */
							dialogResult = 1;
							break;
							
						case 0x09:	/* tab */
							break;
							
						case 0x08:	/* backspace */
							TEKey(c, activeTEa);
							TEKey(c, activeTEb);
							break;
							
						default:
							TEKey('¥', activeTEa);
							TEKey(c, activeTEb);
							break;
					}
					break;
					
				case updateEvt:
					if ((WindowPtr)(theEvent.message) != theWindow) break;

					BeginUpdate(theWindow);
					
					EraseRect(&(theWindow->portRect));
			
					TextFont(systemFont);
					MoveTo(((236 - StringWidth("\pEnter Password")) / 2),15);
					DrawString("\pEnter Password");
		
					/* label the fields */
					MoveTo(10,48);
					DrawString("\pPassword:");
					
					/* draw and frame the fields */
					TEUpdate(&(theWindow->portRect), activeTEa);
					theRect = (**activeTEa).viewRect;
					InsetRect(&theRect, -3, -3);
					FrameRect(&theRect);

					DrawControls(theWindow);
					
					/* frame deafult button */
					PenSize(3,3);
					theRect = (**okButton).contrlRect;
					InsetRect(&theRect, -4, -4);
					FrameRoundRect(&theRect, 16, 16);
					PenSize(1, 1);

					EndUpdate(theWindow);
					
					break;
					
			} /* switch */
		} /* WaitNextEvent */
	} while (dialogResult == 0);

	switch (dialogResult) {
		case 1: /* ok button */
		
			/* verify password */
			GetIText((Handle)(**activeTEb).hText, pw);
			if ((EqualString (pw, password, FALSE, TRUE)) == FALSE) {
				theResult = -1;
				break;
			}
			
			theResult = 1;
			break;
			
		case 2: /* cancel button */
			break;
	}
	
	DisposeControl(okButton);
	DisposeControl(cancelButton);
	TEDispose(activeTEa);
	TEDispose(activeTEb);
	DisposeWindow(theWindow);
	SetPort(remPort);

	return (theResult);

} /* AskPassword */
