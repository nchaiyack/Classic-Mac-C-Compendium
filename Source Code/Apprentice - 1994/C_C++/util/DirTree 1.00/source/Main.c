/* Main.c */
/*****************************************************************************/
/*                                                                           */
/*    File System Tree Viewer                                                */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This software is Public Domain; it may be used for any purpose         */
/*    whatsoever without restriction.                                        */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "Main.h"
#include "DirStuff.h"
#include "EventLoop.h"
#include "Memory.h"
#include "Menus.h"
#include "Alert.h"
#include "GrowIcon.h"
#include "StartupOpen.h"
#include "DataMunging.h"
#include "Numbers.h"
#include "StringDialog.h"


#define BoundingBoxThickness (2)
#define LeadingLineThickness (4)
#define TrailingLineThickness (4)
#define FontPointSize (9)
#define VerticalSeparationSpace (2)
#define NumStatusLines (1)
#define ScrollIncrement (20)
#define AliasPattern (eMediumGrey)
#define XLEAD (24)


struct Node
	{
		Node*						Next;
		Node*						Down;
		char*						Name;
		DirFileTypes		Type;
		OrdType					InstanceWidth;
		OrdType					WhereX;
		OrdType					WhereY;
		OrdType					Width;
		OrdType					Height;
		MyBoolean				SelectedFlag;
		FileSpec*				Location;
	};

struct AliasVec
	{
		AliasVec*				Next;
		OrdType					StartX;
		OrdType					StartY;
		OrdType					DisplaceX;
		OrdType					DisplaceY;
	};


static MyBoolean					ErrorOccurredFlag = False;
static Node*							GlobalList;
static OrdType						TotalWidth;
static OrdType						TotalHeight;
static AliasVec*					ListOfAliases;

static Node*							CurrentlyHilited = NIL;
static Node*							OtherOne = NIL;
static long double				LastClickTime = 0;
static OrdType						LastClickX = -32767;
static OrdType						LastClickY = -32767;
static MyBoolean					AliasesOnTop = False;
static MyBoolean					ShowAliasLines = True;
static char*							SearchKey;

static OrdType						CurrentXIndex;
static OrdType						CurrentYIndex;

static ScrollRec*					VerticalScroll;
static ScrollRec*					HorizontalScroll;
static WinType*						Window;
static OrdType						LeadingSpace;
static OrdType						TextLineHeight;

static long								DirectoryCount;
static long								FileCount;
static long								SymbolicLinkCount;

static MenuType*					mmAppleMenu;
static MenuType*					mmFileMenu;
static MenuType*					mmEditMenu;
static MenuItemType*			mAboutProgram;
static MenuItemType*			mShowAliasesOnTop;
static MenuItemType*			mShowAliasLines;
static MenuItemType*			mClose;
static MenuItemType*			mQuit;
static MenuItemType*			mUndo;
static MenuItemType*			mCut;
static MenuItemType*			mCopy;
static MenuItemType*			mPaste;
static MenuItemType*			mClear;
static MenuItemType*			mSelectAll;
static MenuItemType*			mJumpSymbolicLink;
static MenuItemType*			mReturnToPrevious;
static MenuItemType*			mWhoLinksToThisOne;
static MenuItemType*			mShowCurrentItem;
static MenuItemType*			mGetFileInfo;
static MenuItemType*			mFind;
static MenuItemType*			mFindAgain;


/* this recursively compiles a node list from a directory.  It does NOT */
/* initialize the display positioning values. */
Node*							ListFromDirectory(FileSpec* Where)
	{
		DirectoryRec*		Directory;
		Node*						Start;
		Node*						Tail;
		long						Limit;
		long						Scan;

		RelinquishCPUJudiciouslyCheckCancel();
		Directory = ReadDirectory(Where);
		if (Directory == NIL)
			{
			 MemOut0:
				ErrorOccurredFlag = True;
				return NIL;
			}
		ResortDirectory(Directory);
		Limit = GetDirectorySize(Directory);
		Start = NIL;
		Tail = NIL;
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				Node*						New;
				FileSpec*				Location;

				RelinquishCPUJudiciouslyCheckCancel();
				New = (Node*)AllocPtrCanFail(sizeof(Node),"Node");
				New->SelectedFlag = False;
				if (New == NIL)
					{
					 MemOut1:
						DisposeList(Start);
						DisposeDirectory(Directory);
						goto MemOut0;
					}
				New->Type = GetDirectoryEntryType(Directory,Scan);
				Location = GetDirectoryEntryFileSpec(Directory,Scan);
				if (Location == NIL)
					{
					 MemOut2:
						ReleasePtr((char*)New);
						goto MemOut1;
					}
				New->Location = Location;
				if (New->Type == eDirectory)
					{
						DirectoryCount += 1;
						New->Down = ListFromDirectory(Location);
					}
				else if (New->Type == eSymbolicLink)
					{
						SymbolicLinkCount += 1;
						New->Down = NIL;
					}
				else
					{
						FileCount += 1;
						New->Down = NIL;
					}
				New->Name = GetDirectoryEntryName(Directory,Scan);
				if (New->Name == NIL)
					{
						DisposeFileSpec(New->Location);
						goto MemOut2;
					}
				New->Next = NIL;
				if (Tail != NIL)
					{
						Tail->Next = New;
					}
				 else
					{
						Start = New;
					}
				Tail = New;
			}
		DisposeDirectory(Directory);
		return Start;
	}


Node*							ListFromDirectoryStart(FileSpec* Where)
	{
		Node*						Root;
		Node*						Down;

		DirectoryCount = 0;
		FileCount = 0;
		SymbolicLinkCount = 0;
		Root = (Node*)AllocPtrCanFail(sizeof(Node),"RootNode");
		if (Root == NIL)
			{
				PRERR(ForceAbort,"Not enough memory to continue.");
			}
		Root->SelectedFlag = False;
		Root->Next = NIL;
		if (Where != NIL)
			{
				Root->Name = ExtractFileName(Where);
				Root->Location = DuplicateFileSpec(Where);
			}
		 else
			{
				Root->Location = GetRootFileSpec(); /* root doesn't really exist */
				Root->Name = StringToBlockCopy("/");
			}
		if (Root->Name == NIL)
			{
				PRERR(ForceAbort,"Out of memory");
			}
		Down = ListFromDirectory(Where);
		Root->Down = Down;
		Root->Type = eDirectory;
		return Root;
	}


void							DisposeList(Node* List)
	{
		while (List != NIL)
			{
				Node*					Temp;

				DisposeList(List->Down);
				Temp = List;
				List = List->Next;
				DisposeFileSpec(Temp->Location);
				ReleasePtr(Temp->Name);
				ReleasePtr((char*)Temp);
			}
	}


