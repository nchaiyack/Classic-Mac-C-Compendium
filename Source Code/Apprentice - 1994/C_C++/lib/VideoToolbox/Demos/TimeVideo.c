/*
TimeVideo.c
Copyright © Denis G. Pelli, 1992, 1993

TimeVideo does a thorough test of the timing and synchronization of each video
screen, as well as the integrity of the clut hardware and software, and saves
the results in a self-explanatory text file, “TimeVideo results”. A discussion
of movies, lookup table animation, and synchronization appears in the
VideoToolbox “Video synch” file. The many video driver bugs uncovered by
TimeVideo and its predecessors are reported in that file. (For example, the
TrueVision NuVista driver assumes zero start in 16- and 32-bit modes.) Please
add your results by emailing your “TimeVideo results” to denis_pelli@isr.syr.edu.

Despite Apple’s rules that require it, a few video drivers don’t allow you to
read the clut (i.e. use GDGetEntries), or crash if you attempt it. GDGetEntries
(in GDVideo.c) checks against a list of known offenders and returns a statusErr
is the call is not usable. If we can’t use GDGetEntries then we fall back to a
visual test of the clut, comparing a particular way of loading the clut against
the standard way of loading the clut. Any visible change between these two
conditions suggests an error in the non-standard way of loading the clut.

BUGS:
Version 1.0 hung up on a PowerBook 170 with an Envisio video adapter. I didn’t 
have a chance to figure out why. It works fine on my ordinary PowerBook 170.

HISTORY:
8/23/92	dgp	wrote it, based on my TimeCPU.c
8/26/92	dgp print the driver version only if it's nonzero.
			added summary at end of printout.
9/9/92	dgp	changed printout of system vbl rate from %.1f to %.2f
			Added movie rate to measurements and printout.
9/15/92	dgp	made compatible with System 6.04. Added header to results file.
9/17/92	dgp	Added QUICKLY switch, to select CopyBits or CopyBitsQuickly.
10/5/92	dgp	cosmetic changes
10/6/92	dgp	report program version. Use frames/clut update as the criterion for summary.
10/9/92	dgp	Fixed summary. Too slow means MORE than one frame per clut update.
			Fixed restoration of clut in direct modes (i.e. 16 and 32 bit modes).
			Renamed to GDRestoreDeviceClut and put it in GDVideo.c.
10/13/92 dgp Tom Busey reported that frames were going uncounted
			during the clut timing, which seems to be a problem with some video drivers.
			TimeVideo now reports a frame count based solely on timing in secs and
			the separately measured frame rate, which seems to be a reliable.
			It also double checks the timing in secs vs frames, and if
			it finds a discrepancy, prints a warning to the screen & file. 
10/20/92 dgp Added VBL/frame to summary.
12/8/92	dgp Minor editing of comments.
12/9/92	dgp	1.01 Added _atexit(RestoreCluts) in case user quits prematurely.
12/11/92 dgp 1.02 GDRestoreBlackAndWhite allocates stack space for linearTable
			only if we need it.
12/30/92 dgp 1.03 cosmetic changes.
12/30/92 dgp 1.04 Enhanced summary to account for NAN when no clut access is allowed.
12/30/92 dgp 1.05 Use GDClutSize().
1/6/93	dgp	 1.06 Simplified GDRestoreBlackAndWhite, eliminated linearTable.
1/7/93	dgp  1.07 Included in VideoToolbox-1-93.sea
1/8/93	dgp	 1.08 Added Info-Mac to report file.
1/11/93	dgp	 1.09 In response to bug report from jonathan brecher, added support
			for computers that lack Color QuickDraw.
1/15/93	dgp	1.10 Added version resource.
1/18/93	dgp	1.11 Updated the explanatory text.
1/24/93 dgp	1.12 Updated the explanatory text.
2/6/93 dgp	1.13 Report ROM version.
2/15/93 dgp	1.14 Report ROM version as 124+6*256.
2/22/93	dgp	1.15 Recompiled.
2/27/93	dgp	1.16 Recompiled with new Identify.c.
3/10/93	dgp	1.17 Choose a valid mode for which to print the frame rate.
				 Moved all the timing code into GDInfo.c, so this routine just prints.
3/12/93	dgp		Reorganized the printout and added TestCluts's capabilities.
3/13/93	dgp		Added high-priority timing.
3/16/93	dgp	2.0 Added visual hash inspection.
3/31/93	dgp	2.1 Release version, clut test seems to work.
4/5/93	dgp	2.2 Recognize gray1 error.
4/6/93	dgp	2.3 Remove assumption that mode numbers imply certain number of bits.
4/13/93	dgp	2.3.1 Only set results file type when it is created, not when appending.
4/15/93	dgp		Display Testing window with gray wedge on test screen.
4/16/93	dgp	2.4b Restore compatibility with 1-bit qd.
4/19/93	dgp	2.4.1b Support old Mac II 24-bit NuBus addressing. Fixed two places
				where I used garbage in place of a linear color table when
				gdType==directType. Use new GDNewLinearColorTable.
4/19/93	dgp	2.4.2b Estimate color transformation matrix.
4/19/93	dgp	2.4.3b Use color transformation matrix to assess gray error.
4/19/93	dgp	2.4.4b Fixed RectToAddress to always return a 32-bit address,
					so it won't crash under System 6.
4/25/93	dgp	2.4.5b Correct for -0.5 bias in estimating rgb gains in TestClut.c
4/25/93	dgp	2.4.6b Test both color and gray modes; tolerance=3 lsb.
4/25/93	dgp	2.4.7b Worked around New Palette Manager bugs in original 32-bit QD.
4/27/93	dgp	2.4.8b Fixed stale-cache bug in SetPixelsQuickly. Cleaned up mode
					indexing.
4/28/93	dgp 2.4.9b Rewrote code dealing with SetDepth and HasDepth, wrote GDHasMode().
					Ignore "isGray" for 16 and 32-bit modes.
5/11/93	dgp 2.4.9bb Fixed bug in GDTestClutHash in GDTestClut.c in response to bug 
				report by Jonathan Brecher.
5/17/93	dgp 2.4.9bbb Changed SetEntriesQuickly to respect the color/gray mode of device.
5/18/93	dgp 2.4.9b4 Check that card->device is not NULL.
5/18/93	dgp 2.4.9b5 Suppress printing of identity color transformation.
5/18/93	dgp 2.4.9b6 Note that 16 and 32 bit depths are always in color mode.
5/18/93	dgp 2.5 Release version.
6/2/93	dgp 2.5.1 Updated text reference to info-mac.
7/7/93	dgp 2.5.2 Recompiled with new GDDacSize, for compatibility with
				Radius PowerView.
1/13/94	dgp 2.5.3 Updated description of VideoToolbox.
*/
#include "VideoToolbox.h"
#include <Packages.h>
#include <Traps.h>
#include <math.h>
#include <assert.h>
#include "GDInfo.h"
void TimeVideo(void);
void PrintVideoInfo(FILE *o[2],VideoInfo *card);
void ReportRGBGains(FILE *o[2],Boolean quickly,Boolean isGray,VideoInfo *card);
long GDColors(GDHandle device);
char setEntriesString[][18]={"GDSetEntries","SetEntriesQuickly"};
char colorGrayString[][6]={"color","gray"};
char ColorGrayString[][6]={"Color","Gray"};
#define VERSION "2.5.3"

