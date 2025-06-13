/* BuildInstrument.c */
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

#include "BuildInstrument.h"
#include "Memory.h"
#include "TrashTracker.h"
#include "CompilerScanner.h"
#include "InstrumentStructure.h"
#include "OscillatorSpecifier.h"
#include "OscillatorListSpecifier.h"
#include "LFOSpecifier.h"
#include "LFOListSpecifier.h"
#include "FixedPoint.h"
#include "DataMunging.h"
#include "Envelope.h"
#include "SampleSelector.h"
#include "ModulationSpecifier.h"




#define OPAREN "("
#define CPAREN ")"




#define INSTRLIST_ONCEONLY_LOUDNESS (1L << 0)

#define INSTRLIST_REQUIRED_LOUDNESS (1L << 0)
#define INSTRLIST_REQUIRED_OSCILLATOR (1L << 1)
#define INSTRLIST_REQUIREDMASK (INSTRLIST_REQUIRED_LOUDNESS\
					| INSTRLIST_REQUIRED_OSCILLATOR)


#define LFODEFINITION_ONCEONLY_FREQENVELOPE (1L << 0)
#define LFODEFINITION_ONCEONLY_AMPENVELOPE (1L << 1)
#define LFODEFINITION_ONCEONLY_OSCILLATORTYPE (1L << 2)
#define LFODEFINITION_ONCEONLY_MODULATIONTYPE (1L << 3)
#define LFODEFINITION_ONCEONLY_ADDINGMODE (1L << 4)

#define LFODEFINITION_REQUIRED_FREQENVELOPE (1L << 0)
#define LFODEFINITION_REQUIRED_AMPENVELOPE (1L << 1)
#define LFODEFINITION_REQUIREDMASK (LFODEFINITION_REQUIRED_FREQENVELOPE\
					| LFODEFINITION_REQUIRED_AMPENVELOPE)


#define OSCILLATORDEFINITION_ONCEONLY_TYPE (1L << 0)
#define OSCILLATORDEFINITION_ONCEONLY_SAMPLELIST (1L << 1)
#define OSCILLATORDEFINITION_ONCEONLY_MODULATORS (1L << 2)
#define OSCILLATORDEFINITION_ONCEONLY_LOUDNESS (1L << 3)
#define OSCILLATORDEFINITION_ONCEONLY_FREQMULTIPLIER (1L << 4)
#define OSCILLATORDEFINITION_ONCEONLY_FREQDIVISOR (1L << 5)
#define OSCILLATORDEFINITION_ONCEONLY_MAKEOUTPUT (1L << 6)
#define OSCILLATORDEFINITION_ONCEONLY_LOUDNESSENVELOPE (1L << 7)
#define OSCILLATORDEFINITION_ONCEONLY_INDEXENVELOPE (1L << 8)
#define OSCILLATORDEFINITION_ONCEONLY_STEREOBIAS (1L << 9)
#define OSCILLATORDEFINITION_ONCEONLY_TIMEDISPLACEMENT (1L << 10)
#define OSCILLATORDEFINITION_ONCEONLY_SURROUNDBIAS (1L << 11)
#define OSCILLATORDEFINITION_ONCEONLY_FREQADDER (1L << 12)

#define OSCILLATORDEFINITION_REQUIRED_TYPE (1L << 0)
#define OSCILLATORDEFINITION_REQUIRED_LOUDNESS (1L << 1)
#define OSCILLATORDEFINITION_REQUIRED_LOUDNESSENVELOPE (1L << 2)
#define OSCILLATORDEFINITION_REQUIRED_SAMPLELIST (1L << 3)

#define OSCILLATORDEFINITION_REQUIREDMASK (OSCILLATORDEFINITION_REQUIRED_TYPE\
					| OSCILLATORDEFINITION_REQUIRED_LOUDNESS\
					| OSCILLATORDEFINITION_REQUIRED_LOUDNESSENVELOPE\
					| OSCILLATORDEFINITION_REQUIRED_SAMPLELIST)


#define ENVELOPEDEFINITION_ONCEONLY_TOTALSCALING (1L << 1)
#define ENVELOPEDEFINITION_ONCEONLY_POINTS (1L << 2)

#define ENVELOPEDEFINITION_REQUIREDMASK (0L)


#define ENVPOINTDEFINITION_ONCEONLY_AMPACCENT1 (1L << 0)
#define ENVPOINTDEFINITION_ONCEONLY_AMPACCENT2 (1L << 1)
#define ENVPOINTDEFINITION_ONCEONLY_AMPACCENT3 (1L << 2)
#define ENVPOINTDEFINITION_ONCEONLY_AMPACCENT4 (1L << 3)
#define ENVPOINTDEFINITION_ONCEONLY_AMPFREQ (1L << 4)
#define ENVPOINTDEFINITION_ONCEONLY_RATEACCENT1 (1L << 5)
#define ENVPOINTDEFINITION_ONCEONLY_RATEACCENT2 (1L << 6)
#define ENVPOINTDEFINITION_ONCEONLY_RATEACCENT3 (1L << 7)
#define ENVPOINTDEFINITION_ONCEONLY_RATEACCENT4 (1L << 8)
#define ENVPOINTDEFINITION_ONCEONLY_RATEFREQ (1L << 9)
#define ENVPOINTDEFINITION_ONCEONLY_CURVE (1L << 10)




