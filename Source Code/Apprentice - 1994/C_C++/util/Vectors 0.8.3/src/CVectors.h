#ifndef __DEFINES__
#define __DEFINES__

#ifdef __REDUMP__

/*-------------------------------- Defines -------------------------------*/
#define	NIL	0L
#define TRUE 1
#define FALSE 0

#define	LEFTARROW			28
#define	RIGHTARROW			29
#define UPARROW				30
#define DOWNARROW			31
#define	TAB					'\t'
#define DELETE				0x08
#define	RETURN				0x0D
#define	ENTER				0x03
#define LINEFEED			0x0A

#define gUseWorksheet		1

#define iYes 1
#define iNo 3
#define iCancel 2
#define kMaxShort 32767

/* File Menu Items */
#define iNew				1
#define iOpen				2
#define iClose				4
#define iSave				5
#define iSaveAs				6
#define iSaveACopy			7
#define iRevert				8
#define iPageSetup			10
#define iPrint				11
#define iQuit				13

/* Edit Menu Items */
#define iUndo				1
#define iCut				3
#define iCopy				4
#define iPaste				5
#define iClear				6
#define iSelectAll			8
#define iShowClipboard		9
#define iFormat				11
#define iAlign				13
#define iShiftRight			14
#define iShiftLeft			15

/* Find Menu Items */
#define iSelectSurvey		1
#define iFind				3
#define iFindSame			4
#define iFindSelection		5
#define iDisplaySelection	6
#define iReplace			8
#define iReplaceSame		9

/* Mark Menu Items */
#define iMark				1
#define iUnmark				2
#define iAlphabetical		3

/* Window Menu Items */
#define iTileWindows		1
#define iStackWindows		2

/* View Menu Items */
#define iRotate				1
#define iScale				2
#define iEnlarge			4
#define iReduce				5
#define iFitToWindow		6

/* Reports Menu Items */
#define iSchematic			1
#define iLog				2
#define iRose				3
#define iPosition			4

/* Data Menu Items */
#define iProcess			1
#define iProcessFiles		2
#define iOptions			3
#define iTitlePage			5
#define iDataPage			6
#define iNextPage			8
#define iPreviousPage		9
#define iFirstPage			10
#define iLastPage			11
#define iSelectPage			13
#define iConfigure			14
#define iDeleteTemplate		15

/* Printing Parameters */

#define LeftMargin			36
#define TopMargin			36
#define BottomMargin		36

/* ProcessingFlags bits */

#define ProcessingLevelMask 3
#define ExchangeFormatFlagBit 8
#define ProcessDimensions 16
#define ProcessHeadersFlagBit 32
#define ProcessPeople 64
#define ProcessAttributes 128
/*#define ProcessPrettyPrint 256 */
#define HorizontalDistance 512
#define WarningsFlagBit 1024
#define SnapShotFlagBit 2048
#define ErrorAbortFlagBit 4096

/* TextOutputFlag bits */

#define LogMethodsMask 3
#define GraphicsMethodsMask 0x1C
#define PostscriptFlag 4
#define Super3DFlag 8
#define DXFFlag 12
#define OutputPrettyPrint 32
#define OutputCoordinates 64
/*#define OutputPrettyPrint 128 */
#define OutputClosureStats 256
#define OutputAttributes 512
#define ExchangeMethod 1024
#define DiskOutputFlagBit 2048
#define CommaTab	4096
#define Quoted	8192
#define SparseFull 16384
#define SilentFlagBit 32768

#define gMyScrpHandle TE32KScrpHandle
#define gMyScrpLength TE32KScrpLength

#define MAXWINDS 10
#define	nullStopMask 0

#define rCorrectDialog	129
#define rCorrect2Dialog	130
#define rOptionsDialog 131
#define rProcessFilesDialog 132
#define rMoreOptions 134
#define rRequestDialog 135
#define rUnmarkDialog 136
#define rConfirmDialog 141
#define rRoseParameters 142
#define rConfigureDataPage 143
#define TitlePageDialog 138
#define DataPageDialog 139
#define	MessageDialog 258

