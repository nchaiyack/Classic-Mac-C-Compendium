/*
	3D Buttons Demo
	
	Public Domain source by Zig Zichterman
	ziggr@aol.com
	
	A real quick and dirty demo for the 3D Buttons CDEF.
	
	If you put CDEF 0 in the resource file ("3D Buttons Demo.�.rsrc") for
	this project, the demo will use it for the dialog. I have already 
	put version 1.0b1 of the 3D Buttons CDEF in the resource file.
	
	1.0b3
		07/29/94	zz		make dialog movable. This code isn't Mixed Mode
								savvy, 'cause I don't have a PowerPC yet.
	1.0b2
		07/20/94	zz		call InitCursor() right before running the dialog,
	1.0b1	
		06/29/94	zz		these comments	
	
*/

void	InitializeToolbox();
void	Toggle(DialogPtr inDialog, short inItemNum);
void	Tristate(DialogPtr inDialog, short inItemNum);
void	Disable(DialogPtr inDialog, short inItemNum);
void	SetValue(DialogPtr inDialog, short inItemNum, short inValue);
pascal Boolean	DragginFilter(DialogPtr inDialog, EventRecord *inEvent,
					short *ioItemHit);
void		FlashButton(DialogPtr inDialog, short inItemNum);
pascal void	DefaultFrameProc(DialogPtr inDialog, short inItem);
void		SetupDefaultFrameProc(DialogPtr inDialog);

enum {
	push_enabled	= 1,
	check_enabled,
	radio1_enabled,
	radio2_enabled,
	icon1_enabled,
	icon2_enabled,
	
	push_disabled,
	check_disabled,
	radio1_disabled,
	radio2_disabled,
	icon1_disabled,
	icon2_disabled,
	
	push_trireset,
	
	check_tristate,
	radio1_tristate,
	radio2_tristate,
	
	exit,
	exitFrame
};

void
main(void)
{
	InitializeToolbox();
	MaxApplZone();
	
	DialogPtr	dialog = ::GetNewDialog(128,NULL,(WindowPtr) -1);
	if (!dialog) return;
	
	// set up dialog
	SetValue(dialog,radio1_enabled,1);
	SetValue(dialog,check_tristate,2);
	SetValue(dialog,radio1_tristate,2);
	SetValue(dialog,radio2_tristate,2);
	SetValue(dialog,radio1_disabled,1);
	
	Disable(dialog,push_disabled);
	Disable(dialog,check_disabled);
	Disable(dialog,radio1_disabled);
	Disable(dialog,radio2_disabled);
	Disable(dialog,icon1_disabled);
	Disable(dialog,icon2_disabled);

	SetupDefaultFrameProc(dialog);
	
	::InitCursor();
	short itemHit;
	do {
		::ModalDialog(DragginFilter,&itemHit);
		
		// handle clicks
		switch (itemHit) {
			case check_enabled  :
				Toggle(dialog,itemHit);
				break;
			case radio1_enabled  :
			case radio1_tristate :
				SetValue(dialog,itemHit,1);
				SetValue(dialog,itemHit + 1,0);
				break;
			case radio2_enabled  :
			case radio2_tristate :
				SetValue(dialog,itemHit,1);
				SetValue(dialog,itemHit - 1,0);
				break;
			case check_tristate :
				Tristate(dialog,itemHit);
				break;
			case push_trireset  :
				SetValue(dialog,check_tristate,2);
				SetValue(dialog,radio1_tristate,2);
				SetValue(dialog,radio2_tristate,2);
				break;
		}
	} while (itemHit != exit);
	
	::DisposeDialog(dialog);
}

/**************************************************************************
	Toggle()
	
	Toggle the value of a control item in a dialog
**************************************************************************/
void
Toggle(DialogPtr inDialog, short inItemNum)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,inItemNum,&iType,&iHandle,&iRect);
	if (iHandle) {
		iValue = GetCtlValue((ControlHandle) iHandle);
		iValue = !iValue;
		SetCtlValue((ControlHandle) iHandle,iValue);
	}
}

