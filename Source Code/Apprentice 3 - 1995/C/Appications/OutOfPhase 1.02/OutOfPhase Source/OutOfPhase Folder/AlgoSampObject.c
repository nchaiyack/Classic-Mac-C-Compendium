/* AlgoSampObject.c */
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

#include "AlgoSampObject.h"
#include "AlgoSampList.h"
#include "Memory.h"
#include "DataMunging.h"
#include "CodeCenter.h"
#include "PcodeStack.h"
#include "PcodeSystem.h"
#include "MainWindowStuff.h"
#include "Alert.h"
#include "Numbers.h"
#include "SampleStorageActual.h"
#include "AlgoSampWindow.h"
#include "CompilerRoot.h"
#include "FunctionCode.h"
#include "BufferedFileInput.h"
#include "BufferedFileOutput.h"


struct AlgoSampObjectRec
	{
		MyBoolean								DataModified;

		char*										Name;
		char*										AlgoSampFormula;

		MyBoolean								NeedsToBeRebuilt;
		SampleStorageActualRec*	SampleData; /* NIL if not built */
		NumBitsType							NumBits;
		NumChannelsType					NumChannels;

		long										Origin;
		long										LoopStart1;
		long										LoopStart2;
		long										LoopStart3;
		long										LoopEnd1;
		long										LoopEnd2;
		long										LoopEnd3;
		long										SamplingRate;
		double									NaturalFrequency;

		AlgoSampWindowRec*			AlgoSampWindow;

		struct CodeCenterRec*		CodeCenter;
		struct MainWindowRec*		MainWindow;
		AlgoSampListRec*				AlgoSampList;

		short										SavedWindowXLoc;
		short										SavedWindowYLoc;
		short										SavedWindowWidth;
		short										SavedWindowHeight;
	};


/* allocate and create a new empty algorithmic sample object, with reasonable defaults */
AlgoSampObjectRec*		NewAlgoSampObject(struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoSampListRec* AlgoSampList)
	{
		AlgoSampObjectRec*	AlgoSampObj;

		AlgoSampObj = (AlgoSampObjectRec*)AllocPtrCanFail(sizeof(AlgoSampObjectRec),
			"AlgoSampObjectRec");
		if (AlgoSampObj == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		AlgoSampObj->Name = StringToBlockCopy("untitled");
		if (AlgoSampObj->Name == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)AlgoSampObj);
				goto FailurePoint1;
			}
		AlgoSampObj->AlgoSampFormula = StringToBlockCopy(
			"# samplingrate, origin, loopstart1, loopstart2, loopstart3 : integer\x0a"
			"# loopend1, loopend2, loopend3 : integer\x0a"
			"# naturalfrequency : double; [data | leftdata, rightdata] : fixedarray\x0a");
		if (AlgoSampObj->AlgoSampFormula == NIL)
			{
			 FailurePoint3:
				ReleasePtr(AlgoSampObj->Name);
				goto FailurePoint2;
			}
		AlgoSampObj->DataModified = False;
		AlgoSampObj->NeedsToBeRebuilt = True;
		AlgoSampObj->SampleData = NIL;
		AlgoSampObj->Origin = 0;
		AlgoSampObj->LoopStart1 = 0;
		AlgoSampObj->LoopStart2 = 0;
		AlgoSampObj->LoopStart3 = 0;
		AlgoSampObj->LoopEnd1 = 0;
		AlgoSampObj->LoopEnd2 = 0;
		AlgoSampObj->LoopEnd3 = 0;
		AlgoSampObj->SamplingRate = 22050;
		AlgoSampObj->NaturalFrequency = 261.625565300598635;
		AlgoSampObj->AlgoSampWindow = NIL;
		AlgoSampObj->CodeCenter = CodeCenter;
		AlgoSampObj->MainWindow = MainWindow;
		AlgoSampObj->AlgoSampList = AlgoSampList;
		AlgoSampObj->NumBits = eSample16bit;
		AlgoSampObj->NumChannels = eSampleMono;
		AlgoSampObj->SavedWindowXLoc = 0;
		AlgoSampObj->SavedWindowYLoc = 0;
		AlgoSampObj->SavedWindowWidth = 0;
		AlgoSampObj->SavedWindowHeight = 0;
		return AlgoSampObj;
	}


