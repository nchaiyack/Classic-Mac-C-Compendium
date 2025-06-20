/*
SetFileInfo.c

USAGE

	SetFileInfo("Output File",'xxxx','yyyy');

ARGUMENTS

	char	*fileName;		The name of the file (including HFS
							directory name) to be set.
	
	OStype	'xxxx'			The four character file type, eg 'TEXT'

	OStype	'xxxx'			The four character creator, eg 'XCEL','CGRF','QKPT','QED1'

COMMENTS

	The syntax 'xxxx' results in four characters packed into a short int. This is
	a Macintosh convention, not Standard C. That's ok since this routine is strictly
	Macintosh anyway.

EXAMPLE

	** create Cricket data file **
	myFile=fopen("LuminanceCalibration.data","w");
	SetFileInfo("LuminanceCalibration.data",'TEXT','CGRF');
	fprintf(myFile,"*\n");
	fprintf(myFile,"x\ty\n");
	for(i=0;i<n;i++)fprintf(myFile,"%9g\t%9g\n",x[i],y[i]);
	fclose(myFile);

	
HISTORY
12/87	EJC wrote q_make_crick()
8/89	dgp Renamed and rewritten.
8/24/91	dgp	Made compatible with THINK C 5.0.
12/28/92 dgp Removed obsolete support for THINK C 4.

*****************************************************************************/
#include "VideoToolbox.h"	/* for prototype of itself */
#include <Files.h>

void SetFileInfo(char *fileName,OSType fileType,OSType fileCreator)
{
	FInfo outFileInfo;
	
	CtoPstr(fileName);
	GetFInfo((StringPtr)fileName,0,&outFileInfo);
	outFileInfo.fdType = fileType;
	outFileInfo.fdCreator = fileCreator;
	SetFInfo((StringPtr)fileName,0,&outFileInfo);
	PtoCstr((void *)fileName);
}