/* this is used by ResolveAliases to find the actual location in the tree of */
/* the thing that is referenced by an alias (Target is the file system's idea */
/* of where, but that doesn't help us) */
Node*							FindMatchingItem(Node* List, FileSpec* Target,
										OrdType* OutX, OrdType* OutY)
	{
		while (List != NIL)
			{
				Node*				Temp;

				if (CompareFileSpecs(Target,List->Location))
					{
						*OutX = List->WhereX + LeadingLineThickness;
						*OutY = List->WhereY + (List->Height / 2);
						return List;
					}
				Temp = FindMatchingItem(List->Down,Target,OutX,OutY);
				if (Temp != NIL)
					{
						return Temp;
					}
				List = List->Next;
			}
		return NIL;
	}


/* this scans the list and creates target vectors for all symbolic links */
void							ResolveAliases(Node* List)
	{
		while (List != NIL)
			{
				if (List->Type == eSymbolicLink)
					{
						FileSpec*					Target;

						Target = DereferenceSymbolicLink(List->Location);
						if (Target != NIL)
							{
								OrdType					TargetX;
								OrdType					TargetY;

								if (FindMatchingItem(GlobalList,Target,&TargetX,&TargetY) != NIL)
									{
										AliasVec*				New;

										New = (AliasVec*)AllocPtrCanFail(sizeof(AliasVec),"AliasVec");
										if (New != NIL)
											{
												TargetX -= 1; /* just a bit before the box */
												New->StartX = List->WhereX + List->InstanceWidth;
												New->StartY = List->WhereY + (List->Height / 2);
												New->DisplaceX = TargetX - New->StartX;
												New->DisplaceY = TargetY - New->StartY;
												New->Next = ListOfAliases;
												ListOfAliases = New;
											}
									}
								DisposeFileSpec(Target);
							}
						 else
							{
								EXECUTE(PRERR(AllowResume,"ResolveAliases:  symbolic link wasn't alias"));
							}
					}
				ResolveAliases(List->Down);
				List = List->Next;
			}
	}


void							DisposeAliasList(void)
	{
		while (ListOfAliases != NIL)
			{
				AliasVec*				Temp;

				Temp = ListOfAliases;
				ListOfAliases = ListOfAliases->Next;
				ReleasePtr((char*)Temp);
			}
	}


/* this directory takes a list of directories which are assumed to be contained */
/* within a single directory.  It then calculates the maximum width and height */
/* needed to display the list and recursively sets up items contained in the dir. */
void							FillInSizeFields(Node* List, OrdType FrameX, OrdType FrameY,
										OrdType* WidthOut, OrdType* HeightOut)
	{
		OrdType					SlidingY;
		OrdType					Width;
		OrdType					Height;
		Node*						Scan;
		OrdType					OurMaxWidth;

		/* calculate size information for all contained items */
		Scan = List;
		SlidingY = FrameY + 1;
		OurMaxWidth = 0;
		while (Scan != NIL)
			{
				OrdType				TempWidth;

				/* figure out how wide each item is */
				ERROR((PtrSize(Scan->Name) > 32) || (PtrSize(Scan->Name) < 1),
					PRERR(AllowResume,"FillInSizeFields:  Filename is an invalid length"));
				TempWidth = LengthOfText(GetScreenFont(),FontPointSize,
					Scan->Name,PtrSize(Scan->Name),ePlain) + LeadingLineThickness
					+ (2 * BoundingBoxThickness) + TrailingLineThickness;
				Scan->InstanceWidth = TempWidth;
				/* calculate item's variables */
				if (Scan->Down != NIL)
					{
						/* Scan has members, it's at least as high as it's first member... */
						FillInSizeFields(Scan->Down,FrameX + Scan->InstanceWidth,SlidingY,
							&Width,&Height);
					}
				 else
					{
						/* Scan has no members, so it's one element high & itself wide */
						Width = 0;
						Height = TextLineHeight + VerticalSeparationSpace;
					}
				Scan->WhereX = FrameX;
				Scan->WhereY = SlidingY;
				Scan->Width = Scan->InstanceWidth + Width;
				Scan->Height = Height;
				if (OurMaxWidth < Scan->InstanceWidth + Width)
					{
						OurMaxWidth = Scan->InstanceWidth + Width;
					}
				SlidingY += Height;
				Scan = Scan->Next;
			}
		SlidingY += 1;
		*WidthOut = OurMaxWidth;
		*HeightOut = SlidingY - FrameY;
	}


/* redraw the tree */
void							RedrawArea(Node* List)
	{
		Node*						Scan;

		Scan = List;
		while (Scan != NIL)
			{
				if (IsRectVisible(Window,Scan->WhereX - CurrentXIndex,Scan->WhereY
					+ LeadingSpace - CurrentYIndex,Scan->Width,Scan->Height))
					{
						Patterns			How;

						/* for each whole block that's visible, draw the root of the */
						/* block & all of it's members */
						if (Scan->SelectedFlag)
							{
								DrawLine(Window,eBlack,Scan->WhereX + LeadingLineThickness
									- CurrentXIndex + 1,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,
									0,TextLineHeight);
								DrawLine(Window,eBlack,Scan->WhereX + LeadingLineThickness
									- CurrentXIndex + (Scan->InstanceWidth - LeadingLineThickness
									- TrailingLineThickness) - 2,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,
									0,TextLineHeight);
								InvertedTextLine(Window,GetScreenFont(),FontPointSize,Scan->Name,
									PtrSize(Scan->Name),Scan->WhereX + BoundingBoxThickness
									+ LeadingLineThickness - CurrentXIndex,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,ePlain);
							}
						 else
							{
								DrawLine(Window,eWhite,Scan->WhereX + LeadingLineThickness
									- CurrentXIndex + 1,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,
									0,TextLineHeight);
								DrawLine(Window,eWhite,Scan->WhereX + LeadingLineThickness
									- CurrentXIndex + (Scan->InstanceWidth - LeadingLineThickness
									- TrailingLineThickness) - 2,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,
									0,TextLineHeight);
								DrawTextLine(Window,GetScreenFont(),FontPointSize,Scan->Name,
									PtrSize(Scan->Name),Scan->WhereX + BoundingBoxThickness
									+ LeadingLineThickness - CurrentXIndex,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,ePlain);
							}
						if (Scan == CurrentlyHilited)
							{
								DrawBoxFrame(Window,eWhite,Scan->WhereX + 1
									+ LeadingLineThickness - CurrentXIndex,Scan->WhereY + LeadingSpace
									+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex + 1,
									Scan->InstanceWidth - LeadingLineThickness
									- TrailingLineThickness - 2,TextLineHeight + 1 - 2);
							}
						if (Scan->Type == eSymbolicLink)
							{
								How = eMediumGrey;
							}
						 else
							{
								How = eBlack;
							}
						/* box */
						DrawBoxFrame(Window,How,Scan->WhereX
							+ LeadingLineThickness - CurrentXIndex,Scan->WhereY + LeadingSpace
							+ (Scan->Height - TextLineHeight) / 2 - CurrentYIndex,
							Scan->InstanceWidth - LeadingLineThickness
							- TrailingLineThickness,TextLineHeight + 1);
						/* leading line */
						DrawLine(Window,eBlack,Scan->WhereX - CurrentXIndex,Scan->WhereY
							+ LeadingSpace + (Scan->Height) / 2 - CurrentYIndex,
							LeadingLineThickness,0);
						/* trailing line */
						if (Scan->Type != eFile)
							{
								DrawLine(Window,eBlack,Scan->WhereX + Scan->InstanceWidth
									- TrailingLineThickness - CurrentXIndex,Scan->WhereY + LeadingSpace
									+ (Scan->Height) / 2 - CurrentYIndex,TrailingLineThickness,0);
								if (Scan->Type == eDirectory)
									{
										/* recursive indraw for directories */
										if (Scan->Down == NIL)
											{
												/* empty directory */
												DrawLine(Window,eBlack,Scan->WhereX + Scan->InstanceWidth
													- CurrentXIndex + 0,Scan->WhereY + LeadingSpace
													+ (Scan->Height) / 2 - CurrentYIndex - 3,0,6);
												DrawLine(Window,eBlack,Scan->WhereX + Scan->InstanceWidth
													- CurrentXIndex + 3,Scan->WhereY + LeadingSpace
													+ (Scan->Height) / 2 - CurrentYIndex - 2,0,4);
												DrawLine(Window,eBlack,Scan->WhereX + Scan->InstanceWidth
													- CurrentXIndex + 6,Scan->WhereY + LeadingSpace
													+ (Scan->Height) / 2 - CurrentYIndex - 1,0,2);
											}
										else
											{
												Node*					SubScan;
												OrdType				Upper;
												OrdType				Lower;

												RedrawArea(Scan->Down);
												/* this item was a non-empty directory.  Draw from the */
												/* midpoint of the first item to the midpoint of the */
												/* last item */
												SubScan = Scan->Down;
												Upper = SubScan->WhereY + LeadingSpace
													+ (SubScan->Height) / 2 - CurrentYIndex;
												while (SubScan->Next != NIL)
													{
														SubScan = SubScan->Next;
													}
												Lower = SubScan->WhereY + LeadingSpace
													+ (SubScan->Height) / 2 - CurrentYIndex;
												DrawLine(Window,eBlack,Scan->WhereX + Scan->InstanceWidth
													- CurrentXIndex,Upper,0,Lower - Upper);
											}
									}
							}
					}
				Scan = Scan->Next;
			}
	}