/* dispose of an algorithmic sample object */
void									DisposeAlgoSampObject(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				DisposeAlgoSampWindow(AlgoSampObj->AlgoSampWindow);
				ERROR(AlgoSampObj->AlgoSampWindow != NIL,PRERR(AllowResume,
					"DisposeAlgoSampObject:  window thing not NIL after DisposeAlgoSampWindow"));
			}
		ReleasePtr(AlgoSampObj->Name);
		ReleasePtr(AlgoSampObj->AlgoSampFormula);
		if (AlgoSampObj->SampleData != NIL)
			{
				DisposeSampleStorageActual(AlgoSampObj->SampleData);
			}
		ReleasePtr((char*)AlgoSampObj);
	}


/* find out if algorithmic sample object has been modified */
MyBoolean							HasAlgoSampObjectBeenModified(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampObj->DataModified
					|| HasAlgoSampWindowBeenModified(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->DataModified;
			}
	}


static FunctionParamRec		StereoArgList[] =
	{
		{"loopstart1",eInteger},
		{"loopstart2",eInteger},
		{"loopstart3",eInteger},
		{"loopend1",eInteger},
		{"loopend2",eInteger},
		{"loopend3",eInteger},
		{"origin",eInteger},
		{"samplingrate",eInteger},
		{"naturalfrequency",eInteger},
		{"leftdata",eArrayOfFixed},
		{"rightdata",eArrayOfFixed}
	};
#define STEREOARGLISTLENGTH (sizeof(StereoArgList) / sizeof(StereoArgList[0]))


static FunctionParamRec		MonoArgList[] =
	{
		{"loopstart1",eInteger},
		{"loopstart2",eInteger},
		{"loopstart3",eInteger},
		{"loopend1",eInteger},
		{"loopend2",eInteger},
		{"loopend3",eInteger},
		{"origin",eInteger},
		{"samplingrate",eInteger},
		{"naturalfrequency",eInteger},
		{"data",eArrayOfFixed}
	};
#define MONOARGLISTLENGTH (sizeof(MonoArgList) / sizeof(MonoArgList[0]))