/* prototypes */
static BuildInstrErrors			ParseInstrDefinition(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseInstrList(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseInstrElem(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseNumber(ScannerRec* Scanner, long* ErrorLine,
															double* NumberOut);
static BuildInstrErrors			ParseLfoDefinition(LFOSpecRec* LFO, ScannerRec* Scanner,
															long* ErrorLine, unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseIdentifier(ScannerRec* Scanner, long* ErrorLine,
															char** IdentifierOut);
static BuildInstrErrors			ParseOscillatorDefinition(OscillatorRec* Oscillator,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseLfoElem(LFOSpecRec* LFO, ScannerRec* Scanner,
															long* ErrorLine, unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseOscillatorElem(OscillatorRec* Oscillator,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseEnvelopeDefinition(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions);
static BuildInstrErrors			ParseSamplelistDefinition(SampleSelectorRec* SampleList,
															ScannerRec* Scanner, long* ErrorLine);
static BuildInstrErrors			ParseModulatorList(ModulationSpecRec* ModulatorList,
															ScannerRec* Scanner, long* ErrorLine,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseEnvelopeElem(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions);
static BuildInstrErrors			ParseSamplelistElem(SampleSelectorRec* SampleList,
															ScannerRec* Scanner, long* ErrorLine);
static BuildInstrErrors			ParseModulatorElem(ModulationSpecRec* ModulatorList,
															ScannerRec* Scanner, long* ErrorLine,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);
static BuildInstrErrors			ParseEnvPointList(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine);
static BuildInstrErrors			ParseEnvPointElem(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine);
static BuildInstrErrors			ParseEnvAttributes(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long OnceOnly);
static BuildInstrErrors			ParseEnvOneAttribute(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long* OnceOnlyDefinitions);
static BuildInstrErrors			ParseModulationDynamic(LFOListSpecRec** LFOListOut,
															EnvelopeRec** EnvelopeOut, ScannerRec* Scanner,
															long* ErrorLine, struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList);




/* token enumeration definitions */
typedef enum
	{
		eKeywordInstrument EXECUTE(= -1274),
		eKeywordLoudness,
		eKeywordFrequencylfo,
		eKeywordOscillator,
		eKeywordFreqenvelope,
		eKeywordModulation,
		eKeywordConstant,
		eKeywordSignsine,
		eKeywordPlussine,
		eKeywordSigntriangle,
		eKeywordPlustriangle,
		eKeywordSignsquare,
		eKeywordPlussquare,
		eKeywordSignramp,
		eKeywordPlusramp,
		eKeywordSignlinfuzz,
		eKeywordPluslinfuzz,
		eKeywordAdditive,
		eKeywordMultiplicative,
		eKeywordInversemult,
		eKeywordType,
		eKeywordSampled,
		eKeywordWavetable,
		eKeywordSamplelist,
		eKeywordModulators,
		eKeywordFreqmultiplier,
		eKeywordFreqdivisor,
		eKeywordFreqadder,
		eKeywordMakeoutput,
		eKeywordTrue,
		eKeywordFalse,
		eKeywordLoudnessenvelope,
		eKeywordTotalscaling,
		eKeywordExponential,
		eKeywordLinear,
		eKeywordLevel,
		eKeywordDelay,
		eKeywordSustainpoint,
		eKeywordReleasepoint,
		eKeywordSustainpointnoskip,
		eKeywordReleasepointnoskip,
		eKeywordAmpaccent1,
		eKeywordAmpaccent2,
		eKeywordAmpaccent3,
		eKeywordAmpaccent4,
		eKeywordAmpfreq,
		eKeywordRateaccent1,
		eKeywordRateaccent2,
		eKeywordRateaccent3,
		eKeywordRateaccent4,
		eKeywordRatefreq,
		eKeywordSource,
		eKeywordScale,
		eKeywordOriginadjust,
		eKeywordPhasegen,
		eKeywordOutput,
		eKeywordAmpenvelope,
		eKeywordLoudnesslfo,
		eKeywordIndexenvelope,
		eKeywordIndexlfo,
		eKeywordPoints,
		eKeywordTarget,
		eKeywordOrigin,
		eKeywordLfo,
		eKeywordEnvelope,
		eKeywordStereobias,
		eKeywordDisplacement,
		eKeywordSurroundbias
	} KeywordType;




/* take a block of text and parse it into an instrument definition.  it returns an */
/* error code.  if an error occurs, then *InstrOut is invalid, otherwise it will */
/* be valid.  the text file remains unaltered.  *ErrorLine is numbered from 1. */
BuildInstrErrors						BuildInstrumentFromText(char* TextFile, long* ErrorLine,
															struct InstrumentRec** InstrOut,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TrashTrackRec*						TrashTracker;
		ScannerRec*								Scanner;
		InstrumentRec*						Instrument;
		BuildInstrErrors					Error;
		TokenRec*									Token;

		CheckPtrExistence(TextFile);
		CheckPtrExistence(SampleList);
		CheckPtrExistence(AlgoSampList);
		CheckPtrExistence(WaveTableList);
		CheckPtrExistence(AlgoWaveTableList);
		EXECUTE(*InstrOut = (InstrumentRec*)0x81818181;)
		EXECUTE(*ErrorLine = 0x81818181;)

		TrashTracker = NewTrashTracker();
		if (TrashTracker == NIL)
			{
			 NoMemoryFailurePoint1:
				*ErrorLine = 1;
				return eBuildInstrOutOfMemory;
			}

		Scanner = NewScanner(TrashTracker,TextFile);
		if (Scanner == NIL)
			{
			 NoMemoryFailurePoint2:
				goto NoMemoryFailurePoint1;
			}

		Instrument = NewInstrumentSpecifier();
		if (Instrument == NIL)
			{
			 NoMemoryFailurePoint3:
				goto NoMemoryFailurePoint2;
			}

		AddKeywordToScanner(Scanner,"instrument",eKeywordInstrument);
		AddKeywordToScanner(Scanner,"loudness",eKeywordLoudness);
		AddKeywordToScanner(Scanner,"frequencylfo",eKeywordFrequencylfo);
		AddKeywordToScanner(Scanner,"oscillator",eKeywordOscillator);
		AddKeywordToScanner(Scanner,"freqenvelope",eKeywordFreqenvelope);
		AddKeywordToScanner(Scanner,"modulation",eKeywordModulation);
		AddKeywordToScanner(Scanner,"constant",eKeywordConstant);
		AddKeywordToScanner(Scanner,"signsine",eKeywordSignsine);
		AddKeywordToScanner(Scanner,"plussine",eKeywordPlussine);
		AddKeywordToScanner(Scanner,"signtriangle",eKeywordSigntriangle);
		AddKeywordToScanner(Scanner,"plustriangle",eKeywordPlustriangle);
		AddKeywordToScanner(Scanner,"signsquare",eKeywordSignsquare);
		AddKeywordToScanner(Scanner,"plussquare",eKeywordPlussquare);
		AddKeywordToScanner(Scanner,"signramp",eKeywordSignramp);
		AddKeywordToScanner(Scanner,"plusramp",eKeywordPlusramp);
		AddKeywordToScanner(Scanner,"signlinfuzz",eKeywordSignlinfuzz);
		AddKeywordToScanner(Scanner,"pluslinfuzz",eKeywordPluslinfuzz);
		AddKeywordToScanner(Scanner,"additive",eKeywordAdditive);
		AddKeywordToScanner(Scanner,"multiplicative",eKeywordMultiplicative);
		AddKeywordToScanner(Scanner,"inversemult",eKeywordInversemult);
		AddKeywordToScanner(Scanner,"type",eKeywordType);
		AddKeywordToScanner(Scanner,"sampled",eKeywordSampled);
		AddKeywordToScanner(Scanner,"wavetable",eKeywordWavetable);
		AddKeywordToScanner(Scanner,"samplelist",eKeywordSamplelist);
		AddKeywordToScanner(Scanner,"modulators",eKeywordModulators);
		AddKeywordToScanner(Scanner,"freqmultiplier",eKeywordFreqmultiplier);
		AddKeywordToScanner(Scanner,"freqdivisor",eKeywordFreqdivisor);
		AddKeywordToScanner(Scanner,"freqadder",eKeywordFreqadder);
		AddKeywordToScanner(Scanner,"makeoutput",eKeywordMakeoutput);
		AddKeywordToScanner(Scanner,"true",eKeywordTrue);
		AddKeywordToScanner(Scanner,"false",eKeywordFalse);
		AddKeywordToScanner(Scanner,"loudnessenvelope",eKeywordLoudnessenvelope);
		AddKeywordToScanner(Scanner,"totalscaling",eKeywordTotalscaling);
		AddKeywordToScanner(Scanner,"exponential",eKeywordExponential);
		AddKeywordToScanner(Scanner,"linear",eKeywordLinear);
		AddKeywordToScanner(Scanner,"level",eKeywordLevel);
		AddKeywordToScanner(Scanner,"delay",eKeywordDelay);
		AddKeywordToScanner(Scanner,"sustainpoint",eKeywordSustainpoint);
		AddKeywordToScanner(Scanner,"releasepoint",eKeywordReleasepoint);
		AddKeywordToScanner(Scanner,"sustainpointnoskip",eKeywordSustainpointnoskip);
		AddKeywordToScanner(Scanner,"releasepointnoskip",eKeywordReleasepointnoskip);
		AddKeywordToScanner(Scanner,"ampaccent1",eKeywordAmpaccent1);
		AddKeywordToScanner(Scanner,"ampaccent2",eKeywordAmpaccent2);
		AddKeywordToScanner(Scanner,"ampaccent3",eKeywordAmpaccent3);
		AddKeywordToScanner(Scanner,"ampaccent4",eKeywordAmpaccent4);
		AddKeywordToScanner(Scanner,"ampfreq",eKeywordAmpfreq);
		AddKeywordToScanner(Scanner,"rateaccent1",eKeywordRateaccent1);
		AddKeywordToScanner(Scanner,"rateaccent2",eKeywordRateaccent2);
		AddKeywordToScanner(Scanner,"rateaccent3",eKeywordRateaccent3);
		AddKeywordToScanner(Scanner,"rateaccent4",eKeywordRateaccent4);
		AddKeywordToScanner(Scanner,"ratefreq",eKeywordRatefreq);
		AddKeywordToScanner(Scanner,"source",eKeywordSource);
		AddKeywordToScanner(Scanner,"scale",eKeywordScale);
		AddKeywordToScanner(Scanner,"originadjust",eKeywordOriginadjust);
		AddKeywordToScanner(Scanner,"phasegen",eKeywordPhasegen);
		AddKeywordToScanner(Scanner,"output",eKeywordOutput);
		AddKeywordToScanner(Scanner,"ampenvelope",eKeywordAmpenvelope);
		AddKeywordToScanner(Scanner,"loudnesslfo",eKeywordLoudnesslfo);
		AddKeywordToScanner(Scanner,"indexenvelope",eKeywordIndexenvelope);
		AddKeywordToScanner(Scanner,"indexlfo",eKeywordIndexlfo);
		AddKeywordToScanner(Scanner,"points",eKeywordPoints);
		AddKeywordToScanner(Scanner,"target",eKeywordTarget);
		AddKeywordToScanner(Scanner,"origin",eKeywordOrigin);
		AddKeywordToScanner(Scanner,"lfo",eKeywordLfo);
		AddKeywordToScanner(Scanner,"envelope",eKeywordEnvelope);
		AddKeywordToScanner(Scanner,"stereobias",eKeywordStereobias);
		AddKeywordToScanner(Scanner,"displacement",eKeywordDisplacement);
		AddKeywordToScanner(Scanner,"surroundbias",eKeywordSurroundbias);

		Error = ParseInstrDefinition(Instrument,Scanner,ErrorLine,SampleList,AlgoSampList,
			WaveTableList,AlgoWaveTableList);
		if (Error != eBuildInstrNoError)
			{
				DisposeInstrumentSpecification(Instrument);
				DisposeTrashTracker(TrashTracker);
				return Error;
			}
		if (!ResolveInstrOscillatorModulations(Instrument))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				DisposeInstrumentSpecification(Instrument);
				DisposeTrashTracker(TrashTracker);
				return eBuildInstrUnresolvedOscillatorReferences;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				DisposeInstrumentSpecification(Instrument);
				DisposeTrashTracker(TrashTracker);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenEndOfInput)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				DisposeInstrumentSpecification(Instrument);
				DisposeTrashTracker(TrashTracker);
				return eBuildInstrUnexpectedInput;
			}

		DisposeTrashTracker(TrashTracker);

		*InstrOut = Instrument;
		return eBuildInstrNoError;
	}




/*    1:   <instr_definition>      ::= instrument ( <instr_list> ) */
/* FIRST SET: */
/*  <instr_definition>      : {instrument} */
/* FOLLOW SET: */
/*  <instr_definition>      : {$$$} */
static BuildInstrErrors			ParseInstrDefinition(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		unsigned long							Required;

		CheckPtrExistence(Instrument);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordInstrument))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedInstrument;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedOpenParen;
			}

		Required = 0;
		Error = ParseInstrList(Instrument,Scanner,ErrorLine,&Required,0,SampleList,
			AlgoSampList,WaveTableList,AlgoWaveTableList);
		if (Error != eBuildInstrNoError)
			{
				return Error;
			}
		if (Required != INSTRLIST_REQUIREDMASK)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrSomeRequiredInstrParamsMissing;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedCloseParen;
			}

		return eBuildInstrNoError;
	}




/*    2:   <instr_list>            ::= <instr_elem> ; <instr_list> */
/*    3:                           ::=  */
/* FIRST SET: */
/*  <instr_list>            : {loudness, frequencylfo, oscillator, <instr_elem>} */
/* FOLLOW SET: */
/*  <instr_list>            : {)} */
static BuildInstrErrors			ParseInstrList(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Instrument);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseInstrElem(Instrument,Scanner,ErrorLine,RequiredDefinitions,
					&OnceOnlyDefinitions,SampleList,AlgoSampList,WaveTableList,AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseInstrList(Instrument,Scanner,ErrorLine,RequiredDefinitions,
					OnceOnlyDefinitions,SampleList,AlgoSampList,WaveTableList,AlgoWaveTableList);
			}
		EXECUTE(PRERR(ForceAbort,"ParseInstrList:  control reached end of function"));
	}




/*    4:   <instr_elem>            ::= loudness <number> */
/*    5:                           ::= frequencylfo ( <lfo_definition> ) */
/*    6:                           ::= oscillator <identifier> ( <oscillator_definition> ) */
/* FIRST SET: */
/*  <instr_elem>            : {loudness, frequencylfo, oscillator} */
/* FOLLOW SET: */
/*  <instr_elem>            : {;} */
static BuildInstrErrors			ParseInstrElem(InstrumentRec* Instrument,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct SampleListRec* SampleList,
															struct AlgoSampListRec* AlgoSampList,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Instrument);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedInstrumentMember;
			}

		if (GetTokenKeywordTag(Token) == eKeywordLoudness)
			{
				double										Number;

				if ((*OnceOnlyDefinitions & INSTRLIST_ONCEONLY_LOUDNESS) != 0)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrMultipleInstrLoudness;
					}
				Error = ParseNumber(Scanner,ErrorLine,&Number);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}
				InstrumentSetOverallLoudness(Instrument,Number);
				*OnceOnlyDefinitions |= INSTRLIST_ONCEONLY_LOUDNESS;
				*RequiredDefinitions |= INSTRLIST_REQUIRED_LOUDNESS;
			}
		else if (GetTokenKeywordTag(Token) == eKeywordFrequencylfo)
			{
				unsigned long							LFORequired;
				LFOSpecRec*								LFO;

				/* allocate the LFO */
				LFO = NewLFOSpecifier();
				if (LFO == NIL)
					{
					 LFOAllocFailurePoint1:
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (!LFOListSpecAppendNewEntry(GetInstrumentFrequencyLFOList(Instrument),LFO))
					{
					 LFOAllocFailurePoint2:
						DisposeLFOSpecifier(LFO);
						goto LFOAllocFailurePoint1;
					}

				/* open paren */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenOpenParen)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedOpenParen;
					}

				/* parse the low frequency operator */
				LFORequired = 0;
				Error = ParseLfoDefinition(LFO,Scanner,ErrorLine,&LFORequired,0,WaveTableList,
					AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}
				if (LFORequired != LFODEFINITION_REQUIREDMASK)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrSomeRequiredLFOParamsMissing;
					}

				/* close paren */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenCloseParen)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedCloseParen;
					}
			}
		else if (GetTokenKeywordTag(Token) == eKeywordOscillator)
			{
				char*											Identifier;
				unsigned long							OscillatorRequired;
				OscillatorRec*						Oscillator;

				/* allocate a new oscillator & add it to the instrument */
				Oscillator = NewOscillatorSpecifier();
				if (Oscillator == NIL)
					{
					 OscillatorAllocFailurePoint1:
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (!AppendOscillatorToList(GetInstrumentOscillatorList(Instrument),Oscillator))
					{
					 OscillatorAllocFailurePoint2:
						DisposeOscillatorSpecifier(Oscillator);
						goto OscillatorAllocFailurePoint1;
					}

				/* get the oscillator's name */
				Error = ParseIdentifier(Scanner,ErrorLine,&Identifier);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}
				CheckPtrExistence(Identifier);
				PutOscillatorName(Oscillator,Identifier);

				/* eat the open paren */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenOpenParen)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedOpenParen;
					}

				/* parse oscillator */
				OscillatorRequired = 0;
				Error = ParseOscillatorDefinition(Oscillator,Scanner,ErrorLine,
					&OscillatorRequired,0,WaveTableList,AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}
				if (OscillatorRequired != OSCILLATORDEFINITION_REQUIREDMASK)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrSomeRequiredOscillatorParamsMissing;
					}

				/* resolve sample/wave table data references */
				switch (OscillatorGetWhatKindItIs(Oscillator))
					{
						default:
							EXECUTE(PRERR(ForceAbort,"ParseInstrElem:  bad oscillator type"));
							break;
						case eOscillatorSampled:
							if (!ResolveSamplesInSampleList(OscillatorGetSampleIntervalList(
								Oscillator),SampleList,AlgoSampList))
								{
									*ErrorLine = GetCurrentLineNumber(Scanner);
									return eBuildInstrSomeSamplesDontExist;
								}
							break;
						case eOscillatorWaveTable:
							if (!ResolveWaveTablesInSampleList(OscillatorGetSampleIntervalList(
								Oscillator),WaveTableList,AlgoWaveTableList))
								{
									*ErrorLine = GetCurrentLineNumber(Scanner);
									return eBuildInstrSomeWaveTablesDontExist;
								}
							break;
					}

				/* eat the close paren */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenCloseParen)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedCloseParen;
					}

				*RequiredDefinitions |= INSTRLIST_REQUIRED_OSCILLATOR;
			}
		else
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedInstrumentMember;
			}

		return eBuildInstrNoError;
	}




