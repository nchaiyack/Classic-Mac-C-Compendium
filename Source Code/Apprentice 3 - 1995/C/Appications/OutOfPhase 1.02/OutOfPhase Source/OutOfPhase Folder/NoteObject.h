/* NoteObject.h */

#ifndef Included_NoteObject_h
#define Included_NoteObject_h

/* NoteObject module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* BinaryCodedDecimal */
/* Memory */
/* Screen */
/* Fractions */
/* DrawCommand */
/* DataMunging */
/* Numbers */
/* Frequency */
/* MainWindowStuff */
/* BufferedFileInput */
/* BufferedFileOutput */
/* TempoController */

#include "Screen.h"
#include "BinaryCodedDecimal.h"
#include "MainWindowStuff.h"

/* forward declarations */
struct FractionRec;
struct BufferedInputRec;
struct BufferedOutputRec;

#ifdef ShowMe_NoteObjectRec
/* declared publicly for fast access during play routines */
struct NoteObjectRec
	{
		/* flags field.  if high bit is set, then it is a command and the low order */
		/* bits are the opcode.  if high bit is clear, then it is a note and the low */
		/* order bits determine the duration and some other information */
		unsigned long					Flags;

		/* data fields */
		union
			{
				struct
					{
						/* halfstep pitch index */
						short									Pitch;

						/* portamento rate flag.  this parameter determines how long a portamento */
						/* transition should take, in fractions of a quarter note.  it only has */
						/* effect if this note is the target of a tie.  A value of 0 means it */
						/* should be instantaneous, i.e. no portamento. */
						SmallBCDType					PortamentoDuration;

						/* displacement forward and backward, in fractions of a quarter note. */
						/* this value is added to the current overall early/late adjust factor */
						/* as determined by the default and any adjustments or sweeps in progress */
						/* as initiated by channel commands */
						SmallBCDType					EarlyLateAdjust;

						/* note duration adjustment, in fractions of a quarter note.  this value */
						/* is either added to or multiplied by the duration to make the note run */
						/* longer or shorter.  this does not effect when subsequent notes start. */
						SmallBCDType					DurationAdjust;

						/* note to tie after completion of this note (NIL = none) */
						struct NoteObjectRec*	Tie;

						/* these are fine tuning adjustment parameters.  they correspond to */
						/* some of the values that can be set by commands.  the values in */
						/* these parameters have effect over and above the effects of the */
						/* values set in the commands (i.e. the effects are cumulative) when */
						/* appropriate. */

						/* release points as a fraction of the note's duration */
						/* these are relative to either the beginning of the note (key-down) or */
						/* the end of the note (duration counter runout) as determined by some */
						/* bits in the flags word.  If the flag indicates that the default origin */
						/* should be used, then this value is added to the channel overall/default */
						/* value, otherwise the default value is not used. */
						SmallBCDType					ReleasePoint1;
						SmallBCDType					ReleasePoint2;

						/* adjustment for overall loudness envelope */
						/* this factor multiplicatively scales the output volume of the channel, */
						/* so a value of 1 leaves it unchanged.  This scaling is in addition to */
						/* the channel overall/default scaling, by multiplying the values. */
						SmallBCDType					OverallLoudnessAdjustment;

						/* left-right positioning adjustment for the note */
						/* this factor determines where the sound will come from when stereo */
						/* synthesis is being used.  -1 is far left, 1 is far right, and 0 is */
						/* center.  This is added to the channel overall/default position. */
						SmallBCDType					StereoPositionAdjustment;

						/* front-back positioning adjustment for the note */
						/* this factor determines where the sound will come from when surround */
						/* synthesis is being used.  1 is far front and -1 is far rear.  this */
						/* is added to the channel overall/default position */
						SmallBCDType					SurroundPositionAdjustment;

						/* special accent adjustments value for wave table generators */
						/* these factors are the base-2 log of multiplicative scaling factors. */
						/* therefore, 0 makes no change, 1 doubles the target value, and -1 */
						/* halves it.  the values are combined with the respective global */
						/* channel values via addition. */
						SmallBCDType					Accent1;
						SmallBCDType					Accent2;
						SmallBCDType					Accent3;
						SmallBCDType					Accent4;

						/* then this is the pitch to use to select the sample or wave table from */
						/* the multisample list instead of Pitch.  if this is -1, then Pitch */
						/* should be used for sample selection. */
						short									MultisamplePitchAsIf;

						/* adjustment factor for pitch displacement depth envelope amplitude. */
						/* this factor determines the depth of the frequency LFO generators. */
						/* the envelope controlling the depth is taken as a value from 0..1. */
						/* this parameter provides units, either as Hertz or halfsteps (as */
						/* indicated by some bits in the flags word) and the value is added to */
						/* the overall channel/default value (after Hertz/halfstep conversion). */
						/* If the flags indicate that the default pitch conversion should be */
						/* used, then this value is used to multiplicatively scale the default */
						/* adjustment. */
						SmallBCDType					PitchDisplacementDepthAdjustment;

						/* adjustment factor for pitch displacement rate envelope amplitude. */
						/* this factor determines the rate of the frequency LFO generators.  the */
						/* envelope controlling the rate is taken as a value from 0..1.  this */
						/* parameter provides units, in periods per second, for scaling the rate */
						/* envelope output.  This value is added to the overall channel/default */
						/* value. */
						SmallBCDType					PitchDisplacementRateAdjustment;

						/* selection of pitch displacement envelope start point. */
						/* this specifies when the pitch displacement LFOs start.  this value */
						/* is relative to the start or end of the note, as determined by some */
						/* bits in the flags word.  If the flags specify that the default origin */
						/* should be used, then the value is added to the default start point, */
						/* otherwise the default start point is not used. */
						SmallBCDType					PitchDisplacementStartPoint;

						/* overall envelope rate adjustment. */
						/* this factor scales the total speed with which all envelopes associated */
						/* with the note undergo transitions.  A value of 1 does not change them, */
						/* smaller values accelerate transition.  This value is in addition to */
						/* the channel/default value via multiplication of the values. */
						SmallBCDType					HurryUpFactor;

						/* detuning in either Hertz or halfsteps. */
						/* this value specifies how much to detune the nominal pitch of the note. */
						/* the value is either in units of Hertz or halfsteps, as determined by */
						/* a bit in the flags word, and the detuning is added to the channel */
						/* overall/default detuning.  If the flags indicate that the default */
						/* pitch coversion should be used, then this value is multiplied by */
						/* the channel overall/default value to scale it. */
						SmallBCDType					Detuning;
					}	Note;

				struct
					{
						/* string argument for commands.  NIL means it hasn't been defined */
						char*									StringArgument;
						/* numeric arguments for commands */
						long									Argument1;
						long									Argument2;
						long									Argument3;
					} Command;
			} a;
	};
