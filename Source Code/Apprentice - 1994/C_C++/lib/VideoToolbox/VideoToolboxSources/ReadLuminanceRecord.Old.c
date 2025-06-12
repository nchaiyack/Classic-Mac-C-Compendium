#error "Obsolete version"
/*
ReadLuminanceRecord.c

	ReadLuminanceRecord("LuminanceRecord1.h",&LR,0);
Reads a LuminanceRecord?.h file at runtime. In the past these calibration-data files
could only be used by #including them at compile time.

	long WriteLuminanceRecord(char *filename,luminanceRecord *LP,short flags);
Writes (and verifies) a LuminanceRecord, appending to the file.

	Variable *DescribeLuminanceRecordAssignment(luminanceRecord *LP);
Creates a Variable array to read and write LuminanceRecord assignment files.


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
*/

#include "VideoToolbox.h"
#include "Luminance.h"
#include <assert.h>
#define VARIABLES 32

long ReadLuminanceRecord(char *filename,luminanceRecord *LP,short flags)
{
	Variable *v;
	long n;

	v=DescribeLuminanceRecordAssignment(LP);
	if(v==NULL){
		if(flags&assignNoPrintfExit)return assignMemoryError;
		else PrintfExit("\nReadLuminanceRecord: no room for Variables.\n\007");
	}
	n=InitializeVariables(v,flags);
	LP->dacSize=8;					/*  default value */
	n=ReadAssignmentFile(filename,v,flags);
	free(v);
	return n;
}

long WriteLuminanceRecord(char *filename,luminanceRecord *LP,short flags)
{
	Variable *v=NULL,*v2=NULL;
	luminanceRecord *LP2=NULL;
	long n,m;

	/*  Write */
	v=DescribeLuminanceRecordAssignment(LP);
	if(v==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for Variables.\n\007");
	}
	n=PrintAssignmentsToFile(filename,v,flags);
	SetFileInfo(filename,'TEXT','KAHL');
	if(n<0)goto done;
	
	/*  Verify */
	LP2=(luminanceRecord *)malloc(sizeof(luminanceRecord));
	if(LP2==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for luminanceRecord.\n\007");
	}
	v2=DescribeLuminanceRecordAssignment(LP2);
	if(v2==NULL){
		n=assignMemoryError;
		if(flags&assignNoPrintfExit)goto done;
		else PrintfExit("\nWriteLuminanceRecord: no room for Variables.\n\007");
	}
	m=ReadAssignmentFile(filename,v2,flags|assignReportUnknown);
	if(m<0){
		n=m;
		goto done;
	}
	m=UnequalVariables(v,v2,flags);
	if(m<0){
		n=m;
		goto done;
	}
done:
	if(v2!=NULL)free(v2);
	if(LP2!=NULL)free(LP2);
	if(v!=NULL)free(v);
	return n;
}

