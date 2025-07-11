/* AlgoWaveTableObject.c */
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

#include "AlgoWaveTableObject.h"
#include "AlgoWaveTableList.h"
#include "Memory.h"
#include "DataMunging.h"
#include "CodeCenter.h"
#include "PcodeStack.h"
#include "PcodeSystem.h"
#include "FixedPoint.h"
#include "MainWindowStuff.h"
#include "Alert.h"
#include "Numbers.h"
#include "AlgoWaveTableWindow.h"
#include "WaveTableStorage.h"
#include "CompilerRoot.h"
#include "FunctionCode.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


struct AlgoWaveTableObjectRec
	{
		MyBoolean								DataModified;

		char*										Name;
		char*										AlgoWaveTableFormula;

		MyBoolean								NeedsToBeRebuilt;
		WaveTableStorageRec*		WaveTableData; /* NIL if not built */
		NumBitsType							NumBits;
		long										NumFrames;
		long										NumTables;

		AlgoWaveTableWindowRec*	AlgoWaveTableWindow;

		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		AlgoWaveTableListRec*		AlgoWaveTableList;

		short										SavedWindowXLoc;
		short										SavedWindowYLoc;
		short										SavedWindowWidth;
		short										SavedWindowHeight;
	};


/* allocate and create a new algorithmic wave table */
AlgoWaveTableObjectRec*	NewAlgoWaveTableObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		AlgoWaveTableObjectRec*	AlgoWaveTableObj;

		AlgoWaveTableObj = (AlgoWaveTableObjectRec*)AllocPtrCanFail(
			sizeof(AlgoWaveTableObjectRec),"AlgoWaveTableObjectRec");
		if (AlgoWaveTableObj == NIL)
			{
			 FailurePoint1:
				return AlgoWaveTableObj;
			}
		AlgoWaveTableObj->Name = StringToBlockCopy("untitled");
		if (AlgoWaveTableObj->Name == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)AlgoWaveTableObj);
				goto FailurePoint1;
			}
		AlgoWaveTableObj->AlgoWaveTableFormula = StringToBlockCopy(
			"# frames : integer; tables : integer; data : fixedarray\x0a");
		if (AlgoWaveTableObj->AlgoWaveTableFormula == NIL)
			{
			 FailurePoint3:
				ReleasePtr(AlgoWaveTableObj->Name);
				goto FailurePoint2;
			}
		AlgoWaveTableObj->DataModified = False;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
		AlgoWaveTableObj->WaveTableData = NIL;
		AlgoWaveTableObj->NumBits = eSample16bit;
		AlgoWaveTableObj->NumFrames = 256;
		AlgoWaveTableObj->NumTables = 256;
		AlgoWaveTableObj->AlgoWaveTableWindow = NIL;
		AlgoWaveTableObj->CodeCenter = CodeCenter;
		AlgoWaveTableObj->MainWindow = MainWindow;
		AlgoWaveTableObj->AlgoWaveTableList = AlgoWaveTableList;
		AlgoWaveTableObj->SavedWindowXLoc = 0;
		AlgoWaveTableObj->SavedWindowYLoc = 0;
		AlgoWaveTableObj->SavedWindowWidth = 0;
		AlgoWaveTableObj->SavedWindowHeight = 0;
		return AlgoWaveTableObj;
	}


/* dispose of all data structures associated with wave table */
void									DisposeAlgoWaveTableObject(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				DisposeAlgoWaveTableWindow(AlgoWaveTableObj->AlgoWaveTableWindow);
				ERROR(AlgoWaveTableObj->AlgoWaveTableWindow != NIL,PRERR(ForceAbort,
					"DisposeAlgoWaveTableObject: window thing not NIL after disposing"));
			}
		ReleasePtr(AlgoWaveTableObj->Name);
		ReleasePtr(AlgoWaveTableObj->AlgoWaveTableFormula);
		if (AlgoWaveTableObj->WaveTableData != NIL)
			{
				DisposeWaveTableStorage(AlgoWaveTableObj->WaveTableData);
			}
		ReleasePtr((char*)AlgoWaveTableObj);
	}