/*   80:   <number>                ::= <integertoken> */
/*   81:                           ::= <floattoken> */
/* FIRST SET: */
/*  <number>                : {<integertoken>, <floattoken>} */
/* FOLLOW SET: */
/*  <number>                : {<integertoken>, <floattoken>, ), ;, type, */
/*       exponential, linear, to, sustainpoint, ampaccent1, ampaccent2, */
/*       ampaccent3, ampaccent4, ampfreq, rateaccent1, rateaccent2, rateaccent3, */
/*       rateaccent4, ratefreq, originadjust, <number>, <env_point_list>, */
/*       <env_point_elem>, <env_attributes>, <env_one_attribute>} */
static BuildInstrErrors			ParseNumber(ScannerRec* Scanner, long* ErrorLine,
															double* NumberOut)
	{
		TokenRec*									Token;
		MyBoolean									Negative;

		CheckPtrExistence(Scanner);

		Negative = False;

	 GetAnotherToken:
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		switch (GetTokenType(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedNumber;
				case eTokenInteger:
					*NumberOut = GetTokenIntegerValue(Token);
					break;
				case eTokenSingle:
					*NumberOut = GetTokenSingleValue(Token);
					break;
				case eTokenDouble:
					*NumberOut = GetTokenDoubleValue(Token);
					break;
				case eTokenFixed:
					*NumberOut = largefixed2double(GetTokenFixedValue(Token));
					break;
				case eTokenMinus:
					Negative = !Negative;
					goto GetAnotherToken;
			}

		if (Negative)
			{
				*NumberOut = - *NumberOut;
			}

		return eBuildInstrNoError;
	}




/*    7:   <lfo_definition>        ::= <lfo_elem> ; <lfo_definition> */
/*    8:                           ::=  */
/* FIRST SET: */
/*  <lfo_definition>        : {oscillator, freqenvelope, modulation, */
/*       ampenvelope, <lfo_elem>} */
/* FOLLOW SET: */
/*  <lfo_definition>        : {)} */
static BuildInstrErrors			ParseLfoDefinition(LFOSpecRec* LFO, ScannerRec* Scanner,
															long* ErrorLine, unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(LFO);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseLfoElem(LFO,Scanner,ErrorLine,RequiredDefinitions,
					&OnceOnlyDefinitions,WaveTableList,AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseLfoDefinition(LFO,Scanner,ErrorLine,RequiredDefinitions,
					OnceOnlyDefinitions,WaveTableList,AlgoWaveTableList);
			}

		EXECUTE(PRERR(ForceAbort,"ParseLfoDefinition:  control reached end of function"));
	}




/* the identifier string returned from here is NOT trash-tracker allocated. */
/*   78:   <identifier>            ::= <identifiertoken> */
/*   79:                           ::= <stringtoken> */
/* FIRST SET: */
/*  <identifier>            : {<identifiertoken>, <stringtoken>} */
/* FOLLOW SET: */
/*  <identifier>            : {<integertoken>, <floattoken>, (, scale, */
/*       <number>} */
static BuildInstrErrors			ParseIdentifier(ScannerRec* Scanner, long* ErrorLine,
															char** IdentifierOut)
	{
		TokenRec*									Token;

		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenIdentifier)
			{
				*IdentifierOut = CopyPtr(GetTokenIdentifierString(Token));
			}
		else if (GetTokenType(Token) == eTokenString)
			{
				*IdentifierOut = CopyPtr(GetTokenStringValue(Token));
			}
		else
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedStringOrIdentifier;
			}

		if (*IdentifierOut == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		SetTag(*IdentifierOut,"BuildInstr: ParseIdentifier");

		return eBuildInstrNoError;
	}




/*   27:   <oscillator_definition> ::= <oscillator_elem> ; */
/*       <oscillator_definition> */
/*   28:                           ::=  */
/* FIRST SET: */
/*  <oscillator_definition> : {loudness, type, samplelist, modulators, */
/*       freqmultiplier, freqdivisor, makeoutput, loudnessenvelope, */
/*       loudnesslfo, indexenvelope, indexlfo, <oscillator_elem>} */
/* FOLLOW SET: */
/*  <oscillator_definition> : {)} */
static BuildInstrErrors			ParseOscillatorDefinition(OscillatorRec* Oscillator,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Oscillator);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseOscillatorElem(Oscillator,Scanner,ErrorLine,RequiredDefinitions,
					&OnceOnlyDefinitions,WaveTableList,AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseOscillatorDefinition(Oscillator,Scanner,ErrorLine,
					RequiredDefinitions,OnceOnlyDefinitions,WaveTableList,AlgoWaveTableList);
			}

		EXECUTE(PRERR(ForceAbort,"ParseOscillatorDefinition:  control reached end of function"));
	}




