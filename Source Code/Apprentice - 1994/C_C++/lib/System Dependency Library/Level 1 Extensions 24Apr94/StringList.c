/* StringList.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "StringList.h"
#include "Memory.h"
#include "DataMunging.h"
#include "Scroll.h"
#include "Array.h"


#define BorderInset (1)
#define TextInset (3)


typedef struct ElementRec
	{
		char*							Name;
		void*							Reference;
		MyBoolean					Selected;
	} ElementRec;

struct StringListRec
	{
		WinType*					Window;
		OrdType						X;
		OrdType						Y;
		OrdType						Width;
		OrdType						Height;
		FontType					Font;
		FontSizeType			FontSize;
		OrdType						LineHeight;
		ArrayRec*					ItemList;
		long							TopElement;
		ScrollRec*				Scrollbar;
		MyBoolean					AllowMultipleSelection;
		char*							Title;
		double						LastMouseDownTime;
		OrdType						LastMouseXLoc;
		OrdType						LastMouseYLoc;
	};


static void				StringListScrollHook(long Parameter, ScrollType How,
										StringListRec* ScrollTemp);

static void				RecalibrateScrollBar(StringListRec* List);


/* allocate a new, empty string list.  Title is null terminated */
StringListRec*		NewStringList(WinType* Window, OrdType X, OrdType Y,
										OrdType Width, OrdType Height, FontType Font, FontSizeType FontSize,
										MyBoolean AllowMultipleSelection, char* Title)
	{
		StringListRec*	List;

		List = (StringListRec*)AllocPtrCanFail(sizeof(StringListRec),"StringListRec");
		if (List == NIL)
			{
				return NIL;
			}
		List->Window = Window;
		List->X = X;
		List->Y = Y;
		List->Width = Width;
		List->Height = Height;
		List->Font = Font;
		List->FontSize = FontSize;
		List->LineHeight = GetFontHeight(Font,FontSize);
		List->ItemList = NewArray();
		if (List->ItemList == NIL)
			{
				ReleasePtr((char*)List);
				return NIL;
			}
		List->TopElement = 0;
		if (Title == NIL)
			{
				List->Scrollbar = NewScrollBar(Window,eVScrollBar,X + Width - 16,Y,Height);
			}
		 else
			{
				List->Scrollbar = NewScrollBar(Window,eVScrollBar,X + Width - 16,
					Y + List->LineHeight,Height - List->LineHeight);
			}
		if (List->Scrollbar == NIL)
			{
				DisposeArray(List->ItemList);
				ReleasePtr((char*)List);
				return NIL;
			}
		List->AllowMultipleSelection = AllowMultipleSelection;
		if (Title != NIL)
			{
				/* an error here will only make the display look weird */
				List->Title = StringToBlockCopy(Title);
			}
		 else
			{
				List->Title = NIL;
			}
		List->LastMouseDownTime = 0;
		RecalibrateScrollBar(List);
		return List;
	}


/* dispose the string list and all of the items in it */
void							DisposeStringList(StringListRec* List)
	{
		CheckPtrExistence(List);
		while (ArrayGetLength(List->ItemList) > 0)
			{
				ElementRec*		Element;

				Element = ArrayGetElement(List->ItemList,0);
				ArrayDeleteElement(List->ItemList,0);
				ReleasePtr(Element->Name);
				ReleasePtr((char*)Element);
			}
		if (List->Title != NIL)
			{
				ReleasePtr(List->Title);
			}
		DisposeScrollBar(List->Scrollbar);
		DisposeArray(List->ItemList);
		ReleasePtr((char*)List);
	}


/* find out where the string list is located */
OrdType						GetStringListXLoc(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->X;
	}


/* find out where the string list is located */
OrdType						GetStringListYLoc(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->Y;
	}


/* find out where the string list is located */
OrdType						GetStringListWidth(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->Width;
	}


/* find out where the string list is located */
OrdType						GetStringListHeight(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->Height;
	}