/* find out if any changes have been made to it */
MyBoolean							HasAlgoWaveTableObjectBeenModified(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				return AlgoWaveTableObj->DataModified
					|| HasAlgoWaveTableWindowBeenModified(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				return AlgoWaveTableObj->DataModified;
			}
	}


static FunctionParamRec		ArgList[] =
	{
		{"frames",eInteger},
		{"tables",eInteger},
		{"data",eArrayOfFixed}
	};
#define ARGLISTLENGTH (sizeof(ArgList) / sizeof(ArgList[0]))


/* rebuild the wave table and return True if successful */
MyBoolean							AlgoWaveTableObjectBuild(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		char*								Blob;
		PcodeRec*						FuncCode;
		CompileErrors				Error;
		long								LineNumber;
		ParamStackRec*			ParamList;
		EvalErrors					OtherError;
		OpcodeRec*					ErrorOpcode;
		long								OffendingInstruction;
		DataTypes						ReturnType;
		char*								Buffer;
		largefixedsigned*		Middle;
		long								Limit;
		long								Scan;
		long								NumberOfTablesInIt;
		long								TotalFrameCount;


		CheckPtrExistence(AlgoWaveTableObj);

		/* make sure we are unbuilt */
		AlgoWaveTableObjectUnbuild(AlgoWaveTableObj);
		ERROR(AlgoWaveTableObj->WaveTableData != NIL,PRERR(ForceAbort,
			"AlgoWaveTableObjectBuild:  unbuilt object but it still exists"));

		/* bring the world up to date */
		if (!MainWindowMakeUpToDateFunctions(AlgoWaveTableObj->MainWindow))
			{
				return False;
			}

		/* allocate the new wave table thing */
		AlgoWaveTableObj->WaveTableData = NewWaveTableStorage(
			AlgoWaveTableObjectGetNumBits(AlgoWaveTableObj),
			AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObj));
		if (AlgoWaveTableObj->WaveTableData == NIL)
			{
			 FailurePointNegative1:
				AlertHalt("There is not enough memory available to compile the algorithmic "
					"wave table generator function.",NIL);
			}
		for (Scan = 0; Scan < AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj); Scan += 1)
			{
				if (!WaveTableStorageAppendEntry(AlgoWaveTableObj->WaveTableData))
					{
					 FailurePoint0:
						DisposeWaveTableStorage(AlgoWaveTableObj->WaveTableData);
						goto FailurePointNegative1;
					}
			}

		/* prepare the text blob to be evaluated */
		Blob = AlgoWaveTableObjectGetFormulaCopy(AlgoWaveTableObj);
		if (Blob == NIL)
			{
			 FailurePoint1:
				goto FailurePoint0;
			}

		/* perform compilation */
		Error = CompileSpecialFunction(ArgList,ARGLISTLENGTH,&LineNumber,
			&ReturnType,Blob,&FuncCode);
		ReleasePtr(Blob);
		/* show the error message if there is one */
		if (Error != eCompileNoError)
			{
				if (!AlgoWaveTableObjectOpenWindow(AlgoWaveTableObj))
					{
						AlertHalt("A compile error occurred but there is not enough "
							"memory available to display the error message.",NIL);
						return False;
					}
				AlgoWaveTableWindowHiliteLine(AlgoWaveTableObj->AlgoWaveTableWindow,LineNumber - 1);
				AlertHalt("A compile error occurred:  _",GetCompileErrorString(Error));
				return False;
			}

		/* try to evaluate the code */
		ParamList = NewParamStack();
		if (ParamList == NIL)
			{
			 SecondFailurePoint1:
				DisposePcode(FuncCode);
				AlertHalt("There is not enough memory available to evaluate the algorithmic "
					"wave table generator function.",NIL);
				return False;
			}
		/* add a space for the return value */
		if (!AddIntegerToStack(ParamList,0))
			{
			 SecondFailurePoint2:
				DisposeParamStack(ParamList);
				goto SecondFailurePoint1;
			}
		/* add the special parameters (SymbolStack order MUST be the same */
		/* order as that used for the Parameterlist) */
		if (!AddIntegerToStack(ParamList,AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObj)))
			{
				goto SecondFailurePoint2;
			}
		if (!AddIntegerToStack(ParamList,AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj)))
			{
				goto SecondFailurePoint2;
			}
		Buffer = AllocPtrCanFail(AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObj)
			* AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj)
			* sizeof(largefixedsigned),"degeneratesample");
		if (Buffer == NIL)
			{
				goto SecondFailurePoint2;
			}
		Limit = PtrSize(Buffer);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				Buffer[Scan] = 0;
			}
		if (!AddArrayToStack(ParamList,Buffer))
			{
				ReleasePtr(Buffer);
				goto SecondFailurePoint2;
			}

		/* executing the actual code */
		OtherError = EvaluatePcode(ParamList,FuncCode,
			AlgoWaveTableObj->CodeCenter,&ErrorOpcode,&OffendingInstruction,
			AlgoWaveTableObj->MainWindow,
			&MainWindowGetSampleLeftCopy,&MainWindowGetSampleRightCopy,
			&MainWindowGetSampleMonoCopy,&MainWindowGetWaveTableFrameCount,
			&MainWindowGetWaveTableTableCount,&MainWindowGetWaveTableArray);
		DisposePcode(FuncCode);
		if (OtherError != eEvalNoError)
			{
				char*					FuncNameString;
				FuncCodeRec*	ErrorFunction;
				MyBoolean			SuccessFlag;

				/* present error message */
				AlgoWaveTableObjectOpenWindow(AlgoWaveTableObj);
				SuccessFlag = False;
				ErrorFunction = GetFunctionFromOpcode(AlgoWaveTableObj->CodeCenter,ErrorOpcode);
				if (ErrorFunction == NIL)
					{
						FuncNameString = StringToBlockCopy("<anonymous>");
					}
				 else
					{
						FuncNameString = CopyPtr(GetFunctionName(ErrorFunction));
					}
				if (FuncNameString != NIL)
					{
						char*					Key;

						Key = StringToBlockCopy("_");
						if (Key != NIL)
							{
								char*					BaseMessage;

								BaseMessage = StringFromRaw("Error in function _, instruction _:  _");
								if (BaseMessage != NIL)
									{
										char*					FixedMessage1;

										FixedMessage1 = ReplaceBlockCopy(BaseMessage,Key,FuncNameString);
										if (FixedMessage1 != NIL)
											{
												char*					NumberStr;

												NumberStr = IntegerToString(OffendingInstruction);
												if (NumberStr != NIL)
													{
														char*					FixedMessage2;

														FixedMessage2 = ReplaceBlockCopy(FixedMessage1,Key,NumberStr);
														if (FixedMessage2 != NIL)
															{
																AlertHalt(FixedMessage2,GetPcodeErrorMessage(OtherError));
																SuccessFlag = True;
																ReleasePtr(FixedMessage2);
															}
														ReleasePtr(NumberStr);
													}
												ReleasePtr(FixedMessage1);
											}
										ReleasePtr(BaseMessage);
									}
								ReleasePtr(Key);
							}
						ReleasePtr(FuncNameString);
					}
				if (!SuccessFlag)
					{
						AlertHalt("There is not enough memory available to show the "
							"compile error message.",NIL);
					}
				DisposeParamStack(ParamList);
				return False;
			}

		/* add the new data */
		Middle = (largefixedsigned*)GetStackArray(ParamList,3);
		if (Middle == NIL)
			{
				AlertHalt("NIL array returned from algorithmic wave table function.",NIL);
			 RebuildFailurePoint1:
				DisposeParamStack(ParamList);
				DisposeWaveTableStorage(AlgoWaveTableObj->WaveTableData);
				AlgoWaveTableObj->WaveTableData = NIL;
				AlgoWaveTableObjectOpenWindow(AlgoWaveTableObj);
				return False;
			}
		ERROR(PtrSize((char*)Middle) % sizeof(largefixedsigned) != 0,
			PRERR(ForceAbort,"AlgoWaveTableObjectBuild:  array alignment error"));
		NumberOfTablesInIt = AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj);
		TotalFrameCount = AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObj);
		if ((PtrSize((char*)Middle) / sizeof(largefixedsigned))
			!= NumberOfTablesInIt * TotalFrameCount)
			{
				AlertHalt("Algorithmic wave table function returned array of wrong size.",NIL);
			 RebuildFailurePoint2:
				goto RebuildFailurePoint1;
			}
		for (Scan = 0; Scan < NumberOfTablesInIt; Scan += 1)
			{
				long								Index;

				for (Index = 0; Index < TotalFrameCount; Index += 1)
					{
						WaveTableStorageSetFrame(AlgoWaveTableObj->WaveTableData,Scan,Index,
							Middle[(Scan * TotalFrameCount) + Index]);
					}
			}
		DisposeParamStack(ParamList);
		AlgoWaveTableObj->NeedsToBeRebuilt = False;
		return True;
	}


