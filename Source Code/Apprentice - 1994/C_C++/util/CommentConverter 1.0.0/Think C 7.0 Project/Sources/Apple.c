#ifndef bscoc
#define bscoc
#ifdef apple
// Alternativ: #pragma once


/***************************************************************************/
/*****   Datei, die Event-MCP und Multitasking/QuitApp Funktionen      *****/
/*****   für SETI bereitstellt.                                        *****/
/*****   Autor:    Andreas Amoroso                                     *****/
/*****   Datum:    11.8.1993                                           *****/
/***************************************************************************/



// Module with Functions for the MCP-Environment. Requires availability of System-7
//=================================================================================


// Most of these Headers may be included by Think-C, but those are needed

#include <ctype.h>
#include <fcntl.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <size_t.h>
#include <stat.h>
#include <stdarg.h>
#include <time.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <GestaltEqu.h>
#include <Files.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <types.h>
#include <pascal.h>
#include <Events.h>
#include <EPPC.h>
#include <AppleEvents.h>
#include <OSEvents.h>


#include "Apple.h"
#include "Help.h"

#ifdef modus_mcp

extern char console_environment;	// Definiert in Console.c, Einzige Änderung in Console.c: diese Var global statt static!!!
					// Daher Custom ANSI 6.0.0 Console…
int	argc=0;
static 	Boolean gDone = FALSE;
static Str255	argv[maxFiles];

static unsigned short Modis = 0;



/*void	InitToolbox(void)		// Standard initialization calls
*{
*	InitGraf(&thePort);
*	InitFonts();
*	InitWindows();
*	InitMenus();
*	TEInit();
*	InitDialogs(nil);
*	InitCursor();
*	MaxApplZone();			// Aus Handbuch
*	FlushEvents (everyEvent,0);
*}
*	Ersetzt durch ANSI InitConsole()!!!
*/


static void InitTBandCON(void)
{

	MenuHandle menu,appleMenu;
// static MenuHandle appleMenu;	in Console.c
	int i;

		/*  initialize the Memory Manager  */
	
	if (ROM85 >= 0)
		MaxApplZone();
	for (i = 0; i < 10; i++)
		MoreMasters();
	
		/*  initialize Quickdraw  */
	
	InitGraf(NewPtr(206) + 202);
	
		/*  initialize the Toolbox  */
		
	InitFonts();
	InitWindows();
	TEInit();
	InitDialogs(0);
	InitMenus();
	
		/*  create menus  */
		
	InsertMenu(appleMenu = NewMenu(1, "\p\024"), 0);
	AddResMenu(appleMenu, 'DRVR');
	InsertMenu(menu = NewMenu(2, "\pFile"), 0);
	AppendMenu(menu, "\pQuit/Q");
	InsertMenu(menu = NewMenu(3, "\pEdit"), 0);
	AppendMenu(menu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	DrawMenuBar();
	
		/*  ready to receive events  */
		
	FlushEvents(everyEvent, 0);
	InitCursor();
	console_environment = 1;
}


Boolean AppleEventsInstalled ()
{
	OSErr err;
	long  result;

	// THINK C's MacTraps library provides glue for Gestalt, so
	// it can be called safely under System 6. If an error is
	// returned, then Gestalt for the AppleEvents Selector is
	// not available (this also means that Apple Events are
	// not available)
	err = Gestalt (gestaltAppleEventsAttr, &result);
	return (!err && ((result >> gestaltAppleEventsPresent) & 0x0001));
						// return TRUE if there is n											// error and the proper bit of
						// result is set
}

OSErr	MyGotRequiredParams (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size	actualSize;
	OSErr	myErr;

	myErr = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr,
					typeWildCard, &returnedType,
					nil, 0, &actualSize);

	if (myErr == errAEDescNotFound)	// you got all the required
											//parameters
		return	noErr;
	else
		if (myErr == noErr)  // you missed a required parameter
			return	errAEParamMissed;
		else	// the call to AEGetAttributePtr failed
			return	myErr;
}


pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long handlerRefCon)
{
	
	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err;
	long		index,itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	FILE 		*f;
        StringHandle	pathHandle;
        short		itemHit=0,
        		count=0;
        
//printf("ODOC invoked…\n");
        
	// get the direct parameter--a descriptor list--and put it into a docList
	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList,
			&docList);
	if (err)
			return err;

	// check for missing parameters
	err = MyGotRequiredParams (theAppleEvent);
	if (err)
			return err;

	// count the number of descriptor records in the list
	err = AECountItems (&docList, &itemsInList);
	
	// allocate for pathname with Toolbox-calls
	pathHandle = (StringHandle) NewHandle(sizeof(Str255));
	HLock((Handle) pathHandle);
			
			
	// now get each descriptor record from the list, coerce the returned
	// data to an FSSpec record, and open the associated file
	for (index = 1; index <= itemsInList; index++) {

			err = AEGetNthPtr (&docList, index, typeFSS, &keywd,
							&returnedType, (Ptr) &myFSS, sizeof(myFSS),&actualSize);
			if (err)
					return err;
			PtoCstr(myFSS.name);
			// assemble full pathname to the current directory...
			GetFullPath(myFSS.parID,myFSS.vRefNum,*pathHandle);
			// ...and add the file's name
			strcat((char *) *pathHandle,(const char *) myFSS.name);
			testlength((char *)  *pathHandle);
			// test if file can be opened
			if(!(f = fopen((const char *) *pathHandle,"a"))){
//				InitToolbox();
				do      // and warn with an alert if necessary
				{
//					itemHit = Alert(rCannotOpenFile, nil);
					itemHit = CautionAlert(rCannotOpenFile, nil);
//					Identisch bis auf, nur wird kein Icon Ausgegeben, s. THINK Ref., Amoroso, 8.9.93
				} while(itemHit != dOKButton);
        	                GenExit(1);
                        }
                        // put pathname into an argv-array	
			strcpy((char *) argv[count], (const char *) *pathHandle);
			argc = ++count;
			fclose(f);
	}
	HUnlock((Handle) pathHandle);
	DisposeHandle((Handle) pathHandle);
	gDone = TRUE;
	err = AEDisposeDesc (&docList);
//printf("argc in odoc: %i",argc);
//printf("PRG in odoc: %s\n",*(argv));
//printf("CON in odoc: %s\n",*(argv+1));
//printf("DAT in odoc: %s\n",*(argv+2));
//printf("MyHandleOdoc korrekt verlassen!!");
}


void GetFullPath(long DirID, short vRefNum, StringPtr fullPath)
{
	CInfoPBRec	myPB;	
	DirInfo		*dpb = (DirInfo *) &myPB;
	Str255		dirName;
// Ergänzung Amoroso ANFANG	
	char 		*HelpdirName;
	int		error = 0;
// Ergänzung Amoroso ENDE	
	OSErr		myErr;
	
	// initialize dpb
	dpb->ioNamePtr = dirName;
	dpb->ioVRefNum = vRefNum;
	dpb->ioDrParID = DirID;
	dpb->ioFDirIndex = -1;
	fullPath[0] = '\0';
	
// Ergänzung Amoroso ANFANG
	error = (NULL == (HelpdirName = (char *) calloc (256,sizeof(char))));
	if (!error)
	do
	{
		HelpdirName = (char *)dirName;
		CtoPstr(HelpdirName);		
//	do
//	{
//		CtoPstr(dirName);
//NICHT SAUBER, DA CtoPstr eigentlich (char *) erwartet, dirName aber unsigned char[] ist!
//DAHER: ERGÄNZUNGEN...
// Ergänzung Amoroso ENDE	
		dpb->ioDrDirID = dpb->ioDrParID;
// Ergänzung Amoroso ANFANG 2		
//NICHT SAUBER, DA PBGetCatInfo eigentlich (CInfoPBRec *) erwartet, dpb aber (DirInfo *) ist!
//DAHER: ERGÄNZUNGEN 2...
		myErr = PBGetCatInfo(&myPB,FALSE);
//		myErr = PBGetCatInfo(dpb,FALSE);
// Ergänzung Amoroso ENDE 2	
		PtoCstr(dirName);
		strcat((char *) dirName,":");
		strcat((char *) dirName, (const char *) fullPath);
		
		strcpy((char *) fullPath,(const char *) dirName);
		testlength((char*) fullPath);	
	}while(dpb->ioDrDirID != fsRtDirID);
}			
				 

