/* MTPStuff - Implementation of the MTP client routines                       */

#ifndef __MTPSTUFF__
#define __MTPSTUFF__

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif


pascal void CopyP2CStr (unsigned char *aPStr, unsigned char *aCStr);
pascal long GetFileLength (TMUDDoc *theDoc, short fRef);
pascal void PStrCat (unsigned char *str, unsigned char *src);

pascal void MTPDirSetup (TMUDDoc *theDoc, short vRefNum, long dirID);
pascal void MTPDirCheck (TMUDDoc *theDoc);

pascal void MTPBuildPath (TMUDDoc *theDoc, AppFile *theFile, Str255 path);
pascal void MTPReceive (TMUDDoc *theDoc, Str255 path, Str255 fileName,
		short vRefNum);
pascal void MTPSend (TMUDDoc *theDoc, AppFile *theFile, Str255 path);

#endif
