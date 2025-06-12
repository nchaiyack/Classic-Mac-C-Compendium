/*
GDTestClut.c

This is used by TimeVideo; I canÕt think of any reason for including it in any
other program.

OSErr GDTestClut(FILE *o[2],GDHandle device,short flags,VideoInfo *card);

GDTestClut.c tests whether the video clut can be written and read faithfully,
and saves the results in a user-supplied VideoInfo structure. The test consists
of writing random numbers to all the clut entries, reading them back, and
comparing. IÕve been surprised to find more than a few video drivers that fail
this test, for various reasons.

The bits of ÒflagsÓ are tested independently. If flags&testClutQuicklyFlag then
SetEntriesQuickly() will be tested instead of GDSetEntries/GDDirectSetEntries.
If flags&testClutSeriallyFlag then the clut entries will be set individually,
calling GDSetEntries once for each clut entry, to check the clut entry
addressing. If flags&testClutLinearFlag then a simple sequence, will be loaded
into the clut, instead of random numbers, to help figuring out systematic
errors. The sequence is (0,0,0),(1,0,0), (0,2,0),(0,0,3),(4,4,4),(4,0,0), and so
on.

Returned value is zero if ok, nonzero if error occurred.

GDTestClut recognizes the common driver errors and reports them in a sensible
way, using the various fields of the card->depth[d].clut structure. Errors
accumulate in the card->depth[d].clut structure, allowing you to make multiple
calls to GDTestClut and only then summarize the results. It is important that
you zero card->depth[d].clut.tests and card->depth[d].clutQuickly.tests before
your first call to GDTestClut, to induce it to zero the rest of the clut
structures.

Assumes that GDevice record is valid, i.e. the user has not called GDSetMode().

HISTORY:
3/9/93	dgp	code extracted from the demo TestCluts.c, to create a reusable
		subroutine.
3/10/93	dgp	No longer assume that GDevice record reflects the actual state of the
		driver.
4/5/93	dgp	Add support for grayB.
4/19/93	dgp	Fixed bug in VisibleHash that used garbage in place of
			linear color table when gdType==directType. Use GDNewLinearColorTable.
5/11/93	dgp	GDTestClutHash now checks for valid device, in response to bug report
			by Jonathan Brecher.
5/18/93	dgp	Allow small, 0.001, tolerance in establishing the identity transform.
5/25/93	dgp	Increased proportion of time spent loading clut in VisibleHash().
*/
#include "VideoToolbox.h"
#include <math.h>
#include <Errors.h>
#include "GDInfo.h"

// These functions are solely for use within this file.
ColorSpec *MakeClutTable(GDHandle device,short flags);
OSErr WriteClut(GDHandle device,ColorSpec putTable[],short flags);
OSErr ShowGammaTable(FILE *o[2],GDHandle device);
void RGBToGray(RGBColor *rgb,short dacSize);
Boolean UnequalClutEntry(RGBColor *a,RGBColor *b,short mask);
OSErr VisibleHash(GDHandle device,SetEntriesFunction function,short clutEntries
	,short *hashPtr);
void GDRestoreBlackAndWhite(GDHandle device);
OSErr EstimateClutTransform(short flags,VideoInfo *card);
OSErr ComputeRMSClutError(FILE *file
	,ColorSpec *putTable,ColorSpec *getTable,short flags,VideoInfo *card);
OSErr WriteAndReadClut(GDHandle device,short flags,ColorSpec **put,ColorSpec **get);
#define SixteenBitGray 1

OSErr GDTestClut(FILE *o[2],short flags,VideoInfo *card)
{
	short i,j,clutSize,quickly,isGray;
	OSErr error;
	ColorSpec spec,*putTable=NULL,*getTable=NULL;
	VideoCardClutTest *clut;
	
	if(card->device==NULL)return 0;
	isGray=!TestDeviceAttribute(card->device,gdDevType);
	for(quickly=0;quickly<2;quickly++){
		clut=&card->depth[card->d].clut[quickly][isGray];
		if(clut->read.tested==0){
			for(j=0;j<3;j++)for(i=0;i<3;i++)clut->read.rgbGain[j][i]=NAN;
			for(j=0;j<3;j++)clut->read.rgbError[j]=NAN;
			for(j=0;j<3;j++)clut->read.rgbErrorAtOnce[j]=NAN;
			clut->read.errors=0;
			clut->read.errorsAtOnce=0;
		}
		if(clut->visual.tested==0){
			clut->visual.errors=0;
			clut->visual.errorsAtOnce=0;
		}
		if(clut->hash.tested==0){
			clut->hash.errors=0;
		}
	}
	quickly=((flags&testClutQuicklyFlag)!=0);
	clut=&card->depth[card->d].clut[quickly][isGray];
	if(!clut->read.doTest)return 0;
	
	GDInfo(card);
	if(card->device==NULL)return 0;
	clutSize=card->depth[card->d].clutSize;
	error=GDSaveGamma(card->device);
	error=GDUncorrectedGamma(card->device);
	if(error)return error;
	
	error=EstimateClutTransform(flags&~testClutSeriallyFlag,card);
	if(error)goto done;

	error=WriteAndReadClut(card->device,flags,&putTable,&getTable);
	if(error)goto done;

	// COMPARE
	ComputeRMSClutError(o[1],putTable,getTable,flags,card);
	card->clutTested=1;
	
	done:
	DisposePtr((Ptr)putTable);
	DisposePtr((Ptr)getTable);
	GDRestoreGamma(card->device);
	GDRestoreDeviceClut(card->device);

//	if(error)return GDTestClutVisually(flags,card);
	return error;
}

