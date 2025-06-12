typedef	struct
{
	long	top,left,bottom,right;
	
} LongRect;


typedef	struct
{
	long	h,v;
	
} LongPoint;



typedef pascal Boolean (*TE32KProcPtr)(void);



typedef	struct
{
	LongRect			destRect;
	LongRect			viewRect;
	int					lineHeight;
	int					fontAscent;
	LongPoint			selPoint;
	long				selStart;
	long				selEnd;
	int					active;
	TE32KProcPtr		clikLoop;
	long				clickTime;
	long				clickLoc;
	int					clikStuff;
	long				caretTime;
	int					caretState;
	long				teLength;
	Handle				hText;
	int					txFont;
	char				txFace;
	int					txMode;
	int					txSize;
	int					tabWidth;
	int					tabChars;
	int					maxLineWidth;
	int					crOnly;
	GrafPtr				inPort;
	long				nLines;
	int					theCharWidths[256];
	long				lineStarts[];
	
} 	TE32KRec,*TE32KPtr,**TE32KHandle;


	
extern	void			SetLongRect(LongRect *,long,long,long,long);
extern	void			LongRectToRect(LongRect *,Rect *);
extern	void			RectToLongRect(Rect *,LongRect *);
extern	void    		OffsetLongRect(LongRect *, long, long);

extern	void			TE32KInit(void);
extern	TE32KHandle		TE32KNew(LongRect *,LongRect *);
extern	void			TE32KDispose(TE32KHandle);
extern	void 			TE32KCalText(TE32KHandle);
extern	void			TE32KSetText(Ptr,long,TE32KHandle);
extern	void			TE32KUseTextHandle(Handle,TE32KHandle);
extern	Handle			TE32KGetText(TE32KHandle);
extern	void			TE32KUpdate(LongRect *,TE32KHandle);
extern	void			TE32KScroll(long,long,TE32KHandle);
extern	void			TE32KActivate(TE32KHandle);
extern	void			TE32KDeactivate(TE32KHandle);
extern	void			TE32KIdle(TE32KHandle);
extern	void			TE32KKey(unsigned char,TE32KHandle);
extern	void			TE32KClick(Point,unsigned char,TE32KHandle);
extern	void			TE32KSetSelect(long,long,TE32KHandle);
extern	OSErr			TE32KToScrap(void);
extern	OSErr			TE32KFromScrap(void);
extern	void			TE32KCopy(TE32KHandle);
extern	void			TE32KCut(TE32KHandle);
extern	void			TE32KDelete(TE32KHandle);
extern	void			TE32KInsert(Ptr,long,TE32KHandle);
extern	void			TE32KPaste(TE32KHandle);
extern	Handle			TE32KScrapHandle(void);
extern	long			TE32KGetScrapLen(void);
extern	void			TE32KSetScrapLen(long);
extern	void			TE32KGetPoint(long,LongPoint *,TE32KHandle);
extern	long			TE32KGetOffset(LongPoint *,TE32KHandle);
extern	void			TE32KSelView(TE32KHandle);
extern	void			TE32KSetFontStuff(int,int,int,int,TE32KHandle);
extern	void			TE32KAutoView(char, TE32KHandle);