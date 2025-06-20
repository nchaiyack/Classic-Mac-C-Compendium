#include "rnmac.h"
#include <Aliases.h>

#define customGetDLOG	140

#define	SEQF			"SEQUENCEFILE"
#define CREATORSETTING	"TEXTCREATOR"
#define TYPESETTING		"TEXTTYPE"


static char		*separatorString = "\r\r----\r\r";




static int createFolder(char *theSTRName,char *didCreate);



static void GetCreatorType(long *theCreator,long *theType)
{
int		errCode,i;
char	tempString[256];


	*theCreator = CREATOR;
	*theType = 'TEXT';

	if ((errCode = LoadSTRResource(CREATORSETTING,tempString)) == noErr)
	{
		*theCreator = *((long *) tempString);
	}
	
	if ((errCode = LoadSTRResource(TYPESETTING,tempString)) == noErr)
	{
		*theType = *((long *) tempString);
	}
}


int MyOpenResFile(char *resFileName,int *theFRefNum)
{
FSSpec		theSpec;
char		targetIsFolder,wasAliased;
int			errCode;

	*theFRefNum = -1;
	
	errCode = noErr;
	
	if (!hasSys7Aliases)
	{
		*theFRefNum = OpenResFile(StripAddress(resFileName));
		
		if (*theFRefNum == -1)
		{
			errCode = ResError();
			
			PtoCstr(resFileName);
			sprintf(errorMessage,"MyOpenResFile: OpenResFile(%s) error %d",resFileName,errCode);
			CtoPstr(resFileName);
		}
	}
	
	else
	{
		if ((errCode = FSMakeFSSpec(vRefNum,0,resFileName,&theSpec)) != noErr)
		{
			PtoCstr(resFileName);
			sprintf(errorMessage,"MyOpenResFile: FSMakeFSSpec(%s) error %d",resFileName,errCode);
			CtoPstr(resFileName);
			goto EXITPOINT;
		}
		
		
		if ((errCode = ResolveAliasFile(&theSpec,TRUE,(Boolean *) &targetIsFolder,(Boolean *) &wasAliased)) != noErr)
		{
			PtoCstr(resFileName);
			sprintf(errorMessage,"MyOpenResFile: ResolveAliasFile(%s) error %d",resFileName,errCode);
			CtoPstr(resFileName);
			goto EXITPOINT;
		}
		
		if (targetIsFolder)
		{
			errCode = -1;
			PtoCstr(resFileName);
			sprintf(errorMessage,"MyOpenResFile: attempt to open aliased file '%s' which resolves to a folder, not a file",resFileName);
			CtoPstr(resFileName);
			goto EXITPOINT;
		}
		
		*theFRefNum = FSpOpenResFile(&theSpec,0);
		
		if (*theFRefNum == -1)
		{
			errCode = ResError();
			
			PtoCstr(resFileName);
			sprintf(errorMessage,"MyOpenResFile: FSpOpenResFile(%s) error %d",resFileName,errCode);
			CtoPstr(resFileName);
		}
	}

EXITPOINT:
	
	return(errCode);
}






int MyOpenResFile67SF(sys67SFReply *thesys67SFReply,int *theFRefNum)
{
int			errCode,currentVol;
char		tempString[256];

	*theFRefNum = -1;
	
	errCode = noErr;
	
	if (!hasSys7Aliases)
	{
		errCode = GetVol(tempString,&currentVol);
		
		if (errCode != noErr)
		{
			sprintf(errorMessage,"MyOpenResFile67SF: GetVol() error %d",errCode);
			goto EXITPOINT;
		}
		
		errCode = SetVol(0L,thesys67SFReply->sys6SFReply.vRefNum);
		
		if (errCode != noErr)
		{
			sprintf(errorMessage,"MyOpenResFile67SF: SetVol(%d) error %d",(int) (thesys67SFReply->sys6SFReply.vRefNum),errCode);
			
			goto EXITPOINT;
		}
		
		
		*theFRefNum = OpenResFile(StripAddress(thesys67SFReply->sys6SFReply.fName));
		
		
		if (*theFRefNum == -1)
		{
			errCode = ResError();
			
			PtoCstr(thesys67SFReply->sys6SFReply.fName);
			sprintf(errorMessage,"MyOpenResFile67SF: OpenResFile(%s) error %d",thesys67SFReply->sys6SFReply.fName,errCode);
			CtoPstr(thesys67SFReply->sys6SFReply.fName);
		}
		
		SetVol(0L,currentVol);
	}
	
	else
	{
		*theFRefNum = FSpOpenResFile(&(thesys67SFReply->sys7SFReply.sfFile),0);
		
		if (*theFRefNum == -1)
		{
			errCode = ResError();
			
			PtoCstr(thesys67SFReply->sys7SFReply.sfFile.name);
			sprintf(errorMessage,"MyOpenResFile67SF: FSpOpenResFile(%s) error %d",thesys67SFReply->sys7SFReply.sfFile.name,errCode);
			CtoPstr(thesys67SFReply->sys7SFReply.sfFile.name);
		}
	}

EXITPOINT:
	
	return(errCode);
}




int BuildPathName(char *fName,int theVRefNum,long dirID,char *thePath)
{
CInfoPBRec	theCPB;
char		tempString[256];
int			errCode;
long		currentDirID;

	strcpy(thePath,fName);
	
	tempString[0] = 0;
	currentDirID = dirID;
	
	do
	{
		theCPB.dirInfo.ioCompletion = 0L;
		theCPB.dirInfo.ioNamePtr = (StringPtr) tempString;
		theCPB.dirInfo.ioVRefNum = theVRefNum;
		theCPB.dirInfo.ioFDirIndex = -1;
		theCPB.dirInfo.ioDrDirID = currentDirID;
		
		if ((errCode = PBGetCatInfo(&theCPB,FALSE)) != noErr)
		{
			sprintf(errorMessage,"BuildPathName: PBGetCatInfo(%s) error %d",fName,errCode);
			thePath[0] = 0;
			return(errCode);
		}
		
		PtoCstr(tempString);
		mystrncat(tempString,":",255);
		mystrncat(tempString,thePath,255);
		mystrncpy(thePath,tempString,255);
		
		currentDirID = theCPB.dirInfo.ioDrParID;
	
	} while (currentDirID > 1);
	
	return(noErr);
}




int ResolveAndBuildAliasPathName(char *fName,int theVRefNum,char *thePath)
{
FSSpec		theSpec;
int			errCode;
char		targetIsFolder,wasAliased;
char		cFilename[256];

	errCode = noErr;
	thePath[0] = 0;
	
	PtoCstr(fName);
	mystrncpy(cFilename,fName,255);
	CtoPstr(fName);
	
	if (!hasSys7Aliases)
	{
		errCode = BuildPathName(fName,theVRefNum,0L,thePath);
	}
	
	else
	{
		if ((errCode = FSMakeFSSpec(theVRefNum,0,fName,&theSpec)) != noErr)
		{
			sprintf(errorMessage,"ResolveAndBuildAliasPathName: FSMakeFSSpec(%s) error %d",cFilename,errCode);
		}
		
		else
		{
			errCode = ResolveAliasFile(&theSpec,TRUE,(Boolean *) &targetIsFolder,(Boolean *) &wasAliased);
			
			if (errCode != noErr && errCode != fnfErr)
			{
				sprintf(errorMessage,"ResolveAndBuildAliasPathName: ResolveAliasFile(%s) error %d",cFilename,errCode);
			}
			
			else
			{
				PtoCstr((char *) theSpec.name);
				
				errCode = BuildPathName((char *) theSpec.name,theSpec.vRefNum,theSpec.parID,thePath);
			}
		}
	}
	
	return(errCode);
}





