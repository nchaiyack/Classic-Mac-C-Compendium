/*
	Main.c
	
	This application will open some log components and write some information to the
	logs that they keep.  This is intended to be a demonstration of how to use the
	log component.
	
	4/20/94 dn - Created.
	7/3/94 dn - Modified to work with the Universal Headers.
*/

//#define DEBUGIT 2			/* uncomment this line for debugging */

#include <Libsprintf.h>
#include <Folders.h>
#include <Components.h>

#include "LogLibComponent.h"

/*
	Prototypes
*/
void InitToolbox(void);
void main(void);
Boolean CheckComponentMgr(void);
OSErr SetLog1(ComponentInstance log1);
OSErr SetLog2(ComponentInstance log2);
OSErr SetLog3(ComponentInstance log3);
OSErr WriteTo(ComponentInstance log,char* buffer,Boolean time);
OSErr WriteToLogs(ComponentInstance log1,ComponentInstance log2,ComponentInstance log3);
void AlertError(StringPtr errstr);
void GiveSystemTime(void);

#ifdef DEBUGIT

#include "LogLibComponent Private.h"

Component Registerem(void);

#else
Component GetLogComponent(void);
#endif

/*
	Globals
*/

Component logComp;

ComponentInstance LogFile1,LogFile2,LogFile3;

/*
	InitToolbox
	
	Initializes the Mac Toolbox.
*/
void InitToolbox(){
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

/*
	CheckComponentMgr
	
	Uses Gestalt to check for the availability of the Component Manager.  We assume that if Gestalt
	does not return an error then the Component Manager exists.  If we were concerned with what
	version of the Component Manager that we had we could check the result to see if it is at least
	the version that we need.
*/
Boolean CheckComponentMgr(){
	OSErr err;
	long result;
	
	err=Gestalt(gestaltComponentMgr,&result);
	
	if (err!=noErr)
		return false;
	else
		return true;
}

/*
	main
	
	Main routine.  This doesn't really do much.  It simply creates 3 logs and writes some information
	into them.
*/
void main(){
	Component		logComp;
	OSErr			err;
	
	LogFile1=LogFile2=LogFile3=(ComponentInstance)0;
	
	InitToolbox();
	
	GiveSystemTime();
	
	// is the component mgr available?
	if (CheckComponentMgr()){
		
		GiveSystemTime();

		// ok, first we have to find the component...
#ifdef DEBUGIT
		{	// this function is defined in LogLibComponent.c.  It is here because somehow we have to get over to
			// the file whilst in the debugger.  By stepping into this call, the debugger will switch to that file where
			// we can set up all of our nifty breakpoints...
			
			extern void DumbTest(void);
			
			DumbTest();
		}
		
		logComp=Registerem();
#else
		logComp=GetLogComponent();
#endif
		
		GiveSystemTime();

		if (logComp!=(Component)0){// then we were successful, procede
			
			GiveSystemTime();

			// ok, now open our three instances...
			LogFile1=OpenComponent(logComp);
			
			GiveSystemTime();

			LogFile2=OpenComponent(logComp);
			
			GiveSystemTime();

			LogFile3=OpenComponent(logComp);
			
			GiveSystemTime();

			// they are open, now to change some of the defaults...
			// no change for LogFile1
			
			// change the creator for LogFile2
			err=LogDefaults(LogFile2,'KAHL',kLogDefaultType,kLogDefaultOpenSetting);
			
			GiveSystemTime();

			// change the open setting for LogFile3
			err=LogDefaults(LogFile3,kLogDefaultCreator,kLogDefaultType,true);
			
			GiveSystemTime();

			// ok, the defaults are set, now set up the exact files...
			err=SetLog1(LogFile1);
			
			GiveSystemTime();

			err=SetLog2(LogFile2);
			
			GiveSystemTime();

			err=SetLog3(LogFile3);
			
			GiveSystemTime();

			// call a procedure to write to the 3 logs...
			WriteToLogs(LogFile1,LogFile2,LogFile3);
			
			GiveSystemTime();

			// finally, we have to close the logs before we exit...
			
			LogTime(LogFile1,"Closing the log.\r\r");
			
			err=CloseComponent(LogFile1);
			
			GiveSystemTime();
			
			LogTime(LogFile2,"Closing the log.\r\r");
			
			err=CloseComponent(LogFile2);
			
			GiveSystemTime();
			
			LogTime(LogFile3,"Closing the log.\r\r");
			
			err=CloseComponent(LogFile3);
			
			GiveSystemTime();

		} else
			AlertError("\pLog Component not installed!");
		
		GiveSystemTime();

	} else
		AlertError("\pComponent Manager not installed!");
}

/*
	SetLog1
	
	Sets up log 1.  Indicates to the log component that the log should be named "Log File #1" and kept
	in the same directory as this application.  It uses the Process Manager to retrieve information about
	where the application is.
*/
OSErr SetLog1(ComponentInstance log1){
	ProcessSerialNumber psn;
	ProcessInfoRecPtr pir;
	FSSpec spec2;
	OSErr err;
	Str255 aname="\pLog File #1";
	FSSpec spec;
	short vref;
	long dirid;

	err=GetCurrentProcess(&psn);
	
	if (err!=noErr){
		AlertError("\pGetCurrentProcess Error.");
		ExitToShell();
	}
	
	pir=(ProcessInfoRecPtr)NewPtrClear(sizeof(ProcessInfoRec));
	pir->processInfoLength=sizeof(ProcessInfoRec);
	pir->processName=(StringPtr)NewPtr(32);
	pir->processAppSpec=&spec2;
	
	err=GetProcessInformation(&psn,pir);
	
	if (err!=noErr){
		AlertError("\pGetProcessInformation Error.");
		ExitToShell();
	}

	GiveSystemTime();

	// use the FSSpec in the ProcessInfoRec for log file info, but use our name...
	spec.vRefNum=pir->processAppSpec->vRefNum;
	spec.parID=pir->processAppSpec->parID;
	BlockMove((Ptr)(aname),(Ptr)(spec.name),(aname[0])+1);
	
	err=LogSetupFSSpec(log1,&spec);

	if (err!=noErr){
		AlertError("\pLogSetupFSSpec Error.");
		ExitToShell();
	}
		
	// get rid of the space that we recorded...
	DisposePtr((Ptr)pir->processName);
	DisposePtr((Ptr)pir);
	
	GiveSystemTime();

	return err;
}

/*
	SetLog2
	
	Sets up log2.  This log will go into the prefs folder in the active system file.  It will be named
	"Log File #2".  Uses FindFolder to get the information.
*/
OSErr SetLog2(ComponentInstance log2){
	OSErr err;
	Str255 name="\pLog File #2";
	short vref;
	long dirid;
	
	err=FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&vref,&dirid);
	
	GiveSystemTime();

	err=LogSetup(log2,name,vref,dirid);
	
	GiveSystemTime();

	return err;
}