/**************************************************************************
	Disable()
	
	Disable (dim) a control item in a dialog
**************************************************************************/
void
Disable(DialogPtr inDialog, short inItemNum)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,inItemNum,&iType,&iHandle,&iRect);
	iType |= itemDisable;
	SetDItem(inDialog,inItemNum,iType,iHandle,&iRect);
	
	if (iType & ctrlItem) {
		HiliteControl((ControlHandle) iHandle,255);
	}
}

/**************************************************************************
	Tristate()
	
	Rotate a control value through 0,1 and 2 (2 = tristate)
**************************************************************************/
void
Tristate(DialogPtr inDialog, short inItemNum)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,inItemNum,&iType,&iHandle,&iRect);
	if (iHandle) {
		iValue = GetCtlValue((ControlHandle) iHandle);
		if (++iValue > 2) iValue = 0;
		SetCtlValue((ControlHandle) iHandle,iValue);
	}
}

/**************************************************************************
	SetValue()
	
	Set the value of a control
**************************************************************************/
void
SetValue(DialogPtr inDialog, short inItemNum, short inValue)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	
	GetDItem(inDialog,inItemNum,&iType,&iHandle,&iRect);
	if (iHandle) {
		short max = GetCtlMax((ControlHandle) iHandle);
		if (max < inValue) SetCtlMax((ControlHandle) iHandle, inValue);
		SetCtlValue((ControlHandle) iHandle,inValue);
	}
}


/**************************************************************************
	InitializeToolbox()
	
	The requisite toolbox initialization that every application under
	the sun has.
**************************************************************************/
void
InitializeToolbox()
{
	InitGraf((Ptr) &qd.thePort);		// Toolbox Managers
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
}

/**************************************************************************
	DragginFilter()
	
	My dialog filter proc that makes the modal dialog movable
**************************************************************************/
pascal Boolean
DragginFilter(DialogPtr inDialog, EventRecord *inEvent,	short *ioItemHit)
{
	Boolean	done	= false;
	
	switch (inEvent->what) {
		case mouseDown :
			{
				WindowPtr	hitWindow;
				short	hitPart = ::FindWindow(inEvent->where,&hitWindow);
				if ((hitPart == inDrag) && (hitWindow == inDialog)) {
					RgnHandle	greyRgn = GetGrayRgn();	// pin to desktop
					::DragWindow(hitWindow,inEvent->where,&((**greyRgn).rgnBBox));
					done = true;
				}
			} 
			break;
		case keyDown :
			{
				char charTyped = inEvent->message & 0x00FF;
				switch (charTyped) {
					case 'q' :		// allow cmd-Q, just for kicks
						if (!(inEvent->modifiers & cmdKey)) break;
						// allow fall-through only if command key was down
					case 0x03 :		// enter
					case 0x0D :		// return
						*ioItemHit = exit;
						done = true;
						FlashButton(inDialog,exit);
				}
			}
			break;
	} 
	
	return done;
}

/**************************************************************************
	FlashButton()
	
	Flash a button for 8 ticks
**************************************************************************/
void
FlashButton(DialogPtr inDialog, short inItem)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,inItem,&iType,&iHandle,&iRect);
	if (iHandle) {
		iValue = GetCtlValue((ControlHandle) iHandle);
		iValue = !iValue;
		::HiliteControl((ControlHandle) iHandle,1);
		{ long junk; ::Delay(8,&junk); }
		::HiliteControl((ControlHandle) iHandle,0);
	}
}

/**************************************************************************
	DefaultFrameProc
	
	Draw a frame around the exit button
**************************************************************************/
pascal void
DefaultFrameProc(DialogPtr inDialog, short inItem)
{
	// ignore inItem, just draw a rect around exit button
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,exit,&iType,&iHandle,&iRect);
	::InsetRect(&iRect,-4,-4);
	::PenNormal();
	::PenSize(3,3);
	::FrameRoundRect(&iRect,16,16);
}

/**************************************************************************
	SetupDefaultFrameProc()
	
	Set exitFrame's draw function to DefaultFrameProc()
**************************************************************************/
void
SetupDefaultFrameProc(DialogPtr inDialog)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;
	short	iValue;
	
	GetDItem(inDialog,exitFrame,&iType,&iHandle,&iRect);
	SetDItem(inDialog,exitFrame,iType,(Handle) DefaultFrameProc, &iRect);
}