/* remove the wave table data */
void									AlgoWaveTableObjectUnbuild(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->WaveTableData != NIL)
			{
				DisposeWaveTableStorage(AlgoWaveTableObj->WaveTableData);
				AlgoWaveTableObj->WaveTableData = NIL;
			}
	}


/* rebuild the wave table if it hasn't been built and return success flag */
MyBoolean							AlgoWaveTableObjectMakeUpToDate(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if ((AlgoWaveTableObj->WaveTableData == NIL) || (AlgoWaveTableObj->NeedsToBeRebuilt)
			|| (AlgoWaveTableObj->AlgoWaveTableWindow != NIL))
			{
				return AlgoWaveTableObjectBuild(AlgoWaveTableObj);
			}
		 else
			{
				return True;
			}
	}


/* get a copy of the name of this object */
char*									AlgoWaveTableObjectGetNameCopy(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		char*								NameTemp;

		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				NameTemp = AlgoWaveTableWindowGetNameCopy(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				NameTemp = CopyPtr(AlgoWaveTableObj->Name);
			}
		if (NameTemp != NIL)
			{
				SetTag(NameTemp,"AlgoWaveTableNameCopy");
			}
		return NameTemp;
	}


/* install a new name on the object.  the object becomes owner of the name, so */
/* the caller should not dispose of it. */
void									AlgoWaveTableObjectNewName(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												char* Name)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		CheckPtrExistence(Name);
		ReleasePtr(AlgoWaveTableObj->Name);
		SetTag(Name,"AlgoWaveTableName");
		AlgoWaveTableObj->Name = Name;
		AlgoWaveTableObj->DataModified = True;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
		AlgoWaveTableListAlgoWaveTableNameChanged(AlgoWaveTableObj->AlgoWaveTableList,
			AlgoWaveTableObj);
	}