void main(void)
{
	StackGrow(20000+MAX_SCREENS*sizeof(VideoInfo));
	Require(gestaltOriginalQD);
	TimeVideo();
}

void TimeVideo(void)
{
	long system;
	char string[1000],datafilename[]="TimeVideo results";
	unsigned long v[256],colorMax;
	FILE *o[2],*dataFile;
	int newDataFile;
	unsigned long time;
	short *p;
	Str255 todayStr;
	short i,j,d,error,oldPixelSize,oldIsColor,oldMode,mode,cards,modes,width,height;
	short quickly,isGray,doTest,ok;
	double frames,s,vblPerFrame,f;
	Rect r,screenRect;
	WindowPtr window;
	VideoCardClutTest *clut;
	short doHashTest,flags;
	char blankLine[]="\r"	"          " "          " "          " "          "
							"          " "          " "          " "          " "\r";
	VideoInfo *card;
	
	MaximizeConsoleHeight();
	#if THINK_C
		console_options.title="\pTimeVideo";
	#endif
	printf("\n");	// ask THINK C to initialize quickdraw
	GetDateTime(&time);
	srand(time);
	srandU(time);
	sprintf(string,"Welcome to TimeVideo " VERSION "\n\n"
	"This program will thoroughly test all your video devices and save the "
	"results in the text file “%s”. Don’t be alarmed by the strange "
	"antics of your screens. Everything will soon be back to normal. Just sit back "
	"and enjoy the show.",datafilename);
	sprintf(string,"%s You may quit at any time by hitting Command-period.",string);
	printf(BreakLines(string,80));
	printf("\n\nWould you like the testing to include visual inspections?");
	doHashTest=YesOrNo(1);
	printf("\n");
	if(0)printf(BreakLines("\n"
		"TimeVideo times everything: the video frames, interrupts, and cluts. And it "
		"determines what fraction of the screen you can fill with a real-time movie shown "
		"by CopyBits() or CopyBitsQuickly(). Then it does write-then-read tests of the "
		"clut, to make sure the hardware and software, GDSetEntries or SetEntriesQuickly, "
		"are working correctly.\n",80));
	printf("\n");
	dataFile=fopen(datafilename,"r");
	if(dataFile!=NULL)fclose(dataFile);
	newDataFile=(dataFile==NULL);
	o[0]=stdout;
	o[1]=dataFile=fopen(datafilename,"a");	/* Append to data file */
	if(dataFile==NULL){
		printf("Could neither open nor create “%s”. Perhaps your disk is locked.\n"
			,datafilename);
		newDataFile=0;
	}
	if(newDataFile){
		SetFileInfo(datafilename,'TEXT','ttxt');
		fprintf(dataFile,BreakLines(
		"This file reports the timing and accuracy of all your video screens, "
		"as measured by TimeVideo, a component of the VideoToolbox. To quickly "
		"test a large number of computers, run TimeVideo from a floppy disk; "
		"all the results will be accumulate in a single results file.\n"
		"\nTHE VIDEO TOOLBOX\n"
		"The VideoToolbox is a collection of two hundred C subroutines and several "
		"demo and utility programs that I and others have written to do visual "
		"psychophysics with Macintosh computers. It's free and may not be sold "
		"without permission. It should be useful to anyone who wants to present "
		"accurately specified visual stimuli or use the Mac for psychometric "
		"experiments. The text file “Video synch” discusses all the ways of "
		"synchronizing programs to video displays and the many pitfalls to avoid. The "
		"TimeVideo application checks out the timing of all video devices in "
		"anticipation of their use in critical real-time applications, e.g. movies or "
		"lookup table animation. Low-level routines control video timing and lookup "
		"tables, display real-time movies, and implement the luminance-control "
		"algorithms suggested by Pelli and Zhang (1991). (D.G. Pelli and L. Zhang, 1991, "
		"Accurate control of contrast on microcomputer displays. Vision Research, 31, "
		"1337-1350. Reprints are available.) In particular, "
		"GetPixelsQuickly and SetPixelsQuickly peek and poke pixels in bitmaps and "
		"pixmaps, CopyBitsQuickly copies bit/pixmaps to the screen, and "
		"SetEntriesQuickly and GDSetEntries load the screen's color lookup table, all "
		"without any of QuickDraw's color translations. High-level routines help "
		"analyze psychophysical experiments (e.g. graphing or maximum-likelihood "
		"fitting of psychometric data). Assign.c is a runtime C interpreter for C "
		"assignment statements, which is useful for controlling experiments and "
		"sharing calibration data. This collection has been continually updated since "
		"1991. Many colleagues have indicated that they are using the software in "
		"their labs. Documentation is in the source files themselves. "
		"To get the latest version of the VideoToolbox, just send me your "
		"mailing address, and I’ll mail you a disk. Or download “VideoToolbox.sea” (a "
		"self-extracting archive) electronically from the Info-Mac or MacPsych archives. "
		"The ftp servers are called sumex-aim.stanford.edu [36.44.0.6] (look in "
		"info-mac/Development/SourceCode/) and ftp.stolaf.edu [130.71.128.9] "
		"(look in pub/macpsych/). "
		"Log in as “anonymous”; any password will do. "
		"It's also on CompuServe in the MacDev forum's Library 4 “C and Pascal” "
		"as VIDEOT.SEA.\n",80));
	fprintf(dataFile,
		"Denis Pelli, Professor of Neuroscience\n"
		"Institute for Sensory Research, Syracuse University, Syracuse, NY 13244-5290, USA\n"
		"denis_pelli@isr.syr.edu\n");
	fprintf(dataFile,BreakLines(
		"\nTIME VIDEO\n"
		"For each video card, TimeVideo measures the video frame rate, frequency of VBL "
		"interrupts (supposed to be one per frame), how long it takes to load the clut, "
		"and how much of the screen you can fill with a real-time one-image-per-frame "
		"movie shown by CopyBits() or CopyBitsQuickly(). It then performs a random "
		"write-then-read test of the Color Lookup Table (clut). This tests the clut "
		"memory hardware and the software used to write and read the clut. We test "
		"writing by GDSetEntries(), which passes the request on to the video driver, and "
		"we also test writing by SetEntriesQuickly(), which accesses the hardware "
		"directly. In either case, the clut is read by GDGetEntries(), which passes the "
		"request on to the video driver. The testing is thorough; many video devices fail "
		"at least part of the test. All the driver errors uncovered to date appear in the "
		"VideoToolbox “Video synch” text file. Add your results by emailing this file to "
		"denis_pelli@isr.syr.edu\n"
		"\nGLOSSARY\n"
		"A video frame is a refresh of your video screen. To show a movie, you will want "
		"to reload the image once per frame; the table shows how big that image can be, "
		"as a fraction of the screen area, and still be reloaded once per frame. "
		"(Some video cards have multiple video “pages” that can be switched by "
		"calling GDSetMode(), though I've never tried it.) A “VBL” "
		"interrupt is produced by your video card driver, nominally once per frame, but "
		"some video drivers produce more, which is poor, but not serious. (The "
		"VBLInstall.c program will deal with it.) Suppressed interrupts during clut "
		"updates are bad. It means that the driver disables the VBL interrupt for too "
		"long while it’s loading the clut. This will throw off any interrupt-based "
		"attempt to count frames. The clut is the color lookup table of your video card. "
		"Lookup table animation, e.g. for temporal modulation of contrast, requires that "
		"you reload the clut once per frame, so it’s very important that this be fast "
		"enough. "
		"The first call to SetEntriesQuickly() for each device is slow--a cache is "
		"filled; the reported times are for subsequent calls. "
		"Each video card can be in “Color” or “Gray” mode, as set by the Control "
		"Panel:Monitors or the Macintosh Toolbox call SetDepth(). "
		"In “Gray” mode all colors are transformed to luminance-equivalent "
		"grays. This is done by the video driver, when loading the clut, "
		"but only if the pixelSize≤8. "
		"TimeVideo measures the driver's color transformation matrix, and reports it "
		"if it is other than the identity transformation:\n"
		"(ROut) (1 0 0) (RIn)\n"
		"(GOut)=(0 1 0)x(GIn)\n"
		"(BOut) (0 0 1) (BIn)\n"
		"The clut tests try loading the clut serially, one entry at a time, and "
		"all at once; some video drivers fail the serial test. For more explanation see "
		"the text file called “Video synch” on the VideoToolbox disk.\n"
		"\n"
		"“ok”= passed all tests.\n"
		"“!gray”= passed the color test, but is supposed to be in gray mode.\n"
		"“!color”= passed the gray test, but is supposed to be in color mode.\n"
		"“!serial”= passed when loaded all at once, but failed when loaded serially.\n"
		"“bad”= read did not equal write and the error is reported explicitly.\n"
		"We know that SetEntriesQuickly is “!serial” on the Quadra, alas.\n"
		"\nThis is a TeachText file, "
		"but you may prefer to open it from THINK C, or a word processor, because the columns "
		"in the tables will only align properly if displayed in a monospaced font.\n\n",80));
	}
	ffprintf(o,"TimeVideo version " VERSION "\n");
	GetDateTime(&time);
	IUDateString(time,longDate,todayStr);
	ffprintf(o,"%#s.\n",todayStr);

	// Timer.c requires the Revised Time Manager, which appeared in System 6.0.3.
	// SetDepth() is part of the "new" Palette Manager, which appeared in System 6.0.5.
	Gestalt(gestaltSystemVersion,&system);
	if(system<0x605) {
		PrintfExit("Sorry. Your System is too old; I need at least System 6.0.5.\n");
	}
	card=(VideoInfo *)NewPtr(MAX_SCREENS*sizeof(*card));
	if(card==NULL)PrintfExit("Need more memory. "
		"Use Finder's FIle:Get Info to increase TimeVideo's allocation.\n");
	for(i=0,p=(short *)card;i<GetPtrSize((Ptr)card)/2;i++)*p++=0;	// Zero the cards.

	ffprintf(o,"%s\n",BreakLines(IdentifyMachine(),80));
	ffprintf(o,"Tick rate is %.1f Hz.",TickRate());
	ffprintf(o," System-based VBL rate is %.1f Hz.\n",GDFrameRate(NULL));
	if(QD8Exists())_atexit(RestoreCluts);// In case user quits prematurely.
	for(i=0;;i++){
		card[i].basicTested=0;
		card[i].timeTested=0;
		card[i].clutTested=0;
		for(d=0;d<6;d++){
			card[i].depth[d].timeTested=0;
			for(quickly=0;quickly<2;quickly++)for(isGray=0;isGray<2;isGray++){
				card[i].depth[d].clut[quickly][isGray].read.doTest=0;
				card[i].depth[d].clut[quickly][isGray].read.tested=0;
				card[i].depth[d].clut[quickly][isGray].read.errors=0;
				card[i].depth[d].clut[quickly][isGray].read.errorsAtOnce=0;
				card[i].depth[d].clut[quickly][isGray].hash.tested=0;
				card[i].depth[d].clut[quickly][isGray].hash.doTest=0;
				card[i].depth[d].clut[quickly][isGray].visual.tested=0;
				card[i].depth[d].clut[quickly][isGray].visual.doTest=0;
			}
		}
		if(QD8Exists()){
			card[i].device=GetScreenDevice(i);
			if(card[i].device==NULL)break;
			oldPixelSize=(**(**card[i].device).gdPMap).pixelSize;
			oldMode=(**card[i].device).gdMode;
			oldIsColor=TestDeviceAttribute(card[i].device,gdDevType);
		}else card[i].device=NULL;
		ffprintf(o,"\n%s\n",BreakLines(IdentifyVideo(card[i].device),80));
		printf("Getting card info. . .\r");
		error=GDInfo(&card[i]);
		
		// Specify what tests we want.
		if(card[i].device!=NULL)isGray=!TestDeviceAttribute(card[i].device,gdDevType);
		else isGray=0;
		for(d=0;d<6;d++)for(quickly=0;quickly<2;quickly++){
			card[i].depth[d].clut[quickly][isGray].read.doTest=1;
			card[i].depth[d].clut[quickly][!isGray].read.doTest=1;
		}
		// Further test deepest directType and clutType modes
		d=5;
		while(card[i].depth[d].pixelSize>=0 && card[i].depth[d].pixelSize<16)d--;
		if(d>=0)for(quickly=0;quickly<2;quickly++){
			card[i].depth[d].clut[quickly][isGray].hash.doTest=doHashTest;
			card[i].depth[d].clut[quickly][!isGray].read.doTest=1;
		}
		d=3;
		while(card[i].depth[d].pixelSize==0 || card[i].depth[d].pixelSize>8)d--;
		if(d>=0)for(quickly=0;quickly<2;quickly++){
			card[i].depth[d].clut[quickly][isGray].hash.doTest=doHashTest;
			card[i].depth[d].clut[quickly][!isGray].read.doTest=1;
		}

		// Open window
		if(QD8Exists()){
			r=screenRect=(*card[i].device)->gdRect;
		}else{
			CopyQuickDrawGlobals();	// make sure qd is valid.
			r=screenRect=qd.screenBits.bounds;
		}
		OffsetRect(&r,-r.left,-r.top);
		width=(r.right/=3);
		if(width>256)width=r.right=256;
		height=r.bottom=width;
		CenterRectInRect(&r,&screenRect);
		if(QD8Exists())window=(WindowPtr)NewCWindow(NULL,&r,"\pCLUT Colors",TRUE,noGrowDocProc,(WindowPtr) -1L,0,0);
		else window=NewWindow(NULL,&r,"\pTesting",TRUE,noGrowDocProc,(WindowPtr) -1L,0,0);
			
		for(d=0;d<6;d++)for(isGray=0;isGray<2;isGray++){
			if(card[i].depth[d].pixelSize==0)continue;
			doTest=0;
			for(quickly=0;quickly<2;quickly++){
				doTest|=card[i].depth[d].clut[quickly][isGray].read.doTest;
				doTest|=card[i].depth[d].clut[quickly][isGray].hash.doTest;
				doTest|=card[i].depth[d].clut[quickly][isGray].visual.doTest;
			}
			if(!doTest)continue;
			if(card[i].device!=NULL){
				if(card[i].depth[d].mode!=(**card[i].device).gdMode
					|| isGray!=!TestDeviceAttribute(card[i].device,gdDevType)){
					// On Mac IIci, Sys 6.07, HasDepth returns "mode" of 0x100 
					// at all legal depths.
					if(card[i].depth[d].pixelSize>0)
						ok=HasDepth(card[i].device,card[i].depth[d].pixelSize,1,!isGray);
					else ok=1;
					// Mac IIci Sys 6.07: SetDepth only accepts mode, not pixelSize.
					if(ok)error=SetDepth(card[i].device,card[i].depth[d].mode,1,!isGray);
					if(card[i].depth[d].mode!=(**card[i].device).gdMode
						|| isGray!=!TestDeviceAttribute(card[i].device,gdDevType))continue;
				}
			}else if(card[i].depth[d].pixelSize!=1 || isGray)continue;
			GDInfo(&card[i]);
			
			// Fill window with spectrum of all colors.
			colorMax=GDColors(card[i].device)-1;
			for(j=0;j<width;j++)v[j]=(j*colorMax+width/2)/(width-1);
			for(j=0;j<height;j++)SetWindowPixelsQuickly(window,0,j,v,width);
			
			if(!card[i].depth[d].timeTested){
				error=GDInfoTime(&card[i]);
				for(j=0;j<height;j++)SetWindowPixelsQuickly(window,0,j,v,width);//refresh
			}
			for(quickly=0;quickly<2;quickly++){
				printf(blankLine);
				printf("%d-bit %s pixels: testing clut: %s . . .\r"
					,card[i].depth[d].pixelSize,colorGrayString[isGray]
					,setEntriesString[quickly]);
				if(quickly)flags=testClutQuicklyFlag;
				else flags=0;
				error=GDTestClut(o,flags,&card[i]);
				error=GDTestClut(o,flags|testClutSeriallyFlag,&card[i]);
				error=GDTestClutHash(flags,&card[i]);
			}
		}
		DisposeWindow(window);
		if(QD8Exists()){
			// On Mac IIci, Sys 6.07, HasDepth returns "mode" of 0x100 at all legal depths.
			// and SetDepth only accepts mode, not depth.
			ok=HasDepth(card[i].device,oldPixelSize,1,oldIsColor);
			if(ok)error=SetDepth(card[i].device,oldMode,1,oldIsColor);
		}
		printf(blankLine);
		printf("Getting card info. . .\r");
		error=GDInfo(&card[i]);
		printf(blankLine);
		PrintVideoInfo(o,&card[i]);
		if(!QD8Exists())break;
	}
	cards=i;
	if(GDVersion(card[i].device)==100
		&& EqualString("\p.Display_Video_Apple_RBV1",GDName(card[i].device),1,1)){
			ffprintf(o,
			"NOTE: the built-in driver in the Mac IIci (.Display_Video_Apple_RBV1 version 0)\n"
			"has a bug that causes it to crash if you attempt to read the clut. A temporary\n"
			"patch has been applied that has fixed the driver until the next reboot, as\n"
			"explained in the VideoToolbox file “Video synch”.\n");
	}
	if(dataFile!=NULL){
		fprintf(dataFile,"\n\n");
		fclose(dataFile);
		sprintf(string,"\nThe text file “%s” explains all the results.\n",datafilename);
		printf(BreakLines(string,80));
	}
	DisposePtr((Ptr)card);
}

