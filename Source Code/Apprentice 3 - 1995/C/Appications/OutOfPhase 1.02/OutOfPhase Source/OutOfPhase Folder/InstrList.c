/* InstrList.c */
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

#include "InstrList.h"
#include "StringList.h"
#include "Array.h"
#include "Memory.h"
#include "Alert.h"
#include "DataMunging.h"
#include "InstrObject.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"
#include "Files.h"
#include "Scrap.h"


struct InstrListRec
	{
		StringListRec*					List;
		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		ArrayRec*								InstrArray;
		MyBoolean								InstrListChanged;
	};


#define MAGICSCRAPSTRING ("\xff\x00\x1f\xfe InstrumentObjectScrap")


/* create a new instrument list */
InstrListRec*				NewInstrList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		InstrListRec*			InstrList;

		InstrList = (InstrListRec*)AllocPtrCanFail(sizeof(InstrListRec),"InstrListRec");
		if (InstrList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		InstrList->InstrArray = NewArray();
		if (InstrList->InstrArray == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)InstrList);
				goto FailurePoint1;
			}
		InstrList->List = NewStringList(ScreenID,XLoc,YLoc,Width,Height,
			GetScreenFont(),9,StringListDontAllowMultipleSelection,"Instruments");
		if (InstrList->List == NIL)
			{
			 FailurePoint3:
				DisposeArray(InstrList->InstrArray);
				goto FailurePoint2;
			}
		InstrList->CodeCenter = CodeCenter;
		InstrList->MainWindow = MainWindow;
		InstrList->InstrListChanged = False;
		return InstrList;
	}


/* delete the instrument list and all of the instruments it contains */
void								DisposeInstrList(InstrListRec* InstrList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*		InstrTemp;

				InstrTemp = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				DisposeInstrObject(InstrTemp);
			}
		DisposeArray(InstrList->InstrArray);
		DisposeStringList(InstrList->List);
		ReleasePtr((char*)InstrList);
	}


/* change the location of the instrument list in the window */
void								SetInstrListLocation(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(InstrList);
		SetStringListLoc(InstrList->List,XLoc,YLoc,Width,Height);
	}


/* redraw the list */
void								InstrListRedraw(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		RedrawStringList(InstrList->List);
	}


/* see if the specified coordinates falls inside the instrument list rectangle */
MyBoolean						InstrListHitTest(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc)
	{
		CheckPtrExistence(InstrList);
		return StringListHitTest(InstrList->List,XLoc,YLoc);
	}


/* handle a mouse down event for the instrument list */
void								InstrListDoMouseDown(InstrListRec* InstrList,
											OrdType XLoc, OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(InstrList);
		if (StringListMouseDown(InstrList->List,XLoc,YLoc,Modifiers))
			{
				/* if it returns true, then it was a double click */
				InstrListOpenSelection(InstrList);
			}
	}


/* called when the window becomes active */
void								InstrListBecomeActive(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		EnableStringList(InstrList->List);
	}


/* called when the window becomes inactive */
void								InstrListBecomeInactive(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		DisableStringList(InstrList->List);
	}


/* called when a selection is made in another list, so that this list */
/* is deselected */
void								InstrListDeselect(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		DeselectAllStringListElements(InstrList->List);
	}


/* check to see if there is a selection in this list */
MyBoolean						InstrListIsThereSelection(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		return (GetStringListHowManySelectedItems(InstrList->List) > 0);
	}


/* check to see if any of the instruments contained in this list need to be saved */
MyBoolean						DoesInstrListNeedToBeSaved(InstrListRec* InstrList)
	{
		long							Scan;
		long							Limit;
		MyBoolean					Flag;

		CheckPtrExistence(InstrList);
		Flag = InstrList->InstrListChanged;
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; (Scan < Limit) && !Flag; Scan += 1)
			{
				InstrObjectRec*		InstrTemp;

				InstrTemp = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				if (HasInstrObjectBeenModified(InstrTemp))
					{
						Flag = True;
					}
			}
		return Flag;
	}