/* what font is being used to display the string list */
FontType					GetStringListFont(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->Font;
	}


/* what point size is being used to display the string list */
FontSizeType			GetStringListFontSize(StringListRec* List)
	{
		CheckPtrExistence(List);
		return List->FontSize;
	}


/* how many lines of text are visible in the string list */
long							GetStringListNumVisibleLines(StringListRec* List)
	{
		OrdType					Height;
		long						Temp;

		CheckPtrExistence(List);
		Height = List->Height;
		if (List->Title != NIL)
			{
				Height -= List->LineHeight;
			}
		Temp = Height / List->LineHeight;
		if ((Height % List->LineHeight) != 0)
			{
				/* part of a line is visible */
				Temp += 1;
			}
		return Temp;
	}


/* change the location of the string list */
void							SetStringListLoc(StringListRec* List, OrdType X, OrdType Y,
										OrdType Width, OrdType Height)
	{
		CheckPtrExistence(List);
		List->X = X;
		List->Y = Y;
		List->Width = Width;
		List->Height = Height;
		if (List->Title == NIL)
			{
				SetScrollLocation(List->Scrollbar,X + Width - 16,Y,Height);
			}
		 else
			{
				SetScrollLocation(List->Scrollbar,X + Width - 16,
					Y + List->LineHeight,Height - List->LineHeight);
			}
		RecalibrateScrollBar(List);
		SetClipRect(List->Window,List->X,List->Y,List->Width,List->Height);
		DrawBoxErase(List->Window,List->X,List->Y,List->Width,List->Height);
		RedrawStringList(List);
	}


/* change the font being used to display the string list */
void							SetStringListFontInfo(StringListRec* List, FontType Font,
										FontSizeType FontSize)
	{
		CheckPtrExistence(List);
		List->Font = Font;
		List->FontSize = FontSize;
		List->LineHeight = GetFontHeight(Font,FontSize);
		RedrawStringList(List);
	}


/* internal routine for redrawing string list */
static void				RedrawStringListOneLine(StringListRec* List,
										long LineIndex, MyBoolean PotentialSelection)
	{
		CheckPtrExistence(List);
		if ((LineIndex >= List->TopElement)
			&& (LineIndex <= List->TopElement + List->Height / List->LineHeight))
			{
				long				YOffset;
				OrdType			Top;
				OrdType			Height;

				Top = List->Y;
				Height = List->Height;
				if (List->Title != NIL)
					{
						Top += List->LineHeight;
						Height -= List->LineHeight;
					}
				YOffset = (LineIndex - List->TopElement) * List->LineHeight + BorderInset;
				SetClipRect(List->Window,List->X + BorderInset,Top + BorderInset,
					List->Width - (2 * BorderInset) - 15,Height - (2 * BorderInset));
				AddClipRect(List->Window,List->X,Top + YOffset,
					List->Width,List->LineHeight);
				if ((LineIndex < 0) || (LineIndex >= ArrayGetLength(List->ItemList)))
					{
						/* entry out of range, so we draw an empty line */
						DrawBoxErase(List->Window,List->X,Top + YOffset,
							List->Width,List->LineHeight);
					}
				 else
					{
						ElementRec*		Element;

						/* entry is within range, so we really draw something */
						Element = ArrayGetElement(List->ItemList,LineIndex);
						CheckPtrExistence(Element);
						if (Element->Selected)
							{
								/* selected */
								DrawBoxPaint(List->Window,eBlack,List->X + BorderInset,
									Top + YOffset,TextInset,List->LineHeight);
								InvertedTextLine(List->Window,List->Font,List->FontSize,
									Element->Name,PtrSize(Element->Name),
									List->X + BorderInset + TextInset,Top + YOffset,ePlain);
								DrawBoxPaint(List->Window,eBlack,List->X + BorderInset + TextInset
									+ LengthOfText(List->Font,List->FontSize,
									Element->Name,PtrSize(Element->Name),
									ePlain),Top + YOffset,List->Width,List->LineHeight);
							}
						 else
							{
								/* notselected */
								DrawBoxErase(List->Window,List->X + BorderInset,
									Top + YOffset,TextInset,List->LineHeight);
								DrawTextLine(List->Window,List->Font,List->FontSize,
									Element->Name,PtrSize(Element->Name),
									List->X + BorderInset + TextInset,Top + YOffset,ePlain);
								DrawBoxErase(List->Window,List->X + BorderInset + TextInset
									+ LengthOfText(List->Font,List->FontSize,
									Element->Name,PtrSize(Element->Name),
									ePlain),Top + YOffset,List->Width,List->LineHeight);
							}
						if (PotentialSelection)
							{
								DrawBoxFrame(List->Window,eMediumGrey,List->X + BorderInset,
									Top + YOffset,List->Width - 15 - (2 * BorderInset),
									List->LineHeight);
							}
					}
			}
	}


