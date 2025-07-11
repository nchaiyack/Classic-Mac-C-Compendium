/* ModulationSpecifier.h */

#ifndef Included_ModulationSpecifier_h
#define Included_ModulationSpecifier_h

/* ModulationSpecifier module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* OscillatorSpecifier */
/* OscillatorListSpecifier */
/* DataMunging */
/* Envelope */
/* LFOListSpecifier */

/* list of modes for modulation operators */
/*  - eModulationAdditive:  modulate by adding the two streams together */
/*  - eModulationMultiplicative:  modulate by multiplying the two streams */
typedef enum
	{
		eModulationAdditive EXECUTE(= -13871), /* added into target data */
		eModulationMultiplicative /* multiplied by target data */
	} ModulationTypes;

/* list of destinations for modulation */
/*  - eModulatePhaseGen:  the output of one of the oscillators is used to */
/*       modulate the phase generator of another oscillator */
/*  - eModulateOutput:  the output of one of the oscillators is used to modulate */
/*       the output of another oscillator */
typedef enum
	{
		eModulatePhaseGen EXECUTE(= -9719), /* modulate wave phase generator */
		eModulateOutput /* modulate output from wave lookup */
	} ModDestTypes;

struct ModulationSpecRec;
typedef struct ModulationSpecRec ModulationSpecRec;

/* forwards */
struct OscillatorListRec;
struct OscillatorRec;
struct EnvelopeRec;
struct LFOListSpecRec;

/* create new list of modulation source specifiers */
ModulationSpecRec*		NewModulationSpecifier(void);

/* dispose a list of modulation source specifiers */
void									DisposeModulationSpecifier(ModulationSpecRec* ModSpec);

/* append a new entry.  the object becomes owner of the name */
MyBoolean							AppendModulationSpecEntry(ModulationSpecRec* ModSpec,
												ModulationTypes ModulationType,
												ModDestTypes ModulationDestination,
												struct EnvelopeRec* ScalingEnvelope,
												struct LFOListSpecRec* ScalingLFOList,
												struct EnvelopeRec* OriginAdjustEnvelope,
												struct LFOListSpecRec* OriginAdjustLFOList,
												char* InputOscillatorName);

/* get the total number of entries in the modulation list */
long									GetModulationSpecNumEntries(ModulationSpecRec* ModSpec);

/* resolve named references into actual oscillator references.  returns False */
/* if a name couldn't be resolved. */
MyBoolean							ResolveModulationReferences(ModulationSpecRec* ModSpec,
												struct OscillatorListRec* OscillatorList);

/* get the modulation type of one of the sources */
ModulationTypes				GetModulationOpType(ModulationSpecRec* ModSpec, long Index);

/* get the destination of the modulation */
ModDestTypes					GetModulationDestType(ModulationSpecRec* ModSpec, long Index);

/* get the oscillator name for one of the sources (this is the original string) */
char*									GetModulationOscillatorName(ModulationSpecRec* ModSpec, long Index);

/* get the actual oscillator reference for one of the sources */
struct OscillatorRec*	GetModulationOscillatorRef(ModulationSpecRec* ModSpec, long Index);

/* get the scaling factor envelope */
struct EnvelopeRec*		GetModulationScalingFactorEnvelope(ModulationSpecRec* ModSpec,
												long Index);

/* get the scaling factor LFO list */
struct LFOListSpecRec*	GetModulationScalingFactorLFOList(ModulationSpecRec* ModSpec,
												long Index);

/* get the origin adjust envelope */
struct EnvelopeRec*		GetModulationOriginAdjustEnvelope(ModulationSpecRec* ModSpec,
												long Index);

/* get the origin adjust LFO list */
struct LFOListSpecRec*	GetModulationOriginAdjustLFOList(ModulationSpecRec* ModSpec,
												long Index);

#endif
