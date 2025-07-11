/*
GrabVideoDrivers.c
Gets each active video driver and saves it in a file of the same name, using the
driver's version number as a resource id. These files can later be examined in
ResEdit, using the public domain CODE viewer. You'll want to consult Apple's
Designing Cards and Drivers, 3rd Ed., Addison Wesley.

The ResEdit CODE viewer is a public domain file, for use with ResEdit,
distributed by:
Ira L. Ruben
Apple Computer, Inc.
20525 Mariani Ave., MS: 37-A
Cupertino, Ca. 95014
Ira@Apple.Com

Copyright restrictions prevent us from distributing video drivers themselves,
but this program makes it easy for you to get an accessible copy of all the
video drivers used in your own computer.

The primary users of this program will probably be those who wish to enhance
SetEntriesQuickly.c. I hope that people will share the fruits of their labors by
sending me the enhancements for distribution in the VideoToolbox disk.

This program is also useful for copying the Mac IIsi video driver to the Mac
IIci. Since that's a ROM-based driver I don't know any way to automatically
determine its size, and instead this program uses a generous guess. Consequently
the resulting resource is larger than it needs to be. When you install this
resource in the System file of your Mac IIci the excess space is wasted (both on
disk and in memory), but the excess is only about 1 kB, so it doesn't seem worth
worrying about.

HISTORY:
2/89 dgp Wrote it as "video hacker II.c" using THINK "Light Speed" C 3.
12/30/92 dgp updated it to THINK C 5 and renamed it GrabVideoDrivers.c
*/
#include "VideoToolbox.h"
#include <Errors.h>
#include <Files.h>
#include <Resources.h>
#define dRAMBased		0x0040
Handle GetDriverFromSlotManager(GDHandle device);

void main(void);
void CopyDeviceDriver(GDHandle device);
void AddResourceToFile(unsigned char *filename,unsigned char *name,ResType type
	,int id,Handle handle);
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
VideoDriver *GDDriverAddress(GDHandle device);

void main()
{
	GDHandle device;
	int i;
	
	MaximizeConsoleHeight();
	printf("Welcome to GrabVideoDriver.\n");
	printf("This program cycles through all your video devices and copies\n"
	"each driver into a resource file, suitable for subsequent examination\n"
	"with ResEdit (with the CODE viewer). This is primarily of interest for people\n"
	"who want to enhance SetEntriesQuickly.c to support more video devices.\n");
	for(i=0;i<10;i++){
		device=GetScreenDevice(i);
		if(device==NULL)break;
		CopyDeviceDriver(device);
	}
	printf("\nDone.\n");
}

void CopyDeviceDriver(GDHandle device)
{
	OSErr error;
	unsigned char *name, filename[32];
	int i;
	long driverSize;
	AuxDCE **myAuxDCEHandle;
	Handle handle;
	VideoDriver *driver;
	unsigned char *bytePtr;
	int version;
	
	if(device==NULL)return;
	printf("\n");
	name = GDName(device);
	version=GDVersion(device);
	printf("%#s version %d.\n",name,version);
	
	/* filename must be truncated to 31 characters */
	for (i=0;i<32;i++) filename[i]=name[i];
	if((unsigned char)filename[0] > 31) filename[0]=31;
	// Replace leading "." by "-" since the Mac filing system 
	// is confused by filenames beginning with ".".
	if(filename[1]=='.')filename[1]='-';

	driver=GDDriverAddress(device);
	printf("Offsets: open 0x%x, prime 0x%x, control 0x%x, status 0x%x, close 0x%x\n"
		,driver->open,driver->prime,driver->control,driver->status,driver->close);
	printf("name is at 0x%lx, ",driver->name-(unsigned char *) driver);
	bytePtr=driver->name;
	bytePtr += 1+bytePtr[0];	/* go to end of Pascal string */
	bytePtr = (unsigned char *)((long)(bytePtr+1) & ~1);	// round up to word boundary
	printf("version is at 0x%lx\n",bytePtr-(unsigned char *) driver);

	myAuxDCEHandle = (AuxDCE **) GetDCtlEntry((*device)->gdRefNum);
	if((**myAuxDCEHandle).dCtlFlags & dRAMBased){
		printf("RAM-based driver.\n");
		handle=(Handle)(**myAuxDCEHandle).dCtlDriver;
		HandToHand(&handle);
		printf("%ld bytes\n",GetHandleSize(handle));
	}
	else{
		printf("ROM-based driver.\n");
		handle=GetDriverFromSlotManager(device);
		if(handle!=NULL){
			printf("Got driver with help from Slot Manager.\n");
			printf("%ld bytes\n",GetHandleSize(handle));
		}else{
			// We have a Ptr to the driver, but we don't know how big it is.
			driverSize=(unsigned long) GetPtrSize((Ptr)(**myAuxDCEHandle).dCtlDriver);
			if(driverSize==0 || MemError()){
				driverSize=driver->open;
				if(driverSize<driver->prime)driverSize=driver->prime;
				if(driverSize<driver->control)driverSize=driver->control;
				if(driverSize<driver->status)driverSize=driver->status;
				if(driverSize<driver->close)driverSize=driver->close;
				driverSize*=2;
				printf("Size unknown, guessing (generously) at %ld, twice the highest offset.\n",driverSize);
			}else printf("%ld bytes\n",driverSize);
			PtrToHand((**myAuxDCEHandle).dCtlDriver,&handle,driverSize);
		}
	}
	if(handle!=NULL){
		AddResourceToFile(filename,name,'DRVR',version,handle);
		printf("Driver copied to �%#s� file, using the version number %d as the resource id.\n",filename,version);
		DisposHandle(handle);
	}else printf("Couldn't copy driver.\n");
}

/*
This gets a copy of the driver from the slot manager. Returns NULL unless we can
find exactly the same driver as is specified by the supplied GDHandle. We check
every byte. This is not a useless operation, because although we already have
the address of the driver, we don't necessarily already have its size, and the
slot manager will supply us with a handle, from which we can obtain the size.
*/
Handle GetDriverFromSlotManager(GDHandle device)
{
	SpBlock mySpBlock;
	SEBlock mySEBlock;
	unsigned char *desiredName,name[256],filename[32];
	OSErr error;
	Ptr *handle;
	int i;
	
	desiredName=GDName(device);
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
		if(!EqualString(desiredName,name,1,1))continue;
		error = SGetDriver(&mySpBlock);
		if(!error)continue;
		handle = (Handle) mySpBlock.spResult;
		if(memcmp(*handle,GDDriverAddress(device),GetHandleSize(handle))!=0){
			DisposHandle(handle);
			break;
		}
		return handle;
	}
	return NULL;
}