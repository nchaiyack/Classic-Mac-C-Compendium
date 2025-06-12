/*
*********************************************************************
*	
*	EMBL_Search.h
*	Main header file
*		
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*		
**********************************************************************
*	
*/ 

#pragma once

#include "general.h"

/*
*	symbols
*/

#define kApplSignature	'DLCD'
#define kResFileType		'TEXT'
#define kQryFileType		'QuRy'
#define kPrefFileType	'CDpr'
#define kSeqFileType		'TEXT'

#define kQueryRsrcType	'QRY '
#define kQueryRsrcName	"\pQuery"
#define kHitmapRsrcType	'HITS'
#define kHitmapRsrcName	"\pHitmap"
#define kStdRsrc			128

#define kNameStringRsrcTmpl	300
#define kNameStringRsrc			-16396
#define kNameStringRsrcType	'STR '
#define kNameStringRsrcName	"\pName string"

#define kHelpTextRsrc	128
#define kHelpPictRsrc	128

#define ENTRYNAMELEN	10		/* maximum length of entry names */
#define DIVNAMELEN	12 	/* maximum length of division file names */
#define ACCNOLEN		6		/* maximum length of an accession number */

#define FILEBUF 4096L		/* buffer size for read/write buffer */

#define MAXBUFLINES 260		/* No of lines in results short description buffer */
									/* Must always be greater than max. no of lines
										which can be displayed on a screen */
#define SEQBUFLINES 1000	/* lines in sequence buffer */
#define BROWSEBUFLINES	100

#define MAXBUFPARTS	30		/* number of sequence buffer chunks.
										( less than  (USHRT_MAX/2)/SEQBUFLINES)  */ 

#define MAXHITS		16000L	/* max number of hits. We use the # of hits to 
										update the vertical scroll bar of results windows,
										and it must not be larger than SHRT_MAX */

#define MAXWIN			20		/* Maximum of windows we allow. We allocate memory
										for the windows at startup to avoid heap
										fragmentation */

#define MAXSUBQRY	5			/* Max. # of query sentences */

#define MENUNR		10			/* total number of menus and array indices */
#define APPLE		0			
#define FILE		1
#define EDIT		2
#define OSTUFF		3
#define PREFS		4
#define WINDOWS	5
#define CREATOR	6
#define FORMAT		7
#define XREF		8
#define POPUP		9


#define DB_NUM		2			/* Database codes */
#define DB_EMBL	0
#define DB_SWISS	1

#define EMBL_FORMAT		1	/* sequence format definitions */
#define STADEN_FORMAT	2
#define PIR_FORMAT		3
#define FASTA_FORMAT		4

#define DBNAMELEN	20			/* length of database names */
#define DBRELNUMLEN 10		/* length of release number string */

#define RES_MARGIN	4		/* left and right text margin in result windows */
#define SEQ_MARGIN	4		/* left and right text margin in sequence windows */

#define MAXCOL		80			/* Maximum columns in a window */

#define ENAME_QRY		1
#define ACCNUM_QRY	2
#define KEYWORD_QRY 	3
#define FREETEXT_QRY 4
#define AUTHOR_QRY	5
#define TAXON_QRY		6

#define WILDCARD1 '*'		/* wild card character (fuzzy end) */

#define BOOLEAN_NOOP	0
#define BOOLEAN_AND	1
#define BOOLEAN_OR	2


/*
*	General type definitions.
*	Pascal strings are of type Str255, C strings of type char []
*/

typedef char CString80[81];
typedef CString80 *CString80Ptr, **CString80Hdl;

typedef long **HitmapHdl;

typedef struct {
	short	dbcode;
	char	accno[ACCNOLEN+1];
	char	ename[ENTRYNAMELEN+1];
} XRef, *XRefPtr, **XRefHdl;

typedef struct {				/* date information */
	u_byte dummy;
	u_byte year;
	u_byte month;
	u_byte day;
} Date;

typedef struct {				/* volume information */
	Str255	volName;
	short		vRefNum;
} VolInfo;

