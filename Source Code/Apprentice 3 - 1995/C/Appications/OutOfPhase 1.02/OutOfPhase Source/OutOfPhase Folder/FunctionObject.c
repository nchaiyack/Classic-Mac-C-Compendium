/* FunctionObject.c */
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

#include "FunctionObject.h"
#include "Memory.h"
#include "DataMunging.h"
#include "CodeCenter.h"
#include "Numbers.h"
#include "Alert.h"
#include "Array.h"
#include "PcodeDisassembly.h"
#include "FunctionWindow.h"
#include "FunctionList.h"
#include "CompilerRoot.h"
#include "FunctionCode.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


struct FunctionObjectRec
	{
		MyBoolean								Modified;
		char*										Source;
		char*										Name;

		MyBoolean								Built;
		MyBoolean								NeedsToBeRebuilt;
		CodeCenterRec*					CodeCenter;

		FunctionWindowRec*			Window;
		struct MainWindowRec*		MainWindow;
		FunctionListRec*				FuncList;

		short										SavedWindowXLoc;
		short										SavedWindowYLoc;
		short										SavedWindowWidth;
		short										SavedWindowHeight;
	};


FunctionObjectRec*	NewFunctionObject(struct CodeCenterRec* CodeCenter,
											struct MainWindowRec* MainWindow,
											struct FunctionListRec* FuncList)
	{
		FunctionObjectRec*	FuncObj;

		CheckPtrExistence(CodeCenter);
		FuncObj = (FunctionObjectRec*)AllocPtrCanFail(sizeof(FunctionObjectRec),
			"FunctionObjectRec");
		if (FuncObj == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		FuncObj->Source = AllocPtrCanFail(0,"FuncModuleSourceCode");
		if (FuncObj->Source == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)FuncObj);
				goto FailurePoint1;
			}
		FuncObj->Name = StringToBlockCopy("untitled");
		if (FuncObj->Name == NIL)
			{
			 FailurePoint3:
				ReleasePtr(FuncObj->Source);
				goto FailurePoint2;
			}
		SetTag(FuncObj->Name,"FuncModuleName");
		FuncObj->Modified = False;
		FuncObj->NeedsToBeRebuilt = True;
		FuncObj->Built = False;
		FuncObj->CodeCenter = CodeCenter;
		FuncObj->Window = NIL;
		FuncObj->MainWindow = MainWindow;
		FuncObj->FuncList = FuncList;
		FuncObj->SavedWindowXLoc = 0;
		FuncObj->SavedWindowYLoc = 0;
		FuncObj->SavedWindowWidth = 0;
		FuncObj->SavedWindowHeight = 0;
		return FuncObj;
	}


/* get rid of the function object */
void								DisposeFunctionObject(FunctionObjectRec* FuncObj)
	{
		CheckPtrExistence(FuncObj);
		/* don't leave our object code laying around after we're dead */
		FunctionObjectUnbuild(FuncObj);
		/* close the window if it is open */
		if (FuncObj->Window != NIL)
			{
				DisposeFunctionWindow(FuncObj->Window);
				ERROR(FuncObj->Window != NIL,PRERR(AllowResume,
					"DisposeFunctionObject:  window thing not NIL after DisposeFunctionWindow"));
			}
		/* dispose of used memory */
		ReleasePtr(FuncObj->Source);
		ReleasePtr(FuncObj->Name);
		ReleasePtr((char*)FuncObj);
	}


/* inquire whether the function has been modified */
MyBoolean						HasFunctionObjectBeenModified(FunctionObjectRec* FuncObj)
	{
		MyBoolean					Result;

		CheckPtrExistence(FuncObj);
		Result = FuncObj->Modified;
		if (FuncObj->Window != NIL)
			{
				Result = Result || HasFunctionWindowBeenModified(FuncObj->Window);
			}
		return Result;
	}


