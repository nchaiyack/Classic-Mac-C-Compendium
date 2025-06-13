/* this file is not necessary, but it reduces the size of the
application and improves its speed by avoiding to link a lot
of useless glue routines: from the include files of MPW */

#ifndef THINK_C_5
pascal void SFPutFile(Point, Str255, Str255, ProcPtr, SFReply *)
    = {0x3F3C,0x0001,0xA9EA};

pascal void SFGetFile(Point, Str255, ProcPtr, short, SFTypeList, ProcPtr, SFReply *)
    = {0x3F3C,0x0002,0xA9EA};

pascal void SFPPutFile(Point,Str255,Str255,ProcPtr,SFReply *,short,ProcPtr)
    = {0x3F3C,0x0003,0xA9EA};

pascal void SFPGetFile(Point,Str255,ProcPtr,short,SFTypeList,ProcPtr,SFReply *,short,ProcPtr)
    = {0x3F3C,0x0004,0xA9EA};

pascal void ResrvMem(Size)
    = {0x201F,0xA040};
pascal void SetApplLimit(void *)
    = {0x205F,0xA02D};
pascal void DisposHandle(Handle)
    = {0x205F,0xA023};
pascal void DisposPtr(Ptr)
    = {0x205F,0xA01F};
pascal void HPurge(Handle)
    = {0x205F,0xA049};
pascal void HNoPurge(Handle)
    = {0x205F,0xA04A};
pascal void FlushEvents(short,short )
	   = {0x201F,0xA032};
pascal void HLock(Handle)
    = {0x205F,0xA029};
pascal void HUnlock(Handle )
    = {0x205F,0xA02A};
pascal void LUpdate(RgnHandle theRgn,ListHandle lHandle)
    = {0x3F3C,0x0064,0xA9E7};
pascal void IUDateString(long,DateForm,Str255 )
    = {0x4267,0xA9ED};
pascal void IUTimeString(long,Boolean,Str255)
    = {0x3F3C,0x0002,0xA9ED};
pascal Handle IUGetIntl(short)
    = {0x3F3C,0x0006,0xA9ED}; 
pascal void LActivate(Boolean act,ListHandle lHandle)
    = {0x4267,0xA9E7}; 
pascal void LDispose(ListHandle lHandle)
    = {0x3F3C,0x0028,0xA9E7}; 
pascal Boolean LSearch(void *dataPtr,short dataLen,ProcPtr searchProc,
    Cell *theCell,ListHandle lHandle)
    = {0x3F3C,0x0054,0xA9E7}; 
pascal Boolean LClick(Point pt,short modifiers,ListHandle lHandle)
    = {0x3F3C,0x0018,0xA9E7}; 
pascal void LAutoScroll(ListHandle lHandle)
    = {0x3F3C,0x0010,0xA9E7}; 
pascal void LSetCell(void *dataPtr,short dataLen,Cell theCell,ListHandle lHandle)
    = {0x3F3C,0x0058,0xA9E7}; 
pascal OSErr NMInstall(NMRec* nmReqPtr)
    = {0x205F,0xA05E,0x3E80}; 
pascal OSErr NMRemove(NMRec* nmReqPtr)
    = {0x205F,0xA05F,0x3E80}; 
pascal void LSetSelect(Boolean setIt,Cell theCell,ListHandle lHandle)
    = {0x3F3C,0x005C,0xA9E7}; 
pascal Boolean LGetSelect(Boolean next,Cell *theCell,ListHandle lHandle)
    = {0x3F3C,0x003C,0xA9E7}; 
pascal short LAddRow(short count,short rowNum,ListHandle lHandle)
    = {0x3F3C,0x0008,0xA9E7}; 
pascal ListHandle LNew( /* Rect *rView,Rect *dataBounds,Point cSize,
    short theProc,WindowPtr theWindow,Boolean drawIt,Boolean hasGrow,Boolean scrollHoriz,
    Boolean scrollVert */)
    = {0x3F3C,0x0044,0xA9E7}; 
pascal void MoreMasters(void)
    = 0xA036; 

#else
	#include <Packages.h>
#endif

#define TEScrapHandle() (* (Handle*) 0xAB4)
#define TEGetScrapLen() ((long) * (unsigned short *) 0x0AB0)
#define GetCaretTime() (* (unsigned long*) 0x02F4)