/*
	SetLog3
	
	Sets up log3.  This log will go on the desktop (of the system drive).  It will be named (unbelievably)
	"Log File #3".  Uses FindFolder to get the information.
*/
OSErr SetLog3(ComponentInstance log3){
	OSErr err;
	Str255 name="\pLog File #3";
	short vref;
	long dirid;
	
	err=FindFolder(kOnSystemDisk,kDesktopFolderType,kCreateFolder,&vref,&dirid);
	
	GiveSystemTime();

	err=LogSetup(log3,name,vref,dirid);
	
	GiveSystemTime();

	return err;
}

/*
	WriteTo
	
	Writes a buffer to a specified log.
*/
OSErr WriteTo(ComponentInstance log,char* buffer,Boolean time){
	OSErr err;
	
	if (time)
		err= LogTime(log,buffer);
	else
		err= LogText(log,buffer);
	
	GiveSystemTime();

	return err;
}

/*
	WriteToLogs
	
	Writes to the three logs.
*/
OSErr WriteToLogs(ComponentInstance log1,ComponentInstance log2,ComponentInstance log3){
	char buffer[400];
	short fref;
	ComponentResult cres;
	
	WriteTo(log1,"Starting test #1.\r\r",true);
	WriteTo(log2,"Starting test #2.\r\r",true);
	WriteTo(log3,"Starting test #3.\r\r",true);
	
	WriteTo(log2,"This is some test data...",false);
	WriteTo(log2,"   That goes on the same line.\r",false);
	
	WriteTo(log1,"The current time and date is: ",false);
	WriteTo(log1,".\r\r",true);
	
	WriteTo(log3,"This is pretty boring, huh?\r\rWell, lets try the new libsprintf routines...\r\r",false);
	
	libsprintf(buffer,"This is a test of libsprintf:\r\t%% \t\tvalue\t\tresult\r");
	WriteTo(log1,buffer,false);
	
	libsprintf(buffer,"\td\t\t15\t\t\t%d\r",15);
	WriteTo(log1,buffer,false);
	
	libsprintf(buffer,"\t08x\t\t35\t\t\t%08x\r",35);
	WriteTo(log1,buffer,false);
	
	libsprintf(buffer,"\tc\t\t\'x\'\t\t\t%c\r\r",'x');
	WriteTo(log1,buffer,false);
	
	{
		char buf1[]="\rThis line (or two) is being added straight from the application.\r";
		char buf2[]="An application might want to do this if it has alot of stuff to add to the file,\r";
		char buf3[]="or just because it has data that is not a C or Pascal string that needs to be logged...\r\r";
		long ch1,ch2,ch3;
		char* cp;
		
		ch1=ch2=ch3=0L;
		
		for (cp=buf1;*cp;cp++)
			ch1++;
		
		for (cp=buf2;*cp;cp++)
			ch2++;
		
		for (cp=buf3;*cp;cp++)
			ch3++;
		
		
		cres=LogWrite(log2,&ch1,(Ptr)buf1);
		cres=LogWrite(log2,&ch2,(Ptr)buf2);
		cres=LogWrite(log2,&ch3,(Ptr)buf3);
		
	}
	
	return cres;
}