#define DESK_ID	128
#define	FILE_ID	129
#define EDIT_ID	130
#define FIND_ID 131
#define MARK_ID 132
#define WINDOW_ID 133
#define VIEW_ID 134
#define REPORTS_ID 135
#define DATA_ID 136
#define SELECTTEMPLATE_ID 205
#define DATAFIELDS_ID 138
#define ROTATE_ID 200
#define SCALE_ID 201
#define	WNETrapNum	0x60
#define	UnImplTrapNum	0x9F
#define SUSPEND_RESUME_EVT	15
#define	SUSPEND_RESUME_BIT	0
#define	CLIPBOARD_BIT	0

#define HORIZ 1
#define VERT 2
#define SCALEMENUITEM 6
#define CORNERWIDTH 6
#define CORNERHEIGHT 4
#define OffScreenH 1024
#define OffScreenV 1024

/* Codes for lList management */

#define nDataList 0
#define nFieldLList 1
#define nLinkList 2
#define nLocnList 3
#define nStringList 4
#define nSubSegList 5
#define nSegList 6
#define nUnResSegList 7
#define nColList 8
#define nDimList 9
#define nLineList 10
#define nLongTraList 11
#define nNameList 12
#define nDbList 13
#define nRowList 14
#define nCellList 15
#define nHeaderList 16
#define NMasters 17

#define BlockLen 64 	  	/* # items in a list block */
#define BlockLo 63 			/* mask for index into a block */
#define BlockHi 6 			/* shift term for effective n/BlockLen */
#define BigBlockLen 1024	/* # items in the list for coordinate data */
#define BigBlockLo 1023
#define BigBlockHi 10
#define SmallBlockLen 256
#define SmallBlockLo 255
#define SmallBlockHi 8
#define MedBlockLen 512
#define MedBlockLo 511
#define MedBlockHi 9

/* Data codes. They are Simple Graphics Language inspired. */

/* Flags for strings */
#define NodeAtStart 1
#define NodeAtEnd 2
#define ContigAtStart 4
#define ContigAtEnd 8
#define SuperStringEntry 16

#define Virtual 32	/* for strings, locns */
#define UnMapped 64
#define DeadEnd 128	/* for strings, segments, links, locns */
#define HasLink 256
#define HasTie 512
#define HasLocn 1024
#define LinkFlag 512

/* Flags for segments */
#define Gap 32
#define SegmentFixed 4096
#define PartiallyFixed 8192
#define FixedFlag 16384

#define Move_to 16384
#define Line_to 32768
#define _Rel 8192
#define _Point (Move_to|Line_to)
#define Move_to_rel (Move_to|_Rel)
#define Line_to_rel (Line_to|_Rel)
#define VerticalShot 1
#define DesigLen 4
#define NameLength 64

#define fLink 512

#define StationPrimeRadix 1000		/* factor by which primes are incremented. */

/* Flags for personnel */
#define LastName 1
#define Job 2
#define LastInParty 4
#define Leader 8
#define Book 16
#define Compass 32
#define Tape 64
#define PointPerson 128
#define Sketch 256
#define Backsights 512
#define Foresights 1024
#define Inclinometer 2048

/*----------------------------- Glues ---------------------------------*/

#define Sys6Glues

/*#ifdef Sys6Glues */
/*#define StandardGetFile glueStandardGetFile */
/*#define StandardPutFile glueStandardPutFile */
/*#define FSMakeFSSpec glueFSMakeFSSpec */
/*#endif */

/*----------------------------- Errors ---------------------------------*/

#define OutOfMemory -1
#define DiskErr -2
#define PagingError -3

/*----------------------------- TypeDefs ---------------------------------*/


