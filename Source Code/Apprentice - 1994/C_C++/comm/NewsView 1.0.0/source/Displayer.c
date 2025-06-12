/* Displayer.c */
/*****************************************************************************/
/*                                                                           */
/*    Offline USENET News Viewer                                             */
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
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "Displayer.h"
#include "WindowDispatcher.h"
#include "Array.h"
#include "StringList.h"
#include "Files.h"
#include "Memory.h"
#include "Alert.h"
#include "EditWindowRec.h"
#include "Main.h"
#include "DataMunging.h"


#define MAXARTICLENAMELENGTH (256)

#define INPUTBUFFERSIZE (4096)


typedef struct
	{
		long								FileStartPosition;
		long								BlockLength;
		char*								ArticleName;
	} ArticleAttribRec;


struct DisplayWindowRec
	{
		WinType*						ScreenID;
		StringListRec*			GroupList;
		ArrayRec*						GroupAttributesList; /* array of ArticleAttribRec's */
		ArrayRec*						WindowList; /* array of EditWindowRec's */
		FileType*						DataFile;
		GenericWindowRec*		GenericWindow;
		char*								StaticLineFeed;
	};


static ArrayRec*					OpenWindowList = NIL;


/* initialize the window crud */
MyBoolean						InitializeDisplayer(void)
	{
		OpenWindowList = NewArray();
		if (OpenWindowList == NIL)
			{
			 FailurePoint1:
				return False;
			}
		if (!InitializeEditWindow())
			{
			 FailurePoint2:
				DisposeArray(OpenWindowList);
				goto FailurePoint1;
			}
		return True;
	}


/* clean up internal structures */
void								KillWindowStuff(void)
	{
		ERROR(ArrayGetLength(OpenWindowList) != 0,PRERR(AllowResume,
			"KillWindowStuff:  window list isn't empty"));
		DisposeArray(OpenWindowList);
		ShutdownEditWindow();
	}


/* close all of the currently open things.  returns True if user didn't cancel. */
MyBoolean						DoCloseAllQuitPending(void)
	{
		long							Limit;
		long							Scan;

		Limit = ArrayGetLength(OpenWindowList);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				DisposeDisplayWindow(ArrayGetElement(OpenWindowList,Scan));
			}
		return True;
	}


/* states for parsing articles */
typedef enum
	{
		eRootState EXECUTE(= -13412),
		eRootStateGotSubject,
		eLineFeedState,
		eLineFeedStateGotSubject,
		eStateS,
		eStateU,
		eStateB,
		eStateJ,
		eStateE,
		eStateC,
		eStateT,
		eStateLineFeedPeriod,
		eStateDone
	} States;


#define ReadCharacter(Buffer,File,CharacterOut,BufferPosition) \
					(\
						((*(BufferPosition) % INPUTBUFFERSIZE) == 0)\
					?\
						(ReadCharacterHard((Buffer),(File),(CharacterOut),(BufferPosition)))\
					:\
						((*(CharacterOut)) = (Buffer)[(((*(BufferPosition))++)\
						% INPUTBUFFERSIZE)],True)\
					)

/* returns False if it fails */
static MyBoolean		ReadCharacterHard(char Buffer[INPUTBUFFERSIZE], FileType* File,
											unsigned char* CharacterOut, unsigned long* BufferPosition)
	{
		if ((*BufferPosition % INPUTBUFFERSIZE) == 0)
			{
				long					BytesNotRead;

				BytesNotRead = ReadFromFile(File,Buffer,INPUTBUFFERSIZE);
				if (BytesNotRead != 0)
					{
						if ((GetFileLength(File) != GetFilePosition(File))
							|| (*BufferPosition > GetFileLength(File)))
							{
								return False;
							}
					}
			}
		*CharacterOut = Buffer[(((*BufferPosition)++) % INPUTBUFFERSIZE)];
		return True;
	}


