/***********************************************************************
 ******* PopupMenu Tester **********************************************
 ***********************************************************************
 Written by Paul Celestin
 
 Public Domain - use at your own discretion, in any way you see fit
 
 This simple application demonstrates the use of a System-7 popup menu,
 as well as a movable modal dialog. It doesn't do much else, but maybe
 it has something to offer to someone who is new to all of this.
 
 941103 - 1.0.0 initial version
 **********************************************************************/
 
#include	<Dialogs.h>
#include	<Events.h>

#define		kTestDLOG	128
#define		kMenu		128
#define		kItemQuit	1
#define		kItemPopup	2
#define		kDelay		8

Boolean		gDone = false;

pascal Boolean ModalDragProc(DialogPtr inDialog, EventRecord *inEvent, short *pItem)
{
	Boolean done = false;

	switch (inEvent->what)
	{
		case mouseDown:
		{
			WindowPtr	myWindow;
			RgnHandle	theGrayRgn;

			short thePart = FindWindow(inEvent->where,&myWindow);
			if ((thePart == inDrag) && (myWindow == inDialog))
			{
				theGrayRgn = GetGrayRgn();
				DragWindow(myWindow,inEvent->where,&((**theGrayRgn).rgnBBox));
				done = true;
			}
		}
		break;
		
		case keyDown:
		{
			char typedChar = inEvent->message & 0x00FF;

			switch (typedChar)
			{
				case 0x03: /* the Enter key */
				case 0x0D: /* the Return key */
				{
					short	iType,iValue;
					long	delay;
					Handle	iHandle;
					Rect	iRect;
					
					GetDItem(inDialog,kItemQuit,&iType,&iHandle,&iRect);
					if (iHandle) /* it exists */
					{
						iValue = !GetCtlValue((ControlHandle)iHandle);
						HiliteControl((ControlHandle)iHandle,1);
						Delay(kDelay,&delay);
						HiliteControl((ControlHandle)iHandle,0);
					}
					done = true;
					gDone = true;
				}
				break;
			}
		}
		break;

		}
	return done;
}

main()
{
	short		itemType, itemHit, myChoice;
	Str255		myTitle;
	DialogPtr	myDialog;
	Handle		myPopHandle;
	MenuHandle	myMenu;
	Rect		myPopRect;

	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0);
	InitCursor();

	myDialog = GetNewDialog(kTestDLOG,0,(WindowPtr) -1);
	if (myDialog)
	{
		while (!gDone)
		{
			ModalDialog(ModalDragProc,&itemHit);
			switch(itemHit)
			{
				case kItemQuit:
					gDone = true;
				break;

				case kItemPopup:
					/* user selected the popup menu, let's show selection in title bar */
					GetDItem(myDialog,kItemPopup,&itemType,&myPopHandle,&myPopRect);
					myChoice = GetCtlValue((ControlHandle)myPopHandle);
					myMenu = GetMenu(kMenu);
					GetItem(myMenu,myChoice,myTitle);
					SetWTitle(myDialog,myTitle);
				break;
			}
		}
		DisposeDialog(myDialog);
	}
}
