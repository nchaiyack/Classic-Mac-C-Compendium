/* AlgoSampList.c */
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

#include "AlgoSampList.h"
#include "StringList.h"
#include "Array.h"
#include "Memory.h"
#include "AlgoSampObject.h"
#include "Alert.h"
#include "DataMunging.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"
#include "Files.h"
#include "Scrap.h"


struct AlgoSampListRec
	{
		StringListRec*					List;
		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		ArrayRec*								AlgoSampArray;
		MyBoolean								AlgoSampListChanged;
	};


#define MAGICSCRAPSTRING ("\xff\x00\x1f\xfe AlgoSampObjectScrap")


/* create a new algorithmic sample list */
AlgoSampListRec*		NewAlgoSampList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		AlgoSampListRec*	AlgoSampList;

		AlgoSampList = (AlgoSampListRec*)AllocPtrCanFail(sizeof(AlgoSampListRec),
			"AlgoSampListRec");
		if (AlgoSampList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		AlgoSampList->AlgoSampArray = NewArray();
		if (AlgoSampList->AlgoSampArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)AlgoSampList);
				goto FailurePoint1;
			}
		AlgoSampList->List = NewStringList(ScreenID,XLoc,YLoc,Width,Height,
			GetScreenFont(),9,StringListDontAllowMultipleSelection,"Algorithmic Samples");
		if (AlgoSampList->List == NIL)
			{
			 FailurePoint3:
				DisposeArray(AlgoSampList->AlgoSampArray);
				goto FailurePoint2;
			}
		AlgoSampList->CodeCenter = CodeCenter;
		AlgoSampList->MainWindow = MainWindow;
		AlgoSampList->AlgoSampListChanged = False;
		return AlgoSampList;
	}


/* delete the algorithmic sample list and all of the samples it contains */
void								DisposeAlgoSampList(AlgoSampListRec* AlgoSampList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				DisposeAlgoSampObject(AlgoSampTemp);
			}
		DisposeArray(AlgoSampList->AlgoSampArray);
		DisposeStringList(AlgoSampList->List);
		ReleasePtr((char*)AlgoSampList);
	}


/* change the location of the algorithmic sample list in the window */
void								SetAlgoSampListLocation(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(AlgoSampList);
		SetStringListLoc(AlgoSampList->List,XLoc,YLoc,Width,Height);
	}


/* redraw the list */
void								AlgoSampListRedraw(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		RedrawStringList(AlgoSampList->List);
	}


/* see if the specified coordinates falls inside the sample list rectangle */
MyBoolean						AlgoSampListHitTest(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc)
	{
		CheckPtrExistence(AlgoSampList);
		return StringListHitTest(AlgoSampList->List,XLoc,YLoc);
	}


/* handle a mouse down event for the algorithmic sample list */
void								AlgoSampListDoMouseDown(AlgoSampListRec* AlgoSampList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(AlgoSampList);
		if (StringListMouseDown(AlgoSampList->List,XLoc,YLoc,Modifiers))
			{
				/* if it returns true, then it was a double click */
				AlgoSampListOpenSelection(AlgoSampList);
			}
	}


/* called when the window becomes active */
void								AlgoSampListBecomeActive(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		EnableStringList(AlgoSampList->List);
	}


/* called when the window becomes inactive */
void								AlgoSampListBecomeInactive(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		DisableStringList(AlgoSampList->List);
	}


/* called when a selection is made in another list, so that this list */
/* is deselected */
void								AlgoSampListDeselect(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		DeselectAllStringListElements(AlgoSampList->List);
	}


/* check to see if there is a selection in this list */
MyBoolean						AlgoSampListIsThereSelection(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		return (GetStringListHowManySelectedItems(AlgoSampList->List) > 0);
	}


/* check to see if any of the algo samples contained in this list need */
/* to be saved */
MyBoolean						DoesAlgoSampListNeedToBeSaved(AlgoSampListRec* AlgoSampList)
	{
		long							Scan;
		long							Limit;
		MyBoolean					Flag;

		CheckPtrExistence(AlgoSampList);
		Flag = AlgoSampList->AlgoSampListChanged;
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; (Scan < Limit) && !Flag; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				if (HasAlgoSampObjectBeenModified(AlgoSampTemp))
					{
						Flag = True;
					}
			}
		return Flag;
	}


