/*
	LogLibComponent.c
	
	Code version 1.02
	
	This is the source for a log file component.  For those of you who don't know what a component is, read
	the Component.note file.
	
	5/7/94 dn - Created.
	7/3/94 dn - Modified to work with the Universal Headers.
	6/26/95 dn - Modified to build a Code Resource using Sym C++ 8.0 (Rainbow).
*/

/*
	In PPC applications, constructors and destructors for global and static objects are called automatically
	by __cplusstart, which is actually the main entry point to your application (and not main()).  This
	routine calls your constructors, initializes your QuickDraw globals, calls your main() function, then
	calls your destructors.
	
	In a shared library, constructors for global and static objects are called by __cplusinit, which the linker
	assigns as an initialization routine.  The Code Fragment Manager will automatically call this initialization
	routine when your shared library is loaded.  __cplusinit will also allow hook your shared library's
	QuickDraw globals to the QuickDraw globals of the calling application.  Destructors for global and static
	objects are called by __cplusterm, which the linker assigns as a termination routine.  The Code Fragment
	Manager will automatically call this termination routine when your shared library is unloaded.
	
	In code resources, there is no way to call these constructors or destructors automatically, or hook
	QuickDraw globals, and still preserve the arguments passed to your main entry point (main()).  Instead,
	you will need to call initialization and termination routines to do this for you.
	
	In C, you should use __rsrcinit() and __rsrcterm(), defined in stdlib.h.
	
	In C++, you should use __cplusrsrcinit() and __cplusrsrcterm(), defined in new.h.
	
	The C++ initialization and termination routines will automatically call the C initialization and termination
	routines.  You should only use one set or the other.
	
	You must not use any objects that rely on their constructor, or any QuickDraw globals, before calling
	__rsrcinit() or __cplusrsrcinit().
	
	
	Singlely-entrant code resources :
	
	If your code resource is called only once, you should call __rsrcinit() or __cplusrsrcinit() at the start of
	main(), and __rsrcterm() or __cplusrsrcterm() at the end.
	
	Reentrant code resources :
	
	If your code resource is reentrant, you will need to call __rsrcinit() or __cplusrsrcinit() only the first
	time through, and __rsrcterm() or __cplusrsrcterm() the last time through.
	
	
	Colen Garoutte-Carson
	Symantec Corp.
*/

// #define DEBUGIT 2

#include <Packages.h>

#include "LogLibComponent Private.h"

#if USESROUTINEDESCRIPTORS
// Only use if compiling with Rainbow...
#include <stdlib.h>

static Boolean gDidInit=false,gDoTerm=false;
#endif

/*
	Local prototypes, macros, definitions, etc.
	
*/

// a few macros to make our lives seem easier...
#define mCCFWS(x)	CallComponentFunctionWithStorage(storage,params,(ComponentFunctionUPP)x)
#define mCCF(x)	CallComponentFunction(params,(ComponentFunctionUPP)x)

#define mSetChar(a,b)	(*(a) = b); a++
#define mSetCharV(a,b)	(*(a) = (b+'0'));a++

#define fsDontCache			0x20

// define the HandleState type
typedef char HandleState;

// prototypes for the local functions.
HandleState LockHandle(Handle h);
void UnlockHandle(Handle h,HandleState state);
OSErr CloseLog(LogLibHdl mem);
OSErr TryCloseLog(LogLibHdl mem);
ComponentResult VerifyTypeCreator(Handle storage);
ComponentResult OpenTheLogFile(Handle storage);
OSErr FSWriteNoCache(short refnum, long *count_p, const Ptr buffer_p);
OSErr LogData(Handle storage,char* data);
OSErr LogCR(Handle storage);
OSErr FSFlushFile(short fref);
OSErr LogTheTime(Handle storage);
char* AddIntToBuf(char* buf,char val);

/*
	And now back to our main program...
	
*/

