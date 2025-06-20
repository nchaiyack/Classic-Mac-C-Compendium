/* 
TestGDVideo.c by Denis G. Pelli
Tests all the documented Control and Status calls implemented in GDVideo.c
The calls are documented in Designing Cards and Drivers, 3rd Edition.

The main purpose of this program is to find bugs in video drivers. If you do
find a bug that isn't already listed in the table at the end of �Video synch�
then please send it to denis_pelli@isr.syr.edu 

Unfortunately, the most common way to discover a new bug is for this program to
crash. Please report crashes to denis_pelli@isr.syr.edu

BUGS:

HISTORY:
8/11/89	dgp	wrote it
3/23/90	dgp	introduced special case conditionals to skip tests known to crash
			particular drivers.
3/23/90	dgp	made it compatible with MPW C, as a tool.
3/24/90	dgp	made some of the printouts less cryptic.
3/31/90	dgp	introduced RestoreDeviceClut after SetMode call.
7/12/90	dgp	tidied up the initial printf
7/19/90	dgp	Renamed VideoTest.c 
			Open & close a window on each screen and DrawMenuBar()
			so that QuickDraw will restore each screen.
7/28/90	dgp	italicized name of book.
9/9/90	dgp	Added quit by Command-. for graceful exit from bad situations.
10/17/90 dgp Removed unused variables. Replaced TRUE & FALSE by 1 & 0.
10/20/90	dgp	Apple has renamed the control and status calls, so I followed suit:
				GDGetPageBase replaces GDGetBaseAddr
				GDReset replaces GDInit
				GDGrayPage replace GDGrayScreen
				GDGetPageCnt replaces GDGetPages
10/22/90 dgp Print out number of pages for all modes, not just current mode.
		 dgp Restore clut manually since RestoreDeviceClut() doesn't
		 	work on NuVista.
10/28/90 dgp Only test GetInterrupt & SetInterrupt if System is 6.05 or higher. This
			avoids SetInterrupt() crash on old Apple TFB video driver.
2/12/91	dgp	Preceded GDGrayPage() call by GDSetMode() to cure crash when running
			program immediately after rebooting. Mysterious.
			Added SelectWindow() at end of program.
3/9/91	dgp	Print out mode names.
4/15/91	dgp	Make sure that new palette manager exists before calling RestoreDeviceClut.
			Thanks to Chuck Stein for alerting me to the bug.
6/6/91	dgp	Time and print out how long GDSetEntries takes.
8/24/91	dgp	Made compatible with THINK C 5.0.
1/19/92	dgp	Cosmetic improvements to the printout.
2/25/92	dgp	Put timing info in bold.
8/22/92	dgp Display results of calling GDFrameRate() and GDFramesPerClutUpdate().
8/27/92	dgp	replace SysEnvirons() by Gestalt()
12/17/92 dgp enhanced to work with any dac size.
12/30/92 dgp Use GDClutSize() to determine clut size.
1/6/93	dgp	Tidied up the GetEntries/SetEntries code, now that we don't have
			to test for the Mac IIci driver.
			Skip GetEntries on Relax driver.
1/7/93 dgp	1.1 First numbered version.
1/18/93	dgp	Renamed ModeName() to GDModeName().
1/18/93	dgp	Moved the code that checked for bad drivers into GDGetEntries() in GDVideo.c,
			where it belongs.
2/1/93	dgp Report whether driver is RAM- or ROM-based.
2/5/93	dgp	1.2. Recompiled with latest version of GDVideo, which now supports
			patching of Mac IIci ROM-based driver.
2/6/93	dgp	1.3. Report ROM version, if driver is ROM-based.
2/7/93	dgp	1.4. Recompiled after fixing endless loop in PatchMacIIciVideoDriver
			in GDVideo.c
2/18/93	dgp	1.5 Made compatible with PowerBook 160, by omitting call to GDSetInterrupt.
2/22/93	dgp	1.6 No longer assume that driver uses a version 0 gamma table.
4/17/93	dgp	1.7 Replaced obsolete GDFramesPerClutUpdate and GDTimeClutUpdate by 
			GDTimeClut.
7/7/93	dgp	1.8 A bit more error checking of GDGetGamma.
*/

