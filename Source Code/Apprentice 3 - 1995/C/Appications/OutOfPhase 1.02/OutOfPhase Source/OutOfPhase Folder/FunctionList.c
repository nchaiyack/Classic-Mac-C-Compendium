/* FunctionList.c */
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

#include "FunctionList.h"
#include "StringList.h"
#include "Memory.h"
#include "CodeCenter.h"
#include "Array.h"
#include "FunctionObject.h"
#include "Alert.h"
#include "DataMunging.h"
#include "MainWindowStuff.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"
#include "Files.h"
#include "Scrap.h"


struct FunctionListRec
	{
		StringListRec*					List;
		CodeCenterRec*					CodeCenter;
		struct MainWindowRec*		MainWindow;
		ArrayRec*								FunctionArray;
		MyBoolean								FunctionListChanged;
	};


#define MAGICSCRAPSTRING ("\xff\x00\x1f\xfe FunctionModuleScrap")


/* create a new function list */
FunctionListRec*		NewFunctionList(struct MainWindowRec* MainWindow,
											struct CodeCenterRec* CodeCenter, WinType* ScreenID,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		FunctionListRec*	FuncList;

		FuncList = (FunctionListRec*)AllocPtrCanFail(sizeof(FunctionListRec),
			"FunctionListRec");
		if (FuncList == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		FuncList->List = NewStringList(ScreenID,XLoc,YLoc,Width,Height,
			GetScreenFont(),9,StringListDontAllowMultipleSelection,"Function Modules");
		if (FuncList->List == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)FuncList);
				goto FailurePoint1;
			}
		FuncList->FunctionArray = NewArray();
		if (FuncList->FunctionArray == NIL)
			{
			 FailurePoint3:
				DisposeStringList(FuncList->List);
				goto FailurePoint2;
			}
		FuncList->MainWindow = MainWindow;
		FuncList->CodeCenter = CodeCenter;
		FuncList->FunctionListChanged = False;
		return FuncList;
	}


/* delete the function list and all of the function modules it contains */
void								DisposeFunctionList(FunctionListRec* FuncList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				DisposeFunctionObject(FunctionTemp);
			}
		DisposeArray(FuncList->FunctionArray);
		DisposeStringList(FuncList->List);
		ReleasePtr((char*)FuncList);
	}


/* change the location of the function list in the window */
void								SetFunctionListLocation(FunctionListRec* FuncList,
											OrdType XLoc, OrdType YLoc, OrdType Width, OrdType Height)
	{
		CheckPtrExistence(FuncList);
		SetStringListLoc(FuncList->List,XLoc,YLoc,Width,Height);
	}


/* redraw the list */
void								FunctionListRedraw(FunctionListRec* FuncList)
	{
		CheckPtrExistence(FuncList);
		RedrawStringList(FuncList->List);
	}


/* see if the specified coordinates falls inside the function list rectangle */
MyBoolean						FunctionListHitTest(FunctionListRec* FuncList,
											OrdType XLoc, OrdType YLoc)
	{
		CheckPtrExistence(FuncList);
		return StringListHitTest(FuncList->List,XLoc,YLoc);
	}


/* handle a mouse down event for the function list */
void								FunctionListDoMouseDown(FunctionListRec* FuncList, OrdType XLoc,
											OrdType YLoc, ModifierFlags Modifiers)
	{
		CheckPtrExistence(FuncList);
		if (StringListMouseDown(FuncList->List,XLoc,YLoc,Modifiers))
			{
				/* if it returns true, then it was a double click */
				FunctionListOpenSelection(FuncList);
			}
	}


/* called when the window becomes active */
void								FunctionListBecomeActive(FunctionListRec* FuncList)
	{
		CheckPtrExistence(FuncList);
		EnableStringList(FuncList->List);
	}


/* called when the window becomes inactive */
void								FunctionListBecomeInactive(FunctionListRec* FuncList)
	{
		CheckPtrExistence(FuncList);
		DisableStringList(FuncList->List);
	}


/* called when a selection is made in another list, so that this list */
/* is deselected */
void								FunctionListDeselect(FunctionListRec* FuncList)
	{
		CheckPtrExistence(FuncList);
		DeselectAllStringListElements(FuncList->List);
	}