int MyFSOpen(char *fName,int vRefNum,int *fRefNum)
{
FSSpec		theSpec;
char		targetIsFolder,wasAliased;
int			errCode;

	errCode = noErr;
	
	if (!hasSys7Aliases)
	{
		errCode = FSOpen(fName,vRefNum,fRefNum);
		
		if (errCode != noErr)
		{
			PtoCstr(fName);
			sprintf(errorMessage,"MyFSOpen: FSOpen(%s) error %d",fName,errCode);
			CtoPstr(fName);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if ((errCode = FSMakeFSSpec(vRefNum,0,fName,&theSpec)) != noErr)
		{
			PtoCstr(fName);
			sprintf(errorMessage,"MyFSOpen: FSMakeFSSpec(%s) error %d",fName,errCode);
			CtoPstr(fName);
			goto EXITPOINT;
		}
		
		if ((errCode = ResolveAliasFile(&theSpec,TRUE,(Boolean *) &targetIsFolder,(Boolean *) &wasAliased)) != noErr)
		{
			PtoCstr(fName);
			sprintf(errorMessage,"MyFSOpen: ResolveAliasFile(%s) error %d",fName,errCode);
			CtoPstr(fName);
			goto EXITPOINT;
		}
		
		if (targetIsFolder)
		{
			PtoCstr(fName);
			sprintf(errorMessage,"MyFSOpen: ResolveAliasFile(%s) resolves to a *folder*, not a file!",fName);
			CtoPstr(fName);
			goto EXITPOINT;
		}
		
		if ((errCode = FSpOpenDF(&theSpec,0,fRefNum)) != noErr)
		{
			PtoCstr(fName);
			sprintf(errorMessage,"MyFSOpen: FSpOpenDF(%s) error %d",fName,errCode);
			CtoPstr(fName);
			goto EXITPOINT;
		}
	}
	
EXITPOINT:

	return(errCode);
}





int LoadSTRResource(char *STRName,char *theString)
{
Handle		resHandle;
char		resName[256],tempString[256];
int			errCode;

	errCode = noErr;

	mystrncpy(resName,STRName,255);
	CtoPstr(resName);
	
	UseResFile(settingsFRefNum);
	
	resHandle = GetNamedResource('STR ',resName);
	
	errCode = ResError();
	
	if (resHandle==0L || errCode != noErr)
	{
		if (errCode == noErr)
			errCode = RESERROR;
		
		sprintf(errorMessage,"LoadSTRResource: GetNamedResource('%s') error %d, can't get resource from 'rnMac SETTINGS' file",STRName,errCode);
		
		goto EXITPOINT;
	}
	
	HLock(resHandle);
	PtoCstr((char *) *resHandle);
	mystrncpy(theString,(char *) *resHandle,255);
	CtoPstr((char *) *resHandle);
	HUnlock(resHandle);
	ReleaseResource(resHandle);

EXITPOINT:
	
	UseResFile(rnFRefNum);
	
	return(errCode);
}




char STRResourceIsPresent(char *STRName)
{
Handle		resHandle;
char		resName[256];
int			errCode;

	errCode = noErr;
	
	mystrncpy(resName,STRName,255);
	CtoPstr(resName);
	
	UseResFile(settingsFRefNum);
	
	resHandle = GetNamedResource('STR ',resName);
	
	if (resHandle == 0L || ResError() != noErr)
	{
		UseResFile(rnFRefNum);
		return(FALSE);
	}
	
	else
	{
		ReleaseResource(resHandle);
		UseResFile(rnFRefNum);
		return(TRUE);
	}
}




int PutSTR(char *nameOfSTR,char *theSTR)
{
Handle		theSTRHandle;
int			errCode;
char		tempString[256];

	errCode = noErr;
	theSTRHandle = 0L;
	
	UseResFile(settingsFRefNum);
	
	mystrncpy(tempString,nameOfSTR,255);
	CtoPstr(tempString);
	
	theSTRHandle = GetNamedResource('STR ',tempString);
	
	if (!theSTRHandle)
	{
		theSTRHandle = NewHandle(256L);
		
		errCode = MemError();
		
		if (errCode != noErr || theSTRHandle == 0L)
		{
			if (errCode == noErr)
				errCode = memFullErr;
			
			sprintf(errorMessage,"PutSTR: NewHandle(256L) error %d",errCode);
			
			goto EXITPOINT;
		}
		
		AddResource(theSTRHandle,'STR ',UniqueID('STR '),tempString);
		
		errCode = ResError();
		
		if (errCode != noErr)
		{
			DisposeHandle(theSTRHandle);
			theSTRHandle = 0L;
			
			sprintf(errorMessage,"PutSTR: AddResource(%s) error %d",nameOfSTR,errCode);
			
			goto EXITPOINT;
		}
	}
	
	
	SetHandleSize(theSTRHandle,(long) strlen(theSTR) + 1L);
	
	errCode = MemError();
	
	if (errCode != noErr || GetHandleSize(theSTRHandle) < strlen(theSTR) + 1L)
	{
		if (errCode == noErr)
			errCode = memFullErr;
		
		sprintf(errorMessage,"PutSTR: SetHandleSize(%ld) error %d",(long) (strlen(theSTR) + 1L),errCode);
		
		goto EXITPOINT;
	}
	
	HLock(theSTRHandle);
	mystrncpy((char *) *theSTRHandle,theSTR,255);
	CtoPstr((char *) *theSTRHandle);
	HUnlock(theSTRHandle);
	
	ChangedResource(theSTRHandle);
	WriteResource(theSTRHandle);
	
	
EXITPOINT:

	if (theSTRHandle)
		ReleaseResource(theSTRHandle);
	
	UseResFile(rnFRefNum);
	
	return(errCode);
}




int GetAndIncrementSEQF(char *theSeqStr)
{
char	seqf[256];
int		fileRefNum,i,seqnum,errCode;
long	byteCount;
char	tempString[256];

	errCode = noErr;
	fileRefNum = 0;
	
	if ((errCode = LoadSTRResource(CONFDIR,seqf)) != noErr)
		goto EXITPOINT;
	
	if (strlen(seqf) <= 0 || seqf[strlen(seqf)-1] != ':')
		mystrncat(seqf,":",255);
	
	if ((errCode = LoadSTRResource(SEQF,tempString)) != noErr)
		goto EXITPOINT;
	
	mystrncat(seqf,tempString,255);
	
	CtoPstr(seqf);
	
	if ((errCode = MyFSOpen(seqf,vRefNum,&fileRefNum)) != noErr)
		goto EXITPOINT;
	
	if ((errCode = SetFPos(fileRefNum,fsFromStart,0L)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: SetFPos('%s',fsFromStart) error %d",seqf,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = GetEOF(fileRefNum,&byteCount)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: GetEOF('%s') error %d",seqf,errCode);
		goto EXITPOINT;
	}
	
	if (byteCount > 10)
		byteCount = 10;
	
	if ((errCode = FSRead(fileRefNum,&byteCount,tempString)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: FSRead('%s') error %d",seqf,errCode);
		goto EXITPOINT;
	}
	
	i = 0;
	
	while (tempString[i] >= '0' && tempString[i] <= '9' && i < 255)
		i++;
		
	tempString[i] = 0;
	
	if (i == 0)
		mystrncpy(tempString,"1",255);
	
	seqnum = 1;
	sscanf(tempString,"%d",&seqnum);
	sprintf(tempString,"%d\r",(seqnum<32767) ? seqnum+1 : 1);
	
	byteCount = strlen(tempString);
	
	if ((errCode = SetFPos(fileRefNum,fsFromStart,0L)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: SetFPos('%s',fsFromStart) error %d",seqf,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetEOF(fileRefNum,0L)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: SetEOF('%s') error %d",seqf,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSWrite(fileRefNum,&byteCount,tempString)) != noErr)
	{
		PtoCstr(seqf);
		sprintf(errorMessage,"GetAndIncrementSEQF: FSWrite('%s',%ld) error %d",seqf,byteCount,errCode);
		goto EXITPOINT;
	}
	
	
	if (seqnum < 10)
		sprintf(theSeqStr,"0000%d",seqnum);
	else if (seqnum < 100)
		sprintf(theSeqStr,"000%d",seqnum);
	else if (seqnum < 1000)
		sprintf(theSeqStr,"00%d",seqnum);
	else if (seqnum < 1000)
		sprintf(theSeqStr,"0%d",seqnum);
	else
		sprintf(theSeqStr,"%d",seqnum);


EXITPOINT:

	if (fileRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fileRefNum)) != noErr)
			{
				PtoCstr(seqf);
				sprintf(errorMessage,"GetAndIncrementSEQF: FSClose(%s) error %d",seqf,errCode);
			}
		}
		
		else
			FSClose(fileRefNum);
	}
	
	FlushVol("\p",vRefNum);
	
	return(errCode);
}





