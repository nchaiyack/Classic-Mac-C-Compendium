#include <Traps.h>
#include <stdio.h>
#include <string.h>


#include "PBGetInfo.h"



main()
{
char 		*moof,installed;
Handle 		myCodeRes;
Handle		lockStatusHandle;
long		theSize;
SysEnvRec	theWorld;
CInfoPBRec	myPBRec;
char		volName[256];
char		tempString[256];
int			errCode;
int			vRefNum;
long		freeBytes;
long		oldTrapAddress;

	installed = FALSE;
	
	errCode = GetVInfo(0,volName,&vRefNum,&freeBytes);
	
	if (errCode != noErr)
		SysBeep(1);
	
	else
	{
		errCode = SysEnvirons(1,&theWorld);
		
		if (errCode != noErr)
		{
			SysBeep(1);
			SysBeep(1);
		}
		
		else
		{
			volName[0] = 0;
			
			myPBRec.dirInfo.ioCompletion = 0L;
			myPBRec.dirInfo.ioNamePtr = (StringPtr) volName;
			myPBRec.dirInfo.ioVRefNum = theWorld.sysVRefNum;
			myPBRec.dirInfo.ioFDirIndex = -1;
			myPBRec.dirInfo.ioDrDirID = 0L;
			
			errCode = PBGetCatInfo(&myPBRec,FALSE);
			
			if (errCode != noErr)
			{
				SysBeep(1);
				SysBeep(1);
				SysBeep(1);
			}
			
			else
			{
				myCodeRes = GetNamedResource('RoyW',patchName);
			
				if(myCodeRes == nil)
				{
					SysBeep(1);
					SysBeep(1);
					SysBeep(1);
					SysBeep(1);
				}
				
				else
				{
					theSize = GetHandleSize(myCodeRes);
					for(moof=*myCodeRes;theSize>0 && strcmp(moof,"Moof")!=EQSTR;moof++,theSize--);
					
					if (theSize <= 0L || strcmp(moof,"Moof")!=EQSTR)
					{
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
					}
					
					else
					{
						oldTrapAddress = NGetTrapAddress((int) (_HFSDispatch - 0xA000),OSTrap);
						
						moof += 4L;
						*((long *) moof) = NGetTrapAddress((int) (_HFSDispatch - 0xA000),OSTrap);
						moof += 4L;
						*((int *) moof) = theWorld.sysVRefNum;
						moof += 2L;
						*((int *) moof) = vRefNum;
						moof += 2L;
						*((long *) moof) = myPBRec.dirInfo.ioDrDirID;
						moof += 4L;
						*((long *) moof) = myPBRec.dirInfo.ioDrParID;
						
						HLock(myCodeRes);
						
						NSetTrapAddress((long) StripAddress(*myCodeRes),(_HFSDispatch - 0xA000),OSTrap);
						
						
						strcpy(tempString,LOCKMESSAGE);
						
						myPBRec.dirInfo.ioCompletion = 0L;
						myPBRec.dirInfo.ioNamePtr = (StringPtr) LOCKMESSAGE;
						myPBRec.dirInfo.ioVRefNum = 0;
						myPBRec.dirInfo.ioFDirIndex = 0;
						myPBRec.dirInfo.ioDrDirID = 0L;
						
						errCode = PBGetCatInfo(&myPBRec,FALSE);
						
						if (errCode == noErr)
							installed = TRUE;
						
						doDirStuff();
						
						NSetTrapAddress(oldTrapAddress,(_HFSDispatch - 0xA000),OSTrap);
						ReleaseResource(myCodeRes);
					}
				}
			}
		}
	}
}




