#include "Neat Stuff.h"
#include "Progress Indicator.h"
#include "Standard Stuff.h"

/*******************************************************************************

	On dialog memory management:

	Normally, there is one and only one copy of each kind of dialog in your
	application. When creating these dialogs, there are three approaches we
	can take. The first is to create these dialogs as needed and to dispose of
	them when we are done. The second is to create all of these dialogs at
	application startup time and keep them around for the life of the program.
	When they aren�t needed, they are simply hidden. One advantage to this is
	that dialogs reappear where the user left them. A third approach lies
	somewhere between the first two approaches, where the dialogs are created
	as needed, but are hidden when no longer needed. We take this third
	approach here.


	On handling movable-modal dialogs:

	Movable-modal dialogs are hybrid windows. They mostly look like modeless
	dialogs in that they have a drag region and title bar, but they also have
	the bold rectangle framing them reminiscent of modal dialogs.

	The purpose of movable-modal dialogs is to allow most of the flexibility
	of modeless dialogs in cases where you would normally display a modal
	dialog. The benefits the user gains are that they can move the dialog
	around so that they can see parts of their other windows, and that they
	can switch to other running applications. At the same time, they are in a
	semi-modal state in that they can�t select any other windows in the
	application, and most menu items are disabled.

	Handling movable-modal dialogs is kind of tricky.  Because the user is
	allowed to switch to other applications, and because the application must
	update background windows it owns as the dialog is moved around, the
	application _cannot_ call ModalDialog on movable-modal windows. This means
	that another approach needs to be taken.

	Fortunately, an alternate solution is not very difficult to implement. The
	first thing to do is define a global variable that gets set to TRUE when a
	movable-modal dialog is shown. In our case, this variable is called
	�gInModalState.� When that variable is set to TRUE, two things happen.
	First, our event handling routine makes a special examination of
	mouse-down events. Only mouse-downs in the movable-modal window and in the
	menubar are allowed. Second, our menu setup routine disables most of the
	menu items.

*******************************************************************************/

/*******************************************************************************

	Global Variables.

*******************************************************************************/

short	gCurrentRadio;				/*	Item number of the currently selected
										radio button. */

Boolean	gCurrentFlippedState;		/*	The current state of our command-key
										hints. In other words, are they
										currently displayed, or hidden. */

Boolean	gPeriodicTask = FALSE;		/*	Set to TRUE when we are performing
										some periodic task every time we go
										through the event loop. */

DialogPtr	gModelessDialog = NIL;
DialogPtr	gMovableModalDialog = NIL;


/*******************************************************************************

	DoDialogHit

	Handles any events on modeless dialogs.

*******************************************************************************/
void	DoDialogHit(DialogPtr dlg, short item)
{
	if (item == ok) {
		HideWindow(dlg);
		if (dlg == gMovableModalDialog)
			gInModalState = FALSE;
		AdjustMenus();
	}
}


/*******************************************************************************

	DoMainLoopTasks

	Do any tasks that need to be performed regularly every time we go through
	the main event loop.

*******************************************************************************/
void	DoMainLoopTasks(void)
{
	if (gPeriodicTask) {
		gPeriodicTask = !SetProgressDelta(1);
		if (!gPeriodicTask) {
			HideWindow(gMovableModalDialog);
			gInModalState = FALSE;
			AdjustMenus();
		}
	}
}


