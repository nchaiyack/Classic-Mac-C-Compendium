#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#ifdef CMLmake
#include "CML.h"
#include "CMLglobs.h"
#else
#include "MDL.h"
#include "MDLglobs.h"
#endif

#ifdef MAC
	#pragma segment directives
#else
	#include <search.h>
#endif

#ifdef MPW
#include <CursorCtl.h>
#endif

/* constants specific to here */
#define SecondsInDay 86400

/* macros specific to here */
#define parseTwo() (s+=2,(s[-2]-48)*10 + s[-1]-48)

/* Functions in this file: */

#ifndef DOS 
char *strlwr(char *);
#endif
void printDirectives(void);
int bcompare(void *arg1,void *arg2);
int processDirective(int);
int ignore(void);
int accession(void);
int openField(void);
static void updateHeader(struct segmentRec *);
int sequentialClose(void);
int closeField(void);
int endd(void);
int eofile(void);
int vang(void);
int vdist(void);
int nov(void);
int vangVdist(void);
int hiht(void);
int ccf(void);
int icf(void);
int addccf (void);
int declination (void);
int weight(void);
int convergence (void);
int degreescompass(void);
int degreesinclinometer(void);
int milscompass(void);
int milsinclinometer(void);
int up (void);
int sinusoidal_eccentricity (void);
int corrected (void);
int inches(void);
int meters(void);
int cflr(void);
int noprocess(void);
int virtual(void);
int cartesian(void);
/*	headers */
int reconstruction (void);
int date (void);
int pmug(void);
int project(void);
int compass (void);
int inclinometer (void);
int personnel (void);
int surveylength(void);
int location (void);
int in(void);
/* text blocks */
static int doBlock(int,indexType *);
static int skipBlock(int trigger);
int note(void);
int description(void);
int data(void);
int report(void);
int endreport(void);
int endreport(void);
int objective(void);
int personnelreport(void);
int eebreak(void);
int eeright(void);
int equipment(void);
int hazards(void);
int skills(void);
int route(void);
/* attributes */
int start(void);
int finish(void);
int shottype(void);
int finishshottype(void);
int spray(void);
/* extended elevation */


time_t parseDate(void);
unsigned short parseTime(void);
int parsePersonsName(char *);
int getJob(void);
/* MDL */
int closeShape(void);
int localDatum(void);
/* obsolete */
int group (void);
int CM_origin (void);
int continues (void);
int reverse(void);
int segment(void);
int nofsb(void);

#ifndef CMLmake
int fiduciary(void);
int localDatum(void);
int perimeter(void);
int xysum(void);
int contourArea(void);
int centroid(void);
int type(void);
int sectionThickness(void);
int tissueMag(void);
int edMag(void);
#endif

static char *parseName(void);
struct dbRec *findDb(indexType,int);
struct dbRec *newDb(indexType *,int);
c_float sumFields(void);
int getMonth();
time_t parseDate(void);
char *cleanString(char *,int);
int parsePersonsName(char *tok);
int getJob(void);
int searchJobList(void);

#ifndef DOS 
/* Make up for a nice function in MS C 5.1. Is this in Unix? */

	char *strlwr(char *string)
	{
		char *p;
		p=string;
		while(*string) {
			*string=tolower(*string);
			string++;
		}
		return p;
	}
#endif

void printDirectives(void)
{
	int i,k;
	char *p;
	
	if(OutFile) {
		fputs("Directives:  (preceed with a '#')\n",OutFile);
		p=(DirTable)->tok;
		k=strlen(p);
		fputs(p,OutFile);
		for(i=1;i<NumDirs;i++) {
			p=(DirTable+i)->tok;
			k+=strlen(p);
			if(k>75) {
				k=strlen(p);
				fputc('\n',OutFile);
			} else {
				fputs(",",OutFile);
				k++;
			}
			fputs(p,OutFile);
		}
		fputc('\n',OutFile);
	}
}

int bcompare(void *arg1,void *arg2)
{   
	return strcmp(((dirTablePtr)arg1)->tok,((dirTablePtr)arg2)->tok);
}

int processDirective(int pass)
{
	int i=0;
	dirTablePtr result;
	while((isalnum(*s)||*s=='.'||*s=='_'||*s=='/') && i<MaxDirLength-1) { 
		ThisDir.tok[i++]=tolower(*s);
		s++;
	}
	ThisDir.tok[i]='\0';
	result=(dirTablePtr) bsearch ((char *) &ThisDir,(char *)DirTable,
			NumDirs,DirEntryWidth,bcompare);
	if (!result) {
		if(pass&3)
			errorAlert(UnknownDirective);
		return 1;
	}
	if (result->code & pass)
		return ( (*(result->fp)) () );  /* Wow */
	else
		return 1;
}

int accession(void)
{
	if(ProcessHeaders) {
		clearSpace;
		if(*s=='=') {
			s++;
			clearSpace;
		}
		if(*s=='$') {
			s++;
			clearSpace;
		}
		s=cleanString(s,1);
		ThisContext->head.location = findName(s);
	}
	return 1;
}
	
