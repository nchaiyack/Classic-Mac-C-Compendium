/* AlgoWaveTableList.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "AlgoWaveTableList.h"
#include "StringList.h"
#include "Array.h"
#include "Memory.h"
#include "Alert.h"
#include "DataMunging.h"
#include "AlgoWaveTableObject.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"
#include "Files.h"
#include "Scrap.h"


struct AlgoWaveTableListRec
	{
		StringListRec*					List;
		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		ArrayRec*								AlgoWaveTableArray;
		MyBoolean								AlgoWaveTableListChanged;
	};


#define MAGICSCRAPSTRING ("\xff\x00\x1f\xfe AlgoWaveTableObjectScrap")


AlgoWaveTableListRec*	NewAlgoWaveTableList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		AlgoWaveTableListRec*	AlgoWaveTableList;

		AlgoWaveTableList = (AlgoWaveTableListRec*)AllocPtrCanFail(sizeof(AlgoWaveTableListRec),
			"AlgoWaveTableListRec");
		if (AlgoWaveTableList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		AlgoWaveTableList->AlgoWaveTableArray = NewArray();
		if (AlgoWaveTableList->AlgoWaveTableArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)AlgoWaveTableList);
				goto FailurePoint1;
			}
		AlgoWaveTableList->List = NewStringList(ScreenID,XLoc,YLoc,Width,Height,
			GetScreenFont(),9,StringListDontAllowMultipleSelection,"Algorithmic Wave Tables");
		if (AlgoWaveTableList->List == NIL)
			{
			 FailurePoint3:
				DisposeArray(AlgoWaveTableList->AlgoWaveTableArray);
				goto FailurePoint2;
			}
		AlgoWaveTableList->CodeCenter = CodeCenter;
		AlgoWaveTableList->MainWindow = MainWindow;
		AlgoWaveTableList->AlgoWaveTableListChanged = False;
		return AlgoWaveTableList;
	}


/* delete the algorithmic wave table list and all of the wave tables it contains */
void								DisposeAlgoWaveTableList(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				DisposeAlgoWaveTableObject(AlgoWaveTableTemp);
			}
		DisposeArray(AlgoWaveTableList->AlgoWaveTableArray);
		DisposeStringList(AlgoWaveTableList->List);
		ReleasePtr((char*)AlgoWaveTableList);
	}


/* change the location of the algorithmic wave table list in the window */
void								SetAlgoWaveTableListLocation(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(AlgoWaveTableList);
		SetStringListLoc(AlgoWaveTableList->List,XLoc,YLoc,Width,Height);
	}


/* redraw the list */
void								AlgoWaveTableListRedraw(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		RedrawStringList(AlgoWaveTableList->List);
	}


/* see if the specified coordinates falls inside the wave table list rectangle */
MyBoolean						AlgoWaveTableListHitTest(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc)
	{
		CheckPtrExistence(AlgoWaveTableList);
		return StringListHitTest(AlgoWaveTableList->List,XLoc,YLoc);
	}


/* handle a mouse down event for the algorithmic wave table list */
void								AlgoWaveTableListDoMouseDown(AlgoWaveTableListRec* AlgoWaveTableList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(AlgoWaveTableList);
		if (StringListMouseDown(AlgoWaveTableList->List,XLoc,YLoc,Modifiers))
			{
				/* if it returns true, then it was a double click */
				AlgoWaveTableListOpenSelection(AlgoWaveTableList);
			}
	}


/* called when the window becomes active */
void								AlgoWaveTableListBecomeActive(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		EnableStringList(AlgoWaveTableList->List);
	}


/* called when the window becomes inactive */
void								AlgoWaveTableListBecomeInactive(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		DisableStringList(AlgoWaveTableList->List);
	}


/* called when a selection is made in another list, so that this list */
/* is deselected */
void								AlgoWaveTableListDeselect(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		DeselectAllStringListElements(AlgoWaveTableList->List);
	}


/* check to see if there is a selection in this list */
MyBoolean						AlgoWaveTableListIsThereSelection(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		return (GetStringListHowManySelectedItems(AlgoWaveTableList->List) > 0);
	}