typedef char Desig[DesigLen];
typedef unsigned short indexType;
typedef short coordType;
typedef extended c_float;

typedef void(*EventMethod)(WindowPtr, EventRecord *) ;
typedef void(*WindowMethod)(WindowPtr) ;
typedef void(*RectMethod)(WindowPtr, Rect *) ;
typedef int(*IntMethod)(WindowPtr) ;

/*-------------------- Scrolling Document Record -------------------*/

typedef struct {
	DialogRecord 	docWindow;
	ControlHandle	hScroll;
	ControlHandle	vScroll;
	short			hOffset,vOffset;
	Handle			docData;
	OSType			fDocType;
	short			windResource;
	short			dataPathRefNum;
	short			resourcePathRefNum;
	Boolean			fFileOpen;
	Boolean			fDocWindow;
	Boolean			fNeedtoSave;
	Boolean			fNeedtoUpdateMarks;
	Boolean			fNeedtoSaveAs;
	Boolean			fHaveSelection;
	Boolean			fCanUndo;
	Boolean			fReadOnly;
	ProcPtr			docClik;
	WindowPtr		homeWindow;
	WindowPtr		associatedWindow;
	Handle			marks;
	Handle			windowState;
	MenuHandle		markMenu;
	long			refCon;
	THPrint			fPrintRecord;
	FSSpec			fileSpecs;
	Rect			limitRect;

	Boolean (*makeWindow)(WindowPtr);
	Boolean (*initDoc)(WindowPtr);
	Boolean (*openDocFile)(WindowPtr);
	WindowMethod	destructor;
	
	/* Event actions */
	EventMethod		doTheUpdate;
	void(*draw)(WindowPtr,Rect *,short);
	EventMethod		doActivate;
	WindowMethod	activate;
	WindowMethod	deactivate;
	
	EventMethod		doContent;
	EventMethod		doKeyDown;
	WindowMethod	doIdle;
	void(*adjustCursor)(WindowPtr,Point);
	EventMethod		doGrow;
	void(*doZoom)(WindowPtr,short);
	
	/* Edit menu and clipboard functions */
	Boolean (*doDocMenuCommand)(WindowPtr,short,short,short);
	WindowMethod	adjustDocMenus;
	WindowMethod	doCut;
	WindowMethod	doCopy;
	WindowMethod	doPaste;
	WindowMethod	doClear;
	WindowMethod	doSelectAll;
	WindowMethod	doUndo;

	IntMethod		readDocFile;
	IntMethod		writeDocFile;
	IntMethod		writeDocResourceFork;

	WindowMethod	doPageSetup;
	WindowMethod	doPrint;
	
	/* Scrolling methods */
	WindowMethod	focusOnContent;
	WindowMethod	focusOnWindow;
	WindowMethod	adjustScrollBars;
	short(*getVertSize)(WindowPtr);
	short(*getHorizSize)(WindowPtr);
	short(*getVertLineScrollAmount)(WindowPtr);
	short(*getHorizLineScrollAmount)(WindowPtr);
	short(*getVertPageScrollAmount)(WindowPtr);
	short(*getHorizPageScrollAmount)(WindowPtr);
	
	WindowMethod	displaySelection;
	
	void(*scrollContents)(WindowPtr,short,short);
	WindowMethod	setScrollBarValues;
	RectMethod		getContentRect;
	
} DocumentRecord, *DocumentPeek;

typedef struct {
	long selStart;
	long selEnd;
	char label;
} MarkRec, *MarkRecPtr;

typedef struct {		/* MPSR 1005 */
	short fontSize;
	char fontName[32];
	short fontWidth;	/* 0006 */
	short tabWidth;		/* 0004 */
	Rect userState;
	Rect stdState;
	unsigned long modifiedDate;
	long selStart;
	long selEnd;
	long vScrollValue;
	unsigned char wordWrap;	
	unsigned char showInvisibles;	
} MPSRWindowResource, *MPSRPtr;