/*    9:   <lfo_elem>              ::= freqenvelope ( <envelope_definition> ) */
/*   10:                           ::= ampenvelope ( <envelope_definition> ) */
/*   11:                           ::= oscillator <oscillator_type> */
/*   12:                           ::= modulation <modulation_type> */
/* FIRST SET: */
/*  <lfo_elem>              : {oscillator, freqenvelope, modulation, ampenvelope} */
/* FOLLOW SET: */
/*  <lfo_elem>              : {;} */
static BuildInstrErrors			ParseLfoElem(LFOSpecRec* LFO, ScannerRec* Scanner,
															long* ErrorLine, unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		unsigned long							Required;

		CheckPtrExistence(LFO);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedLFOMember;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedLFOMember;

				case eKeywordFreqenvelope:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_FREQENVELOPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOFreqEnvelope;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Required = 0;
					Error = ParseEnvelopeDefinition(GetLFOSpecFrequencyEnvelope(LFO),
						Scanner,ErrorLine,&Required,0);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrSomeRequiredEnvelopeParamsMissing;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*RequiredDefinitions |= LFODEFINITION_REQUIRED_FREQENVELOPE;
					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_FREQENVELOPE;
					break;

				case eKeywordAmpenvelope:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_AMPENVELOPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOAmpEnvelope;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Required = 0;
					Error = ParseEnvelopeDefinition(GetLFOSpecAmplitudeEnvelope(LFO),
						Scanner,ErrorLine,&Required,0);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrSomeRequiredEnvelopeParamsMissing;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*RequiredDefinitions |= LFODEFINITION_REQUIRED_AMPENVELOPE;
					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_AMPENVELOPE;
					break;

				case eKeywordOscillator:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_OSCILLATORTYPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOOscillatorType;
						}

					/*  13:   <oscillator_type>       ::= constant */
					/*  14:                           ::= signsine */
					/*  15:                           ::= plussine */
					/*  16:                           ::= signtriangle */
					/*  17:                           ::= plustriangle */
					/*  18:                           ::= signsquare */
					/*  19:                           ::= plussquare */
					/*  20:                           ::= signramp */
					/*  21:                           ::= plusramp */
					/*  22:                           ::= signlinfuzz */
					/*  23:                           ::= pluslinfuzz */
					/*  XX:                           ::= wavetable ( <samplelist_definition> */
					/*                                    ) ( <envelope_definition> ) */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenKeyword)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedLFOOscillatorType;
						}
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedLFOOscillatorType;
							case eKeywordConstant:
								SetLFOSpecOscillatorType(LFO,eLFOConstant1);
								break;
							case eKeywordSignsine:
								SetLFOSpecOscillatorType(LFO,eLFOSignedSine);
								break;
							case eKeywordPlussine:
								SetLFOSpecOscillatorType(LFO,eLFOPositiveSine);
								break;
							case eKeywordSigntriangle:
								SetLFOSpecOscillatorType(LFO,eLFOSignedTriangle);
								break;
							case eKeywordPlustriangle:
								SetLFOSpecOscillatorType(LFO,eLFOPositiveTriangle);
								break;
							case eKeywordSignsquare:
								SetLFOSpecOscillatorType(LFO,eLFOSignedSquare);
								break;
							case eKeywordPlussquare:
								SetLFOSpecOscillatorType(LFO,eLFOPositiveSquare);
								break;
							case eKeywordSignramp:
								SetLFOSpecOscillatorType(LFO,eLFOSignedRamp);
								break;
							case eKeywordPlusramp:
								SetLFOSpecOscillatorType(LFO,eLFOPositiveRamp);
								break;
							case eKeywordSignlinfuzz:
								SetLFOSpecOscillatorType(LFO,eLFOSignedLinearFuzz);
								break;
							case eKeywordPluslinfuzz:
								SetLFOSpecOscillatorType(LFO,eLFOPositiveLinearFuzz);
								break;


							case eKeywordWavetable:
								SetLFOSpecOscillatorType(LFO,eLFOWaveTable);

								/* open paren */
								Token = GetNextToken(Scanner);
								if (Token == NIL)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrOutOfMemory;
									}
								if (GetTokenType(Token) != eTokenOpenParen)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrExpectedOpenParen;
									}

								Error = ParseSamplelistDefinition(GetLFOSpecSampleSelector(LFO),
									Scanner,ErrorLine);
								if (Error != eBuildInstrNoError)
									{
										return Error;
									}
								if (!ResolveWaveTablesInSampleList(GetLFOSpecSampleSelector(LFO),
									WaveTableList,AlgoWaveTableList))
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrSomeWaveTablesDontExist;
									}

								/* close paren */
								Token = GetNextToken(Scanner);
								if (Token == NIL)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrOutOfMemory;
									}
								if (GetTokenType(Token) != eTokenCloseParen)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrExpectedCloseParen;
									}

								/* open paren */
								Token = GetNextToken(Scanner);
								if (Token == NIL)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrOutOfMemory;
									}
								if (GetTokenType(Token) != eTokenOpenParen)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrExpectedOpenParen;
									}

								Required = 0;
								Error = ParseEnvelopeDefinition(GetLFOSpecWaveTableIndexEnvelope(LFO),
									Scanner,ErrorLine,&Required,0);
								if (Error != eBuildInstrNoError)
									{
										return Error;
									}
								if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrSomeRequiredEnvelopeParamsMissing;
									}

								/* close paren */
								Token = GetNextToken(Scanner);
								if (Token == NIL)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrOutOfMemory;
									}
								if (GetTokenType(Token) != eTokenCloseParen)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrExpectedCloseParen;
									}
								break;
						}

					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_OSCILLATORTYPE;
					break;

				case eKeywordModulation:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_MODULATIONTYPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOModulationType;
						}

					/*  24:   <modulation_type>       ::= additive */
					/*  25:                           ::= multiplicative */
					/*  26:                           ::= inversemult */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenKeyword)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedLFOModulationType;
						}
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedLFOModulationType;
							case eKeywordAdditive:
								SetLFOSpecModulationMode(LFO,eLFOAdditive);
								break;
							case eKeywordMultiplicative:
								SetLFOSpecModulationMode(LFO,eLFOMultiplicative);
								break;
							case eKeywordInversemult:
								SetLFOSpecModulationMode(LFO,eLFOInverseMultiplicative);
								break;
						}

					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_MODULATIONTYPE;
					break;

				case eKeywordLinear:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_ADDINGMODE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOAddingMode;
						}

					SetLFOSpecAddingMode(LFO,eLFOArithmetic);

					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_ADDINGMODE;
					break;

				case eKeywordExponential:
					if ((*OnceOnlyDefinitions & LFODEFINITION_ONCEONLY_ADDINGMODE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleLFOAddingMode;
						}

					SetLFOSpecAddingMode(LFO,eLFOGeometric);

					*OnceOnlyDefinitions |= LFODEFINITION_ONCEONLY_ADDINGMODE;
					break;
			}

		return eBuildInstrNoError;
	}