/* open an edit window for the selected instrument */
void								InstrListOpenSelection(InstrListRec* InstrList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(InstrList);
		ListOfSelections = GetListOfSelectedItems(InstrList->List);
		if (ListOfSelections != NIL)
			{
				long							Scan;
				long							Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						InstrObjectRec*		InstrTemp;

						InstrTemp = (InstrObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						InstrObjectOpenWindow(InstrTemp);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* create a new instrument and open a window for it */
void								InstrListNewInstr(InstrListRec* InstrList)
	{
		InstrObjectRec*		Instr;

		CheckPtrExistence(InstrList);
		/* create the object */
		Instr = NewInstrObject(InstrList->CodeCenter,InstrList->MainWindow,InstrList);
		if (Instr == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to create a new instrument.",NIL);
				return;
			}
		/* add it to the string list */
		if (!InsertStringListElement(InstrList->List,NIL,NIL,Instr,True))
			{
			 FailurePoint2:
				DisposeInstrObject(Instr);
				goto FailurePoint1;
			}
		MainWindowDeselectAllOtherStringLists(InstrList->MainWindow,InstrList);
		SelectStringListElement(InstrList->List,Instr);
		MakeStringListSelectionVisible(InstrList->List);
		/* add it to the array */
		if (!ArrayAppendElement(InstrList->InstrArray,Instr))
			{
			 FailurePoint3:
				RemoveStringListElement(InstrList->List,Instr,True);
				goto FailurePoint2;
			}
		/* update our internal flags */
		InstrList->InstrListChanged = True;
		/* change the name in the list */
		InstrListInstrNameChanged(InstrList,Instr);
		/* show the window */
		InstrObjectOpenWindow(Instr);
	}


/* delete the selected instrument */
void								InstrListDeleteSelection(InstrListRec* InstrList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(InstrList);
		ListOfSelections = GetListOfSelectedItems(InstrList->List);
		if (ListOfSelections != NIL)
			{
				long								Scan;
				long								Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						InstrObjectRec*			OneToZap;

						OneToZap = (InstrObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						InstrListDeleteInstr(InstrList,OneToZap);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* delete the explicitly specified instrument */
void								InstrListDeleteInstr(InstrListRec* InstrList,
											struct InstrObjectRec* TheInstr)
	{
		long								Scan;
		long								Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			InstrTemp;

				InstrTemp = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				if (TheInstr == InstrTemp)
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
														if (InstrObjectWriteDataOut(TheInstr,Output)
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
								MainWindowNewDeleteUndoInfo(InstrList->MainWindow,BackupFileWhere,
									BackupFile);
								DisposeInstrObject(InstrTemp);
								RemoveStringListElement(InstrList->List,InstrTemp,True);
								ArrayDeleteElement(InstrList->InstrArray,Scan);
								InstrList->InstrListChanged = True;
							}
						 else
							{
								YesNoCancelType		Decision;

								Decision = AskYesNoCancel("Unable to save undo information for object.  "
									"Delete object anyway?",NIL,"Delete","Cancel",NIL/*nothirdbutton*/);
								if (Decision == eYes)
									{
										DisposeInstrObject(InstrTemp);
										RemoveStringListElement(InstrList->List,InstrTemp,True);
										ArrayDeleteElement(InstrList->InstrArray,Scan);
										InstrList->InstrListChanged = True;
									}
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"InstrListDeleteInstr:  couldn't find object"));
	}


/* the name of an instrument has changed, so the name in the scrolling */
/* list must also be changed */
void								InstrListInstrNameChanged(InstrListRec* InstrList,
											struct InstrObjectRec* TheInstr)
	{
		char*							InstrName;

		CheckPtrExistence(InstrList);
		CheckPtrExistence(TheInstr);
		ERROR(ArrayFindElement(InstrList->InstrArray,TheInstr) < 0,
			PRERR(ForceAbort,"InstrListInstrNameChanged:  unknown instrument"));
		InstrName = InstrObjectGetNameCopy(TheInstr);
		if (InstrName != NIL)
			{
				char*							InstrNameNullTerminated;

				InstrNameNullTerminated = BlockToStringCopy(InstrName);
				if (InstrNameNullTerminated != NIL)
					{
						ChangeStringListElementName(InstrList->List,
							InstrNameNullTerminated,TheInstr);
						ReleasePtr(InstrNameNullTerminated);
					}
				ReleasePtr(InstrName);
			}
	}


/* look for a specified instrument.  returns NIL if not found.  the name is */
/* NOT null terminated */
struct InstrObjectRec*	InstrListLookupNamedInstr(InstrListRec* InstrList, char* Name)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		CheckPtrExistence(Name);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			Instr;
				char*								NameCopy;

				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				NameCopy = InstrObjectGetNameCopy(Instr);
				if (NameCopy != NIL)
					{
						if (PtrSize(Name) == PtrSize(NameCopy))
							{
								if (MemEqu(Name,NameCopy,PtrSize(Name)))
									{
										ReleasePtr(NameCopy);
										return Instr;
									}
							}
						ReleasePtr(NameCopy);
					}
			}
		return NIL;
	}


/* document's name has changed, so we need to update the windows */
void								InstrListGlobalNameChange(InstrListRec* InstrList,
											char* NewFilename)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			Instr;

				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				InstrObjectGlobalNameChange(Instr,NewFilename);
			}
	}


/* compile all of the instruments definitions */
MyBoolean						InstrListMakeUpToDate(InstrListRec* InstrList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			Instr;

				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				if (!MakeInstrObjectUpToDate(Instr))
					{
						return False;
					}
			}
		return True;
	}


