/* OscillatorSpecifier.c */
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

#include "OscillatorSpecifier.h"
#include "Memory.h"
#include "SampleSelector.h"
#include "ModulationSpecifier.h"
#include "Envelope.h"
#include "LFOListSpecifier.h"


struct OscillatorRec
	{
		/* what is the name by which we can refer to this oscillator */
		char*											OscillatorName;

		/* what kind of oscillator is it */
		OscillatorTypes						OscillatorType;

		/* mapping from pitch to sample */
		struct SampleSelectorRec*	SampleIntervalList;

		/* list of oscillators that are modulating us and how */
		struct ModulationSpecRec*	ModulatorInputList;

		/* this scales our output so that we can be set relative to other oscillators */
		OscillatorNumType					OutputLoudness;

		/* these are used to make our frequency a multiple of the instrument's */
		/* overall frequency */
		OscillatorNumType					FrequencyAdjustMultiplier;
		long											FrequencyAdjustDivisor;
		OscillatorNumType					FrequencyAdjustAdder;

		/* this switch turns on or off our output, so that we can be used only for */
		/* modulation, if desired.  (True = do use, False = don't use) */
		MyBoolean									MixInFinalOutput;

		/* this envelope determines the total output level with time */
		struct EnvelopeRec*				LoudnessEnvelope;

		/* this LFO list modulates the output of the loudness envelope */
		struct LFOListSpecRec*		LoudnessLFOList;

		/* this envelope determines the wave table selection index with time.  this is */
		/* only used for wave table synthesis, not for sampling */
		struct EnvelopeRec*				ExcitationEnvelope;

		/* this LFO list modulates the output of the excitation envelope.  it is only */
		/* used for wave table synthesis. */
		struct LFOListSpecRec*		ExcitationLFOList;

		/* stereo bias -- fixed amount to move this oscillator left or right by */
		OscillatorNumType					StereoBias;
		/* surround bias -- fixed amount to move this oscillator front or back by */
		OscillatorNumType					SurroundBias;

		/* time displacement -- how much earlier / later to start sample */
		OscillatorNumType					TimeDisplacement;
	};


/* create a new oscillator structure */
OscillatorRec*						NewOscillatorSpecifier(void)
	{
		OscillatorRec*					Osc;

		Osc = (OscillatorRec*)AllocPtrCanFail(sizeof(OscillatorRec),"OscillatorRec");
		if (Osc == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		Osc->OscillatorType = eOscillatorSampled; /* default -- this is kinda ugly */
		Osc->OscillatorName = AllocPtrCanFail(0,"OscillatorName");
		if (Osc->OscillatorName == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)Osc);
				goto FailurePoint1;
			}
		Osc->SampleIntervalList = NewSampleSelectorList(0);
		if (Osc->SampleIntervalList == NIL)
			{
			 FailurePoint3:
				ReleasePtr(Osc->OscillatorName);
				goto FailurePoint2;
			}
		Osc->ModulatorInputList = NewModulationSpecifier();
		if (Osc->ModulatorInputList == NIL)
			{
			 FailurePoint4:
				DisposeSampleSelectorList(Osc->SampleIntervalList);
				goto FailurePoint3;
			}
		Osc->OutputLoudness = 0;
		Osc->FrequencyAdjustMultiplier = 1;
		Osc->FrequencyAdjustDivisor = 1;
		Osc->FrequencyAdjustAdder = 0;
		Osc->MixInFinalOutput = True;
		Osc->LoudnessEnvelope = NewEnvelope();
		if (Osc->LoudnessEnvelope == NIL)
			{
			 FailurePoint5:
				DisposeModulationSpecifier(Osc->ModulatorInputList);
				goto FailurePoint4;
			}
		Osc->LoudnessLFOList = NewLFOListSpecifier();
		if (Osc->LoudnessLFOList == NIL)
			{
			 FailurePoint6:
				DisposeEnvelope(Osc->LoudnessEnvelope);
				goto FailurePoint5;
			}
		Osc->ExcitationEnvelope = NewEnvelope();
		if (Osc->ExcitationEnvelope == NIL)
			{
			 FailurePoint7:
				DisposeLFOListSpecifier(Osc->LoudnessLFOList);
				goto FailurePoint6;
			}
		Osc->ExcitationLFOList = NewLFOListSpecifier();
		if (Osc->ExcitationLFOList == NIL)
			{
			 FailurePoint8:
				DisposeEnvelope(Osc->ExcitationEnvelope);
				goto FailurePoint7;
			}
		Osc->StereoBias = 0;
		Osc->TimeDisplacement = 0;
		return Osc;
	}


/* dispose of an oscillator structure */
void											DisposeOscillatorSpecifier(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		ReleasePtr(Osc->OscillatorName);
		DisposeSampleSelectorList(Osc->SampleIntervalList);
		DisposeModulationSpecifier(Osc->ModulatorInputList);
		DisposeEnvelope(Osc->LoudnessEnvelope);
		DisposeLFOListSpecifier(Osc->LoudnessLFOList);
		DisposeEnvelope(Osc->ExcitationEnvelope);
		DisposeLFOListSpecifier(Osc->ExcitationLFOList);
		ReleasePtr((char*)Osc);
	}


/* get the actual name of the oscillator.  don't dispose this! */
char*											OscillatorGetName(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->OscillatorName;
	}