#else
struct NoteObjectRec;
#endif

typedef struct NoteObjectRec NoteObjectRec;

/* note flags and duration stuff */
#define BIT(x) (1UL << (x))

#define eDurationMask (BIT(0) | BIT(1) | BIT(2) | BIT(3))
#define e64thNote (1 * BIT(0)) /* value 0 skipped */
#define e32ndNote (2 * BIT(0))
#define e16thNote (3 * BIT(0))
#define e8thNote (4 * BIT(0))
#define e4thNote (5 * BIT(0))
#define e2ndNote (6 * BIT(0))
#define eWholeNote (7 * BIT(0))
#define eDoubleNote (8 * BIT(0))
#define eQuadNote (9 * BIT(0))

#define eDivisionMask (BIT(4) | BIT(5))
#define eDiv1Modifier (0 * BIT(4))
#define eDiv3Modifier (1 * BIT(4))
#define eDiv5Modifier (2 * BIT(4))
#define eDiv7Modifier (3 * BIT(4))

#define eDotModifier (BIT(6))
#define eFlatModifier (BIT(7))
#define eSharpModifier (BIT(8))
#define eRestModifier (BIT(9))

#define eRelease1OriginMask (BIT(10) | BIT(11))
#define eRelease1FromDefault (1 * BIT(10))
#define eRelease1FromStart (2 * BIT(10))
#define eRelease1FromEnd (3 * BIT(10))