/* open a new display */
void								OpenDisplayWindow(FileSpec* TheFile)
	{
		DisplayWindowRec*	Window;
		unsigned long			FileLength;
		unsigned long			FilePosition;
		char*							ErrorMessage = NIL;
		char							Buffer[INPUTBUFFERSIZE];

		CheckPtrExistence(TheFile);
		Window = (DisplayWindowRec*)AllocPtrCanFail(sizeof(DisplayWindowRec),"DisplayWindowRec");
		if (Window == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint1:
				AlertHalt(ErrorMessage,NIL);
				DisposeFileSpec(TheFile);
				return;
			}

		/* create display elements */
		Window->ScreenID = MakeNewWindow(eDocumentWindow,eWindowClosable,eWindowZoomable,
			eWindowResizable,2 + WindowOtherEdgeWidths(eDocumentWindow),2
			+ WindowTitleBarHeight(eDocumentWindow),(GetScreenWidth() * 3) / 4,
			(GetScreenHeight() * 3) / 4,(void (*)(void*))&DisplayWindowDoUpdate,Window);
		if (Window->ScreenID == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint2:
				ReleasePtr((char*)Window);
				goto FailurePoint1;
			}
		Window->GroupList = NewStringList(Window->ScreenID,-1,-1,
			GetWindowWidth(Window->ScreenID) + 2,GetWindowHeight(Window->ScreenID) + 2,
			GetMonospacedFont(),9,StringListDontAllowMultipleSelection,NIL);
		if (Window->GroupList == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint3:
				KillWindow(Window->ScreenID);
				goto FailurePoint2;
			}
		Window->GroupAttributesList = NewArray();
		if (Window->GroupAttributesList == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint4:
				DisposeStringList(Window->GroupList);
				goto FailurePoint3;
			}
		Window->WindowList = NewArray();
		if (Window->WindowList == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint5:
				DisposeArray(Window->GroupAttributesList);
				goto FailurePoint4;
			}
		if (!OpenFile(TheFile,&(Window->DataFile),eReadOnly))
			{
				ErrorMessage = "The news file could not be opened.";
			 FailurePoint6:
				DisposeArray(Window->WindowList);
				goto FailurePoint5;
			}
		Window->GenericWindow = CheckInNewWindow(Window->ScreenID,Window,
			(void (*)(void*,MyBoolean,OrdType,OrdType,ModifierFlags))&DisplayWindowDoIdle,
			(void (*)(void*))&DisplayWindowBecomeActive,
			(void (*)(void*))&DisplayWindowBecomeInactive,
			(void (*)(void*))&DisplayWindowResized,
			(void (*)(OrdType,OrdType,ModifierFlags,void*))&DisplayWindowDoMouseDown,
			(void (*)(unsigned char,ModifierFlags,void*))&DisplayWindowDoKeyDown,
			(void (*)(void*))&DisplayWindowClose,
			(void (*)(void*))&DisplayWindowMenuSetup,
			(void (*)(void*,MenuItemType*))&DisplayWindowDoMenuCommand);
		if (Window->GenericWindow == NIL)
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint7:
				CloseFile(Window->DataFile);
				goto FailurePoint6;
			}
		if (!ArrayAppendElement(OpenWindowList,Window))
			{
				ErrorMessage = "There is not enough memory available to open the news file.";
			 FailurePoint8:
				CheckOutDyingWindow(Window->GenericWindow);
				goto FailurePoint7;
			}

		/* create group attribute list & fill in scrolling string list with names */
		Window->StaticLineFeed = NIL;
		FileLength = GetFileLength(Window->DataFile);
		FilePosition = 0;
		while (FilePosition < FileLength)
			{
				long							ArticleNameLength;
				char							ArticleNameBuffer[MAXARTICLENAMELENGTH];
				unsigned char			Character;
				MyBoolean					LoopFlag;
				long							ArticleStartPosition;
				States						SubjState;
				ArticleAttribRec*	Attributes;
				char*							TempStringThang;

				/* get the news group name for the current article */
				ArticleNameLength = 0;
				LoopFlag = True;
				while (LoopFlag)
					{
						if (!ReadCharacter(Buffer,Window->DataFile,&Character,&FilePosition))
							{
								ErrorMessage = "A disk error occurred while reading the file.";
							 FileReadFailurePoint1:
								while (ArrayGetLength(Window->GroupAttributesList))
									{
										ArticleAttribRec*	AttrPtr;

										AttrPtr = ArrayGetElement(Window->GroupAttributesList,0);
										CheckPtrExistence(AttrPtr);
										ReleasePtr(AttrPtr->ArticleName);
										ReleasePtr((char*)AttrPtr);
										ArrayDeleteElement(Window->GroupAttributesList,0);
									}
								ArrayDeleteElement(OpenWindowList,
									ArrayFindElement(OpenWindowList,Window));
								goto FailurePoint8;
							}
						if (Character == 13)
							{
								Window->StaticLineFeed = "\x0d";
								LoopFlag = False;
							}
						else if (Character == 10)
							{
								Window->StaticLineFeed = "\x0a";
								LoopFlag = False;
							}
						else
							{
								ArticleNameBuffer[ArticleNameLength] = Character;
								ArticleNameLength += 1;
							}
					}
				if (ArticleNameLength < MAXARTICLENAMELENGTH)
					{
						ArticleNameBuffer[ArticleNameLength] = ':';
						ArticleNameLength += 1;
					}
				if (ArticleNameLength < MAXARTICLENAMELENGTH)
					{
						ArticleNameBuffer[ArticleNameLength] = ' ';
						ArticleNameLength += 1;
					}
				if (ArticleNameLength < MAXARTICLENAMELENGTH)
					{
						ArticleNameBuffer[ArticleNameLength] = ' ';
						ArticleNameLength += 1;
					}

				/* find out where the article actually begins */
				ArticleStartPosition = FilePosition;

				/* extract the "Subject: " header field and then end of article */
				SubjState = eLineFeedState;
				while (SubjState != eStateDone)
					{
						if (!ReadCharacter(Buffer,Window->DataFile,&Character,&FilePosition))
							{
								ErrorMessage = "A disk error occurred while reading the file.";
								goto FileReadFailurePoint1;
							}
						switch (SubjState)
							{
								default:
									EXECUTE(PRERR(ForceAbort,"OpenDisplayWindow:  bad state"));
									break;
								case eRootState:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									break;
								case eRootStateGotSubject:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedStateGotSubject;
										}
									break;
								case eLineFeedState:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'S') || (Character == 's'))
										{
											SubjState = eStateS;
										}
									else if (Character == '.')
										{
											SubjState = eStateLineFeedPeriod;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eLineFeedStateGotSubject:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedStateGotSubject;
										}
									else if (Character == '.')
										{
											SubjState = eStateLineFeedPeriod;
										}
									else
										{
											SubjState = eRootStateGotSubject;
										}
									break;
								case eStateS:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'U') || (Character == 'u'))
										{
											SubjState = eStateU;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateU:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'B') || (Character == 'b'))
										{
											SubjState = eStateB;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateB:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'J') || (Character == 'j'))
										{
											SubjState = eStateJ;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateJ:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'E') || (Character == 'e'))
										{
											SubjState = eStateE;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateE:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'C') || (Character == 'c'))
										{
											SubjState = eStateC;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateC:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if ((Character == 'T') || (Character == 't'))
										{
											SubjState = eStateT;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateT:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eLineFeedState;
										}
									else if (Character == ':')
										{
											while (SubjState != eRootStateGotSubject)
												{
													if (!ReadCharacter(Buffer,Window->DataFile,
														&Character,&FilePosition))
														{
															ErrorMessage = "A disk error occurred while reading "
																"the file.";
															goto FileReadFailurePoint1;
														}
													if ((Character == 10) || (Character == 13))
														{
															SubjState = eRootStateGotSubject;
														}
													else
														{
															if (ArticleNameLength < MAXARTICLENAMELENGTH)
																{
																	ArticleNameBuffer[ArticleNameLength] = Character;
																	ArticleNameLength += 1;
																}
														}
												}
											/* invariant:  SubjState == eRootStateGotSubject */
										}
									else
										{
											SubjState = eRootState;
										}
									break;
								case eStateLineFeedPeriod:
									if ((Character == 10) || (Character == 13))
										{
											SubjState = eStateDone;
										}
									else
										{
											SubjState = eRootState;
										}
									break;
							}
					}

				/* construct the attributes record */
				Attributes = (ArticleAttribRec*)AllocPtrCanFail(sizeof(ArticleAttribRec),
					"ArticleAttribRec");
				if (Attributes == NIL)
					{
						ErrorMessage = "There is not enough memory available to open the news file.";
						goto FileReadFailurePoint1;
					}
				Attributes->ArticleName = BlockFromRaw(ArticleNameBuffer,ArticleNameLength);
				if (Attributes->ArticleName == NIL)
					{
						ErrorMessage = "There is not enough memory available to open the news file.";
					 FileReadFailurePoint2:
						ReleasePtr((char*)Attributes);
						goto FileReadFailurePoint1;
					}
				Attributes->FileStartPosition = ArticleStartPosition;
				Attributes->BlockLength = FilePosition - ArticleStartPosition;

				/* add thing to list */
				if (!ArrayAppendElement(Window->GroupAttributesList,Attributes))
					{
						ErrorMessage = "There is not enough memory available to open the news file.";
					 FileReadFailurePoint3:
						ReleasePtr(Attributes->ArticleName);
						goto FileReadFailurePoint2;
					}
				TempStringThang = BlockToStringCopy(Attributes->ArticleName);
				if (TempStringThang == NIL)
					{
						ErrorMessage = "There is not enough memory available to open the news file.";
					 FileReadFailurePoint4:
						ArrayDeleteElement(Window->GroupAttributesList,
							ArrayFindElement(Window->GroupAttributesList,Attributes));
						goto FileReadFailurePoint3;
					}
				if (!InsertStringListElement(Window->GroupList,TempStringThang,NIL,
					Attributes,False/*don'tredrawrightaway*/))
					{
						ErrorMessage = "There is not enough memory available to open the news file.";
					 FileReadFailurePoint5:
						ReleasePtr(TempStringThang);
						goto FileReadFailurePoint4;
					}
				ReleasePtr(TempStringThang);

				/* let the user do some other crud */
				RelinquishCPUJudiciouslyCheckCancel();
			}
		RedrawStringList(Window->GroupList);
		if (Window->StaticLineFeed == NIL)
			{
				Window->StaticLineFeed = SYSTEMLINEFEED;
			}
		DisposeFileSpec(TheFile);
	}