/* put a new oscillator name in.  the object becomes owner of the name block */
void											PutOscillatorName(OscillatorRec* Osc, char* NewName)
	{
		CheckPtrExistence(Osc);
		CheckPtrExistence(NewName);
		ReleasePtr(Osc->OscillatorName);
		Osc->OscillatorName = NewName;
	}


/* set the oscillator type */
void											OscillatorSetTheType(OscillatorRec* Osc,
														OscillatorTypes WhatKindOfOscillator)
	{
		ERROR((WhatKindOfOscillator != eOscillatorSampled)
			&& (WhatKindOfOscillator != eOscillatorWaveTable),PRERR(ForceAbort,
			"NewOscillatorSpecifier:  bad oscillator type number"));
		CheckPtrExistence(Osc);
		Osc->OscillatorType = WhatKindOfOscillator;
	}


/* find out what kind of oscillator this is */
OscillatorTypes						OscillatorGetWhatKindItIs(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->OscillatorType;
	}


/* get the pitch interval --> sample mapping */
struct SampleSelectorRec*	OscillatorGetSampleIntervalList(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->SampleIntervalList;
	}


/* get the list of oscillators that are modulating us */
struct ModulationSpecRec*	OscillatorGetModulatorInputList(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->ModulatorInputList;
	}


/* get the output loudness of the oscillator */
OscillatorNumType					OscillatorGetOutputLoudness(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->OutputLoudness;
	}


/* put a new output loudness in for the oscillator */
void											PutOscillatorNewOutputLoudness(OscillatorRec* Osc,
														double NewOutputLevel)
	{
		CheckPtrExistence(Osc);
		Osc->OutputLoudness = NewOutputLevel;
	}


/* get the frequency multiplier factor */
OscillatorNumType					OscillatorGetFrequencyMultiplier(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->FrequencyAdjustMultiplier;
	}


/* get the frequency divisor integer */
long											OscillatorGetFrequencyDivisor(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->FrequencyAdjustDivisor;
	}


/* get the frequency adder thing */
OscillatorNumType					OscillatorGetFrequencyAdder(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->FrequencyAdjustAdder;
	}


/* change the frequency adjust factors */
void											PutOscillatorNewFrequencyFactors(OscillatorRec* Osc,
														double NewMultipler, long NewDivisor)
	{
		CheckPtrExistence(Osc);
		Osc->FrequencyAdjustMultiplier = NewMultipler;
		Osc->FrequencyAdjustDivisor = NewDivisor;
	}


/* put a new frequency adder value */
void											PutOscillatorFrequencyAdder(OscillatorRec* Osc,
														double NewAdder)
	{
		CheckPtrExistence(Osc);
		Osc->FrequencyAdjustAdder = NewAdder;
	}


/* find out if output of this oscillator is to be included in final output */
MyBoolean									IncludeOscillatorInFinalOutput(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->MixInFinalOutput;
	}


/* change whether or not the oscillator is being included in the final output */
void											PutOscillatorIncludeInOutputFlag(OscillatorRec* Osc,
														MyBoolean IncludeInOutputFlag)
	{
		CheckPtrExistence(Osc);
		Osc->MixInFinalOutput = IncludeInOutputFlag;
	}


/* get the loudness envelope for the oscillator */
struct EnvelopeRec*				OscillatorGetLoudnessEnvelope(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->LoudnessEnvelope;
	}


/* get the list of LFO oscillators modulating the loudness envelope output */
struct LFOListSpecRec*		OscillatorGetLoudnessLFOList(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->LoudnessLFOList;
	}


/* get the excitation envelope for the oscillator */
struct EnvelopeRec*				OscillatorGetExcitationEnvelope(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->ExcitationEnvelope;
	}


/* get the list of LFO oscillators modulating the excitation envelope output */
struct LFOListSpecRec*		OscillatorGetExcitationLFOList(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->ExcitationLFOList;
	}


/* resolve modulator named references to oscillators */
MyBoolean									ResolveOscillatorModulators(OscillatorRec* Osc,
														struct OscillatorListRec* ListOfOscillators)
	{
		CheckPtrExistence(Osc);
		CheckPtrExistence(ListOfOscillators);
		return ResolveModulationReferences(Osc->ModulatorInputList,ListOfOscillators);
	}


/* get the stereo bias factor */
OscillatorNumType					OscillatorGetStereoBias(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->StereoBias;
	}


/* put a new value for the stereo bias factor */
void											OscillatorPutStereoBias(OscillatorRec* Osc,
														OscillatorNumType NewStereoBias)
	{
		CheckPtrExistence(Osc);
		Osc->StereoBias = NewStereoBias;
	}


/* get the surround bias factor */
OscillatorNumType					OscillatorGetSurroundBias(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->SurroundBias;
	}


/* put a new value for the surround bias factor */
void											OscillatorPutSurroundBias(OscillatorRec* Osc,
														OscillatorNumType NewSurroundBias)
	{
		CheckPtrExistence(Osc);
		Osc->SurroundBias = NewSurroundBias;
	}


/* get the time displacement factor */
OscillatorNumType					OscillatorGetTimeDisplacement(OscillatorRec* Osc)
	{
		CheckPtrExistence(Osc);
		return Osc->TimeDisplacement;
	}


/* put a new value for the time displacement factor */
void											OscillatorPutTimeDisplacement(OscillatorRec* Osc,
														OscillatorNumType NewTimeDisplacement)
	{
		CheckPtrExistence(Osc);
		Osc->TimeDisplacement = NewTimeDisplacement;
	}