/* do a complete redraw of the string list */
void							RedrawStringList(StringListRec* List)
	{
		long						Scan;
		OrdType					Top;
		OrdType					Height;
		long						Limit;

		CheckPtrExistence(List);
		RecalibrateScrollBar(List);
		Limit = List->TopElement + GetStringListNumVisibleLines(List);
		for (Scan = List->TopElement; Scan < Limit; Scan += 1)
			{
				RedrawStringListOneLine(List,Scan,False);
			}
		SetClipRect(List->Window,List->X,List->Y,List->Width,List->Height);
		Top = List->Y;
		Height = List->Height;
		if (List->Title != NIL)
			{
				DrawTextLine(List->Window,List->Font,List->FontSize,List->Title,
					PtrSize(List->Title),List->X
					+ (List->Width - LengthOfText(List->Font,List->FontSize,
					List->Title,PtrSize(List->Title),ePlain)) / 2,Top,ePlain);
				Top += List->LineHeight;
				Height -= List->LineHeight;
			}
		DrawBoxFrame(List->Window,eBlack,List->X,Top,List->Width,Height);
		RedrawScrollBar(List->Scrollbar);
	}


/* return a count of the number of selected items in the string list */
long							GetStringListHowManySelectedItems(StringListRec* List)
	{
		long						Scan;
		long						Count;
		long						Limit;

		CheckPtrExistence(List);
		Limit = ArrayGetLength(List->ItemList);
		/* first, count how many */
		Count = 0;
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*			Temp;

				Temp = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Temp);
				if (Temp->Selected)
					{
						Count += 1;
					}
			}
		return Count;
	}


/* return an Array containing a list of the References of all selected items */
ArrayRec*					GetListOfSelectedItems(StringListRec* List)
	{
		ArrayRec*				Them;

		CheckPtrExistence(List);
		Them = NewArray();
		if (Them != NIL)
			{
				long						Scan;
				long						Limit;

				Limit = ArrayGetLength(List->ItemList);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						ElementRec*			Temp;

						Temp = ArrayGetElement(List->ItemList,Scan);
						CheckPtrExistence(Temp);
						if (Temp->Selected)
							{
								ArrayAppendElement(Them,Temp->Reference);
							}
					}
			}
		return Them;
	}


/* routine to verify that items don't get put on list more than once */
#if DEBUG
static void				StringListVerifyUniqueID(StringListRec* List, void* Reference)
	{
		long						Scan;
		long						Limit;

		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*			Temp;

				Temp = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Temp);
				if (Temp->Reference == Reference)
					{
						PRERR(ForceAbort,"StringListVerifyUniqueID failed");
					}
			}
	}
#else
	#define StringListVerifyUniqueID(stupid,dumb)
#endif