int openField(void)
{
	int i,j;
	struct fieldRec *se;
	char *p;

#if 0	
	fprintf(OutFile,"openField. LineCnt: %d\n",LineCnt);
#endif
	switch (TheOpenField) {
		case -2:		/* Not processing. */
			return 1;
		case 0:			/* No previously open book but there still may be data. */
			if(Tentative.flag==0x1000) {
				if(ThisContext->pass==1) 
					updateMapping((LineOrder==ToVector)? 0:ThisContext->stationIncr);
				else
					updateString((LineOrder==ToVector)? 0:ThisContext->stationIncr);
			}
			memset(&(ThisContext->head),0,sizeof(struct headerRec));
			CompCor=Convergence;
			InclCor=0;
			CompassUnits=InclinometerUnits=0;
			SEFlag=InchFlag=AziCorrected=InclCorrected=FALSE;
			break;
		case -1:		/* Ignoring previous book. */
			break;
		default:		/* Prevous book not closed. */
			if(ThisContext->pass==1)
				errorAlert(NoCloseField);
			closeField();
			break;
	}  
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	clearSpace;
	if(*s=='=') {
		s++;
		clearSpace;
	}
	if(*s=='$') {
		s++;
		clearSpace;
	}
	p=s;
	if(!isdigit(*s)) {
		/* Accession number */
		p=0;
		cleanString(s,1);
		ThisContext->head.accession=findName(s);
		while(*s && !isdigit(*s)) s++;
	}
	TheOpenField=getPosNum();
	if(p && isgraph(*s)) {
		cleanString(p,1);
		ThisContext->head.accession=findName(p);
	}
	BackgroundField=0;
	FieldInitialURS=UnResSegList->current;
	LineOrder=FALSE;
	VdFlag=0;
	ShotMode=VANG;
	if(FieldLList) {
		se=(FieldList[TheOpenField])? getField(TheOpenField):newField(TheOpenField);
		CompCor-=se->corrAngle*RtoD;
	}
	checkEnd;
	if(FieldsToIgnore) {
		for(i=1;i<=*FieldsToIgnore;i++) {
			if((j=FieldsToIgnore[i])==TheOpenField) {
				TheOpenField=-1;
				skipBlock(0x1000);
				return 0x1000;
			}
			if(j<0) {	/* Indicating a range */
				++i;
				if((abs(j)<=TheOpenField) && (FieldsToIgnore[i]>=TheOpenField)) {
					TheOpenField=-1;
					skipBlock(0x1000);
					return 0x1000;
				}
			}
		}
	}
	return 0x1000;	/* openField() */
}

int ignore(void)
{
	short *p;
	if(!FieldsToIgnore)
		FieldsToIgnore=ecalloc(64,sizeof(short));
	do {
		clearSpace;
		if(*s=='=') {
			s++;
			clearSpace;
		}
		if(*s==',') {
			s++;
			clearSpace;
		}
		if(*s=='$') {
			s++;
			clearSpace;
		}
		if(*s && !isdigit(*s)) {
			errorAlert(FieldExpected);
			return 1;
		}
		if(*FieldsToIgnore>61) {
			errorAlert(IgnoringTooMany);
			return 1;
		}
		p=FieldsToIgnore + ++*FieldsToIgnore;
		*p=getPosNum();
		*(p+1)=0;
	} while(*p);
	return 1;
}

int segment(void)
{
	int temp;
	
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	FieldInitialURS=UnResSegList->current; 
	ShotMode=VANG;
	VdFlag=0;
	VirtualFlag=LineOrder=FALSE;
	
	if(NoFSBFlag && TheOpenDesig[0] && HeaderList && ThisContext->pass==1)
		updateHeader(lookUpSR(TheOpenField,TheOpenDesig));
	clearSpace;
	if(*s=='=') {
		s++;
		clearSpace;
	}
	if(isdesigstart(*s)) {
		for(temp=0;temp<DesigLen;temp++) 
			TheOpenDesig[temp]=(isdesig(*s))? *s++:0;
	} else {
		if(*s=='$') {
			s++;
			clearSpace;
		}
		if(!isdigit(*s)) {
			if(ThisContext->pass==1)
				errorAlert(FieldExpected);
			return 1;
		}
		temp=getPosNum();
		if(!BackgroundField)
			BackgroundField=TheOpenField;
		/* #segment takes precedence over #openField, but we keep the
			#openField number around anyway. */
		if(ThisContext->pass==1 && TheOpenField!=temp)
			errorAlert(NotSameField);
		TheOpenField=temp;
	}
	return 1;
}

int nofsb(void)
{
	TheOpenField=100;
	NoFSBFlag=TRUE;
	return 1;
}

static void updateHeader(struct segmentRec *seg)
{
	struct headerRec *head;
	
	if(seg) {
		if(seg->header)
			head=getHeader(seg->header);
		else {
			head=getNextHeader();
			seg->header=HeaderList->current;
		}
		if(head->date && head->date!=ThisContext->head.date)
			errorAlert(TwoDates);			/* Warning */
		*head=ThisContext->head;
	}
	return;
}

int sequentialClose(void)
{
	if(CloseFlag) {
		simClose(ClosureSteps,ClosureTolerence);
		CloseFlag++;	/* >1 signals use of #close directive */
	}
	return 1;
}

int closeField(void)
{
	struct segmentRec *theURSegment;
	struct unResSegRec *theURS;
	int i,u,n;
	
	switch (TheOpenField) {
		case -2:		/* Not processing. */
			return 1;
		case -1:		/* Ignoring previous book. */
			return 0x1000;
			break;
		case 0:			/* No open book, but there still may be data. */
			if(ThisContext->pass==1)
				errorAlert(NoOpenField);
			break;
		default:		/* Normal case, there is a book open. */
			if(FieldList[TheOpenField-MinField]) {
				if(ThisContext->pass==1 && HeaderList)
					updateHeader(getSR(FieldList[TheOpenField-MinField]));

				if(FieldInitialURS < UnResSegList->current) {
					/* This is convoluted. Watch carefully: */
					for(i=FieldInitialURS+1;i<=UnResSegList->current;i++) {
						theURS = getURS(i);			/* For each URS in this book */
						if(theURS->resSegment < 0) {	/* If p1 resolution allowed */
							theURSegment = getSR(u=theURS->segment); 	/* get its p1SR. */
							if(n=findSegment(TheOpenField,theURSegment->desig)) {
								theURS->resSegment = n;
								theURSegment->segment = n;
								/* merge segment data */
								ResolvedSegments++;
							}
						}
					}
				}
			} else
				errorAlert(NoDataEntered);
			break;
	}
	if(Tentative.flag==0x1000) {
		if(ThisContext->pass==1) 
			updateMapping((LineOrder==ToVector)? 0:ThisContext->stationIncr);
		else
			updateString((LineOrder==ToVector)? 0:ThisContext->stationIncr);
	}

	TheOpenField=BackgroundField=0;
	CompassUnits=InclinometerUnits=0;
	SEFlag=InchFlag=AziCorrected=InclCorrected=FALSE;
	FieldInitialURS = UnResSegList->current;
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	/* Null the current heading fields. */
	memset(&(ThisContext->head),0,sizeof(struct headerRec));
	CompCor=Convergence;
	InclCor=0;
	ShotMode=VANG;
	VdFlag=0;

	VirtualFlag=LineOrder=FALSE;
	
	return 0x1000;	/* closeField() */
}