/* get a copy of the formula that computes the algorithmic wave table */
char*									AlgoWaveTableObjectGetFormulaCopy(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		char*								TextCopy;

		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				TextCopy = AlgoWaveTableWindowGetFormulaCopy(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				TextCopy = CopyPtr(AlgoWaveTableObj->AlgoWaveTableFormula);
			}
		if (TextCopy != NIL)
			{
				SetTag(TextCopy,"AlgoWaveTableFormulaCopy");
			}
		return TextCopy;
	}


/* install a new formula for the algorithmic wave table.  the object becomes the */
/* owner of the memory block so the caller should not dispose of it. */
void									AlgoWaveTableObjectNewFormula(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												char* Formula)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		CheckPtrExistence(Formula);
		ReleasePtr(AlgoWaveTableObj->AlgoWaveTableFormula);
		SetTag(Formula,"AlgoWaveTableFormula");
		AlgoWaveTableObj->AlgoWaveTableFormula = Formula;
		AlgoWaveTableObj->DataModified = True;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
	}


/* find out how many bits are in each sample frame */
NumBitsType						AlgoWaveTableObjectGetNumBits(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				return AlgoWaveTableWindowGetNumBits(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				return AlgoWaveTableObj->NumBits;
			}
	}


/* install a new number of bits in the object. */
void									AlgoWaveTableObjectPutNumBits(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												NumBitsType NewNumBits)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		AlgoWaveTableObjectUnbuild(AlgoWaveTableObj);
		ERROR((NewNumBits != eSample16bit) && (NewNumBits != eSample8bit),
			PRERR(ForceAbort,"AlgoWaveTableObjectPutNumBits:  bad value"));
		AlgoWaveTableObj->NumBits = NewNumBits;
		AlgoWaveTableObj->DataModified = True;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
	}


