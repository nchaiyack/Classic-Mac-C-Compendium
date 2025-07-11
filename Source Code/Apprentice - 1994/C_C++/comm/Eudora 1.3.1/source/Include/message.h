/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * structure to describe message
 **********************************************************************/
typedef struct
{
	TOCType **tocH; 		/* the table of contents to which this message belongs */
	int sumNum; 				/* the summary number of this message's summary */
	TEHandle txes[HEAD_LIMIT];	/* handles for the various message fields */
	Boolean txRo[HEAD_LIMIT]; 	/* whether or not given field is ro */
	MyWindowPtr win;		/* window I'm displayed in */
	Boolean dirty;			/* whether or not message is dirty */
	long weeded;					/* number of header bytes "weeded" out */
	ControlHandle sendButton; 		/* the send button */
	STEHandle stes[2];	/* STE's for incoming windows */
	Handle next;				/* next message in the list */
} MessType, *MessPtr, **MessHandle;

/**********************************************************************
 * prototypes
 **********************************************************************/
int ReadMessage(TOCType **tocH,int sumNum,UPtr buffer);
int MoveMessage(TOCType **tocH,int sumNum,long dirId,UPtr toWhich,Boolean copy);
int MoveSelectedMessages(TOCType **tocH,long dirId,UPtr toWhich,Boolean copy);
void DeleteMessage(TOCType **tocH, int sumNum);
int MessageError(void);
void DoIterativeThingy(TOCType **tocH,int item,long modifiers,short toWhom);
MyWindowPtr DoReplyMessage(MyWindowPtr win,long modifiers,short toWhom,Boolean vis);
short DoRedistributeMessage(MyWindowPtr win,short toWhom);
short DoForwardMessage(MyWindowPtr win,short toWhom);
short DoSalvageMessage(MyWindowPtr win);
void SetMessText(MessType **messH,short whichTXE,UPtr string,short size);
void AppendMessText(MessType **messH,short whichTXE,UPtr string,short size);
void PrependMessText(MessType **messH,short whichTXE,UPtr string,short size);
short BoxNextSelected(TOCHandle tocH,short afterNum);
void QuoteLines(MessType **messH,short whichTXE,int from,int to,short pfid);
void MakeMessTitle(UPtr title,TOCType **tocH,int sumNum);
MyWindowPtr GetAMessage(TOCType **tocH,int sumNum,MyWindowPtr win, Boolean showIt);
MyWindowPtr ReopenMessage(MyWindowPtr win);
#define MAX_HEADER 64
#define Win2MessH(win) ((MessHandle)win->qWindow.refCon)
#define SumOf(mH) (&(*(*mH)->tocH)->sums[(*mH)->sumNum])
#define BodyOf(mH) ((*mH)->txes[BODY])
#define Win2Body(win) BodyOf(Win2MessH(win))


