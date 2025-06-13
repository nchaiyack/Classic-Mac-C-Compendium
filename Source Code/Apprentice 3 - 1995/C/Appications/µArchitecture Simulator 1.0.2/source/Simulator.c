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

#include	<stdlib.h>

#include	"UtilsSys7.h"
#include	"Globals.h"
#include	"AEHandlers.h"
#include	"Animation.h"
#include	"Dump.h"
#include	"Input.h"
#include	"Microprogram_Ed.h"
#include	"Registers.h"
#include	"Simulator.h"
#include	"SimUtils.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

enum conditions {
kCOND_NEVER = 0,
kCOND_N,
kCOND_Z,
kCOND_ALWAYS
};

const short reading[] = { kP_MBR, kP_MBRMEM, 0};
const short readstart[] = { kP_MAR2MEM, 0};
const short writestart[] = { kP_MBRMEM, kP_MAR2MEM, 0};

static short	Abus, Bbus, Cbus;
static union u_mir mir;

static short alu(void);
static void shifter(void);
static Boolean memory(void);
static int compareOpc(const void *opc1, const void *opc2);
static void ZoomEffectMap2MPC(void);

/* MSL: models the Micro Sequencing Logic */
#define MSL()	((mir.bits.cond == kCOND_ALWAYS) || ((mir.bits.cond == kCOND_Z) && \
				(gParts[kP_ALU - kFIRST_PICT] == 0)) || ((mir.bits.cond == kCOND_N) \
				&& (gParts[kP_ALU - kFIRST_PICT] < 0)))

/* AMUX: models the A multiplexer */
#define	AMUX()	(mir.bits.amux ? gParts[kP_MBR - kFIRST_PICT] : \
					gParts[kP_ALATCH - kFIRST_PICT])

/* MMUX: models the M multiplexer */
#define	MMUX()	((mslflag = MSL()) ? mir.bits.addr : gParts[kP_INCR - kFIRST_PICT])

/* alu: models the Arithmetic Logical Unit */

static short alu(void)
{
register short value;

value = AMUX();
gParts[kP_AMUX - kFIRST_PICT] = value;
switch(mir.bits.alu) {
	case 1:	value += Bbus;
		break;
	case 2:	value &= Bbus;
		break;
	case 3:	value = ~value;
		break;
	}
gParts[kP_ALU - kFIRST_PICT] = value;
if (gRstatus > kST_STEPASM) {
	ChangedBox(kP_AMUX - kFIRST_PICT);
	ChangedBox(kP_ALU - kFIRST_PICT);
	}
return value;
}

/* shifter: models the (!) shifter */

static void shifter(void)
{
register short value;

value = alu();
switch(mir.bits.shift) {
	case 1: value <<= 1;
		break;
	case 2: value = (unsigned short)value >> 1;
		break;
	case 3: value >>= 1;
	}
gParts[kP_SHIFTER - kFIRST_PICT] = value;
if (gRstatus > kST_STEPASM) {
	ChangedBox(kP_SHIFTER - kFIRST_PICT);
	}
Cbus = value;
}

/* MAR: models the Memory Address Register */

#define MAR()	\
{	\
if (mir.bits.mar) {	\
	gParts[kP_MAR - kFIRST_PICT] = gParts[kP_BLATCH - kFIRST_PICT];	\
	if (gRstatus > kST_STEPASM) {	\
		ChangedBox(kP_MAR - kFIRST_PICT);	\
		if (gRstatus == kST_STEPSUBCYC)	\
			ActivateObjs(marobjs);	\
		}	\
	}	\
}

/* MAR: models the Memory Buffer Register */

#define MBR()	\
{	\
if (mir.bits.mbr) {	\
	gParts[kP_MBR - kFIRST_PICT] = Cbus;	\
	if (gRstatus > kST_STEPASM) {	\
		ChangedBox(kP_MBR - kFIRST_PICT);	\
		if (gRstatus == kST_STEPSUBCYC)	\
			ActivateObjs(mbrobjs);	\
		}	\
	}	\
}

/* ExecuteInstructions: executes instructions for a clock subcycle,
taking care of the necessary user interface updates */

