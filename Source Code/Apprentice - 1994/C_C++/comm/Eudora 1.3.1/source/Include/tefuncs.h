/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for functions primarily involving TextEdit
 ************************************************************************/
typedef enum {TECUT=3, TECOPY, TEPASTE, TEQPASTE, TECLEAR, TEKEY, TEWRAP=11, TEUNWRAP, TEENUM_LIMIT} TEEnum;
int CountTeLines(TEHandle teh);
ShowInsertion(MyWindowPtr win,short whichEnd);
Boolean TESomething(MyWindowPtr win,TEEnum what,short key,short mods);
void OffsetTE(MyWindowPtr win,TEHandle teh,short offset);
void TEWordSelect(TEHandle teh);
void HFC(MyWindowPtr win);
short TEFakeTab(MyWindowPtr win,short stopDistance);
void InsertCommaIfNeedBe(MyWindowPtr win);
void TEFixup(TEHandle teh);
short TEMaxLine(TEHandle teh);
short TopOffset(TEHandle teh);
void TESelPara(TEHandle teh);
void MakeTopOffset(TEHandle teh,short offset);
int WrapSendWDS(wdsEntry *theWDS);
void NoScrollTESetSelect(short start,short end,TEHandle teh);
void NoScrollTECalText(TEHandle teh);
short TELineOf(short position, TEHandle teh);
short TEInsertDelta(TEHandle teh,Rect *view,short whichEnd);
#define TE_VMARGIN	1		/* inset between view rect and enclosing box */
#define TE_HMARGIN	4		/* ditto */
typedef enum {InsertStart=-1,InsertAny,InsertEnd} ShowInsertEnum;