int endd(void)
{
	if(Tentative.flag==0x1000) {
		if(ThisContext->pass==1) 
			updateMapping((LineOrder==ToVector)? 0:ThisContext->stationIncr);
		else
			updateString((LineOrder==ToVector)? 0:ThisContext->stationIncr);
	}
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	LineOrder=FALSE;
 	return 0xFFF;		/* #end also ends a skipBlock. */
}

int eofile(void)	/* stop reading a file now */
{
	int err=0;
	
	if(ThisContext->file) {
		if(StreamIsFile)
			err=fseek(ThisContext->file,0L,SEEK_END);
		else
			ThisContext->file->_cnt=0;
		if(err)
			errorAlert(DiskErr);
	}
	return endd();
}

int vang(void)	/* default. 3rd field is inclination. */
{
	ShotMode=VANG;
	VdFlag=0;
	return 1;
}

int vdist(void)	/* vertical distance only. 3rd field is vdist.*/
{
	ShotMode=VDIST;
	VdFlag=1;
	return 1;
}

int level(void) /* leveling survey where distance is slope, no vang. */
{
	ShotMode=LEVEL;
	VdFlag=1;
	return 1;
}

int nov(void)	/* no verticals: 2 data fields. */
{
	ShotMode=NOV;
	VdFlag=0;
	return 1;
}

int vangVdist(void)	/* inclinations and vertical angle: 4 data fields. */
{
	ShotMode=VAD;
	VdFlag=1;
	return 1;
}

int hiht(void)	/* instrument-height and target-height and vertical angle */
{
	ShotMode=VAD;
	VdFlag=2;
	return 1;
}

int sinusoidal_eccentricity (void)
{ 	
	clearSpace;
	SError = getFloat();
	clearSpace;
	SWorstAngle = getFloat();
	SEFlag=TRUE;
	return 1; 
}

int localDatum(void)
{
	/* expects a station name. */
	
	clearSpace;
	if(*s=='$') {
		s++;
		Left.flag=1;
		parseSegment(&Left);
		assignSegment(Left,Field,NoP1Resolve);
	} else if(isalpha(*s)) {
		Left.flag=2;
		parseSegment(&Left);
		assignSegment(Left,Field=TheOpenField,Resolve);
	} else if(isdigit(*s)) {
		Left.flag=3;
		Left.this.n = getPosNum();
		Left.Field=TheOpenField;
		Left.this.segment = Previous.this.segment;
	} else
		errorAlert(MissingDirField);
	ThisContext->Field.localDatum=Left.this;
	Left.flag=0;
	return 	1;
}

int CM_origin (void)
/*	The Crowther-Mann datum is south of Cleaveland Avenue at
	Lat 37 10 00, Long 86 05 00
	Add 2375 Easting and -305 Northing to convert to
	the TT8W datum.
*/
{	
	EOffset=2375;
	NOffset=-305;
	AOffset=0;	/* Altitude is still feet above mean sea level. */
	return 1; }

/*------------------------ unimplemented ------------------------*/

int group (void)
{ 	return 1; }
	
int continues (void)
{	return 1; }

int cartesian(void)
{ 	return 1; }

int compass (void)
{
	if(ProcessHeaders) {
		s=cleanString(s,0);
		ThisContext->head.compass = findName(s);
	}
	return 1;
 }

int inclinometer (void)
{
	if(ProcessHeaders) {
		s=cleanString(s,0);
		ThisContext->head.inclinometer = findName(s);
	}
	return 1;
 }

int degreescompass(void) 
{ 	
	CompassUnits=0;
	return 1; 
}

int degreesinclinometer(void) 
{ 	
	InclinometerUnits=0;
	return 1; 
}

int milscompass(void) 
{ 	
	return CompassUnits=1;
}

int milsinclinometer(void) 
{ 	
	return InclinometerUnits=1;
}

int reverse (void)
{	return 1; }

/*------------------------------- Attributes -------------------------------

	#start xxx yyy ... #finish xxx brackets stations within a survey that are
	of class xxx and property yyy, i.e. #start floor sand. As classes may be 
	properties in themselves  (i.e. #start 'Gypsum Flowers') it is not 
	nessessary to have a yyy field.
	
----------------------------------------------------------------------------*/

int start(void)
{
	int className;
	struct dbRec *attribute;
	
	if(ProcessHeaders&&DbList) {
		if(!(className=findName(parseName()))) {
			errorAlert(MissingDirField);
			return 1;
		}
		
		/*	Got a db record now with link properly filled and className in what. */
		attribute=newDb(&(ThisContext->head.attributes),className);
		/*	Following station is where the attribute starts. */
		attribute->where.sta=Previous.this;
		attribute->where.sta.n+=ThisContext->stationIncr;

		attribute->finish.sta.segment = findName(parseName()); /* findName returns 0 if passed empty string */
		attribute->finish.sta.n = 0;
	}
	return 1;
}

int finish(void)
{
	int className;
	struct dbRec *attribute;

	if(ProcessHeaders&&DbList) {
		if(!(className=findName(parseName()))) {
			errorAlert(MissingDirField);
			return 1;
		}
		if(attribute=findDb(ThisContext->head.attributes,className)) 
			attribute->finish.sta.n = Previous.this.n;
		else
			errorAlert(MissingDirField);
	}
	return 1;
}

