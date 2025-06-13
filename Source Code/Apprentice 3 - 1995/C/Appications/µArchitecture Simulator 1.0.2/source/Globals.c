//#pragma load "MacDump"


#include	"Globals.h"

EventRecord	gMyEvent;
GrowZoneUPP			gMyGrowZoneUPP;
ListClickLoopUPP	gInstrClikLoopUPP;
ListClickLoopUPP	gCommentClikLoopUPP;
ListClickLoopUPP	gSwitchCursClikLoopUPP;
TEHandle	gTheInput = nil;
unsigned long	gSleep;
CursHandle	gWatchHandle, gIBeamHandle, gPlusHandle;
short	gRstatus = kST_STOPPED;
short	gZoomFactor;
short	gSubclk;
Boolean	gDoneFlag = false;
Boolean	gInTheForeground = true;
Boolean	gHasColorQD = false, gPwrManagerIsPresent = false;
Boolean gDragManagerActive = false;

struct myprefs gPrefs = { kCurPrefsVersion, 2, {279, 0, 342, 381},
						{75, 0}, {36, 385}, {39, 16}, {50, 10}, {42, 8},
						{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0},
						kPCStart, kStackBase, kStackSize, 99, 0, 198, 0, true, false, false, false, false, true, true, false };

short	gRegs[kNUMOFREGS];	/* the registers of my machine */
short	gParts[kNUMOFPARTS];	/* editable parts in animation window */
Ptr	gMMemory;	/* RAM memory of the "monster": 128K! */
Ptr	gAssMemory;	/* Associative memory for hardware decoding: 256 bytes! */
union u_mir *gCsMemory;	/* Control Store memory: 512 bytes! (128 longs) */
unsigned short	gILCBase;

MenuHandle	gMenu_Apple;
MenuHandle	gMenu_File;
MenuHandle	gMenu_Edit;
MenuHandle	gMenu_Windows;
MenuHandle	gMenu_Control;
MenuHandle	gMenu_Assembler;
MenuHandle	gMenu_Memory;
MenuHandle	gMenu_Registers;
MenuHandle	gMenu_DisasmF;
MenuHandle	gMenu_DumpF;
MenuHandle	gPopMenu;

WindowPtr	gWPtr_Animation;
WindowPtr	gWPtr_Dump;
WindowPtr	gWPtr_Microprogram_Ed;
WindowPtr	gWPtr_Registers;
WindowPtr	gWPtr_Disasm;
WindowPtr	gWPtr_IO;
