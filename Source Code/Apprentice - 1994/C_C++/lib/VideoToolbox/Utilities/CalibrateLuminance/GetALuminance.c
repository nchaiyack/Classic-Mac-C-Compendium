/*
GetALuminance.c
© 1990-1992 Denis G. Pelli
Two subroutines used by CalibrateLuminance.c. Probably not useful outside that context.

HISTORY:
7/28/90	dgp	wrote it.
9/24/90	dgp	changed nBackground to VBackground.
6/25/91 dgp fixed bug pointed out by Sujeet Paul whereby LToVHunt() made a superfluous
call to GetALuminance() with frames=1, forcing use of the A/D even if it wasn't available.
On a machine without an A/D card this would cause CalibrateLuminance to fail.
On machines with an A/D card this wasted a second on each call to LToVHunt().
8/24/91	dgp	Made compatible with THINK C 5.0.
12/17/92 dgp Enhanced to support arbitrary dacSize. Replaced 256 by LP->VMax+1 
in LToVHunt.
12/21/92 dgp No longer load unused dac bits.
*/
#include "VideoToolbox.h"
#include "Luminance.h"

double GetALuminance(luminanceRecord *LP,GDHandle device,
	int frames,double LuminancePerVoltage,int entry,int red,int green,int blue);
void LToVHunt(luminanceRecord *LP,GDHandle device,CWindowPtr window,
	double LuminancePerVoltage,int frames,double darkLuminance);


double GetALuminance(luminanceRecord *LP,GDHandle device,
	int frames,double LuminancePerVoltage,int entry,int red,int green,int blue)
/*
Get luminance produced by an RGB triplet. If frames==0 then ask for manual
reading. If frames!=0 then use A/D to make automatic reading.
*/
{
	static double L;
	static char string[100];
	long finalTicks;
	short left;

	LP->dacSize=Log2L(LP->VMax*2-1);	// in bits, rounded up
	left=LP->leftShift=16-LP->dacSize;
	LP->table[entry].rgb.red   = red<<left;
	LP->table[entry].rgb.green = green<<left;
	LP->table[entry].rgb.blue  = blue<<left;
	LoadLuminances(device,LP,entry,entry);		/* load clut entry */
	if(frames){
		Delay(60L,&finalTicks);					/* wait for photometer to settle */
		LoadLuminances(device,LP,entry,entry);	/* synchronize to display */
		L=VoltsDuringFrame(frames)*LuminancePerVoltage;
	}
	else{
		printf("Please enter luminance in %s (%4.1f):",LP->units,L);
		gets(string);
		sscanf((char *)string,"%lf",&L);
	}
	return L;
}


void LToVHunt(luminanceRecord *LP,GDHandle device,CWindowPtr window,
	double LuminancePerVoltage,int frames,double darkLuminance)
/*
finds the DAC setting j, such that the desired luminance is
between Lj and Lj+1. This is achieved by actually measuring
the resulting luminances, before the LuminanceRecord is
fully defined. The search proceeds by bisection:
LP->LBackground==desired luminance;
LP->VBackground==j;
*/
{
	int ju,jm,jl;
	double LMeasured;
	WindowPtr oldWindow;
	int index=1;

	GetPort(&oldWindow);
	SetPort((WindowPtr)window);
	BringToFront((WindowPtr)window);
	PmBackColor(index);
	EraseRect(&window->portRect);
	jl=0;
	ju=LP->VMax+1;
	while (ju-jl > 1) {
		jm=(ju+jl)/2;
		LMeasured=GetALuminance(LP,device,frames,LuminancePerVoltage
			,index,jm,jm,jm);
		LMeasured-=darkLuminance;
		if (LP->LBackground > LMeasured)
			jl=jm;
		else
			ju=jm;
	}
	LP->VBackground=jl;
	LP->table[index].rgb=(**(**(**device).gdPMap).pmTable).ctTable[index].rgb;
	LoadLuminances(device,LP,index,index);		/* restore clut entry */
	SendBehind((WindowPtr)window,NULL);
	SetPort(oldWindow);
	return;
}
