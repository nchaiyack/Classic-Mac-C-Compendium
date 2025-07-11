// UStructureInspectors.h
//  Copyright  1984-1991 by Apple Computer Inc.	All rights reserved.

#include "UStructureInspectors.h"

#ifndef __UAPPLICATION__
#include <UApplication.h>
#endif

#ifndef __UWINDOW__
#include <UWindow.h>
#endif

#ifndef __UMACAPPUTILITIES__
#include <UMacAppUtilities.h>
#endif

//**************************************************************************************************
//  TIdler														   
//**************************************************************************************************

//--------------------------------------------------------------------------------------------------
#pragma segment MyViewTools

TIdler::TIdler()
{
}

pascal void TIdler::Initialize()	// override 
{
	inherited::Initialize();
	
	fHandler = NULL;
}

//--------------------------------------------------------------------------------------------------

pascal void TIdler::IIdler(TEventHandler *itsHandler)	 
{
	this->IEventHandler(NULL);
	
	fHandler = itsHandler;
	this->SetIdleFreq(fHandler->GetIdleFreq());
}

//--------------------------------------------------------------------------------------------------

pascal Boolean TIdler::DoIdle(IdlePhase phase)	// override 
{
	if (fHandler)
		return fHandler->DoIdle(phase);
	else 
		return FALSE;
}

//**************************************************************************************************
//  TObjectListView														   
//**************************************************************************************************
//--------------------------------------------------------------------------------------------------

pascal Boolean ListsMatch(TList *firstList,TList *secondList)
{
	if (firstList->GetSize() != secondList->GetSize())
		return FALSE;
	else
	{
		//return EqualBlocks(*((Handle) firstList), *((Handle) secondList),GetHandleSize((Handle) firstList));
		for (ArrayIndex i = 1; i <= firstList->GetSize(); i++)
		{
			if (firstList->At(i) != secondList->At(i))
				return FALSE;
		}
		return TRUE;
	}
}

//--------------------------------------------------------------------------------------------------

TObjectListView::TObjectListView()
{
}

//--------------------------------------------------------------------------------------------------

pascal void TObjectListView::Initialize()	// override 
{
	inherited::Initialize();
	
	fAList = NULL;
	fBList = NULL;
	fDisplayedList = NULL;
	fIdler = NULL;
}

//--------------------------------------------------------------------------------------------------
pascal void TObjectListView::DoPostCreate(TDocument *itsDocument)// override 
{
	inherited::DoPostCreate(itsDocument);

	FailInfo fi;
	
	if (fi.Try())
	{
		fAList = NewList();
		fBList = NewList();
		fDisplayedList = fAList;
		
		TIdler *anIdler = new TIdler;
		anIdler->IIdler(this);
		fIdler = anIdler;
		fi.Success();
	}
	else
	{
		fAList = (TList*) FreeIfObject(fAList);
		fBList = (TList*) FreeIfObject(fBList);
		fIdler = (TIdler*) FreeIfObject(fIdler);
		fDisplayedList = NULL;
		fi.ReSignal();
	}
	
}

//--------------------------------------------------------------------------------------------------
pascal void TObjectListView::Free()	// Override 
{
	fAList = (TList*) FreeIfObject(fAList);
	fBList = (TList*) FreeIfObject(fBList);
	fIdler = (TIdler*) FreeIfObject(fIdler);
	
	inherited::Free();
}

//--------------------------------------------------------------------------------------------------

pascal void TObjectListView::BuildList(TList *newList) 
{
	newList->DeleteAll();
}

//--------------------------------------------------------------------------------------------------
pascal Boolean TObjectListView::IsSynchronized() 
{
	TList *theNewList;
	TList *theOldList;
	
	if (fDisplayedList == fAList)
	{
		theNewList = fBList;
		theOldList = fAList;
	}
	else
	{
		theNewList = fAList;
		theOldList = fBList;
	}
		
	this->BuildList(theNewList);
	return ListsMatch(theOldList,theNewList);
}

//--------------------------------------------------------------------------------------------------
pascal void TObjectListView::Synchronize(Boolean /* redraw */) 
{
	TList *theNewList;
	TList *theOldList;
	
	if (fDisplayedList == fAList)
	{
		theNewList = fBList;
		theOldList = fAList;
	}
	else
	{
		theNewList = fAList;
		theOldList = fBList;
	}

	this->BuildList(theNewList);
	if (!ListsMatch(theOldList,theNewList))
	{
		TObject *selectedObject = this->GetSelectedObject();
		
		if (theOldList->GetSize() != theNewList->GetSize())
		{
			this->DelItemLast(fNumOfRows);
			this->InsItemLast((short) theNewList->GetSize());
		}
		fDisplayedList = theNewList;
		if (selectedObject)
			this->SelectObject(selectedObject);
		this->ScrollSelectionIntoView(!kRedraw);
		this->ForceRedraw();
	}	
}

//--------------------------------------------------------------------------------------------------

pascal void TObjectListView::Draw(const VRect &area)		// Override 
{
	if (this->IsSynchronized())
		inherited::Draw(area);
	// Otherwise, let next idle synchronize
}

