/*
ReadLuminanceRecord.c

long InitializeLuminanceRecord(luminanceRecord *LP,short flags);

	i=ReadLuminanceRecord("LuminanceRecord1.h",&LR,0);
Reads a LuminanceRecord?.h file at runtime. In the past, these calibration-data files
could only be used by #including them at compile time.

	long WriteLuminanceRecord(char *filename,luminanceRecord *LP,short flags);
Writes (and verifies) a LuminanceRecord, appending to the file.

	Description *DescribeLuminanceRecord(luminanceRecord *LP);
Creates a Description array to read and write LuminanceRecord assignment files.


HISTORY:
7/29/91 dgp
8/24/91	dgp	Made compatible with THINK C 5.0.
			Preserve default values of LP->VMin and LP->VMax if no new values are read.
8/26/91	dgp	Rewrote using new SetVariable() routine, which makes the code easier to
			read. 
12/17/92 dgp Added dacSize.
12/21/92 dgp Changed type of dacSize from long to short.
6/3/93	dgp	Updated to work with new Assign.c. 
			Created DescribeLuminanceRecordAssignment() and WriteLuminanceRecord().
8/12/93	dgp	Renamed routines to be consistent with new Assign.c.
			Renamed "DescribeLuminanceRecordAssignment" to "DescribeLuminanceRecord".
3/4/94	dgp ReadLuminanceRecord no longer initializes. Added InitializeLuminanceRecord.
*/

#include "VideoToolbox.h"
#include "Luminance.h"
#include <assert.h>
#define VARIABLES 32
long InitializeLuminanceRecord(luminanceRecord *LP,short flags);

long InitializeLuminanceRecord(luminanceRecord *LP,short flags)
{
	Description *d;
	long n;

	d=DescribeLuminanceRecord(LP);
	if(d==NULL){
		if(flags&assignNoPrintfExit)return assignMemoryError;
		else PrintfExit("\InitializeLuminanceRecord: no room for Variables.\n\007");
	}
	n=InitializeDescribedVars(d,flags);
	LP->dacSize=8;					/*  default value */
	free(d);
	return n;
}

long ReadLuminanceRecord(char *filename,luminanceRecord *LP,short flags)
{
	Description *d;
	long n;

	d=DescribeLuminanceRecord(LP);
	if(d==NULL){
		if(flags&assignNoPrintfExit)return assignMemoryError;
		else PrintfExit("\nReadLuminanceRecord: no room for Variables.\n\007");
	}
//	n=InitializeDescribedVars(d,flags);
	LP->dacSize=8;					/*  default value */
	n=ReadAssignmentFile(filename,d,flags);
	free(d);
	return n;
}

long WriteLuminanceRecord(char *filename,luminanceRecord *LP,short flags)
{
	Description *d=NULL,*d2=NULL;
	luminanceRecord *LP2=NULL;
	long n,m;

	/*  Write */
	d=DescribeLuminanceRecord(LP);
	if(d==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for Variables.\n\007");
	}
	n=PrintAssignmentsToFile(filename,d,flags);
	SetFileInfo(filename,'TEXT','KAHL');
	if(n<0)goto done;
	
	/*  Verify */
	LP2=(luminanceRecord *)malloc(sizeof(luminanceRecord));
	if(LP2==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for luminanceRecord.\n\007");
	}
	d2=DescribeLuminanceRecord(LP2);
	if(d2==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for Variables.\n\007");
	}
	m=ReadAssignmentFile(filename,d2,flags|assignReportUnknown);
	if(m<0){
		n=m;
		goto done;
	}
	m=UnequalDescribedVars(d,d2,flags);
	if(m<0){
		n=m;
		goto done;
	}
done:
	if(d2!=NULL)free(d2);
	if(LP2!=NULL)free(LP2);
	if(d!=NULL)free(d);
	return n;
}

