/* IncrementalParameterUpdator.h */

#ifndef Included_IncrementalParameterUpdator_h
#define Included_IncrementalParameterUpdator_h

/* IncrementalParameterUpdator module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* BinaryCodedDecimal */
/* PlayTrackInfoThang */
/* Memory */
/* TrackObject */
/* LinearTransition */
/* NoteObject */
/* FrameObject */

#include "BinaryCodedDecimal.h"

struct IncrParamUpdateRec;
typedef struct IncrParamUpdateRec IncrParamUpdateRec;

/* forwards */
struct TrackObjectRec;
struct LinearTransRec;
struct FrameObjectRec;
struct TempoControlRec;

#ifdef ShowMeIncrParamUpdateRec
struct IncrParamUpdateRec
	{
		LargeBCDType						DefaultStereoPosition;
		LargeBCDType						CurrentStereoPosition;
		struct LinearTransRec*	StereoPositionChange;
		long										StereoPositionChangeCountdown;

		LargeBCDType						DefaultSurroundPosition;
		LargeBCDType						CurrentSurroundPosition;
		struct LinearTransRec*	SurroundPositionChange;
		long										SurroundPositionChangeCountdown;

		LargeBCDType						DefaultVolume;
		LargeBCDType						CurrentVolume;
		struct LinearTransRec*	VolumeChange;
		long										VolumeChangeCountdown;

		LargeBCDType						DefaultReleasePoint1;
		LargeBCDType						CurrentReleasePoint1;
		struct LinearTransRec*	ReleasePoint1Change;
		long										ReleasePoint1ChangeCountdown;
		MyBoolean								ReleasePoint1FromStart; /* True = start, False = end */

		LargeBCDType						DefaultReleasePoint2;
		LargeBCDType						CurrentReleasePoint2;
		struct LinearTransRec*	ReleasePoint2Change;
		long										ReleasePoint2ChangeCountdown;
		MyBoolean								ReleasePoint2FromStart; /* True = start, False = end */

		LargeBCDType						DefaultAccent1;
		LargeBCDType						CurrentAccent1;
		struct LinearTransRec*	Accent1Change;
		long										Accent1ChangeCountdown;

		LargeBCDType						DefaultAccent2;
		LargeBCDType						CurrentAccent2;
		struct LinearTransRec*	Accent2Change;
		long										Accent2ChangeCountdown;

		LargeBCDType						DefaultAccent3;
		LargeBCDType						CurrentAccent3;
		struct LinearTransRec*	Accent3Change;
		long										Accent3ChangeCountdown;

		LargeBCDType						DefaultAccent4;
		LargeBCDType						CurrentAccent4;
		struct LinearTransRec*	Accent4Change;
		long										Accent4ChangeCountdown;

		LargeBCDType						DefaultPitchDisplacementDepthLimit;
		LargeBCDType						CurrentPitchDisplacementDepthLimit;
		struct LinearTransRec*	PitchDisplacementDepthLimitChange;
		long										PitchDisplacementDepthLimitChangeCountdown;
		MyBoolean								PitchDisplacementDepthHertz; /* True = Hertz, False = Halfsteps */

		LargeBCDType						DefaultPitchDisplacementRateLimit;
		LargeBCDType						CurrentPitchDisplacementRateLimit;
		struct LinearTransRec*	PitchDisplacementRateLimitChange;
		long										PitchDisplacementRateLimitChangeCountdown;

		LargeBCDType						DefaultPitchDisplacementStartPoint;
		LargeBCDType						CurrentPitchDisplacementStartPoint;
		struct LinearTransRec*	PitchDisplacementStartPointChange;
		long										PitchDisplacementStartPointChangeCountdown;
		MyBoolean								PitchDisplacementStartPointFromStart; /* True = start, False = end */

		LargeBCDType						DefaultHurryUp;
		LargeBCDType						CurrentHurryUp;
		struct LinearTransRec*	HurryUpChange;
		long										HurryUpChangeCountdown;

		LargeBCDType						DefaultDetune;
		LargeBCDType						CurrentDetune;
		struct LinearTransRec*	DetuneChange;
		long										DetuneChangeCountdown;
		MyBoolean								DetuneHertz; /* True = hertz, False = Halfsteps */

		LargeBCDType						DefaultEarlyLateAdjust;
		LargeBCDType						CurrentEarlyLateAdjust;
		struct LinearTransRec*	EarlyLateAdjustChange;
		long										EarlyLateAdjustChangeCountdown;

		LargeBCDType						DefaultDurationAdjust;
		LargeBCDType						CurrentDurationAdjust;
		struct LinearTransRec*	DurationAdjustChange;
		long										DurationAdjustChangeCountdown;
		MyBoolean								DurationAdjustAdditive; /* True = additive, False = multiplicative */

		struct TempoControlRec*	TempoControl;

		long										TransposeHalfsteps;
	};
#endif

/* build a new incremental parameter updator */
IncrParamUpdateRec*		NewInitializedParamUpdator(struct TrackObjectRec* Template,
												struct TempoControlRec* TempoControl);

/* dispose of the incremental parameter updator */
void									DisposeParamUpdator(IncrParamUpdateRec* Updator);

/* execute a series of update cycles.  the value passed in is the number of */
/* duration ticks.  there are DURATIONUPDATECLOCKRESOLUTION (64*2*3*5*7) ticks */
/* in a whole note */
void									ExecuteParamUpdate(IncrParamUpdateRec* Updator, long NumTicks);

/* evaluate a command frame & set any parameters accordingly */
void									ExecuteParamCommandFrame(IncrParamUpdateRec* Updator,
												struct FrameObjectRec* CommandFrame);

#endif
