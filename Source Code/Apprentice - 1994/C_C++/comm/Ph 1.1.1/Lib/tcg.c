/*______________________________________________________________________

	tcg.c - Think C Glue.
	
	These routines are supplied as glue in MPW, but are missing in Think C,
	even though they're prototyped in <Resources.h>.
	
	Written by Ephraim Vishniac.
_____________________________________________________________________*/

#include <Pascal.h>

short openrfperm(char *fileName,short vRefNum,char permission)
{
	short retval;

	CtoPstr(fileName);
	retval = OpenRFPerm((unsigned char *)fileName, vRefNum, permission);
	PtoCstr((unsigned char *)fileName);
	
	return retval;
}

short openresfile(char *fileName)
{
	short retval;
	CtoPstr(fileName);
	retval = OpenResFile((unsigned char *)fileName);
	PtoCstr((unsigned char *)fileName);
	
	return retval;
}

void createresfile(char *fileName)
{
	CtoPstr(fileName);
	CreateResFile((unsigned char *)fileName);
	PtoCstr((unsigned char *)fileName);
}

void getresinfo(Handle theResource,short *theID,ResType *theType,char *name)
{
	GetResInfo(theResource, theID, theType, (unsigned char *)name);
	PtoCstr((unsigned char *)name);
}

void setresinfo(Handle theResource,short theID,char *name)
{
	if (name != NULL) CtoPstr(name);
	SetResInfo(theResource, theID, (unsigned char *)name);
	if (name != NULL) PtoCstr((unsigned char *)name);
}

void addresource(Handle theResource,ResType theType,short theID,char *name)
{
	if (name != NULL) CtoPstr(name);
	AddResource(theResource, theType, theID, (unsigned char *)name);
	if (name != NULL) PtoCstr((unsigned char *)name);
}