/*
	main or _LogDispatch
	
	Component dispatch routine.  Depending on whether the DEBUGIT flag is set, this will either
	compile to the main function routine or a simple dispatch routine to be passed the the component
	manager's RegisterComponent routine.
*/
#ifdef DEBUGIT
pascal ComponentResult _LogDispatch(ComponentParameters* params, Handle storage){
#else
pascal ComponentResult main(ComponentParameters* params,Handle storage){
#endif
	
	ComponentResult cres=noErr;
	
#ifndef DEBUGIT
#if USESROUTINEDESCRIPTORS
	if (!gDidInit){
		gDidInit=true;
		__rsrcinit();
	}
#endif
#endif

	switch (params->what){	// switch to call the appropriate routine based on the value of the routine to call
		
		case kComponentOpenSelect:		// open request
		
			cres=mCCFWS(_LogOpen);
			break;
		
		case kComponentCloseSelect:		// close
			
			cres=mCCFWS(_LogClose);
#if USESROUTINEDESCRIPTORS
			gDoTerm=true; // flag that we are going away...
#endif
			break;
			
		case kComponentCanDoSelect:		// cando
			
			cres=mCCF(_LogCanDo);
			break;
			
		case kComponentVersionSelect:		// version
			
			cres=mCCF(_LogVersion);
			break;
			
		case kComponentTargetSelect:		// target
			
			cres=mCCFWS(_LogTarget);
			break;
			
		case kComponentRegisterSelect:	// register
			
			cres=mCCFWS(_LogRegister);
			break;
			
		case kLogStorage:				// return the internal storage
			
			cres=mCCFWS(_LogStorage);
			break;
			
		case kLogDefaults:				// change the defaults
			
			cres=mCCFWS(_LogDefaults);
			break;
			
		case kLogSetup:				// set up the log file
			
			cres=mCCFWS(_LogSetup);
			break;
			
		case kLogSetupFSp:				// set up the log file using a FSSpec
			
			cres=mCCFWS(_LogSetupFSp);
			break;
			
		case kLogText:					// log some C-style text
			
			cres=mCCFWS(_LogText);
			break;
			
		case kLogTime:					// log some C-style text prepended by the time and date
			
			cres=mCCFWS(_LogTime);
			break;
		
		case kLogPText:				// log some Pascal style text
		
			cres=mCCFWS(_LogPText);
			break;
		
		case kLogPTime:				// log some Pascal style text prepended by the date and time
			
			cres=mCCFWS(_LogPTime);
			break;
			
		case kLogWrite:				// log some data to the log file
			
			cres=mCCFWS(_LogWrite);
			break;
		
		default:						// invalid selector code
			
			cres=paramErr;
			break;
	}
	
#ifndef DEBUGIT
#if USESROUTINEDESCRIPTORS
	if (gDoTerm){
		gDoTerm=false;
		__rsrcterm();
	}
#endif
#endif

	return cres;
}

/*
	LogOpen
	
	Handles the opening of the log component.  This allocates the internal memory that we will be using and
	initializes it for us.
*/
pascal ComponentResult _LogOpen(Handle storage,ComponentInstance self){
	ComponentResult cres=noErr;
	LogLibHdl mem;
	LogLibPtr mp;
	short ourResFork;
	
	HandleState hs;
	
	mem=(LogLibHdl)NewHandleClear(sizeof(LogLibRec));		// allocate the memory;
	
	if (mem==(LogLibHdl)0){								// then there was a problem...
		cres=MemError();
		
		return cres;
	}
	
	ourResFork=OpenComponentResFile((Component)self);		// open the resource file
	
	// lock down our memory...
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	// initialize our values...
	
	mp->logSpec.vRefNum=0;
	mp->logSpec.parID=0;
	
	mp->fileRefNum=-1;									// indicates that the file is not open
	
	mp->inited=false;
	
	// setup default values...
	mp->keepLogOpen=false;
	mp->savedValue=false;
	mp->wasOpened=false;
	
	mp->creator='R*ch';
	mp->type='TEXT';
	
	// initialize the standard stuff...
	mp->self=(Component)self;
	mp->kidnapper=(ComponentInstance)0;
	mp->delegate=(Component)0;
	mp->delegated=(ComponentInstance)0;
	mp->resFileRefNum=ourResFork;
	
	// unlock the memory
	UnlockHandle((Handle)mem,hs);
	
	SetComponentInstanceStorage(self,(Handle)mem);			// save the storage just created.
	
	return cres;
}

/*
	_LogClose
	
	Closes the log file if it is open.  It also disposes of our storage before going away.
*/
pascal ComponentResult _LogClose(Handle storage,ComponentInstance self){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	CloseComponentResFile((*mem)->resFileRefNum);			// close the resource file
	
	UnlockHandle((Handle)mem,hs);	
	
	cres=(ComponentResult)CloseLog(mem);					// close the log file
	
	DisposeHandle(storage);								// get rid of the internal storage
	
	return cres;
}

/*
	_LogRegister
	
	Handles the registration message.  This message is only received once, during system startup when the
	Component Manager does it's 'thng's. ;-)
	
	So this is where we put the code to display our little icon.  Isn't that special?
	
	The icon is not displayed (and the code is not needed) if the DEBUGIT flag is set.
*/
pascal ComponentResult _LogRegister(Handle storage,ComponentInstance self){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	HandleState hs;

#ifndef DEBUGIT
	extern void ComponentShowInit(void);
	
	hs=LockHandle((Handle)mem);
	
	UseResFile((*mem)->resFileRefNum);
	
	ComponentShowInit();
	
	UnlockHandle((Handle)mem,hs);

#endif

	return cres;
}

/*
	_LogTarget
	
	This message is received when another component is capturing us.  We don't really do much besides save
	their instance.
*/
pascal ComponentResult _LogTarget(Handle storage,ComponentInstance kidnapper){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	
	(*mem)->kidnapper=kidnapper;
	
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	_LogCanDo
	
	Determines if the log component can do the function selector requested.  Returns true if it can, false if not.
*/
pascal ComponentResult _LogCanDo(short selector){
	ComponentResult cres=noErr;
	
	switch(selector){
		case kComponentOpenSelect:	// open request
		case kComponentCloseSelect:
		case kComponentCanDoSelect:
		case kComponentVersionSelect:
		case kComponentTargetSelect:
		case kComponentRegisterSelect:
		case kLogStorage:
		case kLogDefaults:
		case kLogSetup:
		case kLogSetupFSp:
		case kLogText:
		case kLogTime:
		case kLogWrite:
		
			cres=(ComponentResult)true;			// yes, we can do the above functions
			break;
			
		default:
			
			cres=(ComponentResult)false;			// no, an invalid or unsupported selector code
			break;
	}
	return cres;
}

/*
	_LogVersion
	
	Returns the version of the component.
*/
pascal ComponentResult _LogVersion(void){
	ComponentResult cres=noErr;
	
	cres=(ComponentResult)kLogLibComponentVersion;
	
	return cres;
}

/*
	_LogStorage
	
	Returns a handle to the components internal storage.
*/
pascal ComponentResult _LogStorage(Handle storage){
	ComponentResult cres=noErr;
	
	cres=(ComponentResult)storage;
	
	return cres;
}

/*
	_LogDefaults
	
	Changes the defaults for the log file.  If the file already exists, we should find and make the changes anyway.
*/
pascal ComponentResult _LogDefaults(Handle storage,OSType creator,OSType type,Boolean keepOpen){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	
	mp=*mem;
	
	mp->creator=creator;
	mp->type=type;
	mp->keepLogOpen=keepOpen;
	
	cres=VerifyTypeCreator(storage);
	
	UnlockHandle((Handle)mem,hs);
	
	if (keepOpen){
		cres=OpenTheLogFile(storage);
	} else
		cres=TryCloseLog(mem);
	
	return cres;
}

/*
	_LogSetup
	
	Sets up the FSSpec for the log file.
*/
pascal ComponentResult _LogSetup(Handle storage,StringPtr name,short vref,long dirid){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	// we can only do this if the file is not open...
	if (mp->fileRefNum!=-1){
		UnlockHandle((Handle)mem,hs);
		return paramErr;
	}
	
	// now we have to set up the true fsspec in the global
	// we don't use the FSMakeFSSpec call because I am not sure if the string is copied or just a pointer
	// to the string is used (we need a copy so that it will stay around)...
	BlockMove(name,(Ptr)(mp->logSpec.name),(name[0])+1);
	mp->logSpec.parID=dirid;
	mp->logSpec.vRefNum=vref;
	
	
	if ((cres==noErr)||(cres==fnfErr)){
		mp->inited=true;
		mp->wasOpened=false;
	}
	
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	_LogSetupFSp
	
	Use the FSSpec that the user is passing to the routine.
*/
pascal ComponentResult _LogSetupFSp(Handle storage,FSSpec* fsp){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	// we can only do this if the file is not open...
	if (mp->fileRefNum!=-1){
		UnlockHandle((Handle)mem,hs);
		return paramErr;
	}
	
	// copy their fsspec into ours.
	BlockMove((Ptr)fsp->name,(Ptr)(mp->logSpec.name),(fsp->name[0])+1);
	mp->logSpec.parID=fsp->parID;
	mp->logSpec.vRefNum=fsp->vRefNum;
	
	mp->inited=true;
	mp->wasOpened=false;
	
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	_LogText
	
	Add some text to the log file.
*/
pascal ComponentResult _LogText(Handle storage,char* buffer){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	cres=(ComponentResult)LogData(storage,buffer);
		
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	_LogTime
	
	Same as above but it adds the time to the file first. (no carriage return).
*/
pascal ComponentResult _LogTime(Handle storage,char* buffer){
	ComponentResult cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	cres=LogTheTime(storage);
	
	if (cres==noErr){	// then we can log the data...
		
		cres=(ComponentResult)LogData(storage,buffer);
		
	}
	
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	_LogPText and _LogPTime
	
	Used for printing pascal type strings.
*/
pascal ComponentResult _LogPText(Handle storage,unsigned char* buf){
	long size;
	unsigned char cs=buf[0],*cp=buf+1;
	
	size=0L;size += cs; // convert the size to a long
	
	return _LogWrite(storage,&size,(Ptr)cp);
}

pascal ComponentResult _LogPTime(Handle storage,unsigned char* buf){
	long size;
	unsigned char cs=buf[0],*cp=buf+1;
	OSErr err;
	
	size=0L;size += cs;			// convert the size to a long
	
	err=LogTheTime(storage);	// first log the time
	
	if (err==noErr)				// continue to write the text
		err=_LogWrite(storage,&size,(Ptr)cp);
	
	return err;
}

/*
	_LogWrite
	
	Like FSWrite, this procedure writes a buffer of text to the log file.
*/
pascal ComponentResult _LogWrite(Handle storage,long* size,Ptr buffer){
	ComponentResult ret=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	if (mp->fileRefNum==-1){							// then the file is not open, open it now
		ret=OpenTheLogFile(storage);
	}
	
	if (ret==noErr)									// then write the text
		ret=FSWriteNoCache(mp->fileRefNum,size,buffer);	// don't use the cache...
	
	TryCloseLog(mem);								// close the log if it can be closed
	
	UnlockHandle((Handle)mem,hs);
	
	return ret;
}


/*еееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееее*\

									UTILITY FUNCTIONS
									
\*еееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееееее*/

/*
	HandleLock
	
	Locks a handle down, but returns the state that the handle was in previously (for resetting later...)
*/
HandleState LockHandle(Handle h){
	HandleState st;
	
	st=HGetState(h);				// get the current state of the handle
	
	HLock(h);						// lock it down
	
	return st;						// return the state
}

/*
	UnlockHandle
	
	Unlocks a handle, but resets it to the previous setting.
*/
void UnlockHandle(Handle h,HandleState state){
	
	HUnlock(h);					// unlock the handle
	
	HSetState(h,state);				// reset the state to the stored state
}

/*
	CloseLog
	
	Closes the log if it is open...
	
	Even though we are using a version of PBWrite that should allow us to write the file without having
	to worry about the cache, there are two FlushVol calls in here just to ensure that everything is flushed
	completely.
*/
OSErr CloseLog(LogLibHdl mem){
	OSErr cres=noErr;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	if (mp->inited){	// then the FSSpec was initialized, check to see if the file is open
		if (mp->fileRefNum!=-1){	// then the file is still open...
			
			// we will flush the file to ensure that everything is written out before closing...
			cres=FlushVol((StringPtr)0,mp->logSpec.vRefNum);
			
			// close the file
			FSClose(mp->fileRefNum);
			
			// again, just to be sure...
			cres=FlushVol((StringPtr)0,mp->logSpec.vRefNum);
			
			mp->fileRefNum=-1;	// reset the refnum to -1 to indicate the file is closed.
		}
	}
	
	UnlockHandle((Handle)mem,hs);
	
	return cres;
}

/*
	TryCloseLog
	
	Only close the log if it can be closed (i.e. it should not if the keepLogOpen flag is set).
*/
OSErr TryCloseLog(LogLibHdl mem){
	LogLibPtr mp;
	HandleState hs;
	OSErr err=paramErr;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	if (!(mp->keepLogOpen))		// if we can close the file,
		err=CloseLog(mem);		// then close it
	
	UnlockHandle((Handle)mem,hs);
	
	return err;
}

/*
	VerifyTypeCreator
	
	Called when a change is made to the type/creator of the log file as well as the first time this component
	opens the file (to make sure everything is as it should be).
*/
ComponentResult VerifyTypeCreator(Handle storage){
	OSErr cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	
	mp=*mem;
	
	if (mp->inited){ // then the FSSpec is valid, it is possibly an existing file that we should change the stuff for.
		FInfo info;
		
		cres=FSpGetFInfo(&(mp->logSpec),&info);	// get the info for the file
		
		if (cres==noErr){	// make the changes...
			
			if ((info.fdType!=mp->type)||(info.fdCreator!=mp->creator)){	// but only if they are different
				info.fdType=mp->type;
				info.fdCreator=mp->creator;				// reset to the new type and creator
				
				cres=FSpSetFInfo(&(mp->logSpec),&info);	// make the changes
			}
		}
		
		if (cres==fnfErr){	// then the file doesn't exist yet
			cres=noErr;	// but that's ok
		}
		
	}
	
	UnlockHandle((Handle)mem,hs);
	
	return (ComponentResult)cres;
}

/*
	OpenTheLogFile
	
	Opens the log file based on the stored FSSpec and sets the file pointer to the end of the file so that we
	can start appending.  This should also set the cache bit thingy so that our stuff is not cached.
*/
ComponentResult OpenTheLogFile(Handle storage){
	OSErr cres=noErr;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	
	hs=LockHandle((Handle)mem);
	
	mp=*mem;
	
	// cannot do it if the fsspec hasn't been set up...
	if (mp->inited==false){
		UnlockHandle((Handle)mem,hs);
		return paramErr;
	}

	if (mp->fileRefNum!=-1){	// then the file is already open
		UnlockHandle((Handle)mem,hs);
		return noErr;
	}
	
	if (!(mp->wasOpened)){	// if the file hasn't been opened before this component was created, then...
		VerifyTypeCreator(storage);	// it may exist but it also may need the type/creator changed
		mp->wasOpened=true;		// flag this as having been done.
	}
	
	// ok, now use our FSSpec to open the file...
	cres=FSpOpenDF(&(mp->logSpec),fsWrPerm,&(mp->fileRefNum));
	
	if (cres==fnfErr){	// then the file hasn't been created yet...
		cres=FSpCreate(&(mp->logSpec),mp->creator,mp->type,smSystemScript);	// create it
		
		if (cres==noErr)												// it was created
			cres=FSpOpenDF(&(mp->logSpec),fsWrPerm,&(mp->fileRefNum));		// open it again
	}
	
	if (cres!=noErr){				// the file couldn't be opened, mark us as being closed
		mp->fileRefNum=-1;
	} else {	// move the file pointer to the end of the file so that we can begin appending...
		cres=SetFPos(mp->fileRefNum,fsFromLEOF,0);
	}
	
	UnlockHandle((Handle)mem,hs);
	
	return (ComponentResult)cres;
}

/*
	FSWriteNoCache
	
	Writes to the disk without caching the information (which can speed things up somewhat...).
*/
OSErr FSWriteNoCache(short refnum, long *count_p, const Ptr buffer_p){
	OSErr retcode;
	ParamBlockRec pb;
	
	pb.ioParam.ioCompletion = 0;
	pb.ioParam.ioRefNum = refnum;
	pb.ioParam.ioBuffer = buffer_p;
	pb.ioParam.ioReqCount = *count_p;
	pb.ioParam.ioPosMode = fsAtMark | fsDontCache; 
	pb.ioParam.ioPosOffset = 0;
	
	retcode = PBWrite(&pb, false);
	
	*count_p = pb.ioParam.ioActCount;
	
	return retcode;
}

/*
	LogData
	
	Writes the data out to the log file (it has to be open before we get here.
*/
OSErr LogData(Handle storage,char* data){
	long count=0L;
	Ptr pt=(Ptr)data;
	char* cp=data;
	LogLibHdl mem=(LogLibHdl)storage;
	LogLibPtr mp;
	HandleState hs;
	OSErr ret=noErr;
	
	hs=LockHandle((Handle)mem);
	mp=*mem;
	
	if (mp->fileRefNum==-1){		// then open the file
		ret=OpenTheLogFile(storage);
	}
	
	while (*cp){				// calculate the size of the data to be written
		count++;
		cp++;
	}
	
	if (ret==noErr)				// then it is ok to write the data
		ret=FSWriteNoCache(mp->fileRefNum,&count,pt);
	
	TryCloseLog(mem);			// try to close the file
	
	UnlockHandle ((Handle)mem,hs);
	
	return ret;
}

/*
	LogCR
	
	Outputs a carriage return to the log file.
*/
OSErr LogCR(Handle storage){
	char data[20];
	
	data[0]=kNewLine;
	data[1]=0;
	
	return LogData(storage,data);
}

/*
	FSFlushFile
	
	Flushes a file out to disk.
*/
OSErr FSFlushFile(short fref){
	OSErr ret;
	IOParam pb;
	
	pb.ioCompletion=(IOCompletionUPP)0;
	pb.ioRefNum=fref;
	
	ret=PBFlushFile((ParmBlkPtr)&pb,false);
	
	return ret;
}

/*
	LogTheTime
	
	Outputs the time to the file.
*/
OSErr LogTheTime(Handle storage){
	char* timebuf;	// temp storage for putting the time into...
	DateTimeRec dtr;
	char ampm;
	short hour,year;
	char a,b,c,d,e,f;
	OSErr cres;
	char* cp1,*cp2;
	
	timebuf=(char*)NewPtr(100);
	
	if (timebuf!=(char*)0){	// then the space was allocated, ok to procede...
		GetTime(&dtr);
		
		ampm=(dtr.hour < 12)?('a'):('p');	// choose either am or pm based on the hour
		
		hour=dtr.hour;	// recalculate the hour so it is in 12 hour mode.
		
		if (hour<13){
			if (hour<1)
				hour=12;
		} else {
			hour -= 12;
		}
		
		if (hour<1)
			hour=12;
		else
		if (hour>12){
			hour -=12;
			ampm='p';
		}
		
		year=dtr.year;		// calculate the year so that it is only the last 2 digits
		if (year>100)
			year %=100;
		
		// temp storage
		a=dtr.month;
		b=dtr.day;
		c=year;
		d=hour;
		e=dtr.minute;
		f=dtr.second;
		
		// print it to the buffer
		cp1=AddIntToBuf(timebuf,a);
		mSetChar(cp1,'/');cp2=cp1;
		
		cp1=AddIntToBuf(cp2,b);
		mSetChar(cp1,'/');cp2=cp1;
		
		cp1=AddIntToBuf(cp2,c);
		mSetChar(cp1,'\t');cp2=cp1;
		
		cp1=AddIntToBuf(cp2,d);
		mSetChar(cp1,':');cp2=cp1;
		
		cp1=AddIntToBuf(cp2,e);
		mSetChar(cp1,':');cp2=cp1;
		
		cp1=AddIntToBuf(cp2,f);
		mSetChar(cp1,' ');
		
		mSetChar(cp1,ampm);mSetChar(cp1,'m');mSetChar(cp1,'\t');mSetChar(cp1,0);
		
		// write it to the file
		cres=(ComponentResult)LogData(storage,timebuf);
		
		// get rid of the temporary buffer.
		DisposePtr((Ptr)timebuf);
	} else {
		cres=MemError();
	}

	return cres;
}

/*
	AddIntToBuf
	
	Adds a character integer to the buffer (two places only). [i.e. this is a small itoa()]
*/
char* AddIntToBuf(char* buf,char val){
	
	if (val>99){	// ensure that we are only 2 digits...
		val %= 100;
	}
	
	if (val<10){	// special case if less than 10, need to print a zero first...
		
		mSetChar(buf,'0');
		mSetCharV(buf,val);
		
	} else { // 2 digit number...
		char a;
		
		a = val % 10;	// the lower number
		val /= 10;	// the higher number
		
		mSetCharV(buf,val);
		mSetCharV(buf,a);
	}
	
	return buf;
}

#ifdef DEBUGIT

void DumbTest(void);

/*
	DumbTest
	
	Dummy routine which gets us over here from the test routines.  To switch files in the TPM debugger,
	a call must actually be made to a function in that file.  This routine provides this function (the component
	manager intercepts and processes all of the calls so the debugger never gets into the file without this
	function).
*/
void DumbTest(){
	short a;
	
	a=2;
}

#endif