typedef struct LButton
	{
	Rect bounds;
	char name[32];	/* in pascal string format */
	int value;
	struct LButton *next;
	} LButton;
	
typedef struct Transform {
	extended dx,dy,dz;	/* Offset */
	extended x,y,z;		/* Rotation angles in radians */
	extended mag;		/* Magnification */
	extended r[6];		/* 3x2 submatrix, rotation terms. */
	extended t[2];		/* translation terms. */
} Transform;
	
typedef struct MasterRec {
	void *p;
	void **h;
	long other;
} MasterRec;
	

typedef struct Bounds {
	short what,who;
	Rect bounds;
} Bounds;

typedef struct locnRec {
	unsigned short code,length;		/* 4 bytes */
	float x,y,z;				/* 12 bytes */
} Locn;

typedef struct stationRec {
	short survey,n; 
} Station;

typedef struct linkRec {
	struct stationRec from,to;
	long data; 
} Link ;

typedef struct dEntryMap
{
	short flags;
	short spareFlags;
	short nStations;
	short resID;		/* of MPRd resource */
	short firstFields[20];
	short offsets[20];
	short nFields;
	char  fields[1];
} DEntryMap, *DEMapPtr, **DEMapHandle ;

typedef struct {
	short	DlogID;
	char	flag;
	char	spareFlag;
	char	nStations;
	char	nColumns;
	char	colDescriptors[1];
} MRPdType, *MRPdPtr, **MRPdHandle;
	
typedef struct FileStuff
{
	coordType XMapCtr, YMapCtr;	/* Real-world coordinates of bitmap center. Initially 0. */
	short xBMCtr,yBMCtr;		/* Graphport coords. of BM Center. 512 unless Map smaller than BM. */
	short DhBM,DvBM;			/* Offset of window in BM. */

	DocumentPeek doc;
	
	int argv;
	Str255 params;
	Str31 version;
	long minField,maxField;
	Handle fieldList;
	int literals;
	long nameHashArray[37];
	int nMasters;
	int nLongLists;
	MasterRec masters[NMasters];
	Handle longLists;
	Handle intData;		/* Integer form of coordinate data */
	Handle penData;		/* For each intData, a short encodes passage size */
	Handle boundsData;	/* Bounding rect data for each FSB and survey */
	Rect bounds;		/* Bounds of map, in real-world coordinates */
	short drawMore;		/* Flag and count of how many more surveys to draw to BM  */
	Station selected,selectedTie;
	Rect selectedBounds;
	short selectedFSB,tieFSB,sequence;
	char selectedDesig[4];
	char tieDesig[4];
	
	short gain;			/* log base 2 of transform->mag */
	Handle bitHandle;
	GWorldPtr offscreen;
	Transform transform;
	
/*   Rect GraphRect; */

	LButton reduceButton;
	LButton scaleButton;
	LButton enlargeButton;
} FileStuff;

typedef struct dimData {
	unsigned short l,r,c,f,azi;
} DimData;

typedef struct intData {
	unsigned short code;
	coordType x,y;
} IntData;

typedef struct listElement {	/* for R/W blocks, used in l(ong)Lists. */
	Handle handle;
	void *ptr;
	long position;
	unsigned long hit;   
} ListElement;

typedef struct lList {
	unsigned int current;		/* number of elements in list */
	unsigned int currentList;	/* index of listElement last used */
	unsigned long thisHit;		/* current hit number for this list */
	long other;     			/* for anything */
	size_t size;
	short maxE,nLE,nbuffers,nItems,faults;
	short code;					/* code for name of list */
	indexType master;			/* index into master pointers array */
	struct listElement *list;	/* array of pointers and positions */
	long firstList;
	/* 	More can be added here and will not be overwritten by lLists
		from CML */
} LongList;

struct cartData {			/* 12 bytes. */
	float x,y,z; };	

