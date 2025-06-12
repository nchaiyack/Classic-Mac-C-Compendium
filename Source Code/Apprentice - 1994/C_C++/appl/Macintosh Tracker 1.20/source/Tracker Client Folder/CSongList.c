/* CSongList.c */

#include "CSongList.h"
#include "CVScrollBar.h"
#include "CMyDocument.h"
#include "CWindow.h"
#include "CArray.h"
#include "CApplication.h"
#include "Memory.h"
#include "MenuController.h"
#include "LocationConstants.h"
#include "CMyApplication.h"


#define InitialOffset (1)
#define MarkerInset (5)
#define SELECTIONTHICKNESS (2)
#define MaxTypedCharDelay (30)


void				CSongList::ISongList(CMyDocument* TheDocument, CWindow* TheWindow)
	{
		LongPoint			Start,Extent;
		CVScrollBar*	OurScroller;
		FontInfo			MyFontInfo;

		Document = TheDocument;
		GetRect(ScrollingListLocID,&Start,&Extent);
		IViewRect(Start,Extent,TheWindow,TheWindow);

		GetRect(ScrollingListBarLocID,&Start,&Extent);
		OurScroller = new CVScrollBar;
		VScroll = OurScroller;
		OurScroller->IVScrollBar(Start,Extent,this,TheWindow,TheWindow);

		LastClickTime = TickCount();
		LastCharTime = TickCount();
		StartingIndex = 0;
		CharBufferLength = 0;

		SetUpPort();
		Window->SetText(systemFont,0,srcCopy,12,0);
		GetFontInfo(&MyFontInfo);
		LineHeight = MyFontInfo.leading + MyFontInfo.ascent + MyFontInfo.descent;
	}


void				CSongList::DoMouseDown(MyEventRec Event)
	{
		long				SongIndex;
		long				CurrentIndex;

		BecomeKeyReceiver();
		Event.Where = MyGlobalToLocal(Event.Where);
		SongIndex = StartingIndex + (Event.Where.y / LineHeight);

		if (((Event.Modifiers & optionKey) != 0) && (Document->Selection != -1))
			{
				MyBoolean			InWindow;

			 MoveSelection:
				CurrentIndex = -1;
				InWindow = False;
				do
					{
						long					Index2;

						Event.Where = MyGlobalToLocal(GetLongMouseLoc());
						if ((Event.Where.y < 0) && (StartingIndex > 0))
							{
								Hook(VScrollUpOne,0,0);
							}
						if ((Event.Where.y > Extent.y) && (StartingIndex <
							Document->ListOfSongs->GetNumElements() - ((Extent.y - 2) / LineHeight)))
							{
								Hook(VScrollDownOne,0,0);
							}
						Index2 = StartingIndex + (Event.Where.y / LineHeight);
						if ((Event.Where.x >= 0) && (Event.Where.x < Extent.x))
							{
								if ((CurrentIndex != Index2) || !InWindow)
									{
										InWindow = True;
										Redraw(CurrentIndex - 1,CurrentIndex);
										CurrentIndex = Index2;
										SetUpPort();
										Window->ResetPen();
										Window->LEraseRect(LongPointOf(1,(CurrentIndex - StartingIndex)
											* LineHeight - SELECTIONTHICKNESS - 1),LongPointOf(Extent.x - 2,1));
										Window->LPaintRect(LongPointOf(1,(CurrentIndex - StartingIndex)
											* LineHeight - SELECTIONTHICKNESS),
											LongPointOf(Extent.x - 2,2 * SELECTIONTHICKNESS));
										Window->LEraseRect(LongPointOf(1,(CurrentIndex - StartingIndex)
											* LineHeight + SELECTIONTHICKNESS),LongPointOf(Extent.x - 2,1));
									}
							}
						 else
							{
								if ((CurrentIndex != Index2) || InWindow)
									{
										InWindow = False;
										Redraw(CurrentIndex - 1,CurrentIndex);
										CurrentIndex = Index2;
									}
							}
						RelinquishCPUJudiciously();
					} while (StillDown());
				Redraw(CurrentIndex - 1,CurrentIndex);
				if (InWindow)
					{
						/* only move item if mouse released inside the pane */
						Document->MoveSong(Document->Selection,CurrentIndex);
					}
				return;
			}

		if ((Event.When - LastClickTime < DoubleTime)
			&& (SongIndex == Document->Selection))
			{
				/* double click--stop current song and play new song */
				Document->StartThisSong(SongIndex);
			}
		 else
			{
				/* single click--just change the selection */
				if ((SongIndex >= 0) && (SongIndex < Document->ListOfSongs->GetNumElements()))
					{
						Document->SetNewSelection(SongIndex);
					}
				while (StillDown())
					{
						Event.Where = MyGlobalToLocal(GetLongMouseLoc());
						CurrentIndex = StartingIndex + (Event.Where.y / LineHeight);
						if (CurrentIndex != SongIndex)
							{
								goto MoveSelection;
							}
						RelinquishCPUJudiciously();
					}
			}
		LastClickTime = Event.When;
	}