/* da schiaffare dentro la routine successiva */
const short subc0[] = { kP_MPC2INC, kP_INCR, kP_MPC2CST, kP_CST2MIR, kP_REG2LTCH1, kP_REG2LTCH2, 0 };
const short subc1[] = { kP_ALATCH, kP_BLATCH, 0 };
const short shftaluamux[] = { kP_AMUX2ALU, kP_BLTCH2ALU, kP_ALU, kP_ALU2SH, kP_SHIFTER, 0};
const short shftoregs[] = { kP_SH2REGS1, kP_SH2REGS2, kP_SH2REGS3, 0 };
const short marobjs[] = { kP_MAR, kP_BLTCH2MAR1, kP_BLTCH2MAR2, 0 };
const short mbrobjs[] = { kP_MBR, kP_SH2MBR1, kP_SH2MBR2, 0 };
const short mapobjs[] = { kP_MAP, kP_MAPREGS, 0 };
const short fromabus[] = { kP_ALTCH2AMUX, kP_AMUX, 0 };
const short frommbr[] = { kP_MBR2AMUX, kP_AMUX, 0 };
const short fromincr[] = { kP_INC2MMUX1, kP_INC2MMUX2, kP_MMUX, kP_MPC, 0};
const short frommir[] = { kC_ADDR1, kC_ADDR2, kP_MMUX, kP_MPC, 0 };
const short mmux2mpc[] = { kP_MMUX2MPC, 0 };

void ExecuteInstructions(short subclock)
{
register Handle	tempH;
register Ptr	base;
static unsigned short	mapLine;
static Boolean	mslflag;

switch (subclock) {
	case 0:
		gParts[kP_INCR - kFIRST_PICT] = gParts[kP_MPC - kFIRST_PICT] + 1;
		if (gRstatus > kST_STEPASM) {
			ChangedBox(kP_INCR - kFIRST_PICT);
			if (gRstatus == kST_STEPSUBCYC) {
				DeactivateObjs(shftoregs);
				DeactivateObjs(mbrobjs);
				DeactivateObjs(mapobjs);
				DeactivateObjs(writestart);
				if (mslflag)
					DeactivateObjs(frommir);
				else
					DeactivateObjs(fromincr);
				ActivateObjs(subc0);
				if (mir.bits.map)
					SelectLLine(kL_INSTR, mapLine);
				else {
					DeactivateObjs(mmux2mpc);
					SelectLLine(kL_COMMENTS, gParts[kP_MPC - kFIRST_PICT]);
					}
				}
			}
		mir = gCsMemory[gParts[kP_MPC - kFIRST_PICT]];
		Abus = gRegs[mir.bits.a];
		Bbus = gRegs[mir.bits.b];
		break;
	case 1:
		gParts[kP_ALATCH - kFIRST_PICT] = Abus;
		gParts[kP_BLATCH - kFIRST_PICT] = Bbus;
		if (gRstatus > kST_STEPASM) {
			ChangedBox(kP_ALATCH - kFIRST_PICT);
			ChangedBox(kP_BLATCH - kFIRST_PICT);
			if (gRstatus == kST_STEPSUBCYC) {
				DeactivateObjs(subc0);
				ActivateObjs(subc1);
				}
			}
		break;
	case 2:
		shifter();
		MAR()
		if (gRstatus == kST_STEPSUBCYC) {
			DeactivateObjs(subc1);
			ActivateObjs(shftaluamux);
			if (mir.bits.amux)
				ActivateObjs(frommbr);
			else
				ActivateObjs(fromabus);
			}
		break;
	case 3:
		if (mir.bits.dsc == 0) {
			gRegs[mir.bits.c] = Cbus;
			if (gRstatus > kST_STEPASM) {
				ChangedRegister(mir.bits.c);
				if (gRstatus == kST_STEPSUBCYC)
					ActivateObjs(shftoregs);
				}
			}
		MBR()
		gParts[kP_MPC - kFIRST_PICT] = gParts[kP_MMUX - kFIRST_PICT] = MMUX();
		(void)memory();
		if (mir.bits.map) {
//#define	Ext12(x)	(((x) << 4) >> 4)
//#define	Ext11(x)	(((x) << 5) >> 5)
			gRegs[kREG_EXT12] = gRegs[kREG_IR] << 4;
			gRegs[kREG_EXT12] >>= 4;
			gRegs[kREG_EXT11] = gRegs[kREG_IR] << 5;
			gRegs[kREG_EXT11] >>= 5;
			gRegs[kREG_LOW8] = gRegs[kREG_IR] & 0x00FF;
			gParts[kP_MPC - kFIRST_PICT] = *(gAssMemory + *(Byte *)&gRegs[kREG_IR]);
			if (gRstatus > kST_STEPASM) {
				ChangedRegister(kREG_EXT12);
				ChangedRegister(kREG_EXT11);
				ChangedRegister(kREG_LOW8);
				tempH = Get1Resource(krInstructions, kOPCODES);
				base = StripAddress(&((ROpcodePtr)*tempH)->offsetHB) + 2;
				mapLine = (unsigned long)(StripAddress(bsearch(&gRegs[kREG_IR], base,
					*(unsigned short *)*tempH + 1, sizeof(ROpcode), compareOpc))
					- base) / sizeof(ROpcode);
				ZoomEffectMap2MPC();
				if (gRstatus == kST_STEPSUBCYC)
					ActivateObjs(mapobjs);
				else
					SelectLLine(kL_INSTR, mapLine);
				}
			}
		if (gRstatus > kST_STEPASM) {
			ChangedBox(kP_MMUX - kFIRST_PICT);
			ChangedBox(kP_MPC - kFIRST_PICT);
			if (gRstatus == kST_STEPSUBCYC) {
				DeactivateObjs(shftaluamux);
				DeactivateObjs(marobjs);
				if (mir.bits.amux)
					DeactivateObjs(frommbr);
				else
					DeactivateObjs(fromabus);
				if (mslflag)
					ActivateObjs(frommir);
				else
					ActivateObjs(fromincr);
				if (mir.bits.map == 0)
					ActivateObjs(mmux2mpc);
				}
			else
				SelectLLine(kL_COMMENTS, gParts[kP_MPC - kFIRST_PICT]);
			}
		break;
	}
}

