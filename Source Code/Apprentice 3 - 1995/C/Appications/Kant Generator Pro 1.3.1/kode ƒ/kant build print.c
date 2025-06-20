#include "kant build print.h"
#include "kant build lists.h"
#include "printing layer.h"
#include "styled text layer.h"
#include "key layer.h"
#include "resource layer.h"
#include "memory layer.h"
#include "window layer.h"
#include "program globals.h"

#define kReferenceFont		0
#define kReferenceSize		12
#define kReferenceFace		0
#define kInstantFont		monaco
#define kInstantSize		9
#define kInstantFace		0

OSErr PrintTheModule(void)
{
	WindowRef		theWindow;
	short			numRefs, numInstants;
	short			ref, instant;
	TEHandle		hTE;
	Rect			dummy;
	Str255			theStr, instantStr;
	OSErr			oe;
	short			oldRefNum, resID, refNum;
	Boolean			alreadyOpen;
	Handle			resHandle;
	FSSpec			moduleFS;
	
	if ((theWindow=GetIndWindowRef(kBuildWindow))==0L)
		return -1;
	if (theWindow!=GetFrontDocumentWindow())
		return -1;
	
	moduleFS=GetWindowFS(theWindow);
	if ((oe=OpenTheResFile(&moduleFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
		return oe;
	
	SetRect(&dummy, 8000, 8000, 8100, 8100);
	hTE=TEStyleNew(&dummy, &dummy);
	numRefs=GetNumberOfReferences();
	for (ref=0; ref<numRefs; ref++)
	{
		GetIndReferenceName(theStr, ref);
		resHandle=Get1NamedResource('STR#', theStr);
		if (resHandle!=0L)
		{
			GetResInfo(resHandle, &resID, 0L, 0L);
			theStr[++theStr[0]]=key_Return;
			StyledInsertStr255(hTE, theStr, kReferenceFont, kReferenceSize, kReferenceFace);
			numInstants=**((short**)resHandle);
			for (instant=1; instant<=numInstants; instant++)
			{
				GetIndString(theStr, resID, instant);
				theStr[++theStr[0]]=key_Return;
				instantStr[0]=4;
				instantStr[1]=instantStr[2]=instantStr[3]=instantStr[4]=' ';
				AppendStr255(instantStr, theStr);
				StyledInsertStr255(hTE, instantStr, kInstantFont, kInstantSize, kInstantFace);
			}
			ReleaseResource(resHandle);
			resHandle=0L;
		}
	}
	
	CloseTheResFile(oldRefNum, refNum, alreadyOpen);
	
	PrintText(hTE);
	TEDispose(hTE);
	
	return noErr;
}
