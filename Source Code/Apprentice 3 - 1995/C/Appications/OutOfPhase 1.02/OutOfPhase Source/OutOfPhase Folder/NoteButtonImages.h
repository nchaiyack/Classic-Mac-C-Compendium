/* NoteButtonImages.h */

#ifndef Included_NoteButtonImages_h
#define Included_NoteButtonImages_h

/* NoteButtonImages module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */

/* the variables declared externally */
#ifndef Compiling_NoteButtonImages
	extern struct Bitmap*				ArrowButtonBits;
	extern struct Bitmap*				SixtyFourthButtonBits;
	extern struct Bitmap*				ThirtySecondButtonBits;
	extern struct Bitmap*				SixteenthButtonBits;
	extern struct Bitmap*				EighthButtonBits;
	extern struct Bitmap*				QuarterButtonBits;
	extern struct Bitmap*				HalfButtonBits;
	extern struct Bitmap*				WholeButtonBits;
	extern struct Bitmap*				DoubleButtonBits;
	extern struct Bitmap*				QuadButtonBits;
	extern struct Bitmap*				SharpButtonBits;
	extern struct Bitmap*				FlatButtonBits;
	extern struct Bitmap*				NaturalButtonBits;
	extern struct Bitmap*				NoteVsRestButtonBits;
	extern struct Bitmap*				RestVsNoteButtonBits;
	extern struct Bitmap*				NoDotButtonBits;
	extern struct Bitmap*				YesDotButtonBits;
	extern struct Bitmap*				Div1ButtonBits;
	extern struct Bitmap*				Div3ButtonBits;
	extern struct Bitmap*				Div5ButtonBits;
	extern struct Bitmap*				Div7ButtonBits;
	extern struct Bitmap*				CommandButtonBits;

	extern struct Bitmap*				ArrowButtonMouseDownBits;
	extern struct Bitmap*				SixtyFourthButtonMouseDownBits;
	extern struct Bitmap*				ThirtySecondButtonMouseDownBits;
	extern struct Bitmap*				SixteenthButtonMouseDownBits;
	extern struct Bitmap*				EighthButtonMouseDownBits;
	extern struct Bitmap*				QuarterButtonMouseDownBits;
	extern struct Bitmap*				HalfButtonMouseDownBits;
	extern struct Bitmap*				WholeButtonMouseDownBits;
	extern struct Bitmap*				DoubleButtonMouseDownBits;
	extern struct Bitmap*				QuadButtonMouseDownBits;
	extern struct Bitmap*				SharpButtonMouseDownBits;
	extern struct Bitmap*				FlatButtonMouseDownBits;
	extern struct Bitmap*				NaturalButtonMouseDownBits;
	extern struct Bitmap*				NoteVsRestButtonMouseDownBits;
	extern struct Bitmap*				RestVsNoteButtonMouseDownBits;
	extern struct Bitmap*				NoDotButtonMouseDownBits;
	extern struct Bitmap*				YesDotButtonMouseDownBits;
	extern struct Bitmap*				Div1ButtonMouseDownBits;
	extern struct Bitmap*				Div3ButtonMouseDownBits;
	extern struct Bitmap*				Div5ButtonMouseDownBits;
	extern struct Bitmap*				Div7ButtonMouseDownBits;
	extern struct Bitmap*				CommandButtonMouseDownBits;

	extern struct Bitmap*				ArrowButtonSelectedBits;
	extern struct Bitmap*				SixtyFourthButtonSelectedBits;
	extern struct Bitmap*				ThirtySecondButtonSelectedBits;
	extern struct Bitmap*				SixteenthButtonSelectedBits;
	extern struct Bitmap*				EighthButtonSelectedBits;
	extern struct Bitmap*				QuarterButtonSelectedBits;
	extern struct Bitmap*				HalfButtonSelectedBits;
	extern struct Bitmap*				WholeButtonSelectedBits;
	extern struct Bitmap*				DoubleButtonSelectedBits;
	extern struct Bitmap*				QuadButtonSelectedBits;
	extern struct Bitmap*				SharpButtonSelectedBits;
	extern struct Bitmap*				FlatButtonSelectedBits;
	extern struct Bitmap*				NaturalButtonSelectedBits;
	extern struct Bitmap*				NoteVsRestButtonSelectedBits;
	extern struct Bitmap*				RestVsNoteButtonSelectedBits;
	extern struct Bitmap*				NoDotButtonSelectedBits;
	extern struct Bitmap*				YesDotButtonSelectedBits;
	extern struct Bitmap*				Div1ButtonSelectedBits;
	extern struct Bitmap*				Div3ButtonSelectedBits;
	extern struct Bitmap*				Div5ButtonSelectedBits;
	extern struct Bitmap*				Div7ButtonSelectedBits;
	extern struct Bitmap*				CommandButtonSelectedBits;

	extern struct Bitmap*				ArrowButtonSelectedMouseDownBits;
	extern struct Bitmap*				SixtyFourthButtonSelectedMouseDownBits;
	extern struct Bitmap*				ThirtySecondButtonSelectedMouseDownBits;
	extern struct Bitmap*				SixteenthButtonSelectedMouseDownBits;
	extern struct Bitmap*				EighthButtonSelectedMouseDownBits;
	extern struct Bitmap*				QuarterButtonSelectedMouseDownBits;
	extern struct Bitmap*				HalfButtonSelectedMouseDownBits;
	extern struct Bitmap*				WholeButtonSelectedMouseDownBits;
	extern struct Bitmap*				DoubleButtonSelectedMouseDownBits;
	extern struct Bitmap*				QuadButtonSelectedMouseDownBits;
	extern struct Bitmap*				SharpButtonSelectedMouseDownBits;
	extern struct Bitmap*				FlatButtonSelectedMouseDownBits;
	extern struct Bitmap*				NaturalButtonSelectedMouseDownBits;
	extern struct Bitmap*				NoteVsRestButtonSelectedMouseDownBits;
	extern struct Bitmap*				RestVsNoteButtonSelectedMouseDownBits;
	extern struct Bitmap*				NoDotButtonSelectedMouseDownBits;
	extern struct Bitmap*				YesDotButtonSelectedMouseDownBits;
	extern struct Bitmap*				Div1ButtonSelectedMouseDownBits;
	extern struct Bitmap*				Div3ButtonSelectedMouseDownBits;
	extern struct Bitmap*				Div5ButtonSelectedMouseDownBits;
	extern struct Bitmap*				Div7ButtonSelectedMouseDownBits;
	extern struct Bitmap*				CommandButtonSelectedMouseDownBits;
#endif

/* initialize the bitmaps in memory */
MyBoolean						InitializeNoteButtonImages(void);

/* clean up the memory used by the images */
void								CleanupNoteButtonImages(void);

#endif