int shottype(void) {	/* same shottypes as SMAPS */
	clearSpace;
	switch (*s) {
		case 'L':
		case 'l':
			ThisContext->shottype='L';
			ExcludeFlag=-1;	/* length exclusion */
			break;
		case 'P':
		case 'p':
			ThisContext->shottype='P';
			VirtualFlag=TRUE;	/* plot exclusion */
			break;	
		case 'X':
		case 'x':
			ThisContext->shottype='X';	/* don't process */
			TheOpenField=-2;	/* notProcessing flag for #openField and #closeField */
			skipBlock(0x4000);
			break;
		case 'C':
		case 'c':		/* Contrain */
			break;	
		case 0:
		case '\n':
			errorAlert(MissingDirField);
			break;
		default:
			break;
	}
	return 1;
}

int finishshottype(void) {
	switch (ThisContext->shottype) {
		case 'L':
			ExcludeFlag=FALSE;
			break;
		case 'P':
			VirtualFlag=FALSE;
			break;
		case 'X':
			TheOpenField=0;
			ThisContext->shottype=0;
			return 0x4000;
		default:
			break;
	}
	ThisContext->shottype=0;
	return 1;
}

int spray(void)
{
	return ExcludeFlag=1;
}

c_float sumFields(void)
{
	c_float cor=0;
	clearSpace;
	while(*s && (*s!='\n')) {
		cor+=getFloat();
		clearSpace;
	}
	return cor;
}

int ccf (void) 
{
	CompCor=Convergence-sumFields();
	ThisContext->head.fcc=ThisContext->head.bcc=floor(CompCor*100+0.05);
	return 1; 
}

int icf (void) 
{
	InclCor= -sumFields();
	ThisContext->head.fic=ThisContext->head.bic=floor(InclCor*100+0.05);
	return 1; 
}
	
int addccf (void) 
{ 
	CompCor+=sumFields();
	ThisContext->head.fcc=ThisContext->head.bcc=floor(CompCor*100+0.05);
	return 1; 
}
	
int declination (void) 
{ 
	clearSpace;
	CompCor=Convergence+getFloat();
	ThisContext->head.decl=ThisContext->head.fcc=ThisContext->head.bcc=floor(CompCor*100+0.05);
	return 1;
}

int weight(void)
{
	c_float weight;
	clearSpace;
	if((weight=getFloat())==0.0)
		Light=0.5;
	else if(weight>=1.0)
		Light=1/weight;
	else
		Light=1-weight;
	Heavy=1-Light;
	return 1;
}

int convergence (void)
{
	clearSpace;
	Convergence=getFloat();
	CompCor+=Convergence;
	ThisContext->head.decl=ThisContext->head.fcc=ThisContext->head.bcc=floor(CompCor*100+0.05);
	return 1;
}

int corrected (void)
{
	AziCorrected=(ThisDir.tok[0]=='n')? FALSE:TRUE;
	InclCorrected=(ThisDir.tok[0]=='n')? FALSE:TRUE;
	return 1;
}

int up (void) {   /* and down */
	clearSpace;
	DVertical = (ThisDir.tok[0]=='u')? getLength() : -getLength();
	return 1; 
}

int noprocess(void) {
	if(ThisDir.tok[0]=='n')	{	/* noprocess */
		TheOpenField=-2;		/* Kludge flag for #openField and #closeField */
		skipBlock(0x2000);
	} else {
		TheOpenField=0;
		return 0x2000;
	}
}

int meters(void) { /* and feet */
	MeterFlag=(ThisDir.tok[0]=='m');
	InchFlag=FALSE;
	return 1;
}

int inches(void) {
	InchFlag=(ThisDir.tok[0]=='i');
	return 1;
}

int cflr(void) { /* and lrcf */
	RotateDimensions=(ThisDir.tok[0]=='c')?2:0;
	return 1;
}

int virtual(void) {	/* and show */
	char *s;
	s=ThisContext->s0;
	clearSpace;
	s++;
	if(ThisDir.tok[0]=='s') {	/* for "show" */
		VirtualFlag=FALSE;
		ShotFlag&=~Virtual;
	} else {
		VirtualFlag=TRUE;
		ShotFlag|=Virtual;
	}	
	return 1;
}

int closeShape(void) {
	LinkRecPtr theLink;
	struct locnRec *theLocn;
	
	if(ThisContext->pass==1) {
		theLink=nextFromList(P1LList,linkCast);
		theLink->to.segment=theLink->from.segment = Previous.this.segment;
		theLink->to.n=1;
		theLink->from.n=(LineOrder==ToVector)?Previous.this.n:Previous.this.n+ThisContext->stationIncr;
		theLocn=nextFromList(P1LocnList,locnCast);
		if((P1LocnList->current & BlockLo) == BlockLo) 
			/* Don't split the two records of a tie accross a block. */
			theLocn=nextFromList(P1LocnList,locnCast);
		theLocn->code=0;
		theLocn->length=0;	/* not yet known */
		theLocn=nextFromList(P1LocnList,locnCast);
		theLocn->code=CloseShape;
		theLocn->length=0;
		theLink->data = P1LocnList->current;
	}
	endd();
	return 1;
}

#ifdef MAC
#pragma segment headers
#endif

/*-------------------------- Header Directives --------------------------*/

int reconstruction (void) {
	if(ProcessHeaders) {
		s=cleanString(s,0);
		ThisContext->head.object = findName(s);
	} 
	return 1;
}

int location (void) {
	if(ProcessHeaders) {
		s=cleanString(s,0);
		ThisContext->head.location = findName(s);
	}
	return 1;
}
	
int project(void)
{
	if(ProcessHeaders) {
		s=cleanString(s,0);
		ThisContext->head.project = findName(s);
	} 
	return 1;
}

int date (void) 
{
	if(ProcessHeaders) {
		clearSpace;
		ThisContext->head.date=parseDate();
	}
	return 1;
}

int in(void)
{
	if(ProcessHeaders) {
		clearSpace;
		if(ThisDir.tok[0]=='i')
			ThisContext->head.in=parseTime();
		else {
			ThisContext->head.out=parseTime();
		}
	}
	return 1;
}

