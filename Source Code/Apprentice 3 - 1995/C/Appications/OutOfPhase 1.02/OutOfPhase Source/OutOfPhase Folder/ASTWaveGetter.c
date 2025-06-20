/* ASTWaveGetter.c */
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

#include "ASTWaveGetter.h"
#include "Memory.h"
#include "TrashTracker.h"


struct ASTWaveGetterRec
	{
		char*						TheString;
		WaveGetterOp		TheOperation;
	};


/* create a new AST wave getter form */
ASTWaveGetterRec*		NewWaveGetter(char* SampleName, WaveGetterOp TheOperation,
											struct TrashTrackRec* TrashTracker, long LineNumber)
	{
		ASTWaveGetterRec*	WaveGetter;

		CheckPtrExistence(SampleName);
		CheckPtrExistence(TrashTracker);

		WaveGetter = (ASTWaveGetterRec*)AllocTrackedBlock(sizeof(ASTWaveGetterRec),
			TrashTracker);
		if (WaveGetter == NIL)
			{
				return NIL;
			}

		WaveGetter->TheString = SampleName;
		WaveGetter->TheOperation = TheOperation;

		return WaveGetter;
	}


/* type check the wave getter node.  this returns eCompileNoError if */
/* everything is ok, and the appropriate type in *ResultingDataType. */
CompileErrors				TypeCheckWaveGetter(DataTypes* ResultingDataType,
											ASTWaveGetterRec* WaveGetter, long* ErrorLineNumber,
											struct TrashTrackRec* TrashTracker)
	{
		CheckPtrExistence(WaveGetter);
		CheckPtrExistence(TrashTracker);

		switch (WaveGetter->TheOperation)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"TypeCheckWaveGetter:  bad type"));
					break;
				case eWaveGetterSampleLeft:
				case eWaveGetterSampleRight:
				case eWaveGetterSampleMono:
				case eWaveGetterWaveArray:
					*ResultingDataType = eArrayOfFixed;
					break;
				case eWaveGetterWaveFrames:
				case eWaveGetterWaveTables:
					*ResultingDataType = eInteger;
					break;
			}

		return eCompileNoError;
	}


/* generate code for a wave getter.  returns True if successful, or False if it fails. */
MyBoolean						CodeGenWaveGetter(struct PcodeRec* FuncCode,
											long* StackDepthParam, ASTWaveGetterRec* WaveGetter)
	{
		CheckPtrExistence(FuncCode);
		CheckPtrExistence(WaveGetter);

		switch (WaveGetter->TheOperation)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"CodeGenWaveGetter:  bad opcode"));
					break;
				case eWaveGetterSampleLeft:
					if (!AddPcodeInstruction(FuncCode,epGetSampleLeftArray,NIL))
						{
							return False;
						}
					break;
				case eWaveGetterSampleRight:
					if (!AddPcodeInstruction(FuncCode,epGetSampleRightArray,NIL))
						{
							return False;
						}
					break;
				case eWaveGetterSampleMono:
					if (!AddPcodeInstruction(FuncCode,epGetSampleMonoArray,NIL))
						{
							return False;
						}
					break;
				case eWaveGetterWaveFrames:
					if (!AddPcodeInstruction(FuncCode,epGetWaveTableFrames,NIL))
						{
							return False;
						}
					break;
				case eWaveGetterWaveTables:
					if (!AddPcodeInstruction(FuncCode,epGetWaveTableTables,NIL))
						{
							return False;
						}
					break;
				case eWaveGetterWaveArray:
					if (!AddPcodeInstruction(FuncCode,epGetWaveTableArray,NIL))
						{
							return False;
						}
					break;
			}
		if (!AddPcodeOperandString(FuncCode,WaveGetter->TheString,
			PtrSize(WaveGetter->TheString)))
			{
				return False;
			}

		*StackDepthParam += 1;

		return True;
	}