/* add a new element to the string list.  String is the string to be added to the */
/* list, and OurReference is the reference pointer that identifies the item.  */
/* BeforeThisReference is the item to insert it before.  if it is NIL then the */
/* item is appended to the list.  NIL can be passed for the name if the name isn't */
/* known yet.  Returns True if insertion was successful. */
MyBoolean					InsertStringListElement(StringListRec* List, char* String,
										void* BeforeThisReference, void* OurReference, MyBoolean Redraw)
	{
		ElementRec*			NewBox;
		long						InsertionPoint;
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		ERROR(OurReference == NIL,PRERR(ForceAbort,
			"InsertStringListElement:  references can not be NIL"));
		/* converting the reference of an object into an real numerical index into the list */
		/* if we can't find the reference, we default to insertion at end of list */
		Limit = ArrayGetLength(List->ItemList);
		InsertionPoint = Limit;
		if (BeforeThisReference != NIL)
			{
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						ElementRec*			Item;

						Item = ArrayGetElement(List->ItemList,Scan);
						CheckPtrExistence(Item);
						if (BeforeThisReference == Item->Reference)
							{
								InsertionPoint = Scan;
								goto NextThingPoint;
							}
					}
				EXECUTE(PRERR(AllowResume,
					"InsertStringListElement:  couldn't find element to insert before"));
			 NextThingPoint:
				;
			}
		/* else InsertionPoint is already correct */
		/* making the new object */
		StringListVerifyUniqueID(List,OurReference);
		NewBox = (ElementRec*)AllocPtrCanFail(sizeof(ElementRec),"ElementRec");
		if (NewBox == NIL)
			{
			 FailedPoint0:
				return False; /* failed! */
			}
		NewBox->Selected = False;
		NewBox->Reference = OurReference;
		if (String != NIL)
			{
				NewBox->Name = StringToBlockCopy(String);
			}
		 else
			{
				NewBox->Name = AllocPtrCanFail(0,"StringListName");
			}
		if (NewBox->Name == NIL)
			{
			 FailedPoint1:
				ReleasePtr((char*)NewBox);
				goto FailedPoint0;
			}
		if (!ArrayInsertElement(List->ItemList,NewBox,InsertionPoint))
			{
			 FailedPoint2:
				goto FailedPoint1;
			}
		if (Redraw)
			{
				RecalibrateScrollBar(List);
				RedrawStringList(List);
			}
		return True;
	}


/* change the name of a string list element associated with the specified reference */
void							ChangeStringListElementName(StringListRec* List, char* NewName,
										void* Reference)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*		Temp;

				Temp = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Temp);
				if (Reference == Temp->Reference)
					{
						char*					NameTemp;

						NameTemp = StringToBlockCopy(NewName);
						if (NameTemp != NIL)
							{
								ReleasePtr(Temp->Name);
								Temp->Name = NameTemp;
							}
						RedrawStringListOneLine(List,Scan,False);
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"ChangeStringListElementName:  Unknown element"));
	}


/* remove an element from the string list */
void							RemoveStringListElement(StringListRec* List, void* Reference,
										MyBoolean Redraw)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*			Temp;

				Temp = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Temp);
				if (Reference == Temp->Reference)
					{
						ArrayDeleteElement(List->ItemList,Scan);
						ReleasePtr(Temp->Name);
						ReleasePtr((char*)Temp);
						if (Redraw)
							{
								RecalibrateScrollBar(List);
								RedrawStringList(List);
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"RemoveStringListElement:  Unknown element"));
	}


/* see if the specified location is within the string list box */
MyBoolean					StringListHitTest(StringListRec* List, OrdType X, OrdType Y)
	{
		CheckPtrExistence(List);
		return ((X >= List->X) && (Y >= List->Y)
			&& (X < List->X + List->Width) && (Y < List->Y + List->Height));
	}