void DoError(OSErr myErr)
{
	gDone = TRUE;
}


pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon)
{
	short	itemHit=0;
//printf("PDOC invoked…\n");
//GenOutAcknowledge(ModusReturn);
	
	// StopAlert if PDoc-Event was received 
//	InitToolbox();
	do
	{
//		itemHit = Alert(rNoPDocAlert, nil);
		itemHit = StopAlert(rNoPDocAlert, nil);
//		Identisch bis auf, nur wird kein Icon Ausgegeben, s. THINK Ref., Amoroso, 8.9.93
	} while(itemHit != dOKButton);
	GenExit(2);
}

// QuitApplicationEvent received

pascal OSErr  MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon)
{		
//printf("QAPP invoked…\n");
	// set exit-condition true if QuitApp-Event received
	gDone = TRUE;
	return noErr;
}

// OpenApplicationEvent received

/*pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon)
{	
//printf("OAPP1 invoked…\n");
	argc = 0;
	return noErr;
}*/
// Ersetzt durch Version von MyHandleOApp die Alert ausgibt, Amoroso, 8.9.93

pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefCon)
{
	short	itemHit=0;
	
	// StopAlert if PDoc-Event was received 
//printf("OAPP2 invoked…\n");

//	InitToolbox();
	do
	{
//		itemHit = Alert(rNoOAppAlert, nil);
		itemHit = StopAlert(rNoOAppAlert, nil);
//		Identisch bis auf, nur wird kein Icon Ausgegeben, s. THINK Ref., Amoroso, 8.9.93
	} while(itemHit != dOKButton);
	GenExit(0);
}


char **GetProgParams(int *pargc)
{
	Boolean		aEvents=FALSE,
			eventThere=FALSE;
	EventRecord	theEvent;
	OSErr		myErr=0;
	int 		i=0,
			error=0;
	char		**ppargv;
	char 		*ModNam;
	FILE		*fp;
	
#ifdef option_smartparams
	char		*helpArgv[1];
        int		dat_num = 0,
        		con_num = 0;
        FILE 		*fs = NULL;
#endif //ifdef option_smartparams
	
	InitTBandCON();
// 	//allocate pointer-array for *argvs
	ppargv = (char **) NewPtr((maxFiles)*sizeof(char *));
//	// first parameter is the program's name -> get it!	
	*ppargv = (char *) NewPtr(MaxFullPathName + 5 + 6 + 1);		//32 char, 0 oder length, Trennzeichen, 4 char Creator, Trennzeichen, max. 5 char Modifiers
	GetProgName(*ppargv);
	// next parameters are the files' full pathnames they are set below...
	for(i=1;i<maxFiles;i++)
	{
//		*(ppargv+i) = (char *) NewPtr(strlen((const char *) argv[i-1])+1); Nicht am falschen Ende sparen!
		*(ppargv+i) = (char *) NewPtr(256);
		strcpy(*(ppargv+i),"\0");
	}

	if(!(ModNam = (char *)calloc((MaxFileName + 6),sizeof(char))))
		GenExit(4);
	strcpy(ModNam, *ppargv);
	CtoPstr(ModNam);
	*ModNam = *ModNam - 5;
//	Creator und ":" NICHT ausgeben!
	ParamText((unsigned char *) ModNam, 0, 0, 0);
	aEvents = AppleEventsInstalled();
//printf("aEvents: %i\n",aEvents);

	if (aEvents) {
//printf("Entering Block after if (aEvents) {…\n");
//printf("Trying to install odoc\n");
                // Install Apple-Event-Handlers
		myErr = AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,&MyHandleODoc,0, FALSE);
		if(myErr) {
//printf("Installation of odoc failed\n");
			DoError(myErr);
		}
//printf("Trying to install oapp\n");
		myErr = AEInstallEventHandler (kCoreEventClass, kAEOpenApplication,MyHandleOApp,0, FALSE);
		if(myErr) {
//printf("Installation of oapp failed\n");
			DoError(myErr);
		}
//printf("Trying to install pdoc\n");
		myErr = AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments,&MyHandlePDoc,0, FALSE);
		if(myErr) {
//printf("Installation of pdoc failed\n");
			DoError(myErr);
		}
