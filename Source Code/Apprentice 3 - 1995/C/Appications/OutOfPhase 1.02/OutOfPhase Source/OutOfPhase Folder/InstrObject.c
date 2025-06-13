/* InstrObject.c */
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

#include "InstrObject.h"
#include "InstrWindow.h"
#include "InstrList.h"
#include "Memory.h"
#include "DataMunging.h"
#include "BuildInstrument.h"
#include "InstrumentStructure.h"
#include "DataMunging.h"
#include "Numbers.h"
#include "Alert.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


struct InstrObjectRec
	{
		MyBoolean								DataModified;

		char*										Name;

		char*										InstrDefinition;
		InstrumentRec*					BuiltInstrument; /* NIL = not built */
		MyBoolean								UpToDate;

		InstrWindowRec*					InstrWindow;

		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		InstrListRec*						InstrList;

		short										SavedWindowXLoc;
		short										SavedWindowYLoc;
		short										SavedWindowWidth;
		short										SavedWindowHeight;
	};


/* create a new empty instrument object */
InstrObjectRec*				NewInstrObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow, struct InstrListRec* InstrList)
	{
		InstrObjectRec*			InstrObj;

		InstrObj = (InstrObjectRec*)AllocPtrCanFail(sizeof(InstrObjectRec),"InstrObjectRec");
		if (InstrObj == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		InstrObj->Name = StringToBlockCopy("untitled");
		if (InstrObj->Name == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)InstrObj);
				goto FailurePoint1;
			}
		InstrObj->InstrDefinition = AllocPtrCanFail(0,"InstrDefinition");
		if (InstrObj->InstrDefinition == NIL)
			{
			 FailurePoint3:
				ReleasePtr(InstrObj->Name);
				goto FailurePoint2;
			}
		InstrObj->CodeCenter = CodeCenter;
		InstrObj->MainWindow = MainWindow;
		InstrObj->InstrList = InstrList;
		InstrObj->DataModified = False;
		InstrObj->InstrWindow = NIL;
		InstrObj->BuiltInstrument = NIL;
		InstrObj->UpToDate = False;
		InstrObj->SavedWindowXLoc = 0;
		InstrObj->SavedWindowYLoc = 0;
		InstrObj->SavedWindowWidth = 0;
		InstrObj->SavedWindowHeight = 0;
		return InstrObj;
	}


/* dispose of instrument object and all the crud it contains */
void									DisposeInstrObject(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				DisposeInstrWindow(InstrObj->InstrWindow);
				ERROR(InstrObj->InstrWindow != NIL,PRERR(ForceAbort,
					"DisposeInstrObject:  disposed window but window isn't NIL"));
			}
		if (InstrObj->BuiltInstrument != NIL)
			{
				DisposeInstrumentSpecification(InstrObj->BuiltInstrument);
			}
		ReleasePtr(InstrObj->Name);
		ReleasePtr(InstrObj->InstrDefinition);
		ReleasePtr((char*)InstrObj);
	}


/* find out if the object has been changed */
MyBoolean							HasInstrObjectBeenModified(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				return InstrObj->DataModified
					|| HasInstrWindowBeenModified(InstrObj->InstrWindow);
			}
		 else
			{
				return InstrObj->DataModified;
			}
	}


/* indicate that object has been modified */
void									InstrObjHasBeenModified(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		InstrObj->DataModified = True;
		InstrObj->UpToDate = False;
	}


/* unbuild the instrument */
void									UnbuildInstrObject(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		InstrObj->UpToDate = False;
		if (InstrObj->BuiltInstrument != NIL)
			{
				DisposeInstrumentSpecification(InstrObj->BuiltInstrument);
				InstrObj->BuiltInstrument = NIL;
			}
	}