#define eRelease2OriginMask (BIT(12) | BIT(13))
#define eRelease2FromDefault (1 * BIT(12))
#define eRelease2FromStart (2 * BIT(12))
#define eRelease2FromEnd (3 * BIT(12))

#define eRelease3FromStartNotEnd (BIT(14))

#define ePitchDisplacementStartOriginMask (BIT(15) | BIT(16))
#define ePitchDisplacementStartFromDefault (1 * BIT(15))
#define ePitchDisplacementStartFromStart (2 * BIT(15))
#define ePitchDisplacementStartFromEnd (3 * BIT(15))

#define ePitchDisplacementDepthModeMask (BIT(17) | BIT(18))
#define ePitchDisplacementDepthModeDefault (1 * BIT(17))
#define ePitchDisplacementDepthModeHalfSteps (2 * BIT(17))
#define ePitchDisplacementDepthModeHertz (3 * BIT(17))

#define eDetuningModeMask (BIT(19) | BIT(20))
#define eDetuningModeDefault (1 * BIT(19))
#define eDetuningModeHalfSteps (2 * BIT(19))
#define eDetuningModeHertz (3 * BIT(19))

#define eDurationAdjustMask (BIT(21) | BIT(22))
#define eDurationAdjustDefault (1 * BIT(21))
#define eDurationAdjustAdditive (2 * BIT(21))
#define eDurationAdjustMultiplicative (3 * BIT(21))

#define eRetriggerEnvelopesOnTieFlag (BIT(23))

#define ePortamentoHertzNotHalfsteps (BIT(24))

#define eCommandFlag (BIT(31))

