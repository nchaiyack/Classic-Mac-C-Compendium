/* EditImages.h */

#ifndef Included_EditImages_h
#define Included_EditImages_h

/* EditImages module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */
/* EventLoop */

/* the variables declared externally */
#ifndef Compiling_EditImages
	extern struct Bitmap*				Bits8Unselected;
	extern struct Bitmap*				Bits8Selected;
	extern struct Bitmap*				Bits8MouseDown;
	extern struct Bitmap*				Bits16Unselected;
	extern struct Bitmap*				Bits16Selected;
	extern struct Bitmap*				Bits16MouseDown;
	extern struct Bitmap*				MonoUnselected;
	extern struct Bitmap*				MonoSelected;
	extern struct Bitmap*				MonoMouseDown;
	extern struct Bitmap*				StereoUnselected;
	extern struct Bitmap*				StereoSelected;
	extern struct Bitmap*				StereoMouseDown;
	extern struct Bitmap*				PlayMouseUp;
	extern struct Bitmap*				PlayMouseDown;
	extern struct Bitmap*				ZoomInMouseUp;
	extern struct Bitmap*				ZoomInMouseDown;
	extern struct Bitmap*				ZoomOutMouseUp;
	extern struct Bitmap*				ZoomOutMouseDown;
#endif

/* initialize the bitmaps in memory */
MyBoolean						InitializeEditImages(void);

/* clean up the memory used by the images */
void								CleanupEditImages(void);

/* set the cursor to the sample insertion point cursor */
void								SetSampleInsertionCursor(void);

/* set the cursor to the score insertion point cursor when the cursor will INSERT */
void								SetScoreIntersticeCursor(void);

/* set the cursor to the score insertion point cursor when it will OVERLAY */
void								SetScoreOverlayCursor(void);

#endif