Variable *DescribeLuminanceRecordAssignment(luminanceRecord *LP)
{
	Variable *v,*variables;
	int j,n;

	v=variables=(Variable *)malloc(VARIABLES*sizeof(Variable));
	if(v==NULL)return NULL;
	j=0;
	v[j++]=SetAVariable(shortType,&LP->screen,"LR.screen",0,"device=GetScreenDevice(LR.screen);");
	v[j++]=SetVariable(stringType,&LP->id,"LR.id");
	v[j++]=SetVariable(stringType,&LP->name,"LR.name");
	v[j++]=SetVariable(stringType,&LP->date,"LR.date");
	v[j++]=SetVariable(stringType,&LP->notes,"LR.notes");
	v[j++]=SetVariable(shortType,&LP->dacSize,"LR.dacSize");
	v[j++]=SetAVariable(doubleType,&LP->LMin,"LR.LMin",0
		,"luminance at VMin");
	v[j++]=SetAVariable(doubleType,&LP->LMax,"LR.LMax",0
		,"luminance at VMax");
	v[j++]=SetAVariable(doubleType,&LP->LBackground,"LR.LBackground",0
		,"background luminance during calibration");
	v[j++]=SetAVariable(shortType,&LP->VBackground,"LR.VBackground",0
		,"background number used during calibration");
	v[j++]=SetAVariable(doubleType,&LP->dpi,"LR.dpi",0,"pixels/inch");
	v[j++]=SetAVariable(doubleType,&LP->Hz,"LR.Hz",0,"frames/second");
	v[j++]=SetVariable(stringType,&LP->units,"LR.units");
	v[j++]=SetAVariable(longType,&LP->coefficients,"LR.coefficients",0
		,"# of coefficients in polynomial fit");
	v[j++]=SetAVariable(doubleType,&LP->p,"LR.p",sizeof(LP->p)/sizeof(LP->p[0])
		,"L(V)=p[0]+p[1]*V+p[2]*V*V+ . . . ±polynomialError");
	v[j++]=SetAVariable(doubleType,&LP->polynomialError,"LR.polynomialError",0
		,"RMS error of fit");
	v[j++]=SetAVariable(doubleType,&LP->q,"LR.q",sizeof(LP->q)/sizeof(LP->q[0])
		,"L(V)=q[0]+q[1]*V+q[2]*V*V±quadraticError");
	v[j++]=SetAVariable(doubleType,&LP->quadraticError,"LR.quadraticError",0
		,"RMS error of fit");
	v[j++]=SetAVariable(doubleType,&LP->power,"LR.power"
		,sizeof(LP->power)/sizeof(LP->power[0])
		,"L(V)=power[0]+Rectify(power[1]+power[2]*V)^power[3]±powerError */\\\n"
		"    /* where Rectify(x)=x if x³0, and Rectify(x)=0 otherwise. */\\\n"
		"    /* Pelli & Zhang (1991) Eqs.9&10 use symbols v=V/255, */\\\n"
		"    /* alpha=power[0], beta=power[1], kappa=power[2]*255, gamma=power[3]");
	v[j++]=SetAVariable(doubleType,&LP->powerError,"LR.powerError",0
		,"RMS error of fit");
	v[j++]=SetAVariable(doubleType,&LP->fixedPower,"LR.fixedPower"
		,sizeof(LP->fixedPower)/sizeof(LP->fixedPower[0])
		,"L(V)=fixedPower[0]+Rectify(fixedPower[1]+fixedPower[2]*V)^fixedPower[3]±fixedPowerError */\\\n"
		"    /* The exponent fixedPower[3] is fixed.");
	v[j++]=SetAVariable(doubleType,&LP->fixedPowerError,"LR.fixedPowerError",0
		,"RMS error of fit");
	v[j++]=SetVariable(doubleType,&LP->r,"LR.r");
	v[j++]=SetVariable(doubleType,&LP->g,"LR.g");
	v[j++]=SetVariable(doubleType,&LP->b,"LR.b");
	v[j++]=SetVariable(doubleType,&LP->gainAccuracy,"LR.gainAccuracy");
	v[j++]=SetAVariable(doubleType,&LP->gm,"LR.gm",0
		,"The monitor's contrast gain.");
	v[j++]=SetAVariable(shortType,&LP->rangeSet,"LR.rangeSet",0
		,"zero indicates that range parameters have yet to be set");
	v[j++]=SetAVariable(shortType,&LP->L.exists,"LR.L.exists",0
		,"zero indicates that luminance table has yet to be initialized");
	v[j++]=SetAVariable(shortType,&LP->VMin,"LR.VMin",0
		,"minimum value that can be loaded into DAC");
	v[j++]=SetAVariable(shortType,&LP->VMax,"LR.VMax",0
		,"maximum value that can be loaded into DAC");
	v[j++]=SetVariable(0,NULL,NULL);				/* Mark end of list */
	assert(j<=VARIABLES);
	return variables;
}