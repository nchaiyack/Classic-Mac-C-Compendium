/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
short GetMyVR(UPtr name);
long GetMyDirID(short refNum);
short GetDirName(UPtr volName,short vRef, long dirId,UPtr name);
UPtr GetMyVolName(short refNum,UPtr name);
int BlessedDirID(long *sysDirIDPtr);
int MakeResFile(UPtr name,int vRef,long dirId,long creator,long type);
short DirIterate(short vRef,long dirId,HFileInfo *hfi);
int CopyFBytes(short fromRefN,long fromOffset,long length,short toRefN,long toOffset);
Boolean MySFGetFile(ProcPtr filter, long *dirIdPtr, short *vRefPtr, UPtr namePtr,short typeCount,SFTypeList tl);
void StdFileSpot(Point *where, short id);
short RFHOpen(UPtr name,short vRefN,long dirId,short *refN,short perm);
int MyAllocate(short refN,long size);
short SFPutOpen(UPtr name,short *vRef,long creator,long type,short *refN,ProcPtr filter,short id);
Boolean IsText(UPtr volName,long dirId,UPtr name);
short MyOpenResFile(short vRef,long dirId,UPtr name);
short SpinOn(short *rtnCodeAddr,long maxTicks);
#define FSZWrite(refN,count,buf) (((*count)>0) ? FSWrite(refN,count,buf):0)
Boolean IsItAFolder(short vRef,long inDirId,UPtr name);
short FolderFileCount(long inDirId,UPtr name);
short FSHOpen(UPtr name,short vRefN,long dirId,short *refN,short perm);
short GetMyWD(short vRef,long dirID);
short HMove(short vRef,long fromDirId,UPtr fromName,long toDirId,UPtr toName);
short HGetFileInfo(short vRef,long dirId,UPtr name,HFileInfo *hfi);
short HSetFileInfo(short vRef,long dirId,UPtr name,HFileInfo *hfi);
Boolean GetFolder(char *name,short *volume,long *folder,Boolean writeable,Boolean system,Boolean floppy,Boolean desktop);
short CopyRFork(short vRef,long dirId,UPtr name,short fromVRef,
								short fromDirId,Uptr fromName);
short CopyFInfo(short vRef,long dirId,UPtr name,short fromVRef,
								short fromDirId,Uptr fromName);
short MyResolveAlias(short *vRef,long *dirId,UPtr name,Boolean *wasAlias);
short FSWriteP(short refN,UPtr pString);
short GetFileByRef(short refN,short *vRef,long *dirId,UPtr name);
long VolumeFree(short vRef);
short FSTabWrite(short refN,long *count,UPtr buf);