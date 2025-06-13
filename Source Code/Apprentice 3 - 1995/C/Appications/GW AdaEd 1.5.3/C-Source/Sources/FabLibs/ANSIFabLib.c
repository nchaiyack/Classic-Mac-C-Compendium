#include	<StdIO.h>

#include	"UtilsSys7.h"

#ifndef __FSPCOMPAT__
#include "FSpCompat.h"
#endif

#include	"Independents.h"
#include	"MacMemory.h"
#include	"ANSIFabLib.h"


fabFILE *fab_fopen(const char *sourcefilename, SInt8 permission)
{
Str255	tempFName;
IOParam	mypb;
EventRecord	dummyEv;
FSSpec	tempFSSp;
fabFILE	*retptr;

retptr = mmalloc(sizeof(fabFILE));
if (retptr) {
	fabc2pstr((unsigned char *)sourcefilename, tempFName);
	if (noErr == FSMakeFSSpecCompat(0, 0, tempFName, &tempFSSp)) {
		if (noErr == FSpOpenDFCompat(&tempFSSp, permission, &retptr->fileRefNum)) {
			mypb.ioCompletion = nil;
			mypb.ioRefNum = retptr->fileRefNum;
			if (noErr == GetEOF(retptr->fileRefNum, &retptr->FileSize)) {
				if (retptr->MyBuffer = mmalloc(retptr->FileSize)) {
					mypb.ioBuffer = (Ptr)retptr->MyBuffer;
					mypb.ioReqCount = retptr->FileSize;
					mypb.ioPosMode = fsFromStart/* | kNoCacheMask*/;
					mypb.ioPosOffset = 0L;
					(void) PBReadAsync((ParmBlkPtr)&mypb);
					
					while (mypb.ioResult > 0) {
						SystemTask();
						(void)EventAvail(everyEvent, &dummyEv);
						}
					if (mypb.ioResult == noErr) {
						retptr->Current = retptr->MyBuffer;
						}
					else {
						(void) PBCloseAsync((ParmBlkPtr)&mypb);
						mfree(retptr->MyBuffer);
						mfree(retptr);
						retptr = NULL;
						}
					}
				else {
					(void) PBCloseAsync((ParmBlkPtr)&mypb);
					mfree(retptr);
					retptr = NULL;
					}
				}
			else {
				(void) PBCloseAsync((ParmBlkPtr)&mypb);
				mfree(retptr);
				retptr = NULL;
				}
			}
		else {
			mfree(retptr);
			retptr = NULL;
			}
		}
	}
return retptr;
}

void fab_fclose(fabFILE *theFile)
{
IOParam	mypb;

if (theFile) {
	mypb.ioCompletion = nil;
	mypb.ioRefNum = theFile->fileRefNum;
	(void) PBCloseAsync((ParmBlkPtr)&mypb);
	if (theFile->MyBuffer)
		mfree(theFile->MyBuffer);
	mfree(theFile);
	}
}

Boolean fab_feof(fabFILE *theFile)
{
return theFile->Current >= theFile->MyBuffer + theFile->FileSize;
}

int fab_getc(fabFILE *theFile)
{
return fab_feof(theFile) ? EOF : *theFile->Current++;
}

Size fab_fread(char *dest, Size wanted, fabFILE *theFile)
{
Size	correction;

correction = theFile->MyBuffer + theFile->FileSize - theFile->Current;
correction = MIN(wanted, correction);
BlockMoveData(theFile->Current, dest, correction);
theFile->Current += correction;
return correction;
}

Size fab_ftell(const fabFILE *file)
{
return file->Current - file->MyBuffer;
}

void fab_fseek(fabFILE *file, Size offset)
{
file->Current = file->MyBuffer + offset;
}