doDirStuff()
{
int				vRefNum;
SFReply			mySFReply;
Point			topLeftPt;
SFTypeList		fileTypes;
union	{
		char	name[5];
		long	num;
		} fType;

	do
	{
		PrintDir();
		
		topLeftPt.h = 45;
		topLeftPt.v = 45;
		fileTypes[0] = 'APPL';
		
		SFGetFile(topLeftPt,"\p",0L,-1,&fileTypes,0L,&mySFReply);
		
		if (mySFReply.good)
		{
			PtoCstr((char *) mySFReply.fName);
			printf("File selected: %s\n",mySFReply.fName);
			
			fType.num = mySFReply.fType;
			fType.name[4] = 0;
			printf("File type: %s\n",fType.name);
		}
		else
			printf("SFGetFile returns error\n");
		
		More();
		
		
		
	} while (mySFReply.good);
}




More()
{
char	ch;

	printf("\n<more>");
	ch = getchar();
}




PrintDir()
{
int					errCode,index;
char				vName[256];
HParamBlockRec		myPB;

	printf("\n");
	printf("Directory Contents:\n");
	
	index = 1;
	
	do
	{
		vName[0] = 0;
		
		myPB.fileParam.ioCompletion = 0L;
		myPB.fileParam.ioNamePtr = (StringPtr) &vName;
		myPB.fileParam.ioVRefNum = -SFSaveDisk;
		myPB.fileParam.ioFVersNum = 0;
		myPB.fileParam.ioFDirIndex = index++;
		myPB.fileParam.ioDirID = CurDirStore;
		
		errCode = PBHGetFInfo(&myPB,FALSE);
		
		if (errCode==noErr)
		{
			PtoCstr(vName);
			printf("%s\n",vName);
		}
		
	} while (errCode==noErr);
	
	if (errCode != fnfErr)
		printf("*** PBGetFInfo error #%d ***\n",errCode);
	
	More();
}




SetPath(pathName)
StringPtr	pathName;
{
int				errCode;
WDPBRec			myWDPB;

	printf("\n");
	
	PrintCurrentDir();
	
	printf("Changing default volume to '%s'\n",pathName);
	CtoPstr((char *) pathName);
	
	myWDPB.ioCompletion = 0L;
	myWDPB.ioNamePtr = pathName;
	myWDPB.ioVRefNum = -SFSaveDisk;
	myWDPB.ioWDDirID = 0L;
	
	errCode = PBHSetVol(&myWDPB,FALSE);
	
	if (errCode!=noErr)
		printf("*** PBHSetVol Error #%d ***\n",errCode);
	
	else
	{
		myWDPB.ioCompletion = 0L;
		errCode = PBHGetVol(&myWDPB,FALSE);
		
		if (errCode!=noErr)
			printf("*** PBHGetVol Error #%d ***\n",errCode);
		else
		{
			CurDirStore = myWDPB.ioWDDirID;
			SFSaveDisk = -myWDPB.ioVRefNum;
			
			PrintCurrentDir();
		}
	}
		
	
	More();
	
	return(errCode);
}




PrintCurrentDir()
{
WDPBRec		myWDPB;
CInfoPBRec	myCPB;
int			vRefNum;
int			errCode;
char		tempString[256],pathName[256];
long		theDir;

	pathName[0] = 0;
	
	theDir = CurDirStore;
	
	do
	{
		tempString[0] = 0;
		
		myCPB.dirInfo.ioCompletion = 0L;
		myCPB.dirInfo.ioNamePtr = (StringPtr) &tempString;
		myCPB.dirInfo.ioVRefNum = -SFSaveDisk;
		myCPB.dirInfo.ioFDirIndex = -1;
		myCPB.dirInfo.ioDrDirID = theDir;
		
		errCode = PBGetCatInfo(&myCPB,FALSE);
		
		if (errCode!=noErr)
			printf("*** PBGetCatInfo Error #%d ***\n",errCode);
		
		else
		{
			PtoCstr(tempString);
			
			theDir = myCPB.dirInfo.ioDrParID;
			
			strcat(tempString,":");
			strcat(tempString,pathName);
			strcpy(pathName,tempString);
		}
		
	} while (myCPB.dirInfo.ioDrParID>=2 && errCode==noErr);
		
	if (errCode == noErr)
		printf("Full pathname: '%s'\n",pathName);
}