unsigned short parseTime(void)
{
	/* returns time as minutes past midnight */
	unsigned short minutes=0;
	int err=TRUE;
	
	if(isdigit(*s)) {
		minutes=60*getPosNum();
		err=FALSE;
	}
	if(*s==':') {
		s++;
		if(isdigit(*s)) {
			minutes+=getPosNum();
			err=FALSE;
		}
	}
	clearSpace;
	if(tolower(*s)=='p')
		minutes+=720;		/* pm */
	else if(tolower(*s)=='a') {
		if(minutes>=720)	/* 12:01 am is 0:01 */
			minutes-=720;
	} else if(tolower(*s)=='m') { 
		if(minutes && minutes!=720)
			err=TRUE;	/* Only 12:00 or 0:00 midnight okay. */
		minutes=0;
	}
	if(err || minutes>=1440) {
		errorAlert(BadTime);
		minutes=0;
	}
	return minutes;
}

int pmug(void)		/*	person-MINUTES, max is 1092 hours  */
{	
	c_float minutes;
	clearSpace;
	if((minutes=getFloat())<0)
		errorAlert(BadTime);
	else
		ThisContext->head.PMUG=(unsigned short)(60.0*minutes);
	return 1;
}

int fsb(void)
{
	clearSpace;
	ThisContext->head.FSB=getPosNum();
	return 1;
}

int surveylength(void)
{	
	c_float length;
	clearSpace;
	length=getLength();
	if(ThisDir.tok[0]=='n')
		ThisContext->head.newlength=(unsigned short)(10.0*length);
	else
		ThisContext->head.replacedlength=(unsigned short)(10.0*length);
	return 1;
}

/*------------------------------ Text Blocks ----------------------------*/
	
/*------------------------------------------------------------------------
	Notes are text lines or blocks of text put into the permanent CML/MDL
	data record that are also to be printed out in the logsheet pages. 
	
	No internal (including tempfile) copy of notes are made. Instead, a 
	reference to the position of the note in the input file is stored in
	a dbRec. The note is retrieved directly from the input file by 
	printLog. A trigger of 128 indicates a generic text block that may
	or may not be bracketed by #begin...#end.
	
	The triggers for notes and skipBlocks are:
	
		0x1000	ignore
		0x2000	#noprocess
		0x4000	#shottype X
		
		128		#note
		128		#description
		128		#data
		32		#report
		128		#personnelreport
		
-------------------------------------------------------------------------*/

#define getSingleChar(f) getc(f)

int doBlock(int trigger,indexType *ptr)
{
	struct dbRec *db;
	int n,ret;
	if(ProcessHeaders&&DbList&&ThisContext->pass==1) {
		db=getNextDb();
		n=DbList->current;
		db->link=*ptr;	/* May be null. */
		*ptr=n;			/* Simple linked list. */
		db->what=FileListPosn;
		db->where.item=ftell(ThisContext->file)-strlen(ThisContext->s0)+strlen(ThisDir.tok)+2;
		db->finish.item=0;
		if(trigger==128) {
			_clearSpace;
			if(strncmp(s,"#begin",6))
				return 1;
			db->where.item+=7;
		}
		ret=skipBlock(trigger);
		db=getDb(n);
		db->finish.item=ftell(ThisContext->file)-strlen(ThisContext->s0);
	} else if(trigger==128) {
		_clearSpace;
		if(strncmp(s,"#begin",6))
			return 1;
		ret=skipBlock(trigger);
	}
	return ret;
}

static int skipBlock(int trigger)
{
	int ret,doLines,c;
	FILE *theFile;
	struct lineRec *theLine;
	
	theFile=ThisContext->file;	/* local copies for speed. */
		
	if(doLines=(ThisContext->pass==2 && FullListing)) {		
		if(abs(LineList->current-LineCnt)>1)
			errorAlert(UnexpectedError);
		LineList->current=LineCnt+1;	/* When called from the top level, p2Bottom has not
										yet been visited and ->current lags by 1. */
		theLine=(struct lineRec *)getLListBuf(LineCnt,LineList)+(LineCnt&MedBlockLo);
		theLine->state=1;
		theLine=(struct lineRec *)getLListBuf(LineCnt+1,LineList)+((LineCnt+1)&MedBlockLo);
		theLine->state=3;
	}
	LineCnt++;
	while((c=getSingleChar(theFile))!=EOF) {
		if(c=='\n') {
			LineCnt++;
			if(doLines) {
				if(++LineList->current&MedBlockLo)
					theLine++;
				else
					theLine=getLListBuf(LineList->current,LineList);
				theLine->state=3;
			}
			RotateCursor(LineCnt);
		} else if(c=='#') {
			s=getLine(ThisContext->lastLine+1,MaxInLine,theFile);
			ThisContext->lastLine=ThisContext->s0;
			ThisContext->s0=s-1;
			*(s-1)='#';
			ret=processDirective(4);
			if(ret&trigger) {
				if(doLines)
					theLine->state=1;
				return ret;
			}
			if(*(s+strlen(s)-1)=='\n') {
				LineCnt++;
				if(doLines) {
					if(++LineList->current&MedBlockLo)
						theLine++;
					else
						theLine=getLListBuf(LineList->current,LineList);
					theLine->state=3;
				}
			}
		}
	}
	errorAlert(NoEnd);
	return 0;
}

int note(void)
{
	return doBlock(128,&(ThisContext->head.notes));
}


int description(void)
{
	return doBlock(128,&(ThisContext->head.description));
}

int data(void)
{	
	return doBlock(128,&(ThisContext->head.data));
}

int report(void)
{	
	/* Not a text block. */
	if(ProcessHeaders) {
		clearSpace;
		ThisContext->head.report=parseDate();
		if(*s=='.') {
			s++;
			if(isdigit(*s))
				ThisContext->head.report+=getPosNum();
		}
	}
	return 1;
}