/* dispose of a window */
void								DisposeDisplayWindow(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DisposeStringList(Window->GroupList);
		CheckOutDyingWindow(Window->GenericWindow);
		CloseFile(Window->DataFile);
		while (ArrayGetLength(Window->WindowList) != 0)
			{
				DisposeEditWindow(ArrayGetElement(Window->WindowList,0));
			}
		DisposeArray(Window->WindowList);
		while (ArrayGetLength(Window->GroupAttributesList))
			{
				ArticleAttribRec*	AttrPtr;

				AttrPtr = ArrayGetElement(Window->GroupAttributesList,0);
				CheckPtrExistence(AttrPtr);
				ReleasePtr(AttrPtr->ArticleName);
				ReleasePtr((char*)AttrPtr);
				ArrayDeleteElement(Window->GroupAttributesList,0);
			}
		DisposeArray(Window->GroupAttributesList);
		KillWindow(Window->ScreenID);
		ArrayDeleteElement(OpenWindowList,ArrayFindElement(OpenWindowList,Window));
		ReleasePtr((char*)Window);
	}


/* register a new article window */
MyBoolean						DisplayWindowNewArticleWindow(DisplayWindowRec* Window,
											struct EditWindowRec* NewWindow)
	{
		CheckPtrExistence(Window);
		return ArrayAppendElement(Window->WindowList,NewWindow);
	}