/* check to see if there is a selection in this list */
MyBoolean						FunctionListIsThereSelection(FunctionListRec* FuncList)
	{
		CheckPtrExistence(FuncList);
		return (GetStringListHowManySelectedItems(FuncList->List) > 0);
	}


/* check to see if any of the function modules contained in this list need */
/* to be saved */
MyBoolean						DoesFunctionListNeedToBeSaved(FunctionListRec* FuncList)
	{
		long							Scan;
		long							Limit;
		MyBoolean					Flag;

		CheckPtrExistence(FuncList);
		Flag = FuncList->FunctionListChanged;
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; (Scan < Limit) && !Flag; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				if (HasFunctionObjectBeenModified(FunctionTemp))
					{
						Flag = True;
					}
			}
		return Flag;
	}


/* open an edit window for the selected function module */
void								FunctionListOpenSelection(FunctionListRec* FuncList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(FuncList);
		ListOfSelections = GetListOfSelectedItems(FuncList->List);
		if (ListOfSelections != NIL)
			{
				long							Scan;
				long							Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						FunctionObjectRec*		FunctionTemp;

						FunctionTemp = (FunctionObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						FunctionObjectOpenWindow(FunctionTemp);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* create a new function module and open a window for it */
void								FunctionListNewModule(FunctionListRec* FuncList)
	{
		FunctionObjectRec*	Function;

		CheckPtrExistence(FuncList);
		/* create the object */
		Function = NewFunctionObject(FuncList->CodeCenter,FuncList->MainWindow,FuncList);
		if (Function == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to create "
					"a new function module.",NIL);
				return;
			}
		/* add it to the string list */
		if (!InsertStringListElement(FuncList->List,NIL,NIL,Function,True))
			{
			 FailurePoint2:
				DisposeFunctionObject(Function);
				goto FailurePoint1;
			}
		MainWindowDeselectAllOtherStringLists(FuncList->MainWindow,FuncList);
		SelectStringListElement(FuncList->List,Function);
		MakeStringListSelectionVisible(FuncList->List);
		/* add it to the array */
		if (!ArrayAppendElement(FuncList->FunctionArray,Function))
			{
			 FailurePoint3:
				RemoveStringListElement(FuncList->List,Function,True);
				goto FailurePoint2;
			}
		/* update our internal flags */
		FuncList->FunctionListChanged = True;
		/* change the name in the list */
		FunctionListFunctionNameChanged(FuncList,Function);
		/* show the window */
		FunctionObjectOpenWindow(Function);
	}


/* delete the selected function module */
void								FunctionListDeleteSelection(FunctionListRec* FuncList)
	{
		ArrayRec*					ListOfSelections;

		CheckPtrExistence(FuncList);
		ListOfSelections = GetListOfSelectedItems(FuncList->List);
		if (ListOfSelections != NIL)
			{
				long								Scan;
				long								Limit;

				Limit = ArrayGetLength(ListOfSelections);
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						FunctionObjectRec*	OneToZap;

						OneToZap = (FunctionObjectRec*)ArrayGetElement(ListOfSelections,Scan);
						FunctionListDeleteFunction(FuncList,OneToZap);
					}
				DisposeArray(ListOfSelections);
			}
	}


/* delete the explicitly specified function module */
void								FunctionListDeleteFunction(FunctionListRec* FuncList,
											struct FunctionObjectRec* TheFunctionModule)
	{
		long								Scan;
		long								Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*		FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				if (TheFunctionModule == FunctionTemp)
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
														if (FunctionObjectWriteOutData(TheFunctionModule,Output)
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
								MainWindowNewDeleteUndoInfo(FuncList->MainWindow,BackupFileWhere,
									BackupFile);
								DisposeFunctionObject(FunctionTemp);
								RemoveStringListElement(FuncList->List,FunctionTemp,True);
								ArrayDeleteElement(FuncList->FunctionArray,Scan);
								FuncList->FunctionListChanged = True;
							}
						 else
							{
								YesNoCancelType		Decision;

								Decision = AskYesNoCancel("Unable to save undo information for object.  "
									"Delete object anyway?",NIL,"Delete","Cancel",NIL/*nothirdbutton*/);
								if (Decision == eYes)
									{
										DisposeFunctionObject(FunctionTemp);
										RemoveStringListElement(FuncList->List,FunctionTemp,True);
										ArrayDeleteElement(FuncList->FunctionArray,Scan);
										FuncList->FunctionListChanged = True;
									}
							}
						return;
					}
			}
		EXECUTE(PRERR(AllowResume,"FunctionListDeleteFunction:  couldn't find object"));
	}