/* open an edit window for the selected algorithmic sample */
void								AlgoSampListOpenSelection(AlgoSampListRec* AlgoSampList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(AlgoSampList);
		ListOfSelections = GetListOfSelectedItems(AlgoSampList->List);
		if (ListOfSelections != NIL)
			{
				long							Scan;
				long							Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						AlgoSampObjectRec*	AlgoSampTemp;

						AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						AlgoSampObjectOpenWindow(AlgoSampTemp);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* create a new algorithmic sample and open a window for it */
void								AlgoSampListNewAlgoSamp(AlgoSampListRec* AlgoSampList)
	{
		AlgoSampObjectRec*	AlgoSamp;

		CheckPtrExistence(AlgoSampList);
		/* create the object */
		AlgoSamp = NewAlgoSampObject(AlgoSampList->CodeCenter,AlgoSampList->MainWindow,
			AlgoSampList);
		if (AlgoSamp == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to create a new "
					"algorithmic sample.",NIL);
				return;
			}
		/* add it to the string list */
		if (!InsertStringListElement(AlgoSampList->List,NIL,NIL,AlgoSamp,True))
			{
			 FailurePoint2:
				DisposeAlgoSampObject(AlgoSamp);
				goto FailurePoint1;
			}
		MainWindowDeselectAllOtherStringLists(AlgoSampList->MainWindow,AlgoSampList);
		SelectStringListElement(AlgoSampList->List,AlgoSamp);
		MakeStringListSelectionVisible(AlgoSampList->List);
		/* add it to the array */
		if (!ArrayAppendElement(AlgoSampList->AlgoSampArray,AlgoSamp))
			{
			 FailurePoint3:
				RemoveStringListElement(AlgoSampList->List,AlgoSamp,True);
				goto FailurePoint2;
			}
		/* update our internal flags */
		AlgoSampList->AlgoSampListChanged = True;
		/* change the name in the list */
		AlgoSampListAlgoSampNameChanged(AlgoSampList,AlgoSamp);
		/* show the window */
		AlgoSampObjectOpenWindow(AlgoSamp);
	}


/* delete the selected algorithmic sample */
void								AlgoSampListDeleteSelection(AlgoSampListRec* AlgoSampList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(AlgoSampList);
		ListOfSelections = GetListOfSelectedItems(AlgoSampList->List);
		if (ListOfSelections != NIL)
			{
				long								Scan;
				long								Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						AlgoSampObjectRec*	OneToZap;

						OneToZap = (AlgoSampObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						AlgoSampListDeleteAlgoSamp(AlgoSampList,OneToZap);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* delete the explicitly specified algorithmic sample */
void								AlgoSampListDeleteAlgoSamp(AlgoSampListRec* AlgoSampList,
											struct AlgoSampObjectRec* TheAlgoSamp)
	{
		long								Scan;
		long								Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				if (TheAlgoSamp == AlgoSampTemp)
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
														if (AlgoSampObjectWriteOutData(TheAlgoSamp,Output)
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
								MainWindowNewDeleteUndoInfo(AlgoSampList->MainWindow,BackupFileWhere,
									BackupFile);
								DisposeAlgoSampObject(AlgoSampTemp);
								RemoveStringListElement(AlgoSampList->List,AlgoSampTemp,True);
								ArrayDeleteElement(AlgoSampList->AlgoSampArray,Scan);
								AlgoSampList->AlgoSampListChanged = True;
							}
						 else
							{
								YesNoCancelType		Decision;

								Decision = AskYesNoCancel("Unable to save undo information for object.  "
									"Delete object anyway?",NIL,"Delete","Cancel",NIL/*nothirdbutton*/);
								if (Decision == eYes)
									{
										DisposeAlgoSampObject(AlgoSampTemp);
										RemoveStringListElement(AlgoSampList->List,AlgoSampTemp,True);
										ArrayDeleteElement(AlgoSampList->AlgoSampArray,Scan);
										AlgoSampList->AlgoSampListChanged = True;
									}
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"AlgoSampListDeleteAlgoSamp:  couldn't find object"));
	}


/* the name of a algorithmic sample has changed, so the name in the scrolling */
/* list must also be changed */
void								AlgoSampListAlgoSampNameChanged(AlgoSampListRec* AlgoSampList,
											struct AlgoSampObjectRec* TheAlgoSamp)
	{
		char*							AlgoSampName;

		CheckPtrExistence(AlgoSampList);
		CheckPtrExistence(TheAlgoSamp);
		ERROR(ArrayFindElement(AlgoSampList->AlgoSampArray,TheAlgoSamp) < 0,
			PRERR(ForceAbort,"AlgoSampListAlgoSampNameChanged:  unknown algosamp"));
		AlgoSampName = AlgoSampObjectGetNameCopy(TheAlgoSamp);
		if (AlgoSampName != NIL)
			{
				char*							AlgoSampNameNullTerminated;

				AlgoSampNameNullTerminated = BlockToStringCopy(AlgoSampName);
				if (AlgoSampNameNullTerminated != NIL)
					{
						ChangeStringListElementName(AlgoSampList->List,
							AlgoSampNameNullTerminated,TheAlgoSamp);
						ReleasePtr(AlgoSampNameNullTerminated);
					}
				ReleasePtr(AlgoSampName);
			}
	}


/* look for a specified algorithmic sample.  returns NIL if not found.  the name is */
/* NOT null terminated */
AlgoSampObjectRec*	AlgoSampListLookupNamedAlgoSamp(
											AlgoSampListRec* AlgoSampList, char* Name)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		CheckPtrExistence(Name);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSamp;
				char*								NameCopy;

				AlgoSamp = (AlgoSampObjectRec*)ArrayGetElement(AlgoSampList->AlgoSampArray,Scan);
				NameCopy = AlgoSampObjectGetNameCopy(AlgoSamp);
				if (NameCopy != NIL)
					{
						if (PtrSize(Name) == PtrSize(NameCopy))
							{
								if (MemEqu(Name,NameCopy,PtrSize(Name)))
									{
										ReleasePtr(NameCopy);
										return AlgoSamp;
									}
							}
						ReleasePtr(NameCopy);
					}
			}
		return NIL;
	}


/* remove all data arrays for all algorithmic samples */
void								AlgoSampListUnbuildAll(AlgoSampListRec* AlgoSampList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				AlgoSampObjectUnbuild(AlgoSampTemp);
			}
	}


