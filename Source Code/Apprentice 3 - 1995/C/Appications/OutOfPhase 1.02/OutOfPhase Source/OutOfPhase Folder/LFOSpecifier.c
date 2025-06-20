/* LFOSpecifier.c */
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

#include "LFOSpecifier.h"
#include "Memory.h"
#include "Envelope.h"
#include "SampleSelector.h"


struct LFOSpecRec
	{
		/* envelope that determines the frequency of the LFO oscillator */
		EnvelopeRec*					FrequencyEnvelope;

		/* envelope that determines the amplitude of the LFO wave */
		EnvelopeRec*					AmplitudeEnvelope;

		/* envelope that determines wave table index -- wave table LFOs only */
		EnvelopeRec*					WaveTableIndexEnvelope;

		/* sample / wave table sources */
		SampleSelectorRec*		SampleSources;

		/* what kind of wave generator should we use */
		LFOOscTypes						Oscillator;

		/* how should the LFO wave affect the stream being modulated */
		LFOModulationTypes		ModulationMode;

		/* how are the signals combined */
		LFOAdderMode					AddingMode;
	};


/* create a new LFO specification record */
LFOSpecRec*						NewLFOSpecifier(void)
	{
		LFOSpecRec*					LFOSpec;

		LFOSpec = (LFOSpecRec*)AllocPtrCanFail(sizeof(LFOSpecRec),"LFOSpecRec");
		if (LFOSpec == NIL)
			{
			 FailurePoint1:
				return NIL;
			}
		LFOSpec->FrequencyEnvelope = NewEnvelope();
		if (LFOSpec->FrequencyEnvelope == NIL)
			{
			 FailurePoint2:
				ReleasePtr((char*)LFOSpec);
				goto FailurePoint1;
			}
		LFOSpec->AmplitudeEnvelope = NewEnvelope();
		if (LFOSpec->AmplitudeEnvelope == NIL)
			{
			 FailurePoint3:
				DisposeEnvelope(LFOSpec->FrequencyEnvelope);
				goto FailurePoint2;
			}
		LFOSpec->WaveTableIndexEnvelope = NewEnvelope();
		if (LFOSpec->WaveTableIndexEnvelope == NIL)
			{
			 FailurePoint4:
				DisposeEnvelope(LFOSpec->AmplitudeEnvelope);
				goto FailurePoint3;
			}
		LFOSpec->SampleSources = NewSampleSelectorList(0);
		if (LFOSpec->SampleSources == NIL)
			{
			 FailurePoint5:
				DisposeEnvelope(LFOSpec->WaveTableIndexEnvelope);
				goto FailurePoint4;
			}
		LFOSpec->Oscillator = eLFOSignedSine;
		LFOSpec->ModulationMode = eLFOAdditive;
		LFOSpec->AddingMode = eLFOArithmetic;
		return LFOSpec;
	}


/* dispose an LFO specification */
void									DisposeLFOSpecifier(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		DisposeSampleSelectorList(LFOSpec->SampleSources);
		DisposeEnvelope(LFOSpec->WaveTableIndexEnvelope);
		DisposeEnvelope(LFOSpec->FrequencyEnvelope);
		DisposeEnvelope(LFOSpec->AmplitudeEnvelope);
		ReleasePtr((char*)LFOSpec);
	}


/* get the frequency envelope record */
EnvelopeRec*					GetLFOSpecFrequencyEnvelope(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->FrequencyEnvelope;
	}


/* get the amplitude envelope record */
struct EnvelopeRec*		GetLFOSpecAmplitudeEnvelope(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->AmplitudeEnvelope;
	}


/* get the oscillator type for this LFO specifier */
LFOOscTypes						LFOSpecGetOscillatorType(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->Oscillator;
	}


/* change the oscillator type */
void									SetLFOSpecOscillatorType(LFOSpecRec* LFOSpec, LFOOscTypes NewType)
	{
		CheckPtrExistence(LFOSpec);
		LFOSpec->Oscillator = NewType;
	}


/* get the oscillator modulation mode */
LFOModulationTypes		LFOSpecGetModulationMode(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->ModulationMode;
	}


/* change the oscillator modulation mode */
void									SetLFOSpecModulationMode(LFOSpecRec* LFOSpec,
												LFOModulationTypes NewType)
	{
		CheckPtrExistence(LFOSpec);
		LFOSpec->ModulationMode = NewType;
	}


/* find out what the adding mode of the LFO is */
LFOAdderMode					LFOSpecGetAddingMode(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->AddingMode;
	}


/* set a new adding mode for the LFO */
void									SetLFOSpecAddingMode(LFOSpecRec* LFOSpec,
												LFOAdderMode NewAddingMode)
	{
		CheckPtrExistence(LFOSpec);
		LFOSpec->AddingMode = NewAddingMode;
	}


/* for wave table lfo oscillators only */
struct EnvelopeRec*		GetLFOSpecWaveTableIndexEnvelope(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->WaveTableIndexEnvelope;
	}


/* get the sample selector list */
struct SampleSelectorRec*	GetLFOSpecSampleSelector(LFOSpecRec* LFOSpec)
	{
		CheckPtrExistence(LFOSpec);
		return LFOSpec->SampleSources;
	}