typedef struct {				/* if index files are moved to a hard disk we store
										the volume name and the dirID of the new folders */
	Str27	volName;
	long	dirID;
}	DirSpec;
	
	
/*
*	Global preferences
*/

typedef struct {
	DirSpec	inxDirSpec[DB_NUM];			/* path information to index files folders */
	short		format;							/* selected output format 				*/
	short		creator;							/* sequence file creator */
	Str31		creatorName;					/* file name of creator */
	OSType	creatorSig;						/* creator signature */
	Boolean	confirmChg;						/* confirm changes before closing */
	Boolean	startQOpen;						/* open application with empty query screen */
} Prefs;


/*
*	Index files
*/

typedef struct {						/* database information */
	Str255	SeqDName;				/* name of directory that contains sequence data files */
	short		SeqWDRefNum;			/* working directory id of directory that contains
												sequence data files */
	Str255	InxDName;				/* name of directory that contains index files */
	short		InxWDRefNum;			/* working directory id of directory that contains
												index files */
	char		DBName[DBNAMELEN+1];	/* Database name */
	char		DBRelNum[DBRELNUMLEN+1]; /* Release number */
	char		DBRelDate[50];			/* Release date */
	char 		**gDivNames;			/* Handle to array of division names */
	short		ndiv;						/* no of database division files */
	u_long	ename_nrec;				/* no of records in entryname index */
	u_long	actrg_nrec;				/* no of records in acc# target file */
	u_long	achit_nrec;				/* no of records in acc# hit file */
	u_short	actrg_recsize;
	u_long   kwtrg_nrec;				/* no of records in keyword target file */
	u_long	kwhit_nrec;				/* no of records in keyword hit file */
	u_short	kwtrg_recsize;
	u_long	texttrg_nrec;			/* no of records in freetext target file */
	u_long	texthit_nrec;			/* no of records in freetext hit file */
	u_short	texttrg_recsize;
	u_long	authortrg_nrec;		/* no of records in author target file */
	u_long	authorhit_nrec;		/* no of records in author hit file */
	u_short	authortrg_recsize;
	u_long	taxontrg_nrec;			/* no of records in taxonomy target file */
	u_long	taxonhit_nrec;			/* no of records in taxonomy hit file */
	u_short	taxontrg_recsize;
} DBInfo;

typedef struct {						/* list of file names */
	Str255 divFName;					/* Division lookup table file name */
	Str255 enameIdxFName;			/* Entryname index file name */
	Str255 acnumTrgFName;			/* Accession number target file name */
	Str255 acnumHitFName;			/* Accession number hit file name */
	Str255 briefIdxFName;			/* Short description file name */
	Str255 keywTrgFName;
	Str255 keywHitFName;
	Str255 textTrgFName;
	Str255 textHitFName;
	Str255 authorTrgFName;
	Str255 authorHitFName;
	Str255 taxonTrgFName;
	Str255 taxonHitFName;
} IndexFiles;

typedef struct {				/* index file header */
	u_long	file_size;
	u_long	nrecords;
	u_short	record_size;
	char		db_name[DBNAMELEN];
	char		db_relnum[DBRELNUMLEN];
	Date		db_reldate;
	u_byte	free_space[256];
} Header;

typedef struct {				/* division index record */
	u_short	div_code;
	char 		filename[12];
} DivisionRec;

typedef struct {				/* entry name index record */
	char		entry_name[10];
	u_long	annotation_offset;
	u_long	sequence_offset;
	u_short	div_code;
} EnameRec;

typedef struct {				/* short description index record */
	char		entryname[10];
	char		accno[10];
	u_long	seqlen;
	char		description[80];
} DescRec;

typedef u_long HitRec;		/* generic hit file record */

typedef struct {				/* generic target file record */
	u_long	nhits;
	u_long	hitPtr;
	char		value[];
} TargetRec;


/*
*	Windows
*/