#define GAMMA_TABLE	1 /* TRUE or FALSE. Printout of gamma table. */
#define VERSION "1.8"
void GammaReport(GammaTbl *gamma);

#include "VideoToolbox.h"
#include <assert.h>
#include <Files.h>
#if THINK_C
	#include <LoMem.h>
#else
	short MBarHeight : 0xBAA;
#endif
#include <Menus.h>

int PatchMacIIciVideoDriver(void);
#define dRAMBased		0x0040

#define SCREENS 8

void main(void);
void Error(int error);

EventRecord theEvent;

void main()
{
	static WindowPtr window[SCREENS],oldPort;
	static ColorSpec myTable[1024];	// Enough for 10-bit dacs
	ColorSpec black={0,0,0,0};
	WindowPtr theWindow;
	GDHandle device;
	short i,j,error,screen,mode,page,pages,start,count,pixelSize,type,clutSize;
	short textSize=12,dacSize,dataSize,dataCnt,fontNumber,where;
	short gammaDataWidth;
	long systemVersion=0;
	Boolean flag,keepWaiting;
	GammaTbl *gamma=NULL,*myGamma=NULL;
	char *name;
	Ptr baseAddr;
	Rect myRect;
	double f;
	unsigned char *byte;
	unsigned short *word;
	Boolean patch;
	char typeName[][16]={"clutType","fixedType","directType"};
	AuxDCE **auxDCEHandle;
	long size;
	Ptr ptr;
	long romSize,romVersion;
	double s,frames,missingFrames,frameRate;
	
	StackGrow(20000);
	Require(gestalt8BitQD);
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitCursor();
	GetPort(&oldPort);
	_atexit(RestoreCluts);
	patch=PatchMacIIciVideoDriver();
	for(screen=0;screen<SCREENS;screen++){
		SetPort(oldPort);
		device = GetScreenDevice(screen);
		if(device == NULL) break;

		/*
		For reasons that I don't understand, calling GDGrayPage() first,
		immediately after rebooting, results in a crash when using an Apple's
		old "Toby" video card. Preceding the call by
		a call to GDSetMode() cures the symptom. Later calls to GDGrayPage()
		seem to always work fine. Presumably something is not properly initialized
		in the Apple video card driver. (Toby card driver version 4.)
		*/
		error=GDGetMode(device,&mode,&page,&baseAddr);
		error=GDSetMode(device,mode,page,&baseAddr);
		GDRestoreDeviceClut(device);
		
		error=GDGrayPage(device,page);
		if(screen==0)DrawMenuBar();	/* restore menu bar after GrayScreen */

		/* Open window */
		myRect = (*device)->gdRect;	/* rect of desired screen in global coordinates */
		if(screen==0)myRect.top+=MBarHeight;		/* allow for menu bar */
		myRect.top+=17;								/* allow for title bar */
		InsetRect(&myRect,1,1);
		name=GDCardName(device);
		CtoPstr(name);
		window[screen]=
			NewCWindow(NULL,&myRect,(unsigned char *)name,TRUE,documentProc,(WindowPtr) -1L,TRUE,123L);

		/* Write in window */
		SetPort(window[screen]);
		myRect=window[screen]->portRect;
		GetFNum("\pTimes",&fontNumber);
		TextFont(fontNumber);
		TextSize(textSize);
		DrawPrintf("\n");
		if(screen==0)DrawPrintf("Welcome to TestGDVideo " VERSION ". Now testing all your video drivers."
		" Please report any crash to denis_pelli@isr.syr.edu\n");

		DrawPrintf("Driver: %#s version %d",GDName(device),GDVersion(device));
		auxDCEHandle = (AuxDCE **) GetDCtlEntry((*device)->gdRefNum);
		if((**auxDCEHandle).dCtlFlags & dRAMBased){
			/* RAM-based driver. */
			size=GetHandleSize((Handle)(**auxDCEHandle).dCtlDriver);
			DrawPrintf(", occupying %ld bytes in RAM.",size);
		}else{
			/* ROM-based driver. */
			ptr=(**auxDCEHandle).dCtlDriver;
			Gestalt(gestaltROMSize,&romSize);
			Gestalt(gestaltROMVersion,&romVersion);
			DrawPrintf(", at 0x%lx in ROM. %ld K ROM version %ld (%ld)."
				,ptr,romSize/1024,romVersion%256,(romVersion/256));
		}
		DrawPrintf("\n");
		if(GetDeviceSlot(device)>=0)DrawPrintf("Slot: %d\n",GetDeviceSlot(device));
		else DrawPrintf("Slot: none\n");
	
		DrawPrintf("GrayPage: ");
		if(error) Error(error);
		else DrawPrintf("ok\n");

		DrawPrintf("GetMode: ");
		error=GDGetMode(device,&mode,&page,&baseAddr);
		if(error) Error(error);
		else DrawPrintf("%d-bit mode, page %d base address 0x%lX\n",GDPixelSize(device),page,baseAddr);
	
		DrawPrintf("SetMode: ");
		error=GDSetMode(device,mode,page,&baseAddr);
		if(error) Error(error);
		else DrawPrintf("ok\n");

		/* 
			GDSetMode has the annoying side effect of 
			setting the clut to uniform gray, so let's restore the clut.
		*/
		GDRestoreDeviceClut(device);

		for(i=0x80;i<=0x85;i++){
			pixelSize=GDModePixelSize(device,i);
			if(pixelSize>0){
				DrawPrintf("GetPageCnt: ");
				error=GDGetPageCnt(device,i,&pages);
				if(error)DrawPrintf("%d-bit mode n/a\n",pixelSize);
				else DrawPrintf("%d-bit mode has %d pages available\n",pixelSize,pages);
			}
		}

		DrawPrintf("GetPageBase: ");
		page=0;	/* Ask for first page */
		error=GDGetPageBase(device,page,&baseAddr);
		if(error) Error(error);
		else DrawPrintf("%d-bit mode, page %d base address 0x%lX\n"
			,GDModePixelSize(device,mode),page,baseAddr);
		if(pages>1){
			DrawPrintf("GetPageBase: ");
			page=1;	/* Ask for second page */
			error=GDGetPageBase(device,page,&baseAddr);
			if(error) Error(error);
			else DrawPrintf("%d-bit mode, page %d base address 0x%lX\n"
				,GDModePixelSize(device,mode),page,baseAddr);
		}
	
		DrawPrintf("GetGray: ");
		error=GDGetGray(device,&flag);
		if(error) Error(error);
		else DrawPrintf("flag %d\n",flag);
	
		DrawPrintf("SetGray: ");
		flag=0;
		error=GDSetGray(device,flag);
		if(error) Error(error);
		else DrawPrintf("ok\n");
	
		/* GetInterrupt crashes the NuVista driver.  */
		if(!EqualString("\p.Display_Video_NuVista",GDName(device),0,0)){
			/* SetInterrupt crashes the old Apple driver.  */
			if(systemVersion>=0x605){
				DrawPrintf("GetInterrupt: ");
				error=GDGetInterrupt(device,&flag);
				if(error) Error(error);
				else DrawPrintf("%d\n",flag);
		
				if(0){
					// crashes built-in video on PowerBook 160
					DrawPrintf("SetInterrupt: ");
					error=GDSetInterrupt(device,flag);
					if(error) Error(error);
					else DrawPrintf("ok\n");
				}
			}
			else DrawPrintf("�����Skipping GetInterrupt & SetInterrupt.\n");
		}
		else DrawPrintf("�����Skipping GetInterrupt & SetInterrupt to avoid known bug in %s.\n",GDCardName(device));

		DrawPrintf("GetGamma: ");
		gamma=NULL;
		error=GDGetGamma(device,&gamma);
		if(error) Error(error);
		else GammaReport(gamma);
		if(!error && gamma!=NULL)gammaDataWidth=gamma->gDataWidth;
		else gammaDataWidth=0;
		
		DrawPrintf("SetGamma: ");
		if(gamma==NULL || gamma->gDataWidth==0){
			// Get the dac's size from the current gamma table.
			if(gamma!=NULL && gamma->gDataWidth!=0){
				dacSize=gamma->gDataWidth;
				dataCnt=gamma->gDataCnt;
			}
			else{ // The driver won't tell us, so assume 8-bit dacs.
				dacSize=8;
				dataCnt=1L<<dacSize;
			}
			if(dacSize<=8)dataSize=1;
			else dataSize=2;
			myGamma=(GammaTbl *)NewPtr(sizeof(GammaTbl)+dataCnt*dataSize);
			assert(myGamma!=NULL);
			myGamma->gVersion=0;
			myGamma->gType=0;
			myGamma->gFormulaSize=0;
			myGamma->gChanCnt=1;
			myGamma->gDataCnt=dataCnt;
			myGamma->gDataWidth=dacSize;
			byte=(unsigned char *)myGamma->gFormulaData+myGamma->gFormulaSize;
			word=(unsigned short *)byte;
			if(myGamma->gDataWidth<=8)
				for(i=0;i<myGamma->gDataCnt;i++) byte[i]=i;
			else
				for(i=0;i<myGamma->gDataCnt;i++) word[i]=i;
			gamma=myGamma;
		}else myGamma=NULL;
		error=GDSetGamma(device,gamma);
		if(error) Error(error);
		else DrawPrintf("ok.\n");
		if(myGamma!=NULL){
			DisposPtr((Ptr)myGamma);
			myGamma=NULL;
		}

	DrawPrintf("GetGamma: ");
	gamma=NULL;
	error=GDGetGamma(device,&gamma);
	if(error) Error(error);
	else GammaReport(gamma);
	if(!error && gamma!=NULL)gammaDataWidth=gamma->gDataWidth;
	else gammaDataWidth=0;
	
		// GetEntries
		start=0;
		count=GDClutSize(device)-1;
		if(count>sizeof(myTable)/sizeof(myTable[0])-1)
			count=sizeof(myTable)/sizeof(myTable[0])-1;
		DrawPrintf("GetEntries: ");
		error=GDGetEntries(device,start,count,myTable);
		if(error)Error(error);
		else DrawPrintf("ok\n");
		
		// SetEntries
		count=GDClutSize(device)-1;
		switch((*device)->gdType){
			case clutType:
				DrawPrintf("SetEntries: ");
				error=GDSetEntries(device,0,count,((**(**(**device).gdPMap).pmTable)).ctTable);
				if(error)Error(error);
				else DrawPrintf("ok\n");
				break;
			case directType:
				DrawPrintf("DirectSetEntries: ");
				error=GDDirectSetEntries(device,0,0,&black);
				if(error)Error(error);
				else DrawPrintf("ok\n");
				break;
			case fixedType:
				DrawPrintf("A fixedType clut cannot be modified.\n");
				break;
		}
		
		// Miscellaneous
		clutSize=GDClutSize(device);
		pixelSize=1<<(mode&7);
		type=(*device)->gdType;
		DrawPrintf("Currently in mode %d, pixelSize %d bits, clutSize %d entries, %s"
			,mode,pixelSize,clutSize,typeName[type]);
		if(gammaDataWidth!=0)DrawPrintf(", dacSize %d bits",gammaDataWidth);
		DrawPrintf(".\n");

		TextFace(bold);
		f=GDFrameRate(device);
		DrawPrintf("%.1f Hz frame rate (%.1f ms).\n",f,1000.0/f);
		if((*device)->gdType!=fixedType){
			if((*device)->gdType==clutType)name="SetEntries";
			else name="DirectSetEntries";
			error=GDTimeClut(device,GDSetEntries,0
				,&s,&frames,&missingFrames,&frameRate);
			DrawPrintf("Repeatedly loading the clut, by calling %s(), "
				"takes %.2f frame each time.\n",name,frames);
			DrawPrintf("%.1f Hz clut update rate (%.1f ms).\n",1.0/s,1000.0*s);
		}
		TextFace(0);
		
		if(0){
			/* I haven't found any use for these calls, so skip 'em */
			DrawPrintf("GetDefaultMode: ");
			error=GDGetDefaultMode(device,&mode);
			if(error) Error(error);
			else DrawPrintf("0x%X\n",mode);
		
			DrawPrintf("SetDefaultMode: ");
			error=GDSetDefaultMode(device,mode);
			if(error) Error(error);
			else DrawPrintf("ok\n");
		}
		if(patch && EqualString("\p.Display_Video_Apple_RBV1",GDName(device),TRUE,FALSE))
			DrawPrintf(
			"This Mac IIci video driver has been patched (until reboot) to fix a bug\n"
			"that prevents reading the clut. The version number was changed from 0 to %d.\n"
			,GDVersion(device));
	}
	SelectWindow(window[0]);
	SetPort(window[0]);
	DrawPrintf(
		"This completes the test of the known Control and Status calls for each\n"
		"of your video devices. For an explanation of what these calls do, see\n"
		"Apple�s book ");
	TextFace(italic);
	DrawPrintf("Designing Cards and Drivers,");
	TextFace(0);
	DrawPrintf(" 3rd Ed., Addison Wesley, Chapter 9.\n");
	TextFace(bold);
	DrawPrintf("Quit by closing any window, or hitting Command-.\n");
	do{
		keepWaiting=1;
		while(!GetNextEvent(mDownMask+keyDownMask,&theEvent));
		switch(theEvent.what){
		case mouseDown:
			where=FindWindow(theEvent.where,&theWindow);
			switch(where){
			case inContent:
			case inDrag:
			case inGrow:
			case inGoAway:
				SelectWindow(theWindow);
			}
			if(where==inGoAway) keepWaiting = !TrackGoAway(theWindow,theEvent.where);
			break;
		case keyDown:
			if(theEvent.modifiers & cmdKey) switch(theEvent.message & charCodeMask) {
				case '.':
				case 'w':
				keepWaiting=0;
			}
			break;
		}
	} while (keepWaiting);
	SetPort(oldPort);
	for(i=0;i<screen;i++)DisposeWindow(window[i]);
	_exit(0);
}

