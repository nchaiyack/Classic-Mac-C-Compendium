#include "VideoToolbox.h"
#include <Errors.h>
#include <Files.h>
#include <Resources.h>
void AddResourceToFile(unsigned char *filename,unsigned char *name,ResType type,int id,Handle handle);

void AddResourceToFile(unsigned char *filename,unsigned char *name,ResType type,int id,Handle handle)
{
	OSErr error;
	ResType myResType;
	short RefNo;
	int i;
	FInfo outFileInfo;

	if(handle==NULL)return;
	CreateResFile(filename);
	GetFInfo(filename,0,&outFileInfo);
	outFileInfo.fdType = 'rsrc';	// resource
	outFileInfo.fdCreator = 'RSED';	// ResEdit
	SetFInfo(filename,0,&outFileInfo);
	RefNo = OpenResFile(filename);
	if(RefNo == -1)PrintfExit("Can't create file!\007\n");
	AddResource(handle,type,id,name);
	i=ResError();
	if(i != 0) printf("AddResource error %d\007, ",i);
	if(i == resNotFound) printf("resource not found.\n");
	if(i == resFNotFound) printf("resource file not found.\n");
	if(i == addResFailed) printf("add resource failed.\n");
	if(i == rmvResFailed) printf("remove resource failed.\n");
	CloseResFile(RefNo);
}