/* check to see if any of the algorithmic wave tables contained in this list need */
/* to be saved */
MyBoolean						DoesAlgoWaveTableListNeedToBeSaved(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		long							Scan;
		long							Limit;
		MyBoolean					Flag;

		CheckPtrExistence(AlgoWaveTableList);
		Flag = AlgoWaveTableList->AlgoWaveTableListChanged;
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; (Scan < Limit) && !Flag; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				if (HasAlgoWaveTableObjectBeenModified(AlgoWaveTableTemp))
					{
						Flag = True;
					}
			}
		return Flag;
	}


/* open an edit window for the selected algorithmic wave table */
void								AlgoWaveTableListOpenSelection(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(AlgoWaveTableList);
		ListOfSelections = GetListOfSelectedItems(AlgoWaveTableList->List);
		if (ListOfSelections != NIL)
			{
				long							Scan;
				long							Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

						AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
							ListOfSelections,Scan);
						AlgoWaveTableObjectOpenWindow(AlgoWaveTableTemp);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* create a new algorithmic wave table and open a window for it */
void								AlgoWaveTableListNewAlgoWaveTable(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		AlgoWaveTableObjectRec*	AlgoWaveTable;

		CheckPtrExistence(AlgoWaveTableList);
		/* create the object */
		AlgoWaveTable = NewAlgoWaveTableObject(AlgoWaveTableList->CodeCenter,
			AlgoWaveTableList->MainWindow,AlgoWaveTableList);
		if (AlgoWaveTable == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to create a new algorithmic "
					"wave table.",NIL);
				return;
			}
		/* add it to the string list */
		if (!InsertStringListElement(AlgoWaveTableList->List,NIL,NIL,AlgoWaveTable,True))
			{
			 FailurePoint2:
				DisposeAlgoWaveTableObject(AlgoWaveTable);
				goto FailurePoint1;
			}
		MainWindowDeselectAllOtherStringLists(AlgoWaveTableList->MainWindow,AlgoWaveTableList);
		SelectStringListElement(AlgoWaveTableList->List,AlgoWaveTable);
		MakeStringListSelectionVisible(AlgoWaveTableList->List);
		/* add it to the array */
		if (!ArrayAppendElement(AlgoWaveTableList->AlgoWaveTableArray,AlgoWaveTable))
			{
			 FailurePoint3:
				RemoveStringListElement(AlgoWaveTableList->List,AlgoWaveTable,True);
				goto FailurePoint2;
			}
		/* update our internal flags */
		AlgoWaveTableList->AlgoWaveTableListChanged = True;
		/* change the name in the list */
		AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableList,AlgoWaveTable);
		/* show the window */
		AlgoWaveTableObjectOpenWindow(AlgoWaveTable);
	}


