#include "ep.const.h"
#include "ep.extern.h"
#include "DialogMgr.h"

setColors()
{
	DialogPtr	colorsDialogPtr;
	int			itemHit;
	Rect	junkRect;
    int		tempKind;

ControlHandle
	backgroundButton,		/* colors dialog items */
	earthbackgroundButton,
	earthoutlineButton,
	latButton,
	longButton,
	landmassButton,
	
	IButton[6],
	CButton[8];

int		i,
		currentC;
long	c[8],
		t[6];		/* colors temp storage */

int	currentT;

	c[0] = blackColor;
	c[1] = whiteColor;
	c[2] = redColor;
	c[3] = greenColor;
	c[4] = blueColor;
	c[5] = cyanColor;
	c[6] = magentaColor;
	c[7] = yellowColor;

	t[0] = backgroundC;
	t[1] = earthbackgroundC;
	t[2] = earthoutlineC;
	t[3] = latC;
	t[4] = longC;
	t[5] = landC;

	colorsDialogPtr = GetNewDialog (300, 0L, (long) -1);
	GetDItem(colorsDialogPtr,11,&tempKind,&IButton[0],&junkRect);
	GetDItem(colorsDialogPtr,12,&tempKind,&IButton[1],&junkRect);
	GetDItem(colorsDialogPtr,13,&tempKind,&IButton[2],&junkRect);
	GetDItem(colorsDialogPtr,14,&tempKind,&IButton[3],&junkRect);
	GetDItem(colorsDialogPtr,15,&tempKind,&IButton[4],&junkRect);
	GetDItem(colorsDialogPtr,16,&tempKind,&IButton[5],&junkRect);
	GetDItem(colorsDialogPtr,3,&tempKind,&CButton[0],&junkRect);
	GetDItem(colorsDialogPtr,4,&tempKind,&CButton[1],&junkRect);
	GetDItem(colorsDialogPtr,5,&tempKind,&CButton[2],&junkRect);
	GetDItem(colorsDialogPtr,6,&tempKind,&CButton[3],&junkRect);
	GetDItem(colorsDialogPtr,7,&tempKind,&CButton[4],&junkRect);
	GetDItem(colorsDialogPtr,8,&tempKind,&CButton[5],&junkRect);
	GetDItem(colorsDialogPtr,9,&tempKind,&CButton[6],&junkRect);
	GetDItem(colorsDialogPtr,10,&tempKind,&CButton[7],&junkRect);

	currentT = 0;

	do {
		for (i=0;i<6;i++)
			SetCtlValue(IButton[i],i == currentT);
		
		for (i=0;i<8;i++)
			SetCtlValue(CButton[i],c[i] == t[currentT]);

		ModalDialog (0L, &itemHit);
		
		switch (itemHit) {
		case 11:				/* "items" */
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
			currentT = itemHit - 11;
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			t[currentT] = c[itemHit - 3];
			break;
		}
	
	} while ((itemHit != 1/* OK */) && (itemHit != 2/* CANCEL */));

	DisposDialog(colorsDialogPtr);

	if (itemHit == 1) {
		backgroundC 	= t[0];
		earthbackgroundC= t[1];
		earthoutlineC 	= t[2];
		latC 			= t[3];
		longC 			= t[4];
		landC 			= t[5];
	}
}

