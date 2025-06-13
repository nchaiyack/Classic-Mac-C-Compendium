/*
  Mac interface for GNU Chess

  Revision: 10 Feb 1991

  Copyright (C) 1986, 1987, 1988 Free Software Foundation, Inc.
  Copyright (c) 1991  Airy ANDRE

	expanded game save, list, and restore features
	optional auto-updating of positional information

  This file is part of CHESS.

  CHESS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.  No author or distributor
  accepts responsibility to anyone for the consequences of using it
  or for whether it serves any particular purpose or works at all,
  unless he says so in writing.  Refer to the CHESS General Public
  License for full details.

  Everyone is granted permission to copy, modify and redistribute
  CHESS, but only under the conditions described in the
  CHESS General Public License.   A copy of this license is
  supposed to have been given to you along with CHESS so you
  can know your rights and responsibilities.  It should be in a
  file named COPYING.  Among other things, the copyright notice
  and this notice must be preserved on all copies.
  */

/* This file is based on DragManager by Boyd Scott - MacTutor Vol.3 No.7 */

#include "DragMgr.h"


#define Shadow_x 4
#define Shadow_y 6

ShadowRecord ShadowStuff;
BitMap OffScreenBits;

static GrafPtr wMgrPort,OldPort, OffPort;
static RgnHandle UpdateRegion;
static Rect LastRect,OtherUpdateRect,UpdateRect,DragRect,tempBounds, globRect;
static Point  MousePt,OtherMousePt,TestPt;
static long  SynchCount;
  

static Ptr NewBitMap(BitMap *theBitMap, Rect *theRect)
{
 	theBitMap->rowBytes=((theRect->right-theRect->left+15)/16)*2;
 	theBitMap->baseAddr=NewPtr(theBitMap->rowBytes
 									*(theRect->bottom-theRect->top));
 	theBitMap->bounds.top =  theRect->top;
 	theBitMap->bounds.bottom =  theRect->bottom;
 	theBitMap->bounds.left =  theRect->left;
 	theBitMap->bounds.right =  theRect->right;
 	if (MemError()!=noErr) return 0L;
 	return theBitMap->baseAddr;
}

Boolean InitDrag(BitMapPtr UserOffScreenBits, Rect * Taille)
{
  int temp = 0;
  
  globRect = *Taille;
  GetPort(&OldPort);
  if (UserOffScreenBits)
  		OffScreenBits = *UserOffScreenBits;
	 else
		if (!NewBitMap(&OffScreenBits,Taille)) return 0;
  OffPort = (GrafPtr)NewPtr(sizeof(GrafPort));
  if (!OffPort) 
  {
  		DisposPtr(OffScreenBits.baseAddr);
	 	return 0;
  }
  OpenPort(OffPort);
  RectRgn(OldPort->clipRgn,&(**OldPort->clipRgn).rgnBBox);
  UpdateRegion = NewRgn();
  if (!UpdateRegion) 
  {
  		DisposPtr(OffScreenBits.baseAddr);
	 	ClosePort(OffPort);
	 	DisposPtr((Ptr)OffPort);
	 	SetPort(OldPort);
	 	return 0;
  }
	
  
  ShadowStuff.visible = 1;
  ShadowStuff.dx = Shadow_x;
  ShadowStuff.dy = Shadow_y;
  ShadowStuff.CopyMode=srcOr;
   
  CopyBits(&OldPort->portBits, &OffScreenBits, Taille, Taille,
  					srcCopy, OldPort->visRgn);
  ClipRect(Taille);
  SetPort(OldPort);
  return 1;
}

static Boolean TestNil(Ptr theThing, DragHandle DragStuff)
{
  if (!theThing)
  {
  	DisposeDraggable(DragStuff);
 	SetPort(OldPort);
 	return 1;
  }
  return 0;
}