/* clipping region must be properly set up before calling this */
void							RedrawAreaStart(MyBoolean DoAliases)
	{
		if (AliasesOnTop)
			{
				RedrawArea(GlobalList);
			}
		if ((DoAliases || AliasesOnTop) && ShowAliasLines)
			{
				AliasVec*				AliasScan;

				AliasScan = ListOfAliases;
				while (AliasScan != NIL)
					{
						DrawLine(Window,AliasPattern,AliasScan->StartX - CurrentXIndex,
							AliasScan->StartY - CurrentYIndex + LeadingSpace,
							AliasScan->DisplaceX,AliasScan->DisplaceY);
						AliasScan = AliasScan->Next;
					}
			}
		if (!AliasesOnTop)
			{
				RedrawArea(GlobalList);
			}
	}


void							DeselectAll(Node* List)
	{
		while (List != NIL)
			{
				List->SelectedFlag = False;
				DeselectAll(List->Down);
				List = List->Next;
			}
		CurrentlyHilited = NIL;
	}


/* X and Y must be normalized to the tree's origin */
MyBoolean					MouseDownSelect(Node* List, OrdType X, OrdType Y)
	{
		while (List != NIL)
			{
				if ((X >= List->WhereX) && (Y >= List->WhereY)
					&& (X < List->WhereX + List->Width)
					&& (Y < List->WhereY + List->Height))
					{
						OrdType			LocalX;
						OrdType			LocalY;
						OrdType			LocalWidth;
						OrdType			LocalHeight;

						LocalX = List->WhereX + LeadingLineThickness;
						LocalY = List->WhereY + (List->Height - TextLineHeight) / 2;
						LocalWidth = List->InstanceWidth - LeadingLineThickness
							- TrailingLineThickness;
						LocalHeight = TextLineHeight + 1;
						if ((X >= LocalX) && (Y >= LocalY) && (X < LocalX + LocalWidth)
							&& (Y < LocalY + LocalHeight))
							{
								/* found it! */
								List->SelectedFlag = !List->SelectedFlag;
								if (List->SelectedFlag)
									{
										CurrentlyHilited = List;
									}
								 else
									{
										CurrentlyHilited = NIL;
									}
								return True;
							}
						 else
							{
								if (MouseDownSelect(List->Down,X,Y))
									{
										return True;
									}
							}
					}
				List = List->Next;
			}
		return False;
	}


void							HScrollHook(long Parameter, ScrollType How, void* DontCare)
	{
		long						Old;

		Old = CurrentXIndex;
		switch (How)
			{
				case eScrollToPosition:
					CurrentXIndex = Parameter;
					break;
				case eScrollPageMinus:
					CurrentXIndex -= (GetWindowWidth(Window) - 15) - ScrollIncrement;
					break;
				case eScrollPagePlus:
					CurrentXIndex += (GetWindowWidth(Window) - 15) - ScrollIncrement;
					break;
				case eScrollLineMinus:
					CurrentXIndex -= ScrollIncrement;
					break;
				case eScrollLinePlus:
					CurrentXIndex += ScrollIncrement;
					break;
				default:
					EXECUTE(PRERR(AllowResume,"HScrollHook:  Unknown scroll opcode"));
					break;
			}
		if (CurrentXIndex > TotalWidth - (GetWindowWidth(Window) - 15))
			{
				CurrentXIndex = TotalWidth - (GetWindowWidth(Window) - 15);
			}
		if (CurrentXIndex < 0)
			{
				CurrentXIndex = 0;
			}
		SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - LeadingSpace - 15);
		ScrollArea(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - LeadingSpace - 15,Old - CurrentXIndex,0);
		if (Old > CurrentXIndex)
			{
				/* this means we opened a hole on the left edge */
				AddClipRect(Window,0,LeadingSpace,Old - CurrentXIndex,
					GetWindowHeight(Window) - LeadingSpace - 15);
				RedrawAreaStart(True);
			}
		else if (Old < CurrentXIndex)
			{
				/* this means we opened a hole on the right edge */
				AddClipRect(Window,GetWindowWidth(Window) - 15 - (CurrentXIndex - Old),
					LeadingSpace,CurrentXIndex - Old,GetWindowHeight(Window) - LeadingSpace - 15);
				RedrawAreaStart(True);
			}
		/* else we didn't change so don't bother redrawing */
		SetScrollIndex(HorizontalScroll,CurrentXIndex);
		RedrawScrollBar(HorizontalScroll);
	}