/* the name of a function module has changed, so the name in the scrolling */
/* list must also be changed */
void								FunctionListFunctionNameChanged(FunctionListRec* FuncList,
											struct FunctionObjectRec* TheFunctionModule)
	{
		char*							FunctionName;

		CheckPtrExistence(FuncList);
		CheckPtrExistence(TheFunctionModule);
		ERROR(ArrayFindElement(FuncList->FunctionArray,TheFunctionModule) < 0,
			PRERR(ForceAbort,"FunctionListFunctionNameChanged:  unknown function"));
		FunctionName = FunctionObjectGetNameCopy(TheFunctionModule);
		if (FunctionName != NIL)
			{
				char*							FunctionNameNullTerminated;

				FunctionNameNullTerminated = BlockToStringCopy(FunctionName);
				if (FunctionNameNullTerminated != NIL)
					{
						ChangeStringListElementName(FuncList->List,
							FunctionNameNullTerminated,TheFunctionModule);
						ReleasePtr(FunctionNameNullTerminated);
					}
				ReleasePtr(FunctionName);
			}
	}


/* remove all object code for all function modules */
void								FunctionListUnbuildAll(FunctionListRec* FuncList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				FunctionObjectUnbuild(FunctionTemp);
			}
		ERROR(CodeCenterGetNumFunctions(FuncList->CodeCenter) != 0,PRERR(ForceAbort,
			"FunctionListUnbuildAll:  some objects still exist"));
	}


/* build all functions.  returns True if successful. */
MyBoolean						FunctionListMakeUpToDate(FunctionListRec* FuncList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				if (!FunctionObjectMakeUpToDate(FunctionTemp))
					{
						return False;
					}
			}
		return True;
	}


/* the document's name has changed, so update all windows */
void								FunctionListGlobalNameChange(FunctionListRec* FuncList,
											char* NewFilename)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				FunctionObjectGlobalNameChange(FunctionTemp,NewFilename);
			}
	}


/*   4-byte little endian function object count (positive 2s complement) */
/*   n-bytes of data for the function objects */


/* read function objects from a file.  returns True if completely successful. */
FileLoadingErrors		FunctionListReadData(FunctionListRec* FuncList,
											struct BufferedInputRec* Input)
	{
		long							FunctionObjectCount;
		long							Scan;

		CheckPtrExistence(FuncList);
		CheckPtrExistence(Input);

		/*   4-byte little endian function object count */
		if (!ReadBufferedSignedLongLittleEndian(Input,&FunctionObjectCount))
			{
				return eFileLoadDiskError;
			}
		if (FunctionObjectCount < 0)
			{
				return eFileLoadBadFormat;
			}

		/* read in all of the objects */
		for (Scan = 0; Scan < FunctionObjectCount; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp EXECUTE(= (FunctionObjectRec*)0x81818181);
				FileLoadingErrors		Error;

				/* create the new function object */
				Error = FunctionObjectNewFromFile(&FunctionTemp,Input,FuncList->CodeCenter,
					FuncList->MainWindow,FuncList);
				if (Error != eFileLoadNoError)
					{
					 FailurePoint1:
						return Error;
					}
				CheckPtrExistence(FunctionTemp);
				/* add it to the scrolling object list */
				if (!InsertStringListElement(FuncList->List,NIL,NIL,FunctionTemp,True))
					{
					 FailurePoint2:
						DisposeFunctionObject(FunctionTemp);
						Error = eFileLoadOutOfMemory;
						goto FailurePoint1;
					}
				/* add it to the array */
				if (!ArrayAppendElement(FuncList->FunctionArray,FunctionTemp))
					{
					 FailurePoint3:
						RemoveStringListElement(FuncList->List,FunctionTemp,True);
						goto FailurePoint2;
					}
				/* change the name in the list */
				FunctionListFunctionNameChanged(FuncList,FunctionTemp);
			}

		return eFileLoadNoError;
	}