#define queryW	dialogKind
#define resW	userKind
#define seqW	userKind+1


/*
*	Window description structure
*/

typedef struct {
	DialogRecord	dRec;				/* dialog record for query windows; for seq and
												res windows we simply treat it as a
												WindowRecord */
	Boolean			inUse;			/* indicates whether this window record is 
												currently in use */
	ControlHandle	vScroll;			/* vertical scroll bar handle */
	ControlHandle	hScroll;			/* horizontal scroll bar handle */
	ControlHandle	ctrl1;			/* other controls */
	ControlHandle	ctrl2;
	ControlHandle	ctrl3;
	ControlHandle	ctrl4;
	short				vRefNum;			/* volume reference nr of associated file */
	Str255			fName;			/* file name of associated file */
	Handle			userHandle;		/* Handle to user data */
	Boolean			dirty;			/* is contents of window saved to disk ? */
	Boolean			inited;			/* Does a file already exist?	*/
} WDRec, *WDPtr;


/*
*	window-specific data structures
*/

/* queries */

typedef struct {
	short		field;								/* indexed field */
	Str255	text;									/* query text (simple ASCII string) */
} QueryFld;

typedef struct {
	long		magic;								/* magic number to identify queries */
	short		dbcode;								/* database */
	char		DBRelNum[DBRELNUMLEN+1];		/* release */
	short		gBooleanOp;							/* global boolean operator */
	QueryFld	subQuery[MAXSUBQRY];				/* subqueries */
} QueryRec, *QueryPtr, **QueryHdl;


/* results window */

typedef struct {
	u_long ename_rec;					/* offset into ename index */
	struct {								/* diverse flags */
		unsigned char selected:1;
	} flags;
} HitlistRec, *HitlistPtr,**HitlistHdl;


typedef struct {
	short				dbcode;			/* database which was searched for these hits */
	short				nhits;			/* no of hits */
	short				nsel;				/* no of selected hits */
	HitmapHdl		hitmapHdl;		/* pointer to hitmap (hit bitmap) */
	HitlistHdl 		hlHdl;			/* handle to hitlist */
	CString80Hdl	descBufHdl;		/* handle to description buffer */
	short				buftop;			/* hit no of top line in description buffer */
	QueryHdl			queryHdl;		/* handle to query record */
} ResultRec, *ResultPtr, **ResultHdl;



/* sequence window */

typedef struct {
	Str255			fName;						/* name of division file */
	short				vRefNum;						/* working dir refnum of division file */
	long				bufpart[MAXBUFPARTS];	/* byte offsets of parts of sequence */
	short				dbcode;						/* database used */
	short				nlines;						/* # of lines of entry */
	short				buftop;						/* top line in buffer */
	long				seqStart;					/* first sequence line */
	CString80Hdl	lineBufHdl;					/* Handle to line buffer */
	XRefHdl			xrefHdl;						/* Handle to cross-reference buffer */
	short				firstSel;					/* first selected line in entry */
	short				lastSel;						/* last selected line in entry */
} SequenceRec, *SeqRecPtr, **SeqRecHdl;


typedef struct {
	CString80 text;
	long nhits;
} IndexLine;

typedef struct {
	Str255		fName;
	short			fd;
	short			dbcode;
	short			dbfield;
	IndexLine	buf[BROWSEBUFLINES];
	u_long		nrec;
	u_short		recsize;
	long			buftop;
	long			screentop;
	long			selected;
	short			scale;
} BrowseRec, *BrowseRecPtr,**BrowseRecHdl;

/*
*	Other structures
*/

typedef struct {
	short		num;						/* font id */
	FontInfo	finfo;					/* font sizes */
	short		height;					/* font height */
} FONTINFO;

typedef struct {
	long			when;					/* when did the last mouse-up event occur */
	Point			where;				/* where did the last mouse-down event occur (in global
												coordinates */
	WindowPtr	wPtr;					/* window in which it occurred */
} ClickInfo;


typedef enum {
	horizBar,vertBar
} scrollBarType;