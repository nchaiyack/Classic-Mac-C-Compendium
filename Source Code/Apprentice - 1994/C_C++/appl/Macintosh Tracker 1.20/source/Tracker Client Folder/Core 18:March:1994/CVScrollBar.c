/* CVScrollBar.c */

#include "CVScrollBar.h"
#include "CWindow.h"


#define UpArrowID (128)
#define UpArrowSelectedID (132)
#define DownArrowID (129)
#define DownArrowSelectedID (133)


void			CVScrollBar::IVScrollBar(LongPoint Start, LongPoint Extent,
								CViewRect* TheOwner, CWindow* TheWindow, CEnclosure* TheEnclosure)
	{
		Extent.x = 16;
		Owner = TheOwner;
		IViewRect(Start,Extent,TheWindow,TheEnclosure);
		NumCells = 1;
		CellIndex = 0;
		RecalcScrollRects();
		CurrentOperation = mNone;
	}


void			CVScrollBar::DoMouseDown(MyEventRec Event)
	{
		long				Length;
		LongPoint		MouseLoc;
		MyBoolean		OutOfRange;
		short				XorPosition;
		MyBoolean		ImageXored;

		CurrentOperation = FindPart(MyGlobalToLocal(Event.Where));
		switch (CurrentOperation)
			{
				case mUpOne:
					if (VScrollable && Enabled)
						{
							do{
									RedrawVBar();
									if (CurrentOperation == mUpOne)
										{
											Owner->Hook(VScrollUpOne,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mUpOne)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawVBar();
						}
					break;
				case mUpPage:
					if (VScrollable && Enabled)
						{
							do{
									RedrawVBar();
									if (CurrentOperation == mUpPage)
										{
											Owner->Hook(VScrollUpPage,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mUpPage)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawVBar();
						}
					break;
				case mDownOne:
					if (VScrollable && Enabled)
						{
							do{
									RedrawVBar();
									if (CurrentOperation == mDownOne)
										{
											Owner->Hook(VScrollDownOne,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mDownOne)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawVBar();
						}
					break;
				case mDownPage:
					if (VScrollable && Enabled)
						{
							do{
									RedrawVBar();
									if (CurrentOperation == mDownPage)
										{
											Owner->Hook(VScrollDownPage,0,0);
										}
									CurrentOperation = FindPart(MyGlobalToLocal(GetLongMouseLoc()));
									if (CurrentOperation != mDownPage)
										{
											CurrentOperation = mNone;
										}
									RelinquishCPU();
									SendIdleToAll();
								} while (WaitMouseUp());
							CurrentOperation = mNone;
							RedrawVBar();
						}
					break;
				case mVBox:
					if (VScrollable && Enabled)
						{
							if ((Event.Modifiers & optionKey) != 0)
								{
									short		OldMouseLoc = -32767;

									ImageXored = False;
									/* setting up drawing environment */
									SetUpPort();
									Length = PageDownZoneExt.y + PageUpZoneExt.y;
									do{
											static LongPoint	VXorBoxExt = {14,16};

											/* removing any image already there */
											do
												{
													MouseLoc = MyGlobalToLocal(GetLongMouseLoc());
													RelinquishCPU();
													SendIdleToAll();
												} while ((MouseLoc.y - 16 - 8 == OldMouseLoc) && WaitMouseUp());
											SetUpPort();
											Window->ResetPen();
											Window->SetPenMode(srcXor);
											Window->SetPattern(dkGray);
											if (ImageXored)
												{
													Window->LFrameRect(LongPointOf(Extent.x - 16 + 1,
														XorPosition + 16),VXorBoxExt);
													ImageXored = False;
												}
											/* finding new image position */
											OutOfRange = False;
											OldMouseLoc = MouseLoc.y - 16 - 8;
											XorPosition = OldMouseLoc;
											if ((MouseLoc.x < (Extent.x - 16 - 24))
												|| (MouseLoc.x > (Extent.x + 24)))
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
													Window->LFrameRect(LongPointOf(Extent.x - 16 + 1,
														XorPosition + 16),VXorBoxExt);
													ImageXored = True;
												}
											RelinquishCPU();
											SendIdleToAll();
											SetUpPort();
										} while (WaitMouseUp());
									if (ImageXored)
										{
											Owner->Hook(VScrollToLocation,
												((NumCells - 1) * XorPosition) / Length,NumCells);
										}
								}
							 else
								{
									do{
											Length = PageUpZoneExt.y + PageDownZoneExt.y;
											MouseLoc = MyGlobalToLocal(GetLongMouseLoc());
											MouseLoc.y = MouseLoc.y - 16 - 8;
											if (MouseLoc.y < 0)
												{
													MouseLoc.y = 0;
												}
											if (MouseLoc.y > Length)
												{
													MouseLoc.y = Length;
												}
											Owner->Hook(VScrollToLocation,((NumCells - 1) * MouseLoc.y
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


short			CVScrollBar::FindPart(LongPoint Where)
	{
		if (LongPtInRect(Where,OneUpZoneTL,OneUpZoneExt))
			{
				return mUpOne;
			}
		if (LongPtInRect(Where,PageUpZoneTL,PageUpZoneExt))
			{
				return mUpPage;
			}
		if (LongPtInRect(Where,VScrollZoneTL,VScrollZoneExt))
			{
				return mVBox;
			}
		if (LongPtInRect(Where,PageDownZoneTL,PageDownZoneExt))
			{
				return mDownPage;
			}
		if (LongPtInRect(Where,OneDownZoneTL,OneDownZoneExt))
			{
				return mDownOne;
			}
		return mNone;
	}


void			CVScrollBar::RedrawVBar(void)
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
				if (CurrentOperation == mUpOne)
					{
						Window->LDrawPicture(GetPic(UpArrowSelectedID),
							OneUpZoneTL,OneUpZoneExt);
					}
				 else
					{
						Window->LDrawPicture(GetPic(UpArrowID),
							OneUpZoneTL,OneUpZoneExt);
					}
				if (CurrentOperation == mDownOne)
					{
						Window->LDrawPicture(GetPic(DownArrowSelectedID),
							OneDownZoneTL,OneDownZoneExt);
					}
				 else
					{
						Window->LDrawPicture(GetPic(DownArrowID),
							OneDownZoneTL,OneDownZoneExt);
					}
				if (VScrollable)
					{
						Window->SetPattern(ltGray);
						Window->LPaintRect(LongPointOf(PageUpZoneTL.x + 1,PageUpZoneTL.y),
							LongPointOf(PageUpZoneExt.x - 2,PageUpZoneExt.y));
						Window->LPaintRect(LongPointOf(PageDownZoneTL.x + 1,PageDownZoneTL.y),
							LongPointOf(PageDownZoneExt.x - 2,PageDownZoneExt.y));
						Window->SetPattern(black);
						Window->DrawLine(PageUpZoneTL,LongPointOf(0,PageUpZoneExt.y
							+ VScrollZoneExt.y + PageDownZoneExt.y));
						Window->DrawLine(LongPointOf(PageUpZoneTL.x + 15,PageUpZoneTL.y),
							LongPointOf(0,PageUpZoneExt.y + VScrollZoneExt.y + PageDownZoneExt.y));
						Window->LEraseRect(LongPointOf(VScrollZoneTL.x + 1,VScrollZoneTL.y + 1),
							LongPointOf(VScrollZoneExt.x - 2,VScrollZoneExt.y - 2));
						Window->LFrameRect(VScrollZoneTL,VScrollZoneExt);
					}
				 else
					{
						Window->LEraseRect(LongPointOf(VScrollZoneTL.x + 1,VScrollZoneTL.y),
							LongPointOf(VScrollZoneExt.x - 2,VScrollZoneExt.y));
						Window->DrawLine(VScrollZoneTL,LongPointOf(0,VScrollZoneExt.y));
						Window->DrawLine(LongPointOf(VScrollZoneTL.x + 15,VScrollZoneTL.y),
							LongPointOf(0,VScrollZoneExt.y));
					}
			}
	}


void			CVScrollBar::DoUpdate(void)
	{
		RedrawVBar();
	}


void			CVScrollBar::DoResume(void)
	{
		inherited::DoResume();
		RedrawVBar();
	}


void			CVScrollBar::DoSuspend(void)
	{
		inherited::DoSuspend();
		RedrawVBar();
	}


void			CVScrollBar::DoEnable(void)
	{
		inherited::DoEnable();
		RedrawVBar();
	}


void			CVScrollBar::DoDisable(void)
	{
		inherited::DoDisable();
		RedrawVBar();
	}



void				CVScrollBar::RecalcScrollRects(void)
	{
		long		Length;
		long		BoxPos;

		Length = Extent.y;
		OneUpZoneTL.x = Extent.x - 16;
		OneUpZoneTL.y = 0;
		OneUpZoneExt.x = 16;
		OneUpZoneExt.y = 16;
		OneDownZoneTL.x = Extent.x - 16;
		OneDownZoneTL.y = Length - 16;
		OneDownZoneExt.x = 16;
		OneDownZoneExt.y = 16;
		if (NumCells <= 1)
			{
				if (CellIndex == 0)
					{
						VScrollable = False;
						VScrollZoneTL.x = Extent.x - 16;
						VScrollZoneTL.y = 16;
						VScrollZoneExt.x = 16;
						VScrollZoneExt.y = Length - 16 - 16;
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
				VScrollable = True;
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
				PageUpZoneTL.x = Extent.x - 16;
				PageUpZoneTL.y = 16;
				PageUpZoneExt.x = 16;
				PageUpZoneExt.y = BoxPos;
				VScrollZoneTL.x = Extent.x - 16 + 1;
				VScrollZoneTL.y = BoxPos + 16;
				VScrollZoneExt.x = 14;
				VScrollZoneExt.y = 16;
				PageDownZoneTL.x = Extent.x - 16;
				PageDownZoneTL.y = BoxPos + 16 + 16;
				PageDownZoneExt.x = 16;
				PageDownZoneExt.y = (Length - 3*16) - BoxPos;
			}
	}


void			CVScrollBar::SetPosition(long NewCellIndex, long NewNumCells)
	{
		CellIndex = NewCellIndex;
		NumCells = NewNumCells;
		RecalcScrollRects();
		if ((!Suspended) && Enabled)
			{
				RedrawVBar();
			}
	}


void			CVScrollBar::RecalcLocations(LongPoint EnclosureVisRectStart,
						LongPoint EnclosureVisRectExtent, LongPoint EnclosureOrigin)
	{
		inherited::RecalcLocations(EnclosureVisRectStart,EnclosureVisRectExtent,
			EnclosureOrigin);
		RecalcScrollRects();
	}
