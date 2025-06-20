#include <stdio.h>
#include <string.h>
#include <SetUpA4.h>

#undef _TEST


#define ERRORDLOG			256
#define INFODLOG			257
#define MAPFILENAME			"HD Dirs List"
#define FILELISTNAME		"HD File List"
#define CREATOR				'Gbge'
#define STUDENTDIRECTORY	"Student Work"

#define EQSTR			0



typedef  struct myVolumeInfoBlock
{
char		name[32];
int			attrib;
char		finderInfo[32];
};

#define VolumeInfo	struct myVolumeInfoBlock


typedef struct myDirInfoBlock
{
char		name[32];
int			attrib;
long		dirID;
long		parentDirID;
char		finderInfo[32];
};

#define DirInfo	struct myDirInfoBlock


typedef struct myFileInfoBlock
{
char	name[32];
int		attrib;
long	creator;
long	type;
long	creationDate;
long	parentDirID;
char	finderInfo[32];
};

#define FileInfo	struct myFileInfoBlock






static CInfoPBRec	myCPB;
static int			vRefNum;
static char			errorString[256];
static char			currentDirName[256];
static DialogPtr	theDPtr = 0L;
static long			numDirs = 0L,numFiles = 0L;
static VolumeInfo	theVolInfo;
static DirInfo		theDirInfo,*listOfDirs = 0L;
static FileInfo		theFileInfo,*listOfFiles = 0L;
static long			studentWorkDirID = 0L;
static int			sysFolderVRefNum = 0;


int main(void);
static void	OpenInfoWindow(void);
static void	SetInfoMessage(char *theMessage);
static void	CloseInfoWindow(void);
static int	InfoBreak(void);
static int SetupVolRefNum(void);
static void CenterWindow(WindowPtr wPtr);
static pascal void HiliteDefaultButton(DialogPtr theDPtr,int whichItem);
static void ErrorAlert(char *p0);
static int LoadAndParseFiles(void);
static void DisposeFileList(void);
static int LoadAndParseDirs(void);
static void DisposeDirList(void);
static int OptionKey(void);
static int CompareFiles(FileInfo *first,FileInfo *second);
static long FindFile(FileInfo *theFileInfo);
static int CompareDirs(DirInfo *first,DirInfo *second);
static long FindDir(DirInfo *theDirInfo);
static int ResetVolumeAndDirs(void);
static int ResetFilesAndCollectGarbage(void);
static int  ProcessDirectory(long theIODirID);



int main(void)
{
	RememberA0();
	SetUpA4();
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
	
	DeskHook = 0L;
	DragHook = 0L;
	
	ResetVolumeAndDirs();
	ResetFilesAndCollectGarbage();
	
	DisposeFileList();
	DisposeDirList();
	
	CloseInfoWindow();
	
	FlushEvents(everyEvent,0);
	
	RestoreA4();
}





static void	OpenInfoWindow(void)
{
int				itemHit;
EventRecord		theEvent;
DialogPtr		tempDPtr;
GrafPtr			oldPort;

	GetPort(&oldPort);

	theDPtr = GetNewDialog(INFODLOG, 0L,(WindowPtr) -1L);
	
	if (!theDPtr)
	{
		ErrorAlert("Can't get INFODLOG resource!");
		ExitToShell();
	}
	
	SetPort(theDPtr);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	while (!GetNextEvent(updateMask,&theEvent) || theEvent.message != (long) theDPtr);
	
	DialogSelect(&theEvent,&tempDPtr,&itemHit);
	
	SetPort(oldPort);
}




static void	SetInfoMessage(char *theMessage)
{
int				itemHit,type,oldFont,oldSize;
Handle			theItem;
Rect			theRect;
char			tempString[256];
GrafPtr			oldPort;
RgnHandle		theRgn;

	if (!theDPtr)
		OpenInfoWindow();
	
	GetPort(&oldPort);
	SetPort(theDPtr);
	
	oldFont = theDPtr->txFont;
	oldSize = theDPtr->txSize;
	
	strcpy(tempString,theMessage);
	CtoPstr(tempString);
	GetDItem(theDPtr,2,&type,&theItem,&theRect);
	
	InsetRect(&theRect,-2,-2);
	FrameRect(&theRect);
	InsetRect(&theRect,2,2);
	
	theRgn = NewRgn();
	ScrollRect(&theRect,0,-12,theRgn);
	DisposeRgn(theRgn);
	ClipRect(&theRect);
	MoveTo(theRect.left + 3,theRect.bottom - 3);
	
	TextFont(geneva);
	TextSize(10);
	DrawString(tempString);
	
	
	SetRect(&theRect,-32768,-32768,32767,32767);
	ClipRect(&theRect);
	
	TextFont(oldFont);
	TextSize(oldSize);
	SetPort(oldPort);
	
	if (OptionKey())
	{
		while (!Button());
		while (Button());
	}
}



static void	CloseInfoWindow(void)
{
	DisposDialog(theDPtr);
	
	theDPtr = 0L;
}