/* commands (low order 31 bits of the flag word) */
/* there are 4 parameters for commands: <1>, <2>, <3>, and <string> */
/* commands <1>, <2>, and <3> can be interpreted as large BCD numbers (xx.xxxxxx), */
/* extended small BCD numbers (xxxxx.xxx), or as integers */
/* large is represented by <_l>, extended small is <_xs>, integer is <_i> */
typedef enum
	{
		/* tempo adjustments */
		eCmdRestoreTempo EXECUTE(= 17732), /* restore the tempo to the default for the score */
		eCmdSetTempo, /* set tempo to <1xs> number of beats per minute */
		eCmdIncTempo, /* add <1xs> to the tempo control */
		eCmdSweepTempoAbs, /* <1xs> = target tempo, <2xs> = # of beats to reach it */
		eCmdSweepTempoRel, /* <1xs> = target adjust (add to tempo), <2xs> = # beats */

		/* stereo positioning adjustments */
		eCmdRestoreStereoPosition, /* restore stereo position to channel's default */
		eCmdSetStereoPosition, /* set position in channel <1l>: -1 = left, 1 = right */
		eCmdIncStereoPosition, /* adjust stereo position by adding <1l> */
		eCmdSweepStereoAbs, /* <1l> = new pos, <2xs> = # of beats to get there */
		eCmdSweepStereoRel, /* <1l> = pos adjust, <2xs> = # beats to get there */

		/* surround positioning adjustments */
		eCmdRestoreSurroundPosition, /* restore surround position to channel's default */
		eCmdSetSurroundPosition, /* set position in channel <1l>: 1 = front, -1 = rear */
		eCmdIncSurroundPosition, /* adjust surround position by adding <1l> */
		eCmdSweepSurroundAbs, /* <1l> = new pos, <2xs> = # of beats to get there */
		eCmdSweepSurroundRel, /* <1l> = pos adjust, <2xs> = # beats to get there */

		/* overall volume adjustments */
		eCmdRestoreVolume, /* restore the volume to the default for the channel */
		eCmdSetVolume, /* set the volume to the specified level (0..1) in <1l> */
		eCmdIncVolume, /* multiply <1l> by the volume control */
		eCmdSweepVolumeAbs, /* <1l> = new volume, <2xs> = # of beats to reach it */
		eCmdSweepVolumeRel, /* <1l> = volume adjust, <2xs> = # of beats to reach it */

		/* default release point adjustment values */
		eCmdRestoreReleasePoint1, /* restore release point to master default */
		eCmdSetReleasePoint1, /* set the default release point to new value <1l> */
		eCmdIncReleasePoint1, /* add <1l> to default release point for adjustment */
		eCmdReleasePointOrigin1, /* <1i> -1 = from start, 0 = from end of note */
		eCmdSweepReleaseAbs1, /* <1l> = new release, <2xs> = # of beats to get there */
		eCmdSweepReleaseRel1, /* <1l> = release adjust, <2xs> = # of beats to get there */

		eCmdRestoreReleasePoint2, /* restore release point to master default */
		eCmdSetReleasePoint2, /* set the default release point to new value <1l> */
		eCmdIncReleasePoint2, /* add <1l> to default release point for adjustment */
		eCmdReleasePointOrigin2, /* <1i> -1 = from start, 0 = from end of note */
		eCmdSweepReleaseAbs2, /* <1l> = new release, <2xs> = # of beats to get there */
		eCmdSweepReleaseRel2, /* <1l> = release adjust, <2xs> = # of beats to get there */

		/* set the default accent values */
		eCmdRestoreAccent1, /* restore accent value to master default */
		eCmdSetAccent1, /* specify the new default accent in <1l> */
		eCmdIncAccent1, /* add <1l> to the default accent */
		eCmdSweepAccentAbs1, /* <1l> = new accent, <2xs> = # of beats to get there */
		eCmdSweepAccentRel1, /* <1l> = accent adjust, <2xs> = # of beats to get there */

		eCmdRestoreAccent2, /* restore accent value to master default */
		eCmdSetAccent2, /* specify the new default accent in <1l> */
		eCmdIncAccent2, /* add <1l> to the default accent */
		eCmdSweepAccentAbs2, /* <1l> = new accent, <2xs> = # of beats to get there */
		eCmdSweepAccentRel2, /* <1l> = accent adjust, <2xs> = # of beats to get there */

		eCmdRestoreAccent3, /* restore accent value to master default */
		eCmdSetAccent3, /* specify the new default accent in <1l> */
		eCmdIncAccent3, /* add <1l> to the default accent */
		eCmdSweepAccentAbs3, /* <1l> = new accent, <2xs> = # of beats to get there */
		eCmdSweepAccentRel3, /* <1l> = accent adjust, <2xs> = # of beats to get there */

		eCmdRestoreAccent4, /* restore accent value to master default */
		eCmdSetAccent4, /* specify the new default accent in <1l> */
		eCmdIncAccent4, /* add <1l> to the default accent */
		eCmdSweepAccentAbs4, /* <1l> = new accent, <2xs> = # of beats to get there */
		eCmdSweepAccentRel4, /* <1l> = accent adjust, <2xs> = # of beats to get there */

		/* set pitch displacement depth adjustment */
		eCmdRestorePitchDispDepth, /* restore max pitch disp depth value to default */
		eCmdSetPitchDispDepth, /* set new max pitch disp depth <1l> */
		eCmdIncPitchDispDepth, /* add <1l> to the default pitch disp depth */
		eCmdPitchDispDepthMode, /* <1i>:  -1: Hertz, 0: half-steps */
		eCmdSweepPitchDispDepthAbs, /* <1l> = new depth, <2xs> = # of beats */
		eCmdSweepPitchDispDepthRel, /* <1l> = depth adjust, <2xs> = # of beats */

		/* set pitch displacement rate adjustment */
		eCmdRestorePitchDispRate, /* restore max pitch disp rate to the master default */
		eCmdSetPitchDispRate, /* set new max pitch disp rate in seconds to <1l> */
		eCmdIncPitchDispRate, /* add <1l> to the default max pitch disp rate */
		eCmdSweepPitchDispRateAbs, /* <1l> = new rate, <2xs> = # of beats to get there */
		eCmdSweepPitchDispRateRel, /* <1l> = rate adjust, <2xs> = # of beats to get there */

		/* set pitch displacement start point, same way as release point */
		eCmdRestorePitchDispStart, /* restore pitch disp start point to default */
		eCmdSetPitchDispStart, /* set the start point to <1l> */
		eCmdIncPitchDispStart, /* add <1l> to the pitch disp start point */
		eCmdPitchDispStartOrigin, /* specify the origin, same as for release point <1i> */
		eCmdSweepPitchDispStartAbs, /* <1l> = new vib start, <2xs> = # of beats */
		eCmdSweepPitchDispStartRel, /* <1l> = vib adjust, <2xs> = # of beats */

		/* hurry up adjustment */
		eCmdRestoreHurryUp, /* restore default hurryup factor */
		eCmdSetHurryUp, /* set the hurryup factor to <1l> */
		eCmdIncHurryUp, /* add <1l> to the hurryup factor */
		eCmdSweepHurryUpAbs, /* <1l> = new hurryup factor, <2xs> = # of beats */
		eCmdSweepHurryUpRel, /* <1l> = hurryup adjust, <2xs> = # of beats to get there */

		/* default detune */
		eCmdRestoreDetune, /* restore the default detune factor */
		eCmdSetDetune, /* set the detune factor to <1l> */
		eCmdIncDetune, /* add <1l> to current detune factor */
		eCmdDetuneMode, /* <1i>:  -1: Hertz, 0: half-steps */
		eCmdSweepDetuneAbs, /* <1l> = new detune, <2xs> = # of beats */
		eCmdSweepDetuneRel, /* <1l> = detune adjust, <2xs> = # of beats */

		/* default early/late adjust */
		eCmdRestoreEarlyLateAdjust, /* restore the default early/late adjust value */
		eCmdSetEarlyLateAdjust, /* set the early/late adjust value to <1l> */
		eCmdIncEarlyLateAdjust, /* add <1l> to the current early/late adjust value */
		eCmdSweepEarlyLateAbs, /* <1l> = new early/late adjust, <2xs> = # of beats */
		eCmdSweepEarlyLateRel, /* <1l> = early/late delta, <2xs> = # of beats to get there */

		/* default duration adjust */
		eCmdRestoreDurationAdjust, /* restore the default duration adjust value */
		eCmdSetDurationAdjust, /* set duration adjust value to <1l> */
		eCmdIncDurationAdjust, /* add <1l> to the current duration adjust value */
		eCmdSweepDurationAbs, /* <1l> = new duration adjust, <2xs> = # of beats */
		eCmdSweepDurationRel, /* <1l> = duration adjust delta, <2xs> = # of beats */
		eCmdDurationAdjustMode, /* <1i>:  -1: Multiplicative, 0: Additive */

		/* set the meter.  this is used by the editor for placing measure bars. */
		/* measuring restarts immediately after this command */
		eCmdSetMeter, /* <1i> = numerator, <2i> = denominator */
		/* immediately change the measure number */
		eCmdSetMeasureNumber, /* <1i> = new number */

		/* set the track transpose to some number of half-steps */
		eCmdSetTranspose, /* <1i> = signed number of half-steps */
		eCmdAdjustTranspose, /* <1i> = added to the current transpose value */

		/* text marker in the score */
		eCmdMarker /* <string> holds the text */
	} NoteCommands;


