/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * some linked-list macros
 **********************************************************************/
#define LL_Remove(head,item,cast) 																			\
	do {																																	\
	if (head==item) head = (*head)->next; 																\
	else																																	\
		for (M_T1=(uLong)head; M_T1; M_T1=(uLong)(*cast M_T1)->next)				\
		{ 																																	\
			if ((*cast M_T1)->next == item) 																	\
			{ 																																\
				(*cast M_T1)->next = (*item)->next; 														\
				break;																													\
			} 																																\
		} 																																	\
	} while (0)

#define LL_Push(head,item)																							\
	M_T1 = (uLong) ((*(item))->next = head, head = item)

/**********************************************************************
 * declarations for functions in util.c
 **********************************************************************/
Handle NewZHandle(long size);
void WriteZero(UPtr pointer,long size);
void MacInitialize(int masterCount,long ensureStack);
UPtr GetRString(UPtr theString,int theIndex);
UPtr PtoCcpy(UPtr cStr, UPtr pStr);
GetFontID(UPtr theName);
Boolean GrabEvent(EventRecord *theEvent);
void CheckFontSize(int menu,int size,Boolean check);
void CheckFont(int menu,int fontID,Boolean check);
void OutlineFontSizes(int menu,int fontID);
int GetLeading(int fontID,int fontSize);
int GetWidth(int fontID,int fontSize);
int GetDescent(int fontID,int fontSize);
int GetAscent(int fontID,int fontSize);
Boolean IsFixed(int fontID,int fontSize);
UPtr PCat(UPtr string,UPtr suffix);
void AwaitKey(void);
void AddPResource(UPtr,int,long,int,UPtr);
void ChangePResource(UPtr theData,int theLength,long theType,int theID,UPtr theName);
long GetRLong(int index);
int ResourceCpy(short toRef, short fromRef,long type,int id);
void WhiteRect(Rect *r);
int striscmp(UPtr s1,UPtr s2);
void DrawTruncString(UPtr string,int len);
int CalcTextTrunc(UPtr text,short length,short width,GrafPtr port);
#define CalcTrunc(text,width,port) CalcTextTrunc((text)+1,*(text),width,port)
void TrimWhite(PStr s);
void TrimInitialWhite(PStr s);
int WannaSave(MyWindowPtr win);
#ifdef	KERBEROS
int GetPassword(void);
#else
int GetPassword(UPtr forString,UPtr word, int size,short prompt);
#endif
#define PCopy(to,from)	BlockMove(from,to,*(from)+1)
UPtr Tokenize(UPtr string, int size, UPtr *start, UPtr *end, UPtr delims);
void CenterRectIn(Rect *inner,Rect *outer);
void TopCenterRectIn(Rect *inner,Rect *outer);
void BottomCenterRectIn(Rect *inner,Rect *outer);
void ThirdCenterRectIn(Rect *inner,Rect *outer);
typedef enum {Normal, Stop, Note, Caution} AlertEnum;
void MyAppendMenu(MenuHandle menu, UPtr name);
void MyInsMenuItem(MenuHandle menu, UPtr name, short afterItem);
void MyGetItem(MenuHandle menu, short item, UPtr name);
short CurrentModifiers(void);
void SpecialKeys(EventRecord *event);
UPtr PEscCat(UPtr string, UPtr suffix, short escape, char *escapeWhat);
short FindItemByName(MenuHandle menu, UPtr name);
short BinFindItemByName(MenuHandle menu, UPtr name);
void AttachHierMenu(short menu,short item,short hierId);
Boolean DirtyKey(long keyAndChar);
long RemoveSpaces(UPtr text,long size);
UPtr GetRStr(UPtr string,short id);
long atoi(UPtr p);
UPtr PLCat(UPtr string,long num);
UPtr LocalDateTimeStr(UPtr string);
long LocalDateTime(void);
long GMTDateTime(void);
void FixNewlines(UPtr string,long *count);
void GiveTime(void);
long MyMenuKey(EventRecord *event);
void RemoveParens(UPtr string);
long UnadornMessage(EventRecord *event);
Boolean TrimPrefix(UPtr string, UPtr prefix);
UPtr ChangeStrn(short resId,short num,UPtr string);
void RecountStrn(short resId);
void NukeMenuItemByName(short menuId,UPtr itemName);
void RenameItem(short menuId,UPtr oldName,UPtr newName);
Boolean HasSubmenu(MenuHandle mh, short item);
UPtr ComposeString(UPtr into,UPtr format,...);
UPtr VaComposeString(UPtr into,UPtr format,va_list args);
UPtr ComposeRString(UPtr into,short format,...);
UPtr VaComposeRString(UPtr into,short format,va_list args);
void GreyControl(ControlHandle cntl);
Boolean SetGreyControl(ControlHandle cntl, Boolean shdBeGrey);
Boolean IsAUX(void);
long ZoneSecs(void);
void NOOP(void);
Boolean WNE(short eventMask,EventRecord *event,long sleep);
void DlgUpdate(MyWindowPtr win);
long RoundDiv(long quantity,long unit);
void TransLitString(UPtr string);
long TZName2Offset(CStr zoneName);
Boolean EndsWith(PStr name,PStr suffix);
void InvalidatePasswords(Boolean pwGood,Boolean auxpwGood);
Boolean MiniEvents(void);
Boolean MyLClick(Point pt,short modifiers,ListHandle lHandle);

typedef struct {
	Handle textH;
	UPtr textP;
	long len;
	long lineBegin;
	long lineEnd;
	short partial;
} WrapDescriptor, WrapPtr;

void InitWrap(WrapPtr wp,Handle textH,UPtr textP,long len,long offset,long lastLen);
short Wrap(WrapPtr wp);

#define MakePStr(s,p,l) do{*s=MIN(l,sizeof(s)-2);BlockMove(p,s+1,*s);s[*s+1]=0;}while(0)
#define CtoPCpy(p,c) do{short l=strlen(c);MakePStr(p,c,l);}while(0)
#define Pause(t) do {long tk=TickCount(); while (TickCount()-tk<t) {WNE(0,nil,t);}} while(0)
#define PCatC(string,c) (string)[++(string)[0]] = c
#define DIR_MASK	8 	/* mask to use to test file attrib for directory bit */
#define OPTIMAL_BUFFER 4096 /* file buffer size */

#define optSpace 0xca
#define enterChar 0x03
#define escChar 0x1b
#define clearChar 0x1b
#define escKey 0x35
#define clearKey 0x47
#define delChar 0x7f
#define backSpace 0x08
#define returnChar 0x0d
#define bulletChar 0xa5
#define tabChar 0x09
#define leftArrowChar 0x1c
#define rightArrowChar 0x1d
#define upArrowChar 0x1e
#define downArrowChar 0x1f
#define homeChar 0x01
#define endChar 0x04
#define helpChar 0x05
#define pageUpChar 0x0b
#define pageDownChar 0x0c
#define undoKey 0x7a
#define cutKey 0x78
#define copyKey 0x63
#define pasteKey 0x76
#define clearKey 0x47
#define betaChar 0xa7
#define deltaChar 0xc6
#define nbSpaceChar 0xca