int endreport(void)
{
	struct headerRec *head;
	if(ProcessHeaders) {
		if(TheOpenField) {
			;
		} else {
			head=getNextHeader();
			*head=ThisContext->head;
			memset(&(ThisContext->head),0,sizeof(struct headerRec));	
		}
	}
	return 1;
}

int objective(void)
{
	return doBlock(128,&(ThisContext->head.objective));
}

int personnelreport(void)
{	
	return doBlock(128,&(ThisContext->head.personnelreport));
}

int equipment(void)
{
	return doBlock(128,&(ThisContext->head.equipment));
}

int hazards(void)
{
	return doBlock(128,&(ThisContext->head.hazards));
}

int skills(void)
{
	return doBlock(128,&(ThisContext->head.skills));
}

int route(void)
{
	return doBlock(128,&(ThisContext->head.route));
}

/*------------------------- Parsing People's Names --------------------------

	Each name can take up to three tokens, first name, last name, and job.
	The order is first name, last name. If there is only one name, it is 
	the last name and it must be followed by a comma or paretheses to 
	delineate it. 
	
	Jobs are enclosed in paretheses, except for the asterisks, denoting 
	party leader, which may optionally appear after the last name.
	
	segment->personnel is an index to a partyRec. party->name is an index 
	into the NameList, the same one used for all proper nouns. party->data 
	is a bit field that flags that person's jobs for that segment plus some 
	extraneous flags left over from the name parse. The last entry for a 
	segment is flagged with the LastInParty bit.
----------------------------------------------------------------------------*/

int personnel (void) 
{
	int i;
	char tok[2][NameLength],*thePerson;
	int term[2];
	struct dbRec *people;
	int party;
	
	/* 	Processing #personnel is complex and is avoided
		unless specifically called for by the user. */

	if(!DbList) {
		ThisContext->head.personnel=0;
		return 1;
	}	
	party=DbList->current; /* get linked list index now */
	_clearSpace;
	do {
		for(i=0;i<2;i++) {
			if((term[i]=parsePersonsName(tok[i]))&LastName)
				break;
		}
		if(i==2) i--;		/* Didn't get a clear last name. */
		if(i) {				/* First name and last name */
			if((strlen(tok[0])+strlen(tok[1])-1)>NameLength) {
				errorAlert(NameTooLong);
				thePerson=tok[1];
			} else {
				strcat(tok[1],"_");
				/* names stored as Smith_Andy */
				thePerson=strcat(tok[1],tok[0]);
			}
		} else
			thePerson=tok[0];
		/*	By now, have constructed a string in thePerson, that is one 
			party member's name. */
		people = getNextDb();
		people->what = findName(thePerson);
		people->where.item=(term[i]&Job)? (term[i]|=getJob()):term[i];
		_clearSpace;
	} while(isalpha(*s));
	people->where.item|=LastInParty;		/* Linear list, this ends it. */
	ThisContext->head.personnel=party+1;	/* from PartyList->current at beginning of parse. */
	return 1;
}

static char *parseName(void)
{
	int i=0;
	static char tok[NameLength];
	
	_clearSpace;
	if(*s=='\'') {
		s++;
		while(*s && *s!='\'' && i<NameLength) {
			*(tok+i++)=*s++;
		}
		if(*s) s++;
	} else {
		while(isalnum(*s) && i<NameLength)
			*(tok+i++)=*s++;
	}
	*(tok+i)=0;
	if(i==NameLength) {
		errorAlert(NameTooLong);
		while(*s && !isspace(*s)) s++;
	}
	return tok;
}

struct dbRec *newDb(indexType *lastLink,int name)
{
	struct dbRec *db;
	for(;*lastLink;lastLink=&(db->link)) {
		db=getDb(*lastLink);
		if(db->what>name)
			break;
	}
	db=getNextDb();
	db->what=name;
	db->link=*lastLink;
	*lastLink=DbList->current;
	return db;
}

struct dbRec *findDb(indexType link,int name)
{
	struct dbRec *db;
	for(;link;link=db->link) {
		db=getDb(link);
		if(db->what==name)
			return db;
		if(db->what>name)
			return 0;
	}
	return 0;
}

time_t parseDate(void)
{
	int n;
	int year = DefaultCentury;
	int month=0,day=0;
	int tok1,tok2,tok3;
	char *p;
	long ndays;
	for(n=0;n<2;n++) {
		/* Allow up to two tries at an alpha month. */
		if(isalpha(*s)) {
			if((month=getMonth())>0) {
				while(*s && !isdigit(*s)) s++;
				day=getPosNum();
				while(*s && !isdigit(*s)) s++;
				year=getPosNum();
				if(year<1800) year+=DefaultCentury;
				n=3;
			} else if(month==0) {
				errorAlert(BadDate);
				return 0;
			} else {	/* Was a weekday. */
				while(isalpha(*s)) s++;
				while(*s && !isalnum(*s)) s++;
			}
		} else break;
	}
	if(*s && (n<3)) {
		/* coming here, we are still looking for a full date. */
		p=s;
		n=0;
		while(isdigit(*p)) { p++; n++; }
		switch(n) {
			case 8:
				year = 100*parseTwo();
				if(abs(year-DefaultCentury)>100) {	/* 12251985 */
					month=year;
					day=parseTwo();;
					year=100*parseTwo();
					year+=parseTwo();
				} else {							/* 19851225 */
					year+=parseTwo();
					month=parseTwo();
					day=parseTwo();
				}
				break;
			case 6:
				tok1 = parseTwo();
				tok2 = parseTwo();
				tok3 = parseTwo();
				goto fixOrder;

			case 4:
				year = 100*parseTwo();
				year += parseTwo();
				while(*s && !isalnum(*s)) s++;
				if(isalpha(*s)) {
					month = getMonth();
					day = tok1;
					tok1 = 0;
					while(*s && !isdigit(*s)) s++;
					if((year = getPosNum())<1800)
						year += DefaultCentury;
					break;
				} 
				month = getPosNum();
				while(*s && !isdigit(*s)) s++;
				day = getPosNum();
				break;	
			case 2:
			case 1:
				tok1 = getPosNum();
				while(*s && !isalnum(*s)) s++;
				if(isalpha(*s)) {
					month = getMonth();
					day = tok1;
					while(*s && !isdigit(*s)) s++;
					if((year = getPosNum())<1800)
						year += DefaultCentury;
					break;
				} 
				tok2 = getPosNum();
				while(*s && !isdigit(*s)) s++;
				tok3 = getPosNum();
fixOrder:		if(tok1>31) {
					year = (tok1 <1800)? tok1+DefaultCentury : tok1;
					month = tok2;
					day = tok3;
				} else {
					day = tok2;
					month = tok1;
					year = (tok3 <1800)? tok3+DefaultCentury : tok3;
				}
				break;
			default:
				errorAlert(BadDate);
				return 0;
		}
		
	}
	/* Is what we have valid? */
	if(month>12 || day>31 || !month || !day || !year || (year>2040)) {
		errorAlert(BadDate);
		return 0;
	}

	/* convert to internal time */
	ndays = (year-=1904)/4;
	ndays += 365*(year);
	ndays += DaysToMonth[month-1] + day;
	if(!(year%4) && (month<=2))
		ndays--;
	return (SystemOffset + ndays*SecondsInDay + SecondsInDay/2);	/* noon, takes care of leap-seconds */
}

