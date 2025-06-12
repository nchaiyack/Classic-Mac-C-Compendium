/* CHScrollBar.c */

#include "CHScrollBar.h"
#include "CWindow.h"


#define LeftArrowID (131)
#define LeftArrowSelectedID (135)
#define RightArrowID (130)
#define RightArrowSelectedID (134)


void			CHScrollBar::IHScrollBar(LongPoint Start, LongPoint Extent,
								CViewRect* TheOwner, CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		Extent.y = 16;
		Owner = TheOwner;
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
		NumCells = 1;
		CellIndex = 0;
		RecalcScrollRects();
		CurrentOperation = mNone;
	}


void			CHScrollBar::DoMouseDown(MyEventRec Event)
	{
		long				Length;
		LongPoint		MouseLoc;
		MyBoolean		OutOfRange;
		short				XorPosition;
		MyBoolean		ImageXored;

		CurrentOperation = FindPart(MyGlobalToLocal(Event.Where));
		switch (CurrentOperation)
			{
				case mLeftOne:
					if (HScrollable && Enabled)
						{
							do{
									RedrawHBar();
									if (CurrentOperation == mLeftOne)
										{
											Owner->Hook(HScrollLeftOne,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mLeftOne)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawHBar();
						}
					break;
				case mLeftPage:
					if (HScrollable && Enabled)
						{
							do{
									RedrawHBar();
									if (CurrentOperation == mLeftPage)
										{
											Owner->Hook(HScrollLeftPage,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mLeftPage)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawHBar();
						}
					break;
				case mRightOne:
					if (HScrollable && Enabled)
						{
							do{
									RedrawHBar();
									if (CurrentOperation == mRightOne)
										{
											Owner->Hook(HScrollRightOne,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mRightOne)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawHBar();
						}
					break;
				case mRightPage:
					if (HScrollable && Enabled)
						{
							do{
									RedrawHBar();
									if (CurrentOperation == mRightPage)
										{
											Owner->Hook(HScrollRightPage,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mRightPage)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawHBar();
						}
					break;
				case mHBox:
					if (HScrollable && Enabled)
						{
							if ((Event.Modifiers & optionKey) != 0)
								{
									short		OldMouseLoc = -32767;

									ImageXored = False;
									/* setting up drawing environment */
									SetUpPort();
									Length = PageRightZoneExt.x + PageLeftZoneExt.x;
									do{
											static LongPoint	HXorBoxExt = {16,14};

											/* removing any image already there */
											do
												{
													MouseLoc = MyGlobalToLocal(GetLongMouseLoc());
													RelinquishCPU();
													SendIdleToAll();
												} while ((MouseLoc.x - 16 - 8 == OldMouseLoc) && WaitMouseUp());
											SetUpPort();
											Window->ResetPen();
											Window->SetPenMode(srcXor);
											Window->SetPattern(dkGray);
											if (ImageXored)
												{
													Window->LFrameRect(LongPointOf(XorPosition + 16,
														Extent.y - 16 + 1),HXorBoxExt);
													ImageXored = False;
												}
											/* finding new image position */
											OutOfRange = False;
											OldMouseLoc = MouseLoc.x - 16 - 8;
											XorPosition = OldMouseLoc;
											if ((MouseLoc.y < Extent.y - 16 - 24)
												|| (MouseLoc.y > Extent.y + 24))
												{
													OutOfRange = True;
												}
											if (XorPosition < 0)
												{
													XorPosition = 0;
												}
											if (XorPosition > Length)
												{
													XorPosition = Length;
												}
											/* putting new image position */
											if (!OutOfRange)
												{
													Window->LFrameRect(LongPointOf(XorPosition + 16,
														Extent.y - 16 + 1),HXorBoxExt);
													ImageXored = True;
												}
											RelinquishCPU();
											SendIdleToAll();
											SetUpPort();
										} while (WaitMouseUp());
									if (ImageXored)
										{
											Owner->Hook(HScrollToLocation,
												((NumCells - 1) * XorPosition) / Length,NumCells);
										}
								}
							 else
								{
									do{
											Length = PageLeftZoneExt.x + PageRightZoneExt.x;
											MouseLoc = MyGlobalToLocal(GetLongMouseLoc());
											MouseLoc.x = MouseLoc.x - 16 - 8;
											if (MouseLoc.x < 0)
												{
													MouseLoc.x = 0;
												}
											if (MouseLoc.x > Length)
												{
													MouseLoc.x = Length;
												}
											Owner->Hook(HScrollToLocation,((NumCells - 1) * MouseLoc.x
												+ (Length / 2)) / Length,NumCells);
											RelinquishCPU();
											SendIdleToAll();
										} while (WaitMouseUp());
								}
						}
					break;
			}
		CurrentOperation = mNone;
	}


short			CHScrollBar::FindPart(LongPoint Where)
	{
		if (LongPtInRect(Where,OneLeftZoneTL,OneLeftZoneExt))
			{
				return mLeftOne;
			}
		if (LongPtInRect(Where,PageLeftZoneTL,PageLeftZoneExt))
			{
				return mLeftPage;
			}
		if (LongPtInRect(Where,HScrollZoneTL,HScrollZoneExt))
			{
				return mHBox;
			}
		if (LongPtInRect(Where,PageRightZoneTL,PageRightZoneExt))
			{
				return mRightPage;
			}
		if (LongPtInRect(Where,OneRightZoneTL,OneRightZoneExt))
			{
				return mRightOne;
			}
		return mNone;
	}


void			CHScrollBar::RedrawHBar(void)
	{
		SetUpPort();
		Window->ResetPen();
		if (Suspended || !Enabled)
			{
				Window->LEraseRect(LongPointOf(1,1),LongPointOf(Extent.x - 2,Extent.y - 2));
				Window->LFrameRect(ZeroPoint,Extent);
			}
		 else
			{
				if (CurrentOperation == mLeftOne)
					{
						Window->LDrawPicture(GetPic(LeftArrowSelectedID),
							OneLeftZoneTL,OneLeftZoneExt);
					}
				 else
					{
						Window->LDrawPicture(GetPic(LeftArrowID),
							OneLeftZoneTL,OneLeftZoneExt);
					}
				if (CurrentOperation == mRightOne)
					{
						Window->LDrawPicture(GetPic(RightArrowSelectedID),
							OneRightZoneTL,OneRightZoneExt);
					}
				 else
					{
						Window->LDrawPicture(GetPic(RightArrowID),
							OneRightZoneTL,OneRightZoneExt);
					}
				if (HScrollable)
					{
						Window->SetPattern(ltGray);
						Window->LPaintRect(LongPointOf(PageLeftZoneTL.x,PageLeftZoneTL.y + 1),
							LongPointOf(PageLeftZoneExt.x,PageLeftZoneExt.y - 2));
						Window->LPaintRect(LongPointOf(PageRightZoneTL.x,PageRightZoneTL.y + 1),
							LongPointOf(PageRightZoneExt.x,PageRightZoneExt.y - 2));
						Window->SetPattern(black);
						Window->DrawLine(PageLeftZoneTL,LongPointOf(PageLeftZoneExt.x
							+ HScrollZoneExt.x + PageRightZoneExt.x,0));
						Window->DrawLine(LongPointOf(PageLeftZoneTL.x,PageLeftZoneTL.y + 15),
							LongPointOf(PageLeftZoneExt.x + HScrollZoneExt.x + PageRightZoneExt.x,0));
						Window->LEraseRect(LongPointOf(HScrollZoneTL.x + 1,HScrollZoneTL.y + 1),
							LongPointOf(HScrollZoneExt.x - 2,HScrollZoneExt.y - 2));
						Window->LFrameRect(HScrollZoneTL,HScrollZoneExt);
					}
				 else
					{
						Window->LEraseRect(LongPointOf(HScrollZoneTL.x,HScrollZoneTL.y + 1),
							LongPointOf(HScrollZoneExt.x,HScrollZoneExt.y - 2));
						Window->DrawLine(HScrollZoneTL,LongPointOf(HScrollZoneExt.x,0));
						Window->DrawLine(LongPointOf(HScrollZoneTL.x,HScrollZoneTL.y + 15),
							LongPointOf(HScrollZoneExt.x,0));
					}
			}
	}


void			CHScrollBar::DoUpdate(void)
	{
		RedrawHBar();
	}


void			CHScrollBar::DoResume(void)
	{
		inherited::DoResume();
		RedrawHBar();
	}


void			CHScrollBar::DoSuspend(void)
	{
		inherited::DoSuspend();
		RedrawHBar();
	}


void			CHScrollBar::DoEnable(void)
	{
		inherited::DoEnable();
		RedrawHBar();
	}


void			CHScrollBar::DoDisable(void)
	{
		inherited::DoDisable();
		RedrawHBar();
	}



void				CHScrollBar::RecalcScrollRects(void)
	{
		long		Length;
		long		BoxPos;

		Length = Extent.x;
		OneLeftZoneTL.y = Extent.y - 16;
		OneLeftZoneTL.x = 0;
		OneLeftZoneExt.y = 16;
		OneLeftZoneExt.x = 16;
		OneRightZoneTL.y = Extent.y - 16;
		OneRightZoneTL.x = Length - 16;
		OneRightZoneExt.y = 16;
		OneRightZoneExt.x = 16;
		if (NumCells <= 1)
			{
				if (CellIndex == 0)
					{
						HScrollable = False;
						HScrollZoneTL.y = Extent.y - 16;
						HScrollZoneTL.x = 16;
						HScrollZoneExt.y = 16;
						HScrollZoneExt.x = Length - 16 - 16;
					}
				 else
					{
						NumCells = -NumCells;
						goto TryAgainPoint;
					}
			}
		 else
			{
			 TryAgainPoint:
				HScrollable = True;
				if (NumCells == 1)
					{
						if (CellIndex < 0)
							{
								BoxPos = 0;
							}
						 else
							{
								BoxPos = Length - 3*16;
							}
					}
				 else
					{
						BoxPos = ((Length - 3*16)
							* ((long double)CellIndex) / ((long double)(NumCells - 1)));
						if (BoxPos < 0)
							{
								BoxPos = 0;
							}
						if (BoxPos > Length - 3*16)
							{
								BoxPos = Length - 3*16;
							}
					}
				PageLeftZoneTL.y = Extent.y - 16;
				PageLeftZoneTL.x = 16;
				PageLeftZoneExt.y = 16;
				PageLeftZoneExt.x = BoxPos;
				HScrollZoneTL.y = Extent.y - 16 + 1;
				HScrollZoneTL.x = BoxPos + 16;
				HScrollZoneExt.y = 14;
				HScrollZoneExt.x = 16;
				PageRightZoneTL.y = Extent.y - 16;
				PageRightZoneTL.x = BoxPos + 16 + 16;
				PageRightZoneExt.y = 16;
				PageRightZoneExt.x = (Length - 3*16) - BoxPos;
			}
	}


void			CHScrollBar::SetPosition(long NewCellIndex, long NewNumCells)
	{
		CellIndex = NewCellIndex;
		NumCells = NewNumCells;
		RecalcScrollRects();
		if ((!Suspended) && Enabled)
			{
				RedrawHBar();
			}
	}


void			CHScrollBar::RecalcLocations(LongPoint EnclosureVisRectStart,
						LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin)
	{
		inherited::RecalcLocations(EnclosureVisRectStart,EnclosureVisRectExtent,
			EnclosureOrigin);
		RecalcScrollRects();
	}
