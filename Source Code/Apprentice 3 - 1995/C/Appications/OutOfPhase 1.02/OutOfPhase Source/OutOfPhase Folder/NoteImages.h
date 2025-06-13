/* NoteImages.h */

#ifndef Included_NoteImages_h
#define Included_NoteImages_h

/* NoteImages module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */

/* the variables declared externally */
#ifndef Compiling_NoteImages
	extern struct Bitmap*				SixtyFourthNoteImage;
	extern struct Bitmap*				SixtyFourthNoteMask;
	extern struct Bitmap*				ThirtySecondNoteImage;
	extern struct Bitmap*				ThirtySecondNoteMask;
	extern struct Bitmap*				SixteenthNoteImage;
	extern struct Bitmap*				SixteenthNoteMask;
	extern struct Bitmap*				EighthNoteImage;
	extern struct Bitmap*				EighthNoteMask;
	extern struct Bitmap*				QuarterNoteImage;
	extern struct Bitmap*				QuarterNoteMask;
	extern struct Bitmap*				HalfNoteImage;
	extern struct Bitmap*				HalfNoteMask;
	extern struct Bitmap*				WholeNoteImage;
	extern struct Bitmap*				WholeNoteMask;
	extern struct Bitmap*				DoubleNoteImage;
	extern struct Bitmap*				DoubleNoteMask;
	extern struct Bitmap*				QuadNoteImage;
	extern struct Bitmap*				QuadNoteMask;
	extern struct Bitmap*				DotImage;
	extern struct Bitmap*				DotMask;
	extern struct Bitmap*				SharpImage;
	extern struct Bitmap*				SharpMask;
	extern struct Bitmap*				FlatImage;
	extern struct Bitmap*				FlatMask;
	extern struct Bitmap*				NaturalImage;
	extern struct Bitmap*				NaturalMask;
	extern struct Bitmap*				SortaSharpImage;
	extern struct Bitmap*				SortaSharpMask;
	extern struct Bitmap*				SortaFlatImage;
	extern struct Bitmap*				SortaFlatMask;
	extern struct Bitmap*				Div3Image;
	extern struct Bitmap*				Div3Mask;
	extern struct Bitmap*				Div5Image;
	extern struct Bitmap*				Div5Mask;
	extern struct Bitmap*				Div7Image;
	extern struct Bitmap*				Div7Mask;
	extern struct Bitmap*				SixtyFourthRestImage;
	extern struct Bitmap*				SixtyFourthRestMask;
	extern struct Bitmap*				ThirtySecondRestImage;
	extern struct Bitmap*				ThirtySecondRestMask;
	extern struct Bitmap*				SixteenthRestImage;
	extern struct Bitmap*				SixteenthRestMask;
	extern struct Bitmap*				EighthRestImage;
	extern struct Bitmap*				EighthRestMask;
	extern struct Bitmap*				QuarterRestImage;
	extern struct Bitmap*				QuarterRestMask;
	extern struct Bitmap*				HalfRestImage;
	extern struct Bitmap*				HalfRestMask;
	extern struct Bitmap*				WholeRestImage;
	extern struct Bitmap*				WholeRestMask;
	extern struct Bitmap*				DoubleRestImage;
	extern struct Bitmap*				DoubleRestMask;
	extern struct Bitmap*				QuadRestImage;
	extern struct Bitmap*				QuadRestMask;
	extern struct Bitmap*				GreyMask;
#endif

/* initialize the bitmaps in memory */
MyBoolean						InitializeNoteImages(void);

/* clean up the memory used by the images */
void								CleanupNoteImages(void);

#endif