//printf("Trying to install qapp\n");
		myErr = AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,&MyHandleQuit,0, FALSE);
		if(myErr) {
//printf("Installation of qapp failed\n");
			DoError(myErr);
		}		
//FlushEvents (everyEvent,kHighLevelEvent); Weg wg. Modifierbestimmung!
//printf("Just survived FlusheEvents\n");
//error = (!(fp = fopen("BS CO.log","w")));
//InitConsole(); Besser am Anfang und Lokal!!! vgl.: InitTBandCON
//if (error)
//	GenExit(5);
		do{
			eventThere = EventAvail ( everyEvent, &theEvent );
			Modis = theEvent.modifiers;
//			TestStartMods(Modis);
			GetStartMods(*ppargv, Modis);
			eventThere = EventAvail ( highLevelEventMask, &theEvent );
//printf("Just survived EventAvail, returned: %i\n",eventThere);
//fprintf(fp,"theEvent.what= %i\n",theEvent.what);
//Wenn alle Dateioperationen hier rausfallen und das folgende printf gemacht wird geht es auch mit Tasten!!!
//printf("theEvent.what= %i\n",theEvent.what);
		}while(!eventThere);
//fprintf(fp,"Schleife verlassen, jetzt kommt GetNextEvent()…");
		GetNextEvent ( highLevelEventMask, &theEvent );
//printf("Just survived GetNextEvent, returned: %i\n",eventThere);
//fprintf(fp,"Just survived GetNextEvent, returned: %i\n",eventThere);
		if (theEvent.what == kHighLevelEvent){
//printf("Trying to invoke AEProcessAppleEvent…\n");
//fprintf(fp,"Trying to invoke AEProcessAppleEvent…\n");
			myErr = AEProcessAppleEvent (&theEvent);
//printf("Invoked AEProcessAppleEvent, returned: hi:%hi \n",myErr);
//fprintf(fp,"Invoked AEProcessAppleEvent, returned: hi:%hi \n",myErr);
			if(myErr){
//printf("Invoking DoError…\n");
				DoError(myErr);	
			}
		}
//		GetStartMods(*ppargv, theEvent.modifiers); Hier zu spät, da nur HighLevelEvent durchkommt, Mods = 0!
	}	
	// next parameters are the files' full pathnames
	for(i=1;i<argc + 1;i++)
	{
//		*(ppargv+i) = (char *) NewPtr(strlen((const char *) argv[i-1])+1); Hier nicht mehr nötig!!
		strcpy(*(ppargv+i),(const char *) argv[i-1]);
	}
	
	// close with nil-Pointer and increase argc
//	*(ppargv+argc+1) = nil;
//	Feste Array-Länge, nicht nötig!!
	*pargc = ++argc;
//printf("argc in GetProgParams: %i",argc);
//printf("PRG in GetProgParams: %s\n",*(argv));
//printf("CON in GetProgParams: %s\n",*(argv+1));
//printf("DAT in GetProgParams: %s\n",*(argv+2));
//printf("GetProgParams korrekt verlassen!!");
//fclose(fp);
#ifdef option_smartparams
	for(i=1;i<argc;i++){
		if (**(ppargv+i) != 0)
			error = (NULL == (fs = fopen(*(ppargv+i),"r")));
		if (fs != NULL){
			if (!error){
//printf("\nGetProgParams!");
//printf("\n Suche in argv[%i]:\n %.255s\n",i,*(ppargv+i));
//printf("\nError vor 1. Aufr. checkType (Index %i): %i",i,error);			
//GenOutAcknowledge(ModusReturn);
				error = checkType(fs, "&Type=Control", &con_num, i);
			}
			else 
				GenExit(99);
//printf("\ncon_num: %i",con_num);			
			if (!error)
				error = fseek(fs, 0L, SEEK_SET);
			if (!error){
//printf("\nGetProgParams!");
//printf("\n Suche in argv[%i]:\n %.255s\n",i,*(ppargv+i));
//printf("\nError vor 2. Aufr. checkType (Index %i): %i",i,error);			
//GenOutAcknowledge(ModusReturn);
				error = checkType(fs, "&Type=Data", &dat_num, i);
			}
			if (fs)
				fclose(fs);
			fs = NULL;
		}
	}
