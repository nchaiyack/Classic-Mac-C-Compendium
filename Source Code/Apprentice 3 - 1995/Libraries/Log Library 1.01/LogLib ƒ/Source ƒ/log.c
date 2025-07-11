/*
	log.c
	
	Handles Log functions
	
		Code version 1.01
		
		�1994 by Dave Nebinger (dnebing@andy.bgsu.edu)
		All Rights Reserved.
		
		Feel free to use this where ever you wish, just drop me some email
		stating what you are doing with it.
	
	Change Log:  
		5/7/94 dn - Modified a small bug which would nullify folder names, thus
					causing the InitLog routine to always create the log in the
					preferences folder and not in the logFoldName parameter.
		7/3/94 dn - Modified to work with the Universal Headers.
*/


#include "Log.h"

/*
	InitLog
	
	Finds the prefs folder and the designated inside folder.  This should
	only be called once.
	
	
	logFoldName  - 	Name of folder to find in the preferences folder.  If the
					folder does not exist, it will be created.  However, if 
					logFoldName is nil, no folder will be created.
	volRefNum    - 	returns the volume reference number where the prefs folder is
					located.
	prefDirID    - 	returns the directory id for the prefs folder.
	foldDirID    - 	returns the directory id for the newly created folder.  If nil
					was passed in the folder name, foldDirID will be the same 
					as prefDirID.
	
	Returns true if successful, false if there was some kind of error.
*/
Boolean InitLog(Str255 logFoldName,short* volRefNum,long* prefDirID,long* foldDirID){
	OSErr err;
	
	/* standard call to FindFolder to get the necessary information */
	err=FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,volRefNum,prefDirID);
	
	if (err==noErr){
		
		if (logFoldName[0]==0){   /* DN - Duh, this is where I forgot an '=' :-( */
		
			*foldDirID=*prefDirID;
			return true;
		} else {
			CInfoPBRec pb;
			DirInfo *dpb=(DirInfo*)&pb; /* for the directory information */
			
			/* fill in our DirInfo record with our newly acquired information */
			dpb->ioVRefNum=*volRefNum;
			dpb->ioNamePtr=logFoldName;
			dpb->ioDrDirID=*prefDirID;
			dpb->ioCompletion=nil;
			dpb->ioFDirIndex=0;
			
			/* call PBGetCatInfo to see if we can find the folder */
			err=PBGetCatInfo(&pb,false);
			
			/*	if we get through the next if statement, then we are almost
				all set. */
			if ((err==noErr)||(err==dirNFErr)||(err==fnfErr)) { 
				/*	then the folder should exist, use the dir id to put 
					us in that folder */
				
				if ((err==dirNFErr)||(err==fnfErr)) /* then we must create it because
														it does not yet exist... */
					err=DirCreate(*volRefNum,*prefDirID,logFoldName,foldDirID);
				else /* it already exists */
					*foldDirID=dpb->ioDrDirID;
				
				if (err==noErr)
					return true;
			}
		}
	}
	return false; /* error */
}



/*
	OpenLog
	
	Opens the log file in the prefs folder.  If the file does 
	not exist, then we need to create it.
	
	logName - name of the file to open (and create)
	volRefNum - the volume reference number
	dirID - the directory id for where to put the new file.
	
	Returns the freshly opened file's reference number or -1 if there
	was an error.
*/
short OpenLog(Str255 logName,short volRefNum,long DirID){
	OSErr err;
	short refNum;
	FSSpec tempSpec;
	
	/* make our FSSpec.  A fnfErr means the file doesn't exist, but the FSSpec is
		still valid. */
	err=FSMakeFSSpec(volRefNum,DirID,logName,&tempSpec);
	
	/* create the file if necessary */
	if (err==fnfErr)
		err=FSpCreate(&tempSpec,'R*ch','TEXT',smSystemScript);
	
	/* open the file if all is well. */
	if (err==noErr){
		err=FSpOpenDF(&tempSpec,fsRdWrPerm,&refNum);
		
		if (err==noErr){ 
			/* then the log was opened without a flaw.  The next step is to set 
				the file pointer to the end so we can start appending data... */
			err=SetFPos(refNum,fsFromLEOF,0);
			
			if (err==noErr){
				
				/* then we have successfully opened the file, and everything is ready
					to go. */
				
				return refNum;
				
			} // SetFPos
		}// FSpOpenDF
	}// FSMakeFSSpec

	return -1; /* error */
}