/* build the instrument.  returns True if successful */
MyBoolean							BuildInstrObject(InstrObjectRec* InstrObj)
	{
		char*								InstrText;
		BuildInstrErrors		Error;
		long								ErrorLine;
		InstrumentRec*			TheInstrument;

		CheckPtrExistence(InstrObj);
		UnbuildInstrObject(InstrObj);
		InstrText = InstrObjectGetDefinitionCopy(InstrObj);
		if (InstrText == NIL)
			{
				AlertHalt("There is not enough memory available to build instrument "
					"definition.",NIL);
				return False;
			}
		Error = BuildInstrumentFromText(InstrText,&ErrorLine,&TheInstrument,
			MainWindowGetSampleList(InstrObj->MainWindow),MainWindowGetAlgoSampList(
			InstrObj->MainWindow),MainWindowGetWaveTableList(InstrObj->MainWindow),
			MainWindowGetAlgoWaveTableList(InstrObj->MainWindow));
		ReleasePtr(InstrText);
		if (Error != eBuildInstrNoError)
			{
				if (!InstrObjectOpenWindow(InstrObj))
					{
					 CouldntShowErrorPoint:
						AlertHalt("An error occurred while compiling the instrument definition, "
							"but there is not enough memory available to open the window.",NIL);
					}
				 else
					{
						char*								ErrorString;
						MyBoolean						SuccessFlag;

						BringInstrWindowToFront(InstrObj->InstrWindow);
						InstrWindowHilightLine(InstrObj->InstrWindow,ErrorLine - 1);
						/* figure out what the error string is */
						SuccessFlag = False;
						ErrorString = StringFromRaw(BuildInstrGetErrorMessageText(Error));
						if (ErrorString != NIL)
							{
								char*								NumberString;

								NumberString = IntegerToString(ErrorLine);
								if (NumberString != NIL)
									{
										char*								Key;

										Key = StringToBlockCopy("_");
										if (Key != NIL)
											{
												char*								BaseMessage;

												BaseMessage = StringFromRaw("Error on line _:  _");
												if (BaseMessage != NIL)
													{
														char*								FixedMessage;

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
								goto CouldntShowErrorPoint;
							}
					}
				return False;
			}
		InstrObj->BuiltInstrument = TheInstrument;
		InstrObj->UpToDate = True;
		return True;
	}


/* make instrument up to date */
MyBoolean							MakeInstrObjectUpToDate(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (!InstrObj->UpToDate || (InstrObj->InstrWindow != NIL))
			{
				return BuildInstrObject(InstrObj);
			}
		 else
			{
				return True;
			}
	}


/* find out if the instrument object is up to date */
MyBoolean							IsIntrumentObjectUpToDate(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		ERROR(InstrObj->UpToDate && (InstrObj->BuiltInstrument == NIL),PRERR(ForceAbort,
			"IsIntrumentObjectUpToDate:  build instrument inconsistency"));
		return InstrObj->UpToDate;
	}


/* get the definition of the instrument */
struct InstrumentRec*	GetInstrObjectRawData(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (!MakeInstrObjectUpToDate(InstrObj))
			{
				return NIL;
			}
		 else
			{
				return InstrObj->BuiltInstrument;
			}
	}


/* get a copy of the instrument's name */
char*									InstrObjectGetNameCopy(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				return InstrWindowGetNameCopy(InstrObj->InstrWindow);
			}
		 else
			{
				return CopyPtr(InstrObj->Name);
			}
	}


/* install a new name.  the object becomes the owner of the name */
void									InstrObjectPutName(InstrObjectRec* InstrObj, char* Name)
	{
		CheckPtrExistence(InstrObj);
		CheckPtrExistence(Name);
		ReleasePtr(InstrObj->Name);
		InstrObj->Name = Name;
		InstrObj->DataModified = True;
		InstrObj->UpToDate = False;
		InstrListInstrNameChanged(InstrObj->InstrList,InstrObj);
	}


/* get the text definition of the object */
char*									InstrObjectGetDefinitionCopy(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				return InstrWindowGetDefinitionCopy(InstrObj->InstrWindow);
			}
		 else
			{
				return CopyPtr(InstrObj->InstrDefinition);
			}
	}


/* put a new definition of the instrument.  it becomes owner of the block */
void									InstrObjectPutDefinition(InstrObjectRec* InstrObj, char* NewDef)
	{
		CheckPtrExistence(InstrObj);
		CheckPtrExistence(NewDef);
		ReleasePtr(InstrObj->InstrDefinition);
		InstrObj->InstrDefinition = NewDef;
		InstrObj->DataModified = True;
		InstrObj->UpToDate = False;
		UnbuildInstrObject(InstrObj);
	}


/* display the editor window for this object or bring it to the top */
MyBoolean							InstrObjectOpenWindow(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow == NIL)
			{
				/* create window */
				InstrObj->InstrWindow = NewInstrWindow(InstrObj,InstrObj->MainWindow,
					InstrObj->InstrList,InstrObj->SavedWindowXLoc,InstrObj->SavedWindowYLoc,
					InstrObj->SavedWindowWidth,InstrObj->SavedWindowHeight);
			}
		 else
			{
				/* bring window to top */
				BringInstrWindowToFront(InstrObj->InstrWindow);
			}
		return (InstrObj->InstrWindow != NIL);
	}


/* notify the object that the window has disappeared.  The object does not */
/* perform any actions */
void									InstrObjectWindowCloseNotify(InstrObjectRec* InstrObj,
												short NewX, short NewY, short NewWidth, short NewHeight)
	{
		CheckPtrExistence(InstrObj);
		ERROR(InstrObj->InstrWindow == NIL,PRERR(ForceAbort,
			"InstrObjectWindowCloseNotify:  window is already NIL"));
		InstrObj->InstrWindow = NIL;
		InstrObj->SavedWindowXLoc = NewX;
		InstrObj->SavedWindowYLoc = NewY;
		InstrObj->SavedWindowWidth = NewWidth;
		InstrObj->SavedWindowHeight = NewHeight;
	}


/* the document's name changed, so we need to update the window */
void									InstrObjectGlobalNameChange(InstrObjectRec* InstrObj,
												char* NewFilename)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				InstrWindowGlobalNameChange(InstrObj->InstrWindow,NewFilename);
			}
	}


