#include "kant build dialogs.h"
#include "kant build lists.h"
#include "kant build files.h"
#include "dialog layer.h"
#include "key layer.h"
#include "error.h"
#include "menus.h"
#include "main.h"
#include "window layer.h"
#include "memory layer.h"

#define kTextItem			5
#define kEditRefDialog		401
#define kNewRefDialog		402
#define kDeleteAlert		403

static	Boolean DoRefDialogDispatch(Boolean isEdit, Boolean isRef, Str255 oldStr, Str255 newStr);
static	pascal Boolean RefModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
static	pascal Boolean InstantModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem);
static	Boolean CheckDeleteDialogDispatch(Boolean isRef, Str255 theName);

void DoDeleteRef(WindowRef theWindow)
{
	FSSpec			theFS;
	OSErr			oe;
	Str255			theName;
	short			highlightedIndex;
	
	if ((highlightedIndex=GetHighlightedReference())<0)
		return;
	
	theFS=GetWindowFS(theWindow);
	GetHighlightedReferenceName(theName);
	
	if (!CheckDeleteDialogDispatch(TRUE, theName))
		return;
	
	if ((oe=DeleteOneReference(theFS, theName))!=noErr)
	{
		HandleError(kCantDeleteReference, FALSE, FALSE);
		return;
	}
	
	RebuildInsertMenu();
	DeleteHighlightedReference();
}

void DoDeleteInstantiation(WindowRef theWindow)
{
	FSSpec			theFS;
	OSErr			oe;
	Str255			refName, theName;
	short			highlightedIndex;
	
	if ((highlightedIndex=GetHighlightedInstant())<0)
		return;
	
	theFS=GetWindowFS(theWindow);
	GetHighlightedInstantName(theName);
	if (!CheckDeleteDialogDispatch(FALSE, theName))
		return;
	
	GetHighlightedReferenceName(refName);
	if ((oe=DeleteOneInstantiation(theFS, refName, highlightedIndex+1))!=noErr)
	{
		HandleError(kCantDeleteInstantiation, FALSE, FALSE);
		return;
	}
	
	DeleteHighlightedInstantiation();
}

void DoNewRef(WindowRef theWindow)
{
	FSSpec			theFS;
	Str255			refName;
	OSErr			oe;
	
	if (!DoRefDialogDispatch(FALSE, TRUE, "\p", refName))
		return;
	
	theFS=GetWindowFS(theWindow);
	if (ReferenceNameExistsQQ(theFS, refName))
	{
		HandleError(kDuplicateReferenceName, FALSE, FALSE);
		return;
	}
	
	if ((oe=AddOneReference(theFS, refName))!=noErr)
	{
		HandleError(kCantCreateReference, FALSE, FALSE);
		return;
	}
	
	RebuildInsertMenu();
// if we add an "interesting" checkbox to ref dialogs, need to change second param below:
	AddReferenceToList(refName, FALSE, TRUE);
}

void DoNewInstantiation(WindowRef theWindow)
{
	FSSpec			theFS;
	Str255			instantName, refName;
	OSErr			oe;
	short			refIndex;
	
	if ((refIndex=GetHighlightedReference())<0)
		return;
	
	if (!DoRefDialogDispatch(FALSE, FALSE, "\p", instantName))
		return;
	
	GetHighlightedReferenceName(refName);
	
	theFS=GetWindowFS(theWindow);
	if ((oe=AddOneInstantiation(theFS, refName, instantName))!=noErr)
	{
		HandleError(kCantCreateInstantiation, FALSE, FALSE);
		return;
	}
	
	AddInstantToList(instantName, TRUE);
}

void DoEditRef(WindowRef theWindow)
{
	FSSpec			theFS;
	Str255			oldStr, newStr;
	OSErr			oe;
	short			highlightedIndex;
	
	if ((highlightedIndex=GetHighlightedReference())<0)
		return;
	
	theFS=GetWindowFS(theWindow);
	GetHighlightedReferenceName(oldStr);
	
	if (!DoRefDialogDispatch(TRUE, TRUE, oldStr, newStr))
		return;
	
	if (ReferenceNameExistsQQ(theFS, newStr))
	{
		HandleError(kDuplicateReferenceName, FALSE, FALSE);
		return;
	}
	
	if ((oe=ReplaceOneReference(theFS, oldStr, newStr))!=noErr)
	{
		HandleError(kCantReplaceReference, FALSE, FALSE);
		return;
	}
	
	RebuildInsertMenu();
// third param is "interesting" bit
	EditReferenceInList(highlightedIndex, newStr, FALSE);
}