void PrintVideoInfo(FILE *o[2],VideoInfo *card)
{
	short i,j,k,d,tested,depths,quickly,isGray,grayClut,reportRGB;
	char string[100];
	static char s1[]="%-33s",s2[]="%6s",s3[]="  %-9s\n";
	VideoCardClutTest *clut;
	
	if(card->device!=NULL)isGray=!TestDeviceAttribute(card->device,gdDevType);
	else isGray=0;
	ffprintf(o,"%d-bit dacs. ",card->dacSize);
	ffprintf(o,"%dx%d pixels. ",card->width,card->height);
//	ffprintf(o,"%s mode. ",ColorGrayString[isGray]);
	ffprintf(o,"\n");

	if(card->basicTested){
		ffprintf(o,s1,"pixel size");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%d   ",card->depth[d].pixelSize);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"bits");
	
		if(QD8Exists()){
			// I haven't tried to look for 1-bit QD's page swapping scheme
			ffprintf(o,s1,"pages");
			for(d=0;d<6;d++)if(card->depth[d].pixelSize){
				sprintf(string,"%d   ",card->depth[d].pages);
				ffprintf(o,s2,string);
			}
			ffprintf(o,s3,"");
	
			// Meaningless in 1-bit QD
			ffprintf(o,s1,"mode");
			for(d=0;d<6;d++)if(card->depth[d].pixelSize){
				sprintf(string,"0x%x ",card->depth[d].mode);
				ffprintf(o,s2,string);
			}
			ffprintf(o,s3,"");
		}
	}

	if(card->timeTested){
		ffprintf(o,s1,"frame rate");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.1f ",card->depth[d].frameRate);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"Hz");
	
		ffprintf(o,s1,"interrupts per frame");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.1f ",card->depth[d].vblPerFrame);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"");
	
		ffprintf(o,s1,"CopyBits movie size");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].movieRate/card->depth[d].frameRate);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"screen");
	
		ffprintf(o,s1,"CopyBitsQuickly movie size");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].movieRateQuickly/card->depth[d].frameRate);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"screen");
	
		ffprintf(o,s1,"CopyBits data rate");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].movieRate
				*card->depth[d].pixelSize
				*card->width*card->height/8./1024./1024.);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"MB/s");
	
		ffprintf(o,s1,"CopyBitsQuickly data rate");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].movieRateQuickly
				*card->depth[d].pixelSize
				*card->width*card->height/8./1024./1024.);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"MB/s");
	}
	
	if(card->device!=NULL && (**card->device).gdType!=fixedType && card->timeTested){
		ffprintf(o,s1,"GDSetEntries duration");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].framesPerClutUpdate);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"frames");

		ffprintf(o,s1,"GDSetEntries suppresses ints. for");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.1f ",card->depth[d].missingFramesPerClutUpdate);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"frames");

		ffprintf(o,s1,"GDSetEntriesHighPriority duration");
		for(d=0;d<6;d++)if(card->depth[d].pixelSize){
			sprintf(string,"%.2f",card->depth[d].framesPerClutUpdateHighPriority);
			ffprintf(o,s2,string);
		}
		ffprintf(o,s3,"frames");

		if(card->setEntriesQuickly){
			ffprintf(o,s1,"SetEntriesQuickly duration");
			for(d=0;d<6;d++)if(card->depth[d].pixelSize){
				sprintf(string,"%.2f",card->depth[d].framesPerClutUpdateQuickly);
				ffprintf(o,s2,string);
			}
			ffprintf(o,s3,"frames");
		}
	}
	if(card->device!=NULL && (**card->device).gdType!=fixedType && card->clutTested){
		for(quickly=0;quickly<2;quickly++)for(isGray=0;isGray<2;isGray++){
			tested=0;
			for(d=0;d<6;d++)if(card->depth[d].pixelSize)
				if(card->depth[d].clut[quickly][isGray].hash.tested)tested=1;
			if(tested){
				sprintf(string,"%s hash inspection",setEntriesString[quickly]);
				ffprintf(o,s1,string);
				for(d=0;d<6;d++)if(card->depth[d].pixelSize){
					clut=&card->depth[d].clut[quickly][isGray];
					if(clut->hash.tested){
						if(clut->hash.errors)
							sprintf(string,"fail");
						else sprintf(string,"ok");
					}else sprintf(string,"");
					ffprintf(o,s2,string);
				}
				ffprintf(o,s3,"");
			}
		}
	
		for(quickly=0;quickly<2;quickly++)for(isGray=0;isGray<2;isGray++){
			tested=0;
			for(d=0;d<6;d++)if(card->depth[d].pixelSize)
				if(card->depth[d].clut[quickly][isGray].read.tested)tested=1;
			if(tested){
				reportRGB=0;
				sprintf(string,"%s: %s test"
					,colorGrayString[isGray],setEntriesString[quickly]);
				ffprintf(o,s1,string);
				for(d=0;d<6;d++)if(card->depth[d].pixelSize){
					clut=&card->depth[d].clut[quickly][isGray];
					grayClut=isGray && card->depth[d].pixelSize<=8;
					if(clut->read.tested){
						if(clut->read.errors){
							if(clut->read.errorsAtOnce)sprintf(string,"bad");
							else sprintf(string,"!serial");
						}else if(grayClut!=clut->read.identity)sprintf(string,"ok");
						else sprintf(string,"!%s",colorGrayString[grayClut]);
					}else sprintf(string,"");
					ffprintf(o,s2,string);
					if(card->depth[d].pixelSize<=8)reportRGB=!clut->read.identity;
				}
				ffprintf(o,s3,"");
				if(reportRGB)ReportRGBGains(o,quickly,isGray,card);
			}
	
			tested=0;
			for(d=0;d<6;d++)if(card->depth[d].pixelSize)
				if(card->depth[d].clut[quickly][isGray].visual.tested)tested=1;
			if(tested){
				sprintf(string,"%s visual inspection",setEntriesString[quickly]);
				ffprintf(o,s1,string);
				for(d=0;d<6;d++)if(card->depth[d].pixelSize){
					clut=&card->depth[d].clut[quickly][isGray];
					if(clut->visual.tested){
						if(clut->visual.errors){
							if(!clut->visual.errorsAtOnce)sprintf(string,"!serial");
							else sprintf(string,"bad");
						}else sprintf(string,"ok");
					}else sprintf(string,"");
					ffprintf(o,s2,string);
				}
				ffprintf(o,s3,"");
			}
		}
	}
	for(i=0;i<2;i++)if(o[i]!=NULL)fflush(o[i]);	// Save to disk, just in case.
}

