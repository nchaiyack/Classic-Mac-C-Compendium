#include <exceptions.h>
#include <Finder.h>
#include <Folders.h>
#include <Types.h>
#include <Icons.h>
#include <Resources.h>





/*	Custom icons numbered -16496 appear in aliases to volumes and
	servers.  I don't know where this is documented.	*/
#define	kVolumeAliasIconResource	-16496

typedef struct 
{
	OSType		fileCreator;
	OSType		fileType;
	short		DTRefNum;
} GetIconData;


/*  alignment type values  */
typedef short	IconAlignmentType;
#ifdef THINK_C
enum {
	atNone				= 0x0,
	atVerticalCenter	= 0x1,
	atTop				= 0x2,
	atBottom			= 0x3,
	atHorizontalCenter	= 0x4,
	atAbsoluteCenter	= (atVerticalCenter | atHorizontalCenter),
	atCenterTop			= (atTop | atHorizontalCenter),
	atCenterBottom		= (atBottom | atHorizontalCenter),
	atLeft				= 0x8,
	atCenterLeft		= (atVerticalCenter | atLeft),
	atTopLeft			= (atTop | atLeft),
	atBottomLeft		= (atBottom | atLeft),
	atRight				= 0xC,
	atCenterRight		= (atVerticalCenter | atRight),
	atTopRight			= (atTop | atRight),
	atBottomRight		= (atBottom | atRight)
};

/*  transform type values  */
typedef short	IconTransformType;

enum {
	ttNone				= 0x0,
	ttDisabled			= 0x1,
	ttOffline			= 0x2,
	ttOpen				= 0x3,
	ttLabel1			= 0x0100,
	ttLabel2			= 0x0200,
	ttLabel3			= 0x0300,
	ttLabel4			= 0x0400,
	ttLabel5			= 0x0500,
	ttLabel6			= 0x0600,
	ttLabel7			= 0x0700,
	ttSelected			= 0x4000,
	ttSelectedDisabled	= (ttSelected | ttDisabled),
	ttSelectedOffline	= (ttSelected | ttOffline),
	ttSelectedOpen		= (ttSelected | ttOpen)
};

/*  Selector mask values  */
typedef unsigned long	IconSelectorValue;	

enum {
	svLarge1Bit			= 0x00000001,
	svLarge4Bit			= 0x00000002,
	svLarge8Bit			= 0x00000004,
	svSmall1Bit			= 0x00000100,
	svSmall4Bit			= 0x00000200,
	svSmall8Bit			= 0x00000400,
	svMini1Bit			= 0x00010000,
	svMini4Bit			= 0x00020000,
	svMini8Bit			= 0x00040000,
	svAllLargeData		= 0x000000ff,
	svAllSmallData		= 0x0000ff00,
	svAllMiniData		= 0x00ff0000,
	svAll1BitData		= (svLarge1Bit | svSmall1Bit | svMini1Bit),
	svAll4BitData		= (svLarge4Bit | svSmall4Bit | svMini4Bit),
	svAll8BitData		= (svLarge8Bit | svSmall8Bit | svMini8Bit),
	svAllAvailableData	= 0xffffffff
};


typedef pascal OSErr (*IconAction)(ResType theType, Handle *theIcon, void *yourDataPtr);
typedef pascal Handle (*IconGetter)(ResType theType, void *yourDataPtr);

