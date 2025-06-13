/**********************************************************************************************
This code shows how use the Picture Utilities Package to extract the colors from a PICT file. 
Error checking will have to be added. Written by Steve Wagy, CIS 70471,1640.
**********************************************************************************************/

/********************************************************************
 Ported to Metrowerks CodeWarrior by Paul Celestin on 4 November 1994
 ********************************************************************/

#include <Files.h>
#include <PictUtil.h>
#include <QuickDraw.h>

short			globalRef, numTypes;
long			longCount, myEOF, filePos;     	
WindowRecord	wStorage;
CQDProcs    	myProcs;   
CQDProcsPtr   	savedProcs;
Rect		  	myRect, rect;   	  
OSErr		  	err;
PicHandle 		myPic;
WindowPtr		myWindow;      					
PaletteHandle	thePalette;		
SFReply			reply;
PictInfo		thePictInfo;
Point			where;
SFTypeList		typeList;
short     		lockerr;


pascal void GetPICTData(Ptr dataPtr,int byteCount)
{ 
	longCount = byteCount;
	err = FSRead(globalRef,&longCount,dataPtr);
} 


main()
{
	ToolBoxInit();
	Open();
}


ToolBoxInit()
{
	MaxApplZone();
	MoreMasters();
	MoreMasters();
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}


Open()
{	
	where.h = 150;				
	where.v = 120;
	typeList[0] = 'PICT';
	numTypes = 1;
	
	SFGetFile(where,"\p",0L,numTypes,typeList,0L,&reply);
	if	(!reply.good)
		return;
	err = FSOpen(reply.fName,reply.vRefNum,&globalRef);
	myPic = (PicHandle)NewHandle(sizeof(Picture));
    err = SetFPos(globalRef,fsFromStart,512);
    err = GetEOF(globalRef,&myEOF);
    longCount = sizeof(Picture);
    err = FSRead(globalRef,&longCount,*myPic);
    rect = (*myPic)->picFrame;
    myRect.top = rect.top + 20;
    myRect.bottom = rect.bottom + 20;
    myRect.left = rect.left;  
    myRect.right = rect.right;
    myWindow = NewCWindow(&wStorage,&myRect,reply.fName,true,2,(WindowPtr)(-1),false,0L);
    SetPort(myWindow);
    SetStdCProcs(&myProcs);  
    myProcs.getPicProc = (QDGetPicUPP)GetPICTData;
    savedProcs = (CQDProcsPtr)(*qd.thePort).grafProcs;
    (*qd.thePort).grafProcs = (QDProcsPtr)&myProcs;
    err = GetFPos(globalRef,&filePos);
    err = GetPictInfo(myPic,&thePictInfo,returnColorTable,256,systemMethod,0);
    err = SetFPos(globalRef,fsFromStart,filePos);
	thePalette = NewPalette(256,thePictInfo.theColorTable,pmTolerant,0);
    SetPalette(myWindow,thePalette,true);
    DrawPicture(myPic,&myWindow->portRect);
    err = FSClose(globalRef);
    (*qd.thePort).grafProcs = (QDProcsPtr)savedProcs;

	while	(!Button());
	CloseWindow(myWindow);
	DisposeHandle((Handle)myPic);
	DisposeHandle((Handle)thePictInfo.theColorTable);
	DisposePalette(thePalette);
	ExitToShell();
}   

