/* driver.h 2.2 - Useful declarations for driver.c. By Pete Resnick */

#ifndef __DRIVER__
#define __DRIVER__

/* Flags for InstallRAMDriver and RemoveRAMDriver. Add together */
typedef enum {
	open = 0x01,		/* Open the driver after installed */
	thinkDATA = 0x02,	/* Driver uses THINK C, therefore uses DATA rsrc */
	thinkMulSeg = 0x04	/* Driver uses THINK C multi-segment DCOD rsrc's */
} drvrFlagBits;

/* Structure of the driver resource */
typedef struct {
	short drvrFlags;
	short drvrDelay;
	short drvrEMask;
	short drvrMenu;
	short drvrOpen;
	short drvrPrime;
	short drvrCtl;
	short drvrStatus;
	short drvrClose;
	unsigned char drvrName[];
	unsigned char drvrRoutines[];
} DriverStruct, *DriverPtr, **DriverHandle;

/* These two routines are the ones you want to call */
OSErr InstallRAMDriver(Str255 drvrName, short *refNum, drvrFlagBits drvrInstFlags);
OSErr RemoveRAMDriver(short refNum, drvrFlagBits drvrRmvFlags);

/* These are used internally but might be useful in unusual circumstances */
short GetDrvrRefNum(Str255 drvrName);
OSErr GrowUTable(short newEntries);
OSErr DriverAvail(short *unitNum);
OSErr Get1SysXRsrc(Handle *rsrcHndl, ResType rsrcType, short rsrcID, short rsrcInd, StringPtr rsrcName);
Boolean PtrInZone(THz theZone, Ptr thePtr);
OSErr ThinkInit(Handle drvrHandle, short rsrcID, short unitNum, drvrFlagBits drvrInstFlags);
OSErr ThinkChangeCode(Handle drvrHandle, Ptr codePtr, ResType theType);
OSErr ThinkAddRsrcs(Handle codeHandle, ResType theType, short rsrcID, short unitNum);
void ThinkCleanup(Handle drvrHandle, Boolean rmvDATA);
void ThinkRmvRsrcs(Ptr codePtr, Boolean isHandle, Boolean rmvDATA);

#endif /* __DRIVER__ */