/*   29:   <oscillator_elem>       ::= type <oscillator_type> */
/*   30:                           ::= samplelist ( <samplelist_definition> ) */
/*   31:                           ::= modulators ( <modulator_list> ) */
/*   32:                           ::= loudness <number> */
/*   33:                           ::= freqmultiplier <number> */
/*   34:                           ::= freqdivisor <integer> */
/*   35:                           ::= makeoutput <boolean> */
/*   36:                           ::= loudnessenvelope ( <envelope_definition> ) */
/*   37:                           ::= loudnesslfo ( <lfo_definition> ) */
/*   38:                           ::= indexenvelope ( <envelope_definition> ) */
/*   39:                           ::= indexlfo ( <lfo_definition> ) */
/*   XXX:                          ::= stereobias <number> */
/*   XXX:                          ::= displacement <number> */
/* FIRST SET: */
/*  <oscillator_elem>       : {loudness, type, samplelist, modulators, */
/*       freqmultiplier, freqdivisor, makeoutput, loudnessenvelope, */
/*       loudnesslfo, indexenvelope, indexlfo} */
/* FOLLOW SET: */
/*  <oscillator_elem>       : {;} */
static BuildInstrErrors			ParseOscillatorElem(OscillatorRec* Oscillator,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		double										Number;
		unsigned long							Required;

		CheckPtrExistence(Oscillator);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedOscillatorMember;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedOscillatorMember;

				case eKeywordType:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_TYPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscType;
						}

					/*  40:   <oscillator_type>       ::= sampled */
					/*  41:                           ::= wavetable */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenKeyword)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOscType;
						}
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedOscType;
							case eKeywordSampled:
								OscillatorSetTheType(Oscillator,eOscillatorSampled);
								break;
							case eKeywordWavetable:
								OscillatorSetTheType(Oscillator,eOscillatorWaveTable);
								break;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_TYPE;
					*RequiredDefinitions |= OSCILLATORDEFINITION_REQUIRED_TYPE;
					break;

				case eKeywordSamplelist:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_SAMPLELIST) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscSampleList;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Error = ParseSamplelistDefinition(OscillatorGetSampleIntervalList(Oscillator),
						Scanner,ErrorLine);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_SAMPLELIST;
					*RequiredDefinitions |= OSCILLATORDEFINITION_REQUIRED_SAMPLELIST;
					break;

				case eKeywordModulators:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_MODULATORS) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscModulators;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Error = ParseModulatorList(OscillatorGetModulatorInputList(Oscillator),
						Scanner,ErrorLine,WaveTableList,AlgoWaveTableList);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_MODULATORS;
					break;

				case eKeywordLoudness:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_LOUDNESS) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscLoudness;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					PutOscillatorNewOutputLoudness(Oscillator,Number);

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_LOUDNESS;
					*RequiredDefinitions |= OSCILLATORDEFINITION_REQUIRED_LOUDNESS;
					break;

				case eKeywordFreqmultiplier:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_FREQMULTIPLIER) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscFreqMultiplier;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					PutOscillatorNewFrequencyFactors(Oscillator,Number,
						OscillatorGetFrequencyDivisor(Oscillator));

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_FREQMULTIPLIER;
					break;

				case eKeywordFreqdivisor:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_FREQDIVISOR) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscFreqDivisor;
						}

					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenInteger)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedInteger;
						}
					PutOscillatorNewFrequencyFactors(Oscillator,
						OscillatorGetFrequencyMultiplier(Oscillator),GetTokenIntegerValue(Token));

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_FREQDIVISOR;
					break;

				case eKeywordFreqadder:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_FREQADDER) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInserMultipleOscFreqAdder;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					PutOscillatorFrequencyAdder(Oscillator,Number);

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_FREQADDER;
					break;

				case eKeywordMakeoutput:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_MAKEOUTPUT) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscMakeOutput;
						}

					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenKeyword)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedBoolean;
						}
					switch (GetTokenKeywordTag(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedBoolean;
							case eKeywordTrue:
								PutOscillatorIncludeInOutputFlag(Oscillator,True);
								break;
							case eKeywordFalse:
								PutOscillatorIncludeInOutputFlag(Oscillator,False);
								break;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_MAKEOUTPUT;
					break;

				case eKeywordLoudnessenvelope:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_LOUDNESSENVELOPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscLoudnessEnvelope;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Required = 0;
					Error = ParseEnvelopeDefinition(OscillatorGetLoudnessEnvelope(Oscillator),
						Scanner,ErrorLine,&Required,0);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrSomeRequiredEnvelopeParamsMissing;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_LOUDNESSENVELOPE;
					*RequiredDefinitions |= OSCILLATORDEFINITION_REQUIRED_LOUDNESSENVELOPE;
					break;

				case eKeywordLoudnesslfo:
					{
						LFOSpecRec*					LFO;

						/* create the LFO */
						LFO = NewLFOSpecifier();
						if (LFO == NIL)
							{
							 LoudnessLFOAllocFailurePoint1:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (!LFOListSpecAppendNewEntry(OscillatorGetLoudnessLFOList(Oscillator),LFO))
							{
							 LoudnessLFOAllocFailurePoint2:
								DisposeLFOSpecifier(LFO);
								goto LoudnessLFOAllocFailurePoint1;
							}

						/* open paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenOpenParen)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedOpenParen;
							}

						Required = 0;
						Error = ParseLfoDefinition(LFO,Scanner,ErrorLine,&Required,0,
							WaveTableList,AlgoWaveTableList);
						if (Error != eBuildInstrNoError)
							{
								return Error;
							}
						if (Required != LFODEFINITION_REQUIREDMASK)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrSomeRequiredEnvelopeParamsMissing;
							}

						/* close paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenCloseParen)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedCloseParen;
							}
					}
					break;

				case eKeywordIndexenvelope:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_INDEXENVELOPE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscIndexEnvelope;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Required = 0;
					Error = ParseEnvelopeDefinition(OscillatorGetExcitationEnvelope(Oscillator),
						Scanner,ErrorLine,&Required,0);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrSomeRequiredEnvelopeParamsMissing;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_INDEXENVELOPE;
					break;

				case eKeywordIndexlfo:
					{
						LFOSpecRec*					LFO;

						/* create the LFO */
						LFO = NewLFOSpecifier();
						if (LFO == NIL)
							{
							 IndexLFOAllocFailurePoint1:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (!LFOListSpecAppendNewEntry(OscillatorGetExcitationLFOList(Oscillator),LFO))
							{
							 IndexLFOAllocFailurePoint2:
								DisposeLFOSpecifier(LFO);
								goto IndexLFOAllocFailurePoint1;
							}

						/* open paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenOpenParen)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedOpenParen;
							}

						Required = 0;
						Error = ParseLfoDefinition(LFO,Scanner,ErrorLine,&Required,0,
							WaveTableList,AlgoWaveTableList);
						if (Error != eBuildInstrNoError)
							{
								return Error;
							}
						if (Required != LFODEFINITION_REQUIREDMASK)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrSomeRequiredEnvelopeParamsMissing;
							}

						/* close paren */
						Token = GetNextToken(Scanner);
						if (Token == NIL)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrOutOfMemory;
							}
						if (GetTokenType(Token) != eTokenCloseParen)
							{
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedCloseParen;
							}
					}
					break;

				case eKeywordStereobias:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_STEREOBIAS) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscStereoBias;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					OscillatorPutStereoBias(Oscillator,Number);

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_STEREOBIAS;
					break;

				case eKeywordDisplacement:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_TIMEDISPLACEMENT) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscDisplacement;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					OscillatorPutTimeDisplacement(Oscillator,Number);

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_TIMEDISPLACEMENT;
					break;

				case eKeywordSurroundbias:
					if ((*OnceOnlyDefinitions & OSCILLATORDEFINITION_ONCEONLY_SURROUNDBIAS) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleOscSurroundBias;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					OscillatorPutSurroundBias(Oscillator,Number);

					*OnceOnlyDefinitions |= OSCILLATORDEFINITION_ONCEONLY_SURROUNDBIAS;
					break;
			}

		return eBuildInstrNoError;
	}




/*   42:   <envelope_definition>   ::= <envelope_elem> ; <envelope_definition> */
/*   43:                           ::=  */
/* FIRST SET: */
/*  <envelope_definition>   : {totalscaling, points, <envelope_elem>} */
/* FOLLOW SET: */
/*  <envelope_definition>   : {)} */
static BuildInstrErrors			ParseEnvelopeDefinition(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long* ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long OnceOnlyDefinitions)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseEnvelopeElem(Envelope,Scanner,ErrorLine,RequiredDefinitions,
					&OnceOnlyDefinitions);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseEnvelopeDefinition(Envelope,Scanner,ErrorLine,RequiredDefinitions,
					OnceOnlyDefinitions);
			}

		EXECUTE(PRERR(ForceAbort,"ParseEnvelopeDefinition:  control reached end of function"));
	}




/*   68:   <samplelist_definition> ::= <samplelist_elem> ; */
/*       <samplelist_definition> */
/*   69:                           ::=  */
/* FIRST SET: */
/*  <samplelist_definition> : {<identifiertoken>, <stringtoken>, */
/*       <identifier>, <samplelist_elem>} */
/* FOLLOW SET: */
/*  <samplelist_definition> : {)} */
static BuildInstrErrors			ParseSamplelistDefinition(SampleSelectorRec* SampleList,
															ScannerRec* Scanner, long* ErrorLine)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(SampleList);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseSamplelistElem(SampleList,Scanner,ErrorLine);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseSamplelistDefinition(SampleList,Scanner,ErrorLine);
			}

		EXECUTE(PRERR(ForceAbort,"ParseSamplelistDefinition:  control reached end of function"));
	}




/*   71:   <modulator_list>        ::= <modulator_elem> ; <modulator_list> */
/*   72:                           ::=  */
/* FIRST SET: */
/*  <modulator_list>        : {source, <modulator_elem>} */
/* FOLLOW SET: */
/*  <modulator_list>        : {)} */
static BuildInstrErrors			ParseModulatorList(ModulationSpecRec* ModulatorList,
															ScannerRec* Scanner, long* ErrorLine,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(ModulatorList);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseModulatorElem(ModulatorList,Scanner,ErrorLine,WaveTableList,
					AlgoWaveTableList);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseModulatorList(ModulatorList,Scanner,ErrorLine,WaveTableList,
					AlgoWaveTableList);
			}

		EXECUTE(PRERR(ForceAbort,"ParseModulatorList:  control reached end of function"));
	}




/*   45:   <envelope_elem>         ::= totalscaling <number> */
/*   49:                           ::= points ( <env_point_list> ) */
/* FIRST SET: */
/*  <envelope_elem>         : {totalscaling, points} */
/* FOLLOW SET: */
/*  <envelope_elem>         : {;} */
static BuildInstrErrors			ParseEnvelopeElem(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long* RequiredDefinitions,
															unsigned long* OnceOnlyDefinitions)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		double										Number;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedEnvelopeMember;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedEnvelopeMember;

				case eKeywordTotalscaling:
					if ((*OnceOnlyDefinitions & ENVELOPEDEFINITION_ONCEONLY_TOTALSCALING) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvTotalScaling;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetOverallAmplitude(Envelope,Number);

					*OnceOnlyDefinitions |= ENVELOPEDEFINITION_ONCEONLY_TOTALSCALING;
					break;

				case eKeywordPoints:
					if ((*OnceOnlyDefinitions & ENVELOPEDEFINITION_ONCEONLY_POINTS) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPoints;
						}

					/* open paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenOpenParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedOpenParen;
						}

					Error = ParseEnvPointList(Envelope,Scanner,ErrorLine);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}

					/* close paren */
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenType(Token) != eTokenCloseParen)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedCloseParen;
						}

					*OnceOnlyDefinitions |= ENVELOPEDEFINITION_ONCEONLY_POINTS;
					break;
			}

		return eBuildInstrNoError;
	}




