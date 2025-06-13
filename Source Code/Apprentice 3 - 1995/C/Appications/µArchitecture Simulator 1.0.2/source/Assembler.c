/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	<ctype.h>
#include	<stdlib.h>

#include	"UtilsSys7.h"
#include	"Assembler.h"
#include	"Disasm.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"Dump.h"
#include	"Globals.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"MovableModal.h"
#include	"SimUtils.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

#pragma segment Rare

enum {
kOPCMAXLEN = 4,
kGROW_SYMTAB = 50,
kGROW_OBJTAB = 150,
kBITS_12 = 0x0FFF,
kBITS_11 = 0x07FF,
kBITS_8 = 0x00FF
};

struct symtable {
	StringHandle	symb;
	long	value;
	};

typedef struct symtable symtable;
typedef symtable *symtablePtr;
typedef symtablePtr *symtableHandle;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct objtable {
	long	operand;
	Byte	length;
	Byte	class;
	unsigned short	opcod;
	Boolean	isSymbol;
	Boolean	reserved;
	};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct objtable objtable;
typedef objtable *objtablePtr;
typedef objtablePtr *objtableHandle;

static OSErr OnePassAsm(Handle fileBuffer);
static long CountReturns(Handle fileBuffer, char *lastpos);
static Boolean AsmPrefsPreProcessKey(EventRecord *thEv, DialogPtr theD);
//static pascal Boolean AsmEditNumFilter(DialogPtr, EventRecord *, short *);
static int compareMnem(const void *opc1, const void *opc2);
static int cmpsymb(const void *opc1, const void *opc2);


/* myAsmFile: reads theFile to be assembled into memory */

OSErr myAsmFile(FSSpec *theFile)
{
ParamBlockRec	myPB;
EventRecord	dummyEv;
register Handle	tempBuffer;
unsigned long	fileSize;
short	asmFileRefN;
register OSErr	err;

SetCursor(*gWatchHandle);
if ((err = FSpOpenDFCompat(theFile, fsRdPerm, &asmFileRefN)) == noErr) {
	if ((err = GetEOF(asmFileRefN, (long *)&fileSize)) == noErr) {
		if (tempBuffer = NewHandleGeneral(fileSize)) {
			myPB.ioParam.ioCompletion = nil;
			myPB.ioParam.ioRefNum = asmFileRefN;
			HLock(tempBuffer);
			myPB.ioParam.ioBuffer = *tempBuffer;
			myPB.ioParam.ioReqCount = fileSize;
			myPB.ioParam.ioPosMode = fsFromStart;
			myPB.ioParam.ioPosOffset = 0L;
			(void)PBReadAsync(&myPB);
			while (myPB.ioParam.ioResult > 0) {
				SystemTask();
				(void)EventAvail(everyEvent, &dummyEv);
				}
			HUnlock(tempBuffer);
			if ((err = myPB.ioParam.ioResult) == noErr) {
				/* Do damned assembling */
				err = OnePassAsm(tempBuffer);
				InvalDump();
				InvalDisasm();
				}
			DisposeHandle(tempBuffer);
			}
		else err = MemError();
		}
	(void)FSClose(asmFileRefN);
	}
InitCursor();
return(err);
}

/* OnePassAsm: assembles the file passed in the buffer */

