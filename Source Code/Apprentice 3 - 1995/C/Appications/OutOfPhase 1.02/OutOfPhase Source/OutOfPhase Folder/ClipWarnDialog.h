/* ClipWarnDialog.h */

#ifndef Included_ClipWarnDialog_h
#define Included_ClipWarnDialog_h

/* ClipWarnDialog module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Numbers */
/* DataMunging */
/* Alert */
/* Memory */

/* present a dialog box detailing how much clipping occurred.  the DontTellHowToFix */
/* parameter determines whether the inverse volume correction will be displayed. */
void									ClipWarnDialog(long ClippedSampleCount, long TotalSampleCount,
												double MaxExtent, double Correction, MyBoolean DontTellHowToFix);

/* present a dialog box detailing how much clamping occurred and how to fix it. */
void									ClampWarnDialog(float Clamping, double Correction);

#endif