//printf("\n\nCon/Dat: %i, %i\n\n",con_num,dat_num);
//for(i=1;i<argc;i++)
//printf("\nargv[%i]: %s",i,*(ppargv+i));
	if ((con_num > 2) && (dat_num > 2)){
		error = swapArgv(*(ppargv+1), *(ppargv+con_num));
		if(error)
			GenExit(100);
		error = swapArgv(*(ppargv+2), *(ppargv+dat_num));
		if(error)
			GenExit(100);
	}	
	else if ((bsintmin(con_num,dat_num) < 3) && (bsintmax(con_num,dat_num) > 2)){	
		if (bsintmin(con_num,dat_num) == con_num){
			error = swapArgv(*(ppargv+1), *(ppargv+con_num));
			if(error)
				GenExit(100);
			error = swapArgv(*(ppargv+2), *(ppargv+dat_num));
			if(error)
				GenExit(100);
		}
		else if (bsintmin(con_num,dat_num) == dat_num){
			error = swapArgv(*(ppargv+2), *(ppargv+dat_num));
			if(error)
				GenExit(100);
			error = swapArgv(*(ppargv+1), *(ppargv+con_num));
			if(error)
				GenExit(100);
		}
	}	
	else if	(((con_num * dat_num) < 3) && con_num > dat_num){	
			error = swapArgv(*(ppargv+1), *(ppargv+2));
	}
//for(i=1;i<argc;i++)
//printf("\nargv[%i]: %s",i,*(ppargv+i));
//GenOutAcknowledge(ModusReturn);

#endif //ifdef option_smartparams
		
	return ppargv;

}

int GetProgName(char *Argv1){

	ProcessInfoRec 		myInfoRec;
	ProcessSerialNumber 	myPSN;
	FSSpec 			myFSSpec;
	OSErr 			OSError = 0;
	int 			i = 0,
				error = 0,
				StrLength = 0;
	char 			helpStr[MaxFileName + 6], helpStrPtr,
				*TypePtr, *StrPtr;	
	myInfoRec.processInfoLength = (unsigned long)sizeof(ProcessInfoRec);
	myInfoRec.processName = (unsigned char *) calloc (sizeof (unsigned char),MaxFullPathName +5);
	myInfoRec.processAppSpec = &myFSSpec;
	OSError = GetCurrentProcess(&myPSN);
	if(!OSError)
		OSError = GetProcessInformation(&myPSN, &myInfoRec);
	if(!OSError){
		strcpy(Argv1,PtoCstr(myInfoRec.processName));
		strcat(Argv1,":\0");
// N.B.: ":" Darf auf dem Mac nicht für Dateinamen verwendet werden!!!
		StrLength = (int) strlen(Argv1);
		StrPtr = (Argv1 + StrLength);
		TypePtr = (char *) &myInfoRec.processSignature;
		for (i=0;i<4;i++)
			*(StrPtr++) = *(TypePtr++);
		*StrPtr = 0;	
	}
	return (error | (int)OSError);
}

int GetStartMods(char *Argv1, unsigned short mods)	
{
	FILE *Nptr = NULL;
static  int count = 1;	
	int i = 0;
	unsigned short testshort = 1;
	char Num[5] ="";
	char Name[33] = "";
	
	sprintf(Num, "%i", count);
	strcat(Name,"GModifiers");
	strcat(Name,(const char *)Num);
	strcat(Name,".txt");
//	Nptr = fopen("ModifiersApple.txt","w");
//	fprintf(Nptr, "Short Modifiers: %i\nBinär: ", (int) mods );
//	for (i=1; i <= 16; i++){
//		fprintf(Nptr, "mods & testshort: %i\n mods: %i\n testshort: %i\n", (int)(mods & testshort), mods, testshort);
//		fprintf(Nptr, "%i", (int)((mods & testshort) != (unsigned short) 0));
//		testshort = testshort << 1;
//	}
	strcat(Argv1,":\0");
// N.B.: ":" Darf auf dem Mac nicht für Dateinamen verwendet werden!!!
	if(mods & (unsigned short)cmdKey)
		strcat(Argv1,"c\0");
	if(mods & (unsigned short)shiftKey)
		strcat(Argv1,"s\0");
	if(mods & (unsigned short)alphaLock)
		strcat(Argv1,"a\0");
	if(mods & (unsigned short)optionKey)
		strcat(Argv1,"o\0");
	if(mods & (unsigned short)controlKey)
		strcat(Argv1,"k\0");		
// String with ":modifiers csaok" if pressed while startup
// Control,Shift,Alpha Lock,Option,KControl...
//	fprintf(Nptr, "\nARGV in GetStartMods: %s", Argv1);
//	fclose(Nptr);
	return(0);
}

