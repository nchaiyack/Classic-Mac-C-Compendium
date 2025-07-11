#if GENERATINGCFM

   enum {
      myUppListClickLoopProcInfo = kRegisterBased
      | RESULT_SIZE(kNoByteCode)
      | REGISTER_RESULT_LOCATION(kCCRegisterZBit)
   };
   
   #define MyNewListClickLoopProc(userRoutine)  \
      (ListClickLoopUPP)NewRoutineDescriptor((ProcPtr)(userRoutine), \
      myUppListClickLoopProcInfo, GetCurrentISA())
         
   #define kListClickLoopTrue false
   #define kListClickLoopFalse true
      
#else

   #define MyNewListClickLoopProc(userRoutine)  \
      (ListClickLoopUPP)(userRoutine)
         
   #define kListClickLoopTrue true
   #define kListClickLoopFalse false

#endif

/* balloon help */
enum balllll {
kBalloons_Anim = 1010,
kBalloons_Regs,
kBalloons_MPrg,
kBalloons_Dump,
kBAnim_ALatch = 1,
kBAnim_BLatch,
kBAnim_MAR,
kBAnim_MBR,
kBAnim_AMUX,
kBAnim_Shifter,
kBAnim_Inc,
kBAnim_MMUX,
kBAnim_MPC,
kBAnim_ALU,
kBAnim_MSL,
kBAnim_MIR,
kBAnim_Regs,
kBAnim_CStore,
kBAnim_MAP,
kBRegs_Base = 1,
kBRegs_PC,
kBRegs_AC,
kBRegs_SP,
kBRegs_BP,
kBRegs_IR,
kBRegs_A,
kBRegs_B,
kBRegs_C,
kBRegs_D,
kBRegs_E,
kBRegs_X12,
kBRegs_X11,
kBRegs_Lo8,
kBMPr_ABus = 1,
kBMPr_BBus,
kBMPr_CBus,
kBMPr_MAR,
kBMPr_MBR,
kBMPr_BusReq,
kBMPr_Shifter,
kBMPr_Branch,
kBMPr_ALU,
kBMPr_ABusRadio,
kBMPr_MBRRadio,
kBMPr_MAP,
kBMPr_BranchTo,
kBMPr_Comment,
kBMPr_CommentsList,
kBMPr_ConvInstrList,
kBDump_Contents = 1,
kBDisasm_Contents,
kBIO_Contents
};

/*	kScrollbarAdjust and kScrollbarWidth are used in calculating
	values for control positioning and sizing. */
enum {
kScrollbarAdjust = 15,
kScrollbarWidth
};

enum {
khiliteGhost = 50	/* defined in Inside Mac V, but NOT in QuickDraw.h: why??? */
};

enum suoni {
kSndPinzatrice = 256
};

/* Menu list resource IDs */
enum {
kRes_Menu_HMemory = 1,
kRes_Menu_HRegs,
kRes_Menu_HDisasm,
kRes_Menu_HDump,
kRes_Menu_PopDump = 264,

kRes_Menu_Windows = 131,
kRes_Menu_Control,
kRes_Menu_Assembler
};

enum {
kD_Edit = 1,
kD_Disasm = 3,
kD_DisasmFrom,
kD_DumpFrom
};

enum {
kMItem_About__Simulato = 1
};

enum fileitems {
kMItem_New = 1,
kMItem_Open_Control_St,
kMItem_Close = 4,
kMItem_Save_Control_St,
kMItem_Save_Control_St2,
kMItem_Save_CS_Stationery,
kMItem_Revert_to_Saved,
kMItem_HMem = 10,
kMItem_HRegs,
kMItem_Page_Setup = 13,
kMItem_Print,
kMItem_Prefs = 16,
kMItem_Quit = 18
};

enum {
kMItem_Select_All = 7,
kMItem_Insert = 9,
kMItem_Delete,
kMItem_CopyEmailAddr = 12,
kMItem_CopyWWWURL
};

enum windowsitems {
kMItem_Animation = 1,
kMItem_Registers,
kMItem_IO,
kMItem_Microprogram,
kMItem_Disasm,
kMItem_Dump
};

