#include "nim dialogs.h"
#include "nim globals.h"
#include "dialog layer.h"
#include "main.h"

enum	/* dialog IDs */
{
	kNimSetupID=300,
	kPrimeSetupID,
	kWythoffsNimSetupID,
	kColumnsNimSetupID,
	kTurnSetupID,
	kSilverSetupID,
	kCornerSetupID
};

enum	/* generic dialog item nums */
{
	kOKItem=1,
	kCancelItem,
	kUserItem
};

enum	/* dialog item nums for nim & prime nim setup */
{
	kNimNumRowsItem=4,
	kNimMaxPerRowItem,
	kNimPopupItem
};

enum	/* dialog item nums for columns nim setup */
{
	kColumnsNumRowsItem=4,
	kColumnsNumColumnsItem,
	kColumnsDistributionItem
};

enum	/* dialog item nums for Turnablock setup */
{
	kTurnNumRowsItem=4,
	kTurnNumColumnsItem,
	kTurnDistributionItem
};

enum	/* dialog item nums for Silver Dollar setup */
{
	kSilverNumPlacesItem=4,
	kSilverNumTilesItem,
	kSilverDistributionItem
};

enum	/* dialog item nums for queen setup */
{
	kCornerNumRowsItem=4,
	kCornerNumColumnsItem,
	kCornerStartItem
};

#define kCornerSetupTitleResID	300

static	void GetCornerSetupTitle(Str255 theStr);

void DoNimSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kNimSetupID, kUserItem, "\pNim setup",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kNimNumRowsItem, kNimNumRowsMin, kNimNumRowsMax, gNimNumRows);
	SetDialogControlInfo(theDialog, kNimMaxPerRowItem, kNimMaxPerRowMin, kNimMaxPerRowMax,
		gNimMaxPerRow);
	SetDialogControlValue(theDialog, kNimPopupItem, gNimDistribution);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gNimNumRows=GetDialogControlValue(theDialog, kNimNumRowsItem);
		gNimMaxPerRow=GetDialogControlValue(theDialog, kNimMaxPerRowItem);
		gNimDistribution=GetDialogControlValue(theDialog, kNimPopupItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

void DoPrimeSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kPrimeSetupID, kUserItem, "\pPrime Nim setup",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kNimNumRowsItem, kNimNumRowsMin, kNimNumRowsMax, gPrimeNumRows);
	SetDialogControlInfo(theDialog, kNimMaxPerRowItem, kNimMaxPerRowMin, kNimMaxPerRowMax,
		gPrimeMaxPerRow);
	SetDialogControlValue(theDialog, kNimPopupItem, gPrimeDistribution);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gPrimeNumRows=GetDialogControlValue(theDialog, kNimNumRowsItem);
		gPrimeMaxPerRow=GetDialogControlValue(theDialog, kNimMaxPerRowItem);
		gPrimeDistribution=GetDialogControlValue(theDialog, kNimPopupItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

void DoColumnsSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kColumnsNimSetupID, kUserItem, "\pNorthcroft’s Nim setup",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kColumnsNumRowsItem, kColumnsNumRowsMin, kColumnsNumRowsMax,
		gColumnsNumRows);
	SetDialogControlInfo(theDialog, kColumnsNumColumnsItem, kColumnsNumColumnsMin,
		kColumnsNumColumnsMax, gColumnsNumColumns);
	SetDialogControlValue(theDialog, kColumnsDistributionItem, gColumnsDistribution);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gColumnsNumRows=GetDialogControlValue(theDialog, kColumnsNumRowsItem);
		gColumnsNumColumns=GetDialogControlValue(theDialog, kColumnsNumColumnsItem);
		gColumnsDistribution=GetDialogControlValue(theDialog, kColumnsDistributionItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

void DoTurnSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kTurnSetupID, kUserItem, "\pTurn-a-block setup",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kTurnNumRowsItem, kTurnNumRowsMin, kTurnNumRowsMax,
		gTurnNumRows);
	SetDialogControlInfo(theDialog, kTurnNumColumnsItem, kTurnNumColumnsMin,
		kTurnNumColumnsMax, gTurnNumColumns);
	SetDialogControlValue(theDialog, kTurnDistributionItem, gTurnDistribution);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gTurnNumRows=GetDialogControlValue(theDialog, kTurnNumRowsItem);
		gTurnNumColumns=GetDialogControlValue(theDialog, kTurnNumColumnsItem);
		gTurnDistribution=GetDialogControlValue(theDialog, kTurnDistributionItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

void DoSilverSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	
	theDialog=SetupTheDialog(kSilverSetupID, kUserItem, "\pMy daddy... setup",
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kSilverNumPlacesItem, kSilverNumPlacesMin, kSilverNumPlacesMax,
		gSilverNumPlaces);
	SetDialogControlInfo(theDialog, kSilverNumTilesItem, kSilverNumTilesMin, kSilverNumTilesMax,
		gSilverNumTiles);
	SetDialogControlValue(theDialog, kSilverDistributionItem, gSilverDistribution);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gSilverNumPlaces=GetDialogControlValue(theDialog, kSilverNumPlacesItem);
		gSilverNumTiles=GetDialogControlValue(theDialog, kSilverNumTilesItem);
		gSilverDistribution=GetDialogControlValue(theDialog, kSilverDistributionItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

void DoCornerSetup(void)
{
	DialogPtr		theDialog;
	short			itemSelected;
	UniversalProcPtr	modalFilterProc, userItemProc;
	Str255			titleStr;
	
	GetCornerSetupTitle(titleStr);
	theDialog=SetupTheDialog(kCornerSetupID, kUserItem, titleStr,
		(UniversalProcPtr)TwoButtonFilter, &modalFilterProc, &userItemProc);
	SetDialogControlInfo(theDialog, kCornerNumRowsItem, kCornerNumRowsMin, kCornerNumRowsMax,
		gCornerInfo[TheCornerIndex].numRows);
	SetDialogControlInfo(theDialog, kCornerNumColumnsItem, kCornerNumColumnsMin, kCornerNumColumnsMax,
		gCornerInfo[TheCornerIndex].numColumns);
	SetDialogControlValue(theDialog, kCornerStartItem, gCornerInfo[TheCornerIndex].start);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=kOKItem) && (itemSelected!=kCancelItem))
	{
		itemSelected=GetOneDialogEvent(modalFilterProc);
	}
	
	if (itemSelected==kOKItem)
	{
		gCornerInfo[TheCornerIndex].numRows=GetDialogControlValue(theDialog, kCornerNumRowsItem);
		gCornerInfo[TheCornerIndex].numColumns=GetDialogControlValue(theDialog, kCornerNumColumnsItem);
		gCornerInfo[TheCornerIndex].start=GetDialogControlValue(theDialog, kCornerStartItem);
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
}

static	void GetCornerSetupTitle(Str255 theStr)
{
	GetIndString(theStr, kCornerSetupTitleResID, TheCornerIndex+1);
}
