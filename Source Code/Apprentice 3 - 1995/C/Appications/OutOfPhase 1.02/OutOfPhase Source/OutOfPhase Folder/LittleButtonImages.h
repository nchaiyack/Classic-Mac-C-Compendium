/* LittleButtonImages.h */

#ifndef Included_LittleButtonImages_h
#define Included_LittleButtonImages_h

/* LittleButtonImages module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Screen */

/* forward */
struct Bitmap;

/* these bitmaps are 12x12 pixels */
#ifndef Compiling_LittleButtonImages
	extern struct Bitmap*			PlusSignNormal;
	extern struct Bitmap*			PlusSignMouseDown;
	extern struct Bitmap*			MinusSignNormal;
	extern struct Bitmap*			MinusSignMouseDown;
#endif

/* initialize the plus and minus bitmaps */
MyBoolean					InitializeLittleButtonImages(void);

/* dispose of the little button images */
void							ShutdownLittleButtonImages(void);

#endif