//--------------------------------------------------------------------------------------------------

pascal Boolean TObjectListView::DoIdle(IdlePhase /*phase*/)	// override 
{
	this->Synchronize(kRedraw);
	return FALSE;
}

//--------------------------------------------------------------------------------------------------

pascal void TObjectListView::Open()	// override 
{
	gApplication->InstallCohandler(fIdler,TRUE);
	inherited::Open();
}

//--------------------------------------------------------------------------------------------------

pascal void TObjectListView::Close()	// override 
{
	gApplication->InstallCohandler(fIdler,FALSE);
	inherited::Close();
}

//--------------------------------------------------------------------------------------------------
pascal TObject *TObjectListView::GetSelectedObject()
{
	return this->GetNthObject(this->FirstSelectedItem());
}

//--------------------------------------------------------------------------------------------------
pascal TObject *TObjectListView::GetNthObject(short n)
{
	if (fDisplayedList && (n > 0) && (n <= fDisplayedList->GetSize()))
		return fDisplayedList->At(n);
	else
		return NULL;
}

//--------------------------------------------------------------------------------------------------
pascal void TObjectListView::SelectObject(TObject *anObject)
{
	ArrayIndex index = fDisplayedList->GetIdentityItemNo(anObject);
	this->SelectItem((short) index,FALSE,TRUE,TRUE);
}

//**************************************************************************************************
//  TViewHierarchyView														   
//**************************************************************************************************

//--------------------------------------------------------------------------------------------------
TViewHierarchyView::TViewHierarchyView()
{
}

pascal void TViewHierarchyView::BuildList(TList *newList) // Override
{
	newList->DeleteAll();
	this->FlattenHierarchy(NULL,newList);
}

pascal void TViewHierarchyView::FlattenHierarchy(TView *root, TList *theList)
{
	if (root)
	{
		theList->InsertLast(root);

		CSubViewIterator iter(root);
			
		for (TView  *theSubView = iter.FirstSubView(); iter.More(); theSubView = iter.NextSubView())
		{
			this->FlattenHierarchy(theSubView, theList);
		}		
	}
	else
	{
		CWMgrIterator iter;
	
		for (WindowPtr aWinPtr = iter.FirstWMgrWindow(); iter.More(); aWinPtr = iter.NextWMgrWindow())
		{
			TWindow  *aWindow = gApplication->WMgrToWindow(aWinPtr);
			if (aWindow && (aWindow != this->GetWindow()))
				this->FlattenHierarchy(aWindow, theList);
		}
	}
}

//--------------------------------------------------------------------------------------------------

pascal short GetDepth(TView *aView)
{
	if (aView->fSuperView == NULL)
		return 0;
	else
		return 1 + GetDepth(aView->fSuperView);
}

//--------------------------------------------------------------------------------------------------

pascal void TViewHierarchyView::GetItemText(short		anItem,
										 CStr255&	aString)// override 
{	
	if (anItem <= fDisplayedList->GetSize())
	{
		TView *theView = (TView*) this->GetNthObject(anItem);

		if (theView)
		{
			MAName itsName;
			theView->GetClassName(itsName);		
			CStr255 indentString = "......................................................................................................................";
			indentString.Length() = GetDepth(theView) * 2;
			
			CStr255 IDString(theView->fIdentifier);		
			aString = indentString + itsName + " '" + IDString + "'";
		}
		else
			aString = "Error";
	}
}

//--------------------------------------------------------------------------------------------------
pascal TView *TViewHierarchyView::GetSelectedView()
{
	return (TView*) this->GetSelectedObject();
}

//**************************************************************************************************
//  TTargetChainView														   
//**************************************************************************************************

//--------------------------------------------------------------------------------------------------

pascal void FlattenTargetChain(TEventHandler *head, TList *theList)
{
	theList->InsertLast(head);
	if (head->fNextHandler)
		FlattenTargetChain(head->fNextHandler,theList);
}

//--------------------------------------------------------------------------------------------------

TTargetChainView::TTargetChainView()
{
}

pascal void TTargetChainView::BuildList(TList *newList) // Override
{
	newList->DeleteAll();
	FlattenTargetChain(gApplication->GetTarget(),newList);
}

//--------------------------------------------------------------------------------------------------

pascal void TTargetChainView::GetItemText(short		anItem,
										 CStr255&	aString)// override 
{	
	if (anItem <= fDisplayedList->GetSize())
	{
		TEventHandler *theHandler = (TEventHandler*) this->GetNthObject(anItem);

		if (theHandler)
		{
			MAName itsName;
			theHandler->GetClassName(itsName);		
			
			CStr255 IDString(theHandler->fIdentifier);		
			aString = itsName + " '" + IDString + "'";
		}
		else
			aString = "Error";
		
	}
}

//--------------------------------------------------------------------------------------------------
pascal TEventHandler *TTargetChainView::GetSelectedHandler()
{
	return (TEventHandler*) this->GetSelectedObject();
}