/* article window calls this when it is going bye-bye */
void								DisplayWindowArticleDead(DisplayWindowRec* Window,
											struct EditWindowRec* DeadWindow)
	{
		CheckPtrExistence(Window);
		ArrayDeleteElement(Window->WindowList,ArrayFindElement(Window->WindowList,DeadWindow));
	}


void								DisplayWindowDoIdle(DisplayWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers)
	{
		CheckPtrExistence(Window);
		if (CheckCursorFlag)
			{
				SetArrowCursor();
			}
	}


void								DisplayWindowBecomeActive(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		EnableStringList(Window->GroupList);
	}


void								DisplayWindowBecomeInactive(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DisableStringList(Window->GroupList);
	}


void								DisplayWindowResized(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		SetStringListLoc(Window->GroupList,-1,-1,GetWindowWidth(Window->ScreenID) + 2,
			GetWindowHeight(Window->ScreenID) + 2);
	}


static void					OpenANewArticle(DisplayWindowRec* Window)
	{
		ArrayRec*					Selection;
		long							Limit;
		long							Scan;
		char*							ErrorMessage = NIL;

		Selection = GetListOfSelectedItems(Window->GroupList);
		if (Selection == NIL)
			{
				ErrorMessage = "There is not enough memory available to display the article.";
			 FailurePoint0:
				AlertHalt(ErrorMessage,NIL);
				return;
			}
		Limit = ArrayGetLength(Selection);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				char*							Buffer;
				ArticleAttribRec*	AttrPtr;
				EditWindowRec*		EditWindow;

				AttrPtr = ArrayGetElement(Selection,Scan);
				CheckPtrExistence(AttrPtr);
				Buffer = AllocPtrCanFail(AttrPtr->BlockLength,"ArticleBuffer");
				if (Buffer == NIL)
					{
						ErrorMessage = "There is not enough memory available to "
							"display the article.";
					 FailurePoint1:
						DisposeArray(Selection);
						goto FailurePoint0;
					}
				if (!SetFilePosition(Window->DataFile,AttrPtr->FileStartPosition))
					{
						ErrorMessage = "The article could not be read from the file.";
					 FailurePoint2:
						ReleasePtr(Buffer);
						goto FailurePoint1;
					}
				if (0 != ReadFromFile(Window->DataFile,Buffer,AttrPtr->BlockLength))
					{
						ErrorMessage = "The article could not be read from the file.";
					 FailurePoint3:
						goto FailurePoint2;
					}
				EditWindow = NewEditWindow(Buffer,Window->StaticLineFeed,
					AttrPtr->ArticleName,Window);
				if (EditWindow == NIL)
					{
						ErrorMessage = "There is not enough memory available to "
							"display the article.";
					 FailurePoint4:
						goto FailurePoint3;
					}
				ReleasePtr(Buffer);
			}
		DisposeArray(Selection);
	}