typedef struct segmentRec {
	long data;					/* index into locnRec data */
	indexType next;				/* index to next segment in this field (linked list) */
	indexType nextSubSeg;		/* index to next subsegment (linked list) */
	short first,last;			/* first and last points in segment */
	short flags;				/* attribute bits */
	short other;				/* unused */
	Desig desig;				/* segment name, up to four characters */
	struct cartData LSW,UNE;	/* extents */
	short top,bottom;			/* bounding stations */
	short east,west;
	short north,south;
	long length;				/* segment length in 10*basicunits */
	long LineCnt;				/* last input file line dealing with this segment */
	indexType header;
	short survey,FSB;		/* this segment's internal number (its index); FSB number */
	short age;					/* reflects ordering of first reference to a segment */
	short nStations;			/* number of points in segment */
	short nLines; 				/* number of input file lines describing segement */
	short nStrings;			/* number of stringRecs in segment */
	short nTies,nLinks,nLocns;	/* numbers of Ties, Links, and fixed Locations */
	short ownTies;				/* number of ties belonging to this segment. */
	short nJcts;				/* number of branch points */
	indexType links;			/* where linkRecs are */
	indexType strings;			/* where stringRecs are */
	indexType rows; 			/* index into rowRecs of least-squares closure network */
} Survey;

typedef struct strRec {
	short first,last,nFirst,nLast;		/* possibly add short segment */
	unsigned short length,code;
	indexType string;					/* self referencial */
} String;

typedef struct subSegRec {
	long data;
	indexType next,nextSubSeg;
	short first,last,flags,other;
} SubSurvey;

struct dbRec {
	indexType link;
	short what;
	long where,finish;
} ;

struct nameRec {
	char name[NameLength];
	long next;
} ;

struct fieldRec {
	struct cartData LSW,UNE;	/* Extents */
	struct cartData datumData;	/* local datum's data */
	short ThisField, plane;		/* ThisField is the section number given by the user,
									i.e. 212 in $212A1. Often, this is the same as
									plane, which is the z-plane sequence of the
									field. */
	short nSegments,nFids;		/* number of segments and fiduciary marks */
	indexType fidSegment;		/* index of segment containing fiduciary marks */
	indexType tops,bottoms;		/* unimplemented; indexes of segments containing top
									of section points and bottom of section points. */
	short lastField,nextField;	/* adjacent sections */
	float dimension;			/* section thickness */
	float z;					/* z-coordinate */
	float corrAngle,corrOffset;	/* angle and offset of fiduciary correction */
	struct stationRec localDatum;	/* identity of local datum */
};

struct headerRec {
	short segment,FSB;
	time_t report;
	indexType accession;		/* accession code. Index is into nameList. */
	indexType object;			/* name of reconstruction. Index is into nameList. */
	indexType location;			/* place for another reference to a nameList entry */
	indexType personnel;		/* meant for people but can be any kind of linked list of strings */
	indexType notes; 			/* index into yet another kind of ascii data collection */
	indexType project;			/* text describing project supported */
	indexType equipment;
	indexType hazards;
	indexType skills;
	indexType attributes;
	indexType personnelreport;
	indexType objective;
	indexType description;		/* etc, for trip reports */
	indexType data;
	indexType route;
	unsigned short in;			/* time in minutes past midnight */
	unsigned short out;			/* time in minutes past midnight */
	unsigned short PMUG;		/* duration in people-minutes */
	unsigned short newlength;
	unsigned short replacedlength;
	time_t date;				/* a date */
	short decl,fcc,bcc,fic,bic;	/* angle corrections in 1/10 degrees */
	indexType compass;
	indexType inclinometer;
} ;

struct AEinstalls {
    AEEventClass theClass;
    AEEventID theEvent;
    EventHandlerProcPtr theProc;
};
typedef struct AEinstalls AEinstalls;

/*--------------------------- Declarations --------------------------------*/
/* Globals from CML/MDL */