int TestStartMods(unsigned short mods)	
{
	FILE *Nptr = NULL;
static  int count = 1;	
	int i = 0;
	unsigned short testshort = 1;
	char Num[5] ="";
	char Name[33] = "";
	
	sprintf(Num, "%i", count);
	strcat(Name,"GModifiers");
	strcat(Name,(const char *)Num);
	strcat(Name,".txt");
	Nptr = fopen((const char *)Name,"w");
	fprintf(Nptr, "Short Modifiers: %i\nIn Bits: ", (int) mods );
	for (i=1; i <= 16; i++){
		fprintf(Nptr, "%i", (int)(mods & testshort));
		testshort << 1;
	}
	fclose(Nptr);
	return(0);
}								
								


Boolean GetQuitReq(int tick)
{
	EventRecord 	theEvent;
	OSErr		myErr;
	
	// check Event-Queue
	if( WaitNextEvent ( everyEvent, &theEvent, tick, nil ) ) {
				switch (theEvent.what) {
				
				case kHighLevelEvent:
						
						myErr = AEProcessAppleEvent (&theEvent);
						if(myErr)
							DoError(myErr);										
						if(gDone)
							return TRUE;			
						else 	return FALSE;
							
				default:
						return FALSE;
						}
				}
}

void testlength(char *path)
{
	short itemHit = 0;
	
	if (strlen(path) > 255){
//		InitToolbox();
		do
		{
//			itemHit = Alert(rPathTooLong,nil);
			itemHit = StopAlert(rPathTooLong,nil);
//			Identisch bis auf, nur wird kein Icon Ausgegeben, s. THINK Ref., Amoroso, 8.9.93
		}while(itemHit != dOKButton);
		GenExit(0);
	}
}


int GetFileInfo(char * FName, unsigned long *Type, unsigned long *Creator){
	
	FInfo	myFInfo;
	Str255	myFName, *myFNamePtr;
	int	error = 0;
	OSErr	OSError = 0;
	
	myFNamePtr = &myFName;
	error = (255 < (int)strlen(FName));
	if(!error)
		strcpy((char *)myFNamePtr,FName);
	if(!error){
		CtoPstr((char *)myFName);
		OSError = HGetFInfo(0, 0, myFName, &myFInfo);
		if(!OSError){
			*Type = (unsigned long) myFInfo.fdType;
			*Creator = (unsigned long) myFInfo.fdCreator;
		}
	}
	return (error | (int) OSError);
}

int SetFileInfo(char * FName, unsigned long Type, unsigned long Creator){
	
	FInfo	myFInfo;
	Str255	myFName, *myFNamePtr;
//	myNewName, *myNewNamePtr;
	int	error = 0;
	OSErr	OSError = 0;
	
	myFNamePtr = &myFName;
	error = (255 < (int)strlen(FName));
	if(!error)
		strcpy((char *)myFNamePtr,FName);
	if(!error){
		CtoPstr((char *)myFName);
		OSError = HGetFInfo(0, 0, myFName, &myFInfo);
		if(!OSError){
			myFInfo.fdType = (OSType)Type;
			myFInfo.fdCreator = (OSType)Creator;
			OSError = HSetFInfo(0, 0, myFName, &myFInfo);
		}
	}
	return (error | (int) OSError);
}

