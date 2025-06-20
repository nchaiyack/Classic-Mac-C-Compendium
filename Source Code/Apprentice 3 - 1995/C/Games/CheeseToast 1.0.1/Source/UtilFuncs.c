// Utilfuncs.c
#include <stdarg.h>
#include <string.h>

short MyRandom(short limit)
{
	unsigned long r;
	r = (unsigned) Random();
	r = (r*(long)limit)/65536L;
	return (short) r;
}

void GetInputFileSpec(SFReply *theReply,StringPtr prompt,long type)
{
	Point	sfLoc;
	short	nbrTypes;
	SFTypeList	theList;

	if (type) {
		nbrTypes = 1;
		theList[0] = type;
	}
	else {
		nbrTypes = -1;
	}

	sfLoc.h = -1;
	sfLoc.v = -1;
	SFGetFile(sfLoc, prompt, NULL, nbrTypes, theList,
				NULL, theReply);
}

void GetOutputFileSpec(SFReply *theReply,StringPtr prompt,StringPtr defName)
{
	Point	sfLoc;


	sfLoc.h = -1;
	sfLoc.v = -1;

	SFPutFile(sfLoc, prompt, defName, NULL, theReply);

}


int MyFGets(short refNum, char *buf)
{

#define EOL			13
#define MaxBufSize	256

	IOParam	pb;
	OSErr	oe;

	pb.ioCompletion = NULL;
	pb.ioRefNum = refNum;
	pb.ioBuffer = (Ptr) buf;
	pb.ioReqCount = MaxBufSize;
	pb.ioPosMode = fsAtMark | 0x80 | ('\r' << 8);
	pb.ioPosOffset = 0;

	oe = PBRead((ParmBlkPtr) &pb, false);
	if (oe == noErr || oe == eofErr && pb.ioActCount > 0) {
		buf[pb.ioActCount] = 0;	// Add a nul to the end
		return 1;
	}
	else
		return 0;
}

void ErrorExit(char *str,...)
{
	char tbuf[128];
	va_list args;
	va_start(args,str);
	vsprintf(tbuf,str,args);
	va_end(args);

	CtoPstr(tbuf);
	ParamText((StringPtr) tbuf,"\p","\p","\p");
	Alert(128,NULL);
	exit(1);
}

// Assume there is enough space in string to change it
void ChangeFileExtension(StringPtr fName, StringPtr ext)
{
	char 	*p,*fp;
	PtoCstr(fName);
	PtoCstr(ext);
	fp = (char *) fName;
	if ((p = strchr(fp,'.')) == NULL)
		p = fp + strlen(fp);
	strcpy(p,(char *) ext);
	CtoPstr((char *) fName);
	CtoPstr((char *) ext);
}

short min(register short x, register short y)
{
	if (x < y)
		return x;
	else
		return y;
}

short max(register short x, register short y)
{
	if (x > y)
		return x;
	else
		return y;
}