/*
	AlertError
	
	Does an alert for the user to see.  Only if no component mgr or no log component.
*/
void AlertError(StringPtr msg){
	
	ParamText(msg,"\p","\p","\p");
	
	StopAlert(128,(ModalFilterUPP)0);
}

/*
	GiveSystemTime
	
	Gives time to the system to process other items while we are waiting for
	something to happen.
*/
void GiveSystemTime(void){
	EventRecord er;
	
	if (WaitNextEvent(everyEvent,&er,100,(RgnHandle)0)){
		if (er.what==keyDown){
			// check for command-period
			char ch;
			
			ch=er.message&charCodeMask;
			
			if ((er.modifiers&cmdKey)&&(ch=='.')){
				// the user wants to cancel...

				AlertError("\pReceived signal to cancel execution, aborting...");
				
				if (LogFile1!=(ComponentInstance)0){	// then the logs could be open
					CloseComponent(LogFile1);
					CloseComponent(LogFile2);
					CloseComponent(LogFile3);
				}
				
				ExitToShell();
			}
		}
	}
}

#ifdef DEBUGIT

static Str255 nameLog="\pLog Component (Debug)";
static Str255 infoLog="\pLogs items to a file.";

/*
	Registerem
	
	Registers the test component (if the DEBUGIT flag is set)
*/
Component Registerem(){
	Component res;
	ComponentDescription desc;
	
	desc.componentType='LogD';// uses 'LogD' for debug instead of the standard 'LogC'
	desc.componentSubType=kComponentWildCard;
	desc.componentManufacturer='appl';
	desc.componentFlags=kComponentWildCard;
	desc.componentFlagsMask=kComponentWildCard;
	
	res=RegisterComponent(&desc,(ComponentRoutine)_LogDispatch,0,nil,nil,nil);
	
	return res;
}
#else
/*
	GetLogComponent
	
	Try to find the log component.
*/
Component GetLogComponent(void){
	Component aComp;
	
	ComponentDescription desc;
	
	desc.componentType=kLogLibComponentType;
	desc.componentSubType=kComponentWildCard;
	desc.componentManufacturer=kComponentWildCard;
	desc.componentFlags=kComponentWildCard;
	desc.componentFlagsMask=kComponentWildCard;
	
	aComp=FindNextComponent((Component)0,&desc);
	
	return aComp;
}
#endif