int getMonth(void)
{
	int high=18,low=0;
	static struct monthRec monthArray[] = {{"ap",2,4},{"au",2,8},{"d",1,12},{"fe",2,2},{"fr",2,-6},{"ja",2,1},
	{"jul",3,7},{"jun",3,6},{"mar",3,3},{"may",3,5},{"mo",2,-2},{"n",1,11},
	{"o",1,10},{"sa",2,-7},{"se",2,9},{"su",2,-1},{"th",2,-5},{"tu",2,-3},{"w",1,-4}};

	struct monthRec *element;
	strlwr(s);
	for(low=0;low<high;low++) {
		element=monthArray+low;
		if(!strncmp(s,element->name,element->length))
			return element->month;
	}
#if 0
	while (low<=high) {
		mid=(high+low)/2;
		element=monthArray+mid;
		if((month=strncmp(s,element->name,element->length))>0) 
			low = mid-1;
		else if(month<0)
			high = mid-1;
		else
			return element->month;
	}
#endif
	return 0;
}

char *cleanString(char *s,int how)
{
	/* 
	Preprocess name strings, capitalizing words (all else lower
	case). This is done in place. 
	*/
	char *start,*to;	/* The global s is from. */
	
	_clearSpace;		/* Ignore leading space. */
	start=s;			/* Will return first non-blank character. */
	while(!isalpha(*s))
		s++;
	to=s;
	if(how)
		*to=toupper(*to); 			/* Keep case of first letter of each word. */
	to++;
	do {
		s++;
		while(*s && !isspace(*s)) {
			*to++=(how)?toupper(*s):tolower(*s);
			s++;	/* Never increment inside other people's macros. */
		}
		if(*s) {
			/* reduce white space to one space */
			*to++ =' ';
			_clearSpace;
		}
	} while(*to++=(how)?toupper(*s):*s);/* Keep case of first letter of each word. */
	/* delete trailing white space */
	s-=1;
	while(isspace(*s)) *s-- = 0;
	return start;
}

/*-------------------------- parsePersonsName -------------------------
	This is for people. It parses the global input string (*s)
	collecting characters that would be legal in a person's 
	name (i.e. O'Brian, Worthington-Smith) into tok[]. It looks
	ahead to try to figure out if this is a last name and if
	it is followed by a job field. It returns a code telling what
	kind of name it has found.
--------------------------------------------------------------------*/

int parsePersonsName(char *tok)
{
	int i=0,terminator=0;
	while(isalpha(*s)||(*s=='\'')||(*s=='-')) {
		*(tok+i++) = tolower(*s);
		s++;
	}
	*(tok+i) = 0;
/*	if(i==1)
		terminator=FirstName; */
	_clearSpace;
	switch(*s) {
		case '*':
			terminator=LastName+Leader;
			s++; _clearSpace;
			if(*s==',') {
				s++; 
				_clearSpace;
				break;
			} 
			if(*s!='(')
				break;
		case '(':
			terminator|=LastName+Job;
			break;
		case ',':
			terminator=LastName;
			s++; _clearSpace;
			break;
		case '.':
			s++; _clearSpace;
			break;
		default:
			break;
	}
	*tok = toupper(*tok);	/* capitalize name. */
	return terminator;
}

/*------------------------- parse job field -------------------------*/
int getJob(void)
{
	int jobs=0,j;
	if(*s!='(')
		return 0;
	s++;
	_clearSpace;
	do {
		if(*s=='*') {	/* Special case for jobs */
			jobs |= Leader;
			s++;
		} else if(j=searchJobList())	/* General purpose lookup */
			jobs |= j;
		else {
			errorAlert(UnknownJob);
			while(isalnum(*s)) s++;
		}
		_clearSpace;
		if(*s==',' || *s=='/') { s++; _clearSpace; }
	} while(*s && *s!=')') ;
	if(*s) {
		s++; 
		_clearSpace;
		if(*s==',') { s++; _clearSpace; }
	} else
		errorAlert(NoRtParen);
	return jobs;
}

/*
	Job categories should be:
	Leader:   L, *, Leader
	Book:     B, Book
	Tape:     T, Ch, Tape, Chain
	Compass:  Co,Instrument,I
	
	They should be parsed, until the closing ')' is encountered. Allowable
	separators are space, tab, slash, and comma. Allowable category names
	are alphanums. 
*/

/*	
	This routine does a lookup into an array of struct listRecs and returns 
	the code field of a successful lookup. It is sufficient to match just
	the first character of the name, allowing for single-letter abbreviations
	of fields. 
*/