extern char *ErrorTable[];
extern LongList *DataList;
extern LongList *LocnList;

extern short SEFlag;
extern short MeterFlag;
extern short ProcessHeaders;
extern short NoFSBFlag;
extern short UsingStrings;
extern short DoP2;
extern short Super3D;
extern short GraphHow;
extern short WarningFlag;
extern short VdFlag;
extern short ShotMode;
extern short ShotFlag;			/* usually Line_to_rel */
extern short ListingFlag;		/* flag for listing */
extern short FullListing;
extern short ExchangeFormat;
extern short ParseAll;
extern short DumpDBFlag;
extern short CloseFlag;
extern short OutputFlag;
extern short LogFlag;	/* 1: Index, 2: Log, 4: Chains */
extern short ExcludeFlag;
extern short Silent;
extern short VirtualFlag;
extern short SnapShot;
extern short EllipseFlag;
extern short UsingFids;
extern short InchFlag;
extern short AziCorrected;
extern short InclCorrected;
extern short SchematicFlag;
extern short HorizontalDistanceFlag;
extern short StreamIsFile;

extern FILE *OutFile;

/* In View.c */

extern void CorrectDialog(StringPtr,StringPtr);
extern void Correct2Dialog(StringPtr,StringPtr,StringPtr,StringPtr,StringPtr);
extern void FatalError(void);
extern void errorAlertDialog(int);
extern pascal void doMessage(short);
extern WindowPtr FrontLayer(void);
extern void AdjustMenus(WindowPtr);
extern WindowPtr TargetWindow(void);
extern WindowPtr OpenNamedFile(StringPtr);
extern int ConfirmDialog(short,StringPtr);
extern int RequestDialog(short,char *);
extern void doDiagnosticMessage(short, short);

#ifdef Sys6Glues
extern pascal OSErr glueFSMakeFSSpec(short,long,ConstStr255Param,FSSpecPtr);
extern pascal void glueStandardGetFile(FileFilterProcPtr, short, SFTypeList, StandardFileReply *);
extern pascal void glueStandardPutFile(ConstStr255Param,ConstStr255Param, StandardFileReply *);
#endif
extern void InsertMark(DocumentPeek,int,int,Str63);
extern ListHandle FillMarkList(DocumentPeek,WindowPtr,int,int);
extern void DoUnmark(DocumentPeek,int,int);
extern void FillMarkMenu(DocumentPeek);

/* In ViewRouts.c */
extern QDErr NewOffscreenBitMap(GrafPtr *,Rect *,Handle *);
extern int readFile(char *,DocumentPeek,FileStuff *);

extern GWorldPtr prepareGWorld(GWorldPtr,short,short,int,Handle *);
extern void LockLists(FileStuff *);
extern void UnlockLists(FileStuff *);
extern void DisposeLists(FileStuff *);
void cvTempRead(short, void *,long,long);
extern void *getCVLListBuf(int,FileStuff *,int);

extern void SelectObject(Point,WindowPtr);
/*extern ListHandle FillList(FileStuff *,WindowPtr); */
extern int FillBounds(FileStuff *);
extern void HiliteSurvey(Survey *,FileStuff *,int);
extern int DrawSurvey(Survey *,FileStuff *,int);
extern void OpenTextWindow(WindowPtr,int);
extern void doSelectDialog(WindowPtr);
extern void UpdateList (ListHandle);
extern pascal Boolean listFilter(DialogPtr,EventRecord *,short *);
void UpdateMarks(DocumentPeek,long,long,long,long);
extern void PrintBuf(StringPtr,long,DocumentPeek);

/* In ViewDocs.c */
extern void SetDocWindowTitle(WindowPtr,StringPtr);
extern void ShowDocWindow(WindowPtr);
extern void CloseDocFile(DocumentPeek);
extern Boolean openDocFile(WindowPtr);
extern void DeleteDoc(DocumentPeek);