/* build the function module code.  returns True if successful. */
MyBoolean						FunctionObjectBuild(FunctionObjectRec* FuncObj)
	{
		char*							Data;
		long							ErrorLine;
		CompileErrors			Error;
		char*							ErrorString;
		MyBoolean					SuccessFlag;

		CheckPtrExistence(FuncObj);

		/* make sure our old stuff isn't hanging around */
		FunctionObjectUnbuild(FuncObj);

		/* obtain the source text to be compiled */
		Data = FunctionObjectGetSourceCopy(FuncObj);
		if (Data == NIL)
			{
				AlertHalt("There is not enough memory available to "
					"build the function module.",NIL);
				return False;
			}

		/* compile the module */
		Error = CompileModule(&ErrorLine,Data,FuncObj/*signature*/,FuncObj->CodeCenter);

		/* dispose of the copy of the source text */
		ReleasePtr(Data);

		/* if no errors occurred, then everything is fine */
		if (Error == eCompileNoError)
			{
				/* indicate that we are now successfully built & return */
				FuncObj->Built = True;
				FuncObj->NeedsToBeRebuilt = False;
				return True;
			}

		/* since an error occurred, open the editor window so that we can show */
		/* where it occurred */
		if (!FunctionObjectOpenWindow(FuncObj))
			{
			 FailureMessagePoint:
				AlertHalt("A compile error occurred but there is not enough memory available to "
					"display the error message.",NIL);
				return False;
			}

		/* now, hilite the error line. we subtract one because error lines are */
		/* returned starting at 1, but our line numbers start at 0. */
		FunctionWindowHiliteLine(FuncObj->Window,ErrorLine - 1);

		/* figure out what the error string is */
		SuccessFlag = False;
		ErrorString = StringFromRaw(GetCompileErrorString(Error));
		if (ErrorString != NIL)
			{
				char*					NumberString;

				NumberString = IntegerToString(ErrorLine);
				if (NumberString != NIL)
					{
						char*					Key;

						Key = StringToBlockCopy("_");
						if (Key != NIL)
							{
								char*					BaseMessage;

								BaseMessage = StringFromRaw("Error on line _:  _");
								if (BaseMessage != NIL)
									{
										char*					FixedMessage;

										FixedMessage = ReplaceBlockCopy(BaseMessage,Key,NumberString);
										if (FixedMessage != NIL)
											{
												AlertHalt(FixedMessage,ErrorString);
												ReleasePtr(FixedMessage);
												SuccessFlag = True;
											}
										ReleasePtr(BaseMessage);
									}
								ReleasePtr(Key);
							}
						ReleasePtr(NumberString);
					}
				ReleasePtr(ErrorString);
			}
		if (!SuccessFlag)
			{
				goto FailureMessagePoint;
			}
		return False;
	}


/* unconditionally unbuild the functions */
void								FunctionObjectUnbuild(FunctionObjectRec* FuncObj)
	{
		CheckPtrExistence(FuncObj);
		if (FuncObj->Built)
			{
				FlushModulesCompiledFunctions(FuncObj->CodeCenter,FuncObj/*signature*/);
				FuncObj->Built = False;
			}
	}


/* build the functions if necessary.  return True if successful. */
MyBoolean						FunctionObjectMakeUpToDate(FunctionObjectRec* FuncObj)
	{
		CheckPtrExistence(FuncObj);
		if (!FuncObj->Built || FuncObj->NeedsToBeRebuilt || (FuncObj->Window != NIL))
			{
				return FunctionObjectBuild(FuncObj);
			}
		 else
			{
				return True;
			}
	}