void								DisplayWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, DisplayWindowRec* Window)
	{
		OrdType						XSize;
		OrdType						YSize;

		CheckPtrExistence(Window);
		XSize = GetWindowWidth(Window->ScreenID);
		YSize = GetWindowHeight(Window->ScreenID);
		if (((Modifiers & eCommandKey) != 0) && (XLoc >= XSize - 15)
			&& (YLoc >= YSize - 15) && (XLoc < XSize) && (YLoc < YSize))
			{
				UserGrowWindow(Window->ScreenID,XLoc,YLoc);
				DisplayWindowResized(Window);
			}
		else if (StringListHitTest(Window->GroupList,XLoc,YLoc))
			{
				if (StringListMouseDown(Window->GroupList,XLoc,YLoc,Modifiers))
					{
						OpenANewArticle(Window);
					}
			}
	}


void								DisplayWindowDoKeyDown(unsigned char KeyCode,
											ModifierFlags Modifiers, DisplayWindowRec* Window)
	{
		ArrayRec*					Selection;

		CheckPtrExistence(Window);
		switch (KeyCode)
			{
				case 13:  /* return key opens up the selected article */
					OpenANewArticle(Window);
					break;
				case eUpArrow:  /* up arrow selects the previous thing */
					if (ArrayGetLength(Window->GroupAttributesList) != 0)
						{
							Selection = GetListOfSelectedItems(Window->GroupList);
							if (Selection != NIL)
								{
									ArticleAttribRec*	Attr;

									if (ArrayGetLength(Selection) == 0)
										{
											Attr = ArrayGetElement(Window->GroupAttributesList,
												ArrayGetLength(Window->GroupAttributesList) - 1);
										}
									 else
										{
											long							Index;

											Index = ArrayFindElement(Window->GroupAttributesList,
												ArrayGetElement(Selection,0)) - 1;
											if (Index < 0)
												{
													Index = ArrayGetLength(Window->GroupAttributesList) - 1;
												}
											Attr = ArrayGetElement(Window->GroupAttributesList,Index);
										}
									SelectStringListElement(Window->GroupList,Attr);
									MakeStringListSelectionVisible(Window->GroupList);
									DisposeArray(Selection);
								}
						}
					break;
				case eDownArrow:  /* up arrow selects the previous thing */
					if (ArrayGetLength(Window->GroupAttributesList) != 0)
						{
							Selection = GetListOfSelectedItems(Window->GroupList);
							if (Selection != NIL)
								{
									ArticleAttribRec*	Attr;

									if (ArrayGetLength(Selection) == 0)
										{
											Attr = ArrayGetElement(Window->GroupAttributesList,0);
										}
									 else
										{
											long							Index;

											Index = ArrayFindElement(Window->GroupAttributesList,
												ArrayGetElement(Selection,0)) + 1;
											if (Index >= ArrayGetLength(Window->GroupAttributesList))
												{
													Index = 0;
												}
											Attr = ArrayGetElement(Window->GroupAttributesList,Index);
										}
									SelectStringListElement(Window->GroupList,Attr);
									MakeStringListSelectionVisible(Window->GroupList);
									DisposeArray(Selection);
								}
						}
					break;
			}
	}


void								DisplayWindowClose(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		DisposeDisplayWindow(Window);
	}


void								DisplayWindowMenuSetup(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		EnableMenuItem(mCloseFile);
		ChangeItemName(mCloseFile,"Close News File");
	}


void								DisplayWindowDoMenuCommand(DisplayWindowRec* Window,
											struct MenuItemType* MenuItem)
	{
		CheckPtrExistence(Window);
		if (mCloseFile == MenuItem)
			{
				DisposeDisplayWindow(Window);
			}
		else
			{
				EXECUTE(PRERR(AllowResume,"DisplayWindowDoMenuCommand:  unknown menu command"));
			}
	}


void								DisplayWindowDoUpdate(DisplayWindowRec* Window)
	{
		CheckPtrExistence(Window);
		RedrawStringList(Window->GroupList);
	}