/* find out how many frames there are per table */
long									AlgoWaveTableObjectGetNumTables(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				return AlgoWaveTableWindowGetNumTables(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				return AlgoWaveTableObj->NumTables;
			}
	}


/* set the number of tables in the wave table */
void									AlgoWaveTableObjectPutNumTables(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long NumTables)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		AlgoWaveTableObjectUnbuild(AlgoWaveTableObj);
		if (NumTables < 0)
			{
				NumTables = 0;
			}
		AlgoWaveTableObj->NumTables = NumTables;
		AlgoWaveTableObj->DataModified = True;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
	}


/* get number of frames in each table */
long									AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				return AlgoWaveTableWindowGetNumFrames(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		 else
			{
				return AlgoWaveTableObj->NumFrames;
			}
	}


/* set the number of frames in each table */
void									AlgoWaveTableObjectPutNumFrames(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long NumFrames)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		AlgoWaveTableObjectUnbuild(AlgoWaveTableObj);
		AlgoWaveTableObj->NumFrames = NumFrames;
		AlgoWaveTableObj->DataModified = True;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
	}


/* get a raw slice of data from the wave table.  if the wave table is modified, */
/* this pointer becomes invalid. */
char*									AlgoWaveTableObjectGetRawSlice(AlgoWaveTableObjectRec* AlgoWaveTableObj,
												long AlgoWaveTableIndex)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		ERROR((AlgoWaveTableIndex < 0) || (AlgoWaveTableIndex
			> AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj)),PRERR(ForceAbort,
			"AlgoWaveTableObjectGetRawSlice:  table index is out of range"));
		return (char*)WaveTableStorageGetTable(AlgoWaveTableObj->WaveTableData,
			AlgoWaveTableIndex);
	}


/* make the wave table open it's editor window */
MyBoolean							AlgoWaveTableObjectOpenWindow(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow == NIL)
			{
				AlgoWaveTableObj->AlgoWaveTableWindow
					= NewAlgoWaveTableWindow(AlgoWaveTableObj->MainWindow,
					AlgoWaveTableObj,AlgoWaveTableObj->AlgoWaveTableList,
					AlgoWaveTableObj->SavedWindowXLoc,AlgoWaveTableObj->SavedWindowYLoc,
					AlgoWaveTableObj->SavedWindowWidth,AlgoWaveTableObj->SavedWindowHeight);
			}
		 else
			{
				AlgoWaveTableWindowBringToTop(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		return (AlgoWaveTableObj->AlgoWaveTableWindow != NIL);
	}


/* notify the object that the editor window is closing.  the object should */
/* not take any action. */
void									AlgoWaveTableObjectClosingWindowNotify(
												AlgoWaveTableObjectRec* AlgoWaveTableObj,
												short NewX, short NewY, short NewWidth, short NewHeight)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		ERROR(AlgoWaveTableObj->AlgoWaveTableWindow == NIL,PRERR(ForceAbort,
			"AlgoWaveTableObjectClosingWindowNotify:  window not open"));
		AlgoWaveTableObj->AlgoWaveTableWindow = NIL;
		AlgoWaveTableObj->SavedWindowXLoc = NewX;
		AlgoWaveTableObj->SavedWindowYLoc = NewY;
		AlgoWaveTableObj->SavedWindowWidth = NewWidth;
		AlgoWaveTableObj->SavedWindowHeight = NewHeight;
	}


/* the document's name has changed, so we need to update the window */
void									AlgoWaveTableObjectGlobalNameChange(AlgoWaveTableObjectRec*
												AlgoWaveTableObj, char* NewFilename)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				AlgoWaveTableWindowGlobalNameChange(AlgoWaveTableObj->AlgoWaveTableWindow,
					NewFilename);
			}
	}


/* Algorithmic Wave Table Object Subblock Format: */
/*   1-byte format version number */
/*       should be 1 */
/*   2-byte little endian window X position (signed; origin at top-left corner) */
/*   2-byte little endian window Y position */
/*   2-byte little endian window width */
/*   2-byte little endian window height */
/*   4-byte little endian name length descriptor */
/*   n-byte name string (line feed = 0x0a) */
/*   4-byte little endian formula length descriptor */
/*   n-byte formula string (line feed = 0x0a) */
/*   4-byte little endian number of frames */
/*       should be an integral power of 2 in the interval 2..65536 */
/*   4-byte little endian number of tables */
/*   1-byte number of bits */
/*       should be either 8 or 16 */


