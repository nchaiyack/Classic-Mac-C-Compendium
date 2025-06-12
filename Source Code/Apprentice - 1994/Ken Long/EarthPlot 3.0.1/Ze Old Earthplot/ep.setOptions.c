#include "ep.const.h"
#include "ep.extern.h"
#include "DialogMgr.h"

setOptions()
{
	DialogPtr	optionsDialogPtr;
	int			itemHit;
	Rect	junkRect;
    int		tempKind;

ControlHandle
	pictButton,		/* options dialog items */
	bitmapButton,
	intButton, 
	fpButton,
	hiddenlineBox,
	lnlBox,
	squareBox;

int
	tpictButton,		/* options dialog items */
	tbitmapButton,
	tintButton,
	tfpButton,
	thiddenlinesBox,
	tlnlBox,
	tsquareBox;

	tpictButton		= npictButton;
	tbitmapButton	= nbitmapButton;
	tintButton		= nintButton;
	tfpButton		= nfpButton;
	thiddenlinesBox	= nhiddenlinesBox;
	tlnlBox			= nlnlBox;
	tsquareBox		= nsquareBox;

	optionsDialogPtr = GetNewDialog (200, 0L, (long) -1);
	GetDItem(optionsDialogPtr,2,&tempKind,&pictButton,&junkRect);
	GetDItem(optionsDialogPtr,3,&tempKind,&bitmapButton,&junkRect);
	GetDItem(optionsDialogPtr,9,&tempKind,&intButton,&junkRect);
	GetDItem(optionsDialogPtr,8,&tempKind,&fpButton,&junkRect);
	GetDItem(optionsDialogPtr,5,&tempKind,&lnlBox,&junkRect);
	GetDItem(optionsDialogPtr,13,&tempKind,&hiddenlineBox,&junkRect);
	GetDItem(optionsDialogPtr,6,&tempKind,&squareBox,&junkRect);
	
	do {
		SetCtlValue(pictButton,tpictButton);
		SetCtlValue(bitmapButton,tbitmapButton);
		SetCtlValue(intButton,tintButton);
		SetCtlValue(fpButton,tfpButton);
		SetCtlValue(hiddenlineBox,thiddenlinesBox);
		SetCtlValue(lnlBox,tlnlBox);
		SetCtlValue(squareBox,tsquareBox);
	
		ModalDialog (0L, &itemHit);
		
		switch (itemHit) {
		case 2:						/* "picture" */
			tpictButton   = 1;
			tbitmapButton = 0;
			break;
		case 3:						/* "bitmap" */
			tpictButton   = 0;
			tbitmapButton = 1;
			break;
		case 9: 					/* "integer" */
			tintButton		= 1;
			tfpButton		= 0;
			break;
		case 8: 					/* "floating point" */
			tintButton		= 0;
			tfpButton		= 1;
			break;
		case 13:						/* "use hidden lines" */
			thiddenlinesBox	= (thiddenlinesBox) ? 0 : 1;
			break;
		case 5:						/* "draw lat & long" */
			tlnlBox			= (tlnlBox) ? 0 : 1;
			break;
		case 6:						/* "draw lat & long" */
			tsquareBox		= (tsquareBox) ? 0 : 1;
			break;
		}
	
	} while ((itemHit != 1/* OK */) && (itemHit != 12/* CANCEL */));

	DisposDialog(optionsDialogPtr);

	if (itemHit == 1) {
		npictButton		= tpictButton;
		nbitmapButton	= tbitmapButton;
		nintButton		= tintButton;
		nfpButton		= tfpButton;
		nhiddenlinesBox	= thiddenlinesBox;
		nlnlBox			= tlnlBox;
		nsquareBox		= tsquareBox;
	}
}