OSErr EstimateClutTransform(short flags,VideoInfo *card)
{
	short i,j,tableSize,quickly,isGray;
	OSErr error;
	ColorSpec *putTable,*getTable;
	VideoCardClutTest *clut;
	long putSum[3],getSum[3][3];
	unsigned short *put3,*get3,n[3];
	double *a,e,squaredError[3];
		
	error=WriteAndReadClut(card->device,flags|testClutGains,&putTable,&getTable);
	if(error)return error;
	tableSize=GetPtrSize((Ptr)putTable)/sizeof(putTable[0]);
	
	// Estimate transformation
	for(i=0;i<3;i++){
		putSum[i]=0;
		for(j=0;j<3;j++)getSum[i][j]=0;
	}
	for(i=0;i<tableSize;i++){
		put3=(unsigned short *)&putTable[i].rgb;
		get3=(unsigned short *)&getTable[i].rgb;
		putSum[i%3]+=put3[i%3];
		for(j=0;j<3;j++)getSum[j][i%3]+=get3[j];
	}
	isGray=!TestDeviceAttribute(card->device,gdDevType);
	quickly=((flags&testClutQuicklyFlag)!=0);
	clut=&card->depth[card->d].clut[quickly][isGray];
	for(i=0;i<3;i++)for(j=0;j<3;j++){
		clut->read.rgbGain[j][i]=(double)getSum[j][i]/putSum[i];
		if(clut->read.rgbGain[j][i]!=floor(clut->read.rgbGain[j][i]))
			clut->read.rgbGain[j][i]+=0.5*tableSize/putSum[i];
	}
	DisposePtr((Ptr)putTable);
	DisposePtr((Ptr)getTable);
}

OSErr WriteAndReadClut(GDHandle device,short flags,ColorSpec **put,ColorSpec **get)
{
	short i,clutSize,tableSize,error;
	ColorSpec spec,*putTable,*getTable;
	
	clutSize=GDClutSize(device);

	// MAKE TABLE FOR CLUT
	*put=putTable=MakeClutTable(device,flags);
	if(putTable==NULL)return MemError();
	tableSize=GetPtrSize((Ptr)putTable)/sizeof(putTable[0]);
	
	// MAKE BLANK TABLE
	*get=getTable=(ColorSpec *)NewPtr(GetPtrSize((Ptr)putTable));
	if(getTable==NULL)return MemError();
	spec.value=spec.rgb.red=spec.rgb.green=spec.rgb.blue=0;
	for(i=0;i<tableSize;i++)getTable[i]=spec;

	// WRITE CLUT & READ BACK, ONE CLUT-FULL AT A TIME
	for(i=0;i<tableSize;i+=clutSize){
		error=WriteClut(device,&putTable[i],flags);
		if(error)return error;
		error=GDGetEntries(device,0,clutSize-1,&getTable[i]);
		if(error)return error;
	}
	return 0;
}