/*******************************************************************************

	DoMusicMaker

	Show the Music Maker modal dialog box. We do a few things here. First, we
	load in the dialog. We have it marked as invisible in the resource so we
	can futz with it a little bit before showing it on the screen. Next, we
	set one of the radio buttons (you always need at least _one_ radio button
	marked). We also fiddle with the items that get flipped when we hold down
	the command key. Since we are just bringing up the dialog, we set those
	items to their �command-key-is-not-down� state.

	Once that is done, we can show the dialog. To do this, we simply use
	ShowWindow. After all, a DialogPtr is the same as a WindowPtr. When the
	window is shown, we call ModalDialog and let the System handle mouse downs
	and keypresses. When something interesting happens, ModalDialog  returns,
	telling us the number of the item that was selected. In other words, if
	the user clicked on the OK button, we receive the ID number of the OK
	button.

	We look at the item number and decide what to do next. If a radio button
	is clicked on, we select it and deselect the previous button. If a
	checkbox is selected, we toggle its setting. If the OK or Cancel buttons
	are hit, we close the dialog box and exit this function.

*******************************************************************************/
void	DoMusicMaker(void)
{
	short		itemHit;
	DialogPtr	ourDialog;

	ourDialog = GetNewDialog(kMusicMakerDialog, NIL, (WindowPtr) -1);

	gCurrentRadio = 0;
	SetRadioButton(ourDialog, kFirstRadio);

	gCurrentFlippedState = TRUE;		/* To force the update in ... */
	FlipItems(ourDialog, FALSE);		/* ... this call to FlipItems. */

	ShowWindow(ourDialog);
	do {
		ModalDialog(OurFilter, &itemHit);
		if ((itemHit >= kFirstRadio) && (itemHit <= kLastRadio))
			SetRadioButton(ourDialog, itemHit);
		else if ((itemHit >= kFirstCheckBox) && (itemHit <= kLastCheckBox))
			ToggleCheckBox(ourDialog, itemHit);
	} while ((itemHit != ok) && (itemHit != cancel));

	DisposeDialog(ourDialog);
}


/*******************************************************************************

	OurFilter

	This routine is passed to ModalDialog as a �filter procedure�. This means
	that it is called in the core loop of ModalDialog to handle any custom
	actions. For instance, in this routine, we handle certain command-key
	combinations, as well as Enter, Return, and Esc key presses. We also
	handle the display of the command-key hints if the command key is down,
	and the drawing of the frames around the OK button and the icon.

	The first thing this routine does is show the command-key hints if the
	command key is down. Since the user can press the command key at any time,
	we check on Null events to see if it is down. If so, we show the hints.
	Otherwise, we remove the hints from the screen. All of this hint showing
	and hiding is handled by calling FlipItems with a Boolean indicating
	whether we want the hints shown or not.

	If the user presses a key, we�d like to take a look at it and see if we
	want to act on it. If the user presses Return or Enter, we simulate a
	click on the OK button. If the user presses Command-Period or Esc, we
	simulate a click on the Cancel button. If the user presses one of the
	special command keys that corresponds to a radio button or checkbox, we
	simulate a click on one of those. All clicks are simulated by setting
	�itemHit� to the ID number of the item we want to click on.

	We also handle update events. This is so we can frame the OK button and
	the icon. We put a frame around the OK button because all good Macintosh
	programs should frame the button that gets selected when the user presses
	Return or Enter. We frame the icon because one of my friends says it looks
	better that way.

	Notice that we frame the OK button a little differently than the way
	Inside Mac says to. On page I-407, it tells us to draw a 3 pixel thick
	roundrect with corner radii of 16 pixels. However, if you�ve ever followed
	that advice for really big buttons, you�ll see that the result looks
	really dumb. This is because the buttons themselves are not always drawn
	with a 16 pixel radius, which means that the button and frame don�t always
	nest correctly. Therefore, we use the same algorithm the Control Manager
	uses to determine the corner radius, which is to take the height of the
	button and divide it by 2. However, we don�t let the value for the radius
	get too small, or else it starts looking stupid again. So we make sure the
	vertical radius is always at least 16 pixels; this seems to look pretty
	good.

	Finally, a note on the Boolean value we have to return for this function.
	This Boolean value is a signal to the Dialog Manager. If we return TRUE,
	we are telling the Dialog Manager that we did everything that needed to be
	done for that particular event, and that it doesn�t need to do anything
	except return to its event loop and call GetNextEvent. However, if we
	return FALSE, we tell the dialog manager to do what it would normally do.
	So in the case of the update event, we _augment_ what is normally done by
	drawing our stuff, and then returning FALSE to tell the Dialog Manager to
	update everything _it_ is responsible for. In the case of Null events, we
	handling the drawing of our command-key hints, and then return FALSE to
	the Dialog Manager to let it do whatever it needs to do at idle time, like
	blink the insertion point. On the other hand, if the user presses Return,
	Enter, Esc, or one of the magic Command keys we look out for, we return
	TRUE because we don�t want the Dialog Manager to do what it would normally
	do with those key presses (which is to enter them into any EditText items
	which might be active).

*******************************************************************************/
pascal Boolean OurFilter(DialogPtr dlg, EventRecord *event, short *itemHit)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;
	char	keys[] = "BHWATPFSR";
	char	key;
	short	loopy;
	short	radius;

	switch (event->what) {

		case nullEvent:
			if (event->modifiers & cmdKey)			/* Command key down */
				FlipItems(dlg, TRUE);
			else
				FlipItems(dlg, FALSE);
			return FALSE;
			break;

		case keyDown:
		case autoKey:
			key = event->message & charCodeMask;
			if (event->modifiers & cmdKey) {		/* Command key down */
				key = UprChar(key);
				if (key == kPeriod) {
					*itemHit = cancel;
					FlashDialogItem(dlg, *itemHit);
				}
				for (loopy = cstrlen(keys) - 1 ; loopy >= 0 ; --loopy) {
					if (key == keys[loopy]) {
						*itemHit = loopy + kFirstRadio;
					}
				}
				return TRUE;			/* This says we handle ALL command keys */
			} else {
				if ((key == kReturn) || (key == kEnter)) {
					*itemHit = ok;
					FlashDialogItem(dlg, *itemHit);
					return TRUE;
				}
				if (key == kEscape) {
					*itemHit = cancel;
					FlashDialogItem(dlg, *itemHit);
					return TRUE;
				}
			}
			return FALSE;

		case updateEvt:
			SetPort(dlg);
			GetDItem(dlg, ok, &iKind, &iHandle, &iRect);
			InsetRect(&iRect, -4, -4);
			radius = (iRect.bottom - iRect.top) / 2;
			if (radius < 16)
				radius = 16;
			PenNormal();
			PenSize(3,3);
			FrameRoundRect(&iRect, radius, radius);

			GetDItem(dlg, kIconID, &iKind, &iHandle, &iRect);

			InsetRect(&iRect, -3, -3);
			PenSize(2, 2);
			FrameRect(&iRect);

			return FALSE;

		default:
			return FALSE;
	}
}