int OpenAndZeroOrCreate(char *theFileName,int *theFileRefNum,int theVRefNum)
{
int		errCode;
char	localFileName[256];
char	tempString[256];

	errCode = 0;
	*theFileRefNum = 0;
	
	mystrncpy(localFileName,theFileName,255);
	CtoPstr(localFileName);
	
	errCode = MyFSOpen(localFileName,theVRefNum,theFileRefNum);
	
	if (errCode == fnfErr)
	{
		if ((errCode = Create(localFileName,theVRefNum,CREATOR,'TEXT')) != noErr)
		{
			sprintf(errorMessage,"OpenAndZeroOrCreate: Create('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
		
		errCode = MyFSOpen(localFileName,theVRefNum,theFileRefNum);
	}
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"OpenAndZeroOrCreate: MyFSOpen('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(*theFileRefNum,fsFromStart,0L)) != noErr)
	{
		sprintf(errorMessage,"OpenAndZeroOrCreate: SetFPos('%s',fsFromStart) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetEOF(*theFileRefNum,0L)) != noErr)
	{
		sprintf(errorMessage,"OpenAndZeroOrCreate: SetEOF('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}

EXITPOINT:

	if (errCode != noErr && *theFileRefNum)
	{
		FSClose(*theFileRefNum);
	}
	
	return(errCode);
}




int AppendTextFileAsSys67SF(Handle theTEH,sys67SFReply *thesys67SFReply)
{
SFTypeList			mySFTypes;
Point				theTopLeft;
int					errCode,fRefNum,theVRefNum;
long				byteCount;
char				filename[256];

	errCode = noErr;
	fRefNum = 0;
	
	if (hasSys7SFGetPut)
	{
		if (!(thesys67SFReply->sys7SFReply.sfGood))
		{
			mySFTypes[0] = 'TEXT';
			
			theTopLeft.h = -1;
			theTopLeft.v = -1;
		
			CustomGetFile(0L,1,&mySFTypes,thesys67SFReply,customGetDLOG,theTopLeft,0L,0L,0L,0L,0L);
		}
		
		PtoCstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
		mystrncpy(filename,(char *) thesys67SFReply->sys7SFReply.sfFile.name,255);
		CtoPstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
		
		theVRefNum = thesys67SFReply->sys7SFReply.sfFile.vRefNum;
		
		if (!(thesys67SFReply->sys7SFReply.sfGood))
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		else if ((errCode = FSpOpenDF(&(thesys67SFReply->sys7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAsSys67SF: FSpOpenDF('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if (!(thesys67SFReply->sys6SFReply.good))
		{
			theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
			theTopLeft.v = screenBits.bounds.top + 50;
			
			mySFTypes[0] = 'TEXT';
			
			SFGetFile(theTopLeft,"\pSelect Text File To Append To:",0L,1,&mySFTypes,0L,thesys67SFReply);
		}
		
		PtoCstr((char *) (char *) thesys67SFReply->sys6SFReply.fName);
		mystrncpy(filename,(char *) thesys67SFReply->sys6SFReply.fName,255);
		CtoPstr((char *) (char *) thesys67SFReply->sys6SFReply.fName);
		
		theVRefNum = thesys67SFReply->sys6SFReply.vRefNum;
		
		if (!(thesys67SFReply->sys6SFReply.good))
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		else if ((errCode = MyFSOpen((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fRefNum)) != noErr)
			goto EXITPOINT;
	}
	
	
	if ((errCode = SetFPos(fRefNum,fsFromLEOF,0L)) != noErr)
	{
		sprintf(errorMessage,"AppendTextFileAsSys67SF: SetFPos('%s',0) error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	byteCount = strlen(separatorString);
	
	if ((errCode = FSWrite(fRefNum,&byteCount,separatorString)) != noErr)
	{
		sprintf(errorMessage,"AppendTextFileAsSys67SF: FSWrite('%s') error %d",filename,errCode);
		goto EXITPOINT;
	}

	
	if (useTextEdit)
	{
		byteCount = (**((TEHandle) theTEH)).teLength;
		
		HLock((**((TEHandle) theTEH)).hText);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,*((**((TEHandle) theTEH)).hText))) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAsSys67SF: FSWrite('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		byteCount = (**((TE32KHandle) theTEH)).teLength;
		
		HLock((**((TE32KHandle) theTEH)).hText);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,*((**((TE32KHandle) theTEH)).hText))) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAsSys67SF: FSWrite('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
	}
	
	

EXITPOINT:
	
	if (useTextEdit)
		HUnlock((**((TEHandle) theTEH)).hText);
	else
		HUnlock((**((TE32KHandle) theTEH)).hText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"AppendTextFileAsSys67SF: FSClose('%s',0) error %d",filename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	FlushVol("\p",theVRefNum);
	
	return(errCode);
}





int AppendTextFile(Handle theTEH)
{
SFReply				mySFReply;
SFTypeList			mySFTypes;
StandardFileReply	my7SFReply;
Point				theTopLeft;
int					errCode,fRefNum,theVRefNum;
long				byteCount;
char				fileName[256];

	errCode = noErr;
	fRefNum = 0;
	
	if (hasSys7SFGetPut)
	{
		mySFTypes[0] = 'TEXT';
		
		theTopLeft.h = -1;
		theTopLeft.v = -1;
		
		CustomGetFile(0L,1,&mySFTypes,&my7SFReply,customGetDLOG,theTopLeft,0L,0L,0L,0L,0L);
		
		PtoCstr((char *) my7SFReply.sfFile.name);
		mystrncpy(fileName,(char *) my7SFReply.sfFile.name,255);
		CtoPstr((char *) my7SFReply.sfFile.name);
		
		theVRefNum = my7SFReply.sfFile.vRefNum;
		
		if (!my7SFReply.sfGood)
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		else if ((errCode = FSpOpenDF(&(my7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"AppendTextFile: FSpOpenDF('%s') error %d",fileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		mySFTypes[0] = 'TEXT';
		
		SFGetFile(theTopLeft,"\pSelect Text File To Append To:",0L,1,&mySFTypes,0L,&mySFReply);
		
		PtoCstr((char *) (char *) mySFReply.fName);
		mystrncpy(fileName,(char *) mySFReply.fName,255);
		CtoPstr((char *) (char *) mySFReply.fName);
		
		theVRefNum = mySFReply.vRefNum;
		
		if (!mySFReply.good)
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		else if ((errCode = MyFSOpen((char *) mySFReply.fName,mySFReply.vRefNum,&fRefNum)) != noErr)
			goto EXITPOINT;
	}
	
	
	if ((errCode = SetFPos(fRefNum,fsFromLEOF,0L)) != noErr)
	{
		sprintf(errorMessage,"AppendTextFile: SetFPos('%s',0) error %d",fileName,errCode);
		goto EXITPOINT;
	}
	
	byteCount = strlen(separatorString);
	
	if ((errCode = FSWrite(fRefNum,&byteCount,separatorString)) != noErr)
	{
		sprintf(errorMessage,"AppendTextFile: FSWrite('%s') error %d",fileName,errCode);
		goto EXITPOINT;
	}

	
	if (useTextEdit)
	{
		byteCount = (**((TEHandle) theTEH)).teLength;
		
		HLock((**((TEHandle) theTEH)).hText);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,*((**((TEHandle) theTEH)).hText))) != noErr)
		{
			sprintf(errorMessage,"AppendTextFile: FSWrite('%s') error %d",fileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		byteCount = (**((TE32KHandle) theTEH)).teLength;
		
		HLock((**((TE32KHandle) theTEH)).hText);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,*((**((TE32KHandle) theTEH)).hText))) != noErr)
		{
			sprintf(errorMessage,"AppendTextFile: FSWrite('%s') error %d",fileName,errCode);
			goto EXITPOINT;
		}
	}


EXITPOINT:

	if (useTextEdit)
		HUnlock((**((TEHandle) theTEH)).hText);
	else
		HUnlock((**((TE32KHandle) theTEH)).hText);
	
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"AppendTextFile: FSClose('%s',0) error %d",fileName,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	FlushVol("\p",theVRefNum);
	
	return(errCode);
}



int AppendTextFileAs(Handle theTEH,char *filename)
{
int			errCode,fRefNum,choice;
long		byteCount,theCreator,theType;
Handle		theText;
char		pFileName[256];

	errCode = noErr;
	fRefNum = 0;
	theText = 0L;
	
	GetCreatorType(&theCreator,&theType);
	
	if (theTEH)
	{
		mystrncpy(pFileName,filename,255);
		
		CtoPstr(pFileName);
		
		errCode = MyFSOpen(pFileName,vRefNum,&fRefNum);
		
		if (errCode == fnfErr)
		{
			if ((errCode = Create(pFileName,vRefNum,theCreator,theType)) != noErr)
			{
				sprintf(errorMessage,"AppendTextFileAs: Create('%s') error %d",filename,errCode);
				goto EXITPOINT;
			}
			
			errCode = MyFSOpen(pFileName,vRefNum,&fRefNum);
		}
			
		if (errCode != noErr)
			goto EXITPOINT;
			
		if ((errCode = SetFPos(fRefNum,fsFromLEOF,0L)) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAs: SetFPos('%s',0) error %d",filename,errCode);
			goto EXITPOINT;
		}
		
		if (useTextEdit)
		{
			theText = (**((TEHandle) theTEH)).hText;
			byteCount = (**((TEHandle) theTEH)).teLength;
		}
		else
		{
			theText = (**((TE32KHandle) theTEH)).hText;
			byteCount = (**((TE32KHandle) theTEH)).teLength;
		}
		
		HLock(theText);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,*theText)) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAs: FSWrite('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
		
		HUnlock(theText);
		
		byteCount = strlen(separatorString);
		
		if ((errCode = FSWrite(fRefNum,&byteCount,separatorString)) != noErr)
		{
			sprintf(errorMessage,"AppendTextFileAs: FSWrite('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
	}
	
EXITPOINT:

	if (theText)
		HUnlock(theText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"AppendTextFileAs: FSClose(%s) error %d",filename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	FlushVol("\p",vRefNum);
	
	return(errCode);
}





int SaveTextFile(Handle theTEH,char *fileName)
{
SFReply				mySFReply;
StandardFileReply	my7SFReply;
Point				theTopLeft;
int					errCode,fRefNum,theVRefNum;
long				byteCount,theType,theCreator;
Handle				theText;
char				localFilename[256];

	errCode = noErr;
	fRefNum = 0;
	theText = 0L;
	
	GetCreatorType(&theCreator,&theType);
	
	
	if (hasSys7SFGetPut)
	{
		mystrncpy(localFilename,fileName,255);
		CtoPstr(localFilename);
		
		StandardPutFile("\pSave Text File As:",localFilename,&my7SFReply);
		
		PtoCstr((char *) my7SFReply.sfFile.name);
		mystrncpy(localFilename,(char *) my7SFReply.sfFile.name,255);
		CtoPstr((char *) my7SFReply.sfFile.name);
		
		theVRefNum = my7SFReply.sfFile.vRefNum;
		
		if (!my7SFReply.sfGood)
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		if ((errCode = FSpOpenDF(&(my7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			if (errCode==fnfErr)
			{
				if ((errCode = FSpCreate(&(my7SFReply.sfFile),theCreator,theType,my7SFReply.sfScript)) != noErr)
				{
					sprintf(errorMessage,"SaveTextFile: FSpCreate('%s') error %d",localFilename,errCode);
					goto EXITPOINT;
				}
				
				errCode = FSpOpenDF(&(my7SFReply.sfFile),0,&fRefNum);
			}
		}
		
		if (errCode != noErr)
		{
			sprintf(errorMessage,"SaveTextFile: FSpOpenDF('%s') error %d",localFilename,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		mystrncpy(localFilename,fileName,255);
		CtoPstr(localFilename);
		
		SFPutFile(theTopLeft,"\pSave Text File As:",localFilename,0L,&mySFReply);
		
		PtoCstr((char *) (char *) mySFReply.fName);
		mystrncpy(localFilename,(char *) mySFReply.fName,255);
		CtoPstr((char *) (char *) mySFReply.fName);
		
		theVRefNum = mySFReply.vRefNum;
		
		if (!mySFReply.good)
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		if ((errCode = MyFSOpen((char *) mySFReply.fName,mySFReply.vRefNum,&fRefNum)) != noErr)
		{
			if (errCode == fnfErr)
			{
				if ((errCode = Create(mySFReply.fName,mySFReply.vRefNum,theCreator,theType)) != noErr)
				{
					sprintf(errorMessage,"SaveTextFile: Create('%s') error %d",localFilename,errCode);
					goto EXITPOINT;
				}
				
				errCode = MyFSOpen((char *) mySFReply.fName,mySFReply.vRefNum,&fRefNum);
			}
		}
		
		if (errCode != noErr)
		{
			goto EXITPOINT;
		}
	}
	
	
	if ((errCode = SetEOF(fRefNum,0L)) != noErr)
	{
		sprintf(errorMessage,"SaveTextFile: SetEOF('%s',0) error %d",localFilename,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(fRefNum,fsFromStart,0L)) != noErr)
	{
		sprintf(errorMessage,"SaveTextFile: SetFPos('%s',fsFromStart,0) error %d",localFilename,errCode);
		goto EXITPOINT;
	}
	
	if (useTextEdit)
	{
		theText = (**((TEHandle) theTEH)).hText;
		byteCount = (**((TEHandle) theTEH)).teLength;
	}
	else
	{
		theText = (**((TE32KHandle) theTEH)).hText;
		byteCount = (**((TE32KHandle) theTEH)).teLength;
	}
	
	HLock(theText);
	
	if ((errCode = FSWrite(fRefNum,&byteCount,*theText)) != noErr)
	{
		sprintf(errorMessage,"SaveTextFile: FSWrite('%s') error %d",localFilename,errCode);
		goto EXITPOINT;
	}
	

EXITPOINT:

	if (theText)
		HUnlock(theText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"SaveTextFile: FSClose(%s) error %d",localFilename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	FlushVol("\p",theVRefNum);
	
	return(errCode);
}





int SaveExistingTextFile(Handle theTEH,char *defaultFilename,sys67SFReply *thesys67SFReply)
{
Point		theTopLeft;
int			errCode,fRefNum,theVRefNum;
long		byteCount,theCreator,theType;
Handle		theText;
char		filename[256];

	errCode = noErr;
	fRefNum = 0;
	theText = 0L;
	
	GetCreatorType(&theCreator,&theType);
	
	if (hasSys7SFGetPut)
	{
		if (!(thesys67SFReply->sys7SFReply.sfGood))
		{
			mystrncpy(filename,defaultFilename,255);
			CtoPstr(filename);
			
			StandardPutFile("\pSave Text File As:",filename,thesys67SFReply);
		}
		
		if (!(thesys67SFReply->sys7SFReply.sfGood))
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		PtoCstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
		mystrncpy(filename,(char *) thesys67SFReply->sys7SFReply.sfFile.name,255);
		CtoPstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
		
		theVRefNum = thesys67SFReply->sys7SFReply.sfFile.vRefNum;
		
		if ((errCode = FSpOpenDF(&(thesys67SFReply->sys7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			if (errCode==fnfErr)
			{
				if ((errCode = FSpCreate(&(thesys67SFReply->sys7SFReply.sfFile),theCreator,theType,thesys67SFReply->sys7SFReply.sfScript)) != noErr)
				{
					sprintf(errorMessage,"SaveExistingTextFile: FSpCreate('%s') error %d",filename,errCode);
					goto EXITPOINT;
				}
				
				errCode = FSpOpenDF(&(thesys67SFReply->sys7SFReply.sfFile),0,&fRefNum);
			}
		}
		
		if (errCode != noErr)
		{
			goto EXITPOINT;
		}
	}
	
	else
	{
		if (!(thesys67SFReply->sys6SFReply.good))
		{
			theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
			theTopLeft.v = screenBits.bounds.top + 50;
			
			mystrncpy(filename,defaultFilename,255);
			CtoPstr(filename);
			
			SFPutFile(theTopLeft,"\pSave Text File As:",filename,0L,thesys67SFReply);
		}
		
		if (!(thesys67SFReply->sys6SFReply.good))
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
		
		PtoCstr((char *) (char *) thesys67SFReply->sys6SFReply.fName);
		mystrncpy(filename,(char *) thesys67SFReply->sys6SFReply.fName,255);
		CtoPstr((char *) (char *) thesys67SFReply->sys6SFReply.fName);
		
		theVRefNum = thesys67SFReply->sys6SFReply.vRefNum;
		
		if ((errCode = MyFSOpen((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fRefNum)) != noErr)
		{
			if (errCode == fnfErr)
			{
				if ((errCode = Create(thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,theCreator,theType)) != noErr)
				{
					sprintf(errorMessage,"SaveExistingTextFile: Create('%s') error %d",filename,errCode);
					goto EXITPOINT;
				}
				
				errCode = MyFSOpen((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fRefNum);
			}
		}
		
		if (errCode != noErr)
		{
			goto EXITPOINT;
		}
	}
	
	if ((errCode = SetEOF(fRefNum,0L)) != noErr)
	{
		sprintf(errorMessage,"SaveExistingTextFile: SetEOF('%s',0) error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(fRefNum,fsFromStart,0L)) != noErr)
	{
		sprintf(errorMessage,"SaveExistingTextFile: SetFPos('%s',fsFromStart,0) error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	if (useTextEdit)
	{
		theText = (**((TEHandle) theTEH)).hText;
		byteCount = (**((TEHandle) theTEH)).teLength;
	}
	
	else
	{
		theText = (**((TE32KHandle) theTEH)).hText;
		byteCount = (**((TE32KHandle) theTEH)).teLength;
	}
	
	HLock(theText);
	
	if ((errCode = FSWrite(fRefNum,&byteCount,*theText)) != noErr)
	{
		sprintf(errorMessage,"SaveExistingTextFile: FSWrite('%s') error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	
EXITPOINT:

	if (theText)
		HUnlock(theText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"SaveExistingTextFile: FSClose(%s) error %d",filename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	
	FlushVol("\p",theVRefNum);
	
	return(errCode);
}




int InsertTextFile(Handle theTEH)
{
SFReply				mySFReply;
StandardFileReply	my7SFReply;
SFTypeList			mySFTypes;
Point				theTopLeft;
int					errCode,fRefNum,theVRefNum;
long				byteCount;
Ptr					theText;
char				filename[256];

	errCode = noErr;
	theText = 0L;
	fRefNum = 0;
	
	if (hasSys7SFGetPut)
	{
		mySFTypes[0] = 'TEXT';
		
		StandardGetFile(0L,1,&mySFTypes,&my7SFReply);
		
		PtoCstr((char *) my7SFReply.sfFile.name);
		mystrncpy(filename,(char *) my7SFReply.sfFile.name,255);
		CtoPstr((char *) my7SFReply.sfFile.name);
		
		theVRefNum = my7SFReply.sfFile.vRefNum;
		
		if (my7SFReply.sfGood)
		{
			if ((errCode = FSpOpenDF(&(my7SFReply.sfFile),0,&fRefNum)) != noErr)
			{
				sprintf(errorMessage,"InsertTextFile: FSpOpenDF('%s') error %d",filename,errCode);
				goto EXITPOINT;
			}
		}
		
		else
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
	}
	
	else
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		mySFTypes[0] = 'TEXT';
		
		SFGetFile(theTopLeft,"\pSelect Text File To Insert:",0L,1,&mySFTypes,0L,&mySFReply);
		
		PtoCstr((char *) (char *) mySFReply.fName);
		mystrncpy(filename,(char *) mySFReply.fName,255);
		CtoPstr((char *) (char *) mySFReply.fName);
		
		theVRefNum = mySFReply.vRefNum;
		
		if (mySFReply.good)
		{
			if ((errCode = MyFSOpen((char *) mySFReply.fName,mySFReply.vRefNum,&fRefNum)) != noErr)
				goto EXITPOINT;
		}
		
		else
		{
			errCode = USERCANCEL;
			goto EXITPOINT;
		}
	}
		
	if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
	{
		sprintf(errorMessage,"InsertTextFile: GetEOF('%s') error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(fRefNum,fsFromStart,0L)) != noErr)
	{
		sprintf(errorMessage,"InsertTextFile: SetFPos('%s',fsFromStart,0) error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	theText = NewPtr(byteCount);
	
	errCode = MemError();
	
	if (theText == 0L || errCode != noErr)
	{
		if (errCode == noErr)
			errCode = memFullErr;
		
		sprintf(errorMessage,"InsertTextFile: NewPtr(%ld) error %d",byteCount,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSRead(fRefNum,&byteCount,theText)) != noErr)
	{
		sprintf(errorMessage,"InsertTextFile: FSRead('%s') error %d",filename,errCode);
		goto EXITPOINT;
	}
	
	if (useTextEdit)
		TEInsert(theText,byteCount,(TEHandle) theTEH);
	
	else
		TE32KInsert(theText,byteCount,(TE32KHandle) theTEH);
	
	
EXITPOINT:

	if (theText)
		DisposePtr(theText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"InsertTextFile: FSClose(%s) error %d",filename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}
	
	
	return(errCode);
}





int InsertNamedTextFile(char *filename,Handle theTEH)
{
int			errCode,fRefNum;
long		byteCount;
Ptr			theText;
char		tempString[256],pFileName[256];

	errCode = noErr;
	fRefNum = 0;
	theText = 0L;
	
	if (theTEH)
	{
		mystrncpy(pFileName,filename,255);
		CtoPstr(pFileName);
		
		if ((errCode = MyFSOpen(pFileName,vRefNum,&fRefNum)) != noErr)
			goto EXITPOINT;
			
		
		if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
		{
			sprintf(errorMessage,"InsertNamedTextFile: GetEOF('%s',0) error %d",filename,errCode);
			goto EXITPOINT;
		}
		
		if ((errCode = SetFPos(fRefNum,fsFromStart,0L)) != noErr)
		{
			sprintf(errorMessage,"InsertNamedTextFile: SetFPos('%s',0) error %d",filename,errCode);
			goto EXITPOINT;
		}
		
		theText = NewPtr(byteCount);
		
		errCode = MemError();
		
		if (errCode != noErr || theText==0L)
		{
			if (errCode == noErr)
				errCode = memFullErr;
			
			sprintf(errorMessage,"InsertNamedTextFile: NewPtr(%ld) error %d",byteCount,errCode);
			goto EXITPOINT;
		}
		
		if ((errCode = FSRead(fRefNum,&byteCount,theText)) != noErr)
		{
			sprintf(errorMessage,"InsertNamedTextFile: FSRead('%s') error %d",filename,errCode);
			goto EXITPOINT;
		}
		
		if (useTextEdit)
			TEInsert(theText,byteCount,(TEHandle) theTEH);
		else
			TE32KInsert(theText,byteCount,(TE32KHandle) theTEH);
	}
		
EXITPOINT:
	
	if (theText)
		DisposPtr(theText);
	
	if (fRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fRefNum)) != noErr)
			{
				sprintf(errorMessage,"InsertNamedTextFile: FSClose(%s) error %d",filename,errCode);
			}
		}
		
		else
			FSClose(fRefNum);
	}

	return(errCode);
}






int GetWDDirID(int wdRefNum,long *dirID)
{
int			length,errCode;
WDPBRec		myWDPB;
char		tempString[256];

	errCode = noErr;
	*dirID = 0L;
	
	myWDPB.ioCompletion = 0L;
	myWDPB.ioNamePtr = 0L;
	myWDPB.ioVRefNum = wdRefNum;
	myWDPB.ioWDIndex = 0;
	myWDPB.ioWDProcID = 0L;
	
	if ((errCode = PBGetWDInfo(&myWDPB,FALSE)) != noErr)
	{
		sprintf(errorMessage,"GetWDDirID: PBGetWDInfo() error %d",errCode);
		goto EXITPOINT;
	}
	
	*dirID = myWDPB.ioWDDirID;

EXITPOINT:

	return(errCode);
}




int OpenWorkingDirectory(long dirID,int theVRefNum,int *wdRefNum)
{
int			length,errCode;
WDPBRec		myWDPB;

	errCode = noErr;
	*wdRefNum = 0;
	
	myWDPB.ioCompletion = 0L;
	myWDPB.ioNamePtr = 0L;
	myWDPB.ioVRefNum = theVRefNum;
	myWDPB.ioWDProcID = 0L;
	myWDPB.ioWDDirID = dirID;
	
	errCode = PBOpenWD(&myWDPB,FALSE);
	
	if (errCode == noErr)
	{
		*wdRefNum = myWDPB.ioVRefNum;
	}
	
	return(errCode);
}





int CloseWorkingDirectory(int wdRefNum)
{
int			errCode;
WDPBRec		myWDPB;
char		tempString[256];

	myWDPB.ioCompletion = 0L;
	myWDPB.ioVRefNum = wdRefNum;
	
	errCode = PBCloseWD(&myWDPB,FALSE);
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"CloseWorkingDirectory: PBCloseWD error %d",errCode);
	}
	
	return(errCode);
}




int GetFileDirID(char *theFilePathAndName,int vRefNum,long *dirID)
{
int				length,errCode;
CInfoPBRec		theCPB;
char			tempString[256];

	errCode = noErr;
	*dirID = 0L;
	
	mystrncpy(tempString,theFilePathAndName,255);
	CtoPstr(tempString);
	
	theCPB.hFileInfo.ioCompletion = 0L;
	theCPB.hFileInfo.ioNamePtr = (StringPtr) tempString;
	theCPB.hFileInfo.ioVRefNum = vRefNum;
	theCPB.hFileInfo.ioFDirIndex = 0;
	theCPB.hFileInfo.ioDirID = 0L;
	
	errCode = PBGetCatInfo(&theCPB,FALSE);
	
	if (errCode != noErr)
	{
		sprintf(errorMessage,"GetFileDirID: PBGetCatInfo() error %d",errCode);
		goto EXITPOINT;
	}
	
	*dirID = theCPB.hFileInfo.ioFlParID;
	
EXITPOINT:

	return(errCode);
}








static int createFolder(char *theSTRName,char *didCreate)
{
char		tempString[256],theSTR[256];
CInfoPBRec	myCPB;
int			i,errCode;

	errCode = noErr;
	*didCreate = FALSE;
	
	errCode = LoadSTRResource(theSTRName,theSTR);
	
	if (errCode == noErr)
	{
		i = strlen(theSTR);
		if (i > 0 && i < 255 && theSTR[i-1] != ':')
		{
			theSTR[i] = ':';
			theSTR[i+1] = 0;
		}
		
		CtoPstr(theSTR);
		
		myCPB.hFileInfo.ioCompletion = 0L;
		myCPB.hFileInfo.ioNamePtr = (StringPtr) &theSTR;
		myCPB.hFileInfo.ioVRefNum = vRefNum;
		myCPB.hFileInfo.ioFDirIndex = 0L;
		myCPB.hFileInfo.ioDirID = 0L;
		
		errCode = PBGetCatInfo(&myCPB,FALSE);
		
		if (errCode != noErr)
		{
			myCPB.hFileInfo.ioCompletion = 0L;
			myCPB.hFileInfo.ioNamePtr = (StringPtr) &theSTR;
			myCPB.hFileInfo.ioVRefNum = vRefNum;
			myCPB.hFileInfo.ioFDirIndex = 0L;
			myCPB.hFileInfo.ioDirID = 0L;
			
			errCode = PBDirCreate(&myCPB,FALSE);
			
			if (errCode != noErr)
			{
				PtoCstr(theSTR);
				sprintf(errorMessage,"createFolder: PBDirCreate('%s') error %d",theSTR,errCode);
				goto EXITPOINT;
			}
			
			*didCreate = TRUE;
		}
		
		else if (!(myCPB.hFileInfo.ioFlAttrib & 0x10))
		{
			errCode = FILEWASFOLDER;
			PtoCstr(theSTR);
			PtoCstr(theSTRName);
			sprintf(errorMessage,"createFolder: setting %s = '%s' exists but is not a folder!",theSTRName,theSTR);
			goto EXITPOINT;
		}
	}
	
EXITPOINT:

	return(errCode);
}




int createNeededFolders(void)
{
char	tempString[256],seqf[256],didCreate;
int		fileRefNum,errCode,i;
long	byteCount;

	errCode = noErr;
	fileRefNum = 0;
	
	if ((errCode = createFolder(MAILDIR,&didCreate)) != noErr)
		goto EXITPOINT;
	
	if (STRResourceIsPresent(ARTICLEFOLDER) == TRUE)
	{
		if ((errCode = createFolder(ARTICLEFOLDER,&didCreate)) != noErr)
			goto EXITPOINT;
	}
	
	if ((errCode = createFolder(NEWSFOLDER,&didCreate)) != noErr)
		goto EXITPOINT;
	
	if ((errCode = createFolder(SPOOLDIR,&didCreate)) != noErr)
		goto EXITPOINT;
	
	
	if ((errCode = createFolder(CONFDIR,&didCreate)) != noErr)
		goto EXITPOINT;
	
	else if (didCreate)
	{
		if ((errCode = LoadSTRResource(SEQF,seqf)) != noErr)
			goto EXITPOINT;
		
		if ((errCode = LoadSTRResource(CONFDIR,tempString)) != noErr)
			goto EXITPOINT;
		
		i = strlen(tempString);
		if (i > 0 && i < 255 && tempString[i-1] != ':')
		{
			tempString[i] = ':';
			tempString[i+1] = 0;
		}
		
		mystrncat(tempString,seqf,255);
		mystrncpy(seqf,tempString,255);
				
		if ((errCode = OpenAndZeroOrCreate(seqf,&fileRefNum,vRefNum)) != noErr)
			goto EXITPOINT;
		
		strcpy(tempString,"1");
		byteCount = strlen(tempString);
		
		if ((errCode = FSWrite(fileRefNum,&byteCount,tempString)) != noErr)
		{
			sprintf(errorMessage,"createNeededFolders: FSWrite('%s',%ld) error %d",seqf,byteCount,errCode);
			goto EXITPOINT;
		}
	}
	
EXITPOINT:

	if (fileRefNum)
	{
		if (errCode == noErr)
		{
			if ((errCode = FSClose(fileRefNum)) != noErr)
			{
				sprintf(errorMessage,"createNeededFolders: FSClose(%s) error %d",seqf,errCode);
			}
		}
		
		else
			FSClose(fileRefNum);
	}

	return(errCode);
}





void ExtractSFFileName(sys67SFReply *thesys67SFReply,char *fileName)
{
	if (hasSys7SFGetPut)
	{
		PtoCstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
		mystrncpy(fileName,(char *) thesys67SFReply->sys7SFReply.sfFile.name,63);
		CtoPstr((char *) thesys67SFReply->sys7SFReply.sfFile.name);
	}
	
	else
	{
		PtoCstr((char *) thesys67SFReply->sys6SFReply.fName);
		mystrncpy(fileName,(char *) thesys67SFReply->sys6SFReply.fName,63);
		CtoPstr((char *) thesys67SFReply->sys6SFReply.fName);
	}
}





int SelectInputFile(sys67SFReply *thesys67SFReply,SFTypeList *theSFTypes,int numTypes)
{
Point		theTopLeft;
int			errCode;
	
	errCode = noErr;
	
	
	if (hasSys7SFGetPut)
	{
		StandardGetFile(0L,numTypes,theSFTypes,&(thesys67SFReply->sys7SFReply));
		
		if (!thesys67SFReply->sys7SFReply.sfGood)
			errCode = USERCANCEL;
	}
	
	
	else
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 348)/2;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		if (theTopLeft.h < 50)
			theTopLeft.h = 50;
		
		if (theTopLeft.v < 50)
			theTopLeft.v = 50;
		
		SFGetFile(theTopLeft,"\pSelect File:",0L,numTypes,theSFTypes,0L,&(thesys67SFReply->sys6SFReply));
		
		if (!thesys67SFReply->sys6SFReply.good)
			errCode = USERCANCEL;
	}
	
	return(errCode);
}





int SelectOutputFile(sys67SFReply *thesys67SFReply,char *defaultName)
{
Point		theTopLeft;
int			errCode;
char		fileName[64];

	errCode = noErr;
	
	mystrncpy(fileName,defaultName,63);
	CtoPstr(fileName);
	
	if (hasSys7SFGetPut)
	{
		StandardPutFile("\pSave File As:",fileName,&(thesys67SFReply->sys7SFReply));
		
		if (!thesys67SFReply->sys7SFReply.sfGood)
			errCode = -1;
	}
	
	else
	{
		theTopLeft.h = screenBits.bounds.left + (screenBits.bounds.right - screenBits.bounds.left - 304)/2;;
		theTopLeft.v = screenBits.bounds.top + 50;
		
		if (theTopLeft.h < 50)
			theTopLeft.h = 50;
		
		if (theTopLeft.v < 50)
			theTopLeft.v = 50;
		
		SFPutFile(theTopLeft,"\pSave File As:",fileName,0L,&(thesys67SFReply->sys6SFReply));
		
		if (!thesys67SFReply->sys6SFReply.good)
			errCode = -1;
	}
	
	return(errCode);
}




int GetSFTypeCreatorFlags(sys67SFReply *thesys67SFReply,long *theType,long *theCreator,unsigned int *theFlags)
{
int		errCode;
FInfo	fndrInfo;
char	fileName[64];

	*theType = 0L;
	*theCreator = 0L;
	*theFlags = 0;
	
	ExtractSFFileName(thesys67SFReply,fileName);
	
	if (hasSys7SFGetPut)
	{
		errCode = FSpGetFInfo(&(thesys67SFReply->sys7SFReply.sfFile),&fndrInfo);
		
		if (errCode != noErr)
			sprintf(errorMessage,"GetSFTypeCreatorFlags: FSpGetFInfo('%s') error %d",fileName,errCode);
		
		else
		{
			*theType = fndrInfo.fdType;
			*theCreator = fndrInfo.fdCreator;
			*theFlags = fndrInfo.fdFlags;
		}
	}
	
	else
	{
		errCode = GetFInfo((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fndrInfo);
		
		if (errCode != noErr)
			sprintf(errorMessage,"GetSFTypeCreatorFlags: GetFInfo('%s') error %d",fileName,errCode);
		
		else
		{
			*theType = fndrInfo.fdType;
			*theCreator = fndrInfo.fdCreator;
			*theFlags = fndrInfo.fdFlags;
		}
	}
	
	return(errCode);
}




int SetSFTypeCreatorFlags(sys67SFReply *thesys67SFReply,long theType,long theCreator,unsigned int theFlags)
{
int		errCode;
FInfo	fndrInfo;
char	fileName[64];


	ExtractSFFileName(thesys67SFReply,fileName);
	
	if (hasSys7SFGetPut)
	{
		errCode = FSpGetFInfo(&(thesys67SFReply->sys7SFReply.sfFile),&fndrInfo);
		
		if (errCode != noErr)
			sprintf(errorMessage,"SetSFTypeCreatorFlags: FSpGetFInfo('%s') error %d",fileName,errCode);
		
		else
		{
			fndrInfo.fdType = theType;
			fndrInfo.fdCreator = theCreator;
			fndrInfo.fdFlags = theFlags;
			
			errCode = FSpSetFInfo(&(thesys67SFReply->sys7SFReply.sfFile),&fndrInfo);
			
			if (errCode != noErr)
				sprintf(errorMessage,"SetSFTypeCreatorFlags: FSpSetFInfo('%s') error %d",fileName,errCode);
		}
	}
	
	else
	{
		errCode = GetFInfo((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fndrInfo);
		
		if (errCode != noErr)
			sprintf(errorMessage,"SetSFTypeCreatorFlags: GetFInfo('%s') error %d",fileName,errCode);
		
		else
		{
			fndrInfo.fdType = theType;
			fndrInfo.fdCreator = theCreator;
			fndrInfo.fdFlags = theFlags;
			
			errCode = SetFInfo((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fndrInfo);
			
			if (errCode != noErr)
				sprintf(errorMessage,"SetSFTypeCreatorFlags: SetFInfo('%s') error %d",fileName,errCode);
		}
	}
	
	return(errCode);
}




int GetDataForkLength(sys67SFReply *thesys67SFReply,long *dataForkLength)
{
int			errCode,fRefNum;
long		byteCount;
char		theFileName[64];


	errCode = noErr;
	*dataForkLength = 0L;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	if (hasSys7SFGetPut)
	{
		if ((errCode = FSpOpenDF(&(thesys67SFReply->sys7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"GetDataForkLength: FSpOpenDF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if ((errCode = FSOpen((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"GetDataForkLength: FSOpen('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
	{
		FSClose(fRefNum);
		sprintf(errorMessage,"GetDataForkLength: GetEOF('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSClose(fRefNum)) != noErr)
	{
		sprintf(errorMessage,"GetDataForkLength: FSClose('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	*dataForkLength = byteCount;
	
EXITPOINT:

	return(errCode);
}




int GetResourceForkLength(sys67SFReply *thesys67SFReply,long *resForkLength)
{
int			errCode,fRefNum;
long		byteCount;
char		theFileName[64];


	errCode = noErr;
	*resForkLength = 0L;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	if (hasSys7SFGetPut)
	{
		if ((errCode = FSpOpenRF(&(thesys67SFReply->sys7SFReply.sfFile),0,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"GetResourceForkLength: FSpOpenRF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if ((errCode = OpenRF((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,&fRefNum)) != noErr)
		{
			sprintf(errorMessage,"GetResourceForkLength: OpenRF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
	{
		FSClose(fRefNum);
		sprintf(errorMessage,"GetResourceForkLength: GetEOF('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSClose(fRefNum)) != noErr)
	{
		sprintf(errorMessage,"GetResourceForkLength: FSClose('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	*resForkLength = byteCount;
	
EXITPOINT:

	return(errCode);
}




int OpenDataFork(sys67SFReply *thesys67SFReply,int *fRefNum)
{
int			errCode;
char		theFileName[64];


	errCode = noErr;
	
	*fRefNum = 0;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	if (hasSys7SFGetPut)
	{
		if ((errCode = FSpOpenDF(&(thesys67SFReply->sys7SFReply.sfFile),0,fRefNum)) != noErr)
		{
			sprintf(errorMessage,"OpenDataFork: FSpOpenDF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if ((errCode = FSOpen((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,fRefNum)) != noErr)
		{
			sprintf(errorMessage,"OpenDataFork: FSOpen('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	if ((errCode = SetFPos(*fRefNum,fsFromStart,0L)) != noErr)
	{
		FSClose(*fRefNum);
		*fRefNum = 0;
		sprintf(errorMessage,"OpenDataFork: SetFPos('%s',fsFromStart,0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:

	return(errCode);
}




int OpenResourceFork(sys67SFReply *thesys67SFReply,int *fRefNum)
{
int			errCode;
char		theFileName[64];


	errCode = noErr;
	
	*fRefNum = 0;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	if (hasSys7SFGetPut)
	{
		if ((errCode = FSpOpenRF(&(thesys67SFReply->sys7SFReply.sfFile),0,fRefNum)) != noErr)
		{
			sprintf(errorMessage,"OpenResourceFork: FSpOpenRF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	else
	{
		if ((errCode = OpenRF((char *) thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,fRefNum)) != noErr)
		{
			sprintf(errorMessage,"OpenResourceFork: OpenRF('%s') error %d",theFileName,errCode);
			goto EXITPOINT;
		}
	}
	
	if ((errCode = SetFPos(*fRefNum,fsFromStart,0L)) != noErr)
	{
		FSClose(*fRefNum);
		*fRefNum = 0;
		sprintf(errorMessage,"OpenResourceFork: SetFPos('%s',fsFromStart,0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:

	return(errCode);
}




int OpenAndZeroDataFork(long theType,long theCreator,sys67SFReply *thesys67SFReply,int *theFRefNum)
{
int		errCode;
char	theFileName[64];

	*theFRefNum = 0;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	errCode = OpenDataFork(thesys67SFReply,theFRefNum);
	
	if (errCode == fnfErr)
	{
		if (hasSys7SFGetPut)
		{
			errCode = FSpCreate(&(thesys67SFReply->sys7SFReply.sfFile),theCreator,theType,thesys67SFReply->sys7SFReply.sfScript);
		}
		
		else
		{
			errCode = Create(thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,theCreator,theType);
		}
			
		if (errCode != noErr)
		{
			sprintf(errorMessage,"OpenAndZeroDataFork: FSpCreate('%s') error %d",theFileName,errCode);
			*theFRefNum = 0;
			goto EXITPOINT;
		}
		
		errCode = OpenDataFork(thesys67SFReply,theFRefNum);
	}
	
	if (errCode != noErr)
		goto EXITPOINT;
	
	if ((errCode = SetEOF(*theFRefNum,0L)) != noErr)
	{
		FSClose(*theFRefNum);
		*theFRefNum = 0;
		sprintf(errorMessage,"OpenAndZeroDataFork: SetEOF('%s',0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(*theFRefNum,fsFromStart,0L)) != noErr)
	{
		FSClose(*theFRefNum);
		*theFRefNum = 0;
		sprintf(errorMessage,"OpenAndZeroDataFork: SetFPos('%s',fsFromStart,0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:

	return(errCode);
}




int OpenAndZeroResourceFork(long theType,long theCreator,sys67SFReply *thesys67SFReply,int *theFRefNum)
{
int		errCode;
char	theFileName[64];

	*theFRefNum = 0;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	errCode = OpenResourceFork(thesys67SFReply,theFRefNum);
	
	if (errCode == fnfErr)
	{
		if (hasSys7SFGetPut)
		{
			errCode = FSpCreate(&(thesys67SFReply->sys7SFReply.sfFile),theCreator,theType,thesys67SFReply->sys7SFReply.sfScript);
		}
		
		else
		{
			errCode = Create(thesys67SFReply->sys6SFReply.fName,thesys67SFReply->sys6SFReply.vRefNum,theCreator,theType);
		}
			
		if (errCode != noErr)
		{
			sprintf(errorMessage,"OpenAndZeroDataFork: FSpCreate('%s') error %d",theFileName,errCode);
			*theFRefNum = 0;
			goto EXITPOINT;
		}
		
		errCode = OpenDataFork(thesys67SFReply,theFRefNum);
	}
	
	if (errCode != noErr)
		goto EXITPOINT;
	
	if ((errCode = SetEOF(*theFRefNum,0L)) != noErr)
	{
		FSClose(*theFRefNum);
		*theFRefNum = 0;
		sprintf(errorMessage,"OpenAndZeroResourceFork: SetEOF('%s',0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = SetFPos(*theFRefNum,fsFromStart,0L)) != noErr)
	{
		FSClose(*theFRefNum);
		*theFRefNum = 0;
		sprintf(errorMessage,"OpenAndZeroResourceFork: SetFPos('%s',fsFromStart,0L) error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:

	return(errCode);
}





int LoadDataFork(sys67SFReply *thesys67SFReply,Handle *theDataHandle,long *theDataLength)
{
int			errCode,fRefNum;
long		byteCount;
char		theFileName[64];


	errCode = noErr;
	
	*theDataHandle = 0L;
	*theDataLength = 0L;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	errCode = OpenDataFork(thesys67SFReply,&fRefNum);
	
	if (errCode != noErr)
		goto EXITPOINT;
	
	if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
	{
		FSClose(fRefNum);
		fRefNum = 0;
		sprintf(errorMessage,"LoadDataFork: GetEOF('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	*theDataHandle = NewHandle(byteCount);
	
	errCode = MemError();
	
	if (*theDataHandle == 0L || errCode != noErr)
	{
		*theDataHandle = 0L;
		*theDataLength = 0L;
		FSClose(fRefNum);
		fRefNum = 0;
		
		if (errCode == noErr)
			errCode = memFullErr;
		
		sprintf(errorMessage,"LoadDataFork: NewHandle(%ld) error %d",byteCount,errCode);
		
		goto EXITPOINT;
	}
	
	
	*theDataLength = byteCount;
	
	HLock(*theDataHandle);
	
	if ((errCode = FSRead(fRefNum,theDataLength,**theDataHandle)) != noErr)
	{
		DisposHandle(*theDataHandle);
		*theDataHandle = 0L;
		*theDataLength = 0L;
		FSClose(fRefNum);
		fRefNum = 0;
		sprintf(errorMessage,"LoadDataFork: FSRead('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSClose(fRefNum)) != noErr)
	{
		DisposHandle(*theDataHandle);
		*theDataHandle = 0L;
		*theDataLength = 0L;
		sprintf(errorMessage,"LoadDataFork: FSClose('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:
	
	if (*theDataHandle)
		HUnlock(*theDataHandle);
	
	return(errCode);
}




int LoadResourceFork(sys67SFReply *thesys67SFReply,Handle *theDataHandle,long *theDataLength)
{
int			errCode,fRefNum;
long		byteCount;
char		theFileName[64];


	errCode = noErr;
	
	*theDataHandle = 0L;
	*theDataLength = 0L;
	
	ExtractSFFileName(thesys67SFReply,theFileName);
	
	errCode = OpenResourceFork(thesys67SFReply,&fRefNum);
	
	if (errCode != noErr)
		goto EXITPOINT;
	
	
	if ((errCode = GetEOF(fRefNum,&byteCount)) != noErr)
	{
		FSClose(fRefNum);
		fRefNum = 0;
		sprintf(errorMessage,"LoadResourceFork: GetEOF('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	*theDataHandle = NewHandle(byteCount);
	
	errCode = MemError();
	
	if (*theDataHandle == 0L || errCode != noErr)
	{
		*theDataHandle = 0L;
		*theDataLength = 0L;
		
		FSClose(fRefNum);
		fRefNum = 0;
		
		if (errCode == noErr)
			errCode = memFullErr;
		
		sprintf(errorMessage,"LoadResourceFork: NewHandle(%ld) error %d",byteCount,errCode);
		
		goto EXITPOINT;
	}
	
	*theDataLength = byteCount;
	
	HLock(*theDataHandle);
	
	if ((errCode = FSRead(fRefNum,theDataLength,**theDataHandle)) != noErr)
	{
		DisposHandle(*theDataHandle);
		*theDataHandle = 0L;
		*theDataLength = 0L;
		FSClose(fRefNum);
		fRefNum = 0;
		sprintf(errorMessage,"LoadResourceFork: FSRead('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
	if ((errCode = FSClose(fRefNum)) != noErr)
	{
		DisposHandle(*theDataHandle);
		*theDataHandle = 0L;
		*theDataLength = 0L;
		fRefNum = 0;
		sprintf(errorMessage,"LoadResourceFork: FSClose('%s') error %d",theFileName,errCode);
		goto EXITPOINT;
	}
	
EXITPOINT:
	
	if (*theDataHandle)
		HUnlock(*theDataHandle);
	
	return(errCode);
}




int MakeSys67SFReply(sys67SFReply *thesys67SFReply,char *fName,int vRefNum)
{
int		errCode;
char	tempString[64];

	errCode = noErr;
	
	if (hasSys7Aliases)
	{
		mystrncpy(tempString,fName,63);
		CtoPstr(tempString);
		
		errCode = FSMakeFSSpec(vRefNum,0,tempString,&(thesys67SFReply->sys7SFReply.sfFile));
		
		if (errCode != noErr)
			sprintf(errorMessage,"MakeSys67SFReply: FSMakeFSSpec() error %d",errCode);
	}
	
	else
	{
		mystrncpy((char *) thesys67SFReply->sys6SFReply.fName,fName,63);
		CtoPstr((char *) thesys67SFReply->sys6SFReply.fName);
		
		thesys67SFReply->sys6SFReply.vRefNum = vRefNum;
	}
	
	return(errCode);
}




int Sys67FileExists(sys67SFReply *thesys67SFReply)
{
int				errCode;
long			longDummy;
unsigned int	intDummy;

	errCode = GetSFTypeCreatorFlags(thesys67SFReply,&longDummy,&longDummy,&intDummy);
	
	
EXITPOINT:
	
	return(errCode);
}