OSErr ComputeRMSClutError(FILE *file
	,ColorSpec *putTable,ColorSpec *getTable,short flags,VideoInfo *card)
{
	short i,j,clutSize,tableSize,quickly,isGray;
	unsigned short *put3,*get3,n[3],bad;
	double *a,e,squaredError[3],model[3];
	VideoCardClutTest *clut;
		
	isGray=!TestDeviceAttribute(card->device,gdDevType);
	quickly=((flags&testClutQuicklyFlag)!=0);
	clut=&card->depth[card->d].clut[quickly][isGray];
	clutSize=GDClutSize(card->device);
	tableSize=GetPtrSize((Ptr)putTable)/sizeof(putTable[0]);

	// Estimate error of linear model
	for(i=0;i<3;i++){
		squaredError[i]=0;
		n[i]=0;
	}
	bad=0;
	// Allow error of a few least significant steps of dac
	clut->read.tolerance=2.9*(1<<(16-GDDacSize(card->device)));
	for(i=0;i<tableSize;i++){
		put3=(unsigned short *)&putTable[i].rgb;
		get3=(unsigned short *)&getTable[i].rgb;
		for(j=0;j<3;j++){
			model[j]=put3[0]*clut->read.rgbGain[j][0]
				+put3[1]*clut->read.rgbGain[j][1]
				+put3[2]*clut->read.rgbGain[j][2];
			e=get3[j]-model[j];
			bad|=fabs(e)>clut->read.tolerance;
			squaredError[j]+=e*e;
			n[j]++;
		}
	}
	for(i=0;i<3;i++){
		clut->read.rgbError[i]=sqrt(squaredError[i]/n[i]);
		if(!(flags&testClutSeriallyFlag))clut->read.rgbErrorAtOnce[i]=sqrt(squaredError[i]/n[i]);
	}

	clut->read.identity=1;
	for(i=0;i<3;i++)for(j=0;j<3;j++)clut->read.identity&=(fabs(clut->read.rgbGain[i][j]-(i==j))<0.001);
	
	if(bad){
		clut->read.errors=1;
		if(!(flags&testClutSeriallyFlag))clut->read.errorsAtOnce=1;
	}
	if(bad){
		// Print one-line error message to file.
		fprintf(file,"\n");
		if(flags&testClutQuicklyFlag)fprintf(file,"SetEntriesQuickly != GDGetEntries. ");
		else switch((*card->device)->gdType){
		case fixedType:
			break;
		case clutType:
			fprintf(file,"GDSetEntries != GDGetEntries. ");
			break;
		case directType:
			fprintf(file,"GDDirectSetEntries != GDGetEntries. ");
			break;
		}
		fprintf(file,"%d-bit ",card->depth[card->d].pixelSize);
		if(isGray)fprintf(file,"gray pixels.");
		else fprintf(file,"color pixels.");
		if(flags&testClutSeriallyFlag)fprintf(file," Loaded one clut entry at a time.\n");
		else fprintf(file," Loaded whole clut at once.\n");

		// Print each clut error.
		for(i=0;i<tableSize;i++){
			put3=(unsigned short *)&putTable[i].rgb;
			get3=(unsigned short *)&getTable[i].rgb;
			bad=0;
			for(j=0;j<3;j++){
				model[j]=put3[0]*clut->read.rgbGain[j][0]
					+put3[1]*clut->read.rgbGain[j][1]
					+put3[2]*clut->read.rgbGain[j][2];
				e=get3[j]-model[j];
				bad|=fabs(e)>clut->read.tolerance;
			}
			if(0 && bad)fprintf(file,"Clut[%3d] wrote(%04u,%04u,%04u) "
				"expected(%04.0f,%04.0f,%04.0f) but read(%04u,%04u,%04u)\n"
				,i%clutSize,put3[0],put3[1],put3[2]
				,model[0],model[1],model[2]
				,get3[0],get3[1],get3[2]);
			if(bad)fprintf(file,"Clut[%3d] wrote(%04x,%04x,%04x) "
				"expected(%04x,%04x,%04x) but read(%04x,%04x,%04x)\n"
				,i%clutSize,put3[0],put3[1],put3[2]
				,(unsigned short)(0.5+model[0]),(unsigned short)(0.5+model[1]),(unsigned short)(0.5+model[2])
				,get3[0],get3[1],get3[2]);
		}
		fprintf(file,"\n");
	}
	clut->read.tested=1;
}