int searchJobList(void)
{
	Str31 job;
	char *p=job,*q;
	int i,j;
	
	for(q=s;isalnum(*s);s++) {
		*p++ = tolower(*s);
	}
	*p='\0';
	for(j=i=0;i<NumJobs;i++) {
		if(strcmp(job,(JobTable+i)->name)) {
			if(!j && *job==*((JobTable+i)->name))
				j=(JobTable+i)->code;
		} else
			return((JobTable+i)->code);
	}
	s=(*q)?q+1:q;
	return j;
}

/*--------------- Extended Elevation ------------------*/

int eebreak(void)
{
	int k;
	
	if(EEFlag) {
		s=ThisContext->s0;
		k=sprintf(s,"\'break%d\' ..\n",abs(EEFlag));
		EEFlag+=(EEFlag>0)? 1:-1;
		Left.flag=5;
		Previous.flag=0x400;
		if(ThisContext->pass==1)
			findLit(&Left);
		else if(ThisContext->pass==2)
			lookUpLit(&Left);
		else
			return 1;
		s+=(k-1);
		return 0;
	}
	return 1;
}

int eeright(void)
{	
	if(ThisDir.tok[2]=='r')
		ShotFlag&=~EELeftFlagBit;
	else
		ShotFlag|=EELeftFlagBit;
	return 1;
}

#ifndef CMLmake
/*----------------------- Reconstruction Directives ---------------*/

int fiduciary(void)
{ 	
	struct locnRec *lo;
	LinkRecPtr li;
	struct fieldRec *se;
	int n;
	
	/*	
		Fiduciary marks are represented in their own segment, generally
		one per section. In p1, they are stored as locns but with two
		locnRecs reserved so they can be made into ties. In p1.5, when
		next and last sections are known, the master fid can be found
		(it becomes the section's local datum) and the tie between it
		and the last section and the ties to the other fids made.
		
		There are three doubling schemes for fids:
		
		1. 	mininum: # linkRecs = # fids for each section. Fids are not
			doubled (their ties are not bidirectional).
		2. 	middle: #lRs = 2 * #fids. Bidirectional ties from master to
			others, and from master down and up to the next sections
			master.
		3.	maximum: #lRs = 2 * #fids * 2*(#fids-1). As above, but add
			linkRecs for ties of each fid up and down.
			
		Doubling is implemented at p1.5. It is questionable whether
		doubling is necessary as the closure matrix could be filled
		to reflect the concept of relative locns.
		
		This is a pass 1 directive only.
	*/
	
	se = getField(TheOpenField);
	if(!se->fidSegment) {
		if (TheOpenField<MinField) 
			MinField=lowerList(TheOpenField);
		else if (TheOpenField>MaxField)
			MaxField=raiseList(TheOpenField);
		se->fidSegment=newSegmentRec(TheOpenField,"fid",FieldList+TheOpenField-MinField);
	}

	clearSpace;
	if(isdigit(*s)) 
		n=getPosNum();
	else {
		errorAlert(MissingDirField);
		return 1;
	}
	
	
	li=nextFromList(P1LList,linkCast);
	li->from.n = 0;
	li->to.n=n;
	li->from.segment=li->to.segment=se->fidSegment;
	se->nFids++;
	
	ThisContext->localDatum.segment=se->fidSegment;
	ThisContext->localDatum.n=0;
	
	UsingFids=TRUE;
	lo=nextFromList(P1LocnList,locnCast);
	if((P1LocnList->current & BlockLo) == BlockLo) 
		/* Don't split the two records of a tie accross a block. */
		lo=nextFromList(P1LocnList,locnCast);
	lo->code=0;

	lo=nextFromList(P1LocnList,locnCast);
	li->data=P1LocnList->current;
	clearSpace;
	if(*s==',') {s++;clearSpace;}
	lo->data.cart.x=getFloat();
	clearSpace;
	if(*s==',') {s++;clearSpace;}
	lo->data.cart.y=getFloat();
	lo->data.cart.z=0;
	lo->code=Line_to_rel|Virtual;
	
	return 1;
}

int perimeter(void)
{ 	return 1; }

int xysum(void)
{ 	return 1; }

int ysum(void)
{ 	return 1; }

int contourArea(void)
{ 	return 1; }

int centroid(void)
{ 	
	/* 	
		For objects: creates a station 'centroid' with the
		coordinates given, just like 'centroid' = [x,y,z] and also
		makes 'centroid' the localDatum. 
	*/
	struct segmentRec *seg;
	
	if(ThisContext->pass==1) {
		if(!NameList) {
			NameHashArray=ecalloc(37,sizeof(long));
			NameList=lListAlloc(1,BlockLen,sizeof(struct nameRec),4,nNameList);
			NameList->current=36;
		}
		if(Literals)
			seg=getSR(Literals);
		else {
			seg=(struct segmentRec *) nextFromLList(SegList)+(SegList->current & BlockLo);
			Literals=seg->segment=SegList->current;
			seg->Field=-1;	/* a flag for THE lit segment. */
			*seg->desig=0;
			seg->age=0;
			seg->nextSubSeg=0;
			seg->nStations=seg->nLines=seg->first=seg->other=seg->flags=seg->last=0;
			seg->nTies=seg->nLinks=seg->nLocns=0;
			seg->links=NULL;
		}
		seg->nStations++;
	}
	
	ThisContext->localDatum.segment=Literals;	/* a global */
	ThisContext->localDatum.n=findName("centroid");
	
	clearSpace;
	ThisContext->datumData.x=getFloat();
	clearSpace;
	if(*s==',') {s++;clearSpace;}
	ThisContext->datumData.y=getFloat();
	clearSpace;
	if(*s==',') {s++;clearSpace;}
	ThisContext->datumData.z=getFloat();
	
	return 1; }

int type(void)
{ 	return 1; }

int sectionThickness(void)
{	
	struct fieldRec *se;
	clearSpace;
	ThisContext->dimension=getFloat();
	if(TheOpenField) {
		se=getField(TheOpenField);
		se->dimension=ThisContext->dimension;
	}
	return 1; 
}

int tissueMag(void)
{	return 1; }

int edMag(void)
{	return 1; }
#endif