MyBoolean		CSongList::DoKeyDown(MyEventRec Event)
	{
		long				Temp;

		switch (Event.Message & charCodeMask)
			{
				case (uchar)0x1e:  /* macintosh up arrow */
					if (Document->Selection > 0)
						{
							long			StartTemp;

							Document->SetNewSelection(Document->Selection - 1);
						 GetSelectionOnScreen:
							StartTemp = StartingIndex;
							while (Document->Selection < StartTemp)
								{
									StartTemp -= 1;
								}
							while (Document->Selection > StartTemp
								+ ((Extent.y - 2) / LineHeight) - 1)
								{
									StartTemp += 1;
								}
							Hook(VScrollToLocation,StartTemp,0);
						}
					return True;
				case (uchar)0x1f:  /* macintosh down arrow */
					if (Document->Selection < Document->ListOfSongs->GetNumElements() - 1)
						{
							Document->SetNewSelection(Document->Selection + 1);
							goto GetSelectionOnScreen;
						}
					return True;
				case (uchar)0x0d:  /* return key */
				case (uchar)0x03:  /* enter key */
					Document->StartThisSong(Document->Selection);
					return True;
				default:
					BecomeKeyReceiver();
					if (TickCount() - LastCharTime > MaxTypedCharDelay)
						{
							CharBufferLength = 0;
						}
					if (CharBufferLength < MAXTYPEDCHARS)
						{
							CharBuffer[CharBufferLength] = Event.Message & charCodeMask;
							CharBufferLength += 1;
						}
					Temp = 0;
					while (Temp < Document->ListOfSongs->GetNumElements())
						{
							SongRec*			TempSongLoc;

							TempSongLoc = Document->ListOfSongs->GetElementAddress(Temp);
							/* we won't move memory... I promise! */
							if (TempSongLoc->SongName[0] >= CharBufferLength)
								{
									MyBoolean				OK;
									short						Scan;
									char						First;
									char						Second;

									OK = True;
									Scan = 0;
									while ((Scan < CharBufferLength) && OK)
										{
											First = CharBuffer[Scan];
											Second = TempSongLoc->SongName[Scan + 1];
											if ((First >= 'A') && (First <= 'Z'))
												{
													First = First - 'A' + 'a';
												}
											if ((Second >= 'A') && (Second <= 'Z'))
												{
													Second = Second - 'A' + 'a';
												}
											OK = (First == Second);
											Scan += 1;
										}
									if (OK)
										{
											Document->SetNewSelection(Temp);
											LastCharTime = TickCount();
											goto GetSelectionOnScreen;
										}
								}
							Temp += 1;
						}
					Document->SetNewSelection(-1);
					LastCharTime = TickCount();
					return True;
			}
	}


void				CSongList::DoUpdate(void)
	{
		SetUpPort();
		Window->ResetPen();
		Window->LFrameRect(ZeroPoint,Extent);
		Redraw(StartingIndex,StartingIndex + ((Extent.y - 2) / LineHeight + 1));
	}