OSErr GDTestClutVisually(short flags,VideoInfo *card)
{
	short i,error,clutSize,quickly,isGray;
	char blankLine[]="\r"	"          " "          " "          " "          "
							"          " "          " "          " "          " "\r";
	ColorSpec *putTable,*normalTable;
	Boolean weirdError,normalError;
	VideoCardClutTest *clut;
	
	isGray=!TestDeviceAttribute(card->device,gdDevType);
	quickly=((flags&testClutQuicklyFlag)!=0);
	clut=&card->depth[card->d].clut[quickly][isGray];
	if(clut->visual.tested==0){
		clut->visual.errors=0;
		clut->visual.errorsAtOnce=0;
	}
	if(card->device==NULL)return 0;
	clutSize=GDClutSize(card->device);
	normalTable=((**(**(**card->device).gdPMap).pmTable)).ctTable;
	if(card->device==GetMainDevice())
		putTable=MakeClutTable(card->device,testClutNegativeFlag);
	else putTable=MakeClutTable(card->device,flags);
	if(putTable==NULL)return MemError();
	GDSaveGamma(card->device);
	GDUncorrectedGamma(card->device);
	error=WriteClut(card->device,putTable,flags);
	if(error){
		GDRestoreGamma(card->device);
		GDRestoreDeviceClut(card->device);
		return error;
	}
	printf(blankLine);
	printf("Screen should be weirdly colored; watch for subtle change as you hit return:\r");
	while(getcharUnbuffered()==-1);
	error=WriteClut(card->device,putTable,0);
	if(error){
		GDRestoreGamma(card->device);
		GDRestoreDeviceClut(card->device);
		return error;
	}
	printf(blankLine);
	printf("Did you see any change at all?");
	weirdError=YesOrNo(0);
	printf("\r");
	error=WriteClut(card->device,normalTable,flags);
	printf(blankLine);
	printf("Screen should be normal now; watch for subtle change as you hit return:\r");
	while(getcharUnbuffered()==-1);
	error=WriteClut(card->device,normalTable,0);
	printf(blankLine);
	printf("Did you see any change at all?");
	normalError=YesOrNo(0);
	printf("\r");
	GDRestoreGamma(card->device);
	GDRestoreDeviceClut(card->device);
	DisposePtr((Ptr)putTable);
	clut->visual.tested++;
	clut->visual.errors+=(weirdError || normalError);
	if(!(flags&testClutSeriallyFlag))clut->visual.errorsAtOnce+=(weirdError || normalError);
	return 0;
}

ColorSpec *MakeClutTable(GDHandle device,short flags)
{
	short shift,i,j,clutSize;
	RGBColor put;
	ColorSpec *table;
	
	clutSize=GDClutSize(device);
	if((flags&testClutGains) && clutSize<16)clutSize=16;
	table=(ColorSpec *)NewPtr(sizeof(*table)*clutSize);
	if(table==NULL)return table;
	for(i=0;i<clutSize;i++) {
		if(flags&testClutNegativeFlag){
			put=((**(**(**device).gdPMap).pmTable)).ctTable[clutSize-1-i].rgb;
		}else if(flags&testClutGains){
			// Estimate rgb gains of any transformation
			put.red=put.green=put.blue=0;
			j=(0xffffL*i+(clutSize-1)/2)/(clutSize-1);
			switch(i%3){
			case 0:
				put.red=j;
				break;
			case 1:
				put.green=j;
				break;
			case 2:
				put.blue=j;
				break;
			}
		}else if(flags&testClutLinearFlag){
			// Linear test pattern
			put.red=put.green=put.blue=0;
			j=0xffffffff*(i+(clutSize-1)/2)/(clutSize-1);
			switch(i%4){
			case 0:
				put.red=put.green=put.blue=j;
				break;
			case 1:
				put.red=j;
				break;
			case 2:
				put.green=j;
				break;
			case 3:
				put.blue=j;
				break;
			}
		}else{
			// Random test pattern
			put.red=randU();
			put.green=randU();
			put.blue=randU();
		}
		if(!SixteenBitGray){
			put.red&=0xff00;
			put.green&=0xff00;
			put.blue&=0xff00;
		}
		table[i].rgb=put;
	}
	return table;
}

OSErr WriteClut(GDHandle device,ColorSpec putTable[],short flags)
{
	short i,clutSize=GDClutSize(device);
	char priority=7;
	OSErr error;
	SetEntriesFunction function;
		
	if(flags&testClutQuicklyFlag)function=SetEntriesQuickly;
	else function=GDSetEntriesByType;
	if(flags&testClutSeriallyFlag){
		// Load one clut entry at a time
		for(i=0;i<clutSize;i++){
			SwapPriority(&priority);	// Force driver to load clut now.
			error=(function)(device,i,0,&putTable[i]);
			SwapPriority(&priority);
			if(error)return error;
		}
	}else{
		// Load whole clut at once
		SwapPriority(&priority);	// Force driver to load clut now.
		error=(function)(device,0,clutSize-1,putTable);
		SwapPriority(&priority);
		if(error)return error;
	}
}