void							VScrollHook(long Parameter, ScrollType How, void* DontCare)
	{
		long						Old;

		Old = CurrentYIndex;
		switch (How)
			{
				case eScrollToPosition:
					CurrentYIndex = Parameter;
					break;
				case eScrollPageMinus:
					CurrentYIndex -= (GetWindowHeight(Window) - 15)
						- LeadingSpace - ScrollIncrement;
					break;
				case eScrollPagePlus:
					CurrentYIndex += (GetWindowHeight(Window) - 15)
						- LeadingSpace - ScrollIncrement;
					break;
				case eScrollLineMinus:
					CurrentYIndex -= ScrollIncrement;
					break;
				case eScrollLinePlus:
					CurrentYIndex += ScrollIncrement;
					break;
				default:
					EXECUTE(PRERR(AllowResume,"VScrollHook:  Unknown scroll opcode"));
					break;
			}
		if (CurrentYIndex > TotalHeight - (GetWindowHeight(Window) - 15 - LeadingSpace))
			{
				CurrentYIndex = TotalHeight - (GetWindowHeight(Window) - 15 - LeadingSpace);
			}
		if (CurrentYIndex < 0)
			{
				CurrentYIndex = 0;
			}
		SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - LeadingSpace - 15);
		ScrollArea(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - LeadingSpace - 15,0,Old - CurrentYIndex);
		if (Old > CurrentYIndex)
			{
				/* this means we opened a hole on the top edge */
				AddClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					Old - CurrentYIndex);
				RedrawAreaStart(True);
			}
		else if (Old < CurrentYIndex)
			{
				/* this means we opened a hole on the bottom edge */
				AddClipRect(Window,0,GetWindowHeight(Window) - 15 - (CurrentYIndex - Old),
					GetWindowWidth(Window) - 15,CurrentYIndex - Old);
				RedrawAreaStart(True);
			}
		/* else we didn't change so don't bother redrawing */
		SetScrollIndex(VerticalScroll,CurrentYIndex);
		RedrawScrollBar(VerticalScroll);
	}


void							UpdateProcedure(void* Refcon)
	{
		char*						TotalNumber;

		SetClipRect(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
		DrawLine(Window,eBlack,0,LeadingSpace - 1,GetWindowWidth(Window),0);
		DrawBitmap(Window,GetWindowWidth(Window) - 15,GetWindowHeight(Window) - 15,
			GetGrowIcon(True));
		TotalNumber = IntegerToString(DirectoryCount + FileCount + SymbolicLinkCount);
		if (TotalNumber != NIL)
			{
				char*						FileNumber;

				FileNumber = IntegerToString(FileCount);
				if (FileNumber != NIL)
					{
						char*					DirectoryNumber;

						DirectoryNumber = IntegerToString(DirectoryCount);
						if (DirectoryNumber != NIL)
							{
								char*					LinkNumber;

								LinkNumber = IntegerToString(SymbolicLinkCount);
								if (LinkNumber != NIL)
									{
										char*					Str1;

										Str1 = StringFromRaw(
											"_ Items:  _ Directories, _ Files, _ Links");
										if (Str1 != NIL)
											{
												char*					Str2;

												Str2 = StringToBlockCopy(Str1);
												if (Str2 != NIL)
													{
														char*						Key;

														Key = BlockFromRaw("_",1);
														if (Key != NIL)
															{
																char*						AfterFirst;

																AfterFirst = ReplaceBlockCopy(Str1,Key,TotalNumber);
																if (AfterFirst != NIL)
																	{
																		char*						AfterSecond;

																		AfterSecond = ReplaceBlockCopy(AfterFirst,
																			Key,DirectoryNumber);
																		if (AfterSecond != NIL)
																			{
																				char*						AfterThird;

																				AfterThird = ReplaceBlockCopy(AfterSecond,
																					Key,FileNumber);
																				if (AfterThird != NIL)
																					{
																						char*						AfterFourth;

																						AfterFourth = ReplaceBlockCopy(AfterThird,
																							Key,LinkNumber);
																						if (AfterFourth != NIL)
																							{
																								DrawTextLine(Window,GetScreenFont(),
																									FontPointSize,AfterFourth,
																									PtrSize(AfterFourth),4,0,eBold);
																								ReleasePtr(AfterFourth);
																							}
																						ReleasePtr(AfterThird);
																					}
																				ReleasePtr(AfterSecond);
																			}
																		ReleasePtr(AfterFirst);
																	}
																ReleasePtr(Key);
															}
														ReleasePtr(Str2);
													}
												ReleasePtr(Str1);
											}
										ReleasePtr(LinkNumber);
									}
								ReleasePtr(DirectoryNumber);
							}
						ReleasePtr(FileNumber);
					}
				ReleasePtr(TotalNumber);
			}
		RedrawScrollBar(HorizontalScroll);
		RedrawScrollBar(VerticalScroll);
		SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - LeadingSpace - 15);
		RedrawAreaStart(True);
	}


void							JumpSymbolicLink(void)
	{
		if (CurrentlyHilited != NIL)
			{
				if (CurrentlyHilited->Type == eSymbolicLink)
					{
						OrdType 					StupidX;
						OrdType						StupidY;
						Node*							WhichOne;
						FileSpec*					Target;

						SetWatchCursor();
						Target = DereferenceSymbolicLink(CurrentlyHilited->Location);
						if (Target != NIL)
							{
								WhichOne = FindMatchingItem(GlobalList,Target,&StupidX,&StupidY);
								DisposeFileSpec(Target);
								OtherOne = CurrentlyHilited;
								DeselectAll(GlobalList);
								CurrentlyHilited = WhichOne;
								if (CurrentlyHilited != NIL)
									{
										CurrentlyHilited->SelectedFlag = True;
									}
								SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
									GetWindowHeight(Window) - 15 - LeadingSpace);
								RedrawAreaStart(False);
								ShowSpecifiedItem(CurrentlyHilited);
							}
					}
			}
	}