/*   70:   <samplelist_elem>       ::= <identifier> <number> */
/* FIRST SET: */
/*  <samplelist_elem>       : {<identifiertoken>, <stringtoken>, <identifier>} */
/* FOLLOW SET: */
/*  <samplelist_elem>       : {;} */
static BuildInstrErrors			ParseSamplelistElem(SampleSelectorRec* SampleList,
															ScannerRec* Scanner, long* ErrorLine)
	{
		BuildInstrErrors					Error;
		char*											SampleName;
		double										Number;

		CheckPtrExistence(SampleList);
		CheckPtrExistence(Scanner);

		Error = ParseIdentifier(Scanner,ErrorLine,&SampleName);
		if (Error != eBuildInstrNoError)
			{
				return Error;
			}

		Error = ParseNumber(Scanner,ErrorLine,&Number);
		if (Error != eBuildInstrNoError)
			{
				ReleasePtr(SampleName);
				return Error;
			}

		if (!AppendSampleSelector(SampleList,Number,SampleName))
			{
				ReleasePtr(SampleName);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		return eBuildInstrNoError;
	}




/*   73:   <modulator_elem>        ::= source <identifier> scale <modulation_dynamic> */
/*       originadjust <modulation_dynamic> type <modulator_types> target <target_type> */
/* FIRST SET: */
/*  <modulator_elem>        : {source} */
/* FOLLOW SET: */
/*  <modulator_elem>        : {;} */
static BuildInstrErrors			ParseModulatorElem(ModulationSpecRec* ModulatorList,
															ScannerRec* Scanner, long* ErrorLine,
															struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		char*											SourceOscillatorName;
		EnvelopeRec*							ScalingFactorEnvelope;
		LFOListSpecRec*						ScalingFactorLFOList;
		EnvelopeRec*							OriginAdjustEnvelope;
		LFOListSpecRec*						OriginAdjustLFOList;
		ModulationTypes						Modulation;
		ModDestTypes							Destination;

		CheckPtrExistence(ModulatorList);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint1:
				return Error;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordSource))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedSource;
			 FailurePoint2:
				goto FailurePoint1;
			}

		Error = ParseIdentifier(Scanner,ErrorLine,&SourceOscillatorName);
		if (Error != eBuildInstrNoError)
			{
			 FailurePoint3:
				goto FailurePoint2;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint4:
				ReleasePtr(SourceOscillatorName);
				goto FailurePoint3;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordScale))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedScale;
			 FailurePoint5:
				goto FailurePoint4;
			}

		Error = ParseModulationDynamic(&ScalingFactorLFOList,&ScalingFactorEnvelope,
			Scanner,ErrorLine,WaveTableList,AlgoWaveTableList);
		if (Error != eBuildInstrNoError)
			{
			 FailurePoint6:
				goto FailurePoint5;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint7:
				DisposeEnvelope(ScalingFactorEnvelope);
				DisposeLFOListSpecifier(ScalingFactorLFOList);
				goto FailurePoint6;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordOriginadjust))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedOriginadjust;
			 FailurePoint8:
				goto FailurePoint7;
			}

		Error = ParseModulationDynamic(&OriginAdjustLFOList,&OriginAdjustEnvelope,
			Scanner,ErrorLine,WaveTableList,AlgoWaveTableList);
		if (Error != eBuildInstrNoError)
			{
			 FailurePoint9:
				goto FailurePoint8;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint10:
				DisposeEnvelope(OriginAdjustEnvelope);
				DisposeLFOListSpecifier(OriginAdjustLFOList);
				goto FailurePoint9;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordType))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedType;
			 FailurePoint11:
				goto FailurePoint10;
			}

		/*  74:   <modulator_types>       ::= additive */
		/*  75:                           ::= multiplicative */
		/*  XX:                           ::= inversemult */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint12:
				goto FailurePoint11;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedModulationTypeSpecifier;
			 FailurePoint13:
				goto FailurePoint12;
			}
		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					Error = eBuildInstrExpectedModulationTypeSpecifier;
				 FailurePoint14:
					goto FailurePoint13;
				case eKeywordAdditive:
					Modulation = eModulationAdditive;
					break;
				case eKeywordMultiplicative:
					Modulation = eModulationMultiplicative;
					break;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint15:
				goto FailurePoint14;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordTarget))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedTarget;
			 FailurePoint16:
				goto FailurePoint15;
			}

		/*  76:   <target_type>           ::= phasegen */
		/*  77:                           ::= output */
		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint17:
				goto FailurePoint16;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrExpectedModulationTargetSpecifier;
			 FailurePoint18:
				goto FailurePoint17;
			}
		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					Error = eBuildInstrExpectedModulationTargetSpecifier;
				 FailurePoint19:
					goto FailurePoint18;
				case eKeywordPhasegen:
					Destination = eModulatePhaseGen;
					break;
				case eKeywordOutput:
					Destination = eModulateOutput;
					break;
			}

		if (!AppendModulationSpecEntry(ModulatorList,Modulation,Destination,
			ScalingFactorEnvelope,ScalingFactorLFOList,OriginAdjustEnvelope,
			OriginAdjustLFOList,SourceOscillatorName))
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				Error = eBuildInstrOutOfMemory;
			 FailurePoint20:
				goto FailurePoint19;
			}

		return eBuildInstrNoError;
	}




/*   50:   <env_point_list>        ::= <env_point_elem> ; <env_point_list> */
/*   51:                           ::=  */
/* FIRST SET: */
/*  <env_point_list>        : {<integertoken>, <floattoken>, <number>, */
/*       <env_point_elem>} */
/* FOLLOW SET: */
/*  <env_point_list>        : {)} */
static BuildInstrErrors			ParseEnvPointList(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenCloseParen)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseEnvPointElem(Envelope,Scanner,ErrorLine);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				/* get semicolon */
				Token = GetNextToken(Scanner);
				if (Token == NIL)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrOutOfMemory;
					}
				if (GetTokenType(Token) != eTokenSemicolon)
					{
						*ErrorLine = GetCurrentLineNumber(Scanner);
						return eBuildInstrExpectedSemicolon;
					}

				return ParseEnvPointList(Envelope,Scanner,ErrorLine);
			}

		EXECUTE(PRERR(ForceAbort,"ParseEnvPointList:  control reached end of function"));
	}




/*   52:   <env_point_elem>        ::= delay <number> level <number> <env_attributes> */
/*   XX:                           ::= origin */
/* FIRST SET: */
/*  <env_point_elem>        : {delay} */
/* FOLLOW SET: */
/*  <env_point_elem>        : {<integertoken>, <floattoken>, ), <number>, */
/*       <env_point_list>, <env_point_elem>} */
static BuildInstrErrors			ParseEnvPointElem(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		double										Delay;
		double										Level;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedDelayOrOrigin;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedDelayOrOrigin;

				case eKeywordDelay:
					Error = ParseNumber(Scanner,ErrorLine,&Delay);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}

					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if ((GetTokenType(Token) != eTokenKeyword)
						|| ((GetTokenKeywordTag(Token) != eKeywordLevel)
							&& (GetTokenKeywordTag(Token) != eKeywordScale)))
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedLevelOrScale;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Level);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}

					if (!EnvelopeInsertPhase(Envelope,GetEnvelopeNumFrames(Envelope)))
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrOutOfMemory;
						}
					if (GetTokenKeywordTag(Token) == eKeywordScale)
						{
							EnvelopeSetPhaseTargetType(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
								eEnvelopeTargetScaling);
						}
					 else
						{
							EnvelopeSetPhaseTargetType(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
								eEnvelopeTargetAbsolute);
						}
					EnvelopeSetPhaseDuration(Envelope,GetEnvelopeNumFrames(Envelope) - 1,Delay);
					EnvelopeSetPhaseFinalValue(Envelope,GetEnvelopeNumFrames(Envelope) - 1,Level);

					return ParseEnvAttributes(Envelope,Scanner,ErrorLine,0);

				case eKeywordOrigin:
					EnvelopeSetOrigin(Envelope,GetEnvelopeNumFrames(Envelope));
					return eBuildInstrNoError;
			}
		EXECUTE(PRERR(ForceAbort,"ParseEnvPointElem:  Control reached end of function"));
	}




/*   50:   <env_attributes>        ::= <env_one_attribute> <env_attributes> */
/*   51:                           ::=  */
/* FIRST SET: */
/*  <env_attributes>        : {exponential, linear, sustainpoint, */
/*       ampaccent1, ampaccent2, ampaccent3, ampaccent4, ampfreq, rateaccent1, */
/*       rateaccent2, rateaccent3, rateaccent4, ratefreq, <env_one_attribute>} */
/* FOLLOW SET: */
/*  <env_attributes>        : {;} */
static BuildInstrErrors			ParseEnvAttributes(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long OnceOnly)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		if (GetTokenType(Token) == eTokenSemicolon)
			{
				UngetToken(Scanner,Token);
				return eBuildInstrNoError;
			}
		 else
			{
				UngetToken(Scanner,Token);

				Error = ParseEnvOneAttribute(Envelope,Scanner,ErrorLine,&OnceOnly);
				if (Error != eBuildInstrNoError)
					{
						return Error;
					}

				return ParseEnvAttributes(Envelope,Scanner,ErrorLine,OnceOnly);
			}

		EXECUTE(PRERR(ForceAbort,"ParseEnvAttributes:  control reached end of function"));
	}