/* create a new note with space for the specified number of parameters */
NoteObjectRec*				NewNote(void);

/* create a new command */
NoteObjectRec*				NewCommand(void);

/* dispose of the note or command */
void									DisposeNote(NoteObjectRec* Note);

/* find out what type of thing it is */
MyBoolean							IsItACommand(NoteObjectRec* Note);


/* convert the duration of the note into a fraction */
void									GetNoteDurationFrac(NoteObjectRec* Note, struct FractionRec* Frac);

/* draw the command on the screen, or measure how many pixels wide the image will be */
/* if it will draw, it assumes the clipping rectangle to be set up properly */
OrdType								DrawCommandOnScreen(WinType* Window, OrdType X, OrdType Y,
												FontType Font, FontSizeType FontSize, OrdType FontHeight,
												NoteObjectRec* Note, MyBoolean ActuallyDraw, MyBoolean GreyedOut);

/* get a static null terminated string literal containing the name of a command */
char*									GetCommandName(NoteCommands Command);

typedef enum
	{
		eNoParameters EXECUTE(= -425),
		e1SmallExtParameter, /* <1xs> */
		e2SmallExtParameters, /* <1xs> <2xs> */
		e1LargeParameter, /* <1l> */
		eFirstLargeSecondSmallExtParameters, /* <1l> <2xs> */
		e1ParamReleaseOrigin, /* origin <1i> */
		e1PitchDisplacementMode, /* hertz/steps <1i> */
		e2IntegerParameters, /* <1i> <2i> */
		e1DurationAdjustMode, /* multiplicative/additive <1i> */
		e1IntegerParameter, /* <1i> */
		e1StringParameterWithLineFeeds /* <string> */
	} CommandAddrMode;