void							ReturnToPreviousLink(void)
	{
		if (OtherOne != NIL)
			{
				Node*					Swapper;

				Swapper = CurrentlyHilited;
				DeselectAll(GlobalList);
				CurrentlyHilited = OtherOne;
				OtherOne = Swapper;
				CurrentlyHilited->SelectedFlag = True;
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


static void				HiliteRecursive(Node* List, FileSpec* DesiredTarget)
	{
		while (List != NIL)
			{
				if (List->Type == eSymbolicLink)
					{
						FileSpec*			Temp;

						Temp = DereferenceSymbolicLink(List->Location);
						if (Temp != NIL)
							{
								if (CompareFileSpecs(Temp,DesiredTarget))
									{
										if (CurrentlyHilited == NIL)
											{
												/* remember the first one */
												CurrentlyHilited = List;
											}
										List->SelectedFlag = True;
									}
								DisposeFileSpec(Temp);
							}
					}
				HiliteRecursive(List->Down,DesiredTarget);
				List = List->Next;
			}
	}


void							HiliteLinksToCurrent(void)
	{
		Node*						CurrentBackupCopy;

		if (CurrentlyHilited != NIL)
			{
				FileSpec*				Original;

				SetWatchCursor();
				CurrentBackupCopy = CurrentlyHilited;
				Original = CurrentlyHilited->Location;
				OtherOne = CurrentlyHilited;
				DeselectAll(GlobalList);
				HiliteRecursive(GlobalList,Original);
				if (CurrentlyHilited == NIL)
					{
						ErrorBeep();
						CurrentlyHilited = CurrentBackupCopy;
						CurrentlyHilited->SelectedFlag = True;
					}
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


void							ShowSpecifiedItem(Node* Item)
	{
		if (Item != NIL)
			{
				if ((Item->WhereX < CurrentXIndex + 15)
					|| (Item->WhereX + Item->InstanceWidth
					> CurrentXIndex + GetWindowWidth(Window) - 15 - 15))
					{
						HScrollHook(Item->WhereX
							- (GetWindowWidth(Window) - 15 - Item->InstanceWidth) / 2,
							eScrollToPosition,NIL);
					}
				if ((Item->WhereY + (Item->Height - TextLineHeight) / 2
					< CurrentYIndex + 15)
					|| (Item->WhereY + (Item->Height - TextLineHeight) / 2 + TextLineHeight
					> CurrentYIndex + GetWindowHeight(Window) - 15 - 15 - LeadingSpace))
					{
						VScrollHook(Item->WhereY + (Item->Height - TextLineHeight) / 2
							-	(GetWindowHeight(Window) - 15 - LeadingSpace) / 2,eScrollToPosition,NIL);
					}
			}
	}


typedef enum {eLookForCurrent, eLookForNext, eLookForLast} SearchStateType;

/* while FlippedOver is false, the procedure scans for CurrentlyHilited and */
/* then sets it True.  Then it stops at the next selected item */
static void				SelectSearch(Node* List, Node** BeforeCurrent,
										Node** AfterCurrent, Node** Last, SearchStateType* State)
	{
		while (List != NIL)
			{
				SelectSearch(List->Down,BeforeCurrent,AfterCurrent,Last,State);
				if (List->SelectedFlag)
					{
						switch (*State)
							{
								case eLookForCurrent:
									if (List == CurrentlyHilited)
										{
											/* found current! */
											*State = eLookForNext;
										}
									 else
										{
											*BeforeCurrent = List;
										}
									break;
								case eLookForNext:
									*AfterCurrent = List;
									*State = eLookForLast;
									break;
								case eLookForLast:
									*Last = List;
									break;
							}
					}
				List = List->Next;
			}
	}


void							AdvanceToNext(void)
	{
		if (CurrentlyHilited != NIL)
			{
				SearchStateType		State;
				Node*							BeforeCurrent;
				Node*							AfterCurrent;
				Node*							Last;

				State = eLookForCurrent;
				BeforeCurrent = NIL;
				AfterCurrent = NIL;
				Last = NIL;
				SelectSearch(GlobalList,&BeforeCurrent,&AfterCurrent,&Last,&State);
				if (BeforeCurrent == NIL)
					{
						BeforeCurrent = CurrentlyHilited;
					}
				if (AfterCurrent == NIL)
					{
						AfterCurrent = CurrentlyHilited;
					}
				if (Last == NIL)
					{
						Last = AfterCurrent;
					}
				CurrentlyHilited = AfterCurrent;
				CheckPtrExistence((char*)CurrentlyHilited); /* it had better not be NIL! */
				CurrentlyHilited->SelectedFlag = True;
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


void							AdvanceToPrevious(void)
	{
		if (CurrentlyHilited != NIL)
			{
				SearchStateType		State;
				Node*							BeforeCurrent;
				Node*							AfterCurrent;
				Node*							Last;

				State = eLookForCurrent;
				BeforeCurrent = NIL;
				AfterCurrent = NIL;
				Last = NIL;
				SelectSearch(GlobalList,&BeforeCurrent,&AfterCurrent,&Last,&State);
				if (BeforeCurrent == NIL)
					{
						BeforeCurrent = CurrentlyHilited;
					}
				if (AfterCurrent == NIL)
					{
						AfterCurrent = CurrentlyHilited;
					}
				if (Last == NIL)
					{
						Last = AfterCurrent;
					}
				CurrentlyHilited = BeforeCurrent;
				CheckPtrExistence((char*)CurrentlyHilited); /* it had better not be NIL! */
				CurrentlyHilited->SelectedFlag = True;
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


void							EnterSubLevel(void)
	{
		if (CurrentlyHilited != NIL)
			{
				/* enter nearest contained level */
				if (CurrentlyHilited->Down != NIL)
					{
						OrdType					Index;
						Node*						Scan;

						Scan = CurrentlyHilited->Down;
						Index = 0;
						while ((Scan->Next != NIL) && (CurrentlyHilited->Height / 2
							>= Index + Scan->Height))
							{
								Index += Scan->Height;
								Scan = Scan->Next;
							}
						DeselectAll(GlobalList);
						Scan->SelectedFlag = True;
						CurrentlyHilited = Scan;
						SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
							GetWindowHeight(Window) - 15 - LeadingSpace);
					}
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


static MyBoolean		ExitCurrentLevel(Node* List)
	{
		Node*							Scan;

		Scan = List;
		while (Scan != NIL)
			{
				if (ExitCurrentLevel(Scan->Down))
					{
						if (CurrentlyHilited == NIL)
							{
								CurrentlyHilited = Scan;
							}
						return True;
					}
				if (Scan == CurrentlyHilited)
					{
						CurrentlyHilited = NIL;
						return True;
					}
				Scan = Scan->Next;
			}
		return False;
	}


void							ExitLevel(void)
	{
		if (CurrentlyHilited != NIL)
			{
				Node*						Temp;
				Node*						Old;

				Old = CurrentlyHilited;
				ExitCurrentLevel(GlobalList);
				Temp = CurrentlyHilited;
				if (Temp == NIL)
					{
						Temp = Old;
					}
				DeselectAll(GlobalList);
				Temp->SelectedFlag = True;
				CurrentlyHilited = Temp;
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(Temp);
			}
	}


void							GoDown(MyBoolean JumpToBottom)
	{
		if (CurrentlyHilited != NIL)
			{
				Node*					Scan;

				Scan = CurrentlyHilited;
			 Loopish:
				if (Scan->Next != NIL)
					{
						Scan = Scan->Next;
						if (JumpToBottom)
							{
								goto Loopish;
							}
					}
				DeselectAll(GlobalList);
				Scan->SelectedFlag = True;
				CurrentlyHilited = Scan;
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(Scan);
			}
	}


void							GoUp(MyBoolean JumpToTop)
	{
		if (CurrentlyHilited != NIL)
			{
				Node*					Old;

				Old = CurrentlyHilited;
				if (ExitCurrentLevel(GlobalList))
					{
						Node*					Temp;

						ERROR(CurrentlyHilited->Down == NIL,
							PRERR(ForceAbort,"GoUp:  Internal error:  list is inconsistent"));
						Temp = CurrentlyHilited->Down;
						if (!JumpToTop)
							{
								while ((Temp != Old) && Temp->Next != Old)
									{
										Temp = Temp->Next;
										ERROR(Temp==NIL,PRERR(ForceAbort,"GoUp: Internal Error list is inconsistent"));
									}
							}
						DeselectAll(GlobalList);
						Temp->SelectedFlag = True;
						CurrentlyHilited = Temp;
					}
				SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
					GetWindowHeight(Window) - 15 - LeadingSpace);
				RedrawAreaStart(False);
				ShowSpecifiedItem(CurrentlyHilited);
			}
	}


/* performs case insensitive comparison to see if Instance is contained in Search */
/* this could be improved to use wildcards. */
static MyBoolean		TextPtrsAreEqual(char* Instance, char* Search)
	{
		long						InstanceLength;
		long						SearchLength;
		long						Scan;

		InstanceLength = PtrSize(Instance);
		SearchLength = PtrSize(Search);
		for (Scan = 0; Scan <= SearchLength - InstanceLength; Scan += 1)
			{
				long						Index;

				for (Index = 0; Index < InstanceLength; Index += 1)
					{
						char						Left;
						char						Right;

						Left = Instance[Index];
						if ((Left >= 'A') && (Left <= 'Z'))
							{
								Left = Left - 'A' + 'a';
							}
						Right = Search[Scan + Index];
						if ((Right >= 'A') && (Right <= 'Z'))
							{
								Right = Right - 'A' + 'a';
							}
						if (Left != Right)
							{
								goto SkipToNextPoint;
							}
					}
				return True; /* we get here if Instance was entirely in Search */
			 SkipToNextPoint:
				;
			}
		return False;
	}


typedef enum {eFindLookForCurrent, eFindLookForNext, eFindStop} FindStateType;

static void				RecursiveFind(Node* List, Node** Found, FindStateType* State)
	{
		while (List != NIL)
			{
				switch (*State)
					{
						case eFindLookForCurrent:
							if (List == CurrentlyHilited)
								{
									/* found current! */
									*State = eLookForNext;
								}
							break;
						case eFindLookForNext:
							if (TextPtrsAreEqual(SearchKey,List->Name))
								{
									*Found = List;
									*State = eFindStop;
								}
							break;
						case eFindStop:
							return;
							break;
					}
				RecursiveFind(List->Down,Found,State);
				List = List->Next;
			}
	}


void							Find(void)
	{
		Node*						Found;
		FindStateType		State;

		Found = NIL;
		if (CurrentlyHilited == NIL)
			{
				State = eFindLookForNext;
			}
		 else
			{
				State = eFindLookForCurrent;
			}
		RecursiveFind(GlobalList,&Found,&State);
		DeselectAll(GlobalList);
		if (Found == NIL)
			{
				ErrorBeep();
			}
		 else
			{
				CurrentlyHilited = Found;
				CurrentlyHilited->SelectedFlag = True;
			}
		OtherOne = NIL;
		SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
			GetWindowHeight(Window) - 15 - LeadingSpace);
		RedrawAreaStart(False);
		ShowSpecifiedItem(CurrentlyHilited);
	}


void							ShowFileInfoForSelection(Node* Item)
	{
		FileInfoRec			Info;

		CheckPtrExistence(Item);
		if (Item->Location != NIL)
			{
				if (GetFileStatistics(Item->Location,&Info))
					{
					}
			}
	}


int								main(int argc, char* argv[])
	{
		MyBoolean				GotTheDirectory;
		MyBoolean				ContinueFlag;
		FileSpec*				RootDirectory;

		APRINT(("+main"));

		/* level 0 initialization */
		if (!InitializeScreen())
			{
				goto TotalFailureEscapePoint;
			}

		/* level 1 initialization */
		InitializeAlertSubsystem();
		InitializeGrowIcon();

		/* application initialization */
		mmAppleMenu = MakeAppleMenu();
		mAboutProgram = MakeNewMenuItem(mmAppleMenu,"About DirTree...",0);
		ShowMenu(mmAppleMenu);
		mmFileMenu = MakeNewMenu("File");
		mShowAliasLines = MakeNewMenuItem(mmFileMenu,"Show Alias Paths",0);
		mShowAliasesOnTop = MakeNewMenuItem(mmFileMenu,"Alias Paths Above Text",0);
		AppendSeparator(mmFileMenu);
		mClose = MakeNewMenuItem(mmFileMenu,"Close",'W');
		AppendSeparator(mmFileMenu);
		mQuit = MakeNewMenuItem(mmFileMenu,"Quit",'Q');
		ShowMenu(mmFileMenu);
		mmEditMenu = MakeNewMenu("Edit");
		mUndo = MakeNewMenuItem(mmEditMenu,"Undo",'Z');
		AppendSeparator(mmEditMenu);
		mCut = MakeNewMenuItem(mmEditMenu,"Cut",'X');
		mCopy = MakeNewMenuItem(mmEditMenu,"Copy",'C');
		mPaste = MakeNewMenuItem(mmEditMenu,"Paste",'V');
		mClear = MakeNewMenuItem(mmEditMenu,"Clear",0);
		mSelectAll = MakeNewMenuItem(mmEditMenu,"Select All",0);
		AppendSeparator(mmEditMenu);
		mJumpSymbolicLink = MakeNewMenuItem(mmEditMenu,"Jump To Link Target",'J');
		mReturnToPrevious = MakeNewMenuItem(mmEditMenu,"Return To Previous Item",'R');
		mWhoLinksToThisOne = MakeNewMenuItem(mmEditMenu,"Who Links To This File",'A');
		AppendSeparator(mmEditMenu);
		mShowCurrentItem = MakeNewMenuItem(mmEditMenu,"Show Current Selection",'S');
		mGetFileInfo = MakeNewMenuItem(mmEditMenu,"Get Info...",'I');
		AppendSeparator(mmEditMenu);
		mFind = MakeNewMenuItem(mmEditMenu,"Find...",'F');
		mFindAgain = MakeNewMenuItem(mmEditMenu,"Find Again",'G');
		ShowMenu(mmEditMenu);

		/* find out which directory to start with. */
		PrepareStartupDocuments(argc,argv);
		GotTheDirectory = False;
		SearchKey = AllocPtrCanFail(0,"SearchKey");
		do
			{
				/* dummy event loop to let Macintosh receive open events */
				GetAnEvent(NIL,NIL,NIL,NIL,NIL,NIL);
				/* see if anything came in */
				if (GetStartupObject(&RootDirectory))
					{
						GotTheDirectory = True;
						/* if RootDirectory is NIL, then we do the entire file system, */
						/* otherwise we do the directory that it contains */
					}
			} while (!GotTheDirectory);

		/* create the graphics window */
		Window = MakeNewWindow(eDocumentWindow,eWindowClosable,eWindowZoomable,
			eWindowResizable,
			XLEAD + WindowOtherEdgeWidths(eDocumentWindow),
			1 + WindowTitleBarHeight(eDocumentWindow),
			GetScreenWidth() - (XLEAD + WindowOtherEdgeWidths(eDocumentWindow)) - 1 - 2,
			GetScreenHeight() - (1 + WindowTitleBarHeight(eDocumentWindow)) - 1 - 2,
			&UpdateProcedure,NIL);
		if (Window == 0)
			{
			 Oops:
				AlertHalt("Error:  Couldn't open window.",NIL);
				return 0;
			}
		TextLineHeight = GetFontHeight(GetScreenFont(),FontPointSize);
		LeadingSpace = TextLineHeight * NumStatusLines + 2;
		VerticalScroll = NewScrollBar(Window,eVScrollBar,GetWindowWidth(Window) - 15,
			LeadingSpace -1,GetWindowHeight(Window) + 2 - LeadingSpace - 15);
		if (VerticalScroll == NIL)
			{
				goto Oops;
			}
		HorizontalScroll = NewScrollBar(Window,eHScrollBar,-1,GetWindowHeight(Window) - 15,
			GetWindowWidth(Window) + 2 - 15);
		if (HorizontalScroll == NIL)
			{
				goto Oops;
			}

		/* digest the file system */
		GlobalList = ListFromDirectoryStart(RootDirectory);
		if (RootDirectory != NIL)
			{
				DisposeFileSpec(RootDirectory);
			}
		/* at least one item in GlobalList -- the root directory */
		if (ErrorOccurredFlag)
			{
				AlertHalt("The program ran out of memory while analyzing "
					"the specified directory.",NIL);
				goto AwwSkipit;
			}
		FillInSizeFields(GlobalList,5,5,&TotalWidth,&TotalHeight);
		TotalWidth += 5;
		TotalHeight += 5;
		ListOfAliases = NIL;
		ResolveAliases(GlobalList);
		CurrentXIndex = 0;
		CurrentYIndex = (TotalHeight - (GetWindowHeight(Window) - LeadingSpace - 15)) / 2;
		SetMaxScrollIndex(VerticalScroll,TotalHeight - (GetWindowHeight(Window)
			- LeadingSpace - 15) + 1);
		SetScrollIndex(VerticalScroll,CurrentYIndex);
		SetMaxScrollIndex(HorizontalScroll,TotalWidth - (GetWindowWidth(Window) - 15) + 1);
		SetScrollIndex(HorizontalScroll,CurrentXIndex);

		/* perform main event loop */
		ContinueFlag = True;
		while (ContinueFlag)
			{
				OrdType					XLoc;
				OrdType					YLoc;
				ModifierFlags		Modifiers;
				EventType				TheEvent;
				char						KeyPress;
				MenuItemType*		MenuItem;
				WinType*				EWin;

				TheEvent = GetAnEvent(&XLoc,&YLoc,&Modifiers,&EWin,&MenuItem,&KeyPress);
				switch (TheEvent)
					{
						case eNoEvent:
							break;
						case eKeyPressed:
							if (KeyPress == 9)
								{
									if ((Modifiers & eCommandKey) == 0)
										{
											AdvanceToNext();
										}
									 else
										{
											AdvanceToPrevious();
										}
								}
							else if ((Modifiers & eControlKey) != 0)
								{
									if (KeyPress == eLeftArrow)
										{
											if ((Modifiers & eCommandKey) != 0)
												{
													HScrollHook(0,eScrollToPosition,NIL);
												}
											else if ((Modifiers & eOptionKey) != 0)
												{
													HScrollHook(0,eScrollPageMinus,NIL);
												}
											else
												{
													HScrollHook(0,eScrollLineMinus,NIL);
												}
											RedrawScrollBar(HorizontalScroll);
										}
									else if (KeyPress == eRightArrow)
										{
											if ((Modifiers & eCommandKey) != 0)
												{
													HScrollHook(TotalWidth,eScrollToPosition,NIL);
												}
											else if ((Modifiers & eOptionKey) != 0)
												{
													HScrollHook(0,eScrollPagePlus,NIL);
												}
											else
												{
													HScrollHook(0,eScrollLinePlus,NIL);
												}
											RedrawScrollBar(HorizontalScroll);
										}
									else if (KeyPress == eUpArrow)
										{
											if ((Modifiers & eCommandKey) != 0)
												{
													VScrollHook(0,eScrollToPosition,NIL);
												}
											else if ((Modifiers & eOptionKey) != 0)
												{
													VScrollHook(0,eScrollPageMinus,NIL);
												}
											else
												{
													VScrollHook(0,eScrollLineMinus,NIL);
												}
											RedrawScrollBar(VerticalScroll);
										}
									else if (KeyPress == eDownArrow)
										{
											if ((Modifiers & eCommandKey) != 0)
												{
													VScrollHook(TotalHeight,eScrollToPosition,NIL);
												}
											else if ((Modifiers & eOptionKey) != 0)
												{
													VScrollHook(0,eScrollPagePlus,NIL);
												}
											else
												{
													VScrollHook(0,eScrollLinePlus,NIL);
												}
											RedrawScrollBar(VerticalScroll);
										}
								}
							else
								{
									if (KeyPress == eLeftArrow)
										{
											ExitLevel();
										}
									else if (KeyPress == eRightArrow)
										{
											EnterSubLevel();
										}
									else if (KeyPress == eUpArrow)
										{
											GoUp((Modifiers & eCommandKey) != 0);
										}
									else if (KeyPress == eDownArrow)
										{
											GoDown((Modifiers & eCommandKey) != 0);
										}
								}
							break;
						case eMouseDown:
							if (ScrollHitTest(HorizontalScroll,XLoc,YLoc))
								{
									ScrollHitProc(HorizontalScroll,Modifiers,XLoc,YLoc,NIL,&HScrollHook);
								}
							else if (ScrollHitTest(VerticalScroll,XLoc,YLoc))
								{
									ScrollHitProc(VerticalScroll,Modifiers,XLoc,YLoc,NIL,&VScrollHook);
								}
							else if ((XLoc >= GetWindowWidth(Window) - 15)
								&& (YLoc >= GetWindowHeight(Window) - 15)
								&& (XLoc < GetWindowWidth(Window)) && (YLoc < GetWindowHeight(Window)))
								{
									UserGrowWindow(Window,XLoc,YLoc);
									goto WindowSizeChangedPoint;
								}
							else if (XLoc >= LeadingSpace)
								{
									long double			TimerValue;

									TimerValue = ReadTimer();
									OtherOne = CurrentlyHilited;
									DeselectAll(GlobalList);
									MouseDownSelect(GlobalList,XLoc + CurrentXIndex,
										YLoc + CurrentYIndex - LeadingSpace);
									SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
										GetWindowHeight(Window) - 15 - LeadingSpace);
									RedrawAreaStart(False);
									if ((TimerDifference(TimerValue,LastClickTime)
										<= GetDoubleClickInterval())
										&& (XLoc - LastClickX <= 3) && (XLoc - LastClickX >= -3)
										&& (YLoc - LastClickY <= 3) && (YLoc - LastClickY >= -3))
										{
											if (CurrentlyHilited != NIL)
												{
													if (CurrentlyHilited->Type == eSymbolicLink)
														{
															JumpSymbolicLink();
														}
													 else
														{
															HiliteLinksToCurrent();
														}
												}
											LastClickX = -32767;
											LastClickY = -32767;
										}
									 else
										{
											LastClickTime = ReadTimer();
											LastClickX = XLoc;
											LastClickY = YLoc;
										}
								}
							break;
						case eMouseUp:
							break;
						case eCheckCursor:
							SetArrowCursor();
							break;
						case eMenuStarting:
							EnableMenuItem(mAboutProgram);
							EnableMenuItem(mQuit);
							EnableMenuItem(mClose);
							EnableMenuItem(mShowAliasesOnTop);
							if (AliasesOnTop)
								{
									SetItemCheckmark(mShowAliasesOnTop);
								}
							EnableMenuItem(mShowAliasLines);
							if (ShowAliasLines)
								{
									SetItemCheckmark(mShowAliasLines);
								}
							if (CurrentlyHilited != NIL)
								{
									if (CurrentlyHilited->Type == eSymbolicLink)
										{
											EnableMenuItem(mJumpSymbolicLink);
										}
									EnableMenuItem(mWhoLinksToThisOne);
									EnableMenuItem(mShowCurrentItem);
									EnableMenuItem(mGetFileInfo);
								}
							if (OtherOne != NIL)
								{
									EnableMenuItem(mReturnToPrevious);
								}
							EnableMenuItem(mFind);
							EnableMenuItem(mFindAgain);
							break;
						case eMenuCommand:
							if (MenuItem == mAboutProgram)
								{
									AlertInfo("DirTree directory displayer by Thomas R. Lawrence "
										"(tomlaw@world.std.com), 1993.",NIL);
								}
							else if ((MenuItem == mQuit) || (MenuItem == mClose))
								{
									ContinueFlag = False;
								}
							else if (MenuItem == mShowAliasesOnTop)
								{
									AliasesOnTop = !AliasesOnTop;
									SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
										GetWindowHeight(Window) - 15 - LeadingSpace);
									RedrawAreaStart(True);
								}
							else if (MenuItem == mShowAliasLines)
								{
									ShowAliasLines = !ShowAliasLines;
									SetClipRect(Window,0,LeadingSpace,GetWindowWidth(Window) - 15,
										GetWindowHeight(Window) - 15 - LeadingSpace);
									DrawBoxErase(Window,0,0,GetWindowWidth(Window),
										GetWindowHeight(Window));
									RedrawAreaStart(True);
								}
							else if (MenuItem == mJumpSymbolicLink)
								{
									JumpSymbolicLink();
								}
							else if (MenuItem == mReturnToPrevious)
								{
									ReturnToPreviousLink();
								}
							else if (MenuItem == mWhoLinksToThisOne)
								{
									HiliteLinksToCurrent();
								}
							else if (MenuItem == mShowCurrentItem)
								{
									ShowSpecifiedItem(CurrentlyHilited);
								}
							else if (MenuItem == mGetFileInfo)
								{
									ShowFileInfoForSelection(CurrentlyHilited);
								}
							else if (MenuItem == mFind)
								{
									if (DoStringDialog("Search for:",&SearchKey,
										mCut,mPaste,mCopy,mUndo,mSelectAll,mClear))
										{
											CurrentlyHilited = NIL;
											Find();
										}
								}
							else if (MenuItem == mFindAgain)
								{
									Find();
								}
							else
								{
									EXECUTE(PRERR(AllowResume,"main:  Unknown menu command"));
								}
							break;
						case eWindowClosing:
							ContinueFlag = False;
							break;
						case eWindowResized:
							/* clear the area */
						 WindowSizeChangedPoint:
							SetClipRect(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
							DrawBoxErase(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
							/* resize scrollbars */
							SetScrollLocation(VerticalScroll,GetWindowWidth(Window) - 15,
								LeadingSpace - 1,GetWindowHeight(Window) + 2 - LeadingSpace - 15);
							SetScrollLocation(HorizontalScroll,-1,GetWindowHeight(Window) - 15,
								GetWindowWidth(Window) + 2 - 15);
							SetMaxScrollIndex(VerticalScroll,TotalHeight - (GetWindowHeight(Window)
								- LeadingSpace - 15) + 1);
							SetScrollIndex(VerticalScroll,CurrentYIndex);
							SetMaxScrollIndex(HorizontalScroll,TotalWidth
								- (GetWindowWidth(Window) - 15) + 1);
							SetScrollIndex(HorizontalScroll,CurrentXIndex);
							/* move the button */
							/* redraw everything */
							UpdateProcedure(NIL);
							break;
						case eActiveWindowChanged:
							if (EWin == Window)
								{
									EnableScrollBar(VerticalScroll);
									EnableScrollBar(HorizontalScroll);
								}
							 else
								{
									DisableScrollBar(VerticalScroll);
									DisableScrollBar(HorizontalScroll);
								}
							SetClipRect(Window,0,0,GetWindowWidth(Window),GetWindowHeight(Window));
							DrawBitmap(Window,GetWindowWidth(Window) - 15,GetWindowHeight(Window) - 15,
								GetGrowIcon(EWin == Window));
							break;
					}
			}

	 AwwSkipit:
		/* application cleanup */
		ClearStartupDocuments();
		DisposeScrollBar(VerticalScroll);
		DisposeScrollBar(HorizontalScroll);
		KillWindow(Window);
		DisposeAliasList();
		DisposeList(GlobalList);
		ReleasePtr(SearchKey);
		KillMenuAndDeleteItems(mmAppleMenu);
		KillMenuAndDeleteItems(mmFileMenu);
		KillMenuAndDeleteItems(mmEditMenu);

		/* level 1 cleanup */
		ShutdownGrowIcon();
		ShutdownAlertSubsystem();

		/* level 0 cleanup */
		ShutdownScreen();

	 TotalFailureEscapePoint:
		APRINT(("-main"));
		return 0;
	}