/*
	Log
	
	A printf type function that takes random parameters, puts them together and 
	then prints the new string to the specified log file.
	
	refNum - the log file's reference number
	fmt - a printf-style format string
	
	returns true if everything went well, otherwise it will return false after
	trying to close the log file.
*/
Boolean Log(short refnum,const char* fmt,...){
	char stringBuff[1000]; /* gives us plenty of room to work with */
	OSErr err;
	long llen; /* line length */
	  
	/* first we expand the string... */
	vsprintf(stringBuff,fmt,__va(fmt));
  
	/* now we need to know the length... */
	llen=(long)strlen(stringBuff);
	
	/* now we can write the string... */
	err=FSWrite(refnum,&llen,stringBuff);
	
	/* if we have an error then we close the log and return false. */
	if (err!=noErr){
		CloseLog(refnum);
		return false; /* error */
	} else
		return true;
}




/*
	LogTime
	
	A printf type function that takes random parameters, puts them together and 
	then prints the new string to the specified log file.  Also included is the 
	date and time...
	
	The date is printed using mo/da/yr, time is hr:min followed by am/pm.
	
	refNum - the log file's reference number
	fmt - a printf-style format string
	
	returns true if everything went well, otherwise it will return false after
	trying to close the log file.
*/
Boolean LogTime(short refnum,const char* fmt,...){
	char stringBuff[1000]; /* temporary string buffers */
	char tempBuff[1000];
	OSErr err;
	long llen; /* string len */
	DateTimeRec dtr; /* record for receiving the date and time info */
	short y1,y2,y3; /* misc variables */
  
	/* first we expand the string... */
	vsprintf(tempBuff,fmt,__va(fmt));
  
	/* now add the date and time... */
	GetTime(&dtr);
	y1=dtr.year;
	
	/* extract the last two numbers from the year */
	if (y1>100){
		y2=y1/100;
		y2 *= 100;
		
		y1 -= y2;
	}
	
	y2=dtr.hour; /* get the hour */
	
	/* determine am or pm, and yes this part could have been done a little
		nicer, but it works and that's all you should be concerned about ;-) */
	if (y2>=12){ /* then we are in pm */
		if (y2>12) /* then decrement so it is normal time. */
			y2 -= 12;
		
		/* now build the string */
		sprintf(stringBuff,"%02d/%02d/%02d\t%02d:%02dpm\t%s",
			dtr.month,dtr.day,y1, y2,dtr.minute, tempBuff);
			
  } else{ /* then we are in am */
  	if (y2==0) /* then pretend it is after midnight (use 12 instead of zero) */
  		y2=12;
  	
		/* build the string */
		sprintf(stringBuff,"%02d/%02d/%02d\t%02d:%02dam\t%s",
			dtr.month,dtr.day,y1, y2,dtr.minute, tempBuff);
	}
	
	/* now we need to know the length... */
	llen=(long)strlen(stringBuff);

	/* now we can write the string... */
	err=FSWrite(refnum,&llen,stringBuff);
	
	/* if we have an error then we close the log and return false. */
	if (err!=noErr){
		CloseLog(refnum);
		return false; /* error */
	} else
		return true;
}




/*
	CloseLog
	
	Closes the specified log file.
	
	refnum - reference number for the log file to close.
*/
void CloseLog(short refnum){
	
	FSClose(refnum);
}