void ReportRGBGains(FILE *o[2],Boolean quickly,Boolean isGray,VideoInfo *card)
{
	short j,k,d,depths;
	VideoCardClutTest *clut;
	double rgbGain[3][3],rgbError[3];
	Boolean integralGains,bad;

	depths=0;
	for(j=0;j<3;j++){
		for(k=0;k<3;k++)rgbGain[j][k]=0.;
		rgbError[j]=0.0;
	}
	integralGains=1;
	for(d=0;d<6;d++)if(card->depth[d].pixelSize && card->depth[d].pixelSize<=8){
		clut=&card->depth[d].clut[quickly][isGray];
		bad=0;
		for(j=0;j<3;j++)bad|=clut->read.rgbError[j]>2.0*(1<<(16-4));
		if(bad)continue;
		for(j=0;j<3;j++){
			for(k=0;k<3;k++)rgbGain[j][k]+=clut->read.rgbGain[j][k];
			rgbError[j]+=clut->read.rgbError[j];
		}
		depths++;
	}
	for(j=0;j<3;j++){
		for(k=0;k<3;k++){
			rgbGain[j][k]/=depths;
			integralGains&=(rgbGain[j][k]==floor(rgbGain[j][k]));
		}
		rgbError[j]/=depths;
	}
	if(IsFinite(rgbGain[0][0]))for(j=0;j<3;j++){
		ffprintf(o," (%cOut±%4.1f%%)%c"
			,"RGB"[j],rgbError[j]*100./USHRT_MAX," = "[j]);
		if(integralGains)ffprintf(o,"(%1.0f %1.0f %1.0f)"
			,rgbGain[j][0],rgbGain[j][1],rgbGain[j][2]);
		else ffprintf(o,"(%3.2f %3.2f %3.2f)"
			,rgbGain[j][0],rgbGain[j][1],rgbGain[j][2]);
		ffprintf(o,"%c(%cIn)\n"," x "[j],"RGB"[j]);
	}
}