/* return a text block containing a disassembly of the function module */
char*								FunctionObjectDisassemble(FunctionObjectRec* FuncObj)
	{
		ArrayRec*					ListOfFunctions;
		long							Scan;
		char*							BigDaddie;
		long							Limit;

		CheckPtrExistence(FuncObj);

		/* make sure there is something to disassemble */
		if (!FunctionObjectMakeUpToDate(FuncObj))
			{
				return NIL;
			}

		/* allocate the initial text buffer */
		BigDaddie = AllocPtrCanFail(0,"BigDaddieDisassembly");
		if (BigDaddie == NIL)
			{
			 FailurePoint0:
				return NIL;
			}

		/* get a list of functions that need to be disassembled */
		ListOfFunctions = GetListOfFunctionsForModule(FuncObj->CodeCenter,
			FuncObj/*signature*/);

		if (ListOfFunctions == NIL)
			{
			 FailurePoint1:
				ReleasePtr(BigDaddie);
				goto FailurePoint0;
			}

		/* for each function, disassemble it */
		Limit = ArrayGetLength(ListOfFunctions);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				FuncCodeRec*			TheFunction;
				char*							FuncName;
				PcodeRec*					FuncCode;
				char*							TextTemp;
				long							OldSize;
				char*							DisDump;

				TheFunction = (FuncCodeRec*)ArrayGetElement(ListOfFunctions,Scan);
				/* get the actual name (not a copy), null terminated */
				FuncName = GetFunctionName(TheFunction);
				/* get the code to be disassembled */
				FuncCode = GetFunctionPcode(TheFunction);
				/* resize the array to make space for the names */
				OldSize = PtrSize(BigDaddie);
				TextTemp = ResizePtr((char*)BigDaddie,OldSize + 2 + PtrSize(FuncName) + 1);
				if (TextTemp == NIL)
					{
					 FailurePoint2:
						DisposeArray(ListOfFunctions);
						goto FailurePoint1;
					}
				BigDaddie = TextTemp;
				/* copy over name of function */
				PRNGCHK(BigDaddie,&(BigDaddie[OldSize + 0]),2);
				CopyData("\x0a\x0a",&(BigDaddie[OldSize + 0]),2);
				PRNGCHK(BigDaddie,&(BigDaddie[OldSize + 2]),PtrSize(FuncName));
				CopyData(FuncName,&(BigDaddie[OldSize + 2]),PtrSize(FuncName));
				PRNGCHK(BigDaddie,&(BigDaddie[OldSize + 2 + PtrSize(FuncName)]),1);
				CopyData("\x0a",&(BigDaddie[OldSize + 2 + PtrSize(FuncName)]),1);
				/* obtain the text of the disassembly for the specified function */
				DisDump = DisassemblePcode(FuncCode,'\x0a');
				if (DisDump == NIL)
					{
						goto FailurePoint2;
					}
				/* resize the big text block to make room for the disassembly */
				OldSize = PtrSize(BigDaddie);
				TextTemp = ResizePtr((char*)BigDaddie,OldSize + PtrSize(DisDump));
				if (TextTemp == NIL)
					{
						ReleasePtr(DisDump);
						goto FailurePoint2;
					}
				BigDaddie = TextTemp;
				/* copy the disassembly text over */
				PRNGCHK(BigDaddie,&(BigDaddie[OldSize]),PtrSize(DisDump));
				CopyData(DisDump,&(BigDaddie[OldSize]),PtrSize(DisDump));
				/* dispose of the old disassembly text */
				ReleasePtr(DisDump);
			}

		/* get rid of the list of functions */
		DisposeArray(ListOfFunctions);

		/* return the full disassembly to the caller */
		return BigDaddie;
	}


/* get a copy of the name of this function module, no null termination */
char*								FunctionObjectGetNameCopy(FunctionObjectRec* FuncObj)
	{
		char*							NameCopy;

		CheckPtrExistence(FuncObj);
		if (FuncObj->Window != NIL)
			{
				NameCopy = FunctionWindowGetNameCopy(FuncObj->Window);
			}
		 else
			{
				NameCopy = CopyPtr(FuncObj->Name);
			}
		if (NameCopy != NIL)
			{
				SetTag(NameCopy,"FuncObjNameCopy");
			}
		return NameCopy;
	}


/* open the window for this function */
MyBoolean						FunctionObjectOpenWindow(FunctionObjectRec* FuncObj)
	{
		CheckPtrExistence(FuncObj);
		if (FuncObj->Window != NIL)
			{
				FunctionWindowBringToTop(FuncObj->Window);
			}
		 else
			{
				FuncObj->Window = NewFunctionWindow(FuncObj->MainWindow,FuncObj,
					FuncObj->FuncList,FuncObj->SavedWindowXLoc,FuncObj->SavedWindowYLoc,
					FuncObj->SavedWindowWidth,FuncObj->SavedWindowHeight);
			}
		return (FuncObj->Window != NIL);
	}


/* get the text for this function */
char*								FunctionObjectGetSourceCopy(FunctionObjectRec* FuncObj)
	{
		char*							DataCopy;

		CheckPtrExistence(FuncObj);
		if (FuncObj->Window != NIL)
			{
				DataCopy = FunctionWindowGetSourceCopy(FuncObj->Window);
			}
		 else
			{
				DataCopy = CopyPtr(FuncObj->Source);
			}
		if (DataCopy != NIL)
			{
				SetTag(DataCopy,"FuncObjSourceCopy");
			}
		return DataCopy;
	}


