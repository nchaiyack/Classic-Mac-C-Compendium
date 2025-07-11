// UStructureInspectors.h
//  Copyright  1984-1991 by Apple Computer Inc.	All rights reserved.

#ifndef __USTRUCTUREINSPECTORS__
#define __USTRUCTUREINSPECTORS__

#ifndef __UGRIDVIEW__
#include <UGridView.h>
#endif

class TIdler : public TEventHandler
{
public:
	TEventHandler *fHandler;

	TIdler();
	virtual pascal void Initialize();	// Override 
	virtual pascal void IIdler(TEventHandler *itsHandler);
	virtual pascal Boolean DoIdle(IdlePhase phase);	// Override 
};

class TObjectListView : public TTextListView
{
public:
	TList  *fAList;
	TList  *fBList;
	TList  *fDisplayedList;
	TIdler *fIdler;

	TObjectListView();
	virtual pascal void Initialize();			// override 
	virtual pascal void DoPostCreate(TDocument *itsDocument);// override 
	virtual pascal void Free();	// Override 
	virtual pascal void BuildList(TList *newList) ;
	virtual pascal Boolean IsSynchronized();
	virtual pascal void Synchronize(Boolean redraw);
	virtual pascal Boolean DoIdle(IdlePhase phase);	// override 
	virtual pascal void Draw(const VRect &area);
	virtual pascal void Open();	// override 
	virtual pascal void Close();	// override 
	virtual pascal TObject *GetSelectedObject();
	virtual pascal TObject *GetNthObject(short n);
	virtual pascal void SelectObject(TObject *anObject);
};

class TViewHierarchyView : public TObjectListView
{
public:
	virtual pascal void FlattenHierarchy(TView *root, TList *theList);
	virtual pascal void BuildList(TList *newList) ;	// Override
	virtual pascal void GetItemText(short	anItem,
									CStr255 &aString);// override 
	virtual pascal TView *GetSelectedView();
	TViewHierarchyView();
};

class TTargetChainView : public TObjectListView
{
public:
	virtual pascal void BuildList(TList *newList) ;	// Override
	virtual pascal void GetItemText(short	anItem,
									CStr255&	aString);// override 
	virtual pascal TEventHandler *GetSelectedHandler();
	TTargetChainView();
};

#endif