MyBoolean		CSongList::DoMenuCommand(ushort MenuCommandValue)
	{
		long				Temp;
		long				StartTemp;

		switch (MenuCommandValue)
			{
				case mFileClose:
					Document->GoAway();
					return True;
				case mFileSaveAs:
					Document->SaveFileAs();
					return True;
				case mFileSave:
					Document->SaveFile();
					return True;
				case mDeleteSelection:
					Document->RemoveSongFromList(Document->Selection);
					Temp = Document->Selection;
					Document->SetNewSelection(-1);
					return True;
				case mPlaySelection:
					Document->StartThisSong(Document->Selection);
					return True;
				case mStopPlaying:
					Document->CancelCurrentSong();
					return True;
				case mIncreaseVolume:
					Document->DoVolumeUp();
					return True;
				case mDecreaseVolume:
					Document->DoVolumeDown();
					return True;
				case mShowSelection:
					StartTemp = StartingIndex;
					while (Document->Selection < StartTemp)
						{
							StartTemp -= 1;
						}
					while (Document->Selection > StartTemp + ((Extent.y - 2) / LineHeight) - 1)
						{
							StartTemp += 1;
						}
					Hook(VScrollToLocation,StartTemp,0);
					return True;
				case mShowPlaying:
					StartTemp = StartingIndex;
					while (Document->Playing < StartTemp)
						{
							StartTemp -= 1;
						}
					while (Document->Playing > StartTemp + ((Extent.y - 2) / LineHeight) - 1)
						{
							StartTemp += 1;
						}
					Hook(VScrollToLocation,StartTemp,0);
					return True;
				case mUseEspieTracker:
					Document->SetTrackerServerToUse(eTrackerMarkEspie);
					break;
				case mUseSeideTracker:
					Document->SetTrackerServerToUse(eTrackerFrankSeide);
					break;
				case mUseRossetTracker:
					Document->SetTrackerServerToUse(eTrackerAntoineRosset);
					break;
				default:
					return Application->DoMenuCommand(MenuCommandValue);
			}
	}


void				CSongList::EnableMenuItems(void)
	{
		Application->EnableMenuItems();
		MyEnableItem(mFileClose);
		MyEnableItem(mFileSaveAs);
		if (!Document->UpToDate)
			{
				MyEnableItem(mFileSave);
			}
		if (Document->Playing != -1)
			{
				MyEnableItem(mStopPlaying);
				MyEnableItem(mIncreaseVolume);
				MyEnableItem(mDecreaseVolume);
			}
		if (Document->Selection != -1)
			{
				MyEnableItem(mDeleteSelection);
				MyEnableItem(mPlaySelection);
			}
		if (Document->Selection != -1)
			{
				MyEnableItem(mShowSelection);
			}
		if (Document->Playing != -1)
			{
				MyEnableItem(mShowPlaying);
			}
		if (Document->Selection != -1)
			{
				MyEnableItem(mUseEspieTracker);
				MySetItemMark(mUseEspieTracker,noMark);
				MyEnableItem(mUseSeideTracker);
				MySetItemMark(mUseSeideTracker,noMark);
				MyEnableItem(mUseRossetTracker);
				MySetItemMark(mUseRossetTracker,noMark);
				switch (Document->FindOutTrackerServerToUse())
					{
						case eTrackerMarkEspie:
							MySetItemMark(mUseEspieTracker,checkMark);
							break;
						case eTrackerFrankSeide:
							MySetItemMark(mUseSeideTracker,checkMark);
							break;
						case eTrackerAntoineRosset:
							MySetItemMark(mUseRossetTracker,checkMark);
							break;
					}
			}
	}


