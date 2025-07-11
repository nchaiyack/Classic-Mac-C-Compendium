
/************************************************************

 StdCompression.h

 Copyright Apple Computer, Inc. 1991
 All rights reserved

************************************************************/


#ifndef	__STDCOMPRESSION__
#define	__STDCOMPRESSION__

#ifndef __IMAGECOMPRESSION__
#include <ImageCompression.h>
#endif


typedef struct {
	long			flags;
	CodecType		theCodecType;
	CodecComponent	theCodec;
	CodecQ			spatialQuality;
	CodecQ			temporalQuality;
	short			depth;
	Fixed			frameRate;
	long			keyFrameRate;
	long			reserved1;
	long			reserved2;
} SCParams;


typedef pascal Boolean (*SCModalFilterProcPtr)(DialogPtr theDialog,
	EventRecord *theEvent, short *itemHit, long refcon);

typedef pascal short (*SCModalHookProcPtr)(DialogPtr theDialog,
	short itemHit, SCParams *params, long refcon);


/*
 *	Flags for SCParams.
 */
#define	scShowMotionSettings	(1L<<0)
#define	scListEveryCodec		(1L<<1)
#define	scAllowZeroFrameRate	(1L<<2)
#define	scAllowZeroKeyFrameRate	(1L<<3)


/*
 *	Possible test flags for setting test image.
 */
#define	scPreferCropping			(1<<0)
#define	scPreferScaling				(1<<1)
#define scPreferScalingAndCropping	(scPreferScaling + scPreferCropping)


/*
 *	Dimensions of the image preview box.
 */
#define	scTestImageWidth	80
#define	scTestImageHeight	80


/*
 *	Possible items returned by hookProc.
 */
#define	scSettingsChangedItem	-1
#define	scOKItem				1
#define	scCancelItem			2
#define	scCustomItem			3


pascal ComponentInstance
OpenStdCompression(void);

pascal ComponentResult
SCGetCompressionExtended(ComponentInstance ci,SCParams *params,Point where,
	SCModalFilterProcPtr filterProc,SCModalHookProcPtr hookProc,long refcon,StringPtr customName)
	= {0x2F3C,24,1,0x7000,0xA82A};

pascal ComponentResult
SCGetCompression(ComponentInstance ci,SCParams *params,Point where)
	= {0x42A7,0x42A7,0x42A7,0x42A7,0x2F3C,24,1,0x7000,0xA82A};

pascal ComponentResult
SCPositionRect(ComponentInstance ci,Rect *rp,Point *where)
	= {0x2F3C,8,2,0x7000,0xA82A};

pascal ComponentResult
SCPositionDialog(ComponentInstance ci,short id,Point *where)
	= {0x2F3C,6,3,0x7000,0xA82A};

pascal ComponentResult
SCSetTestImagePictHandle(ComponentInstance ci, PicHandle testPict, Rect *testRect, short testFlags)
	= {0x2F3C,10,4,0x7000,0xA82A};

pascal ComponentResult
SCSetTestImagePictFile(ComponentInstance ci, short testFileRef, Rect *testRect, short testFlags)
	= {0x2F3C,8,5,0x7000,0xA82A};

pascal ComponentResult
SCSetTestImagePixMap(ComponentInstance ci, PixMapHandle testPixMap, Rect *testRect, short testFlags)
	= {0x2F3C,10,6,0x7000,0xA82A};

pascal ComponentResult
SCGetBestDeviceRect(ComponentInstance ci, Rect *r) 
 	= {0x2F3C,4,7,0x7000,0xA82A};
	

#endif // __STDCOMPRESSION__