/* delete the selected algorithmic wave table */
void								AlgoWaveTableListDeleteSelection(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(AlgoWaveTableList);
		ListOfSelections = GetListOfSelectedItems(AlgoWaveTableList->List);
		if (ListOfSelections != NIL)
			{
				long								Scan;
				long								Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						AlgoWaveTableObjectRec*	OneToZap;

						OneToZap = (AlgoWaveTableObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						AlgoWaveTableListDeleteAlgoWaveTable(AlgoWaveTableList,OneToZap);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* delete the explicitly specified algorithmic wave table */
void								AlgoWaveTableListDeleteAlgoWaveTable(AlgoWaveTableListRec* AlgoWaveTableList,
											struct AlgoWaveTableObjectRec* TheAlgoWaveTable)
	{
		long								Scan;
		long								Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				if (TheAlgoWaveTable == AlgoWaveTableTemp)
					{
						FileSpec*					BackupFileWhere;
						FileType*					BackupFile;
						MyBoolean					Success = False;

						BackupFileWhere = NewTempFileSpec(CODE4BYTES('?','?','?','?'),
							CODE4BYTES('?','?','?','?'));
						if (BackupFileWhere != NIL)
							{
								if (OpenFile(BackupFileWhere,&BackupFile,eReadAndWrite))
									{
										BufferedOutputRec*	Output;

										Output = NewBufferedOutput(BackupFile);
										if (Output != NIL)
											{
												if (WriteBufferedOutput(Output,sizeof(MAGICSCRAPSTRING),
													MAGICSCRAPSTRING))
													{
														if (AlgoWaveTableObjectWriteDataOut(TheAlgoWaveTable,Output)
															== eFileLoadNoError)
															{
																Success = True;
															}
													}
												if (!EndBufferedOutput(Output))
													{
														Success = False;
													}
											}
										 else
											{
												CloseFile(BackupFile);
											}
									}
								 else
									{
										DeleteFile(BackupFileWhere);
										DisposeFileSpec(BackupFileWhere);
									}
							}
						if (Success)
							{
								MainWindowNewDeleteUndoInfo(AlgoWaveTableList->MainWindow,BackupFileWhere,
									BackupFile);
								DisposeAlgoWaveTableObject(AlgoWaveTableTemp);
								RemoveStringListElement(AlgoWaveTableList->List,AlgoWaveTableTemp,True);
								ArrayDeleteElement(AlgoWaveTableList->AlgoWaveTableArray,Scan);
								AlgoWaveTableList->AlgoWaveTableListChanged = True;
							}
						 else
							{
								YesNoCancelType		Decision;

								Decision = AskYesNoCancel("Unable to save undo information for object.  "
									"Delete object anyway?",NIL,"Delete","Cancel",NIL/*nothirdbutton*/);
								if (Decision == eYes)
									{
										DisposeAlgoWaveTableObject(AlgoWaveTableTemp);
										RemoveStringListElement(AlgoWaveTableList->List,AlgoWaveTableTemp,True);
										ArrayDeleteElement(AlgoWaveTableList->AlgoWaveTableArray,Scan);
										AlgoWaveTableList->AlgoWaveTableListChanged = True;
									}
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"AlgoWaveTableListDeleteAlgoWaveTable:  couldn't find object"));
	}


/* the name of a algorithmic wave table has changed, so the name in the scrolling */
/* list must also be changed */
void								AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableListRec* AlgoWaveTableList,
											struct AlgoWaveTableObjectRec* TheAlgoWaveTable)
	{
		char*							AlgoWaveTableName;

		CheckPtrExistence(AlgoWaveTableList);
		CheckPtrExistence(TheAlgoWaveTable);
		ERROR(ArrayFindElement(AlgoWaveTableList->AlgoWaveTableArray,TheAlgoWaveTable) < 0,
			PRERR(ForceAbort,"AlgoWaveTableListAlgoWaveTableNameChanged:  unknown algowavetable"));
		AlgoWaveTableName = AlgoWaveTableObjectGetNameCopy(TheAlgoWaveTable);
		if (AlgoWaveTableName != NIL)
			{
				char*							AlgoWaveTableNameNullTerminated;

				AlgoWaveTableNameNullTerminated = BlockToStringCopy(AlgoWaveTableName);
				if (AlgoWaveTableNameNullTerminated != NIL)
					{
						ChangeStringListElementName(AlgoWaveTableList->List,
							AlgoWaveTableNameNullTerminated,TheAlgoWaveTable);
						ReleasePtr(AlgoWaveTableNameNullTerminated);
					}
				ReleasePtr(AlgoWaveTableName);
			}
	}


/* look for a specified algorithmic wave table.  returns NIL if not found.  the name is */
/* NOT null terminated */
struct AlgoWaveTableObjectRec*	AlgoWaveTableListLookupNamedAlgoWaveTable(
											AlgoWaveTableListRec* AlgoWaveTableList, char* Name)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		CheckPtrExistence(Name);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTable;
				char*										NameCopy;

				AlgoWaveTable = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				NameCopy = AlgoWaveTableObjectGetNameCopy(AlgoWaveTable);
				if (NameCopy != NIL)
					{
						if (PtrSize(Name) == PtrSize(NameCopy))
							{
								if (MemEqu(Name,NameCopy,PtrSize(Name)))
									{
										ReleasePtr(NameCopy);
										return AlgoWaveTable;
									}
							}
						ReleasePtr(NameCopy);
					}
			}
		return NIL;
	}


/* remove all data arrays for all algorithmic wave tables */
void								AlgoWaveTableListUnbuildAll(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				AlgoWaveTableObjectUnbuild(AlgoWaveTableTemp);
			}
	}


