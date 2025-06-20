#include "pref.h"
#include <string.h>
#include "folders.h"	
#include "ftp.h"
	
#define kCreatorType 'dFTP'
#define kFileType 'pref'
#define kUserStrings 1000
#define kHostString 1
#define kUserString 2
#define kPswdString 3

extern char host[256];
extern char user[256];
extern char pswd[256];

//--------------------------------------------------------------------------
	short OpenPref(short *refNum,short perm)
//--------------------------------------------------------------------------
{
	SysEnvRec theWorld;
	FSSpec myFSSpec;
	short oe,foundVRefNum;
	long foundDirID;
	
	oe = SysEnvirons(1,&theWorld);
	
	oe = FindFolder(theWorld.sysVRefNum,kPreferencesFolderType,
					false,&foundVRefNum,&foundDirID);
	oe = FSMakeFSSpec(foundVRefNum,foundDirID,"\pDropFTP Pref",&myFSSpec);
	if (oe == fnfErr && perm != fsRdPerm ) {
		oe = FSpCreate(&myFSSpec,kCreatorType,kFileType,smSystemScript);
	} 
	
	if (oe == noErr) {
		oe = FSpOpenDF(&myFSSpec,perm,refNum);
		oe = SetFPos(*refNum,fsFromStart,0);
	} else {
		 if (perm != fsRdPerm)
		 	ErrorMessage("Trouble saving pref file");
	}
	return oe;
	
}

//--------------------------------------------------------------------------
	void ReadPref(void)
//--------------------------------------------------------------------------
{
	short len,refNum,oe;
	long count;
	
	if (OpenPref(&refNum,fsRdPerm) == noErr){
		count = 2;	//read number of strings ignore for now
		oe = FSRead(refNum,&count,&len);
		
		count = 2;	//read in str len
		oe = FSRead(refNum,&count,&len);
		count = len;
		oe = FSRead(refNum,&count,host);
		host[len] = 0;
		
		count = 2;	//read in str len
		oe = FSRead(refNum,&count,&len);
		count = len;
		oe = FSRead(refNum,&count,user);
		user[len] = 0;
		
		count = 2;	//read in str len
		oe = FSRead(refNum,&count,&len);
			count = len;
		oe = FSRead(refNum,&count,pswd);
		pswd[len] = 0;
		ScramblePW (pswd,len);		// unscramble
		oe = FSClose(refNum);
	} else {
		Str255 temp;
	
		GetIndString(temp,kUserStrings,kHostString);
		strcpy(host,p2cstr((StringPtr)temp));
		GetIndString(temp,kUserStrings,kUserString);
		strcpy(user,p2cstr((StringPtr)temp));
		GetIndString(temp,kUserStrings,kPswdString);
		strcpy(pswd,p2cstr((StringPtr)temp));
	}
}

//--------------------------------------------------------------------------
	void WritePref(void)
//--------------------------------------------------------------------------
{
	short len,refNum,oe;
	long count;
	
	if (OpenPref(&refNum,fsRdWrPerm) ==noErr){
		count = 2;	//read number of strings ignore for now
		len = 3;
		
		oe = FSWrite(refNum,&count,&len);
		
		count = 2;	
		len = strlen(host);
		oe = FSWrite(refNum,&count,&len);
		count = len;
		oe = FSWrite(refNum,&count,host);

		
		count = 2;	
		len = strlen(user);
		oe = FSWrite(refNum,&count,&len);
		count = len;
		oe = FSWrite(refNum,&count,user);
	
	
		count = 2;	
		len = strlen(user);
		ScramblePW (pswd,len);		// scramble
		oe = FSWrite(refNum,&count,&len);
		count = len;
		oe = FSWrite(refNum,&count,pswd);
		ScramblePW (pswd,len);		// unscramble
		oe = FSClose(refNum);
	}
}
/*----------------------------------------------------------------------------
	ScramblePW
	
	Scrambles (and unscrambles) saved passwords. This is not really secure,
	just something to foil people browsing using disk editors.
	
	Entry:	pw = the password.
			len = length of password.
			
	Exit:	Each byte nibble-swapped and bit-flipped.
----------------------------------------------------------------------------*/

static void ScramblePW (char *pw, short len)
{
	char *p, *pEnd;
	
	pEnd = pw + len;
	for (p = pw; p < pEnd; p++) *p = (((*p >> 4) & 0x0f) | ((*p & 0x0f) << 4)) ^ 0xff;
}