void GammaReport(GammaTbl *gamma)
{
	short i,j,newFontNumber,fontNumber,textSize;
	unsigned char *byte;
	unsigned short *word;
	WindowPtr window;

	if(gamma==NULL)DrawPrintf("error--NULL gamma table pointer\n");
	else{
		DrawPrintf("gamma table @%lx indicates that the video card has %d-bit dacs, "
		"and %d entries in its clut.\n"
		,gamma,gamma->gDataWidth,gamma->gDataCnt);
		DrawPrintf("gVersion %d, gType %d, gFormulaSize %d, gChanCnt %d\n"
		,gamma->gVersion,gamma->gType,gamma->gFormulaSize,gamma->gChanCnt);
		if(GAMMA_TABLE && gamma->gVersion==0 && gamma->gType==0){
			GetFNum("\pMonaco",&newFontNumber);
			GetPort(&window);
			fontNumber=window->txFont;
			textSize=window->txSize;
			TextFont(newFontNumber);
			TextSize(9);
			byte=(unsigned char *)gamma->gFormulaData+gamma->gFormulaSize;
			word=(unsigned short *)byte;
			DrawPrintf("Gamma Table:\n");
			for(i=0;i<gamma->gDataCnt;i+=64) {
				DrawPrintf("%3d: ",i);
				if(gamma->gDataWidth<=8)
					for(j=0;j<64;j++) DrawPrintf(" %3u",byte[i+j]);
				else
					for(j=0;j<64;j++) DrawPrintf(" %3u",word[i+j]);
				DrawPrintf("\n");
			}
			TextFont(fontNumber);
			TextSize(textSize);
		}
	}
}

void Error(int error)
{
	switch(error){
	case -17:
	case -18:
		DrawPrintf("n/a\n");
		break;
	default:
		DrawPrintf("error %d\n",error);
	}
}