/* build all algorithmic wave tables.  returns True if successful. */
MyBoolean						AlgoWaveTableListMakeUpToDate(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				if (!AlgoWaveTableObjectMakeUpToDate(AlgoWaveTableTemp))
					{
						return False;
					}
			}
		return True;
	}


/* the document's name has changed, so we need to update the windows */
void								AlgoWaveTableListGlobalNameChange(AlgoWaveTableListRec*
											AlgoWaveTableList, char* NewFilename)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				AlgoWaveTableObjectGlobalNameChange(AlgoWaveTableTemp,NewFilename);
			}
	}


/*   4-byte little endian number of algorithmic wave table objects (positive 2s complement) */
/*   n-byte data for all the algorithmic wave table objects */


/* read algorithmic wave table objects from a file.  returns True if successful. */
FileLoadingErrors		AlgoWaveTableListReadData(AlgoWaveTableListRec* AlgoWaveTableList,
											struct BufferedInputRec* Input)
	{
		signed long				NumberOfObjects;
		long							Scan;

		CheckPtrExistence(AlgoWaveTableList);
		CheckPtrExistence(Input);

		/*   4-byte little endian number of objects (positive 2s complement) */
		if (!ReadBufferedSignedLongLittleEndian(Input,&NumberOfObjects))
			{
				return eFileLoadDiskError;
			}
		if (NumberOfObjects < 0)
			{
				return eFileLoadBadFormat;
			}

		/*   n-byte data for all the objects */
		for (Scan = 0; Scan < NumberOfObjects; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTable EXECUTE(= (AlgoWaveTableObjectRec*)0x81818181);
				FileLoadingErrors				Error;

				/* load the object */
				Error = AlgoWaveTableObjectNewFromFile(&AlgoWaveTable,Input,
					AlgoWaveTableList->CodeCenter,AlgoWaveTableList->MainWindow,AlgoWaveTableList);
				if (Error != eFileLoadNoError)
					{
					 FailurePoint1:
						return Error;
					}
				CheckPtrExistence(AlgoWaveTable);
				/* add it to the string list */
				if (!InsertStringListElement(AlgoWaveTableList->List,NIL,NIL,AlgoWaveTable,True))
					{
					 FailurePoint2:
						DisposeAlgoWaveTableObject(AlgoWaveTable);
						Error = eFileLoadOutOfMemory;
						goto FailurePoint1;
					}
				/* add it to the array */
				if (!ArrayAppendElement(AlgoWaveTableList->AlgoWaveTableArray,AlgoWaveTable))
					{
					 FailurePoint3:
						RemoveStringListElement(AlgoWaveTableList->List,AlgoWaveTable,True);
						goto FailurePoint2;
					}
				/* change the name in the list */
				AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableList,AlgoWaveTable);
			}

		return eFileLoadNoError;
	}


/* write algorithmic wave table objects to a file.  returns True if successful. */
FileLoadingErrors		AlgoWaveTableListWriteData(AlgoWaveTableListRec* AlgoWaveTableList,
											struct BufferedOutputRec* Output)
	{
		long							NumberOfObjects;
		long							Scan;

		CheckPtrExistence(AlgoWaveTableList);
		CheckPtrExistence(Output);

		/*   4-byte little endian number of objects (positive 2s complement) */
		NumberOfObjects = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfObjects))
			{
				return eFileLoadDiskError;
			}

		/*   n-byte data for all the objects */
		for (Scan = 0; Scan < NumberOfObjects; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;
				FileLoadingErrors				Error;

				/* get the object */
				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				/* write the object out */
				Error = AlgoWaveTableObjectWriteDataOut(AlgoWaveTableTemp,Output);
				/* handle errors */
				if (Error != eFileLoadNoError)
					{
						return Error;
					}
			}

		return eFileLoadNoError;
	}