/* get the addressing mode for a command */
CommandAddrMode				GetCommandAddressingMode(NoteCommands Command);


/* get the actual string argument contained in a command. NIL == not defined yet */
char*									GetCommandStringArg(NoteObjectRec* Command);

/* get the first numeric argument contained in a command */
long									GetCommandNumericArg1(NoteObjectRec* Command);

/* get the second numeric argument contained in a command */
long									GetCommandNumericArg2(NoteObjectRec* Command);

/* get the third numeric argument contained in a command */
long									GetCommandNumericArg3(NoteObjectRec* Command);

/* put a new string into the command.  the command becomes owner of the string. */
void									PutCommandStringArg(NoteObjectRec* Command, char* NewArg);

/* put a new first numeric argument into the command */
void									PutCommandNumericArg1(NoteObjectRec* Command, long NewValue);

/* put a new second numeric argument into the command */
void									PutCommandNumericArg2(NoteObjectRec* Command, long NewValue);

/* put a new third numeric argument into the command */
void									PutCommandNumericArg3(NoteObjectRec* Command, long NewValue);


/* get the pitch of a note */
short									GetNotePitch(NoteObjectRec* Note);

/* get the portamento transition time for the note */
double								GetNotePortamentoDuration(NoteObjectRec* Note);

/* get the early/late adjustment factor for a note */
double								GetNoteEarlyLateAdjust(NoteObjectRec* Note);

/* get the duration adjust value for a note */
double								GetNoteDurationAdjust(NoteObjectRec* Note);

/* get a pointer to the note that this note ties to or NIL if there is no tie */
NoteObjectRec*				GetNoteTieTarget(NoteObjectRec* Note);

/* get the first release point position from a note */
double								GetNoteReleasePoint1(NoteObjectRec* Note);

/* get the second release point position from a note */
double								GetNoteReleasePoint2(NoteObjectRec* Note);

/* get the overall loudness factor for the note */
double								GetNoteOverallLoudnessAdjustment(NoteObjectRec* Note);

/* get the stereo positioning for the note */
double								GetNoteStereoPositioning(NoteObjectRec* Note);

/* get the surround positioning for the note */
double								GetNoteSurroundPositioning(NoteObjectRec* Note);

/* get the first accent factor for the note */
double								GetNoteAccent1(NoteObjectRec* Note);

/* get the second accent factor for the note */
double								GetNoteAccent2(NoteObjectRec* Note);

/* get the third accent factor for the note */
double								GetNoteAccent3(NoteObjectRec* Note);

/* get the fourth accent factor for the note */
double								GetNoteAccent4(NoteObjectRec* Note);

/* get the pitch that the table selector should treat the note as using */
short									GetNoteMultisampleFalsePitch(NoteObjectRec* Note);

/* get the pitch displacement depth adjust factor for the note */
double								GetNotePitchDisplacementDepthAdjust(NoteObjectRec* Note);

/* get the pitch displacement rate adjust factor for the note */
double								GetNotePitchDisplacementRateAdjust(NoteObjectRec* Note);

/* get the pitch displacement envelope start point */
double								GetNotePitchDisplacementStartPoint(NoteObjectRec* Note);