#ifdef __cplusplus
extern "C" {
#endif




pascal OSErr PlotIconID(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							short theResID)
	= {0x303C, 0x0500, 0xABC9};
	
pascal OSErr NewIconSuite(Handle *theIconSuite)
	= {0x303C, 0x0207, 0xABC9};
	
pascal OSErr AddIconToSuite(Handle theIconData,
							Handle theSuite,
							ResType theType)
	= {0x303C, 0x0608, 0xABC9};
	
pascal OSErr GetIconFromSuite(Handle *theIconData,
							Handle theSuite,
							ResType theType)
	= {0x303C, 0x0609, 0xABC9};
	
pascal OSErr ForEachIconDo(Handle theSuite,
							IconSelectorValue selector,
							IconAction action,
							void *yourDataPtr)
	= {0x303C, 0x080A, 0xABC9};
	
pascal OSErr GetIconSuite(Handle *theIconSuite,
							short theResID,
							IconSelectorValue selector)
	= {0x303C, 0x0501, 0xABC9};
	
pascal OSErr DisposeIconSuite(Handle theIconSuite,
							Boolean disposeData)
	= {0x303C, 0x0302, 0xABC9};
	
pascal OSErr PlotIconSuite(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							Handle theIconSuite)
	= {0x303C, 0x0603, 0xABC9};
	
pascal OSErr MakeIconCache(Handle *theHandle,
							IconGetter makeIcon,
							void *yourDataPtr)
	= {0x303C, 0x0604, 0xABC9};
	
pascal OSErr LoadIconCache(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							Handle theIconCache)
	= {0x303C, 0x0606, 0xABC9};

pascal OSErr PlotIconMethod(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							IconGetter theMethod,
							void *yourDataPtr)
	= {0x303C, 0x0805, 0xABC9};
	
pascal OSErr GetLabel(short labelNumber,
							RGBColor *labelColor,
							Str255 labelString)
	= {0x303c, 0x050B, 0xABC9};
	
pascal Boolean PtInIconID(Point testPt,
							Rect *iconRect,
							IconAlignmentType alignment,
							short iconID)
	= {0x303c, 0x060D, 0xABC9};
	
pascal Boolean PtInIconSuite(Point testPt,
							Rect *iconRect,
							IconAlignmentType alignment,
							Handle theIconSuite)
	= {0x303c, 0x070E, 0xABC9};
	
pascal Boolean PtInIconMethod(Point testPt,
							Rect *iconRect,
							IconAlignmentType alignment,
							IconGetter theMethod,
							void *yourDataPtr)
	= {0x303c, 0x090F, 0xABC9};
	
pascal Boolean RectInIconID(Rect *testRect,
							Rect *iconRect,
							IconAlignmentType alignment,
							short iconID)
	= {0x303c, 0x0610, 0xABC9};
	
pascal Boolean RectInIconSuite(Rect *testRect,
							Rect *iconRect,
							IconAlignmentType alignment,
							Handle theIconSuite)
	= {0x303c, 0x0711, 0xABC9};
	
pascal Boolean RectInIconMethod(Rect *testRect,
							Rect *iconRect,
							IconAlignmentType alignment,
							IconGetter theMethod,
							void *yourDataPtr)
	= {0x303c, 0x0912, 0xABC9};
	
pascal OSErr IconIDToRgn(RgnHandle theRgn,
							Rect *iconRect,
							IconAlignmentType alignment,
							short iconID)
	= {0x303c, 0x0613, 0xABC9};
	
pascal OSErr IconSuiteToRgn(RgnHandle theRgn,
							Rect *iconRect,
							IconAlignmentType alignment,
							Handle theIconSuite)
	= {0x303c, 0x0714, 0xABC9};
	
pascal OSErr IconMethodToRgn(RgnHandle theRgn,
							Rect *iconRect,
							IconAlignmentType alignment,
							IconGetter theMethod,
							void *yourDataPtr)
	= {0x303c, 0x0915, 0xABC9};
	
pascal OSErr SetSuiteLabel(Handle theSuite, short theLabel)
	= {0x303C, 0x0316, 0xABC9};

pascal short GetSuiteLabel(Handle theSuite)
	= {0x303C, 0x0217, 0xABC9};

pascal OSErr GetIconCacheData(Handle theCache, void **theData)
	= {0x303C, 0x0419, 0xABC9};
		
pascal OSErr SetIconCacheData(Handle theCache, void *theData)
	= {0x303C, 0x041A, 0xABC9};
		
pascal OSErr GetIconCacheProc(Handle theCache, IconGetter *theProc)
	= {0x303C, 0x041B, 0xABC9};
		
pascal OSErr SetIconCacheProc(Handle theCache, IconGetter theProc)
	= {0x303C, 0x041C, 0xABC9};
	
pascal OSErr PlotIconHandle(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							Handle theIcon)
	= {0x303C, 0x061D, 0xABC9};
	
pascal OSErr PlotSICNHandle(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							Handle theSICN)
	= {0x303C, 0x061E, 0xABC9};
	
pascal OSErr PlotCIconHandle(const Rect *theRect,
							IconAlignmentType align,
							IconTransformType transform,
							CIconHandle theCIcon)
	= {0x303C, 0x061F, 0xABC9};
	
#ifdef __cplusplus
}
#endif


#endif	// THINK_C






pascal OSErr GetFileIcon(
/* --> */	FSSpec				*thing,
/* --> */	IconSelectorValue	iconSelector,
/* <-- */	Handle				*theSuite);

Boolean	IsVolEjected( short vRefNum );

OSErr	GetCustomFileIcon(
/* --> */	FSSpec				*filespec,
/* --> */	IconSelectorValue	iconSelector,
/* <-- */	Handle				*theSuite);

OSErr	GetNormalFileIcon(
/* --> */	CInfoPBRec			*cpb,
/* --> */	IconSelectorValue	iconSelector,
/* <-- */	Handle				*theSuite);

void GetFinderFilename(
/* <-- */	StringPtr       _finderFilename);

//static	pascal OSErr GetIconProc(ResType theType, Handle *theIcon, void *yourDataPtr);
pascal OSErr GetIconProc(ResType theType, Handle *theIcon, void *yourDataPtr);

//static	short	FindDesktopDatabase(
short	FindDesktopDatabase(
/* --> */	short	firstVRefNum,
/* --> */	OSType	fileCreator);

//static	Boolean	InOneDesktop(
Boolean	InOneDesktop(
/* --> */	short	vRefNum,
/* --> */	OSType	fileCreator,
/* <-- */	short	*dtRefNum);

pascal OSErr GetResourceIcons(
/* <-- */	Handle	*theSuite,
/* --> */	short	theID,
/* --> */	long	theSelector);

//static pascal OSErr CopyOneIcon(
pascal OSErr CopyOneIcon(
/* --> */	ResType		theType,
/* <-> */	Handle		*theIcon,
/* --- */	void		*yourDataPtr);

OSErr CopyEachIcon(
/* <-> */	Handle theSuite);

short	FindGenericIconID(
/* --> */	OSType theType,
/* <-- */	Boolean	*inFinder);

pascal OSErr Get1IconSuite(
/* <-- */	Handle	*theSuite,
/* --> */	short	theID,
/* --> */	long	theSelector);

//static pascal OSErr Get1Icon(
pascal OSErr Get1Icon(
/* --> */	ResType	theType,
/* <-> */	Handle	*theIcon,
/* --> */	short	*resID);

pascal OSErr TestHandle(ResType theType, Handle *theIcon, void *yourDataPtr);

Boolean IsSuiteEmpty( Handle theSuite );







