
#include <stdio.h>
#include <math.h>
#include <console.h>

//--------------------------------------------------------  ToolBoxInit

void ToolBoxInit (void)

{
	short		count;
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
}

//--------------------------------------------------------  DebugNum

void DebugNum (long value)

{
	Str255		theString;
	
	NumToString(value, (StringPtr)&theString);
	DebugStr(theString);	
}

//--------------------------------------------------------  DumpToResEditFile

void DumpToResEditFile (StringPtr fileName, ResType type, Ptr data, long dataSize)

{
	Handle		newResource;
	OSErr		theErr;
	short		vRefNum = 0;
	short		iFileRef;
	
	theErr = Create(fileName, vRefNum, 'RSED', 'rsrc');
	if (theErr != noErr)
		DebugStr("\pCouldn't Create()");
	
	CreateResFile(fileName);
	if (ResError() != noErr)
		DebugStr("\pCouldn't CreateResFile()");
	
	iFileRef = OpenResFile(fileName);
	if ((ResError() != noErr) || (iFileRef == -1))
		DebugStr("\pCouldn't OpenResFile()");
	
	if (PtrToHand(data, &newResource, dataSize) != noErr)
			DebugStr("\pCouldn't PtrToHand()");
	AddResource(newResource, type, 128, "\p");
	ChangedResource(newResource);
}

//--------------------------------------------------------  main

void main (void)

{
	#define		kMaxDegrees			3600
	#define		kAngleScale			8192.0
	#define		kPi					 3.14159
	
	double		sinNumber, cosNumber, realAngle;
	long		dummyLong;
	short		i;
	short		sinTable[kMaxDegrees];
	short		cosTable[kMaxDegrees];
	
	ToolBoxInit();
	console_options.pause_atexit = 1;
	
	for (i = 0; i < kMaxDegrees; i++)
	{
		realAngle = (double)i * (double)kPi / (double)kMaxDegrees;
		sinNumber = sin(realAngle) * (double)kAngleScale;
		cosNumber = cos(realAngle) * (double)kAngleScale;
		
		sinTable[i] = (short)sinNumber;
		cosTable[i] = (short)cosNumber;
		
		printf("Sin %d \n", sinTable[i]);
		printf("Cos %d \n", cosTable[i]);
	}
	Delay(600, &dummyLong);
	
	DumpToResEditFile ("\pSin Table", 'sin ', (Ptr)&sinTable, sizeof(sinTable));
	DumpToResEditFile ("\pCos Table", 'cos ', (Ptr)&cosTable, sizeof(cosTable));
}