extern DocumentPeek MakeDoc(OSType,FSSpecPtr);
extern int DoClose(WindowPtr);
extern void ScrollClick(WindowPtr,EventRecord *);
extern Boolean InitScrollDoc(WindowPtr);

extern void GetTERect(WindowPtr,Rect *);
extern Boolean SetTERect(WindowPtr);
extern void SizeScrollBars(WindowPtr);

extern DoSave(WindowPtr);
extern DoSaveACopy(WindowPtr);
extern DoSaveAs(WindowPtr);
extern int DoRevert(WindowPtr);
extern Boolean doDocMenuCommand(WindowPtr, short, short, short);

extern int mapDoClose(WindowPtr);
extern int textDoClose(WindowPtr);
extern void AddTextMemberFunctions(DocumentPeek);

extern int SizeToFit(FileStuff *,Rect);
extern void FillOffscreen(DocumentPeek,int);
/*extern void DrawMore(FileStuff *fStuff); */
/*extern void FillWindow(FileStuff *fStuff); */

extern void DrawLButton(LButton *);
extern int doLittleButtons(WindowPtr, Point);
extern pascal void doButton(DialogPtr,short);
extern pascal void doFrame(DialogPtr,short);
extern pascal void doLine(DialogPtr,short);
extern void doPageSetup(WindowPtr);
extern void mapDoPrint(WindowPtr);

extern pascal void ScrollAction(ControlHandle,short);

extern DocumentPeek doDataDialog(void);
extern void InitToolWind(void);
extern void LocToMap(Point *,FileStuff *);
extern void MapToBM(Point *,FileStuff *);
extern void MapToLoc(Point *,FileStuff *);
extern void showMouseLoc(WindowPtr, Boolean);
extern void RectMapToLoc(Rect *,FileStuff *);
extern void setOffscreenPort(FileStuff *,int);
extern void unlockOffscreenBits(FileStuff *fStuff);
extern BitMap *getOffscreenBits(FileStuff *fStuff);
extern pascal void doButton(DialogPtr,short);
extern void AddPaneMemberFunctions(DocumentPeek);
extern void AddText(WindowPtr,Ptr, long);
extern void getContentRect(WindowPtr, Rect *);
extern void GetDocWindowTitle(WindowPtr,StringPtr);

extern void destructor(WindowPtr);
extern void activate(WindowPtr);
extern void deactivate(WindowPtr);
extern int doGrow(WindowPtr,EventRecord *);
extern void doZoom(WindowPtr,short);
extern void scrollDoZoom(WindowPtr,short);
extern void scrollDoGrow(WindowPtr,EventRecord *);
extern void doActivate(WindowPtr, EventRecord *);
extern void CombinedScroll(WindowPtr,short,short);

extern Handle Main(char *,long,char *);

extern void CloseResourceFile(DocumentPeek);
extern void CommandLineError(short);
extern OSErr GetVolRefNum(char *,short *);
extern pascal Boolean DialogStandardKeyDown(DialogPtr, EventRecord *, short *);
extern char *ConditionFileName(char *);

extern Handle TE32KScrpHandle;
extern long TE32KScrpLength;
extern c_float DtoR;
extern c_float RtoD;

/* in CVEntry.c */
extern void ToggleDialog(WindowPtr, short);
extern DocumentPeek MakeModelessDialog(long);
extern void doConfigureDialog(WindowPtr);

/*------------------------------ Macros ---------------------------------*/

