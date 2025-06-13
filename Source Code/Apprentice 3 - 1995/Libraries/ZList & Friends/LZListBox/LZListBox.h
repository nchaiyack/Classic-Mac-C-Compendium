// ===========================================================================
//	LZListBox.h						©1995 Chris K. Thomas. All rights reserved.
// ===========================================================================

#pragma once

#include <LPane.h>
#include <LCommander.h>
//#include <LBroadcaster.h>
#include <LDragAndDrop.h>
#include "ZList.h"

#ifndef __LISTS__
#include <Lists.h>
#endif

class	LFocusBox;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif


#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif


class LZListBox : public LPane,
				 public LCommander,
				 public LDragAndDrop
//				 public LBroadcaster
				  {
public:
	enum { class_ID = 'zbox' };

					LZListBox(const SPaneInfo &inPaneInfo,
						ListFeature inFeatures, Int16 inTextTraitsID,
						LCommander *inSuper);
	virtual			~LZListBox();
				
	ZList *			GetMacZList() const;
	LFocusBox *		GetFocusBox();
	
	virtual Int32	GetValue() const;
	virtual void	SetValue(Int32 inValue);
	
	virtual StringPtr	GetDescriptor(Str255 outDescriptor) const;
	virtual void		SetDescriptor(ConstStr255Param inDescriptor);
	
	virtual Boolean	FocusDraw();
	virtual Boolean	ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual Boolean	HandleKeyPress(const EventRecord& inKeyEvent);
	
	virtual void	ResizeFrameBy(Int16 inWidthDelta, Int16 inHeightDelta,
									Boolean inRefresh);
	
	virtual void	MoveBy(Int32 inHorizDelta, Int32 inVertDelta,
									Boolean inRefresh);

	
protected:
	ZList			*mMacZList;
	LFocusBox		*mFocusBox;
	ResIDT			mTextTraitsID;

	virtual void	InsideDropArea(DragReference inDragRef);
	virtual Boolean	DragIsAcceptable(DragReference inDragRef);	
	virtual void	DoDragReceive(DragReference inDragRef);
	
	virtual void	DrawSelf();	
	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);
	
	virtual void	BeTarget();
	virtual void	DontBeTarget();
	
	virtual void	ActivateSelf();
	virtual void	DeactivateSelf();
	
	virtual void	DoNavigationKey(const EventRecord &inKeyEvent);
	
private:
	void			InitListBox(ListFeature, Int16,
						Int16 inTextTraitsID);
};