/* memory: models the external RAM memory */

static Boolean memory(void)
{
register short *tempP;
register unsigned short	firstWordinWind;
static Boolean rdInitiated = false, wrInitiated = false;
register Boolean	stoppedForIO = false;

if (mir.bits.rd) {
	if (rdInitiated) {
		tempP = ((short *)gMMemory + (unsigned short)gParts[kP_MAR - kFIRST_PICT]);
		gParts[kP_MBR - kFIRST_PICT] = *tempP;
		if (gRstatus > kST_STEPASM) {
			ChangedBox(kP_MBR - kFIRST_PICT);
			if (gRstatus == kST_STEPSUBCYC)
				ActivateObjs(reading);
			}
		/* handle memory mapped input */
		if ((Ptr)tempP == (gMMemory + kSIZE_RAM - 4)) {
			if ((gInTheForeground == false)||(gWPtr_IO != FrontWindow())) {
				SendmyAE(kFCR_MINE, kAEmySignalIO, myIdleFunct, kAENoReply | kAEAlwaysInteract | kAECanSwitchLayer);
				stoppedForIO = true;
				}
			}
		rdInitiated = false;
		}
	else {
		rdInitiated = true;
		if (gRstatus == kST_STEPSUBCYC)
			ActivateObjs(readstart);
		}
	}
if (mir.bits.wr) {
	if (wrInitiated) {
		tempP = ((short *)gMMemory + (unsigned short)gParts[kP_MAR - kFIRST_PICT]);
		*tempP = gParts[kP_MBR - kFIRST_PICT];
		wrInitiated = false;
		/* update Dump Window only if necessary */
		if (EmptyRgn(gWPtr_Dump->visRgn) == false)
			if ((firstWordinWind = (unsigned short)GetControlValue(dumpVScroll) << 4)
								<= (unsigned short)gParts[kP_MAR - kFIRST_PICT])
				if (((firstWordinWind - 1) + 
						((PRCT_B(gWPtr_Dump) - PRCT_T(gWPtr_Dump)) / dumpLineHeight) << 3)
						>= (unsigned short)gParts[kP_MAR - kFIRST_PICT])
					InvalDump();
		/* handle memory mapped output */
		if ((Ptr)tempP == (gMMemory + kSIZE_RAM - 2)) {
			DoKeyDown(gWPtr_IO, ((unsigned char *)tempP)[1], false);
			if ((gInTheForeground == false)||(gWPtr_IO != FrontWindow()))
				SendmyAE(kFCR_MINE, kAEmySignalIO, myIdleFunct, kAENoReply | kAEAlwaysInteract | kAECanSwitchLayer);
			*tempP = 0;
			}
		}
	else {
		wrInitiated = true;
		if (gRstatus == kST_STEPSUBCYC)
			ActivateObjs(writestart);
		}
	}
return stoppedForIO;
}

/* compareOpc: compare function to search for the instruction
currently executing */