/* build all algorithmic sample tables.  returns True if successful. */
MyBoolean						AlgoSampListMakeUpToDate(AlgoSampListRec* AlgoSampList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				if (!AlgoSampObjectMakeUpToDate(AlgoSampTemp))
					{
						return False;
					}
			}
		return True;
	}


/* document's name changed, so we have to update the windows */
void								AlgoSampListGlobalNameChange(AlgoSampListRec* AlgoSampList,
											char* NewFilename)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				AlgoSampObjectGlobalNameChange(AlgoSampTemp,NewFilename);
			}
	}


/*   4-byte little endian number of algorithmic sample objects (positive 2s complement) */
/*   n-byte data for the algorithmic sample objects */


/* read algorithmic sample objects from a file.  returns True if completely successful. */
FileLoadingErrors		AlgoSampListReadData(AlgoSampListRec* AlgoSampList,
											struct BufferedInputRec* Input)
	{
		signed long				NumSampleObjects;
		long							Scan;

		CheckPtrExistence(AlgoSampList);
		CheckPtrExistence(Input);

		/*   4-byte little endian number of objects (positive 2s complement) */
		if (!ReadBufferedSignedLongLittleEndian(Input,&NumSampleObjects))
			{
				return eFileLoadDiskError;
			}
		if (NumSampleObjects < 0)
			{
				return eFileLoadBadFormat;
			}

		/* load things from the file */
		for (Scan = 0; Scan < NumSampleObjects; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp EXECUTE(= (AlgoSampObjectRec*)0x81818181);
				FileLoadingErrors		Error;

				/* read in the object */
				Error = AlgoSampObjectNewFromFile(&AlgoSampTemp,Input,AlgoSampList->CodeCenter,
					AlgoSampList->MainWindow,AlgoSampList);
				if (Error != eFileLoadNoError)
					{
					 FailurePoint1:
						return Error;
					}
				CheckPtrExistence(AlgoSampTemp);
				/* add it to the string list */
				if (!InsertStringListElement(AlgoSampList->List,NIL,NIL,AlgoSampTemp,True))
					{
					 FailurePoint2:
						DisposeAlgoSampObject(AlgoSampTemp);
						Error = eFileLoadOutOfMemory;
						goto FailurePoint1;
					}
				/* add it to the array */
				if (!ArrayAppendElement(AlgoSampList->AlgoSampArray,AlgoSampTemp))
					{
					 FailurePoint3:
						RemoveStringListElement(AlgoSampList->List,AlgoSampTemp,True);
						goto FailurePoint2;
					}
				/* change the name in the list */
				AlgoSampListAlgoSampNameChanged(AlgoSampList,AlgoSampTemp);
			}

		return eFileLoadNoError;
	}


/* write algorithmic sample objects to a file.  returns True if completely successful. */
FileLoadingErrors		AlgoSampListWriteData(AlgoSampListRec* AlgoSampList,
											struct BufferedOutputRec* Output)
	{
		long							NumberOfObjects;
		long							Scan;

		CheckPtrExistence(AlgoSampList);
		CheckPtrExistence(Output);

		/*   4-byte little endian number of objects (positive 2s complement) */
		NumberOfObjects = ArrayGetLength(AlgoSampList->AlgoSampArray);
		if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfObjects))
			{
				return eFileLoadDiskError;
			}

		/* write out the objects */
		for (Scan = 0; Scan < NumberOfObjects; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;
				FileLoadingErrors		Error;

				/* get the object */
				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				/* write it out */
				Error = AlgoSampObjectWriteOutData(AlgoSampTemp,Output);
				/* handle any errors */
				if (Error != eFileLoadNoError)
					{
						return Error;
					}
			}

		return eFileLoadNoError;
	}


