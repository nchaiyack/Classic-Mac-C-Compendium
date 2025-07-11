/* GrowIcon.h */

#ifndef Included_GrowIcon_h
#define Included_GrowIcon_h

/* GrowIcon module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */

/* grow icon knows the image that goes in the lower-right hand corner of the */
/* window which can be clicked in to resize the window. */

/* this is here so we don't have to drag Screen in. */
struct Bitmap;

/* allocate the internal bitmap of the grow icon */
MyBoolean				InitializeGrowIcon(void);

/* dispose of the internal bitmap of the grow icon */
void						ShutdownGrowIcon(void);

/* get a pointer to the bitmap.  The flag is used to choose the appropriate bitmap. */
/* True means the window is active, so the grow icon should be returned.  False means */
/* the window is inactive, so the empty box should be returned */
struct Bitmap*	GetGrowIcon(MyBoolean GrowIconFlag);

#endif