/* after a file has been saved, this is called to mark all objects as not modified. */
void								AlgoWaveTableListMarkAllObjectsSaved(
											AlgoWaveTableListRec* AlgoWaveTableList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoWaveTableList);
		Limit = ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoWaveTableObjectRec*	AlgoWaveTableTemp;

				AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
					AlgoWaveTableList->AlgoWaveTableArray,Scan);
				AlgoWaveTableObjectMarkAsSaved(AlgoWaveTableTemp);
			}
		AlgoWaveTableList->AlgoWaveTableListChanged = False;
	}


/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						AlgoWaveTableListCopyObject(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		ArrayRec*							ListOfSelections;
		MyBoolean							TotalSuccessFlag = False;

		CheckPtrExistence(AlgoWaveTableList);
		ListOfSelections = GetListOfSelectedItems(AlgoWaveTableList->List);
		if (ListOfSelections != NIL)
			{
				if (ArrayGetLength(ListOfSelections) >= 1)
					{
						AlgoWaveTableObjectRec*	AlgoWaveTableTemp;
						FileSpec*						TempFileLocation;

						AlgoWaveTableTemp = (AlgoWaveTableObjectRec*)ArrayGetElement(
							ListOfSelections,0);
						/* open the temporary file */
						TempFileLocation = NewTempFileSpec(CODE4BYTES('\?','\?','\?','\?'),
							CODE4BYTES('\?','\?','\?','\?'));
						if (TempFileLocation != NIL)
							{
								FileType*							FileDescriptor;

								if (OpenFile(TempFileLocation,&FileDescriptor,eReadAndWrite))
									{
										BufferedOutputRec*		BufferedFile;

										BufferedFile = NewBufferedOutput(FileDescriptor);
										if (BufferedFile != NIL)
											{
												MyBoolean							WriteSucceeded = False;

												if (WriteBufferedOutput(BufferedFile,sizeof(MAGICSCRAPSTRING),
													MAGICSCRAPSTRING))
													{
														if (AlgoWaveTableObjectWriteDataOut(AlgoWaveTableTemp,
															BufferedFile) == eFileLoadNoError)
															{
																WriteSucceeded = True;
															}
													}
												if (EndBufferedOutput(BufferedFile) && WriteSucceeded)
													{
														char*							Buffer;
														long							NumberOfBytes;

														NumberOfBytes = GetFileLength(FileDescriptor);
														Buffer = AllocPtrCanFail(NumberOfBytes,
															"AlgoWaveTableListCopyObject:  scrap buffer");
														if (Buffer != NIL)
															{
																if (SetFilePosition(FileDescriptor,0))
																	{
																		if (0 == ReadFromFile(FileDescriptor,
																			Buffer,NumberOfBytes))
																			{
																				if (SetScrapToThis(Buffer))
																					{
																						TotalSuccessFlag = True;
																					}
																			}
																	}
																ReleasePtr(Buffer);
															}
													}
											}
										CloseFile(FileDescriptor);
									}
								DeleteFile(TempFileLocation);
								DisposeFileSpec(TempFileLocation);
							}
					}
				DisposeArray(ListOfSelections);
			}
		return TotalSuccessFlag;
	}


/* try to paste the clipboard in as an algorithmic wave table object.  returns False if */
/* it failed or the clipboard did not contain an algorithmic wave table object. */
MyBoolean						AlgoWaveTableListPasteObject(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		MyBoolean					TotalSuccessFlag = False;
		char*							Scrap;

		CheckPtrExistence(AlgoWaveTableList);
		Scrap = GetCopyOfScrap();
		if (Scrap != NIL)
			{
				FileSpec*					TempFileLocation;

				TempFileLocation = NewTempFileSpec(CODE4BYTES('\?','\?','\?','\?'),
					CODE4BYTES('\?','\?','\?','\?'));
				if (TempFileLocation != NIL)
					{
						FileType*							FileDescriptor;

						if (OpenFile(TempFileLocation,&FileDescriptor,eReadAndWrite))
							{
								BufferedOutputRec*		BufferedFile;

								BufferedFile = NewBufferedOutput(FileDescriptor);
								if (BufferedFile != NIL)
									{
										MyBoolean							WriteSucceeded = False;

										if (WriteBufferedOutput(BufferedFile,PtrSize(Scrap),Scrap))
											{
												WriteSucceeded = True;
											}
										if (EndBufferedOutput(BufferedFile) && WriteSucceeded)
											{
												TotalSuccessFlag = AlgoWaveTableListPasteFromFile(
													AlgoWaveTableList,FileDescriptor);
											}
									}
								CloseFile(FileDescriptor);
							}
						DeleteFile(TempFileLocation);
						DisposeFileSpec(TempFileLocation);
					}
				ReleasePtr(Scrap);
			}
		return TotalSuccessFlag;
	}