/* do a mouse down in the string list to select items.  returns True if it was */
/* a double click. */
MyBoolean					StringListMouseDown(StringListRec* List, OrdType X, OrdType Y,
										ModifierFlags Modifiers)
	{
		MyBoolean				Inside;
		long						Hilited;
		OrdType					Top;
		OrdType					Height;

		CheckPtrExistence(List);
		if (ScrollHitTest(List->Scrollbar,X,Y))
			{
				ScrollHitProc(List->Scrollbar,0,X,Y,List,
					(void (*)(long,ScrollType,void*))&StringListScrollHook);
				return False;
			}
		if ((TimerDifference(ReadTimer(),List->LastMouseDownTime)
			< GetDoubleClickInterval()) && (X - List->LastMouseXLoc > -3)
			&& (X - List->LastMouseXLoc < 3) && (Y - List->LastMouseYLoc > -3)
			&& (Y - List->LastMouseYLoc < 3) && ((Modifiers & eCommandKey) == 0))
			{
				List->LastMouseDownTime = ReadTimer();
				List->LastMouseXLoc = X;
				List->LastMouseYLoc = Y;
				return True;
			}
		List->LastMouseDownTime = ReadTimer();
		List->LastMouseXLoc = X;
		List->LastMouseYLoc = Y;

		Inside = False;
		Top = List->Y;
		Height = List->Height;
		if (List->Title != NIL)
			{
				Top += List->LineHeight;
				Height -= List->LineHeight;
			}
		do
			{
				if ((X < List->X) || (X > List->Width + List->X - 15))
					{
						if (Inside)
							{
								Inside = False;
								RedrawStringListOneLine(List,Hilited,False);
							}
					}
				 else
					{
						long				PossibleIndex;

						if (Y < Top)
							{
								Y = Top;
								if (Inside)
									{
										Inside = False;
										RedrawStringListOneLine(List,Hilited,False);
									}
								StringListScrollHook(0,eScrollLineMinus,List);
							}
						else if (Y > Top + Height - 1)
							{
								Y = Top + Height - 1;
								if (Inside)
									{
										Inside = False;
										RedrawStringListOneLine(List,Hilited,False);
									}
								StringListScrollHook(0,eScrollLinePlus,List);
							}
						else
							{
								PossibleIndex = (Y - Top - BorderInset) / List->LineHeight
									+ List->TopElement;
								if (Inside && (Hilited != PossibleIndex))
									{
										Inside = False;
										RedrawStringListOneLine(List,Hilited,False);
									}
								if (!Inside)
									{
										Inside = True;
										Hilited = PossibleIndex;
										RedrawStringListOneLine(List,Hilited,True);
									}
							}
					}
			} while (eMouseUp != GetAnEvent(&X,&Y,&Modifiers,NIL,NIL,NIL));
		if (Inside)
			{
				if ((Hilited >= 0) && (Hilited < ArrayGetLength(List->ItemList)))
					{
						ElementRec*		Item;

						Item = ArrayGetElement(List->ItemList,Hilited);
						CheckPtrExistence(Item);
						if ((Modifiers & eCommandKey) != 0)
							{
								if (Item->Selected)
									{
										DeselectStringListElement(List,Item->Reference);
									}
								 else
									{
										SelectStringListElement(List,Item->Reference);
									}
							}
						 else
							{
								SelectStringListElement(List,Item->Reference);
							}
					}
			}
		return False; /* no double click */
	}


/* select (hilite) the specified element in the list */
void							SelectStringListElement(StringListRec* List, void* Reference)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		/* first, deselect anything if multiple selection isn't allowed */
		if (!List->AllowMultipleSelection)
			{
				DeselectAllStringListElements(List);
			}
		/* now, select the element we are interested in */
		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*		Item;

				Item = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Item);
				if (Reference == Item->Reference)
					{
						if (!Item->Selected)
							{
								Item->Selected = True;
								RedrawStringListOneLine(List,Scan,False);
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"SelectStringListElement:  Unknown element"));
	}


/* deselect the specified element in the list */
void							DeselectStringListElement(StringListRec* List, void* Reference)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		/* first, deselect anything if multiple selection isn't allowed */
		if (!List->AllowMultipleSelection)
			{
				DeselectAllStringListElements(List);
			}
		/* now, select the element we are interested in */
		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*		Item;

				Item = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Item);
				if (Reference == Item->Reference)
					{
						if (Item->Selected)
							{
								Item->Selected = False;
								RedrawStringListOneLine(List,Scan,False);
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"SelectStringListElement:  Unknown element"));
	}


