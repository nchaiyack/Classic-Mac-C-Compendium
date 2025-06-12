/*

		NeXT CDEF Tester.c++
		Version 1.0
		
		by Hiep Dam, 3G Software
		This file & code in this file public domain.
		Contacting author:
			America Online: StarLabs
			Delphi		  : StarLabs
			Internet	  : starlabs@aol.com, starlabs@delphi.com

		Last update: May 4, 1994

*/


enum {
	exitBtn = 1,
	stdCheckBtn = 2,
	stdRadioBtn = 3,
	NeXTBtn1 = 4,
	disabledNeXTBtn = 5,
	sysFontNeXTBtn = 6,
	geneva9NeXTBtn = 7,
	cicnBtn1 = 8,
	cicnBtn2 = 9,
	smallCicnBtn = 10,
	flipCicnBtn1 = 11,
	flipCicnBtn2 = 12,
	NeXTBtn1Status = 13,
	disabledNeXTBtnStatus = 14,
	sysFontNeXTBtnStatus = 15,
	geneva9NeXTBtnStatus = 16,
	cicnBtn1Status = 17,
	cicnBtn2Status = 18,
	smallCicnBtnStatus = 19,
	checkBox1 = 23,
	checkBox2 = 24,
	checkBox3 = 25,
	radio1 = 26,
	radio2 = 27,
	radio3 = 28,
	checkBoxBegin = checkBox1,
	checkBoxEnd = radio3,
	checkBoxDisable = 29,

	statusOffset = 9,			// NeXTBtn1Status - NeXTBtn1
	
	windowFont = systemFont,
	windowFontSize = 12
};

void InitMac();
void DoDialog();
void DrawControlValue(short theVal);
void HandleMovableDialog(EventRecord *theEvent);

DialogPtr theDialog;


void main() {
	InitMac();
	DoDialog();
} // END main

void DoDialog() {	
	Handle itemH; short itemT; Rect itemR;

	theDialog = GetNewDialog(128, nil, (WindowPtr)-1);
	SetPort(theDialog);
	TextFont(windowFont);
	TextSize(windowFontSize);

	ShowWindow(theDialog);

	short itemHit, controlValue, onValue, offValue;
	do {
		EventRecord theEvent; Boolean eventExists;
		eventExists = GetNextEvent(everyEvent, &theEvent);
		HandleMovableDialog(&theEvent);
		if (IsDialogEvent(&theEvent)) {
			if (DialogSelect(&theEvent, &theDialog, &itemHit)) {
				switch(itemHit) {
					case stdCheckBtn:
					case stdRadioBtn:
						GetDItem(theDialog, itemHit, &itemT, &itemH, &itemR);
						controlValue = GetCtlValue((ControlHandle)itemH);
						controlValue = !controlValue;
						SetCtlValue((ControlHandle)itemH, controlValue);
						DrawControlValue(controlValue);
					break;
		
					case flipCicnBtn1:
					case flipCicnBtn2: {
						GetDItem(theDialog, itemHit, &itemT, &itemH, &itemR);
						controlValue = GetCtlValue((ControlHandle)itemH);
						onValue = GetCtlMax((ControlHandle)itemH);
						offValue = GetCtlMin((ControlHandle)itemH);
						if (controlValue == onValue)
							controlValue = offValue;
						else
							controlValue = onValue;
						SetCtlValue((ControlHandle)itemH, controlValue);
						DrawControlValue(controlValue);
					} break;
		
					case NeXTBtn1Status:
					case disabledNeXTBtnStatus:
					case sysFontNeXTBtnStatus:
					case geneva9NeXTBtnStatus:
					case cicnBtn1Status:
					case cicnBtn2Status:
					case smallCicnBtnStatus:
					case checkBoxDisable:
						GetDItem(theDialog, itemHit, &itemT, &itemH, &itemR);
						controlValue = GetCtlValue((ControlHandle)itemH);
						onValue = GetCtlMax((ControlHandle)itemH);
						offValue = GetCtlMin((ControlHandle)itemH);
						if (controlValue == onValue)
							controlValue = offValue;
						else
							controlValue = onValue;
						SetCtlValue((ControlHandle)itemH, controlValue);
						if (itemHit == checkBoxDisable) {
							for (short i = checkBoxBegin; i <= checkBoxEnd; i++) {
								GetDItem(theDialog, i, &itemT, &itemH, &itemR);
								if (itemT < 4 || itemT > 7)
									continue;
								if (controlValue == onValue)
									HiliteControl((ControlHandle)itemH, 0);
								else
									HiliteControl((ControlHandle)itemH, 255);
							}
						}
						else {
							GetDItem(theDialog, itemHit - statusOffset, &itemT, &itemH, &itemR);
							if (controlValue == onValue)
								HiliteControl((ControlHandle)itemH, 0);
							else
								HiliteControl((ControlHandle)itemH, 255);
						}
						DrawControlValue(controlValue);
					break;

					case radio1:
					case radio2:
					case radio3:
						for (short i = radio1; i <= radio3; i++) {
							GetDItem(theDialog, i, &itemT, &itemH, &itemR);
							SetCtlValue((ControlHandle)itemH, 0);
						}
						GetDItem(theDialog, itemHit, &itemT, &itemH, &itemR);
						SetCtlValue((ControlHandle)itemH, 1);
					break;

					case checkBox1:
					case checkBox2:
					case checkBox3:
						GetDItem(theDialog, itemHit, &itemT, &itemH, &itemR);
						controlValue = GetCtlValue((ControlHandle)itemH);
						controlValue = !controlValue;
						SetCtlValue((ControlHandle)itemH, controlValue);
						DrawControlValue(controlValue);
					break;
				} // END switch
			}
		}
	} while (itemHit != exitBtn);
} // END DoDialog

void HandleMovableDialog(EventRecord *theEvent) {
	if (theEvent->what == mouseDown) {
		short mouseLoc;
		WindowPtr theWindow;

		mouseLoc = FindWindow(theEvent->where, &theWindow);
		switch (mouseLoc) {
			case inDrag: {
				Rect dragRect = screenBits.bounds;
				InsetRect(&dragRect, 10, 10);
				DragWindow(theWindow, theEvent->where, &dragRect);
			} break;
			
			default: {
			} break;
		} // END switch
	}
	else if (theEvent->what == keyDown) {
		short itemT; Handle itemH; Rect itemR;
		GetDItem(theDialog, 1, &itemT, &itemH, &itemR);
		HiliteControl((ControlHandle)itemH, inButton);
		long dummy;
		Delay(7, &dummy);
		HiliteControl((ControlHandle)itemH, 0);
		ExitToShell();
	}
} // END HandleMovableDialog

void DrawControlValue(short theVal) {
	RGBColor RGBltGray, RGBwhite;
	RGBltGray.red = RGBltGray.blue = RGBltGray.green = 56797;
	RGBwhite.red = RGBwhite.blue = RGBwhite.green = 65535;
	TextFont(geneva);
	TextSize(9);
	TextMode(srcCopy);
	RGBBackColor(&RGBltGray);

	Str255 tempStr;
	NumToString(theVal, tempStr);
	MoveTo(theDialog->portRect.left + 10, theDialog->portRect.bottom - 6);
	DrawString("\pValue: "); DrawString(tempStr); DrawString("\p       ");

	TextFont(windowFont);
	TextSize(windowFontSize);
	TextMode(srcOr);
	RGBBackColor(&RGBwhite);
} // END DrawControlValue

void InitMac() {
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}