/* write function objects to a file.  returns True if completely successful. */
FileLoadingErrors		FunctionListWriteData(FunctionListRec* FuncList,
											struct BufferedOutputRec* Output)
	{
		long							NumObjects;
		long							Scan;

		CheckPtrExistence(FuncList);
		CheckPtrExistence(Output);

		/*   4-byte little endian function object count */
		NumObjects = ArrayGetLength(FuncList->FunctionArray);
		if (!WriteBufferedSignedLongLittleEndian(Output,NumObjects))
			{
				return eFileLoadDiskError;
			}

		/* write those little buggers out */
		for (Scan = 0; Scan < NumObjects; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;
				FileLoadingErrors		Error;

				/* get the thing */
				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				/* write it */
				Error = FunctionObjectWriteOutData(FunctionTemp,Output);
				/* handle any bad things */
				if (Error != eFileLoadNoError)
					{
						return Error;
					}
			}

		return eFileLoadNoError;
	}


/* after a file has been saved, this is called to mark all objects as not modified. */
void								FunctionListMarkAllObjectsSaved(FunctionListRec* FuncList)
	{
		long							Scan;
		long							Limit;

		CheckPtrExistence(FuncList);
		Limit = ArrayGetLength(FuncList->FunctionArray);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FunctionObjectRec*	FunctionTemp;

				FunctionTemp = (FunctionObjectRec*)ArrayGetElement(FuncList->FunctionArray,Scan);
				FunctionObjectMarkAsSaved(FunctionTemp);
			}
		FuncList->FunctionListChanged = False;
	}


/* copy the selected object in the list to the clipboard.  return False if failed. */
MyBoolean						FunctionListCopyObject(FunctionListRec* FuncList)
	{
		ArrayRec*							ListOfSelections;
		MyBoolean							TotalSuccessFlag = False;

		CheckPtrExistence(FuncList);
		ListOfSelections = GetListOfSelectedItems(FuncList->List);
		if (ListOfSelections != NIL)
			{
				if (ArrayGetLength(ListOfSelections) >= 1)
					{
						FunctionObjectRec*	FunctionTemp;
						FileSpec*						TempFileLocation;

						FunctionTemp = (FunctionObjectRec*)ArrayGetElement(ListOfSelections,0);
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
														if (FunctionObjectWriteOutData(FunctionTemp,BufferedFile)
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
															"FunctionListCopyObject:  scrap buffer");
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


/* try to paste the clipboard in as a function object.  returns False if it failed */
/* or the clipboard did not contain a function object. */
MyBoolean						FunctionListPasteObject(FunctionListRec* FuncList)
	{
		MyBoolean					TotalSuccessFlag = False;
		char*							Scrap;

		CheckPtrExistence(FuncList);
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
												TotalSuccessFlag = FunctionListPasteFromFile(FuncList,
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


/* try to paste a function object in from a file */
MyBoolean						FunctionListPasteFromFile(FunctionListRec* FuncList,
											struct FileType* File)
	{
		MyBoolean					TotalSuccessFlag = False;

		CheckPtrExistence(FuncList);
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
										FunctionObjectRec*		FunctionTemp EXECUTE(= (FunctionObjectRec*)0x81818181);

										if (eFileLoadNoError == FunctionObjectNewFromFile(&FunctionTemp,
											InputFile,FuncList->CodeCenter,FuncList->MainWindow,FuncList))
											{
												CheckPtrExistence(FunctionTemp);
												/* add it to the scrolling object list */
												if (!InsertStringListElement(FuncList->List,NIL,NIL,FunctionTemp,True))
													{
													 FailurePoint:
														DisposeFunctionObject(FunctionTemp);
													}
												 else
													{
														MainWindowDeselectAllOtherStringLists(FuncList->MainWindow,FuncList);
														SelectStringListElement(FuncList->List,FunctionTemp);
														MakeStringListSelectionVisible(FuncList->List);
														/* add it to the array */
														if (!ArrayAppendElement(FuncList->FunctionArray,FunctionTemp))
															{
																RemoveStringListElement(FuncList->List,FunctionTemp,True);
																goto FailurePoint;
															}
														 else
															{
																/* change the name in the list */
																FunctionListFunctionNameChanged(FuncList,FunctionTemp);
																TotalSuccessFlag = True;
																FuncList->FunctionListChanged = True;
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