static int	InfoBreak(void)
{
int				itemHit, type;
Handle			theItem;
Rect			theRect;
EventRecord		theEvent;
DialogPtr		tempDPtr;


	itemHit = 0;
	
	if (GetNextEvent(everyEvent,&theEvent))
	{
		if (!IsDialogEvent(&theEvent) || !DialogSelect(&theEvent,&tempDPtr,&itemHit) || tempDPtr!=theDPtr)
			itemHit = 0;
	}
	
	if (itemHit == 1)
		return(TRUE);
	else
		return(FALSE);
}





static int SetupVolRefNum(void)
{
long		freeBytes;
int			errCode;
char		volName[256];
SysEnvRec	theWorld;


#ifndef _TEST

	if ((errCode = GetVInfo(0,volName,&vRefNum,&freeBytes)) != noErr)
	{
		sprintf(errorString,"SetupVolRefNum:\rGetVInfo() error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = SysEnvirons(1,&theWorld)) != noErr)
	{
		sprintf(errorString,"SetupVolRefNum:\rSysEnvirons() error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	sysFolderVRefNum = theWorld.sysVRefNum;

#else
	
	if ((errCode = GetVol(volName,&vRefNum)) != noErr)
	{
		sprintf(errorString,"SetupVolRefNum:\rGetVol() error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	sysFolderVRefNum = vRefNum;

#endif

	
	return(noErr);
}




static void CenterWindow(WindowPtr wPtr)
{
int		screenWidth,screenHeight,windowWidth,windowHeight,left,top;

	if (wPtr == 0L)
		return;
	
	screenWidth = screenBits.bounds.right - screenBits.bounds.left;
	screenHeight = screenBits.bounds.bottom - screenBits.bounds.top - 20;
	
	windowWidth = wPtr->portRect.right - wPtr->portRect.left;
	windowHeight = wPtr->portRect.bottom - wPtr->portRect.top;
	
	
	left = screenBits.bounds.left + (screenWidth - windowWidth)/2;
	
	top = screenBits.bounds.top + 20 + (screenHeight - windowHeight)/2;
	
	if (left < 0)
		left = 0;
	
	if (top < 20)
		top = 20;
	
	MoveWindow(wPtr,left,top,FALSE);
}



static pascal void HiliteDefaultButton(DialogPtr theDPtr,int whichItem)
{
int			type;
Handle		theItem;
Rect		theRect;

	GetDItem(theDPtr, ((DialogPeek) theDPtr)->aDefItem, &type, &theItem, &theRect);
	PenNormal();
	PenSize(3,3);
	InsetRect(&theRect,-4,-4);
	FrameRoundRect(&theRect,16,16);
	PenSize(1,1);
}



static void ErrorAlert(char *p0)
{
GrafPtr			oldPort;
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type;
Handle			theItem;
Rect			theRect;
EventRecord		theEvent;

	GetPort(&oldPort);
	
	InitCursor();
	
	if (!(theDPtr = GetNewDialog(ERRORDLOG, NULL,(WindowPtr) -1L)))
	{
		SysBeep(1);
		ExitToShell();
	}
	
	SetPort(theDPtr);
	
	CenterWindow(theDPtr);
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 3, &type, &theItem, &theRect);
	SetDItem(theDPtr, 3, type, HiliteDefaultButton, &theRect);
	
	GetDItem(theDPtr, 2, &type, &theItem, &theRect);
	CtoPstr(p0);
	SetIText(theItem, p0);
	PtoCstr(p0);
	
	if (GetNextEvent(updateMask,&theEvent) && theEvent.message == (long) theDPtr)
		DialogSelect(&theEvent,&tempDPtr,&itemHit);
	
	SysBeep(1);
	
	do 
	{
		itemHit = 0;
		
		while (!GetNextEvent(everyEvent,&theEvent));
		
		if (theEvent.what==keyDown || theEvent.what==autoKey)
		{
			if ((theEvent.message & charCodeMask)=='\r' || (theEvent.message & charCodeMask)==0x03)
				itemHit = ((DialogPeek) theDPtr)->aDefItem;
			else
				SysBeep(1);
		}
		
		else
		{
			tempDPtr = (DialogPtr) 0L;
			
			if (!IsDialogEvent(&theEvent) || !DialogSelect(&theEvent,&tempDPtr,&itemHit) || tempDPtr!=theDPtr)
				itemHit = 0;
		}
		
	} while (itemHit!=1);
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
}




static int LoadAndParseFiles(void)
{
char		filesFName[256];
int			errCode;
int			filesFRefNum;
long		byteCount,theSize;


	DisposeFileList();
	
	errCode = SetupVolRefNum();
	
	if (errCode != noErr)
		return(errCode);
	
	strcpy(filesFName,FILELISTNAME);
	CtoPstr(filesFName);
	
	errCode=FSOpen(filesFName,sysFolderVRefNum,&filesFRefNum);
		
	if (errCode!=noErr)
	{
		sprintf(errorString,"LoadAndParseFiles:\rFSOpen('%s') error %d",FILELISTNAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = SetFPos(filesFRefNum,fsFromStart,0L))!=noErr)
	{
		FSClose(filesFRefNum);
		sprintf(errorString,"LoadAndParseFiles:\rSetFPos('%s',fsFromStart) error %d",FILELISTNAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = GetEOF(filesFRefNum,&theSize))!=noErr)
	{
		FSClose(filesFRefNum);
		sprintf(errorString,"LoadAndParseFiles:\rGetEOF('%s') error %d",FILELISTNAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	numFiles = theSize / sizeof(FileInfo);
	
	if (theSize % sizeof(FileInfo))
	{
		DisposeFileList();
		FSClose(filesFRefNum);
		sprintf(errorString,"LoadAndParseFiles:\rbad file size-- %ld extra bytes ",theSize - (numFiles * sizeof(FileInfo)));
		ErrorAlert(errorString);
		return(errCode);
	}
	
	
	listOfFiles = (FileInfo *) NewPtr(numFiles * sizeof(FileInfo));
	
	byteCount = numFiles * sizeof(FileInfo);
	
	if ((errCode = FSRead(filesFRefNum,&byteCount,(Ptr) listOfFiles))!=noErr)
	{
		DisposeFileList();
		FSClose(filesFRefNum);
		sprintf(errorString,"LoadAndParseFiles:\rFSRead('%s') error %d",FILELISTNAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = FSClose(filesFRefNum))!=noErr)
	{
		DisposeFileList();
		sprintf(errorString,"LoadAndParseFiles:\rFSClose('%s') error %d",FILELISTNAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	
	return(noErr);
}




static void DisposeFileList(void)
{
	if (listOfFiles)
		DisposPtr(listOfFiles);
	
	listOfFiles = 0L;
	numFiles = 0L;
}


static int LoadAndParseDirs(void)
{
char		dirsFName[256];
int			errCode;
int			dirsFRefNum;
long		byteCount,theSize;


	DisposeDirList();
	
	errCode = SetupVolRefNum();
	
	if (errCode != noErr)
		return(errCode);
	
	strcpy(dirsFName,MAPFILENAME);
	CtoPstr(dirsFName);
	
	errCode=FSOpen(dirsFName,sysFolderVRefNum,&dirsFRefNum);
		
	if (errCode!=noErr)
	{
		sprintf(errorString,"LoadAndParseDirs:\rFSOpen('%s') error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = SetFPos(dirsFRefNum,fsFromStart,0L))!=noErr)
	{
		FSClose(dirsFRefNum);
		sprintf(errorString,"LoadAndParseDirs:\rSetFPos('%s',fsFromStart) error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = GetEOF(dirsFRefNum,&theSize))!=noErr)
	{
		FSClose(dirsFRefNum);
		sprintf(errorString,"LoadAndParseDirs:\rGetEOF('%s') error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	
	byteCount = sizeof(VolumeInfo);
	
	if ((errCode = FSRead(dirsFRefNum,&byteCount,&theVolInfo))!=noErr)
	{
		FSClose(dirsFRefNum);
		sprintf(errorString,"LoadAndParseDirs:\rFSRead('%s') error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	theSize -= byteCount;
	numDirs = theSize / sizeof(DirInfo);
	
	if (theSize % sizeof(DirInfo))
	{
		DisposeDirList();
		FSClose(dirsFRefNum);
		sprintf(errorString,"LoadAndParseDirs:\rbad file size-- %ld extra bytes ",theSize - (numDirs * sizeof(DirInfo)));
		ErrorAlert(errorString);
		return(errCode);
	}
	
	listOfDirs = (DirInfo *) NewPtr(numDirs * sizeof(DirInfo));
	
	byteCount = numDirs * sizeof(DirInfo);
	
	if ((errCode = FSRead(dirsFRefNum,&byteCount,(Ptr) listOfDirs))!=noErr)
	{
		DisposeDirList();
		FSClose(dirsFRefNum);
		sprintf(errorString,"LoadAndParseDirs:\rFSRead('%s') error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	if ((errCode = FSClose(dirsFRefNum))!=noErr)
	{
		DisposeDirList();
		sprintf(errorString,"LoadAndParseDirs:\rFSClose('%s') error %d",MAPFILENAME,errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	
	return(noErr);
}




static void DisposeDirList(void)
{
	if (listOfDirs)
		DisposePtr(listOfDirs);
	
	listOfDirs = 0L;
	numDirs = 0L;
}





static int OptionKey(void)
{
char	theKeyMap[16];

	GetKeys(theKeyMap);
	
	if (theKeyMap[7] & 0x04)
		return(TRUE);
	else
		return(FALSE);
}







static int CompareFiles(FileInfo *first,FileInfo *second)
{
int		errCode;

	if (first && second)
	{
		errCode = strcmp(first->name,second->name);
		
		if (errCode > 0)
			return(1);
		
		else if (errCode < 0)
			return(-1);
		
		
		if (first->creator > second->creator)
			return(1);
		
		else if (first->creator < second->creator)
			return(-1);
		
		
		
		if (first->type > second->type)
			return(1);
		
		else if (first->type < second->type)
			return(-1);
		
		
		
		if (first->creationDate > second->creationDate)
			return(1);
		
		else if (first->creationDate < second->creationDate)
			return(-1);
		
		
		
		if (first->parentDirID > second->parentDirID)
			return(1);
		
		else if (first->parentDirID < second->parentDirID)
			return(-1);
		
		
		
		return(0);
	}
	
	return(0);
}



static long FindFile(FileInfo *theFileInfo)
{
register long	i,delta,found,matchWeight,tempWeight;

	i = 0;
	
	if (numFiles & 1L)
		delta = (numFiles / 2L) + 1L;
	else
		delta = (numFiles / 2L);
	
	found = -1L;
	
	while (delta >= 1L && found < 0L)
	{
		tempWeight = CompareFiles(&(listOfFiles[i]),theFileInfo);
		
		if (tempWeight == 0L)
			return(i);
		
		else if (tempWeight < 0L)
			i += delta;
		
		else
			i -= delta;
		
		
		if (delta == 1L)
			delta = 0L;
		
		else if (delta & 1L)
			delta = (delta >> 1L) + 1L;
		else
			delta = (delta >> 1L);
	}
	
	found = -1L;
	matchWeight = 0;
	
	for (i = 0;i < numFiles;i++)
	{
		tempWeight = 0;
		
		if (strcmp(theFileInfo->name,listOfFiles[i].name) == EQSTR)
			tempWeight++;
		
		if (theFileInfo->creator == listOfFiles[i].creator)
			tempWeight++;
		
		if (theFileInfo->type == listOfFiles[i].type)
			tempWeight++;
		
		if (theFileInfo->creationDate == listOfFiles[i].creationDate)
			tempWeight++;
		
		if (theFileInfo->parentDirID == listOfFiles[i].parentDirID)
			tempWeight++;
		
		if (theFileInfo->attrib == listOfFiles[i].attrib)
			tempWeight++;
		
		if (strcmp(theFileInfo->finderInfo,listOfFiles[i].finderInfo) == EQSTR)
			tempWeight++;
		
		
		
		if (tempWeight > matchWeight)
		{
			matchWeight = tempWeight;
			found = i;
		}
	}
	
	
	if (found >= 0L && (theFileInfo->creator != listOfFiles[found].creator || 
		theFileInfo->type != listOfFiles[found].type || 
		theFileInfo->creationDate != listOfFiles[found].creationDate))
			found = -1L;
	
	return(found);
}





static int CompareDirs(DirInfo *first,DirInfo *second)
{
int		errCode;

	if (first && second)
	{
		errCode = strcmp(first->name,second->name);
		
		if (errCode > 0)
			return(1);
		
		else if (errCode < 0)
			return(-1);
		
		
		if (first->dirID > second->dirID)
			return(1);
		
		else if (first->dirID < second->dirID)
			return(-1);
		
		
		
		if (first->parentDirID > second->parentDirID)
			return(1);
		
		else if (first->parentDirID < second->parentDirID)
			return(-1);
		
		
		if (first->attrib > second->attrib)
			return(1);
		
		else if (first->attrib < second->attrib)
			return(-1);
		
		
		
		errCode = strcmp(first->finderInfo,second->finderInfo);
		
		if (errCode > 0)
			return(1);
		
		else if (errCode < 0)
			return(-1);
		
		return(0);
	}
	
	return(0);
}



static long FindDir(DirInfo *theDirInfo)
{
register long	i,delta,result;

	i = 0;
	
	
	if (numDirs & 1L)
		delta = (numDirs / 2L) + 1L;
	else
		delta = (numDirs / 2L);
	
	
	while (delta >= 1L)
	{
		result = CompareDirs(&(listOfDirs[i]),theDirInfo);
		
		if (result == 0L)
			return(i);
		
		else if (result < 0L)
			i += delta;
		
		else
			i -= delta;
		
		
		if (delta == 1L)
			delta = 0L;
		
		else if (delta & 1L)
			delta = (delta >> 1L) + 1L;
		else
			delta = (delta >> 1L);
	}
	
	
	for (i = 0;i < numDirs;i++)
	{
		if (theDirInfo->dirID == listOfDirs[i].dirID)
			return(i);
	}
	
	return(-1L);
}









static int ResetVolumeAndDirs(void)
{
int				errCode;
HParamBlockRec	myHPB;
CMovePBRec		myCMovePB;
WDPBRec			myWDPB;
int				j,i;
char			tempString[256];
long			theParentDirID;

	errCode = SetupVolRefNum();
	
	if (errCode != noErr)
		return(errCode);
	
	tempString[0] = 0;
	
	myHPB.volumeParam.ioCompletion = 0L;
	myHPB.volumeParam.ioNamePtr = (StringPtr) &tempString;
	myHPB.volumeParam.ioVRefNum = vRefNum;
	myHPB.volumeParam.ioVolIndex = 0;
	
	errCode = PBHGetVInfo(&myHPB,FALSE);
	
	if (errCode != noErr)
	{
		sprintf(errorString,"ResetVolumeAndDirs:\rPBHGetVInfo() error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	SetInfoMessage("Loading previously saved volume/directory information");
	
	errCode = LoadAndParseDirs();
	
	if (errCode != noErr)
		return(errCode);
	
	PtoCstr(tempString);
	errCode = strcmp(tempString,theVolInfo.name);
	CtoPstr(tempString);
	
	if (errCode != EQSTR)
	{
		PtoCstr(tempString);
		sprintf(errorString,"Renaming volume from '%s' to '%s'",tempString,theVolInfo.name);
		CtoPstr(tempString);
		SetInfoMessage(errorString);
		
		strcpy(errorString,theVolInfo.name);
		CtoPstr(errorString);
		
		myHPB.volumeParam.ioCompletion = 0L;
		myHPB.volumeParam.ioNamePtr = 0L;
		myHPB.volumeParam.ioVRefNum = vRefNum;
		myHPB.ioParam.ioMisc = (Ptr) &errorString;
		myHPB.fileParam.ioDirID = 0;
		
		errCode = PBHRename(&myHPB,FALSE);
		
		if (errCode != noErr)
		{
			PtoCstr(tempString);
			sprintf(errorString,"ResetVolumeAndDirs:\rPBHRename('%s' -> '%s') error %d",tempString,theVolInfo.name,errCode);
			CtoPstr(tempString);
			ErrorAlert(errorString);
		}
		
		else
		{
			strcpy(tempString,theVolInfo.name);
			CtoPstr(tempString);
		}
	}
	
	
	myHPB.volumeParam.ioCompletion = 0L;
	myHPB.volumeParam.ioNamePtr = (StringPtr) &tempString;
	myHPB.volumeParam.ioVAtrb = theVolInfo.attrib;
	
	for (j=0;j<32;j++)
		((unsigned char *) &(myHPB.volumeParam.ioVFndrInfo))[j] = theVolInfo.finderInfo[j];
	
	SetInfoMessage("Resetting volume info");
	
	errCode = PBSetVInfo(&myHPB,FALSE);
	
	if (errCode != noErr)
	{
		sprintf(errorString,"ResetVolumeAndDirs:\rPBSetVInfo() error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	SetInfoMessage("Beginning reset of directory information");
	
	for (i=0;i<numDirs;i++)
	{
		if (InfoBreak())
		{
			DisposeDirList();
			return(-1);
		}
		
		tempString[0] = 0;
		
		myCPB.dirInfo.ioCompletion = 0L;
		myCPB.dirInfo.ioNamePtr = (StringPtr) &tempString;
		myCPB.dirInfo.ioVRefNum = vRefNum;
		myCPB.dirInfo.ioFDirIndex = -1;
		myCPB.dirInfo.ioDrDirID = listOfDirs[i].dirID;
		
		errCode = PBGetCatInfo(&myCPB,FALSE);
		
		if (errCode == fnfErr)
		{
			sprintf(errorString,"ResetVolumeAndDirs:\rPBGetCatInfo('%s') error %d \r\rDirectory has probably been deleted...",listOfDirs[i].name,errCode);
			ErrorAlert(errorString);
		}
		
		else if (errCode != noErr)
		{
			sprintf(errorString,"ResetVolumeAndDirs:\rPBGetCatInfo('%s') error %d",listOfDirs[i].name,errCode);
			ErrorAlert(errorString);
		}
		
		else
		{
			theParentDirID = myCPB.dirInfo.ioDrParID;
			
			PtoCstr(tempString);
			errCode = strcmp(tempString,listOfDirs[i].name);
			CtoPstr(tempString);
			
			if (errCode != EQSTR)
			{
				myWDPB.ioCompletion = 0L;
				myWDPB.ioNamePtr = 0L;
				myWDPB.ioVRefNum = vRefNum;
				myWDPB.ioWDProcID = CREATOR;
				myWDPB.ioWDDirID = myCPB.dirInfo.ioDrParID;
				
				errCode = PBOpenWD(&myWDPB,FALSE);
				
				if (errCode != noErr)
				{
					sprintf(errorString,"ResetVolumeAndDirs:\rPBOpenWD() error %d",errCode);
					ErrorAlert(errorString);
				}
				
				else
				{
					PtoCstr(tempString);
					sprintf(errorString,"Renaming directory from '%s' to '%s'",tempString,listOfDirs[i].name);
					CtoPstr(tempString);
					SetInfoMessage(errorString);
					
					strcpy(errorString,listOfDirs[i].name);
					CtoPstr(errorString);
					
					myHPB.fileParam.ioCompletion = 0L;
					myHPB.fileParam.ioNamePtr = (StringPtr) tempString;
					myHPB.fileParam.ioVRefNum = myWDPB.ioVRefNum;
					myHPB.ioParam.ioMisc = (Ptr) &errorString;
					myHPB.fileParam.ioDirID = 0L;
					
					errCode = PBHRename(&myHPB,FALSE);
					
					if (errCode != noErr)
					{
						PtoCstr(tempString);
						sprintf(errorString,"ResetVolumeAndDirs:\rPBHRename('%s' -> '%s') error %d",tempString,listOfDirs[i].name,errCode);
						CtoPstr(tempString);
						ErrorAlert(errorString);
					}
					
					else
					{
						strcpy(tempString,listOfDirs[i].name);
						CtoPstr(tempString);
					}
					
					errCode = PBCloseWD(&myWDPB,FALSE);
				}
			}
			
			myCPB.dirInfo.ioCompletion = 0L;
			myCPB.dirInfo.ioNamePtr = 0L;
			myCPB.dirInfo.ioVRefNum = vRefNum;
			myCPB.dirInfo.ioFlAttrib = listOfDirs[i].attrib;
			myCPB.dirInfo.ioDrDirID = listOfDirs[i].dirID;
			
			for (j=0;j<16;j++)
				((char *) &(myCPB.dirInfo.ioDrUsrWds))[j] = listOfDirs[i].finderInfo[j];
			
			for (j=0;j<16;j++)
				((char *) &(myCPB.dirInfo.ioDrFndrInfo))[j] = listOfDirs[i].finderInfo[j + 16];
			
			errCode = PBSetCatInfo(&myCPB,FALSE);
			
			if (errCode != noErr)
			{
				PtoCstr(tempString);
				sprintf(errorString,"ResetVolumeAndDirs:\rPBSetCatInfo(%s) error %d",tempString,errCode);
				CtoPstr(tempString);
				ErrorAlert(errorString);
			}
			
			if (theParentDirID != listOfDirs[i].parentDirID)
			{
				PtoCstr(tempString);
				sprintf(errorString,"Moving '%s'",tempString);
				CtoPstr(tempString);
				SetInfoMessage(errorString);
				
				myCMovePB.ioCompletion = 0L;
				myCMovePB.ioNamePtr = 0L;
				myCMovePB.ioVRefNum = vRefNum;
				myCMovePB.ioNewName = 0L;
				myCMovePB.ioNewDirID = listOfDirs[i].parentDirID;
				myCMovePB.ioDirID = listOfDirs[i].dirID;
				
				errCode = PBCatMove(&myCMovePB,FALSE);
				
				if (errCode != noErr)
				{
					PtoCstr(tempString);
					sprintf(errorString,"ResetVolumeAndDirs:\rPBCatMove(%s) error %d",tempString,errCode);
					CtoPstr(tempString);
					ErrorAlert(errorString);
				}
			}
		}
	}
	
	DisposeDirList();
}




static int ResetFilesAndCollectGarbage(void)
{
int		errCode;
char	tempString[256];


	errCode = SetupVolRefNum();
	
	if (errCode != noErr)
		return(errCode);
	
	errCode = LoadAndParseDirs();
	
	if (errCode != noErr)
		return(errCode);
	
	errCode = LoadAndParseFiles();
	
	if (errCode != noErr)
		return(errCode);
	
	strcpy(tempString,":");
	strcat(tempString,STUDENTDIRECTORY);
	strcat(tempString,":");
	CtoPstr(tempString);
	
	myCPB.dirInfo.ioCompletion = 0L;
	myCPB.dirInfo.ioNamePtr = (StringPtr) &tempString;
	myCPB.dirInfo.ioVRefNum = vRefNum;
	myCPB.dirInfo.ioFDirIndex = 0;
	myCPB.dirInfo.ioDrDirID = 0L;
	
	errCode = PBGetCatInfo(&myCPB,FALSE);
	
	if (errCode!=noErr && errCode!=fnfErr)
	{
		sprintf(errorString,"ResetFilesAndCollectGarbage:\rPBGetCatInfo error %d",errCode);
		ErrorAlert(errorString);
		return(errCode);
	}
	
	studentWorkDirID = myCPB.dirInfo.ioDrDirID;
	
	strcpy(currentDirName,theVolInfo.name);
	
	ProcessDirectory(0L);
	
	FlushVol("\p",vRefNum);
	
}




static int  ProcessDirectory(long theIODirID)
{
int				index,errCode,j;
char			tempString[256];
long			match;
CMovePBRec		myCMovePB;
WDPBRec			myWDPB;
HParamBlockRec	myHPB;


	if (InfoBreak())
		return(-1);
	
	if (currentDirName[0])
	{
		sprintf(errorString,"Checking %s",currentDirName);
		SetInfoMessage(errorString);
	}
	
	index = 1;
		
	do
	{
		tempString[0] = 0;
		
		myCPB.hFileInfo.ioCompletion = 0L;
		myCPB.hFileInfo.ioNamePtr = (StringPtr) &tempString;
		myCPB.hFileInfo.ioVRefNum = vRefNum;
		myCPB.hFileInfo.ioFDirIndex = index++;
		myCPB.hFileInfo.ioDirID = theIODirID;
		
		errCode = PBGetCatInfo(&myCPB,FALSE);
		
		if (errCode!=noErr && errCode!=fnfErr)
		{
			sprintf(errorString,"ProcessDirectory:\rPBGetCatInfo error %d",errCode);
			ErrorAlert(errorString);
			return(errCode);
		}
		
		else if (errCode == noErr)
		{
			PtoCstr(tempString);
			
			if (myCPB.hFileInfo.ioFlAttrib & 0x10)
			{
				strcpy(theDirInfo.name,tempString);
				theDirInfo.dirID = myCPB.dirInfo.ioDrDirID;
				theDirInfo.parentDirID = myCPB.dirInfo.ioDrParID;
				theDirInfo.attrib = myCPB.dirInfo.ioFlAttrib;
				
				for (j=0;j<16;j++)
					theDirInfo.finderInfo[j] = ((char *) &(myCPB.dirInfo.ioDrUsrWds))[j];
				
				for (j=0;j<16;j++)
					theDirInfo.finderInfo[j + 16] = ((char *) &(myCPB.dirInfo.ioDrFndrInfo))[j];
				
				match = FindDir(&theDirInfo);
				
				if (myCPB.dirInfo.ioDrParID != studentWorkDirID && (match < 0L || CompareDirs(&(listOfDirs[match]),&theDirInfo) != 0))
				{
					sprintf(errorString,"Moving directory '%s' to %s directory",tempString,STUDENTDIRECTORY);
					SetInfoMessage(errorString);
					
					CtoPstr(tempString);
					
					myCMovePB.ioCompletion = 0L;
					myCMovePB.ioNamePtr = 0L;
					myCMovePB.ioVRefNum = vRefNum;
					myCMovePB.ioNewName = 0L;
					myCMovePB.ioNewDirID = studentWorkDirID;
					myCMovePB.ioDirID = myCPB.dirInfo.ioDrDirID;
					
					errCode = PBCatMove(&myCMovePB,FALSE);
					
					if (errCode != noErr)
					{
						PtoCstr(tempString);
						sprintf(errorString,"ProcessDirectory:\rPBCatMove(%s) error %d",tempString,errCode);
						CtoPstr(tempString);
						ErrorAlert(errorString);
						errCode = noErr;
					}
					
					else
						index--;
				}
			}
			
			else
			{
				theFileInfo.type = myCPB.hFileInfo.ioFlFndrInfo.fdType;
				theFileInfo.creator = myCPB.hFileInfo.ioFlFndrInfo.fdCreator;
				theFileInfo.creationDate = myCPB.hFileInfo.ioFlCrDat;
				theFileInfo.parentDirID = myCPB.hFileInfo.ioFlParID;
				theFileInfo.attrib = myCPB.hFileInfo.ioFlAttrib;
				strcpy(theFileInfo.name,tempString);
				
				for (j=0;j<16;j++)
					theFileInfo.finderInfo[j] = ((char *) &(myCPB.hFileInfo.ioFlFndrInfo))[j];
				
				for (j=0;j<16;j++)
					theFileInfo.finderInfo[j + 16] = ((char *) &(myCPB.hFileInfo.ioFlXFndrInfo))[j];
				
				match = FindFile(&theFileInfo);
				
				if (match < 0L && myCPB.hFileInfo.ioFlParID != studentWorkDirID)
				{
					sprintf(errorString,"Moving file '%s' to %s directory",tempString,STUDENTDIRECTORY);
					SetInfoMessage(errorString);
					
					CtoPstr(tempString);
					
					myCMovePB.ioCompletion = 0L;
					myCMovePB.ioNamePtr = (StringPtr) &tempString;
					myCMovePB.ioVRefNum = vRefNum;
					myCMovePB.ioNewName = 0L;
					myCMovePB.ioNewDirID = studentWorkDirID;
					myCMovePB.ioDirID = myCPB.hFileInfo.ioFlParID;
					
					errCode = PBCatMove(&myCMovePB,FALSE);
					
					if (errCode != noErr)
					{
						PtoCstr(tempString);
						sprintf(errorString,"ProcessDirectory:\rPBCatMove(%s) error %d",tempString,errCode);
						CtoPstr(tempString);
						ErrorAlert(errorString);
						errCode = noErr;
					}
					
					else
						index--;
				}
				
				else
				{
					CtoPstr(tempString);
					
					myCPB.hFileInfo.ioCompletion = 0L;
					myCPB.hFileInfo.ioNamePtr = (StringPtr) &tempString;
					myCPB.hFileInfo.ioVRefNum = vRefNum;
					myCPB.hFileInfo.ioFlAttrib = listOfFiles[match].attrib;
					myCPB.hFileInfo.ioDirID = myCPB.hFileInfo.ioFlParID;
					
					for (j=0;j<16;j++)
						((char *) &(myCPB.hFileInfo.ioFlFndrInfo))[j] = listOfFiles[match].finderInfo[j];
					
					for (j=0;j<16;j++)
						((char *) &(myCPB.hFileInfo.ioFlXFndrInfo))[j] = listOfFiles[match].finderInfo[j + 16];
					
					errCode = PBSetCatInfo(&myCPB,FALSE);
					
					PtoCstr(tempString);
					
					if (errCode != noErr)
					{
						sprintf(errorString,"ProcessDirectory:\rPBSetCatInfo(%s) error %d",tempString,errCode);
						ErrorAlert(errorString);
						errCode = noErr;
					}
					
					
					if (strcmp(listOfFiles[match].name,theFileInfo.name) != EQSTR)
					{
						myWDPB.ioCompletion = 0L;
						myWDPB.ioNamePtr = 0L;
						myWDPB.ioVRefNum = vRefNum;
						myWDPB.ioWDProcID = CREATOR;
						myWDPB.ioWDDirID = myCPB.hFileInfo.ioFlParID;
						
						errCode = PBOpenWD(&myWDPB,FALSE);
						
						if (errCode != noErr)
						{
							sprintf(errorString,"ProcessDirectory:\rPBOpenWD() error %d",errCode);
							ErrorAlert(errorString);
							errCode = noErr;
						}
						
						else
						{
							sprintf(errorString,"Renaming file from '%s' to '%s'",tempString,listOfFiles[match].name);
							SetInfoMessage(errorString);
							
							strcpy(errorString,listOfFiles[match].name);
							CtoPstr(errorString);
							CtoPstr(tempString);
							
							myHPB.fileParam.ioCompletion = 0L;
							myHPB.fileParam.ioNamePtr = (StringPtr) tempString;
							myHPB.fileParam.ioVRefNum = myWDPB.ioVRefNum;
							myHPB.ioParam.ioMisc = (Ptr) &errorString;
							myHPB.fileParam.ioDirID = 0L;
							
							errCode = PBHRename(&myHPB,FALSE);
							
							PtoCstr(tempString);
								
							if (errCode != noErr)
							{
								sprintf(errorString,"ProcessDirectory:\rPBHRename('%s' -> '%s') error %d",tempString,listOfFiles[match].name,errCode);
								ErrorAlert(errorString);
								errCode = noErr;
							}
							
							else
								strcpy(tempString,listOfFiles[match].name);
							
							PBCloseWD(&myWDPB,FALSE);
						}
					}
				
					if (listOfFiles[match].parentDirID != myCPB.hFileInfo.ioFlParID)
					{
					 	sprintf(errorString,"Moving file '%s'",tempString);
						CtoPstr(tempString);
						SetInfoMessage(errorString);
						
						myCMovePB.ioCompletion = 0L;
						myCMovePB.ioNamePtr = (StringPtr) &tempString;
						myCMovePB.ioVRefNum = vRefNum;
						myCMovePB.ioNewName = 0L;
						myCMovePB.ioNewDirID = listOfFiles[match].parentDirID;
						myCMovePB.ioDirID = myCPB.hFileInfo.ioFlParID;
						
						errCode = PBCatMove(&myCMovePB,FALSE);
						
						PtoCstr(tempString);
						
						if (errCode != noErr)
						{
							sprintf(errorString,"ProcessDirectory:\rPBCatMove(%s) error %d",tempString,errCode);
							ErrorAlert(errorString);
							errCode = noErr;
						}
						
						else
							index--;
					}
				}
			}
		}
		
	} while (errCode==noErr);
	
	if (errCode == fnfErr)
		errCode = noErr;
	
	
	
	if (errCode == noErr)
	{
		index = 1;
			
		do
		{
			tempString[0] = 0;
			
			myCPB.hFileInfo.ioCompletion = 0L;
			myCPB.hFileInfo.ioNamePtr = (StringPtr) &tempString;
			myCPB.hFileInfo.ioVRefNum = vRefNum;
			myCPB.hFileInfo.ioFDirIndex = index++;
			myCPB.hFileInfo.ioDirID = theIODirID;
			
			errCode = PBGetCatInfo(&myCPB,FALSE);
			
			if (errCode!=noErr && errCode!=fnfErr)
			{
				sprintf(errorString,"ProcessDirectory:\rPBGetCatInfo error %d",errCode);
				ErrorAlert(errorString);
				return(errCode);
			}
			
			else if (errCode == noErr && myCPB.dirInfo.ioFlAttrib & 0x10 && myCPB.dirInfo.ioDrDirID != studentWorkDirID)
			{
				PtoCstr(tempString);
				strcpy(errorString,currentDirName);
				strcat(currentDirName,":");
				strcat(currentDirName,tempString);
				strcpy(tempString,errorString);
				
				errCode = ProcessDirectory(myCPB.dirInfo.ioDrDirID);
				
				strcpy(currentDirName,tempString);
			}
			
		} while (errCode==noErr);
		
		if (errCode == fnfErr)
			errCode = noErr;
	}	
	
	FlushVol("\p",vRefNum);
	
	return(errCode);
}
