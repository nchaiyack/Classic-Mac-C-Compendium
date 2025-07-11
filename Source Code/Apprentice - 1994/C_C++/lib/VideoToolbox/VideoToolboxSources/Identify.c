/* Identify.c
Each routine returns an informative C string. Here are samples. (I suggest you
use BreakLines() to printf the longer ones.)

IdentifyCompiler:
"THINK C 6 compiled to generate 68020 instructions,
generate 68881 instructions, and use 12-byte universal-format doubles."

IdentifyOwner:
"Denis Pelli"

IdentifyModel:
"PowerBook 170"

IdentifyMachine:
"Denis Pelli�s PowerBook 170 with 68030 and 68882 running System 7.10, ROM 1024K
version 124+6*256. Caching data & instructions."

IdentifyVideo(device):
"PowerBook 170 �Macintosh D Built-In Video� (.Display_Video_Apple_TIM) in slot 0"

The computer model appears only if the slot==0, which indicates built-in video. The
curly quotes embrace the card name, and the parentheses embrace the driver name.
The driver's version number appears only if it's non-zero.

HISTORY:
1/29/92	dgp wrote IdentifyCompiler.
2/25/92	dgp	wrote IdentifyMachine.
8/26/92	dgp get owner and model name from the System file
2/20/93	dgp	added ROM version
2/27/93	dgp merged IdentifyCompiler.c and IdentifyMachine.c into the new Identify.c,
			and added IdentifyVideo.
4/25/93	dgp	IdentifyMachine reports 24/32-bit addressing.
12/14/93 dgp If THINK C compiler version is "5" I report "5 or 6.0" since Symantec
			forgot to increment the version number when they released 6.0. They
			fixed this in version 6.01.
3/2/94	dgp	Extracted IdentifyOwner from within IdentifyMachine
*/
#include "VideoToolbox.h"
#include <Traps.h>	// _HWPriv
#include "mc68881.h"

/*
This table was complete in summer '92, but does not include the newer models.
However, the table is used only as a backup, when the System lacks a machine name
string, which is a feature of System 7. Since all new machines nominally require
System 7, it is unlikely that it will ever be necessary to look them up in
this table.
*/
char machineName[][20]={
	"Unknown Macintosh","Macintosh","Mac XL","Mac 512KE","Mac Plus","Mac SE","Mac II"
	,"Mac IIx","Mac IIcx","Mac SE/30","Mac Portable","Mac IIci","Unknown Macintosh"
	,"Mac IIfx","Unknown Macintosh","Unknown Macintosh","Unknown Macintosh"
	,"Mac Classic","Mac IIsi","Mac LC","Mac Quadra 900","PowerBook 170"
	,"Mac Quadra 700","Mac Classic II","PowerBook 100","PowerBook 140","Quadra 950"
	,"Unknown Macintosh"
};

char processorName[][20]={
	"unknown processor","68000","68010","68020","68030","68040","unknown processor"
};

char fpuName[][32]={
	"no floating point unit","68881","68882","built-in floating point"
	,"unknown floating point unit"
};

char *IdentifyCompiler(void)
{
	static char string[200];
	char *compiler,*longs,*floating,version[4],*format;
	double v;
	
	string[0]=0;
	compiler="";
	#if THINK_C
		compiler="THINK ";
	#endif
	#if applec
		compiler="MPW ";
	#endif
	v=0;
	#if THINK_C==1
		v=4;
	#endif
	#if THINK_C>1
		v=THINK_C;
	#endif
	if(v>0.0){
		if(v==5.0)sprintf(version,"5 or 6.0 "); /* Symantec forgot to update version # */
		else sprintf(version,"%1.0f ",v);
	}else sprintf(version,"");
	if(mc68020)longs="generate 68020 instructions";
	else longs="not generate 68020 instructions";
	if(mc68881)floating="generate 68881 instructions";
	else floating="do all float arithmetic through SANE";
	if(sizeof(double)==12){
		format="native-format doubles";
		#if THINK_C>1 
			#if !__option(native_fp)
				format="universal-format doubles";
			#endif
		#endif
	}else format="doubles";
	sprintf(string,"%sC %scompiled to %s, \n%s, and use %ld-byte %s."
		,compiler,version,longs,floating,sizeof(double),format);
	return string;
}