OSErr GDTestClutHash(short flags,VideoInfo *card){
	SetEntriesFunction function;
	OSErr error;
	short isGray,quickly;
	VideoCardClutTest *clut;
	
	if(card->device==NULL)return 0;
	quickly=((flags&testClutQuicklyFlag)!=0);
	isGray=!TestDeviceAttribute(card->device,gdDevType);
	clut=&card->depth[card->d].clut[quickly][isGray];
	if(clut->hash.tested==0)clut->hash.errors=0;
	if(!clut->hash.doTest)return 0;
	if(quickly)function=SetEntriesQuickly;
	else function=GDSetEntriesByType;
	error=VisibleHash(card->device,function,0,&clut->hash.errors);
	clut->hash.tested=1;
	return error;
}

OSErr VisibleHash(GDHandle device,SetEntriesFunction function,short clutEntries
	,short *hashPtr)
{
	OSErr error;
	short clutSize,i;
	short hash;
	long tick;
	ColorSpec *putTable,*linearTable=NULL;
	char string[100];
	char blankLine[]="\r"	"          " "          " "          " "          "
							"          " "          " "          " "          " "\r";

	if(device==NULL || (**device).gdType==fixedType)return 0;
	clutSize=GDClutSize(device);
	if(clutEntries<0 || clutEntries>clutSize)return 1;
	if(clutEntries==0)clutEntries=clutSize;
	if((**device).gdType==directType){
		if(function==GDSetEntries)function=GDDirectSetEntries;
		putTable=linearTable=GDNewLinearColorTable(device);
		if(linearTable==NULL)return MemError();
	}else putTable=((**(**(**device).gdPMap).pmTable)).ctTable;
	error=(function)(device,0,clutEntries-1,putTable);
	if(!error){
		printf(blankLine);
		if(device==GetMainDevice())
			printf("Do you see any dynamic black specks on this screen? (No):");
		else printf("Do you see any dynamic black specks on the test screen? (No):");
		fflush(stdout);
		do{
			tick=TickCount();
			do{
				(function)(device,0,clutEntries-1,putTable);
			}while(TickCount()-tick<30);
		}while(!kbhit());
		hash=YesOrNo(0);
		printf("\r");
		if(linearTable!=NULL)DisposePtr((Ptr)linearTable);
		if(hashPtr!=NULL)*hashPtr=hash;
	}
	return error;
}

void GDRestoreBlackAndWhite(GDHandle device)
// Restore the first & last clut entries to white and black.
{
	short clutSize,error;
	ColorSpec white={255,0xffff,0xffff,0xffff},black={0,0,0,0};

	clutSize=GDClutSize(device);
	switch((**device).gdType){
	case clutType:
		error=GDSetEntries(device,0,0,&white);
		error=GDSetEntries(device,clutSize-1,0,&black-(clutSize-1));
		break;
	case directType:
		error=GDDirectSetEntries(device,0,0,&black);
		error=GDDirectSetEntries(device,clutSize-1,0,&white-(clutSize-1));
		break;
	default:
		break;
	}
}
Boolean UnequalClutEntry(RGBColor *a,RGBColor *b,short mask)
{
	return (a->red&mask)!=(b->red&mask)
		||(a->green&mask)!=(b->green&mask)
		||(a->blue&mask)!=(b->blue&mask);
}

/*
When you set a video screen to monochrome or "gray" (as opposed to "color"),
e.g. using the Control Panel:Monitors, the request is passed on to the video
driver. The video driver transforms each of your rgb triplets to a
luminance-equivalent gray, using a formula that must be very similar, if not
equivalent, to the code below. The rounding is bad, e.g. any gray rgb triplet
(i,i,i), other than (0,0,0), is transformed to (i-1,i-1,i-1), which is darker,
failing to preserve luminance. However, my goal was to replicate Apple's crumby
transformation, not to improve it. I presume that the reason that I have to trim
my numbers down a tad (-0.00001) is that I'm doing this with 80-bit precision
whereas the driver uses the 64-bit precision of the SANE routines. Presumably I
could obtain the same result by compiling this subroutine separately, to use
64-bit floating point, since all the arguments are ints.
*/
void RGBToGray(RGBColor *rgb,short dacSize)
// Empirical formula to replicate Apple's luminance mapping.
{
	short i,shift;
	unsigned long n;
	
	if(!SixteenBitGray){
		shift=16-dacSize;
		i=(rgb->red>>shift)*(0.30-0.00001)
			+(rgb->green>>shift)*(0.59-0.00001)+(rgb->blue>>shift)*(0.11);
		n=0xffffffffUL/((1<<dacSize)-1);
		rgb->red=rgb->green=rgb->blue=(i*n)>>16;
	}else{
		rgb->red=rgb->green=rgb->blue=rgb->red*(0.30-0.00001)
			+rgb->green*(0.59)+rgb->blue*(0.11-0.00001);
	}
}