/* read in an object from the file. */
FileLoadingErrors			AlgoWaveTableObjectNewFromFile(AlgoWaveTableObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		AlgoWaveTableObjectRec*		AlgoWaveTableObj;
		unsigned char							UnsignedChar;
		signed long								SignedLong;
		signed short							SignedShort;
		FileLoadingErrors					Error;

		CheckPtrExistence(Input);
		CheckPtrExistence(CodeCenter);
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(AlgoWaveTableList);

		AlgoWaveTableObj = (AlgoWaveTableObjectRec*)AllocPtrCanFail(
			sizeof(AlgoWaveTableObjectRec),"AlgoWaveTableObjectRec");
		if (AlgoWaveTableObj == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint1:
				return Error;
			}

		/*   1-byte format version number */
		/*       should be 1 */
		if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
			{
				Error = eFileLoadDiskError;
			 FailurePoint2:
				ReleasePtr((char*)AlgoWaveTableObj);
				goto FailurePoint1;
			}
		if (UnsignedChar != 1)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint3:
				goto FailurePoint2;
			}

		/*   2-byte little endian window X position (signed; origin at top-left corner) */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint4:
				goto FailurePoint3;
			}
		AlgoWaveTableObj->SavedWindowXLoc = SignedShort;

		/*   2-byte little endian window Y position */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint5:
				goto FailurePoint4;
			}
		AlgoWaveTableObj->SavedWindowYLoc = SignedShort;

		/*   2-byte little endian window width */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint6:
				goto FailurePoint5;
			}
		AlgoWaveTableObj->SavedWindowWidth = SignedShort;

		/*   2-byte little endian window height */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint7:
				goto FailurePoint6;
			}
		AlgoWaveTableObj->SavedWindowHeight = SignedShort;

		/*   4-byte little endian name length descriptor */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint8:
				goto FailurePoint7;
			}
		if (SignedLong < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint9:
				goto FailurePoint8;
			}

		/*   n-byte name string (line feed = 0x0a) */
		AlgoWaveTableObj->Name = AllocPtrCanFail(SignedLong,"AlgoWaveTableObjectRec:  name");
		if (AlgoWaveTableObj->Name == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint10:
				goto FailurePoint9;
			}
		if (!ReadBufferedInput(Input,SignedLong,AlgoWaveTableObj->Name))
			{
				Error = eFileLoadDiskError;
			 FailurePoint11:
				ReleasePtr(AlgoWaveTableObj->Name);
				goto FailurePoint10;
			}

		/*   4-byte little endian formula length descriptor */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint12:
				goto FailurePoint11;
			}
		if (SignedLong < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint13:
				goto FailurePoint12;
			}

		/*   n-byte formula string (line feed = 0x0a) */
		AlgoWaveTableObj->AlgoWaveTableFormula = AllocPtrCanFail(SignedLong,
			"AlgoWaveTableObjectRec:  formula");
		if (AlgoWaveTableObj->AlgoWaveTableFormula == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint14:
				goto FailurePoint13;
			}
		if (!ReadBufferedInput(Input,SignedLong,AlgoWaveTableObj->AlgoWaveTableFormula))
			{
				Error = eFileLoadDiskError;
			 FailurePoint15:
				ReleasePtr(AlgoWaveTableObj->AlgoWaveTableFormula);
				goto FailurePoint14;
			}

		/*   4-byte little endian number of frames */
		/*       should be an integral power of 2 between 2 and 65536 */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint16:
				goto FailurePoint15;
			}
		if ((SignedLong != 2) && (SignedLong != 4) && (SignedLong != 8)
			&& (SignedLong != 16) && (SignedLong != 32) && (SignedLong != 64)
			&& (SignedLong != 128) && (SignedLong != 256) && (SignedLong != 512)
			&& (SignedLong != 1024) && (SignedLong != 2048) && (SignedLong != 4096)
			&& (SignedLong != 8192) && (SignedLong != 16384) && (SignedLong != 32768)
			&& (SignedLong != 65536))
			{
				Error = eFileLoadBadFormat;
			 FailurePoint17:
				goto FailurePoint16;
			}
		AlgoWaveTableObj->NumFrames = SignedLong;

		/*   4-byte little endian number of tables */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint18:
				goto FailurePoint17;
			}
		if (SignedLong < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint19:
				goto FailurePoint18;
			}
		AlgoWaveTableObj->NumTables = SignedLong;

		/*   1-byte number of bits */
		/*       should be either 8 or 16 */
		if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
			{
				Error = eFileLoadDiskError;
			 FailurePoint20:
				goto FailurePoint19;
			}
		if (UnsignedChar == 8)
			{
				AlgoWaveTableObj->NumBits = eSample8bit;
			}
		else if (UnsignedChar == 16)
			{
				AlgoWaveTableObj->NumBits = eSample16bit;
			}
		else
			{
				Error = eFileLoadBadFormat;
			 FailurePoint21:
				goto FailurePoint20;
			}

		/* fill in the other fields */
		AlgoWaveTableObj->DataModified = False;
		AlgoWaveTableObj->NeedsToBeRebuilt = True;
		AlgoWaveTableObj->WaveTableData = NIL;
		AlgoWaveTableObj->AlgoWaveTableWindow = NIL;
		AlgoWaveTableObj->CodeCenter = CodeCenter;
		AlgoWaveTableObj->MainWindow = MainWindow;
		AlgoWaveTableObj->AlgoWaveTableList = AlgoWaveTableList;

		*ObjectOut = AlgoWaveTableObj;
		return eFileLoadNoError;
	}