void				CSongList::Redraw(long Start, long End)
	{
		long				Scan;
		LongPoint		TopLeft;
		LongPoint		BottomRight;
		Handle			String;
		SongRec			Temp;

		SetUpPort();
		Window->SetClipRect(LongPointOf(1,1),LongPointOf(Extent.x - 2,Extent.y - 2));
		Window->SetText(systemFont,0,srcCopy,12,0);
		for (Scan = Start; Scan <= End; Scan += 1)
			{
				TopLeft.x = 1;
				TopLeft.y = (Scan - StartingIndex) * LineHeight + 1;
				BottomRight.x = Extent.x - 2;
				BottomRight.y = LineHeight;
				if (Scan != Document->Selection)
					{
						Window->LEraseRect(TopLeft,BottomRight);
						Window->SetTextMode(srcCopy);
						Window->SetPenMode(patCopy);
					}
				 else
					{
						Window->LPaintRect(TopLeft,BottomRight);
						Window->SetTextMode(srcBic);
						Window->SetPenMode(patBic);
					}
				TopLeft.x += 5;
				BottomRight.x -= 5;
				if (Scan == Document->Playing)
					{
						Window->LPaintOval(LongPointOf(TopLeft.x,TopLeft.y + MarkerInset
							+ InitialOffset),LongPointOf(LineHeight - MarkerInset * 2,
							LineHeight - MarkerInset * 2));
					}
				TopLeft.x += LineHeight - MarkerInset * 2 + 8;
				BottomRight.x -= LineHeight - MarkerInset * 2 + 8;
				if (Document->ListOfSongs->GetElement(Scan,&Temp))
					{
						String = PString2Handle(Temp.SongName);
						Window->LDrawText(TopLeft,BottomRight,String,JustifyLeft);
						ReleaseHandle(String);
					}
			}
	}


void				CSongList::RecalculateScrollBars(void)
	{
		VScroll->SetPosition(StartingIndex,Document->ListOfSongs->GetNumElements()
			- ((Extent.y - 2) / LineHeight) + 1);
	}


long				CSongList::Hook(short OperationID, long Operand1, long Operand2)
	{
		SetUpPort();
		Window->SetClipRect(LongPointOf(1,1),LongPointOf(Extent.x - 2,Extent.y - 2));
		switch (OperationID)
			{
				case VScrollToLocation:
					{
						long				Delta;

						if (Operand1 < 0)
							{
								Operand1 = 0;
							}
						if ((Operand1 > Document->ListOfSongs->GetNumElements()
							- ((Extent.y - 2) / LineHeight)) && (Document->ListOfSongs->GetNumElements()
							- ((Extent.y - 2) / LineHeight) > 0))
							{
								Operand1 = Document->ListOfSongs->GetNumElements()
									- ((Extent.y - 2) / LineHeight);
							}
						Delta = Operand1 - StartingIndex; /* how many cells to move down by */
						Window->ScrollLong(LongPointOf(1,1),LongPointOf(Extent.x - 2,Extent.y - 2),
							LongPointOf(0,(-Delta) * LineHeight));
						StartingIndex += Delta;
						if (Delta < 0)
							{
								/* shifted image down; opened at top */
								Redraw(StartingIndex,StartingIndex - Delta);
							}
						 else
							{
								/* shifted up, opened at bottom */
								Redraw(StartingIndex + ((Extent.y - 2) / LineHeight) - Delta,
									StartingIndex + ((Extent.y - 2) / LineHeight) + 1);
							}
						VScroll->SetPosition(StartingIndex,Document->ListOfSongs->GetNumElements()
							- ((Extent.y - 2) / LineHeight) + 1);
					}
					return 0;
				case VScrollUpOne:
					if (StartingIndex - 1 >= 0)
						{
							return Hook(VScrollToLocation,StartingIndex - 1,0);
						}
					return 0;
				case VScrollUpPage:
					if (StartingIndex - ((Extent.y - 2) / LineHeight - 1) < 0)
						{
							return Hook(VScrollToLocation,0,0);
						}
					 else
						{
							return Hook(VScrollToLocation,StartingIndex
								- (Extent.y / LineHeight - 1),0);
						}
				case VScrollDownOne:
					if (StartingIndex + 1 <= Document->ListOfSongs->GetNumElements()
						- ((Extent.y - 2) / LineHeight))
						{
							return Hook(VScrollToLocation,StartingIndex + 1,0);
						}
					return 0;
				case VScrollDownPage:
					if (StartingIndex + ((Extent.y - 2) / LineHeight - 1)
						> Document->ListOfSongs->GetNumElements() - ((Extent.y - 2) / LineHeight))
						{
							return Hook(VScrollToLocation,Document->ListOfSongs->GetNumElements()
								- ((Extent.y - 2) / LineHeight),0);
						}
					 else
						{
							return Hook(VScrollToLocation,StartingIndex
								+ ((Extent.y - 2) / LineHeight - 1),0);
						}
				default:
					return inherited::Hook(OperationID,Operand1,Operand2);
			}
	}