Boolean NewDraggable(PicHandle thePicture, 
							PicHandle theMask,
							BitMapPtr UserPicBits,
							BitMapPtr UserShadowBits,
							DragHandle *DragStuff)
{
  GetPort(&OldPort);
  SetPort(OffPort);
  SetPortBits(&OffScreenBits);
  *DragStuff = (DragHandle)NewHandle(sizeof(DragRecord));
  if (!(*DragStuff))
  {
	 	SetPort(OldPort);
	 	return 0;
  }
  
  MoveHHi((Handle)*DragStuff);
  HLock((Handle)*DragStuff);
  
  tempBounds = (**thePicture).picFrame;
  if (UserPicBits) (***DragStuff).PictureBits = *UserPicBits;
	 else if (TestNil((Ptr)NewBitMap(&(***DragStuff).PictureBits,&tempBounds),
	 			*DragStuff)) return 0;
  OffsetRect(&tempBounds,-tempBounds.left,-tempBounds.top);
  if (TestNil(NewBitMap(&(***DragStuff).UnderBits,&tempBounds),*DragStuff))  return 0;
  tempBounds = (**theMask).picFrame;

  if (UserShadowBits) (***DragStuff).ShadowBits = *UserShadowBits;
	 else if (TestNil((Ptr)NewBitMap(&(***DragStuff).ShadowBits,&tempBounds),*DragStuff)) return 0;
  OffsetRect(&tempBounds,-tempBounds.left,-tempBounds.top);
  if (TestNil((Ptr)NewBitMap(&(***DragStuff).UnderShadowBits,&tempBounds),*DragStuff))  return 0;
  SetPortBits(&(***DragStuff).PictureBits);
  EraseRect(&(***DragStuff).PictureBits.bounds);
	 
  (***DragStuff).ShadowRegion = NewRgn();
  if (TestNil((Ptr)&(***DragStuff).ShadowRegion,*DragStuff)) return 0;
  (***DragStuff).ThePictureRgn = NewRgn();
  if (TestNil((Ptr)(***DragStuff).ThePictureRgn,*DragStuff)) return 0;

  PenMode(patCopy);
  OpenRgn();
  HLock((Handle)thePicture);
  DrawPicture(thePicture,&(**thePicture).picFrame);
  HUnlock((Handle)thePicture);
  CloseRgn((***DragStuff).ThePictureRgn);

  if (EmptyRgn((***DragStuff).ThePictureRgn))
	 		RectRgn((***DragStuff).ThePictureRgn,&(**thePicture).picFrame);
  SetPortBits(&(***DragStuff).PictureBits);
  PenMode(patCopy);
  HLock((Handle)thePicture);
  DrawPicture(thePicture,&(**thePicture).picFrame);
  HUnlock((Handle)thePicture);
	 
  SetPortBits(&(***DragStuff).ShadowBits);
  PenMode(patCopy);
  EraseRect(&(***DragStuff).ShadowBits.bounds);

  CopyRgn((***DragStuff).ThePictureRgn,(***DragStuff).ShadowRegion);

  HLock((Handle)theMask);
  DrawPicture(theMask,&(**theMask).picFrame);
  HUnlock((Handle)theMask);
	 
  OffsetRgn((***DragStuff).ShadowRegion,ShadowStuff.dx,ShadowStuff.dy);
  DragRect = (***DragStuff).PictureBits.bounds;
  SetRect(&LastRect,0,0,0,0);

  HUnlock((Handle)*DragStuff);
  SetPort(OldPort);
  return 1;
}