/* enable the scrollbar display in the list */
void							EnableStringList(StringListRec* List)
	{
		CheckPtrExistence(List);
		EnableScrollBar(List->Scrollbar);
	}


/* disable the scrollbar display in the list */
void							DisableStringList(StringListRec* List)
	{
		CheckPtrExistence(List);
		DisableScrollBar(List->Scrollbar);
	}


/* internal scrollhook for scrolling in the string list */
static void				StringListScrollHook(long Parameter, ScrollType How,
										StringListRec* ScrollTemp)
	{
		CheckPtrExistence(ScrollTemp);
		switch (How)
			{
				case eScrollToPosition:
					ScrollTemp->TopElement = Parameter;
					break;
				case eScrollPageMinus:
					ScrollTemp->TopElement -= GetStringListNumVisibleLines(ScrollTemp) - 2;
					break;
				case eScrollPagePlus:
					ScrollTemp->TopElement += GetStringListNumVisibleLines(ScrollTemp) - 2;
					break;
				case eScrollLineMinus:
					ScrollTemp->TopElement -= 1;
					break;
				case eScrollLinePlus:
					ScrollTemp->TopElement += 1;
					break;
				default:
					EXECUTE(PRERR(AllowResume,"StringListScrollHook:  Unknown scroll opcode"));
					break;
			}
		if (ScrollTemp->TopElement > ArrayGetLength(ScrollTemp->ItemList)
			- GetStringListNumVisibleLines(ScrollTemp) + 1)
			{
				ScrollTemp->TopElement = ArrayGetLength(ScrollTemp->ItemList)
					- GetStringListNumVisibleLines(ScrollTemp) + 1;
			}
		if (ScrollTemp->TopElement < 0)
			{
				ScrollTemp->TopElement = 0;
			}
		RecalibrateScrollBar(ScrollTemp);
		RedrawStringList(ScrollTemp);
	}


/* deselect all of the items in the string list that are selected */
void							DeselectAllStringListElements(StringListRec* List)
	{
		long						Scan;
		long						Limit;

		CheckPtrExistence(List);
		Limit = ArrayGetLength(List->ItemList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*		Item;

				Item = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Item);
				if (Item->Selected)
					{
						Item->Selected = False;
						RedrawStringListOneLine(List,Scan,False);
					}
			}
	}


static void				RecalibrateScrollBar(StringListRec* List)
	{
		long						Maximum;

		CheckPtrExistence(List);
		Maximum = ArrayGetLength(List->ItemList) - GetStringListNumVisibleLines(List) + 2;
		if (Maximum < 1)
			{
				Maximum = 1;
			}
		SetMaxScrollIndex(List->Scrollbar,Maximum);
		SetScrollIndex(List->Scrollbar,List->TopElement);
	}


/* make sure selection is visible in the window */
void							MakeStringListSelectionVisible(StringListRec* List)
	{
		long						WhatIndexToUse;
		long						Scan;
		long						Count;
		long						Limit;

		CheckPtrExistence(List);
		Limit = ArrayGetLength(List->ItemList);
		/* first, count how many */
		Count = 0;
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				ElementRec*			Temp;

				Temp = ArrayGetElement(List->ItemList,Scan);
				CheckPtrExistence(Temp);
				if (Temp->Selected)
					{
						WhatIndexToUse = Scan;
						if (WhatIndexToUse < List->TopElement)
							{
								List->TopElement = WhatIndexToUse;
							}
						if (WhatIndexToUse > List->TopElement
							+ (GetStringListNumVisibleLines(List) - 2))
							{
								List->TopElement = WhatIndexToUse
									- (GetStringListNumVisibleLines(List) - 2);
							}
						RedrawStringList(List);
						return;
					}
			}
	}