/* install new data in the object.  this is used when the editor window closes, */
/* to put the new data into this object.  the object becomes the owner of Data, */
/* so the caller should not release it. */
void								FunctionObjectNewSource(FunctionObjectRec* FuncObj, char* Data)
	{
		CheckPtrExistence(FuncObj);
		CheckPtrExistence(Data);
		SetTag(Data,"FuncObjSourceImported");
		ReleasePtr(FuncObj->Source);
		FuncObj->Source = Data;
		FuncObj->Modified = True;
		FuncObj->NeedsToBeRebuilt = True;
	}


/* give function module a new name.  the object becomes the owner of Name, so */
/* the caller should not release it. */
void								FunctionObjectNewName(FunctionObjectRec* FuncObj, char* Name)
	{
		CheckPtrExistence(FuncObj);
		CheckPtrExistence(Name);
		SetTag(Name,"FuncObjNameImported");
		ReleasePtr(FuncObj->Name);
		FuncObj->Name = Name;
		FuncObj->Modified = True;
		FuncObj->NeedsToBeRebuilt = True;
		FunctionListFunctionNameChanged(FuncObj->FuncList,FuncObj);
	}


/* this is called when the window is closing.  it notifies the object.  the */
/* object should not take any action. */
void								FunctionObjectClosingWindowNotify(FunctionObjectRec* FuncObj,
											short NewWindowX, short NewWindowY, short NewWindowWidth,
											short NewWindowHeight)
	{
		CheckPtrExistence(FuncObj);
		ERROR(FuncObj->Window == NIL,PRERR(ForceAbort,
			"FunctionObjectClosingWindowNotify:  window isn't open"));
		FuncObj->Window = NIL;
		FuncObj->SavedWindowXLoc = NewWindowX;
		FuncObj->SavedWindowYLoc = NewWindowY;
		FuncObj->SavedWindowWidth = NewWindowWidth;
		FuncObj->SavedWindowHeight = NewWindowHeight;
	}


/* the document's name has changed, so we need to update the window */
void								FunctionObjectGlobalNameChange(FunctionObjectRec* FuncObj,
											char* NewFilename)
	{
		CheckPtrExistence(FuncObj);
		if (FuncObj->Window != NIL)
			{
				FunctionWindowGlobalNameChange(FuncObj->Window,NewFilename);
			}
	}


/* Function Object Subblock Structure: */
/*   1-byte function object version number */
/*       should be 1 */
/*   2-byte little endian window x location (origin at top-left of screen) */
/*   2-byte little endian window y location */
/*   2-byte little endian window width */
/*   2-byte little endian window height */
/*   4-byte little endian object name length (positive 2s complement) */
/*   n-byte name data (line feed = 0x0a) */
/*   4-byte little endian function source text length (positive 2s complement) */
/*   n-byte function source text data (line feed = 0x0a) */