/* build the algorithmic sample.  returns True if successful. */
MyBoolean							AlgoSampObjectBuild(AlgoSampObjectRec* AlgoSampObj)
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

		CheckPtrExistence(AlgoSampObj);

		/* make sure we are unbuilt */
		AlgoSampObjectUnbuild(AlgoSampObj);
		ERROR(AlgoSampObj->SampleData != NIL,PRERR(ForceAbort,
			"AlgoSampObjectBuild:  called AlgoSampObjectUnbuild but object still exists"));

		/* bring the world up to date */
		if (!MainWindowMakeUpToDateFunctions(AlgoSampObj->MainWindow))
			{
				return False;
			}

		/* prepare the text blob to be evaluated */
		Blob = AlgoSampObjectGetFormulaCopy(AlgoSampObj);
		if (Blob == NIL)
			{
			 FailurePoint1:
				AlertHalt("There is not enough memory available to compile the algorithmic "
					"sample generator function.",NIL);
				return False;
			}

		/* perform compilation */
		Error = CompileSpecialFunction((AlgoSampObjectGetNumChannels(AlgoSampObj)
			== eSampleStereo) ? StereoArgList : MonoArgList,(AlgoSampObjectGetNumChannels(
			AlgoSampObj) == eSampleStereo) ? STEREOARGLISTLENGTH : MONOARGLISTLENGTH,
			&LineNumber,&ReturnType,Blob,&FuncCode);
		ReleasePtr(Blob);
		/* show the error message if there is one */
		if (Error != eCompileNoError)
			{
				if (!AlgoSampObjectOpenWindow(AlgoSampObj))
					{
						AlertHalt("A compile error occurred but there is not enough memory "
							"available to display the error message.",NIL);
						return False;
					}
				AlgoSampWindowHiliteLine(AlgoSampObj->AlgoSampWindow,LineNumber - 1);
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
					"sample generator function.",NIL);
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
		/* loopstart */
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopStart1(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopStart2(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopStart3(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		/* loopend */
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopEnd1(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopEnd2(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetLoopEnd3(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		/* origin */
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetOrigin(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		/* samplingrate */
		if (!AddIntegerToStack(ParamList,AlgoSampObjectGetSamplingRate(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		/* basefrequency */
		if (!AddDoubleToStack(ParamList,AlgoSampObjectGetNaturalFrequency(AlgoSampObj)))
			{
				goto SecondFailurePoint2;
			}
		/* data */
		if (AlgoSampObjectGetNumChannels(AlgoSampObj) == eSampleStereo)
			{
				char*					Buffer;

				Buffer = AllocPtrCanFail(0,"degeneratesample");
				if (Buffer == NIL)
					{
						goto SecondFailurePoint2;
					}
				if (!AddArrayToStack(ParamList,Buffer))
					{
						ReleasePtr(Buffer);
						goto SecondFailurePoint2;
					}
				Buffer = AllocPtrCanFail(0,"degeneratesample");
				if (Buffer == NIL)
					{
						goto SecondFailurePoint2;
					}
				if (!AddArrayToStack(ParamList,Buffer))
					{
						ReleasePtr(Buffer);
						goto SecondFailurePoint2;
					}
			}
		 else
			{
				char*					Buffer;

				Buffer = AllocPtrCanFail(0,"degeneratesample");
				if (Buffer == NIL)
					{
						goto SecondFailurePoint2;
					}
				if (!AddArrayToStack(ParamList,Buffer))
					{
						ReleasePtr(Buffer);
						goto SecondFailurePoint2;
					}
			}

		/* executing the actual code */
		OtherError = EvaluatePcode(ParamList,FuncCode,
			AlgoSampObj->CodeCenter,&ErrorOpcode,&OffendingInstruction,AlgoSampObj->MainWindow,
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
				AlgoSampObjectOpenWindow(AlgoSampObj);
				SuccessFlag = False;
				ErrorFunction = GetFunctionFromOpcode(AlgoSampObj->CodeCenter,ErrorOpcode);
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
		if (AlgoSampObjectGetNumChannels(AlgoSampObj) == eSampleStereo)
			{
				largefixedsigned*					Left;
				largefixedsigned*					Right;
				long											Limit;
				long											Scan;

				Left = (largefixedsigned*)GetStackArray(ParamList,10);
				Right = (largefixedsigned*)GetStackArray(ParamList,11);
				if ((Left == NIL) || (Right == NIL))
					{
						AlertHalt("NIL array returned from function building algorithmic sample.",NIL);
					 StereoRebuildFailurePoint1:
						DisposeParamStack(ParamList);
						AlgoSampObjectOpenWindow(AlgoSampObj);
						return False;
					}
				if (PtrSize((char*)Left) != PtrSize((char*)Right))
					{
						AlertHalt("Left and Right algorithmic sample arrays are not the same size.",NIL);
					 StereoRebuildFailurePoint2:
						goto StereoRebuildFailurePoint1;
					}
				ERROR(PtrSize((char*)Left) % sizeof(largefixedsigned) != 0,
					PRERR(ForceAbort,"AlgoSampObjectBuild:  array alignment error"));
				Limit = PtrSize((char*)Left) / sizeof(largefixedsigned);
				AlgoSampObj->SampleData = NewSampleStorageActual(AlgoSampObjectGetNumBits(
					AlgoSampObj),eSampleStereo,Limit);
				if (AlgoSampObj->SampleData == NIL)
					{
						AlertHalt("There is not enough memory available to build "
							"the algorithmic sample.",NIL);
					 StereoRebuildFailurePoint3:
						goto StereoRebuildFailurePoint2;
					}
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						SetSampleStorageActualValue(AlgoSampObj->SampleData,Scan,
							eLeftChannel,Left[Scan]);
						SetSampleStorageActualValue(AlgoSampObj->SampleData,Scan,
							eRightChannel,Right[Scan]);
					}
			}
		 else
			{
				largefixedsigned*					Middle;
				long											Limit;
				long											Scan;

				Middle = (largefixedsigned*)GetStackArray(ParamList,10);
				if (Middle == NIL)
					{
						AlertHalt("NIL array returned from function building algorithmic sample.",NIL);
					 MonoRebuildFailurePoint1:
						DisposeParamStack(ParamList);
						AlgoSampObjectOpenWindow(AlgoSampObj);
						return False;
					}
				ERROR(PtrSize((char*)Middle) % sizeof(largefixedsigned) != 0,
					PRERR(ForceAbort,"AlgoSampObjectBuild:  array alignment error"));
				Limit = PtrSize((char*)Middle) / sizeof(largefixedsigned);
				AlgoSampObj->SampleData = NewSampleStorageActual(AlgoSampObjectGetNumBits(
					AlgoSampObj),eSampleMono,Limit);
				if (AlgoSampObj->SampleData == NIL)
					{
						AlertHalt("There is not enough memory available to build "
							"the algorithmic sample.",NIL);
					 MonoRebuildFailurePoint2:
						goto MonoRebuildFailurePoint1;
					}
				for (Scan = 0; Scan < Limit; Scan += 1)
					{
						SetSampleStorageActualValue(AlgoSampObj->SampleData,Scan,
							eMonoChannel,Middle[Scan]);
					}
			}
		DisposeParamStack(ParamList);
		AlgoSampObj->NeedsToBeRebuilt = False;
		return True;
	}


/* unconditionally unbuild the algorithmic sample */
void									AlgoSampObjectUnbuild(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->SampleData != NIL)
			{
				DisposeSampleStorageActual(AlgoSampObj->SampleData);
				AlgoSampObj->SampleData = NIL;
			}
	}


/* build the algorithmic sample if necessary.  return True if successful. */
MyBoolean							AlgoSampObjectMakeUpToDate(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if ((AlgoSampObj->SampleData == NIL) || (AlgoSampObj->NeedsToBeRebuilt)
			|| (AlgoSampObj->AlgoSampWindow != NIL))
			{
				return AlgoSampObjectBuild(AlgoSampObj);
			}
		 else
			{
				return True;
			}
	}


/* get a copy of the algorithmic sample's name. */
char*									AlgoSampObjectGetNameCopy(AlgoSampObjectRec* AlgoSampObj)
	{
		char*								NameTemp;

		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				NameTemp = AlgoSampWindowGetNameCopy(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				NameTemp = CopyPtr(AlgoSampObj->Name);
			}
		if (NameTemp != NIL)
			{
				SetTag(NameTemp,"AlgoSampNameCopy");
			}
		return NameTemp;
	}


/* set the algorithmic sample's name.  the object becomes the owner of the Name */
/* block so the caller should not release it. */
void									AlgoSampObjectNewName(AlgoSampObjectRec* AlgoSampObj, char* Name)
	{
		CheckPtrExistence(AlgoSampObj);
		CheckPtrExistence(Name);
		ReleasePtr(AlgoSampObj->Name);
		SetTag(Name,"AlgoSampName");
		AlgoSampObj->Name = Name;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
		AlgoSampListAlgoSampNameChanged(AlgoSampObj->AlgoSampList,AlgoSampObj);
	}


/* get a copy of the formula that generates the algorithmic sample */
char*									AlgoSampObjectGetFormulaCopy(AlgoSampObjectRec* AlgoSampObj)
	{
		char*								TextCopy;

		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				TextCopy = AlgoSampWindowGetFormulaCopy(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				TextCopy = CopyPtr(AlgoSampObj->AlgoSampFormula);
			}
		if (TextCopy != NIL)
			{
				SetTag(TextCopy,"AlgoSampFormulaCopy");
			}
		return TextCopy;
	}


/* install a new formula into the algorithmic sample.  the object becomes the owner */
/* of the formula, so the caller should not release it */
void									AlgoSampObjectNewFormula(AlgoSampObjectRec* AlgoSampObj,
												char* Formula)
	{
		CheckPtrExistence(AlgoSampObj);
		CheckPtrExistence(Formula);
		ReleasePtr(AlgoSampObj->AlgoSampFormula);
		SetTag(Formula,"AlgoSampFormula");
		AlgoSampObj->AlgoSampFormula = Formula;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


/* get the number of bits in a channel for the algorithmic sample */
NumBitsType						AlgoSampObjectGetNumBits(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetNumBits(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->NumBits;
			}
	}


/* change the number of bits in a channel for the algorithmic sample */
void									AlgoSampObjectPutNumBits(AlgoSampObjectRec* AlgoSampObj,
												NumBitsType NewNumBits)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObjectUnbuild(AlgoSampObj);
		ERROR((NewNumBits != eSample16bit) && (NewNumBits != eSample8bit),
			PRERR(ForceAbort,"AlgoSampObjectPutNumBits:  bad value"));
		AlgoSampObj->NumBits = NewNumBits;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


/* get the number of channels in the algorithmic sample */
NumChannelsType				AlgoSampObjectGetNumChannels(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetNumChannels(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->NumChannels;
			}
	}


/* change the number of channels for the algorithmic sample */
void									AlgoSampObjectPutNumChannels(AlgoSampObjectRec* AlgoSampObj,
												NumChannelsType NewNumChannels)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObjectUnbuild(AlgoSampObj);
		ERROR((NewNumChannels != eSampleMono) && (NewNumChannels != eSampleStereo),
			PRERR(ForceAbort,"AlgoSampObjectPutNumChannels:  bad value"));
		AlgoSampObj->NumChannels = NewNumChannels;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


/* get the number of data frames in the algorithmic sample */
long									AlgoSampObjetGetNumFrames(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		ERROR(AlgoSampObj->SampleData == NIL,PRERR(ForceAbort,
			"AlgoSampObjetGetNumFrames:  sample data has not been built"));
		return GetSampleStorageActualNumFrames(AlgoSampObj->SampleData);
	}


long									AlgoSampObjectGetOrigin(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetOrigin(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->Origin;
			}
	}


long									AlgoSampObjectGetLoopStart1(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopStart1(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopStart1;
			}
	}


long									AlgoSampObjectGetLoopStart2(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopStart2(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopStart2;
			}
	}


long									AlgoSampObjectGetLoopStart3(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopStart3(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopStart3;
			}
	}


long									AlgoSampObjectGetLoopEnd1(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopEnd1(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopEnd1;
			}
	}


long									AlgoSampObjectGetLoopEnd2(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopEnd2(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopEnd2;
			}
	}


long									AlgoSampObjectGetLoopEnd3(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetLoopEnd3(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->LoopEnd3;
			}
	}


long									AlgoSampObjectGetSamplingRate(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetSamplingRate(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->SamplingRate;
			}
	}


double								AlgoSampObjectGetNaturalFrequency(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				return AlgoSampWindowGetNaturalFrequency(AlgoSampObj->AlgoSampWindow);
			}
		 else
			{
				return AlgoSampObj->NaturalFrequency;
			}
	}


char*									AlgoSampObjectGetRawData(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if ((AlgoSampObj->SampleData == NIL) || AlgoSampObj->NeedsToBeRebuilt)
			{
				return NIL;
			}
		return GetSampleStorageActualRawData(AlgoSampObj->SampleData);
	}


void									AlgoSampObjectPutOrigin(AlgoSampObjectRec* AlgoSampObj,
												long Origin)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->Origin = Origin;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopStart1(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopStart1 = LoopStart;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopStart2(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopStart2 = LoopStart;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopStart3(AlgoSampObjectRec* AlgoSampObj,
												long LoopStart)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopStart3 = LoopStart;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopEnd1(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopEnd1 = LoopEnd;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopEnd2(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopEnd2 = LoopEnd;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutLoopEnd3(AlgoSampObjectRec* AlgoSampObj,
												long LoopEnd)
	{
		CheckPtrExistence(AlgoSampObj);
		AlgoSampObj->LoopEnd3 = LoopEnd;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutSamplingRate(AlgoSampObjectRec* AlgoSampObj,
												long SamplingRate)
	{
		CheckPtrExistence(AlgoSampObj);
		if (SamplingRate < MINSAMPLINGRATE)
			{
				SamplingRate = MINSAMPLINGRATE;
			}
		if (SamplingRate > MAXSAMPLINGRATE)
			{
				SamplingRate = MAXSAMPLINGRATE;
			}
		AlgoSampObj->SamplingRate = SamplingRate;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


void									AlgoSampObjectPutNaturalFrequency(AlgoSampObjectRec* AlgoSampObj,
												double NaturalFrequency)
	{
		CheckPtrExistence(AlgoSampObj);
		if (NaturalFrequency < MINNATURALFREQ)
			{
				NaturalFrequency = MINNATURALFREQ;
			}
		if (NaturalFrequency > MAXNATURALFREQ)
			{
				NaturalFrequency = MAXNATURALFREQ;
			}
		AlgoSampObj->NaturalFrequency = NaturalFrequency;
		AlgoSampObj->DataModified = True;
		AlgoSampObj->NeedsToBeRebuilt = True;
	}


/* call which makes object open its editor window */
MyBoolean							AlgoSampObjectOpenWindow(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow == NIL)
			{
				AlgoSampObj->AlgoSampWindow = NewAlgoSampWindow(AlgoSampObj->MainWindow,
					AlgoSampObj,AlgoSampObj->AlgoSampList,AlgoSampObj->SavedWindowXLoc,
					AlgoSampObj->SavedWindowYLoc,AlgoSampObj->SavedWindowWidth,
					AlgoSampObj->SavedWindowHeight);
			}
		 else
			{
				AlgoSampWindowBringToTop(AlgoSampObj->AlgoSampWindow);
			}
		return (AlgoSampObj->AlgoSampWindow != NIL);
	}


/* this is called by the window when it is closing to notify the object. */
/* the object should not take any action. */
void									AlgoSampObjectClosingWindowNotify(AlgoSampObjectRec* AlgoSampObj,
												short NewX, short NewY, short NewWidth, short NewHeight)
	{
		CheckPtrExistence(AlgoSampObj);
		ERROR(AlgoSampObj->AlgoSampWindow == NIL,PRERR(ForceAbort,
			"AlgoSampObjectClosingWindowNotify:  window not open"));
		AlgoSampObj->AlgoSampWindow = NIL;
		AlgoSampObj->SavedWindowXLoc = NewX;
		AlgoSampObj->SavedWindowYLoc = NewY;
		AlgoSampObj->SavedWindowWidth = NewWidth;
		AlgoSampObj->SavedWindowHeight = NewHeight;
	}


/* the document's name has changed, so the windows need to be updated */
void									AlgoSampObjectGlobalNameChange(AlgoSampObjectRec* AlgoSampObj,
												char* NewFilename)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				AlgoSampWindowGlobalNameChange(AlgoSampObj->AlgoSampWindow,NewFilename);
			}
	}


/* Algorithmic Sample Object Subblock Format: */
/*   1-byte format version number */
/*       should be 1 */
/*   2-bytes little endian window X location (signed, origin at top-left corner) */
/*   2-bytes little endian window Y location */
/*   2-bytes little endian window width */
/*   2-bytes little endian window height */
/*   4-bytes little endian name length descriptor */
/*   n-bytes name string (line feed = 0x0a) */
/*   4-bytes little endian formula length descriptor */
/*   n-bytes formula string (line feed = 0x0a) */
/*   1-byte number of bits */
/*       should be 8 or 16 */
/*   1-byte number of channels */
/*       1 = mono */
/*       2 = stereo */
/*   4-bytes little endian sample origin */
/*   4-bytes little endian loop 1 start point */
/*   4-bytes little endian loop 1 end point */
/*   4-bytes little endian loop 2 start point */
/*   4-bytes little endian loop 2 end point */
/*   4-bytes little endian loop 3 start point */
/*   4-bytes little endian loop 3 end point */
/*   4-bytes little endian sampling rate */
/*       should be between 100 and 65535 */
/*   4-byte little endian natural frequency fractional portion */
/*       unsigned; divide by 2^32 to get the actual fraction */
/*   4-byte little endian natural frequency integer portion */
/*       total natural frequency should be between 0.01 and 1e6 */


/* read from the file and create a new algorithmic sample object from it. */
FileLoadingErrors			AlgoSampObjectNewFromFile(AlgoSampObjectRec** ObjectOut,
												struct BufferedInputRec* Input, struct CodeCenterRec* CodeCenter,
												struct MainWindowRec* MainWindow,
												struct AlgoSampListRec* AlgoSampList)
	{
		unsigned char				UnsignedChar;
		signed short				SignedShort;
		AlgoSampObjectRec*	AlgoSampObj;
		FileLoadingErrors		Error;
		signed long					SignedLong;
		unsigned long				UnsignedLong;

		CheckPtrExistence(Input);
		CheckPtrExistence(CodeCenter);
		CheckPtrExistence(MainWindow);
		CheckPtrExistence(AlgoSampList);

		AlgoSampObj = (AlgoSampObjectRec*)AllocPtrCanFail(sizeof(AlgoSampObjectRec),
			"AlgoSampObjectRec");
		if (AlgoSampObj == NIL)
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
				ReleasePtr((char*)AlgoSampObj);
				goto FailurePoint1;
			}

		/*   2-bytes little endian window X location (signed, origin at top-left corner) */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint3:
				goto FailurePoint2;
			}
		AlgoSampObj->SavedWindowXLoc = SignedShort;

		/*   2-bytes little endian window Y location */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint4:
				goto FailurePoint3;
			}
		AlgoSampObj->SavedWindowYLoc = SignedShort;

		/*   2-bytes little endian window width */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint5:
				goto FailurePoint4;
			}
		AlgoSampObj->SavedWindowWidth = SignedShort;

		/*   2-bytes little endian window height */
		if (!ReadBufferedSignedShortLittleEndian(Input,&SignedShort))
			{
				Error = eFileLoadDiskError;
			 FailurePoint6:
				goto FailurePoint5;
			}
		AlgoSampObj->SavedWindowHeight = SignedShort;

		/*   4-bytes little endian name length descriptor */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint7:
				goto FailurePoint6;
			}
		if (SignedLong < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint8:
				goto FailurePoint7;
			}

		/*   n-bytes name string (line feed = 0x0a) */
		AlgoSampObj->Name = AllocPtrCanFail(SignedLong,"AlgoSampObjectRec:  name");
		if (AlgoSampObj->Name == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint9:
				goto FailurePoint8;
			}
		if (!ReadBufferedInput(Input,SignedLong,AlgoSampObj->Name))
			{
				Error = eFileLoadDiskError;
			 FailurePoint10:
				ReleasePtr(AlgoSampObj->Name);
				goto FailurePoint9;
			}

		/*   4-bytes little endian formula length descriptor */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint11:
				goto FailurePoint10;
			}
		if (SignedLong < 0)
			{
				Error = eFileLoadBadFormat;
			 FailurePoint12:
				goto FailurePoint11;
			}

		/*   n-bytes formula string (line feed = 0x0a) */
		AlgoSampObj->AlgoSampFormula = AllocPtrCanFail(SignedLong,
			"AlgoSampObjectRec:  formula");
		if (AlgoSampObj->AlgoSampFormula == NIL)
			{
				Error = eFileLoadOutOfMemory;
			 FailurePoint13:
				goto FailurePoint12;
			}
		if (!ReadBufferedInput(Input,SignedLong,AlgoSampObj->AlgoSampFormula))
			{
				Error = eFileLoadDiskError;
			 FailurePoint14:
				ReleasePtr(AlgoSampObj->AlgoSampFormula);
				goto FailurePoint13;
			}

		/*   1-byte number of bits */
		/*       should be 8 or 16 */
		if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
			{
				Error = eFileLoadDiskError;
			 FailurePoint15:
				goto FailurePoint14;
			}
		if (UnsignedChar == 8)
			{
				AlgoSampObj->NumBits = eSample8bit;
			}
		else if (UnsignedChar == 16)
			{
				AlgoSampObj->NumBits = eSample16bit;
			}
		else
			{
				Error = eFileLoadBadFormat;
			 FailurePoint16:
				goto FailurePoint15;
			}

		/*   1-byte number of channels */
		/*       1 = mono */
		/*       2 = stereo */
		if (!ReadBufferedUnsignedChar(Input,&UnsignedChar))
			{
				Error = eFileLoadDiskError;
			 FailurePoint17:
				goto FailurePoint16;
			}
		if (UnsignedChar == 1)
			{
				AlgoSampObj->NumChannels = eSampleMono;
			}
		else if (UnsignedChar == 2)
			{
				AlgoSampObj->NumChannels = eSampleStereo;
			}
		else
			{
				Error = eFileLoadBadFormat;
			 FailurePoint18:
				goto FailurePoint17;
			}

		/*   4-bytes little endian sample origin */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint19:
				goto FailurePoint18;
			}
		AlgoSampObj->Origin = SignedLong;

		/*   4-bytes little endian loop 1 start point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint20:
				goto FailurePoint19;
			}
		AlgoSampObj->LoopStart1 = SignedLong;

		/*   4-bytes little endian loop 1 end point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint21:
				goto FailurePoint20;
			}
		AlgoSampObj->LoopEnd1 = SignedLong;

		/*   4-bytes little endian loop 2 start point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint22:
				goto FailurePoint21;
			}
		AlgoSampObj->LoopStart2 = SignedLong;

		/*   4-bytes little endian loop 2 end point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint23:
				goto FailurePoint22;
			}
		AlgoSampObj->LoopEnd2 = SignedLong;

		/*   4-bytes little endian loop 3 start point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint24:
				goto FailurePoint23;
			}
		AlgoSampObj->LoopStart3 = SignedLong;

		/*   4-bytes little endian loop 3 end point */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint25:
				goto FailurePoint24;
			}
		AlgoSampObj->LoopEnd3 = SignedLong;

		/*   4-bytes little endian sampling rate */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint26:
				goto FailurePoint25;
			}
		if (SignedLong < MINSAMPLINGRATE)
			{
				SignedLong = MINSAMPLINGRATE;
			}
		if (SignedLong > MAXSAMPLINGRATE)
			{
				SignedLong = MAXSAMPLINGRATE;
			}
		AlgoSampObj->SamplingRate = SignedLong;

		/*   4-byte little endian natural frequency fractional portion */
		/*       unsigned; divide by 2^32 to get the actual fraction */
		if (!ReadBufferedUnsignedLongLittleEndian(Input,&UnsignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint27:
				goto FailurePoint26;
			}
		AlgoSampObj->NaturalFrequency = (double)UnsignedLong / 4294967296.0L;

		/*   4-byte little endian natural frequency integer portion */
		if (!ReadBufferedSignedLongLittleEndian(Input,&SignedLong))
			{
				Error = eFileLoadDiskError;
			 FailurePoint28:
				goto FailurePoint27;
			}
		AlgoSampObj->NaturalFrequency += SignedLong;
		if (AlgoSampObj->NaturalFrequency < MINNATURALFREQ)
			{
				AlgoSampObj->NaturalFrequency = MINNATURALFREQ;
			}
		if (AlgoSampObj->NaturalFrequency > MAXNATURALFREQ)
			{
				AlgoSampObj->NaturalFrequency = MAXNATURALFREQ;
			}

		/* fill in the other fields */
		AlgoSampObj->DataModified = False;
		AlgoSampObj->NeedsToBeRebuilt = True;
		AlgoSampObj->SampleData = NIL;
		AlgoSampObj->AlgoSampWindow = NIL;
		AlgoSampObj->CodeCenter = CodeCenter;
		AlgoSampObj->MainWindow = MainWindow;
		AlgoSampObj->AlgoSampList = AlgoSampList;

		*ObjectOut = AlgoSampObj;
		return eFileLoadNoError;
	}


/* write the object out to the specified file. */
FileLoadingErrors			AlgoSampObjectWriteOutData(AlgoSampObjectRec* AlgoSampObj,
												struct BufferedOutputRec* Output)
	{
		char*								StringTemp;
		double							NaturalFreqTemp;

		CheckPtrExistence(AlgoSampObj);
		CheckPtrExistence(Output);

		/*   1-byte format version number */
		/*       should be 1 */
		if (!WriteBufferedUnsignedChar(Output,1))
			{
				return eFileLoadDiskError;
			}

		/*   2-bytes little endian window X location (signed, origin at top-left corner) */
		/* if the window is open when we save, the most recent coordinates of the */
		/* window will not be used */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoSampObj->SavedWindowXLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-bytes little endian window Y location */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoSampObj->SavedWindowYLoc))
			{
				return eFileLoadDiskError;
			}

		/*   2-bytes little endian window width */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoSampObj->SavedWindowWidth))
			{
				return eFileLoadDiskError;
			}

		/*   2-bytes little endian window height */
		if (!WriteBufferedSignedShortLittleEndian(Output,AlgoSampObj->SavedWindowHeight))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian name length descriptor */
		StringTemp = AlgoSampObjectGetNameCopy(AlgoSampObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-bytes name string (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   4-bytes little endian formula length descriptor */
		StringTemp = AlgoSampObjectGetFormulaCopy(AlgoSampObj);
		if (StringTemp == NIL)
			{
				return eFileLoadOutOfMemory;
			}
		if (!WriteBufferedSignedLongLittleEndian(Output,PtrSize(StringTemp)))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}

		/*   n-bytes formula string (line feed = 0x0a) */
		if (!WriteBufferedOutput(Output,PtrSize(StringTemp),StringTemp))
			{
				ReleasePtr(StringTemp);
				return eFileLoadDiskError;
			}
		ReleasePtr(StringTemp);

		/*   1-byte number of bits */
		/*       should be 8 or 16 */
		switch (AlgoSampObjectGetNumBits(AlgoSampObj))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"AlgoSampObjectWriteOutData:  bad num bits"));
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

		/*   1-byte number of channels */
		/*       1 = mono */
		/*       2 = stereo */
		switch (AlgoSampObjectGetNumChannels(AlgoSampObj))
			{
				default:
					EXECUTE(PRERR(ForceAbort,"AlgoSampObjectWriteOutData:  bad num channels"));
					break;
				case eSampleMono:
					if (!WriteBufferedUnsignedChar(Output,1))
						{
							return eFileLoadDiskError;
						}
					break;
				case eSampleStereo:
					if (!WriteBufferedUnsignedChar(Output,2))
						{
							return eFileLoadDiskError;
						}
					break;
			}

		/*   4-bytes little endian sample origin */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetOrigin(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 1 start point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopStart1(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 1 end point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopEnd1(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 2 start point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopStart2(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 2 end point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopEnd2(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 3 start point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopStart3(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian loop 3 end point */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetLoopEnd3(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-bytes little endian sampling rate */
		/*       should be between 100 and 65535 */
		if (!WriteBufferedSignedLongLittleEndian(Output,
			AlgoSampObjectGetSamplingRate(AlgoSampObj)))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian natural frequency fractional portion */
		/*       unsigned; divide by 2^32 to get the actual fraction */
		NaturalFreqTemp = AlgoSampObjectGetNaturalFrequency(AlgoSampObj);
		if (!WriteBufferedUnsignedLongLittleEndian(Output,
			(unsigned long)((NaturalFreqTemp - (long)NaturalFreqTemp) * 4294967296.0L)))
			{
				return eFileLoadDiskError;
			}

		/*   4-byte little endian natural frequency integer portion */
		/*       total natural frequency should be between 0.01 and 1e6 */
		if (!WriteBufferedSignedLongLittleEndian(Output,(long)NaturalFreqTemp))
			{
				return eFileLoadDiskError;
			}

		return eFileLoadNoError;
	}


/* mark the object as saved */
void									AlgoSampObjectMarkAsSaved(AlgoSampObjectRec* AlgoSampObj)
	{
		CheckPtrExistence(AlgoSampObj);
		if (AlgoSampObj->AlgoSampWindow != NIL)
			{
				AlgoSampWindowWritebackModifiedData(AlgoSampObj->AlgoSampWindow);
			}
		AlgoSampObj->DataModified = False;
	}