enum hieritems {
kMItem_Open = 1,
kMItem_Save_As
};

enum controlitems {
kMItem_Go = 1,
kMItem_Stop,
kMItem_StepSub = 4,
kMItem_StepMicro,
kMItem_StepInstr,
kMItem_ResetMem = 8,
kMItem_ResetRegs,
kMItem_Disasmfrom = 11,
kMItem_Dumpfrom,
kMItem_SelectCurMPC
};

enum asmitems {
kMItem_Assemble_file = 1,
kMItem_AsmPrefs = 3
};

/* sizes */
enum sizes {
kNUMOFPARTS = 10,
kNUMOFSTATEPARTS = 23,
kSIZE_RAM = 128*1024L,
kSIZE_ASSMEM = 256,
kSIZE_CSMEM = 512
};

/* registers */
enum registers {
kREG_PC = 0,
kREG_AC,
kREG_SP,
kREG_BP,
kREG_IR,
kREG_A,
kREG_B,
kREG_C,
kREG_D,
kREG_E,
kREG_EXT12,
kREG_EXT11,
kREG_LOW8,
kREG_ZERO,
kREG_ONE,
kREG_MINUS1,
kNUMOFREGS
};

enum micstatus {
kST_STOPPED = 0,
kST_GOING,
kST_STEPASM,
kST_STEPMPROG,
kST_STEPSUBCYC
};

/* resources types, file types, creator */
enum filetypes {
krInstructions = 'OPCO',
krHandlers = 'HAND',
kFTY_CSTORE = '�PRG',
kFTY_RAM = '�RAM',
kFCR_MINE = '�SIM',
kFTY_CSTOREPAD = 'sPRG',
kFTY_REG = '�REG'
};

/* alerts */

enum somealerts {
kALRT_RESETMEM = 128,
kALRT_NOSAFESAVE = 129,
kALRT_NOSAVE = 132,
kALRT_RESETREGS = 139,
kALRT_STKOVERFLOW = 140,
kALRT_MEMTOOFULL = 141,
kALRT_REVERT,
kALRT_CPURUNNING = 143,
kALRT_STKUNDERFLOW = 144,
kALRT_INFLOOP,
kALRT_TOSHELL,
kALRT_WRONGEXISTINGDOC,
kALRT_SAVE = 200
};

/* strings */
enum {
kSTR_PREFSFILENAME = 138
};

enum {
kSTR_RAMPROMPT = 133,
kSTR_RAMDEFNAME,
kSTR_REGSPROMPT = 139,
kSTR_REGSDEFNAME
};

/* save selectors */
enum savesel {
kGOT_SAVE = 0,
kGOT_SAVEAS,
kGOT_SAVESTATIONERY
};

enum defaultregs {
kPCStart = 8192,
kStackBase = 65536-4098,
kStackSize = 1024,
kPCExitToShell = 4096,
kIllegalVector = 3,
kIllegalHandler = 0x300,
kIllegalHandlerCode = 128
};

enum myAEIDs {
kAEmySignalIO = 'I/O '
};

enum editdialogstrs {
kInThe = 0,
kAtMem,
kValueFollow = 130
};

enum {
kCurPrefsVersion = 4
};

enum {
kfIsStationery = (1 << 11)
//fNameLocked = (1 << 12)
};


#define PTR2MEMWORD(address)	((unsigned long)((Ptr)(address) - gMMemory) >> 1)

#define PRCT_T(f)	(f->portRect.top)
#define PRCT_L(f)	(f->portRect.left)
#define PRCT_B(f)	(f->portRect.bottom)
#define PRCT_R(f)	(f->portRect.right)

/* Define HiWrd and LoWrd macros for efficiency. */
#define HiWrd(aLong)	(short)(((unsigned long)aLong) >> 16)
#define LoWrd(aLong)	(short)((aLong) & 0xFFFF)

/* ======================================================= */