/* get the hurryup factor for the note */
double								GetNoteHurryUpFactor(NoteObjectRec* Note);

/* get the detuning adjustment for the note */
double								GetNoteDetuning(NoteObjectRec* Note);


/* change the pitch of a note */
void									PutNotePitch(NoteObjectRec* Note, short NewPitch);

/* get the portamento transition time for the note */
void									PutNotePortamentoDuration(NoteObjectRec* Note,
												double NewPortamentoDuration);

/* change the early/late adjustment factor for a note */
void									PutNoteEarlyLateAdjust(NoteObjectRec* Note, double NewEarlyLate);

/* change the duration adjust value for a note */
void									PutNoteDurationAdjust(NoteObjectRec* Note,
												double NewDurationAdjust);

/* change the pointer to the note that this note ties to. */
void									PutNoteTieTarget(NoteObjectRec* Note, NoteObjectRec* NewTieTarget);

/* change the first release point position from a note */
void									PutNoteReleasePoint1(NoteObjectRec* Note, double NewReleasePoint1);

/* change the second release point position from a note */
void									PutNoteReleasePoint2(NoteObjectRec* Note, double NewReleasePoint2);

/* change the overall loudness factor for the note */
void									PutNoteOverallLoudnessAdjustment(NoteObjectRec* Note,
												double NewLoudnessAdjust);

/* change the stereo positioning for the note */
void									PutNoteStereoPositioning(NoteObjectRec* Note,
												double NewStereoPosition);

/* change the surround positioning for the note */
void									PutNoteSurroundPositioning(NoteObjectRec* Note,
												double NewSurroundPosition);

/* change the first accent factor for the note */
void									PutNoteAccent1(NoteObjectRec* Note, double NewAccent1);

/* change the second accent factor for the note */
void									PutNoteAccent2(NoteObjectRec* Note, double NewAccent2);

/* change the third accent factor for the note */
void									PutNoteAccent3(NoteObjectRec* Note, double NewAccent3);

/* change the fourth accent factor for the note */
void									PutNoteAccent4(NoteObjectRec* Note, double NewAccent4);

/* change the pitch that the table selector should treat the note as using */
void									PutNoteMultisampleFalsePitch(NoteObjectRec* Note,
												short NewFalsePitch);

/* change the pitch displacement depth adjust factor for the note */
void									PutNotePitchDisplacementDepthAdjust(NoteObjectRec* Note,
												double NewPitchDisplacementDepthAdjust);

/* change the pitch displacement rate adjust factor for the note */
void									PutNotePitchDisplacementRateAdjust(NoteObjectRec* Note,
												double NewPitchDisplacementRateAdjust);

/* change the pitch displacement envelope start point */
void									PutNotePitchDisplacementStartPoint(NoteObjectRec* Note,
												double NewPitchDisplacementStartPoint);

/* change the hurryup factor for the note */
void									PutNoteHurryUpFactor(NoteObjectRec* Note, double NewHurryUpFactor);

/* change the detuning adjustment for the note */
void									PutNoteDetuning(NoteObjectRec* Note, double NewDetuning);


/* get the opcode for a command */
unsigned long					GetCommandOpcode(NoteObjectRec* Command);

/* put a new opcode in the command */
void									PutCommandOpcode(NoteObjectRec* Command, unsigned long NewOpcode);


/* get the duration of a note */
unsigned long					GetNoteDuration(NoteObjectRec* Note);

/* get the duration division of a note */
unsigned long					GetNoteDurationDivision(NoteObjectRec* Note);

/* get the dot status of a note */
MyBoolean							GetNoteDotStatus(NoteObjectRec* Note);

/* get the flat or sharp status of a note */
unsigned long					GetNoteFlatOrSharpStatus(NoteObjectRec* Note);

/* get the rest status of a note */
MyBoolean							GetNoteIsItARest(NoteObjectRec* Note);

/* get the first release point origin of a note */
unsigned long					GetNoteRelease1Origin(NoteObjectRec* Note);

/* get the second release point origin of a note */
unsigned long					GetNoteRelease2Origin(NoteObjectRec* Note);