#define getData(x) ((Locn *) getCVLListBuf((int)(x),fStuff,nDataList)+(((int)(x))&BigBlockLo))
#define getLocn(x) ((Locn *) getCVLListBuf((int)(x),fStuff,nLocnList)+(((int)(x))&BlockLo))
#define getDims(x) ((DimData *) getCVLListBuf((int)(x),fStuff,nDimList)+(((int)(x))&BlockLo))
#define getSurvey(x) ((Survey *) getCVLListBuf((int)(x),fStuff,nSegList)+(((int)(x))&BlockLo))
#define getSubSurvey(x) ((SubSurvey *) getCVLListBuf((int)(x),fStuff,nSubSegList)+(((int)(x))&BlockLo))
#define getLink(x) ((Link *) getCVLListBuf((int)(x),fStuff,nLinkList)+(((int)(x))&BlockLo))
#define getStr(x) ((String *) getCVLListBuf((int)(x),fStuff,nStringList)+(((int)(x))&BlockLo))
#define getName(x) ((struct nameRec *) getCVLListBuf((int)(x),fStuff,nNameList)+(((int)(x))&BlockLo))
#define getPlace(x) ((struct nameRec *) getCVLListBuf((int)(x),fStuff,nNameList)+(((int)(x))&BlockLo))
#define getDb(x) ((struct dbRec *) getCVLListBuf((int)(x),fStuff,nDbList)+(((int)(x))&BlockLo))
#define getHeader(x) ((struct headerRec *) getCVLListBuf((int)(x),fStuff,nHeaderList)+(((int)(x))&BlockLo))

#define desigCpy(a1,a2) strncpy(a1,a2,DesigLen)

#define getAzi(str) (isalpha(*str))? cvQuadtof((StringPtr)str):atof(str)
#define isfdigit(s) (isdigit(*s)||*s=='-'||*s=='+'||*s=='.')
#define isbdigit(s) (isfdigit||tolower(*s)=='n'||tolower(*s)=='s'||tolower(*s)=='e'||tolower(*s)=='w')

#define ScaleToBM(x,g) (((g)<0)?(x)>>(-g):(x)<<(g))

#define sizeofMark(m) (9+((MarkRecPtr)(m))->label)
#define setDocName(doc,str) BlockMove(str,(doc)->fileSpecs.name,*str+1)
#define pascstr(str) (*((str)+*(str)='\0',(str)+1)
#define isfullpath(str) (*(str)!=':' && strchr((str)+1,':'))
#define DialogStandardFilter DialogStandardKeyDown

/*----------------------------- Method Macros ---------------------------------*/

#define AdjustScrollBars(w) (*((DocumentPeek)(w))->adjustScrollBars)(w)
#define ScrollContents(w,h,v) (*(((DocumentPeek)w)->scrollContents))(w,(h),(v))
#define DoActivate(w,e) (*((DocumentPeek)(w))->doActivate)((WindowPtr)(w),(e))
#define DoContent(w,e) (*((DocumentPeek)(w))->doContent)((WindowPtr)(w),(e))
#define DoTheUpdate(w,e) (*((DocumentPeek)(w))->doTheUpdate)((WindowPtr)(w),(e))
#define SetScrollBarValues(w) (*((DocumentPeek)(w))->setScrollBarValues)(w)
#define OpenDocFile(w) (*((DocumentPeek)(w))->openDocFile)(w)
#define WriteDocFile(w) (*((DocumentPeek)(w))->writeDocFile)(w)
#define ReadDocFile(w) (*((DocumentPeek)(w))->readDocFile)(w)
#define FocusOnContent(w) (*((DocumentPeek)(w))->focusOnContent)(w)
#define FocusOnWindow(w) (*((DocumentPeek)(w))->focusOnWindow)(w)
#define DisplaySelection(w) (*((DocumentPeek)(w))->displaySelection)(w)

#define SetMenuAbility(m,i,b) (b)?EnableItem(m,i):DisableItem(m,i)
#define RectWidth(r) (r).right-(r).left
#define RectHeight(r) (r).bottom-(r).top
#define GetContentRect(w,r) (*(((DocumentPeek)w)->getContentRect))(w,r) 
#define GetVertSize(w) (*(((DocumentPeek)w)->getVertSize))(w)
#define GetHorizSize(w) (*(((DocumentPeek)w)->getHorizSize))(w)

#else
#pragma load "CVHeaders"
#endif

#endif