union u_mir {
	struct {
		unsigned	amux	: 1;
		unsigned	cond	: 2;
		unsigned	alu		: 2;
		unsigned	shift	: 2;
		unsigned	mbr		: 1;
		unsigned	rd		: 1;
		unsigned	wr		: 1;
		unsigned	mar		: 1;
		unsigned	dsc		: 1;
		unsigned	a		: 4;
		unsigned	b		: 4;
		unsigned	c		: 4;
		unsigned	map		: 1;
		unsigned	addr	: 7;
		} bits;
	unsigned long cstore;
	};

typedef union u_mir un_mir;


#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ROpcode {
	OSType	instr;
	Byte	length;
	Byte	class;
	Byte	offsetHB;
	Byte	offsetLB;
	Byte	lastHB;
	Byte	lastLB;
	};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct ROpcode ROpcode;
typedef ROpcode *ROpcodePtr;
typedef ROpcodePtr *ROpcodeHandle;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct ROpcodeDisasm {
	OSType	instr;
	Byte	length;
	Byte	class;
	short	first;
	short	last;
	};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

typedef struct ROpcodeDisasm ROpcDisasm;
typedef ROpcDisasm *ROpcDisasmPtr;
typedef ROpcDisasmPtr *ROpcDisasmHandle;

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif
struct myprefs {
	short	prefsVersion;
	short	RegsBase;
	Rect	IOUserState;
	Point	AnimTopLeft;
	Point	RegsTopLeft;
	Point	MProgTopLeft;
	Point	DumpTopLeft;
	Point	DisasmTopLeft;
	Point	generalPrefsTL;
	Point	asmPrefsTL;
	Point	editValueTL;
	Point	aboutTL;
	Point	creditsTL;
	unsigned short	DefPCValue;
	unsigned short	DefSPValue;
	short	DefStkSize;
	short	DumpHeight;
	short	DumpScrollVal;
	short	DisasmHeight;
	short	DisasmScrollVal;
	Boolean AnimVisible;
	Boolean	RegsVisible;
	Boolean	IOVisible;
	Boolean	DumpVisible;
	Boolean	DisasmVisible;
	Boolean infLoopsDetect;
	Boolean	remembWind;
	Boolean	NeXTScroll;
	};
#if defined(powerc) || defined(__powerc)
#pragma options align=reset
#endif

/************************************************************/
extern EventRecord	gMyEvent;
extern GrowZoneUPP		gMyGrowZoneUPP;
extern ListClickLoopUPP	gInstrClikLoopUPP;
extern ListClickLoopUPP	gCommentClikLoopUPP;
extern ListClickLoopUPP	gSwitchCursClikLoopUPP;
extern TEHandle	gTheInput;
extern unsigned long	gSleep;
extern CursHandle	gWatchHandle, gIBeamHandle, gPlusHandle;
extern short	gRstatus, gZoomFactor;
extern short	gSubclk;
extern Boolean	gDoneFlag;
extern Boolean  gInTheForeground;
extern Boolean	gHasColorQD, gPwrManagerIsPresent;
extern Boolean gDragManagerActive;

extern struct myprefs gPrefs;

extern short	gRegs[kNUMOFREGS];
extern short	gParts[kNUMOFPARTS];
extern Ptr	gMMemory;
extern Ptr	gAssMemory;
extern union u_mir *gCsMemory;
extern unsigned short	gILCBase;

extern MenuHandle	gMenu_Apple;
extern MenuHandle	gMenu_File;
extern MenuHandle	gMenu_Edit;
extern MenuHandle	gMenu_Windows;
extern MenuHandle	gMenu_Control;
extern MenuHandle	gMenu_Assembler;
extern MenuHandle	gMenu_Memory;
extern MenuHandle	gMenu_Registers;
extern MenuHandle	gMenu_DisasmF;
extern MenuHandle	gMenu_DumpF;
extern MenuHandle	gPopMenu;


extern WindowPtr	gWPtr_Animation;
extern WindowPtr	gWPtr_Dump;
extern WindowPtr	gWPtr_Microprogram_Ed;
extern WindowPtr	gWPtr_Registers;
extern WindowPtr	gWPtr_Disasm;
extern WindowPtr	gWPtr_IO;

//pascal Boolean HVInRect(short h, short v, const Rect *r) = 0xA8AD;