Description *DescribeLuminanceRecord(luminanceRecord *LP)
{
	Description *d,*variables;
	int j,n;

	d=variables=(Description *)malloc(VARIABLES*sizeof(Description));
	if(d==NULL)return NULL;
	j=0;
	d[j++]=Describe(shortType,&LP->screen,"LR.screen"
		,"device=GetScreenDevice(LR.screen);");
	d[j++]=Describe(stringType,&LP->id,"LR.id",NULL);
	d[j++]=Describe(stringType,&LP->name,"LR.name",NULL);
	d[j++]=Describe(stringType,&LP->date,"LR.date",NULL);
	d[j++]=Describe(stringType,&LP->notes,"LR.notes",NULL);
	d[j++]=Describe(shortType,&LP->dacSize,"LR.dacSize",NULL);
	d[j++]=Describe(doubleType,&LP->LMin,"LR.LMin","luminance at VMin");
	d[j++]=Describe(doubleType,&LP->LMax,"LR.LMax"
		,"luminance at VMax");
	d[j++]=Describe(doubleType,&LP->LBackground,"LR.LBackground"
		,"background luminance during calibration");
	d[j++]=Describe(shortType,&LP->VBackground,"LR.VBackground"
		,"background number used during calibration");
	d[j++]=Describe(doubleType,&LP->dpi,"LR.dpi","pixels/inch");
	d[j++]=Describe(doubleType,&LP->Hz,"LR.Hz","frames/second");
	d[j++]=Describe(stringType,&LP->units,"LR.units",NULL);
	d[j++]=Describe(longType,&LP->coefficients,"LR.coefficients"
		,"# of coefficients in polynomial fit");
	d[j++]=DescribeArray(doubleType,&LP->p,"LR.p"
		,"L(V)=p[0]+p[1]*V+p[2]*V*V+ . . . �polynomialError"
		,sizeof(LP->p)/sizeof(LP->p[0]),0L);
	d[j++]=Describe(doubleType,&LP->polynomialError,"LR.polynomialError"
		,"RMS error of fit");
	d[j++]=DescribeArray(doubleType,&LP->q,"LR.q"
		,"L(V)=q[0]+q[1]*V+q[2]*V*V�quadraticError",sizeof(LP->q)/sizeof(LP->q[0]),0L);
	d[j++]=Describe(doubleType,&LP->quadraticError,"LR.quadraticError"
		,"RMS error of fit");
	d[j++]=DescribeArray(doubleType,&LP->power,"LR.power"
		,"L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]�powerError */\\\n"
		"    /* where Rectify(x)=x if x�0, and Rectify(x)=0 otherwise. */\\\n"
		"    /* Pelli & Zhang (1991) Eqs.9&10 use symbols v=V/255, */\\\n"
		"    /* alpha=power[0], beta=power[1], kappa=power[2]*255, gamma=power[3]"
		,sizeof(LP->power)/sizeof(LP->power[0]),0L);
	d[j++]=Describe(doubleType,&LP->powerError,"LR.powerError"
		,"RMS error of fit");
	d[j++]=DescribeArray(doubleType,&LP->fixedPower,"LR.fixedPower"
		,"L(V)=fixedPower[0]+Rectify(fixedPower[1]+fixedPower[2]*V)^fixedPower[3]�fixedPowerError */\\\n"
		"    /* The exponent fixedPower[3] is fixed."
		,sizeof(LP->fixedPower)/sizeof(LP->fixedPower[0]),0L);
	d[j++]=Describe(doubleType,&LP->fixedPowerError,"LR.fixedPowerError"
		,"RMS error of fit");
	d[j++]=Describe(doubleType,&LP->r,"LR.r",NULL);
	d[j++]=Describe(doubleType,&LP->g,"LR.g",NULL);
	d[j++]=Describe(doubleType,&LP->b,"LR.b",NULL);
	d[j++]=Describe(doubleType,&LP->gainAccuracy,"LR.gainAccuracy",NULL);
	d[j++]=Describe(doubleType,&LP->gm,"LR.gm"
		,"The monitor's contrast gain.");
	d[j++]=Describe(shortType,&LP->rangeSet,"LR.rangeSet"
		,"zero indicates that range parameters have yet to be set");
	d[j++]=Describe(shortType,&LP->L.exists,"LR.L.exists"
		,"zero indicates that luminance table has yet to be initialized");
	d[j++]=Describe(shortType,&LP->VMin,"LR.VMin"
		,"minimum value that can be loaded into DAC");
	d[j++]=Describe(shortType,&LP->VMax,"LR.VMax"
		,"maximum value that can be loaded into DAC");
	d[j++]=Describe(0,NULL,NULL,NULL);				/* Mark end of list */
	assert(j<=VARIABLES);
	return variables;
}