/* dispose of all of the objects from the instrument definitions */
void								InstrListUnbuildAll(InstrListRec* InstrList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			Instr;

				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				UnbuildInstrObject(Instr);
			}
	}


/*   4-byte little endian number of instrument objects (positive 2s complement) */
/*   n-byte data for the instrument objects */


/* read instrument definitions from a file.  returns True if completely successful. */
FileLoadingErrors		InstrListReadData(InstrListRec* InstrList,
											struct BufferedInputRec* Input)
	{
		signed long				NumberOfObjects;
		long							Scan;

		CheckPtrExistence(InstrList);
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

		/*   n-byte data for the objects */
		for (Scan = 0; Scan < NumberOfObjects; Scan += 1)
			{
				InstrObjectRec*		Instr EXECUTE(= (InstrObjectRec*)0x81818181);
				FileLoadingErrors	Error;

				/* load the object */
				Error = InstrObjectNewFromFile(&Instr,Input,InstrList->CodeCenter,
					InstrList->MainWindow,InstrList);
				if (Error != eFileLoadNoError)
					{
					 FailurePoint1:
						return Error;
					}
				CheckPtrExistence(Instr);
				/* add it to the string list */
				if (!InsertStringListElement(InstrList->List,NIL,NIL,Instr,True))
					{
					 FailurePoint2:
						DisposeInstrObject(Instr);
						Error = eFileLoadOutOfMemory;
						goto FailurePoint1;
					}
				/* add it to the array */
				if (!ArrayAppendElement(InstrList->InstrArray,Instr))
					{
					 FailurePoint3:
						RemoveStringListElement(InstrList->List,Instr,True);
						goto FailurePoint2;
					}
				/* change the name in the list */
				InstrListInstrNameChanged(InstrList,Instr);
			}

		return eFileLoadNoError;
	}


/* write instrument definitions to a file.  returns True if completely successful. */
FileLoadingErrors		InstrListWriteData(InstrListRec* InstrList,
											struct BufferedOutputRec* Output)
	{
		long							NumberOfObjects;
		long							Scan;

		CheckPtrExistence(InstrList);
		CheckPtrExistence(Output);

		/*   4-byte little endian number of objects (positive 2s complement) */
		NumberOfObjects = ArrayGetLength(InstrList->InstrArray);
		if (!WriteBufferedSignedLongLittleEndian(Output,NumberOfObjects))
			{
				return eFileLoadDiskError;
			}

		/*   n-byte data for the objects */
		for (Scan = 0; Scan < NumberOfObjects; Scan += 1)
			{
				InstrObjectRec*			Instr;
				FileLoadingErrors		Error;

				/* get the object */
				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				/* write the object out */
				Error = InstrObjectWriteDataOut(Instr,Output);
				/* handle errors */
				if (Error != eFileLoadNoError)
					{
						return Error;
					}
			}

		return eFileLoadNoError;
	}


