/* GrabSlotDrivers.c
Repeatedly calls the Slot Manager to get every slot driver and puts each in its
own file. These files can then be perused with the ResEdit CODE viewer. 

The ResEdit CODE viewer is a public domain file, for use with ResEdit, distributed by:
Ira L. Ruben
Apple Computer, Inc.
20525 Mariani Ave., MS: 37-A
Cupertino, Ca. 95014
Ira@Apple.Com

Note that GrabSlotDrivers gets the driver directly from the card's configuration
ROM, whereas it is possible that your run-time System is actually superceding
that by a later version of the driver loaded from the System file or by an INIT.
If you want the active version of the driver it would be best to get it from the
device manager, which is what GrabVideoDrivers.c does, though it skips non-video
devices.

HISTORY:
2/89 dgp Wrote it as "video hacker II" using THINK C 3.
1/3/93 dgp updated it to THINK C 5 and renamed it GrabSlotDrivers.c
5/11/93	dgp	added check for Slot Manager, in response to bug report by Jonathan Brecher.
*/
#include "VideoToolbox.h"
#include <Errors.h>
#include <Resources.h>
#include <Traps.h>
typedef struct {
	short flags;
	short blanks[3];
	short open;
	short prime;
	short control;
	short status;
	short close;
	Str255 name;
} VideoDriver;
void AddResourceToFile(unsigned char *filename,unsigned char *name,ResType type,int id,Handle handle);
void GrabSlotDrivers(void);

void main(void)
{
	Require(0);
	if(!TrapAvailable(_SlotManager))
		PrintfExit("Sorry, no Slot Manager.\n");
	MaximizeConsoleHeight();
	printf("Welcome to GrabSlotDrivers.\n"
	"This programs copies all drivers from your slot devices.\n"
	"Note that this gets the driver from the card's configuration ROM, whereas\n"
	"it is possible that your run-time System replaces\n"
	"that by a later version of the driver loaded from the System file or INIT.\n");
	GrabSlotDrivers();
}

void GrabSlotDrivers(void)
{
	SpBlock mySpBlock;
	SEBlock mySEBlock;
	unsigned char name[256],filename[32];
	OSErr error;
	Ptr *handle;
	int i;
	int version;
	VideoDriver *videoDriverPtr;
	unsigned char *bytePtr;

	mySpBlock.spsExecPBlk = (Ptr) &mySEBlock;
	mySpBlock.spSlot = 0;
	mySpBlock.spID = 0;
	mySpBlock.spExtDev = 0;

	while(1){
		error = SNextSRsrc(&mySpBlock);
		if(error==smNoMoresRsrcs) break;
		if(error){
			printf("SNextSRsrc error %d\n",error);
			break;
		}
		mySpBlock.spResult = (unsigned long) &name;
		error = SReadDrvrName(&mySpBlock);
		printf("SReadDrvrName=%#s\n",name);	
		printf("spSlot=%d,spID=%d,spExtDev=%d"
			,mySpBlock.spSlot,mySpBlock.spID,mySpBlock.spExtDev);
		printf(",spCategory=%d,spCType=%d,spDrvrSW=%d,spDrvrHW=%d,spRefNum=%d\n"
			,mySpBlock.spCategory,mySpBlock.spCType,mySpBlock.spDrvrSW
			,mySpBlock.spDrvrHW,mySpBlock.spRefNum);

		/* filename must be truncated to 31 characters */
		for (i=0;i<32;i++) filename[i]=name[i];
		if((unsigned char)filename[0] > 31) filename[0]=31;
		// Replace leading "." by "-" since the Mac filing system 
		// is confused by filenames beginning with ".".
		if(filename[1]=='.')filename[1]='-';
		
		/* get handle to desired driver resource */
		error = SGetDriver(&mySpBlock);
		if(!error) {
			handle = (Ptr *) mySpBlock.spResult;
	
			// Is this a video driver?
			videoDriverPtr=*(VideoDriver **)handle;
			if(EqualString(name,videoDriverPtr->name,1,1)){
				// Probably is a video driver. The version number of a video driver 
				// is the first word-aligned word after the name string.
				bytePtr= videoDriverPtr->name;
				bytePtr += 1+bytePtr[0];	/* go to end of Pascal string */
				bytePtr = (unsigned char *)((long)(bytePtr+1) & ~1);	// round up to word boundary
				version = *(short *)bytePtr;
			} else version=rand();
		
			AddResourceToFile(filename,name,'DRVR',version,handle);
			printf("Driver copied to �%#s� file, using %d as id.\n",filename,version);
			DisposHandle(handle);
		}
		else printf("No driver.\n");
		printf("\n");
	}
}