void DoEditInstantiation(WindowRef theWindow)
{
	FSSpec			theFS;
	Str255			oldStr, newStr, refName;
	OSErr			oe;
	short			refIndex, instantIndex;
	
	if ((refIndex=GetHighlightedReference())<0)
		return;
	
	theFS=GetWindowFS(theWindow);
	GetHighlightedReferenceName(refName);
	GetHighlightedInstantName(oldStr);
	instantIndex=GetHighlightedInstant();
	
	if (!DoRefDialogDispatch(TRUE, FALSE, oldStr, newStr))
		return;
	
	if ((oe=ReplaceOneInstantiation(theFS, refName, instantIndex+1, newStr))!=noErr)
	{
		HandleError(kCantReplaceInstantiation, FALSE, FALSE);
		return;
	}
	
	EditInstantInList(instantIndex, newStr);
}

static	Boolean DoRefDialogDispatch(Boolean isEdit, Boolean isRef, Str255 oldStr, Str255 newStr)
/* returns FALSE if cancelled or unchanged */
{
	DialogPtr		theDialog;
	UniversalProcPtr	modalFilterProc, userItemProc;
	Boolean			changed;
	Str255			theStr;
	short			itemSelected;
	
	theDialog=SetupTheDialog(isEdit ? kEditRefDialog : kNewRefDialog, 3,
		isEdit ? (isRef ? "\pEdit reference" : "\pEdit instantiation") :
				 (isRef ? "\pNew reference" : "\pNew instantiation"),
		isRef ? (UniversalProcPtr)RefModalFilter : (UniversalProcPtr)InstantModalFilter,
		&modalFilterProc, &userItemProc);
	ParamText(isRef ? "\preference" : "\pinstantiation", "\p", isRef ? "\pname" : "\ptext", "\p");
	SetDialogItemString(theDialog, kTextItem, oldStr);
	HighlightDialogTextItem(theDialog, kTextItem, 0, 32767);
	DisplayTheDialog(theDialog, TRUE);
	
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		itemSelected=GetOneDialogEvent(theDialog, modalFilterProc);
	}
	
	changed=FALSE;
	
	if (itemSelected==1)
	{
		GetDialogItemString(theDialog, kTextItem, theStr);
		
		if ((isRef) && (theStr[0]==0x00))
			changed=FALSE;
		else
		{
			changed=(!Mymemcompare((Ptr)oldStr, (Ptr)theStr, oldStr[0]+1));
			if (changed)
			{
				Mymemcpy(newStr, theStr, theStr[0]+1);	/* return new name */
			}
		}
	}
	
	ShutDownTheDialog(theDialog, modalFilterProc, userItemProc);
	
	while (HandleSingleEvent(FALSE)) {};
	
	return changed;
}

static	pascal Boolean RefModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	unsigned char	theChar;
	
	if (PoorMansFilterProc(theDialog, theEvent, theItem, 0L))
		return TRUE;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar>=key_LeftArrow) && (theChar<=key_DownArrow))
				return FALSE;
			if (theChar==0x08)
				return FALSE;
			if ((theChar>='a') && (theChar<='z'))
				return FALSE;
			if ((theChar>='A') && (theChar<='Z'))
				return FALSE;
			if ((theChar>='0') && (theChar<='9'))
				return FALSE;
			if ((theChar=='-') || (theChar=='_'))
				return FALSE;
			if (theChar==' ')
			{
				theEvent->message='-';
				return FALSE;
			}
			
			theEvent->what=nullEvent;
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}

static	pascal Boolean InstantModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	return PoorMansFilterProc(theDialog, theEvent, theItem, 0L);
}

static	Boolean CheckDeleteDialogDispatch(Boolean isRef, Str255 theName)
{
	ModalFilterUPP	procFilter = NewModalFilterProc(TwoButtonFilter);
	short			result;
	
	result=DisplayTheAlert(kCautionAlert, kDeleteAlert,
		isRef ? "\preference" : "\pinstantiation", theName, "\p", "\p",
		(UniversalProcPtr)TwoButtonFilter);
	while (HandleSingleEvent(FALSE)) {};
	
	return (result==1);
}
