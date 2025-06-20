/* DeterminedNoteStructure.h */

#ifndef Included_DeterminedNoteStructure_h
#define Included_DeterminedNoteStructure_h

/* DeterminedNoteStructure module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* BinaryCodedDecimal */
/* NoteObject */
/* IncrementalParameterUpdator */
/* FloatingPoint */
/* Frequency */
/* PlayTrackInfoThang */

struct FrozenNoteRec;
typedef struct FrozenNoteRec FrozenNoteRec;

/* forwards */
struct IncrParamUpdateRec;
struct NoteObjectRec;

#ifdef ShowMeFrozenNoteRec
struct FrozenNoteRec
	{
		/* reference to the note that defines this note.  this is used for determining */
		/* the following things: */
		/*   - hertz or halfsteps for portamento */
		/*   - the target of a tie */
		struct NoteObjectRec*		OriginalNote;

		/* frequency determined by pitch index + detuning, in Hertz */
		float										NominalFrequency;

		/* frequency used for doing multisampling, in Hertz */
		float										MultisampleFrequency;

		/* acceleration of envelopes */
		float										HurryUpFactor;

		/* portamento duration, in envelope ticks */
		long										PortamentoDuration;

		/* duration, in envelope ticks */
		long										Duration;

		/* first release point, in envelope ticks after start of note */
		long										ReleasePoint1;
		MyBoolean								Release1FromStart;
		/* second release point, in envelope ticks after start of note */
		long										ReleasePoint2;
		MyBoolean								Release2FromStart;
		/* third release point, in envelope ticks after start of note */
		long										ReleasePoint3;
		MyBoolean								Release3FromStart;

		/* overall loudness adjustment for envelopes, including global volume scaling */
		float										LoudnessAdjust;

		/* stereo positioning for note */
		float										StereoPosition;

		/* accent values for controlling envelopes */
		float										Accent1;
		float										Accent2;
		float										Accent3;
		float										Accent4;

		/* pitch displacement maximum depth */
		float										PitchDisplacementDepthLimit;
		/* operation mode:  False = half steps, True = hertz */
		MyBoolean								PitchDisplacementDepthInHertz;

		/* pitch displacement maximum rate, in LFO Hertz */
		float										PitchDisplacementRateLimit;

		/* pitch displacement start point, in envelope clocks after start of note */
		long										PitchDisplacementStartPoint;

		/* list crud */
		FrozenNoteRec*					Next;
	};
#endif

/* flush cached frozen note blocks */
void								FlushFrozenNoteStructures(void);

/* make sure the frozen note structure is valid */
#if DEBUG
void								ValidateFrozenNote(FrozenNoteRec* FrozenNote);
#else
#define ValidateFrozenNote(x) ((void)0)
#endif

/* dispose of a note object */
void								DisposeFrozenNote(FrozenNoteRec* FrozenNote);

/* build a new note object with all parameters determined.  *StartAdjustOut */
/* indicates how many ticks before (negative) or after (positive) now that */
/* the key-down should occur.  this is added to the scanning gap size and envelope */
/* origins to figure out how to schedule the note */
FrozenNoteRec*			FixNoteParameters(struct IncrParamUpdateRec* GlobalParamSource,
											struct NoteObjectRec* Note, long* StartAdjustOut,
											float OverallVolumeScaling, float EnvelopeTicksPerDurationTick);

#endif