/* after a file has been saved, this is called to mark all objects as not modified. */
void								AlgoSampListMarkAllObjectsSaved(AlgoSampListRec* AlgoSampList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(AlgoSampList);
		Limit = ArrayGetLength(AlgoSampList->AlgoSampArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				AlgoSampObjectRec*	AlgoSampTemp;

				AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(
					AlgoSampList->AlgoSampArray,Scan);
				AlgoSampObjectMarkAsSaved(AlgoSampTemp);
			}
		AlgoSampList->AlgoSampListChanged = False;
	}


/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						AlgoSampListCopyObject(AlgoSampListRec* AlgoSampList)
	{
		ArrayRec*							ListOfSelections;
		MyBoolean							TotalSuccessFlag = False;

		CheckPtrExistence(AlgoSampList);
		ListOfSelections = GetListOfSelectedItems(AlgoSampList->List);
		if (ListOfSelections != NIL)
			{
				if (ArrayGetLength(ListOfSelections) >= 1)
					{
						AlgoSampObjectRec*	AlgoSampTemp;
						FileSpec*						TempFileLocation;

						AlgoSampTemp = (AlgoSampObjectRec*)ArrayGetElement(ListOfSelections,0);
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
														if (AlgoSampObjectWriteOutData(AlgoSampTemp,BufferedFile)
															== eFileLoadNoError)
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
															"AlgoSampListCopyObject:  scrap buffer");
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


/* try to paste the clipboard in as an algorithmic sample object.  returns False if */
/* it failed or the clipboard did not contain an algorithmic sample object. */
MyBoolean						AlgoSampListPasteObject(AlgoSampListRec* AlgoSampList)
	{
		MyBoolean					TotalSuccessFlag = False;
		char*							Scrap;

		CheckPtrExistence(AlgoSampList);
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
												TotalSuccessFlag = AlgoSampListPasteFromFile(
													AlgoSampList,FileDescriptor);
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


/* try to paste an algorithmic sample object from the file */
MyBoolean						AlgoSampListPasteFromFile(AlgoSampListRec* AlgoSampList,
											struct FileType* File)
	{
		MyBoolean					TotalSuccessFlag = False;

		CheckPtrExistence(AlgoSampList);
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
										AlgoSampObjectRec*		AlgoSampTemp EXECUTE(= (AlgoSampObjectRec*)0x81818181);

										if (eFileLoadNoError == AlgoSampObjectNewFromFile(&AlgoSampTemp,InputFile,
											AlgoSampList->CodeCenter,AlgoSampList->MainWindow,AlgoSampList))
											{
												CheckPtrExistence(AlgoSampTemp);
												/* add it to the scrolling object list */
												if (!InsertStringListElement(AlgoSampList->List,NIL,NIL,AlgoSampTemp,True))
													{
													 FailurePoint:
														DisposeAlgoSampObject(AlgoSampTemp);
													}
												 else
													{
														MainWindowDeselectAllOtherStringLists(AlgoSampList->MainWindow,AlgoSampList);
														SelectStringListElement(AlgoSampList->List,AlgoSampTemp);
														MakeStringListSelectionVisible(AlgoSampList->List);
														/* add it to the array */
														if (!ArrayAppendElement(AlgoSampList->AlgoSampArray,AlgoSampTemp))
															{
																RemoveStringListElement(AlgoSampList->List,AlgoSampTemp,True);
																goto FailurePoint;
															}
														 else
															{
																/* change the name in the list */
																AlgoSampListAlgoSampNameChanged(AlgoSampList,AlgoSampTemp);
																TotalSuccessFlag = True;
																AlgoSampList->AlgoSampListChanged = True;
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


/* find out how many algorithmic samples there are in this list */
long								AlgoSampListHowMany(AlgoSampListRec* AlgoSampList)
	{
		CheckPtrExistence(AlgoSampList);
		return ArrayGetLength(AlgoSampList->AlgoSampArray);
	}


/* get an indexed algorithmic sample from the list */
struct AlgoSampObjectRec*	AlgoSampListGetIndexedAlgoSamp(AlgoSampListRec* AlgoSampList,
											long Index)
	{
		AlgoSampObjectRec*	AlgoSamp;

		CheckPtrExistence(AlgoSampList);
		ERROR((Index < 0) || (Index >= AlgoSampListHowMany(AlgoSampList)),PRERR(ForceAbort,
			"AlgoSampListGetIndexedAlgoSamp:  index out of range"));
		AlgoSamp = (AlgoSampObjectRec*)ArrayGetElement(AlgoSampList->AlgoSampArray,Index);
		CheckPtrExistence(AlgoSamp);
		return AlgoSamp;
	}