/*******************************************************************************

	ShowModeless

	If we haven�t already done so, create the modeless dialog. Next, show the
	dialog, bring it to front, and make it the current grafPort just for luck.


*******************************************************************************/
void	ShowModeless(void)
{
	if (gModelessDialog == NIL) {
		gModelessDialog = GetNewDialog(kModelessDialog, NIL, (WindowPtr) -1);
	}

	ShowWindow(gModelessDialog);
	SelectWindow(gModelessDialog);
	SetPort(gModelessDialog);

	AdjustMenus();
}


/*******************************************************************************

	StartMovable

	Kicks off our movable modal dialog that has a progress indicator. First,
	we check to see if we already have such a dialog lying around. If not,
	then we create one, and save its pointer in a global variable for the next
	time we need this dialog. After the dialog is created, we set the string
	that indicates what we�re doing. Once all that�s done, we show the window
	and bring it to the front. Finally, we initialize our progress indicator
	routines, and set a global flag telling a routine elsewhere in this file
	(DoMainLoopTasks) that it should get off its duff and start doing what we
	pay it for.

*******************************************************************************/
void	StartMovable(void)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	Str255	theString;
	short	itemHit;

	if (gMovableModalDialog == NIL) {
		gMovableModalDialog = GetNewDialog(kMovableModalDialog, NIL, (WindowPtr) -1);

		GetIndString(theString, kDialogStrings, kErasingDisk);
		GetDItem(gMovableModalDialog, kMMText, &iType, &iHandle, &iRect);
		SetIText(iHandle, theString);
	}

	ShowWindow(gMovableModalDialog);
	SelectWindow(gMovableModalDialog);
	SetPort(gMovableModalDialog);

	GetDItem(gMovableModalDialog, kMMProgressBar, &iType, &iHandle, &iRect);
	InitProgressIndicator(gMovableModalDialog, iRect, 500);

	gPeriodicTask = TRUE;
	gInModalState = TRUE;
	AdjustMenus();
}


/*******************************************************************************

	Utility dialog routines

*******************************************************************************/

/*******************************************************************************

	FlashDialogItem

	Utility to quickly flash a button in the dialog. We highlight the button
	by calling HiliteControl, waiting a little bit so that the user can see
	the button highlighted, and then turning off the button by calling
	HiliteControl again.

	This routine assumes that the item we want to flash is backed up by a
	Control Manager control, and really works best if the item is a simple
	button control.

*******************************************************************************/
void	FlashDialogItem(DialogPtr dlg, short itemToFlash)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;
	long	ignored;

	GetDItem(dlg, itemToFlash, &iKind, &iHandle, &iRect);
	HiliteControl((ControlHandle) iHandle, 1);
	Delay(8, &ignored);
	HiliteControl((ControlHandle) iHandle, 0);
}


