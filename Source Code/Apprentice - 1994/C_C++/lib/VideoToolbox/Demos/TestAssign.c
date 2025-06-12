/*
TestAssign.c

This is significantly more complicated than it needs to be. The fancy stuff
is merely to put Assign.c through its paces. In particular, if the array a[]
didn't change dimensions from block to block in testAssign1 then we could
read the whole file by a single call to ReadAssignmentFile, instead of
opening the file and making repeated calls to ReadAssignmentBlock, etc.

For a good example of the use of Assign, see ReadLuminanceRecord.c.

HISTORY:
8/93	dgp	created this new version, which cobbles together several older versions.
8/12/93	dgp	updated to work with new Assign.c
9/8/93	dgp	call Require() and use new AllocateDescriptions.
*/
#include "VideoToolbox.h"
#include <assert.h>
#if MAC_C
	#include "Luminance.h"
#endif
#define PREFERENCES_FOLDER 0
void moreTests(void);
void main(void);

typedef struct {
	double *a,logC,viewingDistance;
	long trials;
	char *message;
	unsigned char *m;
} Psychophysics;
Description *DescribePsychophysics(Psychophysics *p);

Description *DescribePsychophysics(Psychophysics *p)
{
	Description *d;
	int i;
	
	d=AllocateDescriptions(10);
	i=0;
	d[i++]=Describe(stringType,&p->message,"message",NULL);
	d[i++]=Describe(doubleType,&p->viewingDistance,"viewingDistance",NULL);
	d[i++]=Describe(shortType,&p->trials,"trials",NULL);
	d[i++]=Describe(doubleType,&p->logC,"logC",NULL);
	d[i++]=Describe(unsignedCharPtrType,&p->m,"m",NULL);
	d[i++]=Describe(doublePtrType,&p->a,"a",NULL);
	assert(i<=10);		/*  make sure array was big enough */
	d[i++]=NullDescription();		/*  mark end of array */
	return d;
}

void main(void)
{
	Description *d;
	static Psychophysics psychophysics,*p;
	short i,j,flags=assignReportUnknown;
	FILE *stream;
	char filename[]="testAssign1";
	
	Require(0);
	p=&psychophysics;
	d=DescribePsychophysics(p);
	InitializeDescribedVars(d,flags);
	stream=fopen(filename,"r");
	do{
		printf("\n/******** New block ********/\n");
		ReadAssignmentBlock(stream,d,flags);
			/* a real program would do something useful here with the data in p */
			/* instead we fool around with print statements */
			PrintAssignments(stdout,d,flags);
			printf("Now dimensioned as a");
			for(i=0;;i++){
				j=FindDescribedDim(d,&p->a,i,flags);
				if(j<=0)break;
				printf("[%d]",j);
			}
			printf(".\n");
		FreeDescribedPtrVars(d,flags);
	} while(!feof(stream));
	fclose(stream);
	FreeDescribedVars(d,flags);
	FreeDescriptions(d);
	if(0)moreTests();
}
void moreTests(void)
{
	static double logC,maskLogC,viewingDistance,*a,conditionLogC[4],conditionMaskLogC[4];
	short trials,i,flags;
	static char *kindOfTrial,*message,image[1000];
	unsigned char *m;
	Description *d;
	FILE *stream;
	static char filename[]="testAssign2";
	short error;
	static double b[2][2],c[1][2][3];
	
	flags=assignReportUnknown;
	
	printf("\n\n\nMore tests.\n");
	#ifdef _LUMINANCE_
		if(1){
			static luminanceRecord LR;
			
			// Test ReadLuminanceRecord.c
			if(PREFERENCES_FOLDER)error=OpenPreferencesFolder();
			i=ReadLuminanceRecord("LuminanceRecord1.h",&LR,flags);
			if(PREFERENCES_FOLDER)error=ClosePreferencesFolder();
			if(i<=0)PrintfExit("Couldn't read LuminanceRecord, error %d.\n",i);
			i=WriteLuminanceRecord("LR.h",&LR,flags);
			if(i<=0)PrintfExit("Couldn't write/verify LuminanceRecord, error %d.\n",i);
			printf("ReadLuminanceRecord.c seems to work fine. Created LR.h\n\n");
		}
	#endif
	#if 1
		/* parse testAssign2 */
		d=AllocateDescriptions(14);
		i=0;
		d[i++]=Describe(doublePtrType,&a,"a","array");
		d[i++]=Describe(unsignedCharPtrType,&m,"m","chars");
		d[i++]=DescribeArray(charType,&image,"image","Image data",100L,0L);
		d[i++]=Describe(stringType,&message,"message",NULL);
		d[i++]=Describe(doubleType,&logC,"logC","log contrast");
		d[i++]=Describe(doubleType,&maskLogC,"maskLogC",NULL);
		d[i++]=Describe(stringType,&kindOfTrial,"kindOfTrial",NULL);
		d[i++]=Describe(doubleType,&viewingDistance,"viewingDistance","inches");
		d[i++]=Describe(shortType,&trials,"trials",NULL);
		d[i++]=DescribeArray(doubleType,&b,"b",NULL,2L,2L,0L);
		d[i++]=DescribeArray(doubleType,&c,"c",NULL,1L,2L,3L,0L);
		assert(i<=14);								/* make sure array was big enough */
		d[i++]=NullDescription();					/* mark end of array */
		InitializeDescribedVars(d,0);
		stream=fopen(filename,"r");
		ReadAssignmentLine(stream,d
			,assignHexFloats|assignReportUnknown|assignEchoAssignments);
		FreeDescribedVars(d,0);
		fclose(stream);
		ReadAssignmentFile(filename,d,assignHexFloats|assignReportUnknown);
		PrintAssignments(stdout,d,0);
		FreeDescriptions(d);
	#endif
}