/* write the object out to the file. */
FileLoadingErrors			AlgoWaveTableObjectWriteDataOut(
												AlgoWaveTableObjectRec* AlgoWaveTableObj,
												struct BufferedOutputRec* Output)
	{
		char*								StringTemp;

		CheckPtrExistence(AlgoWaveTableObj);
		CheckPtrExistence(Output);

		/*   1-byte format version number */
		/*       should be 1 */
		if (!WriteBufferedUnsignedChar(Output,1))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window X position (signed; origin at top-left corner) */
		/* note that if the window is open when the file is saved, then the most recent */
		/* coordinates of the window will not be used. */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoWaveTableObj->SavedWindowXLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window Y position */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoWaveTableObj->SavedWindowYLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window width */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoWaveTableObj->SavedWindowWidth))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window height */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoWaveTableObj->SavedWindowHeight))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian name length descriptor */
		StringTemp = AlgoWaveTableObjectGetNameCopy(AlgoWaveTableObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-byte name string (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   4-byte little endian formula length descriptor */
		StringTemp = AlgoWaveTableObjectGetFormulaCopy(AlgoWaveTableObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-byte formula string (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   4-byte little endian number of frames */
		/*       should be an integral power of 2 between 2 and 65536 */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoWaveTableObjectGetNumFrames(AlgoWaveTableObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian number of tables */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoWaveTableObjectGetNumTables(AlgoWaveTableObj)))
			{
				return eFileLoadDiskError;
			}

		/*   1-byte number of bits */
		/*       should be either 8 or 16 */
		switch (AlgoWaveTableObjectGetNumBits(AlgoWaveTableObj))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"AlgoWaveTableObjectWriteDataOut:  bad num bits"));
					break;
				case eSample8bit:
					if (!WriteBufferedUnsignedChar(Output,8))
						{
							return eFileLoadDiskError;
						}
					break;
				case eSample16bit:
					if (!WriteBufferedUnsignedChar(Output,16))
						{
							return eFileLoadDiskError;
						}
					break;
			}

		return eFileLoadNoError;
	}


/* mark algorithmic wave table object as saved */
void									AlgoWaveTableObjectMarkAsSaved(AlgoWaveTableObjectRec* AlgoWaveTableObj)
	{
		CheckPtrExistence(AlgoWaveTableObj);
		if (AlgoWaveTableObj->AlgoWaveTableWindow != NIL)
			{
				AlgoWaveTableWindowWritebackModifiedData(AlgoWaveTableObj->AlgoWaveTableWindow);
			}
		AlgoWaveTableObj->DataModified = False;
	}