/*******************************************************************************

	SetDialogItemState

	Utility to set the selected state of a dialog item. This is really
	intended to be used on dialog items that are backed up by Control Manager
	controls only.

*******************************************************************************/
void	SetDialogItemState(DialogPtr dlg, short controlNumber, short value)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, controlNumber, &iKind, &iHandle, &iRect);
	SetCtlValue((ControlHandle) iHandle, value);
}


/*******************************************************************************

	GetDialogItemState

	Utility to return the selected state of a dialog item. This is really
	intended to be used on dialog items that are backed up by Control Manager
	controls only.

*******************************************************************************/
short	GetDialogItemState(DialogPtr dlg, short controlNumber)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, controlNumber, &iKind, &iHandle, &iRect);
	return GetCtlValue((ControlHandle) iHandle);
}


/*******************************************************************************

	SetRadioButton

	Simple utility routine to set a radio button in a dialog box. We take a
	look at the item we are requested to set and compare it with the item that
	is currently set. If they are the same, we don�t need to do anything.
	However, if they are different, we turn off the old one, and then turn on
	the new one. We also remember which item we just turned on so that we can
	refer to it the next time this routine is called.

	If the number identifying the currently selected button is zero, we
	interpret that to be a magic number meaning that _no_ radio buttons are
	currently selected. If that�s the case, there are no buttons to be turned
	off, and we just turn on the new button.

	This routine is very simple in that it supports just a single family of
	buttons. In other words, _all_ radio buttons in the dialog are connected
	together; turning on any single button with turn off any other button in
	the dialog. If you want to support multiple families of radio buttons, you
	will have to do a little more work.

*******************************************************************************/
void	SetRadioButton(DialogPtr dlg, short buttonNumber)
{
	if (buttonNumber != gCurrentRadio) {
		if (gCurrentRadio != 0)
			SetDialogItemState(dlg, gCurrentRadio, 0);
		gCurrentRadio = buttonNumber;
		if ((gCurrentRadio >= kFirstRadio) && (gCurrentRadio <= kLastRadio))
			SetDialogItemState(dlg, gCurrentRadio, 1);
	}
}


/*******************************************************************************

	ToggleCheckBox

	Simple utility to toggle the setting of a checkbox item in a dialog box.
	We get the old setting, take a look at it so that we know what to change
	it into, and then call a utility routine that sets a dialog item to an
	absolute value.

*******************************************************************************/
void	ToggleCheckBox(DialogPtr dlg, short buttonNumber)
{
	short newState;

	if (GetDialogItemState(dlg, buttonNumber) == 0)
		newState = 1;
	else
		newState = 0;
	SetDialogItemState(dlg, buttonNumber, newState);
}


/*******************************************************************************

	SetDialogItemTitle

	Change the text associated with a particular dialog item. This is a little
	tricky since there are two ways to change the text. If you are dealing
	with an EditText or StatText (static text) item, you must call SetIText.
	If you are dealing with a dialog item that is backed up by a Control
	Manager control (like a simple button, radio button, or checkbox), you
	must call SetCTitle.

	We determine what kind of dialog item we are dealing with by calling
	GetDItem. Returned in the �kind� parameter is a number that identifies
	what sort of item we are handling. First, we strip off the upper bit,
	which identifies the item as being enabled or disabled. Once that bit is
	removed, we can examine the kind of the item and act accordingly.

	Notice the special handling we give to controls when we call SetCTitle.
	This is to take care of �excessive flashing� as Online Companion puts it.
	When you call SetCTitle, the control manager first calls HideControl to
	remove the control with its old text from the screen. It then changes the
	control�s title in the ControlRecord, and reshows the control by calling
	ShowControl. At this point, the control is properly shown on the screen
	with its correct, new title.

	However, there�s a little time bomb lurking in the works. When HideControl
	was called, the Control Manager called InvalRect on the area the control
	occupied. Even though ShowControl was later called on the same area and
	everything is drawn correctly, that rectangle is still marked as invalid
	and is incorporated into the update region for the dialog. The event loop
	at the heart of ModalDialog will then get an update event for that area
	and redraw the button _again_! This can cause the button to flicker and
	flash more than we would like. We already know that that area is
	adequately drawn, so we tell the Event Manager to hoof it by validating it
	with a call to ValidRect.

*******************************************************************************/
void	SetDialogItemTitle(DialogPtr dlg, short item, Str255 *newTitle)
{
	short	iKind;
	Handle	iHandle;
	Rect	iRect;

	GetDItem(dlg, item, &iKind, &iHandle, &iRect);
	iKind &= ~itemDisable;				// Strip off the enable/disable bit
	if ((iKind == statText) || (iKind == editText)) {
		SetIText(iHandle, *newTitle);
	} else {
		SetCTitle((ControlHandle) iHandle, *newTitle);
		SetPort(dlg);
		ValidRect(&iRect);
	}
}