/*   52:   <env_one_attribute>     ::= sustainpoint <integertoken> */
/*   53:                           ::= ampaccent1 <number> */
/*   54:                           ::= ampaccent2 <number> */
/*   55:                           ::= ampaccent3 <number> */
/*   56:                           ::= ampaccent4 <number> */
/*   57:                           ::= ampfreq <number> <number> */
/*   58:                           ::= rateaccent1 <number> */
/*   59:                           ::= rateaccent2 <number> */
/*   60:                           ::= rateaccent3 <number> */
/*   61:                           ::= rateaccent4 <number> */
/*   62:                           ::= ratefreq <number> <number> */
/*   63:                           ::= exponential */
/*   64:                           ::= linear */
/* FIRST SET: */
/*  <env_one_attribute>     : {exponential, linear, sustainpoint, */
/*       ampaccent1, ampaccent2, ampaccent3, ampaccent4, ampfreq, */
/*       rateaccent1, rateaccent2, rateaccent3, rateaccent4, ratefreq} */
/* FOLLOW SET: */
/*  <env_one_attribute>     : {;, exponential, linear, */
/*       sustainpoint, ampaccent1, ampaccent2, ampaccent3, ampaccent4, */
/*       ampfreq, rateaccent1, rateaccent2, rateaccent3, rateaccent4, */
/*       ratefreq, <env_attributes>, <env_one_attribute>} */
static BuildInstrErrors			ParseEnvOneAttribute(EnvelopeRec* Envelope,
															ScannerRec* Scanner, long *ErrorLine,
															unsigned long* OnceOnlyDefinitions)
	{
		TokenRec*									Token;
		BuildInstrErrors					Error;
		double										Number;

		CheckPtrExistence(Envelope);
		CheckPtrExistence(Scanner);

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrNoError;
			}
		if (GetTokenType(Token) != eTokenKeyword)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedEnvPointMember;
			}

		switch (GetTokenKeywordTag(Token))
			{
				default:
					*ErrorLine = GetCurrentLineNumber(Scanner);
					return eBuildInstrExpectedEnvPointMember;

				case eKeywordSustainpoint:
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrNoError;
						}
					if (GetTokenType(Token) != eTokenInteger)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedInteger;
						}
					switch (GetTokenIntegerValue(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedIntBetween1And3;
							case 1:
								if (GetEnvelopeReleasePoint1(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint1(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointSkip);
								break;
							case 2:
								if (GetEnvelopeReleasePoint2(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint2(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointSkip);
								break;
							case 3:
								if (GetEnvelopeReleasePoint3(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint3(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointSkip);
								break;
						}
					break;

				case eKeywordReleasepoint:
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrNoError;
						}
					if (GetTokenType(Token) != eTokenInteger)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedInteger;
						}
					switch (GetTokenIntegerValue(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedIntBetween1And3;
							case 1:
								if (GetEnvelopeReleasePoint1(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint1(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointSkip);
								break;
							case 2:
								if (GetEnvelopeReleasePoint2(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint2(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointSkip);
								break;
							case 3:
								if (GetEnvelopeReleasePoint3(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint3(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointSkip);
								break;
						}
					break;

				case eKeywordSustainpointnoskip:
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrNoError;
						}
					if (GetTokenType(Token) != eTokenInteger)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedInteger;
						}
					switch (GetTokenIntegerValue(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedIntBetween1And3;
							case 1:
								if (GetEnvelopeReleasePoint1(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint1(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointNoSkip);
								break;
							case 2:
								if (GetEnvelopeReleasePoint2(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint2(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointNoSkip);
								break;
							case 3:
								if (GetEnvelopeReleasePoint3(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint3(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeSustainPointNoSkip);
								break;
						}
					break;

				case eKeywordReleasepointnoskip:
					Token = GetNextToken(Scanner);
					if (Token == NIL)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrNoError;
						}
					if (GetTokenType(Token) != eTokenInteger)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrExpectedInteger;
						}
					switch (GetTokenIntegerValue(Token))
						{
							default:
								*ErrorLine = GetCurrentLineNumber(Scanner);
								return eBuildInstrExpectedIntBetween1And3;
							case 1:
								if (GetEnvelopeReleasePoint1(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint1(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointNoSkip);
								break;
							case 2:
								if (GetEnvelopeReleasePoint2(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint2(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointNoSkip);
								break;
							case 3:
								if (GetEnvelopeReleasePoint3(Envelope) != -1)
									{
										*ErrorLine = GetCurrentLineNumber(Scanner);
										return eBuildInstrEnvSustainPointAlreadyDefined;
									}
								EnvelopeSetReleasePoint3(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
									eEnvelopeReleasePointNoSkip);
								break;
						}
					break;

				case eKeywordAmpaccent1:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_AMPACCENT1) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointAmpAccent1;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent1Amp(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_AMPACCENT1;
					break;

				case eKeywordAmpaccent2:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_AMPACCENT2) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointAmpAccent2;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent2Amp(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_AMPACCENT2;
					break;

				case eKeywordAmpaccent3:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_AMPACCENT3) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointAmpAccent3;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent3Amp(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_AMPACCENT3;
					break;

				case eKeywordAmpaccent4:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_AMPACCENT4) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointAmpAccent4;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent4Amp(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_AMPACCENT4;
					break;

				case eKeywordAmpfreq:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_AMPFREQ) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointAmpFreq;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetFreqAmpRolloff(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetFreqAmpNormalization(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_AMPFREQ;
					break;

				case eKeywordRateaccent1:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_RATEACCENT1) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointRateAccent1;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent1Rate(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_RATEACCENT1;
					break;

				case eKeywordRateaccent2:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_RATEACCENT2) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointRateAccent2;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent2Rate(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_RATEACCENT2;
					break;

				case eKeywordRateaccent3:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_RATEACCENT3) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointRateAccent3;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent3Rate(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_RATEACCENT3;
					break;

				case eKeywordRateaccent4:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_RATEACCENT4) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointRateAccent4;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetAccent4Rate(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_RATEACCENT4;
					break;

				case eKeywordRatefreq:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_RATEFREQ) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointRateFreq;
						}

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetFreqRateRolloff(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					Error = ParseNumber(Scanner,ErrorLine,&Number);
					if (Error != eBuildInstrNoError)
						{
							return Error;
						}
					EnvelopeSetFreqRateNormalization(Envelope,Number,GetEnvelopeNumFrames(Envelope) - 1);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_RATEFREQ;
					break;

				case eKeywordExponential:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_CURVE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointCurveSpec;
						}

					EnvelopeSetPhaseTransitionType(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
						eEnvelopeLinearInDecibels);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_CURVE;
					break;

				case eKeywordLinear:
					if ((*OnceOnlyDefinitions & ENVPOINTDEFINITION_ONCEONLY_CURVE) != 0)
						{
							*ErrorLine = GetCurrentLineNumber(Scanner);
							return eBuildInstrMultipleEnvPointCurveSpec;
						}

					EnvelopeSetPhaseTransitionType(Envelope,GetEnvelopeNumFrames(Envelope) - 1,
						eEnvelopeLinearInAmplitude);

					*OnceOnlyDefinitions |= ENVPOINTDEFINITION_ONCEONLY_CURVE;
					break;
			}

		return eBuildInstrNoError;
	}




/*   XX:   <modulation_dynamic>    ::= envelope ( <envelope_definition> ) */
/*                                     {lfo ( <lfo> )} */
/* FOLLOW SET: */
/*   originadjust, type */
static BuildInstrErrors			ParseModulationDynamic(LFOListSpecRec** LFOListOut,
															EnvelopeRec** EnvelopeOut, ScannerRec* Scanner,
															long* ErrorLine, struct WaveTableListRec* WaveTableList,
															struct AlgoWaveTableListRec* AlgoWaveTableList)
	{
		BuildInstrErrors					Error;
		TokenRec*									Token;
		unsigned long							Required;
		EnvelopeRec*							Envelope;
		LFOListSpecRec*						LFOList;
		LFOSpecRec*								LFO;

		Envelope = NewEnvelope();
		if (Envelope == NIL)
			{
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		LFOList = NewLFOListSpecifier();
		if (LFOList == NIL)
			{
				DisposeEnvelope(Envelope);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordEnvelope))
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedEnvelope;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedOpenParen;
			}

		Required = 0;
		Error = ParseEnvelopeDefinition(Envelope,Scanner,ErrorLine,&Required,0);
		if (Error != eBuildInstrNoError)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				return Error;
			}
		if (Required != ENVELOPEDEFINITION_REQUIREDMASK)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrSomeRequiredEnvelopeParamsMissing;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedCloseParen;
			}

		/* we should have used tail recursion, but that's a pain for an ad-hoc addition */
	 LoopPoint:

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if ((GetTokenType(Token) == eTokenKeyword)
			&& ((GetTokenKeywordTag(Token) == eKeywordOriginadjust)
				|| (GetTokenKeywordTag(Token) == eKeywordType)))
			{
				UngetToken(Scanner,Token);
				*EnvelopeOut = Envelope;
				*LFOListOut = LFOList;
				return eBuildInstrNoError;
			}

		if ((GetTokenType(Token) != eTokenKeyword)
			|| (GetTokenKeywordTag(Token) != eKeywordLfo))
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedLFO;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenOpenParen)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedOpenParen;
			}

		LFO = NewLFOSpecifier();
		if (LFO == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		Required = 0;
		Error = ParseLfoDefinition(LFO,Scanner,ErrorLine,&Required,0,WaveTableList,
			AlgoWaveTableList);
		if (Error != eBuildInstrNoError)
			{
				DisposeLFOSpecifier(LFO);
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				return Error;
			}
		if (Required != LFODEFINITION_REQUIREDMASK)
			{
				DisposeLFOSpecifier(LFO);
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrSomeRequiredLFOParamsMissing;
			}
		if (!LFOListSpecAppendNewEntry(LFOList,LFO))
			{
				DisposeLFOSpecifier(LFO);
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}

		Token = GetNextToken(Scanner);
		if (Token == NIL)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrOutOfMemory;
			}
		if (GetTokenType(Token) != eTokenCloseParen)
			{
				DisposeEnvelope(Envelope);
				DisposeLFOListSpecifier(LFOList);
				*ErrorLine = GetCurrentLineNumber(Scanner);
				return eBuildInstrExpectedCloseParen;
			}

		goto LoopPoint;
	}




/* get a static null terminated string describing the error */
char*												BuildInstrGetErrorMessageText(BuildInstrErrors ErrorCode)
	{
		char*											S EXECUTE(= (char*)0x81818181);

		switch (ErrorCode)
			{
				default:
					EXECUTE(PRERR(ForceAbort,"BuildInstrGetErrorMessageText:  bad error code"));
					break;
				case eBuildInstrOutOfMemory:
					S = "Out of memory";
					break;
				case eBuildInstrUnexpectedInput:
					S = "Unexpected end of text";
					break;
				case eBuildInstrExpectedInstrument:
					S = "Expected 'instrument'";
					break;
				case eBuildInstrExpectedOpenParen:
					S = "Expected '"OPAREN"'";
					break;
				case eBuildInstrExpectedCloseParen:
					S = "Expected '"CPAREN"'";
					break;
				case eBuildInstrSomeRequiredInstrParamsMissing:
					S = "Some required instrument parameters have not been specified";
					break;
				case eBuildInstrExpectedSemicolon:
					S = "Expected ';'";
					break;
				case eBuildInstrExpectedInstrumentMember:
					S = "Expected 'loudness', 'frequencylfo', or 'oscillator'";
					break;
				case eBuildInstrMultipleInstrLoudness:
					S = "Instrument parameter 'loudness' has already been specified";
					break;
				case eBuildInstrSomeRequiredLFOParamsMissing:
					S = "Some required LFO parameters have not been specified";
					break;
				case eBuildInstrSomeRequiredOscillatorParamsMissing:
					S = "Some required oscillator parameters have not been specified";
					break;
				case eBuildInstrExpectedNumber:
					S = "Expected a number";
					break;
				case eBuildInstrExpectedStringOrIdentifier:
					S = "Expected a string or identifier";
					break;
				case eBuildInstrExpectedLFOMember:
					S = "Expected 'freqenvelope', 'ampenvelope', 'oscillator', 'modulation', "
						"'linear', or 'exponential'";
					break;
				case eBuildInstrMultipleLFOFreqEnvelope:
					S = "LFO parameter 'freqenvelope' has already been specified";
					break;
				case eBuildInstrSomeRequiredEnvelopeParamsMissing:
					S = "Some required envelope parameters have not been specified";
					break;
				case eBuildInstrMultipleLFOAmpEnvelope:
					S = "LFO parameter 'ampenvelope' has already been specified";
					break;
				case eBuildInstrMultipleLFOOscillatorType:
					S = "LFO parameter 'oscillator' has already been specified";
					break;
				case eBuildInstrExpectedLFOOscillatorType:
					S = "Expected 'constant', 'signsine', 'plussine', 'signtriangle', "
						"'plustriangle', 'signsquare', 'plussquare', 'signramp', 'plusramp', "
						"'signlinfuzz', or 'pluslinfuzz'";
					break;
				case eBuildInstrMultipleLFOModulationType:
					S = "LFO parameter 'modulation' has already been specified";
					break;
				case eBuildInstrMultipleLFOAddingMode:
					S = "LFO adding mode ('linear' or 'exponential') has already been specified";
					break;
				case eBuildInstrExpectedLFOModulationType:
					S = "Expected 'additive', 'multiplicative', or 'inversemult'";
					break;
				case eBuildInstrExpectedOscillatorMember:
					S = "Expected 'type', 'samplelist', 'modulators', 'loudness', "
						"'freqmultiplier', 'freqdivisor', 'freqadder', 'makeoutput', "
						"'loudnessenvelope', 'loudnesslfo', 'indexenvelope', 'indexlfo', "
						"'stereobias', or 'displacement'";
					break;
				case eBuildInstrMultipleOscType:
					S = "Oscillator parameter 'type' has already been specified";
					break;
				case eBuildInstrMultipleOscSampleList:
					S = "Oscillator parameter 'samplelist' has already been specified";
					break;
				case eBuildInstrMultipleOscModulators:
					S = "Oscillator parameter 'modulators' has already been specified";
					break;
				case eBuildInstrMultipleOscLoudness:
					S = "Oscillator parameter 'loudness' has already been specified";
					break;
				case eBuildInstrMultipleOscFreqMultiplier:
					S = "Oscillator parameter 'freqmultiplier' has already been specified";
					break;
				case eBuildInstrMultipleOscFreqDivisor:
					S = "Oscillator parameter 'freqdivisor' has already been specified";
					break;
				case eBuildInstrMultipleOscMakeOutput:
					S = "Oscillator parameter 'makeoutput' has already been specified";
					break;
				case eBuildInstrMultipleOscLoudnessEnvelope:
					S = "Oscillator parameter 'loudnessenvelope' has already been specified";
					break;
				case eBuildInstrMultipleOscIndexEnvelope:
					S = "Oscillator parameter 'indexenvelope' has already bee specified";
					break;
				case eBuildInstrExpectedOscType:
					S = "Expected 'sampled' or 'wavetable'";
					break;
				case eBuildInstrExpectedInteger:
					S = "Expected an integer";
					break;
				case eBuildInstrExpectedBoolean:
					S = "Expected 'true' or 'false'";
					break;
				case eBuildInstrExpectedEnvelopeMember:
					S = "Expected 'totalscaling' or 'points'";
					break;
				case eBuildInstrMultipleEnvTotalScaling:
					S = "Envelope parameter 'totalscaling' has already been specified";
					break;
				case eBuildInstrMultipleEnvPoints:
					S = "Envelope parameter 'points' has already been specified";
					break;
				case eBuildInstrExpectedSource:
					S = "Expected 'source'";
					break;
				case eBuildInstrExpectedScale:
					S = "Expected 'scale'";
					break;
				case eBuildInstrExpectedOriginadjust:
					S = "Expected 'originadjust'";
					break;
				case eBuildInstrExpectedType:
					S = "Expected 'type'";
					break;
				case eBuildInstrExpectedModulationTypeSpecifier:
					S = "Expected 'additive' or 'multiplicative'";
					break;
				case eBuildInstrExpectedTarget:
					S = "Expected 'target'";
					break;
				case eBuildInstrExpectedModulationTargetSpecifier:
					S = "Expected 'phasegen' or 'output'";
					break;
				case eBuildInstrExpectedDelayOrOrigin:
					S = "Expected 'delay' or 'origin'";
					break;
				case eBuildInstrExpectedLevelOrScale:
					S = "Expected 'level' or 'scale'";
					break;
				case eBuildInstrExpectedEnvPointMember:
					S = "Expected 'sustainpoint', 'sustainpointnoskip', 'releasepoint', "
						"'releasepointnoskip', 'ampaccent1', 'ampaccent2', 'ampaccent3', "
						"'ampaccent4', 'ampfreq', 'rateaccent1', 'rateaccent2', 'rateaccent3', "
						"'rateaccent4', 'ratefreq', 'exponential', or 'linear'";
					break;
				case eBuildInstrExpectedIntBetween1And3:
					S = "Expected an integer in the range [1..3]";
					break;
				case eBuildInstrEnvSustainPointAlreadyDefined:
					S = "That envelope sustain point has already been specified";
					break;
				case eBuildInstrMultipleEnvPointAmpAccent1:
					S = "Envelope parameter 'ampaccent1' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointAmpAccent2:
					S = "Envelope parameter 'ampaccent2' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointAmpAccent3:
					S = "Envelope parameter 'ampaccent3' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointAmpAccent4:
					S = "Envelope parameter 'ampaccent4' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointAmpFreq:
					S = "Envelope parameter 'ampfreq' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointRateAccent1:
					S = "Envelope parameter 'rateaccent1' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointRateAccent2:
					S = "Envelope parameter 'rateaccent2' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointRateAccent3:
					S = "Envelope parameter 'rateaccent3' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointRateAccent4:
					S = "Envelope parameter 'rateaccent4' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointRateFreq:
					S = "Envelope parameter 'ratefreq' has already been specified";
					break;
				case eBuildInstrMultipleEnvPointCurveSpec:
					S = "Envelope parameter 'exponential' or 'linear' has "
						"already been specified";
					break;
				case eBuildInstrUnresolvedOscillatorReferences:
					S = "Some named oscillator references in modulation lists can not be "
						"resolved: the named oscillators do not exist";
					break;
				case eBuildInstrSomeSamplesDontExist:
					S = "Some named samples or algorithmic samples do not exist";
					break;
				case eBuildInstrSomeWaveTablesDontExist:
					S = "Some named wave tables or algorithmic wave tables do not exist";
					break;
				case eBuildInstrExpectedEnvelope:
					S = "Expected 'envelope'";
					break;
				case eBuildInstrExpectedLFO:
					S = "Expected 'lfo'";
					break;
				case eBuildInstrMultipleOscStereoBias:
					S = "Oscillator parameter 'stereobias' has already been specified";
					break;
				case eBuildInstrMultipleOscDisplacement:
					S = "Oscillator parameter 'displacement' has already been specified";
					break;
				case eBuildInstrMultipleOscSurroundBias:
					S = "Oscillator parameter 'surroundbias' has already been specified";
					break;
				case eBuildInserMultipleOscFreqAdder:
					S = "Oscillator parameter 'freqadder' has already been specified";
					break;
			}

		return S;
	}