/* create a new object initialized with data read in from a file. */
FileLoadingErrors		FunctionObjectNewFromFile(FunctionObjectRec** ObjectOut,
											struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
											struct MainWindowRec* MainWindow,
											struct FunctionListRec* FuncList)
	{
		FunctionObjectRec*	FuncObj;
		signed short				SignedShort;
		FileLoadingErrors		Error;
		signed long					BlockLength;
		unsigned char				FormatVersionNumber;

		CheckPtrExistence(Input);
		CheckPtrExistence(CodeCenter);
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(FuncList);

		/* create a new object thang */
		FuncObj = (FunctionObjectRec*)AllocPtrCanFail(sizeof(FunctionObjectRec),
			"FunctionObjectRec");
		if (FuncObj == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint1:
				return Error;
			}

		/*   1-byte function object version number */
		/*       should be 1 */
		if (!ReadBufferedUnsignedChar(Input,&FormatVersionNumber))
			{
				Error = eFileLoadDiskError;
			 FailurePoint2:
				ReleasePtr((char*)FuncObj);
				goto FailurePoint1;
			}
		if (FormatVersionNumber != 1)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint3:
				goto FailurePoint2;
			}

		/*   2-byte little endian window x location (origin at top-left of screen) */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint4:
				goto FailurePoint3;
			}
		FuncObj->SavedWindowXLoc = SignedShort;

		/*   2-byte little endian window y location */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint5:
				goto FailurePoint4;
			}
		FuncObj->SavedWindowYLoc = SignedShort;

		/*   2-byte little endian window width */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint6:
				goto FailurePoint5;
			}
		FuncObj->SavedWindowWidth = SignedShort;

		/*   2-byte little endian window height */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint7:
				goto FailurePoint6;
			}
		FuncObj->SavedWindowHeight = SignedShort;

		/*   4-byte little endian object name length */
		if (!ReadBufferedSignedLongLittleEndian(Input,&BlockLength))
			{
				Error = eFileLoadDiskError;
			 FailurePoint8:
				goto FailurePoint7;
			}
		if (BlockLength < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint9:
				goto FailurePoint8;
			}

		/*   n-byte name data (line feed = 0x0a) */
		FuncObj->Name = AllocPtrCanFail(BlockLength,"FunctionObjectRec: name");
		if (FuncObj->Name == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint10:
				goto FailurePoint9;
			}
		if (!ReadBufferedInput(Input,BlockLength,FuncObj->Name))
			{
				Error = eFileLoadDiskError;
			 FailurePoint11:
				ReleasePtr(FuncObj->Name);
				goto FailurePoint10;
			}

		/*   4-byte little endian function source text length */
		if (!ReadBufferedSignedLongLittleEndian(Input,&BlockLength))
			{
				Error = eFileLoadDiskError;
			 FailurePoint12:
				goto FailurePoint11;
			}
		if (BlockLength < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint13:
				goto FailurePoint12;
			}

		/*   n-byte function source text data (line feed = 0x0a) */
		FuncObj->Source = AllocPtrCanFail(BlockLength,"FunctionObjectRec: source");
		if (FuncObj->Source == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint14:
				goto FailurePoint13;
			}
		if (!ReadBufferedInput(Input,BlockLength,FuncObj->Source))
			{
				Error = eFileLoadDiskError;
			 FailurePoint15:
				ReleasePtr(FuncObj->Source);
				goto FailurePoint14;
			}

		/* now fill in the other stuff */
		FuncObj->Modified = False;
		FuncObj->Built = False;
		FuncObj->NeedsToBeRebuilt = True;
		FuncObj->CodeCenter = CodeCenter;
		FuncObj->Window = NIL;
		FuncObj->MainWindow = MainWindow;
		FuncObj->FuncList = FuncList;

		*ObjectOut = FuncObj;
		return eFileLoadNoError;
	}


/* write the data in an object out to disk */
FileLoadingErrors		FunctionObjectWriteOutData(FunctionObjectRec* FuncObj,
											struct BufferedOutputRec* Output)
	{
		char*							StringTemp;

		CheckPtrExistence(FuncObj);
		CheckPtrExistence(Output);

		/*   1-byte function object version number */
		/*       should be 1 */
		if (!WriteBufferedUnsignedChar(Output,1))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window x location (origin at top-left of screen) */
		/* note that the way we are doing it here, if the window is open, the most */
		/* recent changes to the location will not used. */
		if (!WriteBufferedSignedShortLittleEndian(Output,FuncObj->SavedWindowXLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window y location */
		if (!WriteBufferedSignedShortLittleEndian(Output,FuncObj->SavedWindowYLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window width */
		if (!WriteBufferedSignedShortLittleEndian(Output,FuncObj->SavedWindowWidth))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window height */
		if (!WriteBufferedSignedShortLittleEndian(Output,FuncObj->SavedWindowHeight))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian object name length */
		StringTemp = FunctionObjectGetNameCopy(FuncObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-byte name data (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   4-byte little endian function source text length */
		StringTemp = FunctionObjectGetSourceCopy(FuncObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-byte function source text data (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		return eFileLoadNoError;
	}


/* mark all items in the function object as not changed */
void								FunctionObjectMarkAsSaved(FunctionObjectRec* FuncObj)
	{
		CheckPtrExistence(FuncObj);
		if (FuncObj->Window != NIL)
			{
				FunctionWindowWritebackModifiedData(FuncObj->Window);
			}
		FuncObj->Modified = False;
	}