char *IdentifyModel(void)
{
	OSErr error;
	long machine;
	int machines=sizeof(machineName)/sizeof(machineName[0]);
	static char string[32];

	string[0]=0;
	error=Gestalt(gestaltMachineType,&machine);
	if(!error){
		GetIndString((unsigned char *)string,kMachineNameStrID,machine);
		p2cstr((unsigned char *)string);
		if(strlen(string)==0){
			if(machine<0 || machine>=machines)machine=0;
			sprintf(string,"%#s",machineName[machine]);
		}
	}
	return string;
}

char *IdentifyVideo(GDHandle device)
// E.g. "PowerBook 170 �Macintosh D Built-In Video� (.Display_Video_Apple_TIM)"
{
	static char string[256];
	long quickDraw;

	string[0]=0;
	Gestalt(gestaltQuickdrawVersion,&quickDraw);
	if(quickDraw<gestalt8BitQD){
		sprintf(string,"%s ",IdentifyModel());
		sprintf(string,"%s�%s�",string,"1-bit QuickDraw");
	}else{
		if(GetDeviceSlot(device)==0)sprintf(string,"%s ",IdentifyModel());
		sprintf(string,"%s�%s�",string,GDCardName(device));
		if(GDVersion(device)==0)sprintf(string,"%s (%#s)",string,GDName(device));
		else sprintf(string,"%s (%#s version %d)"
			,string,GDName(device),GDVersion(device));
		sprintf(string,"%s slot %d",string,GetDeviceSlot(device));
	}
	return string;
}

char *IdentifyOwner(void)
{
	static char string[64];
	unsigned char **owner;
	
	string[0]=0;
	owner=GetString(-16096);	// Get owner's name from System file
	if(owner!=NULL && *owner[0]>0)sprintf(string,"%#s",*owner);
	return string;
}

char *IdentifyMachine(void)
{
	OSErr error;
	long fpu,processor,system,value;
	int processors=sizeof(processorName)/sizeof(processorName[0]);
	int fpus=sizeof(fpuName)/sizeof(fpuName[0]);
	static char string[256];
	Boolean cacheData=1,cacheInstructions=1;
	long romSize,romVersion,qD;
	char *owner;
	
	string[0]=0;
	error=Gestalt(gestaltSystemVersion,&system);
	if(error)return string;						/* Gestalt not available */
	Gestalt(gestaltProcessorType,&processor);
	if(processor<0 || processor>=processors)processor=processors-1;
	Gestalt(gestaltFPUType,&fpu);
	if(fpu<0 || fpu>=fpus)fpu=fpus-1;
	owner=IdentifyOwner();
	if(strlen(owner)>0)sprintf(string,"%s�s ",owner);
	sprintf(string,"%s%s",string,IdentifyModel());
	sprintf(string,"%s with %s and %s running System %lx.%lx"
		,string,processorName[processor],fpuName[fpu]
		,system/0x100,system%0x100/0x10);
	system%=0x10;
	if(system)sprintf(string,"%s.%lx",string,system);
	Gestalt(gestaltROMSize,&romSize);
	Gestalt(gestaltROMVersion,&romVersion);
	sprintf(string,"%s, %ldK ROM version %ld+%ld*256."
		,string,romSize/1024,romVersion%256,romVersion/256);
	if(TrapAvailable(_HWPriv)){
		cacheData=SwapDataCache(1);
		SwapDataCache(cacheData);
		cacheInstructions=SwapInstructionCache(1);
		SwapInstructionCache(cacheInstructions);
		if(cacheData || cacheInstructions)sprintf(string,"%s Caching",string);
		else sprintf(string,"%s No caching",string);
		if(cacheData)sprintf(string,"%s data",string);
		if(cacheData && cacheInstructions)sprintf(string,"%s &",string);
		if(cacheInstructions)sprintf(string,"%s instructions",string);
		sprintf(string,"%s.",string);
	}
	value=0;
	Gestalt(gestaltAddressingModeAttr,&value);
	if(value&1<<gestalt32BitAddressing)sprintf(string,"%s 32-bit addressing.",string);
	else sprintf(string,"%s 24-bit addressing.",string);
	Gestalt(gestaltQuickdrawVersion,&qD);
	switch(qD/0x100){
		case 0:
			sprintf(string,"%s 1-bit QuickDraw.",string);
			break;
		case 1:
			sprintf(string,"%s 8-bit QuickDraw.",string);
			break;
		case 2:
			sprintf(string,"%s 32-bit QuickDraw 1.%02lx.",string,qD%0x100);
			break;
		default:
			sprintf(string,"%s QuickDraw 0x%lx.",string,qD);
			break;
	}
	return string;
}