/* Instrument Object Subblock Format: */
/*   1-byte format version number */
/*       should be 1 */
/*   2-byte little endian window X location (signed; origin at top-left of screen) */
/*   2-byte little endian window Y location */
/*   2-byte little endian window width */
/*   2-byte little endian window height */
/*   4-byte little endian name string length */
/*   n-byte name string (line feed = 0x0a) */
/*   4-byte little endian instrument definition length */
/*   n-byte instrument definition string (line feed = 0x0a) */


/* create a new object from data in the file. */
FileLoadingErrors			InstrObjectNewFromFile(InstrObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow, struct InstrListRec* InstrList)
	{
		signed long					SignedLong;
		signed short				SignedShort;
		unsigned char				UnsignedChar;
		FileLoadingErrors		Error;
		InstrObjectRec*			InstrObj;

		CheckPtrExistence(Input);
		CheckPtrExistence(CodeCenter);
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(InstrList);

		InstrObj = (InstrObjectRec*)AllocPtrCanFail(sizeof(InstrObjectRec),"InstrObjectRec");
		if (InstrObj == NIL)
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
				ReleasePtr((char*)InstrObj);
				goto FailurePoint1;
			}
		if (UnsignedChar != 1)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint3:
				goto FailurePoint2;
			}

		/*   2-byte little endian window X location (signed; origin at top-left of screen) */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint4:
				goto FailurePoint3;
			}
		InstrObj->SavedWindowXLoc = SignedShort;

		/*   2-byte little endian window Y location */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint5:
				goto FailurePoint4;
			}
		InstrObj->SavedWindowYLoc = SignedShort;

		/*   2-byte little endian window width */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint6:
				goto FailurePoint5;
			}
		InstrObj->SavedWindowWidth = SignedShort;

		/*   2-byte little endian window height */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint7:
				goto FailurePoint6;
			}
		InstrObj->SavedWindowHeight = SignedShort;

		/*   4-byte little endian name string length */
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
		InstrObj->Name = AllocPtrCanFail(SignedLong,"InstrObjectRec:  name");
		if (InstrObj->Name == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint10:
				goto FailurePoint9;
			}
		if (!ReadBufferedInput(Input,SignedLong,InstrObj->Name))
			{
				Error = eFileLoadDiskError;
			 FailurePoint11:
				ReleasePtr(InstrObj->Name);
				goto FailurePoint10;
			}

		/*   4-byte little endian instrument definition length */
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

		/*   n-byte instrument definition string (line feed = 0x0a) */
		InstrObj->InstrDefinition = AllocPtrCanFail(SignedLong,"InstrObjectRec: definition");
		if (InstrObj->InstrDefinition == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint14:
				goto FailurePoint13;
			}
		if (!ReadBufferedInput(Input,SignedLong,InstrObj->InstrDefinition))
			{
				Error = eFileLoadDiskError;
			 FailurePoint15:
				ReleasePtr(InstrObj->InstrDefinition);
				goto FailurePoint14;
			}

		/* fill in the other fields */
		InstrObj->DataModified = False;
		InstrObj->BuiltInstrument = NIL;
		InstrObj->UpToDate = False;
		InstrObj->InstrWindow = NIL;
		InstrObj->CodeCenter = CodeCenter;
		InstrObj->MainWindow = MainWindow;
		InstrObj->InstrList = InstrList;

		*ObjectOut = InstrObj;
		return eFileLoadNoError;
	}


/* write the data in the object to the file. */
FileLoadingErrors			InstrObjectWriteDataOut(InstrObjectRec* InstrObj,
												struct BufferedOutputRec* Output)
	{
		char*								StringTemp;

		CheckPtrExistence(InstrObj);
		CheckPtrExistence(Output);

		/*   1-byte format version number */
		/*       should be 1 */
		if (!WriteBufferedUnsignedChar(Output,1))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window X location (signed; origin at top-left of screen) */
		/* the way we're doing this, if the window is open when we write the data out, */
		/* then the most recent location of the window will not be saved. */
		if (!WriteBufferedSignedShortLittleEndian(Output,InstrObj->SavedWindowXLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window Y location */
		if (!WriteBufferedSignedShortLittleEndian(Output,InstrObj->SavedWindowYLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window width */
		if (!WriteBufferedSignedShortLittleEndian(Output,InstrObj->SavedWindowWidth))
			{
				return eFileLoadDiskError;
			}

		/*   2-byte little endian window height */
		if (!WriteBufferedSignedShortLittleEndian(Output,InstrObj->SavedWindowHeight))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian name string length */
		StringTemp = InstrObjectGetNameCopy(InstrObj);
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

		/*   4-byte little endian instrument definition length */
		StringTemp = InstrObjectGetDefinitionCopy(InstrObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-byte instrument definition string (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		return eFileLoadNoError;
	}


/* mark the instrument object as saved */
void									InstrObjectMarkAsSaved(InstrObjectRec* InstrObj)
	{
		CheckPtrExistence(InstrObj);
		if (InstrObj->InstrWindow != NIL)
			{
				InstrWindowWritebackModifiedData(InstrObj->InstrWindow);
			}
		InstrObj->DataModified = False;
	}
