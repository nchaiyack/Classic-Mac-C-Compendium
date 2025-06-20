#define	kEnter	((char) 0x03)
#define kReturn	((char) 0x0D)

#define kOurSICNs		1964
#define kGenericLDEF	1000

typedef struct {
	StringHandle	theString;
	short			iconID;
	short			iconIndex;
	Boolean			enabled;
} ListItemRecord, *ListItemPtr, **ListItemHandle;

void	DoShowCustomList(void);
void	DoShowCustomWindow(void);

//
// Icon utilities stuff from Technote #306
//
typedef short	IconAlignmentType;
#define atNone				0x00
#define atVerticalCenter	0x01
#define atTop				0x02
#define atBottom			0x03
#define atHorizontalCenter	0x04
#define atAbsoluteCenter	(atVerticalCenter + atHorizontalCenter)
#define atCenterTop			(atTop + atHorizontalCenter)
#define atCenterBottom		(atBottom + atHorizontalCenter)
#define atLeft				0x08
#define atCenterLeft		(atVerticalCenter + atLeft)
#define atTopLeft			(atTop + atLeft)
#define atBottomLeft		(atBottom + atLeft)
#define atRight				0x0C
#define atCenterRight		(atVerticalCenter + atRight)
#define atTopRight			(atTop + atRight)
#define atBottomRight		(atBottom + atRight)

typedef short	IconTransformType;
#define ttNone				0x00
#define ttDisabled			0x01
#define ttOffline			0x02
#define ttOpen				0x03
#define ttSelected			0x4000
#define ttSelectedDisabled	(ttSelected + ttDisabled)
#define ttSelectedOffline	(ttSelected + ttOffline)
#define ttSelectedOpen		(ttSelected + ttOpen)

pascal OSErr	PlotIconID(Rect*, IconAlignmentType, IconTransformType, short)
		= {0x303C, 0x0500, 0xABC9};