static OSErr OnePassAsm(Handle fileBuffer)
{
Str255	tempS;
Handle	SortedOpcodeTable;
symtableHandle	SymbolTable;
objtableHandle	ObjTable;
Handle	tempH;
Ptr	myEOF;
long	ILC;
ROpcodePtr	found;
symtablePtr	foundsym;
Size	ObjTabOffset, SymTabOffset, tmpSize;
short	build;
short	numinstr;
OSErr	err;
register Byte	i;
register char *ex = nil;
register char *copy;

err = noErr;
ILC = (long)gILCBase << 1;
DetachResource(SortedOpcodeTable = Get1Resource(krInstructions, kOPCODES));
HNoPurge(SortedOpcodeTable);
qsort((*SortedOpcodeTable) + 2, numinstr = (*(unsigned short *)*SortedOpcodeTable) + 1,
	sizeof(ROpcode), compareMnem);

/* init symbol & object table */
if ((SymbolTable = (symtableHandle)NewHandleGeneral(sizeof(symtable)*kGROW_SYMTAB))) {
	if ((ObjTable = (objtableHandle)NewHandleGeneral(sizeof(objtable)*kGROW_OBJTAB))) {
		SymTabOffset = 0L;
		ObjTabOffset = 0L;
		HLock(fileBuffer);
		ex = *fileBuffer;
		myEOF = (Ptr)((Size)ex + InlineGetHandleSize(fileBuffer));
		do {
			if (*ex == 13)
				ex++;
			else {
				if (ispunct(*ex))
					while (*ex++ != 13)
						;
				else {
					if (isspace(*ex) == 0) {
						copy = (char *)&tempS[1];
						i = 0;
						do {
							*copy++ = *ex++;
							i++;
							}
						while (isspace(*ex) == 0);
						tempS[0] = i;
						/* add to symbol table */
						if (SymTabOffset >= (tmpSize = InlineGetHandleSize((Handle)SymbolTable)))
							SetHandleSize((Handle)SymbolTable, tmpSize + sizeof(symtable)*kGROW_SYMTAB);
						tempH = (Handle)NewString(tempS);
						((symtablePtr)(*(Handle)SymbolTable + SymTabOffset))->symb = (StringHandle)tempH;
						((symtablePtr)(*(Handle)SymbolTable + SymTabOffset))->value = ILC;
						SymTabOffset += sizeof(symtable);
						}
					if (*ex == 13)
						ex++;
					else {
						while (isspace(*ex++))
							;
						--ex;
						copy = (char *)&tempS;
						i = 0;
						do {
							*copy++ = *ex++;
							i++;
							}
						while (isspace(*ex) == 0);
						if (i > kOPCMAXLEN) {
							err = kasmErrInvalidOpcode;
							break;
							}
						else {
							while (i < kOPCMAXLEN) {
								i++;
								*copy++ = ' ';
								}
							if (ObjTabOffset >= (tmpSize = InlineGetHandleSize((Handle)ObjTable)))
								SetHandleSize((Handle)ObjTable, tmpSize + sizeof(objtable)*kGROW_OBJTAB);
							found = (ROpcodePtr)bsearch(&tempS, (*SortedOpcodeTable)+2,
									numinstr, sizeof(ROpcode), compareMnem);
							if (found == nil) {
								err = kasmErrInvalidOpcode;
								break;
								}
							else {
								*(long *)&((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->length = *(long *)(&found->length);
								i = found->class;
								if (i == kCLASS_16_0) {
									((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->isSymbol = false;
									((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->operand = 0L;
									}
								else {
									while (isspace(*ex++))
										;
									--ex;
									if (*ex == '#') {
										++ex;
										copy = (char *)&tempS[1];
										i = 0;
										do {
											*copy++ = *ex++;
											i++;
											}
										while (isspace(*ex) == 0);
										tempS[0] = i;
										((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->isSymbol = false;
										StringToNum(tempS, &((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->operand);
										}
									else {
										copy = (char *)&tempS[1];
										i = 0;
										do {
											*copy++ = *ex++;
											i++;
											}
										while (isspace(*ex) == 0);
										tempS[0] = i;
										tempH = (Handle)NewString(tempS);
										((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->isSymbol = true;
										((objtablePtr)(*(Handle)ObjTable + ObjTabOffset))->operand = (long)tempH;
										}
									}
								ObjTabOffset += sizeof(objtable);
								}
							while (*ex++ != 13)
								;
							ILC += found->length;
							}
						}
					}
				}
			}
		while (ex < myEOF);
		HUnlock(fileBuffer);
		if (err == noErr)
			/* check for ILC out of bounds */
			if (ILC > kSIZE_RAM - 4096)
				err = kasmErrPotHeapDamage;
		if (err == noErr) {
			ILC = (long)gILCBase << 1;
			qsort(*SymbolTable, SymTabOffset / sizeof(symtable), sizeof(symtable), cmpsymb);
			for(tmpSize = 0; tmpSize < ObjTabOffset; tmpSize += sizeof(objtable)) {
				if (((objtablePtr)(*(Handle)ObjTable + tmpSize))->isSymbol) {
					foundsym = (symtablePtr)
						bsearch(&((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand,
						*SymbolTable, SymTabOffset / sizeof(symtable),
						sizeof(symtable), cmpsymb);
					((objtablePtr)(*(Handle)ObjTable + tmpSize))->isSymbol = false;
					DisposeHandle((Handle)((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand);
					((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand =
							(foundsym ? (((objtablePtr)(*(Handle)ObjTable + tmpSize))->class >= kCLASS_16_16_REL
							? (foundsym->value - ILC - ((objtablePtr)(*(Handle)ObjTable + tmpSize))->length) >> 1
							: foundsym->value) : (err = kasmErrSymbolNotDef, 0L));
					}
				switch (((objtablePtr)(*(Handle)ObjTable + tmpSize))->class) {
					case kCLASS_4_12:
					case kCLASS_4_12_REL:
						build = (((objtablePtr)(*(Handle)ObjTable + tmpSize))->opcod |
							(((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand & kBITS_12));
						*(short *)(gMMemory + ILC) = build;
						ILC += 2;
						break;
					case kCLASS_5_11:
						build = (((objtablePtr)(*(Handle)ObjTable + tmpSize))->opcod |
							(((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand & kBITS_11));
						*(short *)(gMMemory + ILC) = build;
						ILC += 2;
						break;
					case kCLASS_8_8:
						build = (((objtablePtr)(*(Handle)ObjTable + tmpSize))->opcod |
							(((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand & kBITS_8));
						*(short *)(gMMemory + ILC) = build;
						ILC += 2;
						break;
					case kCLASS_16_0:
						*(short *)(gMMemory + ILC) = ((objtablePtr)(*(Handle)ObjTable + tmpSize))->opcod;
						ILC += 2;
						break;
					case kCLASS_16_16:
					case kCLASS_16_16_REL:
						*(short *)(gMMemory + ILC) = ((objtablePtr)(*(Handle)ObjTable + tmpSize))->opcod;
						ILC += 2;
						*(short *)(gMMemory + ILC) = ((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand;
						ILC += 2;
						break;
					}
				}
			}
		if ((err != noErr) && (err != kasmErrSymbolNotDef))
			for(tmpSize = 0; tmpSize < ObjTabOffset; tmpSize += sizeof(objtable))
				if (((objtablePtr)(*(Handle)ObjTable + tmpSize))->isSymbol)
					DisposeHandle((Handle)((objtablePtr)(*(Handle)ObjTable + tmpSize))->operand);
		for(tmpSize = 0; tmpSize < SymTabOffset; tmpSize += sizeof(symtable))
			DisposeHandle((Handle)((symtablePtr)(*(Handle)SymbolTable + tmpSize))->symb);
		DisposeHandle((Handle)ObjTable);
		}
	else err = MemError();
	DisposeHandle((Handle)SymbolTable);
	}
else err = MemError();
DisposeHandle(SortedOpcodeTable);
if (err > 0) {
	InitCursor();
	if (ex)
		MyNumToString(1L + CountReturns(fileBuffer, ex), tempS);
	else
		*(short *)tempS = 0x013F;
	ParamText(tempS, nil, nil, nil);
	StopAlert_UPP(kALRT_ASM + err, myStdFilterProcNoCancel);
	err = 0;
	}
return err;
}

static long CountReturns(Handle fileBuffer, char *lastpos)
{
register char *scanptr;
register char *end = StripAddress(lastpos);
register long	cnt = 0L;

for ( scanptr = StripAddress(*fileBuffer); scanptr <= end; )
	if (*scanptr++ == 13)
		++cnt;
return cnt;
}


enum {
kDLOG_ASMPREFS = 263,
kItemAsmDestLoc = 3
};

void DoAsmPrefsDialog(void)
{
Str255	AsmDestStr;

dialogItems	things[] = {{ ok, 0, 1L },
						{ cancel, 0, 0L },
						{ kItemAsmDestLoc, 0, 0L },
						{ 0, 0, 0L}
						};

things[kItemAsmDestLoc-1].refCon = (long)&AsmDestStr;

ShortToHexString(gILCBase, AsmDestStr);

if (HandleMovableModalDialog(things, gPrefs.remembWind ? &gPrefs.asmPrefsTL : nil, nil, nil, nil, nil, nil,
	AdjustMenus,
	Handle_My_Menu,
	DomyKeyEvent,
	AsmPrefsPreProcessKey,
	nil,
	DoUpdate,
	DoActivate,
	DoHighLevelEvent,
	DoOSEvent,
	DoIdle,
	ULONG_MAX,
	kDLOG_ASMPREFS) == ok) {
	HexStringToShort(AsmDestStr, (short *)&gILCBase);
	}
}

Boolean AsmPrefsPreProcessKey(EventRecord *thEv, DialogPtr theD)
{
//short	iHit;
unsigned char	keypressed;
Boolean	result = true;

keypressed = CHARFROMMESSAGE(thEv->message);
if ((keypressed >= 'a') && (keypressed <= 'z')) {
	keypressed -= 'a' - 'A';	/* cambiare! */
	CHARFROMMESSAGE(thEv->message) = keypressed;
	}
//iHit = ((DialogPeek)theD)->editField + 1;
if (keypressed >= 32 && ((thEv->modifiers & cmdKey) == 0)) {
	result = ( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L, &keypressed,
							1L, 0L, 0L) >= 0L );
	}
return result;
}

/* AsmEditNumFilter: filterProc for the asm prefs dialog */
/*
static pascal Boolean AsmEditNumFilter(DialogPtr theD, EventRecord *thEv, short *iHit)
{
enum {
kITEM_VALUE = 3
};

GrafPtr	savePort;
unsigned char	keypressed;
register Boolean	retVal;

switch(thEv->what) {
	case keyDown	:
	case autoKey	:
		keypressed = CHARFROMMESSAGE(thEv->message);
		if ((keypressed >= 'a') && (keypressed <= 'z')) {
			keypressed -= 'a' - 'A';
			CHARFROMMESSAGE(thEv->message) = keypressed;
			}
		if ((keypressed >= 32) && (keypressed != 0x7F) && ((thEv->modifiers & cmdKey) == 0)) {
			*iHit = kITEM_VALUE;
			return( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L, &keypressed, 1L, 0L, 0L) < 0L );
			}
		break;
	case updateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoUpdate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	case activateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoActivate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	}
GetPort(&savePort);
SetPort(theD);
retVal = StdFilterProc(theD, thEv, iHit);
SetPort(savePort);
return retVal;
}
*/

/* compareMnem: used to compare opcodes in the opcode table */

static int compareMnem(const void *opc1, const void *opc2)
{

return(*(long *)opc1 - *(long *)opc2);
}

/* cmpsymb: used to compare strings in the Symbol Table */

static int cmpsymb(const void *opc1, const void *opc2)
{
return(RelString(**(StringHandle *)opc1, **(StringHandle *)opc2, true, true));
}

#endif

