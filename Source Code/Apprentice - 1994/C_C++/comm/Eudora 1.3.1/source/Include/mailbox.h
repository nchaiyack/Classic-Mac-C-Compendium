/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * the basic TOC types
 **********************************************************************/
typedef enum {
	UNREAD=1,
	READ,
	REPLIED,
	REDIST, 
	UNSENDABLE, 
	SENDABLE, 
	QUEUED,
	FORWARDED,
	SENT,
	UNSENT,
	STATE_ENUM_LIMIT
} StateEnum;

typedef struct
{
	long offset;		/* byte offset in file */
	long length;		/* length of message, in bytes */
	int bodyOffset; /* byte where the body begins, relative to offset */
	StateEnum state;/* current state of the message */
	char date[32];	/* header items to keep handy */
	unsigned long seconds;	 /* the value of seconds when message arrived */
	unsigned long flags;		/* some binary values */
	Rect savedPos;	/* saved window position */
	Byte priority;	/* display as 1-5, keep as 1-200 */
	Byte origPriority;
	short tableId;	/* resid of xlate table to use (0 for none, -1 for default) */
	long spare[3];
	char from[64];	/* header items to keep handy */
	char subj[64];	/* header items to keep handy */
	int dateTrunc;	/* length to draw for date */
	int fromTrunc;	/* length to draw for from */
	Boolean selected;/* is it selected? */
	Handle messH; 	/* message structure (and window) if any */
} MSumType, *MSumPtr;

typedef struct
{
	short version;		/* TOC version number */
	short refN; 		/* path ref number of open file */
	Str31 name; 	 /* name of box */
	long dirId; 					/* memory ONLY--not valid on disk */
	short usedK;	/* number of Kbytes used by contents */
	short totalK; /* number of Kbytes total */
	long volumeFree;	/* free space on TOC volume */
	short needRedo;	/* min summary we need to look at */
	short maxValid;	/* max summary known to not need redraw */
	long waste[52];
	short vRef; 		/* volume ref # of box */
	short count;			/* number of messages described */
	Boolean dirty;	/* whether or not this toc needs to be written out */
	Handle next;		/* the next TOC in the chain */
	MyWindowPtr win;/* the window for this toc */
	short which;			/* which type of mailbox (in, out, or regular) */
	Boolean spareBoolean;
	MSumType sums[1]; 	/* summaries of the messages contained in block */
} TOCType, *TOCPtr, **TOCHandle;
#define FLAG_NBK				1 			/* message has never been saved */
#define FLAG_SIG				2 			/* DO add signature to this message */
#define FLAG_BX_TEXT		4 			/* binhex any text attachments */
#define FLAG_WRAP_OUT 	8 			/* send this message right away */
#define FLAG_KEEP_COPY	16			/* keep a copy of this message */
#define FLAG_TABS 			32			/* put tabs in the body of this message */
#define FLAG_OUT				(1<<27)	/* message was outgoing */
#define FLAG_ADDRERR		(1<<28) /* addressing error in outgoing message */
#define FLAG_ZOOMED 		(1<<29) /* was window zoomed when saved */
#define FLAG_ICON_BAR 	(1<<30) /* use icon bar in this message */
#define DEFAULT_TABLE (-1)
#define NO_TABLE 0
/**********************************************************************
 * function prototypes
 **********************************************************************/
Boolean BadMailboxName(long inDirId,UPtr name,Boolean folder,long *newDirId);
int BoxFClose(TOCHandle tocH);
int BoxFOpen(TOCHandle tocH);
void CalcSumLengths(TOCHandle tocH,int sumNum);
TOCHandle CheckTOC(long dirId,UPtr boxName);
void DeleteSum(TOCHandle tocH,int sumNum);
TOCHandle FindTOC(long dirId,UPtr name);
int FlushTOCs(Boolean andClose,Boolean canSkip);
TOCHandle GetInTOC(void);
int GetMailbox(long dirId, UPtr name, Boolean showIt);
Boolean GetNewMailbox(long inDirId, UPtr name, Boolean *folder,long *newDirId,Boolean *xfer);
TOCHandle GetOutTOC(void);
void InvalSum(TOCHandle tocH,int sum);
TOCHandle ReadTOC(long dirId,UPtr boxName);
int RemoveMailbox(long inDirId,UPtr name);
int RenameMailbox(long inDirId,UPtr oldName, UPtr newName, Boolean folder);
Boolean SaveMessageSum(MSumPtr sum,TOCHandle tocH);
void SetState(TOCHandle tocH, int sumNum, int state);
void SortTOC(TOCHandle tocH, int (*compare)());
int SumFromCompare(MSumPtr sum1, MSumPtr sum2);
int SumSubjCompare(MSumPtr sum1, MSumPtr sum2);
int SumTimeCompare(MSumPtr sum1, MSumPtr sum2);
int RevSumFromCompare(MSumPtr sum1, MSumPtr sum2);
int RevSumSubjCompare(MSumPtr sum1, MSumPtr sum2);
int RevSumTimeCompare(MSumPtr sum1, MSumPtr sum2);
int SumStatCompare(MSumPtr sum1, MSumPtr sum2);
int RevSumStatCompare(MSumPtr sum1, MSumPtr sum2);
int SumPriorCompare(MSumPtr sum1, MSumPtr sum2);
int RevSumPriorCompare(MSumPtr sum1, MSumPtr sum2);
TOCHandle TOCByName(long dirId,UPtr name);
int WriteTOC(TOCHandle tocH);
void CalcAllSumLengths(TOCHandle toc);
Boolean MessagePosition(Boolean save,MyWindowPtr win);
void TooLong(UPtr name);
short FindDirLevel(long dirId);
void AddBoxCountMenu(MenuHandle mh, short item, long dirId);
void Rebox(void);
short GetTOCK(TOCHandle tocH,short *usedK, short *totalK);
void CleanseTOC(TOCHandle tocH);
Boolean GetTransferParams(short menu,short item,long *dirId,UPtr name);
void TempWarning(UPtr filename);