void DragItTo(DragHandle DragStuff, Point MousePt, Boolean Centered, Boolean Last)
{
	GetPort(&OldPort);

	SetPort(OffPort);
	
	MoveHHi((Handle)DragStuff);
	HLock((Handle)DragStuff);


	OtherMousePt = MousePt;
	OffsetRgn((**DragStuff).ThePictureRgn,-DragRect.left,-DragRect.top);
	OffsetRgn((**DragStuff).ShadowRegion,-DragRect.left,-DragRect.top);
	OffsetRect(&DragRect,-DragRect.left,-DragRect.top); 
	if (Centered) 
	{
		MousePt.h = MousePt.h-DragRect.right/2;
	 	MousePt.v = MousePt.v-DragRect.bottom/2;
	}
  OffsetRect(&DragRect,MousePt.h,MousePt.v);
  OffsetRgn((**DragStuff).ThePictureRgn,MousePt.h,MousePt.v);
  OffsetRgn((**DragStuff).ShadowRegion,MousePt.h,MousePt.v);
  
  if (ShadowStuff.visible)
  {
	  OffsetRect(&DragRect,ShadowStuff.dx,ShadowStuff.dy);
	  if (SectRect(&globRect,&DragRect,&UpdateRect))  {};
	  OtherUpdateRect=UpdateRect;
	  OffsetRect(&OtherUpdateRect,-OtherUpdateRect.left,-OtherUpdateRect.top);
	  CopyBits(&OffScreenBits,&(**DragStuff).UnderShadowBits,&UpdateRect,&OtherUpdateRect,
	  					srcCopy,0L);
	  OffsetRect(&DragRect,-ShadowStuff.dx,-ShadowStuff.dy);
  }
  
  if (SectRect(&globRect,&DragRect,&UpdateRect))  {};
  OtherUpdateRect=UpdateRect;
  OffsetRect(&OtherUpdateRect,-OtherUpdateRect.left,-OtherUpdateRect.top);
  CopyBits(&OffScreenBits,&(**DragStuff).UnderBits,&UpdateRect,&OtherUpdateRect,
  					srcCopy,0L);
	
  if (ShadowStuff.visible)  {
  	OffsetRect(&DragRect,ShadowStuff.dx, ShadowStuff.dy);
  	CopyBits(&(**DragStuff).ShadowBits, &OffScreenBits,
  							&(**DragStuff).ShadowBits.bounds, &DragRect,
	 						ShadowStuff.CopyMode, 0L /*(*OldPort).clipRgn*/);
 	OffsetRect(&DragRect,-ShadowStuff.dx,-ShadowStuff.dy);
  }
  
  CopyBits(&(**DragStuff).ShadowBits,&OffScreenBits,
  					&(**DragStuff).ShadowBits.bounds, &DragRect,
  					srcOr, 0L/*(*OldPort).clipRgn*/);
  CopyBits(&(**DragStuff).PictureBits,&OffScreenBits,
  					&(**DragStuff).PictureBits.bounds,&DragRect,
  					notSrcBic,(**DragStuff).ThePictureRgn);

  UnionRect(&DragRect,&LastRect,&UpdateRect);
  
  if (ShadowStuff.visible)  {
	OffsetRect(&DragRect,ShadowStuff.dx,ShadowStuff.dy);
  	UnionRect(&DragRect,&UpdateRect,&UpdateRect);
    OffsetRect(&DragRect,-ShadowStuff.dx,-ShadowStuff.dy);
  }
  if( SectRect(&globRect,&UpdateRect,&UpdateRect))  {};
  RectRgn(UpdateRegion,&UpdateRect);
  
  SetPort(OldPort);
  SynchCount = TickCount();
  while (TickCount()==SynchCount);
  
  CopyBits(&OffScreenBits,&OldPort->portBits,&UpdateRect,&UpdateRect,
  					srcCopy,0L /*OldPort->clipRgn*/);

  if (!Last) {
	  SetPort(OffPort);
	
	  if (SectRect(&DragRect,&OffScreenBits.bounds,&UpdateRect))  {};
	  OtherUpdateRect=UpdateRect;
	  OffsetRect(&OtherUpdateRect,-OtherUpdateRect.left,-OtherUpdateRect.top);
	  CopyBits(&(**DragStuff).UnderBits,&OffScreenBits,&OtherUpdateRect,&UpdateRect,
	  					srcCopy,0L /*OldPort->clipRgn*/);
	  LastRect=DragRect;
	  if (ShadowStuff.visible)  {
		  OffsetRect(&DragRect,ShadowStuff.dx,ShadowStuff.dy);
		  if (SectRect(&DragRect,&OffScreenBits.bounds,&UpdateRect))  {};
		  OtherUpdateRect=UpdateRect;
		  OffsetRect(&OtherUpdateRect,-OtherUpdateRect.left,-OtherUpdateRect.top);
		  CopyBits(&(**DragStuff).UnderShadowBits,&OffScreenBits,&OtherUpdateRect,
		  			&UpdateRect, srcCopy, 0L /*(*OldPort).clipRgn*/);
		  UnionRect(&LastRect,&DragRect,&LastRect);
		  OffsetRect(&DragRect,-ShadowStuff.dx,-ShadowStuff.dy);  
	  }
		SetPort(OldPort);
	}
	HUnlock((Handle)DragStuff);
}

void DisposeDraggable(DragHandle DragStuff)
{
	GetPort(&OldPort);
	SetPort(OffPort);
	SetPortBits(&OffScreenBits);
	if (DragStuff)  {
		HLock((Handle)DragStuff);
		  if (OffScreenBits.baseAddr);
		  
		  if ((**DragStuff).PictureBits.baseAddr)  DisposPtr((**DragStuff).PictureBits.baseAddr);
		  if ((**DragStuff).UnderBits.baseAddr)  DisposPtr((**DragStuff).UnderBits.baseAddr);
		  if ((**DragStuff).UnderShadowBits.baseAddr)  DisposPtr((**DragStuff).UnderShadowBits.baseAddr);
		  if ((**DragStuff).ShadowBits.baseAddr)  DisposPtr((**DragStuff).ShadowBits.baseAddr);
		  if ((**DragStuff).ThePictureRgn)  DisposHandle((Handle)(**DragStuff).ThePictureRgn);
		  if ((**DragStuff).ShadowRegion)  DisposHandle((Handle)(**DragStuff).ShadowRegion);
		HUnlock((Handle)DragStuff);
		DisposHandle((Handle)DragStuff);
	}
	SetPort(OldPort);
}

void UpdateOffScreen(void)
{
	SetPort(OffPort);
	SetPortBits(&OffScreenBits);
}

void CloseDrag (Boolean DisposeBitMap)
{
	if (OffPort)
	{
		ClosePort(OffPort);
	  	DisposPtr((Ptr)OffPort);
	}
	if (UpdateRegion)  DisposHandle((Handle)UpdateRegion);
	if (DisposeBitMap)
		if (OffScreenBits.baseAddr)  DisposPtr(OffScreenBits.baseAddr);
}
