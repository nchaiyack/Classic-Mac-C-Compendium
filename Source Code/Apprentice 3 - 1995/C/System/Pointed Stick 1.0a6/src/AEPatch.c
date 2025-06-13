#include "Global.h"
#include "AEPatch.h"
#include "Remember.h"			// RememberFileAndParent
#include <AppleEvents.h>

#define selAEProcessAppleEvent		0x021B
#define selAEGetNthPtr				0x100A
#define selAEGetNthDesc				0x0A0B

typedef struct {
	long				oldA4;
	long				retAddr;
	Size				*actualSize;
	Size				maximumSize;
	Ptr					dataPtr;
	DescType			*typeCode;
	AEKeyword			*theAEKeyword;
	DescType			desiredType;
	long				index;
	const AEDescList	*theAEDescList;
} AEGetNthParamParameters;

typedef struct {
	long				oldA4;
	long				retAddr;
	AEDesc				*result;
	AEKeyword			*theAEKeyword;
	DescType			desiredType;
	long				index;
	const AEDescList	*theAEDescList;
} AEGetNthDescParameters;

UniversalProcPtr	gOldAppleEvent;

long				pOldRetAddr;
long				pOldA4;
short				pSnarfParams;
DescType			*pTypePtr;
void				*pDataPtr;
AEDesc				*pResultDescPtr;

void				ParseResult(void);
void				CheckAndRemember(DescType type, void *dataPtr);

//--------------------------------------------------------------------------------

void	MyAppleEvent()
{
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyAppleEvent");
	#endif

	SetUpA4();
	asm {
		cmp.w	#selAEProcessAppleEvent,D0
		bne.s	@checkGets

		clr.w	pSnarfParams(A4)
		move.l	8(A7),A0			; 8 = OldA4 + retAddr
		cmp.l	#'aevt',EventRecord.message(A0)
		bne.s	@wrongEvent
		cmp.l	#'odoc',EventRecord.where(A0)
		bne.s	@wrongEvent
		move.w	#1,pSnarfParams(A4)
	@wrongEvent
		move.l	gOldAppleEvent(A4),A0
		move.l	(A7)+,A4
		jmp		(A0)

	@checkGets
		tst.w	pSnarfParams(A4)
		beq.s	@wrongEvent
		
		cmp.w	#selAEGetNthPtr,D0
		bne.s	@checkGetDesc

		move.l	AEGetNthParamParameters.typeCode(A7),pTypePtr(A4)
		move.l	AEGetNthParamParameters.dataPtr(A7),pDataPtr(A4)
		clr.l	pResultDescPtr(A4)

		bra.s	@CallPack8

	@checkGetDesc
		cmp.w	#selAEGetNthDesc,D0
		bne.s	@wrongEvent

		clr.l	pTypePtr(A4)
		clr.l	pDataPtr(A4)
		move.l	AEGetNthDescParameters.result(A7),pResultDescPtr(A4)
	
	@CallPack8
		move.l	(A7)+,pOldA4(A4)
		move.l	(A7)+,pOldRetAddr(A4)
		
		move.l	gOldAppleEvent(A4),A0
		jsr		(A0)
	
		bsr		ParseResult
		
		move.l	pOldRetAddr(A4),-(A7)
		move.l	pOldA4(A4),A4
		rts
	}
}


//--------------------------------------------------------------------------------

void	ParseResult()
{
	unsigned char	oldDataHandleState;

	if (pDataPtr != NIL)
	{
		CheckAndRemember(*pTypePtr, pDataPtr);
	}
	else if (pResultDescPtr != NIL)
	{
		oldDataHandleState = HGetState(pResultDescPtr->dataHandle);
		HLock(pResultDescPtr->dataHandle);
		CheckAndRemember(pResultDescPtr->descriptorType, *pResultDescPtr->dataHandle);
		HSetState(pResultDescPtr->dataHandle, oldDataHandleState);
	}
}

void	CheckAndRemember(DescType type, void* dataPtr)
{
	FSSpec		mySpec;

	if (type == typeFSS)
	{
		RememberFileAndParent((FSSpec*) dataPtr);
	}
	else if (type == typeAlias)
	{
	
	}
}