/* get the third release from start instead of end flag of a note */
MyBoolean							GetNoteRelease3FromStartInsteadOfEnd(NoteObjectRec* Note);

/* get the pitch displacement origin of a note */
unsigned long					GetNotePitchDisplacementStartOrigin(NoteObjectRec* Note);

/* get the pitch displacement depth pitch conversion mode of a note */
unsigned long					GetNotePitchDisplacementDepthConversionMode(NoteObjectRec* Note);

/* get the detuning pitch conversion mode of a note */
unsigned long					GetNoteDetuneConversionMode(NoteObjectRec* Note);

/* get the retrigger envelope on tie status of a note */
MyBoolean							GetNoteRetriggerEnvelopesOnTieStatus(NoteObjectRec* Note);

/* get the duration adjustment mode of a note */
unsigned long					GetNoteDurationAdjustMode(NoteObjectRec* Note);

/* get a flag indicating that portamento should use Hertz instead of halfsteps */
MyBoolean							GetNotePortamentoHertzNotHalfstepsFlag(NoteObjectRec* Note);


/* change the duration of a note */
void									PutNoteDuration(NoteObjectRec* Note, unsigned long NewDuration);

/* change the duration division of a note */
void									PutNoteDurationDivision(NoteObjectRec* Note,
												unsigned long NewDivision);

/* change the dot status of a note */
void									PutNoteDotStatus(NoteObjectRec* Note, MyBoolean HasADot);

/* change the flat or sharp status of a note */
void									PutNoteFlatOrSharpStatus(NoteObjectRec* Note,
												unsigned long NewFlatOrSharpStatus);

/* change the rest status of a note */
void									PutNoteIsItARest(NoteObjectRec* Note, MyBoolean IsARest);

/* change the first release point origin of a note */
void									PutNoteRelease1Origin(NoteObjectRec* Note,
												unsigned long NewReleasePoint1Origin);

/* change the second release point origin of a note */
void									PutNoteRelease2Origin(NoteObjectRec* Note,
												unsigned long NewReleasePoitn2Origin);

/* change the third release from start instead of end flag of a note */
void									PutNoteRelease3FromStartInsteadOfEnd(NoteObjectRec* Note,
												MyBoolean ShouldWeReleasePoint3FromStartInsteadOfEnd);

/* change the pitch displacement origin of a note */
void									PutNotePitchDisplacementStartOrigin(NoteObjectRec* Note,
												unsigned long NewPitchDisplacementStartOrigin);

/* change the pitch displacement depth pitch conversion mode of a note */
void									PutNotePitchDisplacementDepthConversionMode(NoteObjectRec* Note,
												unsigned long NewPitchDisplacementDepthConversionMode);

/* change the detuning pitch conversion mode of a note */
void									PutNoteDetuneConversionMode(NoteObjectRec* Note,
												unsigned long NewDetuneConversionMode);

/* change the retrigger envelope on tie status of a note */
void									PutNoteRetriggerEnvelopesOnTieStatus(NoteObjectRec* Note,
												MyBoolean ShouldWeRetriggerEnvelopesOnTie);

/* change the duration adjustment mode of a note */
void									PutNoteDurationAdjustMode(NoteObjectRec* Note,
												unsigned long NewDurationAdjustMode);

/* change the flag indicating that portamento should use Hertz instead of halfsteps */
void									PutNotePortamentoHertzNotHalfstepsFlag(NoteObjectRec* Note,
												MyBoolean ShouldWeUseHertzInsteadOfHalfsteps);


/* read a note object in from a file.  this does not handle ties since they */
/* are done separately. */
FileLoadingErrors			NoteObjectNewFromFile(NoteObjectRec** ObjectOut,
												struct BufferedInputRec* Input);

/* write a note object to the file.  this does not handle ties since they are */
/* done separately. */
FileLoadingErrors			NoteObjectWriteDataOut(NoteObjectRec* Note,
												struct BufferedOutputRec* Output);

/* make a complete copy of the note/command and any blocks it has allocated */
NoteObjectRec*				DeepCopyNoteObject(NoteObjectRec* Note);

#endif