/* after a file has been saved, this is called to mark all objects as not modified. */
void								InstrListMarkAllObjectsSaved(InstrListRec* InstrList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(InstrList);
		Limit = ArrayGetLength(InstrList->InstrArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				InstrObjectRec*			Instr;

				Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Scan);
				InstrObjectMarkAsSaved(Instr);
			}
		InstrList->InstrListChanged = False;
	}


/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						InstrListCopyObject(InstrListRec* InstrList)
	{
		ArrayRec*							ListOfSelections;
		MyBoolean							TotalSuccessFlag = False;

		CheckPtrExistence(InstrList);
		ListOfSelections = GetListOfSelectedItems(InstrList->List);
		if (ListOfSelections != NIL)
			{
				if (ArrayGetLength(ListOfSelections) >= 1)
					{
						InstrObjectRec*			InstrTemp;
						FileSpec*						TempFileLocation;

						InstrTemp = (InstrObjectRec*)ArrayGetElement(ListOfSelections,0);
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
														if (InstrObjectWriteDataOut(InstrTemp,BufferedFile)
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
															"InstrListCopyObject:  scrap buffer");
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


/* try to paste the clipboard in as an instrument.  returns False if it failed or the */
/* clipboard did not contain an instrument. */
MyBoolean						InstrListPasteObject(InstrListRec* InstrList)
	{
		MyBoolean					TotalSuccessFlag = False;
		char*							Scrap;

		CheckPtrExistence(InstrList);
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
												TotalSuccessFlag = InstrListPasteFromFile(InstrList,
													FileDescriptor);
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


/* try to paste the instrument object in from the file */
MyBoolean						InstrListPasteFromFile(InstrListRec* InstrList,
											struct FileType* File)
	{
		MyBoolean					TotalSuccessFlag = False;

		CheckPtrExistence(InstrList);
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
										InstrObjectRec*				InstrTemp EXECUTE(= (InstrObjectRec*)0x81818181);

										if (eFileLoadNoError == InstrObjectNewFromFile(&InstrTemp,InputFile,
											InstrList->CodeCenter,InstrList->MainWindow,InstrList))
											{
												CheckPtrExistence(InstrTemp);
												/* add it to the scrolling object list */
												if (!InsertStringListElement(InstrList->List,NIL,NIL,InstrTemp,True))
													{
													 FailurePoint:
														DisposeInstrObject(InstrTemp);
													}
												 else
													{
														MainWindowDeselectAllOtherStringLists(InstrList->MainWindow,InstrList);
														SelectStringListElement(InstrList->List,InstrTemp);
														MakeStringListSelectionVisible(InstrList->List);
														/* add it to the array */
														if (!ArrayAppendElement(InstrList->InstrArray,InstrTemp))
															{
																RemoveStringListElement(InstrList->List,InstrTemp,True);
																goto FailurePoint;
															}
														 else
															{
																/* change the name in the list */
																InstrListInstrNameChanged(InstrList,InstrTemp);
																TotalSuccessFlag = True;
																InstrList->InstrListChanged = True;
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


/* find out how many instruments there are in this list */
long								InstrListHowMany(InstrListRec* InstrList)
	{
		CheckPtrExistence(InstrList);
		return ArrayGetLength(InstrList->InstrArray);
	}


/* get an indexed instruments from the list */
struct InstrObjectRec*	InstrListGetIndexedInstr(InstrListRec* InstrList, long Index)
	{
		InstrObjectRec*	Instr;

		CheckPtrExistence(InstrList);
		ERROR((Index < 0) || (Index >= InstrListHowMany(InstrList)),
			PRERR(ForceAbort,"InstrListGetIndexedInstr:  index out of range"));
		Instr = (InstrObjectRec*)ArrayGetElement(InstrList->InstrArray,Index);
		CheckPtrExistence(Instr);
		return Instr;
	}
