/*
	Drop¥MPSR.h
	
	Header file for Drop¥MPSR.c
	
*/

#pragma once

#ifndef __H_Drop_MPSR__
#define __H_Drop_MPSR__

typedef struct PREFSTRUCT{
	OSType creator;
	unsigned short left;
	unsigned short right;
	unsigned short top;
	unsigned short bottom;
	unsigned short shiftdown;
	unsigned short shiftleft;
} PrefRec,* PrefPtr,** PrefHand;

#ifdef __cplusplus
extern "C"{
#endif

void MPSR_Preflight(short count,Handle* dataHdl);
void MPSR_Postflight(short count,Handle dataHdl);
OSErr MPSR_Process(FSSpecPtr theSpec);
Boolean MPSR_Init(void);
void MPSR_Cleanup(void);
void SetCreator(FSSpec* fsp,OSType type);
void SetMPSRRes(FSSpecPtr myFSSPtr);
void DSLocalError(StringPtr sp,OSErr err);
void CopyRect(Rect* from,Rect* to);

#ifdef __cplusplus
}
#endif

#endif
