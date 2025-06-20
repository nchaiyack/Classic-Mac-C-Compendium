enum
{
	smallAlert = 128,		/* for small stuff */
	largeAlert,				/* for large stuff */
	saveAlert				/* save changes alert */
};

void PositionDialog(ResType theType, short theID);
pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum);
pascal Boolean OneButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
pascal Boolean TwoButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
pascal Boolean ThreeButtonFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
short FakeSelect(DialogPtr theDialog, short itemNum);
void SetButtonHighlight(DialogPtr theDialog, short itemNum, Boolean isOn);
void SetButtonTitle(DialogPtr theDialog, short itemNum, Str255 theStr);
