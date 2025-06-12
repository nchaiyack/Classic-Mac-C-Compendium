#define CUR_VERSION 0x01	/* File Header - Version 1 */
#define SIGNATURE   'GnCh'

typedef struct FileHeader {
	char version;
	long signature;
	long reserved[32];
} FileHeader;

#define  sqr(x) ((double)(x)*(double)(x))

#ifndef BOOK
#define BOOK "gnuchess.book"
#endif
#define pxx " PNBRQK"
#define qxx " PNBRQK"
#define rxx "12345678"
#define cxx "abcdefgh"

#define scrntoxy(x) (reverse?9-(x):(x))

/* coordinates within a square for the following are ([1,5],[1,3]) */
#define SQW (5)
#define SQH (3)
#define TAB (45)
#define VIR_C(s)  ( reverse ? 7-column(s) : column(s) )
#define VIR_R(s)  ( reverse ? 7-row(s) : row(s) )
#define VSQ_X(x)  ( reverse ? SQW + 1 - (x) : (x) )
#define VSQ_Y(y)  ( reverse ? SQH + 1 - (y) : (y) )
#define Vblack(s) ( ! ((VIR_C(s) + VIR_R(s)) % 2) )
/* Squares swapped */
#define Vcoord(s,x,y) \
	((SQW)*(VIR_C(s)))+(x),((SQH)*(7-VIR_R(s))+(y))
/* Squares and internal locations swapped */
#define VcoordI(s,x,y) \
	((SQW)*(VIR_C(s)))+(VSQ_X(x)),((SQH)*(7-VIR_R(s))+(VSQ_Y(y)))
/* Squares and internal rows swapped */
#define VcoordR(s,x,y) \
	((SQW)*(VIR_C(s)))+(x),((SQH)*(7-VIR_R(s))+(VSQ_Y(y)))

extern  char mvstr[5][6];
extern  long evrate;

extern short PositionFlag;
extern short coords;
extern short stars;
extern short rv;
extern short shade;
extern short preview;
extern short idoit;
extern short drawn;
extern short undo;
extern short anim;
extern short showvalue;
extern short newgame;
extern short getgame;
extern short wne;

extern  char buff[100];

#define WhiteMenu 200
#define BlackMenu 201

#define	On  TRUE
#define	Off  FALSE
#define	NotSpect  TRUE


extern Boolean	Finished;
extern CursHandle	ClockCursor;
extern CursHandle	ArrowCursor;
extern CursHandle	CrossCursor;
extern Rect	DragArea;
extern Rect	GrowArea;
extern MenuHandle	Menu;
extern WindowPtr	WindBoard, WindList, WindThink;
extern int	width,height;

extern short background;

extern Point	thePoint;
extern Rect	MinSize;
	
extern Rect	nameRec, ValueRec, ValueFRec,chessRectOff;

extern Rect	MsgRec, MsgFRec;
extern Rect	ThinkRec[2], ThinkFRec[2];
extern Rect 	ScoreRec, ScoreFRec;

extern int 	theScore;
extern Str255	Msg;

extern Str255	ThinkMove[2][30];
extern int		maxThink;

extern DragHandle	myDragStuff;
extern Point	pt;

extern Pattern blackPat, whitePat;

extern Rect	CaseRec, CaseFRec;
extern int		MouseX, MouseY;

extern int 	ref;
extern long	nbre;

extern  short saveBoard[64], saveColor[64];

extern int towho;

extern ListHandle List;


/* From draw.c */
void OffDrawCase(Rect *r, Pattern color);
Point CenterRect(Rect *r) ;
Rect * Rectangle(int xr, int yr, Rect *r);
int PieceToNum(int color, int type);
void PicInRect(Rect *rec, DragHandle DragStuff, Point *Pt);
void DrawPieces(int Num, int sq);
void DragFromTo(DragHandle Drag, Point PtFrom, Point PtTo);
void AnimePieces(int before,int after ,int d, int a, int anim);

/* From update.c */
pascal void UpdateGraphic(WindowPtr theWindow, int ItemNo);
pascal void UpdateChronos(WindowPtr theWindow, int ItemNo);
pascal void UpdateMsg(WindowPtr theWindow, int ItemNo);
pascal void UpdateThink(WindowPtr theWindow, int ItemNo);
pascal void UpdateValue(WindowPtr theWindow, int ItemNo);
pascal void UpdateCase(WindowPtr theWindow, int ItemNo);
pascal void UpdateListe(WindowPtr theWindow, int ItemNo);
void UpdateMenus(void);