/*******************************************************************************

	FlipItems

	This routine changes the text associated with dialog  items. Basically, we
	allow the items to have two states: the flipped on state and the flipped
	off state (sometimes called the �being given the bird� state). The state
	the we want the items to be in is passed in the �flippedOn� parameter.

	We decide which items to change and how to change them by using a couple
	of �STR#� resources. Each of these resources contains one string for each
	of the items in the dialog box. The first �STR#� resource contains the
	strings for the items when they are in the off state. The other �STR#�
	contains the strings that get shown when the items are in the on state.

	It doesn�t make sense to change the text of all items. For example, the
	icon doesn�t have any text to change. Also, the OK has only one state. To
	handle items that don�t change, we enter zero-length strings in their
	place in the �STR#� resource. When we come to one of those strings, we
	just skip over that item.

	Once we�ve changed the text of all the affected items, we make a note of
	the state we�ve left them in. This way, if we call this routine to put the
	items into State A, and they�re already in State A, there is nothing to
	do, and we just skip out of the function.

	By the way, take a look at how we determine how many strings there are in
	the �STR#� resources. The format of a �STR#� resource is as follows:

		short		n=number of strings in resource
		x1 bytes	String #1
		x2 bytes	String #2
		x3 bytes	String #3
		...			...
		xn bytes	String #n

	To find the number of strings in the resource, we coerce the handle to the
	resource into a pointer to a short and fetch the number. Once we have
	that, we can make that many calls to GetIndString to read all the strings
	one at a time.

	Notice that we make a big assumption about the relationship between the
	two �STR#� resources and the number of items in the dialog. In our
	example, we have 26 items in the dialog and 26 strings in each �STR#�
	resources. If any of these conditions changes, we might be in trouble.
	However, you can never protect against _everything_ a user armed with
	ResEdit may do, so we take our chances and assume everything is the way we
	designed it to be.

*******************************************************************************/
void	FlipItems(DialogPtr dlg, Boolean flippedOn)
{
	short	loopy;
	short	numberOfStrings;
	short	resourceNumber;
	Handle	strHandle;
	Str255	stringMeAlong;
	
	if (flippedOn != gCurrentFlippedState) {

		if (flippedOn)
			resourceNumber = kFlippedStrings;
		else
			resourceNumber = kSourceStrings;

		strHandle = GetResource('STR#', resourceNumber);
		numberOfStrings = **(short **) strHandle;

		for (loopy = 1 ; loopy <= numberOfStrings ; ++loopy) {
			GetIndString(stringMeAlong, resourceNumber, loopy);
			if (stringMeAlong[0] != 0) {
				SetDialogItemTitle(dlg, loopy, &stringMeAlong);
			}
		}
	}
	gCurrentFlippedState = flippedOn;
}


/*******************************************************************************

	cstrlen

	Return the length of a C-string (a series of characters terminated by a
	NULL. We have our own copy so that we don�t have to link with ANSI.

*******************************************************************************/
long cstrlen( char *s )
{
	long	size = -1;
	char*	str = s;
	
	do {
		++size;
	} while (*str++);
	return size;
}


/*******************************************************************************

	UprChar

	Convert a lower case letter to upper case. If we�re running under System
	7.0, we could just use the new UpperText() call instead.

*******************************************************************************/
char UprChar( char c )
{
	Str255	tempString;

	tempString[0] = 1;
	tempString[1] = c;
	UprString(tempString, TRUE);
	return tempString[1];
}