static int compareOpc(const void *opc1, const void *opc2)
{
register unsigned short *pntr;
register unsigned short	a, b, c;

a = *(unsigned short *)opc1;
pntr = (unsigned short *)opc2;
b = *pntr++;
c = *pntr;
return ((c < a ? (int)1L : (int)0L) - (a < b ? (int)1L : (int)0L));
}

/* ExecuteInstructionsGO: executes a conventional instruction of the
simulated program, updating only the Dump window */

void ExecuteInstructionsGO(void)
{
enum {
kMAX_COMPUTE = 120L
};

register unsigned long	tickc;
register unsigned long	interval;
register short value;
register Boolean	mslflag, hasPaused;

tickc = TickCount();
do {
	gParts[kP_INCR - kFIRST_PICT] = gParts[kP_MPC - kFIRST_PICT] + 1;
	mir = gCsMemory[gParts[kP_MPC - kFIRST_PICT]];
	gParts[kP_ALATCH - kFIRST_PICT] = gRegs[mir.bits.a];
	gParts[kP_BLATCH - kFIRST_PICT] = gRegs[mir.bits.b];
	value = AMUX();
	gParts[kP_AMUX - kFIRST_PICT] = value;
	switch(mir.bits.alu) {
		case 1:	value += gParts[kP_BLATCH - kFIRST_PICT];
			break;
		case 2:	value &= gParts[kP_BLATCH - kFIRST_PICT];
			break;
		case 3:	value = ~value;
			break;
		}
	gParts[kP_ALU - kFIRST_PICT] = value;
	switch(mir.bits.shift) {
		case 1: value <<= 1;
			break;
		case 2: value = (unsigned short)value >> 1;
			break;
		case 3: value >>= 1;
		}
	gParts[kP_SHIFTER - kFIRST_PICT] = value;
	if (mir.bits.mar)
		gParts[kP_MAR - kFIRST_PICT] = gParts[kP_BLATCH - kFIRST_PICT];
	if (mir.bits.dsc == 0)
		gRegs[mir.bits.c] = value;
	if (mir.bits.mbr)
		gParts[kP_MBR - kFIRST_PICT] = value;
	gParts[kP_MPC - kFIRST_PICT] = gParts[kP_MMUX - kFIRST_PICT] = MMUX();
	hasPaused = memory();
	if (mir.bits.map) {
		gRegs[kREG_EXT12] = gRegs[kREG_IR] << 4;
		gRegs[kREG_EXT12] >>= 4;
		gRegs[kREG_EXT11] = gRegs[kREG_IR] << 5;
		gRegs[kREG_EXT11] >>= 5;
		gRegs[kREG_LOW8] = gRegs[kREG_IR] & 0x00FF;
		gParts[kP_MPC - kFIRST_PICT] = *(gAssMemory + *(Byte *)&gRegs[kREG_IR]);
		}
	interval = TickCount() - tickc;
	}
while ((gParts[kP_MPC - kFIRST_PICT]) && (interval < kMAX_COMPUTE));
if (gPrefs.infLoopsDetect && (hasPaused == false) && (interval >= kMAX_COMPUTE) && gParts[kP_MPC - kFIRST_PICT]) {
	(void)StopAlert_AE(kALRT_INFLOOP, myStdFilterProcNoCancel, myIdleFunct);
	StopIt();
	}
}

/* ZoomEffectMap2MPC: the hardware decoding unit is modifying
the Micro Program Counter; let the user have a visual feedback */

static void ZoomEffectMap2MPC(void)
{
GrafPtr	savePort;
Rect	mapRect, mpcRect;
register PicHandle	tempPH;

GetPort(&savePort);
SetPort(gWPtr_Animation);
tempPH = images[kP_MAP - kFIRST_PICT];
if (*tempPH == nil)
	LoadResource((Handle)tempPH);
mapRect = (*tempPH)->picFrame;
tempPH = images[kP_MPC - kFIRST_PICT];
if (*tempPH == nil)
	LoadResource((Handle)tempPH);
mpcRect = (*tempPH)->picFrame;
/* zoomFactor: 5 good on a Plus, 8 on a Quadra 700 */
ZoomRectToRect(&mapRect, &mpcRect, gZoomFactor);
SetPort(savePort);
}

/* StopIt: stops the processing */

void StopIt(void)
{
gRstatus = kST_STOPPED;
ChangedAllRegisters();
ChangedAllBoxes();
}

#endif