#ifdef option_smartparams
int checkType(FILE *fp, char *keyWord, int *result, int count)
{
	char	*word = NULL,
		*word_ptr = NULL,
		*char_ptr = NULL,
		*UpperKeyWord = NULL,
  		c = 0;
  	int 	compareLength = 0,
  		found = 0,
  		i = 0,
  		error = 0;
  	long 	fp_pos = 0,
		fphelp = 0;

	
//printf("\ncheckType!");
//printf("\nError: %i, Result: %i, Count: %i",error,*result,count);
	
	compareLength = (int)strlen(keyWord);
//printf("\nComparelength: %i", compareLength);
	if(!*result)
		error = (NULL == (UpperKeyWord = (char *) calloc((size_t)(1 + compareLength),sizeof(char))));
//printf("\n!Error && !Result: %i",(!error && !*result));
	if(!*result && !error)
		error = (NULL == (word = (char *) calloc((size_t)(1 + compareLength),sizeof(char))));
	else
		return(error);
//printf("\nPointer initialisiert: %p, %p", UpperKeyWord, word);
	if (!*result && !error){
		strcpy(UpperKeyWord,(const char *)keyWord);
		word_ptr = UpperKeyWord;
		char_ptr = keyWord;
		if (*char_ptr != 0) 
			do{
				c = *char_ptr;
//printf("\nchar_ptr: %c",c);
				char_ptr++;	
				if(isgraph(c))   			
					*word_ptr = toupper(c);
//printf("\nword_ptr: %c",toupper(c));
				word_ptr++;
//GenOutAcknowledge(ModusReturn);							
			}while (c);
//printf("\nSuche nach Keyword: %s bzw.: %s",keyWord,UpperKeyWord);					
		word_ptr = word;
		while(!found){
			do{
				c = fgetc(fp);
			}while((c != '&') && (c != EOF));			//sucht & in der Datei
//printf("\nChar %i gefunden, gesucht %i",c,'&');
//printf("\nDateizeiger = %i",ftell(fp));
//GenOutAcknowledge(ModusReturn);						
			if(c == EOF){
				free(word);
				free(UpperKeyWord);
				return(error);
			}
			fphelp = ftell(fp);				//3 Zeilen eingefügt um Dateizeiger wieder vor gefundenes & zu bringen
			fphelp--;
			fseek(fp,fphelp,SEEK_SET);
			fp_pos = ftell(fp);				//fpos wird zugewiesen bevor der Vergleich beginnt!
	   		i = 0;
			while((i < compareLength) && (c != ';')){  	//kopiert compareLength Zeichen in strin word
				c = fgetc(fp);
				if(c == EOF){
					free(word);
					free(UpperKeyWord);
					return(error);
				}
				if(isgraph(c)){   			//isgraph(' ') = 0!!!!!
					*word_ptr = toupper(c);
					word_ptr++;
					i++;
				}
			}
	    	*word_ptr = '\0';					// string in word wird abgeschlossen
//printf("\nEingelesenes Wort: %s",word);					
//printf("\nDateizeiger = %i",ftell(fp));
//GenOutAcknowledge(ModusReturn);						
		if(!strncmp(word,UpperKeyWord,compareLength)){		// Identität => strncmp = 0!
			
			found = 1;					// gefunden
			*result = count;				// Index vermerken
//printf("\nÄndere Result: %i ",*result);			
		}
		else
			word_ptr = word;				// Übertragungszeiger wieder auf Anfang für word reservierten Bereich
		} // while !found 					// solange bis gefunden!
	free(word);
	free(UpperKeyWord);
	}
	else 
		if(UpperKeyWord != NULL)
			free(UpperKeyWord);
//printf("\nRückgabewert: %i",error);
	return(error);
}

int swapArgv(char *argFrom, char *argTo)				// Swap two argvs
{
	char HelpStr[MaxFullPathName];
	
	strcpy(HelpStr, (const char *)argTo);
	strcpy(argTo, (const char *)argFrom);
	strcpy(argFrom, (const char *)HelpStr);
	return 0;
} 
#endif //ifdef option_smartparams
#endif //ifdef modus_mcp

#endif //ifdef apple

#endif //ifndef bsoc