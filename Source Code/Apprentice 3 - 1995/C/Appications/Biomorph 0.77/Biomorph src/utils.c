#include <MacHeaders>
#include <stdio.h>
#include <string.h>
#include "constants.h"
#include "mathtype.h"
#include "globals.h"


void ReadCoords(void)
{
	char s[256];	/* temporary string */
	int itemType;
	Handle h=NULL;
	Rect box;
	
	GetDItem( gControlDialog, kETXMin, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gXmin);
	GetDItem( gControlDialog, kETXMax, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gXmax);
	GetDItem( gControlDialog, kETYMin, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gYmin);
	GetDItem( gControlDialog, kETYMax, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gYmax);
	GetDItem( gControlDialog, kETCreal, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gCreal);
	GetDItem( gControlDialog, kETCimag, &itemType, &h, &box);
		GetIText( h, s);
		PtoCstr(s);
		sscanf(s, "%lf", &gCimag);
	
	gCenterX = gXmax - (gXmax - gXmin)/2.0;
	gCenterY = gYmax - (gYmax - gYmin)/2.0;
}


void WriteCoords(void)
{
	Str255 s;
	int itemType;
	Handle h;
	Rect box;
	
	sprintf( (char*)s, "%lf", gXmin);
	CtoPstr(s);
	GetDItem( gControlDialog, kETXMin, &itemType, &h, &box);
	SetIText( h, s);

	sprintf( (char*)s, "%lf", gXmax);
	CtoPstr(s);
	GetDItem( gControlDialog, kETXMax, &itemType, &h, &box);
	SetIText( h, s);

	sprintf( (char*)s, "%lf", gYmin);
	CtoPstr(s);
	GetDItem( gControlDialog, kETYMin, &itemType, &h, &box);
	SetIText( h, s);

	sprintf( (char*)s, "%lf", gYmax);
	CtoPstr(s);
	GetDItem( gControlDialog, kETYMax, &itemType, &h, &box);
	SetIText( h, s);

	sprintf( (char*)s, "%lf", gCreal);
	CtoPstr(s);
	GetDItem( gControlDialog, kETCreal, &itemType, &h, &box);
	SetIText( h, s);

	sprintf( (char*)s, "%lf", gCimag);
	CtoPstr(s);
	GetDItem( gControlDialog, kETCimag, &itemType, &h, &box);
	SetIText( h, s);

} // WriteCoords()



// ----------------  dialog stuff here

void Error( int s1, int s2, int s3, int s4, int severity)
// display error msgs.
// All errors use the same ALRT resource (kErrAlrt)
// and s1 - s4 are the STR# indices to use for ParamText()
// severity is the icon # to use for the alert box
{
	Str255 p1, p2, p3, p4;
	
	GetIndString( &p1, kErrStrList, s1);
	GetIndString( &p2, kErrStrList, s2);
	GetIndString( &p3, kErrStrList, s3);
	GetIndString( &p4, kErrStrList, s4);
	ParamText( p1, p2, p3, p4);
	switch (severity) {
	case stopIcon:
		(void)StopAlert( kErrAlertID, NULL);
		break;
	case cautionIcon:
		(void)CautionAlert( kErrAlertID, NULL);
		break;
	case noteIcon:
	default:
		(void)NoteAlert( kErrAlertID, NULL);
		break;
	} // switch

} // Error()


void UserItem(DialogPtr theDialog,short itemNum,void *theProc)
{
	short	aType;
	Rect	aBox;
	Handle	theItem;

	GetDItem(theDialog,itemNum,&aType,&theItem,&aBox);
	SetDItem(theDialog,itemNum,aType,(Handle)theProc,&aBox);
} // UserItem()


pascal void BoxItem(WindowPtr theWindow,short itemNum)
{
	short	aType;
	Rect	theBox;
	Handle	aHandle;

	GetDItem(theWindow,itemNum,&aType,&aHandle,&theBox);
	PenNormal();
	ForeColor(blackColor);
	
	FrameRect(&theBox);
}  // BoxItem()


pascal void OutlineButton(DialogPtr theDialog, short itemNum)
{
	short	aType;
	Rect	box;
	Handle	h;
	
	GetDItem( theDialog, itemNum, &aType, &h, &box);
	SetPort( (GrafPtr)theDialog);
	PenNormal();
	PenSize(3,3);
	FrameRoundRect( &box, 16, 16);
} // DefaultButton()


// --------------------
void Print(char *s)
// prints the C string to the gSelectWindow
{
	static int y=0;
	int maxy;
	GrafPtr savedPort;

	GetPort(&savedPort);
	SetPort(gSelectWindow);
	
	if (s == (char*)0)
	{
		y = 0;
		EraseRect(&gSelectWindow->portRect);
	}
	else
	{
		maxy = gSelectWindow->portRect.bottom;
		y += gPrintY;  // initial pen positions
		if (y > maxy)
		{	y=gPrintY;
			EraseRect(&(gSelectWindow->portRect));
		}
		MoveTo(0,y);
		DrawText(s, 0, strlen(s));
	}
	
	SetPort(savedPort);
} // Print()


MathType dabs(MathType x)
{
	if (x < 0.0)
		return 0.0-x;
	else
		return x;
}