/* try to paste the algorithmic wave table object in from the file */
MyBoolean						AlgoWaveTableListPasteFromFile(AlgoWaveTableListRec* AlgoWaveTableList,
											struct FileType* File)
	{
		MyBoolean					TotalSuccessFlag = False;

		CheckPtrExistence(AlgoWaveTableList);
		if (SetFilePosition(File,0))
			{
				BufferedInputRec*	InputFile;

				InputFile = NewBufferedInput(File);
				if (InputFile != NIL)
					{
						char							HeaderTest[sizeof(MAGICSCRAPSTRING)];

						if (ReadBufferedInput(InputFile,sizeof(MAGICSCRAPSTRING),HeaderTest))
							{
								if (MemEqu(MAGICSCRAPSTRING,HeaderTest,sizeof(MAGICSCRAPSTRING)))
									{
										AlgoWaveTableObjectRec*		AlgoWaveTableTemp EXECUTE(= (AlgoWaveTableObjectRec*)0x81818181);

										if (eFileLoadNoError == AlgoWaveTableObjectNewFromFile(&AlgoWaveTableTemp,
											InputFile,AlgoWaveTableList->CodeCenter,AlgoWaveTableList->MainWindow,
											AlgoWaveTableList))
											{
												CheckPtrExistence(AlgoWaveTableTemp);
												/* add it to the scrolling object list */
												if (!InsertStringListElement(AlgoWaveTableList->List,NIL,NIL,AlgoWaveTableTemp,True))
													{
													 FailurePoint:
														DisposeAlgoWaveTableObject(AlgoWaveTableTemp);
													}
												 else
													{
														MainWindowDeselectAllOtherStringLists(AlgoWaveTableList->MainWindow,
															AlgoWaveTableList);
														SelectStringListElement(AlgoWaveTableList->List,AlgoWaveTableTemp);
														MakeStringListSelectionVisible(AlgoWaveTableList->List);
														/* add it to the array */
														if (!ArrayAppendElement(AlgoWaveTableList->AlgoWaveTableArray,AlgoWaveTableTemp))
															{
																RemoveStringListElement(AlgoWaveTableList->List,AlgoWaveTableTemp,True);
																goto FailurePoint;
															}
														 else
															{
																/* change the name in the list */
																AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableList,AlgoWaveTableTemp);
																TotalSuccessFlag = True;
																AlgoWaveTableList->AlgoWaveTableListChanged = True;
															}
													}
											}
									}
							}
						EndBufferedInput(InputFile);
					}
			}
		return TotalSuccessFlag;
	}


/* find out how many algorithmic wave tables there are in this list */
long								AlgoWaveTableListHowMany(AlgoWaveTableListRec* AlgoWaveTableList)
	{
		CheckPtrExistence(AlgoWaveTableList);
		return ArrayGetLength(AlgoWaveTableList->AlgoWaveTableArray);
	}


/* get an indexed algorithmic wave tables from the list */
struct AlgoWaveTableObjectRec*	AlgoWaveTableListGetIndexedAlgoWaveTable(
											AlgoWaveTableListRec* AlgoWaveTableList, long Index)
	{
		AlgoWaveTableObjectRec*	AlgoWaveTable;

		CheckPtrExistence(AlgoWaveTableList);
		ERROR((Index < 0) || (Index >= AlgoWaveTableListHowMany(AlgoWaveTableList)),
			PRERR(ForceAbort,"AlgoWaveTableListGetIndexedAlgoWaveTable:  index out of range"));
		AlgoWaveTable = (AlgoWaveTableObjectRec*)ArrayGetElement(
			AlgoWaveTableList->AlgoWaveTableArray,Index);
		CheckPtrExistence(AlgoWaveTable);
		return AlgoWaveTable;
	}
