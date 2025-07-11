#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#ifdef CMLmake
#include "CML.h"
#include "CMLglobs.h"
#else
#include "MDL.h"
#include "MDLglobs.h"
#endif

#ifdef MPW
#include <cursorctl.h>
#endif

#ifdef MAC
	#include <files.h>
	#include <strings.h>
	#pragma segment routines
#else
	#include <search.h>
#endif

#ifdef DOS
#include <conio.h>

/* #define atan2(a,b) eatan2((a),(b)) */
/* extern extended eatan2(extended y,extended x); */

struct tm *cmllocaltime(const time_t *);
char *cmltime(const time_t *);
void RotateCursor(int);
#endif

#ifdef UNIX
void RotateCursor(int);
#endif

/* subroutines in _routines.c */
int includeFile (int);
int getPosNum(void);
void clearComment(void);
int printComment(void);
int parsePlanar(struct locnRec *);
c_float getFloat(void);
c_float parseFloat(char **);
c_float parseFixed(char **);
static c_float quadtof(char *);
static c_float getAzi(char *);
c_float getLength(void);
int parsePolarData(struct locnRec *);
void tokenize(int,...);
int parseDims(struct dimData *);
int updateMapping(int);
int addUnresolvedSegment(struct parseRec *, int);
int checkDesig(struct parseRec *);
int parseSegment(struct parseRec *);
FILE *getFile(char *,char *,long);
struct locnRec *getFixedData(struct segmentRec *, int);
struct locnRec *p2Planar(void);
void updateString(int);
int fixSegment(struct segmentRec *, struct locnRec *, short);
struct subSegRec *fixSubSegment(struct segmentRec *,struct strRec *,int *,int *);
void findExtents(void);
static int findSegmentBounds(struct segmentRec *);
int nextNode(struct segmentRec *seg);
struct context *newContext(int,char *, FILE *);
void averageDataLine(struct locnRec *);

#ifndef CMLmake
void transformCart(struct cartData *,c_float,struct cartData *);
void sortSections(void);
int FieldSortCompare(short *,short *);
int lscompare(short *a,LinkRecPtr b);
int nextNode(struct segmentRec *);
int processFids(LinkRecPtr);
#endif

#if defined(MDLView) || defined(VECTORS)
char *getLine(char *, int, FILE *);
#endif

#ifdef DEBUG
void printTrace(struct segmentRec *,LinkRecPtr);
#endif

int includeFile (int p)
{
	clearSpace;
	if (p==1)
		return (p1aFile(s));
	else
		return (p2aFile(s));
}

int getPosNum(void)
{
	int n=0;
	while(isdigit(*s)) {
		n *= 10;
		n += *s++ - 48;
	}
	return (n);
}

void clearComment(void)
{
	if(*s=='\\') {
		if(s=getLine(ThisContext->lastLine,MaxInLine,ThisContext->file)) {
			ThisContext->lastLine=ThisContext->s0;
			ThisContext->s0=s;
			LineCnt++;
			ContinuationLines++;
		} else
			return;
		clearSpace;
		return;
	}
	s++;
	if(*s=='&') {	/* comment directive */
		s++;
		/* Add appropriate code here for continuation line. */

	}
	while (*s && *s != ')') s++;
	if (*s=='\0') {
		errorAlert (NoRtParen);		/* Warning */
		return;			/* will return with *s==NULL */
	}
	s++;	/* past ')' */
	clearSpace;
}

int printComment(void)
{
	int col;
	if(*s=='\\') {	/* Continuation line break gets ignored. */
		if(s=getLine(ThisContext->lastLine,MaxInLine,ThisContext->file)) {
			ThisContext->lastLine=ThisContext->s0;
			ThisContext->s0=s;
			LineCnt++;
			ContinuationLines++;
		} else
			return 0;
		_clearSpace;
		col=0;
	} else {
		putc(*s,OutFile);
		s++;
		for (col=1;*s && *s != ')';s++,col++) {
			putc(*s,OutFile);
		}
		if(*s) {
			putc(*s,OutFile);	/* past ')' */
			s++;col++;
			_clearSpace;
		}
	}
	if(*s=='(' || *s=='\\')
		col+=printComment();
	return col;
}

int parsePlanar(struct locnRec *location)
{
	int rel;
	c_float x,y,z;
	x=EOffset;
	y=NOffset;
	z=AOffset;
	EOffset=NOffset=AOffset=0;
	
	location->code=(rel = *s++ =='{')? Line_to_rel|Virtual:_Point ;
	clearSpace;
	if(isfdigit) {
		location->data.cart.x=getFloat()+x;
		clearSpace;
		if(*s++ == ',') {
			clearSpace;
			if(isfdigit) {
				location->data.cart.y=getFloat()+y;
				clearSpace;
				if(*s++ == ',') {
					clearSpace;
					if(isfdigit)
					location->data.cart.z=getFloat()+z;
					clearSpace;
					if((*s == ']') || (rel && *s == '}')) {
						s++;
						return (P1LocnList->current);
					} else {
						errorAlert(RtSqBracketExpected);
						return 0;
					}
				}
			}
		}
	}
	s--;		/* so error report of line can point to correct char */
	errorAlert(Syntax);
	return 0;
}

/*--------------------------------------------------------------------- 	
	getFloat assumes that upon entry clearSpace has been done, so that
	it is looking at the first parsable character of a float. 
	getFloat uses getPosNum, for speed. For speed, it handles fixed 
	point floating point representations with getPosNum but will 
	default to parseFloat(), which uses atof(). The important point
	is that throughout CML/MDL parsing routines advance the global 
	pointer s.
	
	========== This may be a place for trials ============
-----------------------------------------------------------------------*/

c_float getFloat(void)
{
	c_float place=1,whole,fraction;
	int neg=FALSE;
	char *p;
	if (*s=='+') 
		s++;
	if (*s=='-') {
		s++;
		neg=TRUE;
	}
	if (*s=='.') 
		whole=0;
	else if(isdigit(*s))
		whole=getPosNum();
	else if(islower(*s)=='e')
		return (neg)? -parseFloat(&s):parseFloat(&s);
	else {
		errorAlert(NotAFloat);
		return 0;
	}
	fraction = 0;
	if (*s=='.') {
		place = 1;
		p = ++s;
		while(isdigit(*p)) {			/* here is one approach */
			p++;
			place *= 0.1;
		}
		fraction = place * getPosNum();
	}
	
	/*==============================================================
		We have just parsed a fixed point number. If there is not a
		a space or a delimeter, we should default to some sort of 
		atof call.
		
		An alternative for all of this is:
		#define getFloat() parseFloat(&s)
	===============================================================*/
	
	return (neg)? -(whole+fraction) : whole+fraction;
}

c_float parseFloat(char **str)
{
	char *p,*s;
	s=p=*str;
	while(isdigit(*s)) s++;
	if(*s=='.') {
		s++;
		while(isdigit(*s)) s++;
	}
	if(*s=='E' || *s=='e') {
		s++;
		if(*s=='-' || *s=='+') s++;
		while(isdigit(*s)) s++;
	}
	*str=s;
	return (atof(p));
}

/*---------------------------------------------------------------------
	parseFixed parses fixed point numbers. For example, it rather than
	parseFloat is called by quadtof because the 'E' of quad notation
	means that exponents cannot be used in them.
---------------------------------------------------------------------*/

c_float parseFixed(char **str)
{
	c_float place=1;
	int n=0,m=0,neg=FALSE;
	char *p;
	
	p=*str;
	if (*p=='+') 
		p++;
	if (*p=='-') {
		p++;
		neg=TRUE;
	}
	
	if(!isdigit(*p) && *p!='.') {
		*str=p;
		errorAlert(NotAFloat);
		return 0;
	}
	
	for(;isdigit(*p);p++) {
		n*=10;
		n+=*p-48;
	}
	
	if (*p=='.') {
		p++;
		for(place=1;isdigit(*p);p++) {
			m*=10; 
			m+=*p-48;
			place *= 0.1;
		}
	}
	*str=p;
	return (neg)? -(n+m*place) : n+m*place;
}

static c_float quadtof(char *str)
{
	c_float ang;
	char ns,ew,*p;
	
	p=str;
	ns=tolower(*p);
	p++;
	if(isalpha(*p)) {
		ew=tolower(*p);
		p++;
		ang=parseFixed(&p);
	} else {
		ang=parseFixed(&p);
		ew=tolower(*p);
		p++;
	}
	if(ns=='n'&&ew=='e')
		return(ang);
	if(ns=='n'&&ew=='w')
		return(360-ang);
	if(ns=='s'&&ew=='e')
		return(180-ang);
	if(ns=='s'&&ew=='w')
		return(180+ang);
	errorAlert(BadBearing);
	fprintf(OutFile,"with str,ns,ew,ang %s,%c,%c,%f\n",str,ns,ew,ang);
	return(0);
}

static c_float getAzi(char *str)
{
	c_float ang;
	ang = (isalpha(*str))? quadtof(str):atof(str) ;
	if(CompassUnits)
		ang*=0.05625;	/* Mils to degress conversion. */
	if(WarningFlag)
		if(ang<0 || ang>360)
			errorAlert(AziOutOfRange);
	return ang;
}

c_float getLength(void)
{
	/*-------------------------------------------------------------
		Routine to parse distances. Without sacrificing speed, it must
		deal with fixed (i.e. 12.5) and exponential notations (5.4e-3),
		scale factors (i.e. #meters, #cm, #microns, etc.), fixed 
		notation for feet-inches (i.e. #inch, 12.11, which means 12'11")
		and various ft-inch notations: 12'11", 12ft11, 12'11.
	---------------------------------------------------------------*/
	c_float place,whole=0,fraction=0;
	int neg=FALSE,meterFlag;
	char *p;
	meterFlag=MeterFlag;
	if (*s=='+') 
		s++;
	if (*s=='-') {
		s++;
		neg=TRUE;
	}
	if(isdigit(*s))
		whole=getPosNum();
	/*	Whole is either 0 or some integer. Now get the fraction. */
	if(*s=='.') {
		p = ++s;
		if(InchFlag)
			place = 0.0833333333333333333333;	/* 1/12th */
		else {
			/* here is one approach */
			for(place=1;isdigit(*p);p++)
				place *= 0.1;
		}
		fraction = place*getPosNum();
		if(tolower(*s)=='m') {
			s++;
			meterFlag=TRUE;
		}
	} else if(tolower(*s)=='e')
		fraction = parseFloat(&s);
	else if(*s=='\'') {
		s++;
		fraction = parseFloat(&s)/12;
		if(*s=='"') s++;
	} else if(tolower(*s)=='f') {
		s++;
		if(tolower(*s)=='t') s++;
		fraction = parseFloat(&s)/12;
		if(*s=='"') s++;
	} else if(tolower(*s)=='m') {
		s++;
		meterFlag=TRUE;
	}
	
	/*==============================================================
		We have just parsed a fixed point number. If there is not a
		a space or a delimeter, we should default to some sort of 
		atof call.
		
		An alternative for all of this is:
		#define getFloat() parseFloat(&s)
	===============================================================*/
	
	place=(neg)? -(whole+fraction) : whole+fraction;
	return (meterFlag)? place*MtoFt:place;
}

/*	Returns the index number of its locn record. */

int parsePolarData(struct locnRec *locn)
{
	c_float azi=0,vang=0,vdist=0,azicorr,theight=0;
	c_float fs,bs,fv,bv,shotLen,hlen,dz;
	c_float dim[4];	/* CML and Exchange Format dimension order is l,r,c,f. */
	int isDim[4]; 	/* Flags for dimensions. Space is cheap. */
	int i,k,num,j;
	int commaCnt=0;
	int nSlashes=0;
	int vertShot=0;
	int seflag=FALSE;
	int isWeight=FALSE;	

	int tok[13],comma[13],weight[13];
	char *n[13],*ends;
	struct dimData *dims=0;

	/* 	
		Note on Sinusoidal-eccentricity:
		The variable seflag is a local flag that is not quite a copy of the 
		global SEFlag. Here's why:
			The s-e calculation can only be made when there is no compass 
			backsight and when the compass has been calibrated for eccentricity,
			as indicated by a #s_e directive. In the code that follows,seflag 
			is set to SEFlag only when there is a single compass shot.
	*/
		
	clearSpace;

	if(!isfdigit) {
		errorAlert(NotAFloat);
		return 0;
	}
	if(locn->code) {	
	/*-----------------------------------------------------------------------
	 	Data location already used. This could arise normally, that is not be
		an error, from an implicit tie following the main segment. The shot
		data will be placed in the global TempLocn. If TempLocn is already
		filled (locn->code&_Rel) then this truly is an error.
		
		For now, the second set of data is discarded.
	------------------------------------------------------------------------*/
		if(locn->code&_Rel) {
			if(TempLocn.code&_Rel) {
				fprintf(OutFile,"in ppoldata l->code: %d l: %lu tl: %lu\n",locn->code,locn,&TempLocn);
				errorAlert(DoubleData);		/* Error */
			}
			locn=&TempLocn;
		}
	}
	TempLocn.code=0;	/* Extinguish any prior use of TempLocn. */
	ThisContext->fields=0;
	shotLen=getLength();
	locn->length=(short)floor(shotLen*10.0+0.5);
	
	clearSpace;
	
	azicorr=(AziCorrected)? 0.0:180.0;
	for(i=0;i<13 && *s;i++) {
		if(isbdigit) {	/* this tests for valid bearing numbers, quad or fixed point. */
			*(tok+i)=1;		
			*(n+i)=s++;
			while(isbdigit) s++;
			clearSpace;
			if(*s=='*') {
				if(!isWeight) {
					/* initializing weight[] here saves thousands of operations. */
					isWeight=TRUE;
					weight[0]=weight[1]=weight[2]=weight[3]=0;
				}
				weight[i]=1;
				s++;
				clearSpace;
			}
			if(*s==',') {
				commaCnt++;
				*(comma+i)=1;
				s++;
				clearSpace;
			} else if(*s=='/') {
				*(tok+i)=2; /* was 1 */
				s++;
				nSlashes++;
				*(comma+i)=0;
				clearSpace;
			} else
				*(comma+i)=0;
		} else if(*s==',') {
			*(tok+i)=0;
			commaCnt++;
			*(comma+i)=1;
			s++;
			clearSpace;
		} else if(*s==';' || *s=='|') 
			break;
		else {
			errorAlert(ExpectedComma);
			return(0);
		}
	}
	
	if(i>13) {
		errorAlert(TooManyFields);
		return(0);
	}
	
	ends=s;
	if(commaCnt>2)		/* There is dimensional data.cart. */
		k = (comma[i-1])? i-3-nSlashes : i-4-nSlashes;
	else
		k = i;
	if(VdFlag) {	/* Last field before dimensions is a vertical distance or target height. */
		s=n[--k];
		vdist=getLength();	/* Peel it off now. It is never paired. */
		ThisContext->fields=0x10;
	}
	if(VdFlag>1) {	/* Was target height. Get instrument height. */
		s=n[--k];
		theight=vdist;
		vdist=getLength()-theight;
		ThisContext->fields=0x11;
	}
		
	if(comma[k-1]) {				/* Test for a trailing comma in the shot data, for some */
		commaCnt--;					/* simplification in that that comma can be ignored (i.e. */
		errorAlert(ExtraCommas)	;	/* len fs,bs fv, ).  This deserves a warning. */
	}

	switch(commaCnt) {
		case 3:
		case 0:
			seflag=SEFlag;		/* Crowther-Mann domain. No backsight. */
			if(k>2) {				/* a b c ...*/
				errorAlert(TooManyFields);
				s=n[--k];
				vdist = getLength();	/* will be added */
				ThisContext->fields=0x10;
			}
			if(k==2) {		/* a  c */
				azi=getAzi(n[0]);
				vang=getFloatP(n[1]);
				ThisContext->fields|=5;
			} else if (k==1) { 		/* c */
				if(ShotMode<2) {	/* Vang or vad, there is a vertical angle. It can only by +-90 */
					if(abs(num=atoi(n[0]))!=90)	{	/* Let's not deal with floats here */
						errorAlert(ExpectedVerticalShot);
						return 0;
					}
					vang=num;
					vertShot=TRUE;
					ThisContext->fields|=4;
				} else {
					azi=getAzi(n[0]);
					ThisContext->fields|=1;
				}
			} else {
				errorAlert(Syntax);
				ThisContext->fields=0;
				return 0;
			}
			break;	/* calculate ds's */

		case 4:				
		case 1:				
			if(k>3) {
				errorAlert(TooManyFields);
				s=n[--k];
				vdist = getLength();	/* will be added */
				ThisContext->fields=0x10;
			}
			if(k==3) {				/* 1: 	a	 c,d	*/
									/* 2:	a,b	 c		*/
									/* 3:	 ,b	 c 		*/			
				if (comma[0] ) {		/* 2: or 3: */
					if(!tok[1]) {
						errorAlert (Syntax);
						ThisContext->fields=0;
						return 0;
					}
					bs=getAzi(n[1]);		/* 2nd numeral */
					if(tok[0]) {			/* 2 */
						fs=getAzi(n[0]);		/* 1st numeral */
						azi=(fs<bs)? fs+360 : fs;
						ThisContext->fields|=7;
						if(isWeight) 
							azi=(weight[0])?
								Heavy*azi+Light*(bs+azicorr) :
								Light*azi+Heavy*(bs+azicorr);
						else
							azi=(azi+bs+azicorr)/2;
					} else {				/* 3 */
						azi=bs+azicorr;
						ThisContext->fields|=6;
					}
					vang=getFloatP(n[2]);
				} else {				/* 1:	*/
					if(tok[0]==0) {
						errorAlert (Syntax);
						ThisContext->fields=0;
						return 0;
					}
					seflag=SEFlag;
					azi=getAzi(n[0]);
					fv=getFloatP(n[1]);
					bv = (InclCorrected)? -getFloatP(n[2]):getFloatP(n[2]);
					ThisContext->fields|=0xd;
					if(isWeight)
						vang=(weight[1])?
							Heavy*fv-Light*bv:
							Light*fv-Heavy*bv;
					else
						vang=(fv-bv)/2;
				}
			} else if (k==2) {			/* 1:	,a		*/
										/* 2:	a	,d 	*/
										/* 4:	a,b		*/
				if(!comma[0] || !tok[1]) {
					errorAlert(Syntax);
					ThisContext->fields=0;
					return 0;
				}
				if(tok[0]) {				/* 2 or 4 */
					if(ShotMode>1) {		/* 4: #nov or #vdist */
						fs=getAzi(n[0]);
						bs=getAzi(n[1]);
						azi=(fs<bs)? fs+360.0 : fs;
						ThisContext->fields|=3;
						if(isWeight) 
							azi=(weight[0])?
								Heavy*azi+Light*(bs+azicorr) :
								Light*azi+Heavy*(bs+azicorr);
						else
							azi=(azi+bs+azicorr)/2;
					} else {				/* 2 */
						seflag=SEFlag;
						azi=getAzi(n[0]);
						vang = (InclCorrected)? getFloatP(n[1]):-getFloatP(n[1]);
						ThisContext->fields|=9;
					}
				} else {					/* 1 */
					seflag=SEFlag;
					azi = getAzi(n[1])+azicorr;
					vang = 0.0;
					ThisContext->fields|=2;
				}
			} else {
				errorAlert(Syntax);
				ThisContext->fields=0;
				return 0;
			}
			break;

		case 5:		
		case 2:				
			if(k>4) {
				errorAlert(TooManyFields);
				s=n[--k];
				vdist = getLength();	/* will be added */
				ThisContext->fields|=0x10;
			}
			
			if(k==4) {					/* 1:	a,b	 c,d		*/
										/* 3:	 ,b	 c,d		*/
				if(!comma[0] || !tok[1]) {
					errorAlert(Syntax);
					ThisContext->fields=0;
					return 0;
				}
				bs=getAzi(n[1]);
				if(tok[0]) {	/* 1 */
					fs=getAzi(n[0]);
					azi=(fs<bs)? fs+360.0 : fs;
					ThisContext->fields|=0xf;
					if(isWeight) 
						azi=(weight[0])?Heavy*azi+Light*(bs+azicorr):Light*azi+Heavy*(bs+azicorr);
					else
						azi=(azi+bs+azicorr)/2;
				} else {
					azi=bs+azicorr;
					ThisContext->fields|=0xe;
				}
				if(!comma[2] || !tok[3]) {
					errorAlert(Syntax);
					ThisContext->fields=0;
					return 0;
				}
				bv=(InclCorrected)? -getFloatP(n[3]) : getFloatP(n[3]);
				if(tok[2]) {	/* 1 or 3  */
					fv=getFloatP(n[2]);
					if(isWeight)
						vang=(weight[2])?Heavy*fv-Light*bv:Light*fv-Heavy*bv;
					else
						vang=(fv-bv)/2;
				} else {			/* 2, below */
					errorAlert(Syntax);
					ThisContext->fields=0;
					return 0;
				}
			} else if(k==3) {			/* 1:	 ,b	  ,d  */
										/* 2:	a,    ,d  */
										/* 3:	a,b   ,d  */
				if(!tok[2]) {
					errorAlert(Syntax);
					ThisContext->fields=0;
					return 0;
				}
				if(!tok[0]) {		/* 1 */
					bs=getAzi(n[1]);
					azi=bs+azicorr;
					ThisContext->fields|=0xa;
				} else {
					azi=fs=getAzi(n[0]);
					if(tok[1]) {	/* 3 */
						bs=getAzi(n[1]);
						azi=(fs<bs)? fs+360.0 : fs;
						ThisContext->fields|=0xb;
						if(isWeight) 
							azi=(weight[0])?Heavy*azi+Light*(bs+azicorr):Light*azi+Heavy*(bs+azicorr);
						else
							azi=(azi+bs+azicorr)/2;
					} else {			/* 2 */
						ThisContext->fields|=0xb;
						errorAlert(ExtraCommas);
					}
				}
				vang = (InclCorrected)? getFloatP(n[2]) : -getFloatP(n[2]);
			} else {
				errorAlert(Syntax);
				ThisContext->fields=0;
				return 0;
			}
			break;
		default: 
			errorAlert(Syntax);
			ThisContext->fields=0;
			return 0;
	}
	if(ParseAll) {
		j=k+VdFlag;	/* one or two parameters beyond kth */
		if(j<i) {
		/* Do dimensional data */
			for(num=0;num<4;j++,num++) {
				if((j>=i)||(tok[j]==0)) {
					isDim[num]=0;
					dim[num]=0;
				} else if(tok[j]==1) {
					isDim[num]=1;
					s=n[j];
					dim[num]=getLength();
				} else if(tok[j]==2) {
					isDim[num]=2;
					if(*n[j+1]=='+') {
						s=n[j];
						dim[num]=getLength();
						s=n[j+1];
						dim[num]+=getLength();
					} else {
						s=n[j+1];
						dim[num]=getLength();
					}
					j++;
				}
			}
		} else {
			for(num=0;num<4;num++)		/* illegal values in the dim array corrupted the atof */
				dim[num]=0;				/* function in Microsoft C v. 5.1 !!! */
		}

		if(ExchangeFormat) { /* Repeat the parse to tokenize. This is only done for a particular
								kind of listing and it destroys the cartesian data. */
			locn->data.tok.data=ftell(TokFile);
			tokenize(1,1,shotLen,-1);
			switch(commaCnt) {
				case 3:
				case 0:
					if(k==2) {		/* a  c */
						tokenize(4,1,azi,3,vang,-1); 	/*	"%10.2f%10s%10.2f%10s",azi,"",vang,""); */
					} else if (k==1) { 	/* c */
						if(ShotMode<2)
							tokenize(4,3,vang,-1); 		/* "%30d%10s",num,""); */
						else
							tokenize(4,1,azi,-1); 		/* "%10.2f%30s",azi,""); */
					}
					break;	/* calculate ds's */
		
				case 4:				/* 1: 	a	 c,d	*/
				case 1:				/* 2:	a,b	 c		*/
									/* 3:	 ,b	 c 		*/
									/* 4:	a,b			*/
					if(k==3) {
						if (comma[0] ) {	/* 2: or 3: */
							if(tok[0]) 			/* 2 */
								tokenize(4,1,fs,2,bs,3,vang,-1); /* "%10.2f%10.2f%10.2f%10s",fs,bs,vang,""); */
							else				/* 3 */
								tokenize(4,2,bs,3,vang,-1);		/* "%10s%10.2f%10.2f%10s","",bs,vang,""); */
						} else 					/* 1:	*/
							tokenize(4,1,azi,3,fv,4,bv,-1);		/* "%10.2f%10s%10.2f%10.2f",azi,"",fv,bv); */
							
						} else if (k==2) {		/* 1:	,a		*/
												/* 2:	a	,d 	*/
												/* 4:	a,b		*/
												
							if(tok[0]) {		/* 2 */
								if(ShotMode>1)		/* 4: #nov or #vdist */
									tokenize(4,1,fs,2,bs,-1);	/* "%10.2f%10.2f%20s",fs,bs,""); */
								else
									tokenize(4,1,azi,4,-vang);	/* "%10.2f%20s%10.2f",azi,"",-vang); */
							} else						/* 1 */
								tokenize(4,2,azi-180.0,-1);		/* "%10s%10.2f%20s","",azi-180.0,""); */
						}
						break;
					
				case 5:				/* 1:	a,b	 c,d		*/
				case 2:				/* 3:	 ,b	 c,d		*/
					if(k==4) {
						bs=getAzi(n[1]);
						if(tok[0]) 	/* 1 */
							tokenize(4,1,fs,2,bs,3,fv,4,bv,-1);	/* "%10.2f%10.2f%10.2f%10.2f%",fs,bs,fv,bv); */
						else if(tok[2]) 	/* 3 (or 1) */
							tokenize(4,2,bs,3,fv,4,bv,-1);		/* "%10s%10.2f%10.2f%10.2f","",bs,fv,bv); */
						break;
					} 
					if(k==3) {			/*	a, ,d or a,b ,d	*/
										/* 2:	 ,b	  ,d		*/
						if(!tok[0]) {
							tokenize(4,2,bs,4,-vang,-1);		/* "%10s%10.2f%10s%10.2f","",bs,"",-vang); */
							break;
						}
						if(tok[1]) 
							tokenize(4,1,fs,2,bs,4,-vang,-1);	/* "%10.2f%10.2f%10s%10.2f",fs,bs,"",-vang); */
						else
							tokenize(4,1,fs,4,-vang,-1);		/* "%10.2f%20s%10.2f",fs,"",-vang); */
						break;
					} 
				default: ;
			}
			k+=VdFlag; /* Bump k back up past Vdist field so dimensional data will 
								be correctly handled. */
								
			if(k<i) {	
				for(num=0;num<4;num++) {
					if(isDim[k=(num+RotateDimensions)%4])
						tokenize(1,1,dim[k],-1);
					else
						tokenize(1,-1);
				}
				if(VdFlag)
					tokenize(3,3,vdist+theight,-1);				/* "%30.2f",vdist); */
				if(VdFlag>1)
					tokenize(3,3,theight,-1);
			} else {
				if(VdFlag) 
					tokenize(7,7,vdist+theight,-1);				/* "%70.2f",vdist); */
				if(VdFlag>1)
					tokenize(3,3,theight,-1);
			}
			fprintf(TokFile,"\n");
			locn->code |= ShotFlag;		/* Line_to_rel|Virtual */
			if(ExcludeFlag) {
				ExcludeFlag=(ExcludeFlag==1)?0:ExcludeFlag;
				return 0;
			} else
				return (int)locn->length;
		} else {
			/* 	Have parsed dimensional data, but not in Exchange Format. Must process
				the dimensional data. */
			dims=findDims(locn);
			if(ThisContext->stationIncr==-1) {
				dims->r=10*dim[0+RotateDimensions];
				dims->l=10*dim[1+RotateDimensions];
			} else {
				dims->l=10*dim[0+RotateDimensions];
				dims->r=10*dim[1+RotateDimensions];
			}
			dims->c=10*dim[(2+RotateDimensions)%4];
			dims->f=10*dim[(3+RotateDimensions)%4];
		}
	}	/* End ParseAll */
	if(WarningFlag) {
		/* Place most data-error warnings here. */
		if(shotLen<=0) errorAlert(BadDistance);
		if(fabs(vang)>90) errorAlert(InclinationTooBig);
		if((ThisContext->fields&3)==3) {	/* Both fs and bs */
			if(fabs(fabs(fs-bs)-azicorr)>AngleError)
				errorAlert(AziDisagree);
		}
		if((ThisContext->fields&12)==12)	{	/* Both fv and bv */
			if(fabs(fv+bv)>AngleError)
				errorAlert(InclDisagree);
		}
	}
	
	if(vertShot || fabs(vang)==90.0) {
			locn->data.cart.z=(vang>0)? vdist+shotLen : vdist-shotLen;
			locn->code |= VerticalShot;
			locn->data.cart.x=locn->data.cart.y=0;
			hlen=0;
	} else {
		azi+=(seflag)?
			  CompCor+SError*cos((azi-SWorstAngle)*DtoR)
			: CompCor;
		if(dims) 
			dims->azi=(vertShot)? 0xFFFF : (short)(azi*10) % 3600;

		switch (ShotMode) {
			case VANG:
				vang+=InclCor;
				hlen = shotLen*cos(vang*=DtoR);
				locn->data.cart.z = shotLen*sin(vang);
				break;
			case VDIST:
				hlen = shotLen;
				locn->data.cart.z = vdist;
				shotLen=sqrt(shotLen*shotLen+vdist*vdist);
				break;
			case VAD:
				vang+=InclCor;
				hlen = shotLen*cos(vang*=DtoR);
				locn->data.cart.z = dz = vdist+shotLen*sin(vang);
				shotLen=sqrt(hlen*hlen+dz*dz);
				break;
			case LEVEL:
				hlen=sqrt(shotLen*shotLen-vdist*vdist);
				locn->data.cart.z = vdist;
				break;
			case NOV:
				hlen = shotLen;
				locn->data.cart.z = 0;
				break;
			default:
				_errorAlert(UnexpectedError,0,11);
				break;
		}
#ifdef CMLmake
		locn->data.cart.x = hlen*sin(azi*=DtoR);
		locn->data.cart.y = hlen*cos(azi);
#else
		locn->data.cart.x = hlen*cos(azi*=DtoR);
		locn->data.cart.y = hlen*sin(azi);
#endif
	}
	if(DVertical) {
		locn->data.cart.z += DVertical;
		DVertical=0;
	}
	locn->code |= ShotFlag;
	
	if(HorizontalDistanceFlag)
		locn->length=(short)floor(hlen*10.0+0.5);
		
	if(ExcludeFlag) {	/* A value of 1 means a #spray directive is in effect. */
		ExcludeFlag=(ExcludeFlag==1)?0:ExcludeFlag;
		return 0;
	} else {
		TotalDist+=shotLen;
		TotalHDist+=hlen;
		return (int)locn->length;
	}
}

void tokenize(int fields,...)
{
	int col,n;
	va_list param;
	c_float val;
	
	va_start(param,fields);
	for(col=1;col<=fields;col++) {
		if((n=va_arg(param,int))==-1) break;
		while(col<n) {
			if(Sep)
				fprintf(TokFile,"%c",Sep);
			else
				fprintf(TokFile,"%10s","");
			col++;
		}
		val=va_arg(param,c_float);
		if(Sep)
			fprintf(TokFile,"%.2f%c",val,Sep);
		else
			fprintf(TokFile,"%10.2f",val);
	}
	while(col<=fields) {
		if(Sep)
			fprintf(TokFile,"%c",Sep);
		else
			fprintf(TokFile,"%10s","");
		col++;
	}
	va_end(param);
	return;
}

int parseDims(struct dimData *dims)
{
	int num;
	c_float dim[4];

	dims->azi=0xFFFF;
	clearSpace;
	if(isdigit(*s) || *s==',' || *s=='.') {
		for(num=0;num<4;num++) {
			dim[num]=getLength();
			clearSpace;
			if(*s=='+') {
				s++;
				clearSpace;
				dim[num]+=getLength();
				clearSpace;
			} else if(*s=='/') {
				s++;
				clearSpace;
				dim[num]=getLength();
				clearSpace;
			} else if(tolower(*s)=='p') {
				s++;
				clearSpace;
			}
			if(num<3) {
				if(*s==',')
					s++;
				else
					errorAlert(ExpectedComma);
			}
		}
		dims->l=10*dim[RotateDimensions];
		dims->r=10*dim[1+RotateDimensions];
		dims->f=10*dim[(2+RotateDimensions)&4];
		dims->c=10*dim[(3+RotateDimensions)&4];
		return 1;
	} else
		return 0;
}

int updateMapping(int incr)
{
	MapRecPtr mR;
	struct segmentRec *seg;
	int temp;
	
	if((Previous.this.n+incr)==Previous.first)
		return 1;
			
	mR=nextFromList(MapList,mapCast);
	mR->segment=Previous.this.segment;
	mR->first=Previous.first;
	mR->last=Previous.first=Previous.this.n+incr;
	seg=getSR(mR->segment);
	seg->LineCnt=LineCnt;
	ThisContext->segment=mR->segment;
	
	/* Move the data from the current context to this segment record. */
	if(!seg->age) {
		seg->age = ++CurrentAge;
		if(CurrentAge==1 && FirstAged.segment==0) {
			FirstAged.segment = mR->segment;
			FirstAged.n = (incr == -1)? mR->last : mR->first;
		}
	}
	if(mR->first > mR->last) {	/* Switch them. */
		temp = mR->first;
		mR->first = mR->last;
		mR->last = temp;
	}
	return 1;
}

/*------------------------ addUnresolvedSegment ----------------------------	
	Returns a segment number & has the side effect of parseRec.Field := 0.
	It allocates its own record on the UnResSegList. If an Field is open
	(by the #openField directive) it checks the list of segment designations
	in that section (book) before assigning an unresolved segment number. When 
	the open section (book) is closed, the unresolved segment belonging to 
	that section are checked again.
--------------------------------------------------------------------------*/

int addUnresolvedSegment(struct parseRec *pR,int resolveFlag)
{
	int i;
	struct unResSegRec *URS;

	if(ThisContext->pass==1) {
		/* Create an unResSegRec and do side effects. */
		URS = (struct unResSegRec *) nextFromLList(UnResSegList)+(UnResSegList->current&BlockLo);
		i = newSegmentRec(-UnResSegList->current-1,pR->desig,0);	/* points back to URS */
		URS->segment=i;
		URS->resSegment=resolveFlag;	/* If negative, can be resolved in p1. */
		URS->line=LineCnt;
		pR->this.segment=i;
		pR->Field=0;
		return i;
	} else if(ThisContext->pass==2) {
		for(i=1;i<=UnResSegList->current;i++) {
			URS=getURS(i);
			if(URS->line=LineCnt) {
				pR->Field=0;
				return(pR->this.segment=URS->segment);
			}
		}
		if(TheOpenField) {
			pR->Field=TheOpenField;
			i=findSegment(TheOpenField,pR->desig);
			return(pR->this.segment=i);
		}	/* Create an unResSegRec and do side effects. */
		return 0;
	} else 
		_errorAlert(UnexpectedError,0,12);
	return 0;
}

/*---------------------------------------------------------------------
	checkDesig (formally unresolvedSurvey) tries to match the segment
	designation being parsed with any previous or tentative segments,
	should they exist. If these do not exist or no match is found, it
	falls through to addUnresolvedSegment. checkDesig always returns a
	segment number; UnexpectedError can never happen.
----------------------------------------------------------------------*/

int checkDesig(struct parseRec *pR)
{
	switch(Tentative.flag) {
/*======================
	Eliminate this routine. Roll it out in p1aFile. We are double-casing.
======================*/
		case 0x2000:
			if(desigCmp(pR->desig,Tentative.desig)) {
				pR->Field=Tentative.Field;
				return(pR->this.segment=Tentative.this.segment);
			}
		case 0x1000:
			if(desigCmp(pR->desig,Previous.desig)) {
				pR->Field=Previous.Field;
				return(pR->this.segment=Previous.this.segment);
			}
		case 0:		/* No match, add to list of unresolved segments. */
			return(assignSegment(*pR,Field=TheOpenField,Resolve));
		default:
			_errorAlert(UnexpectedError,0,13);
			return 0;
	}
}

/*	returns Field==0 for URSs (as $A10) */
int parseSegment(struct parseRec *pRec)
{
	int i;
	switch(pRec->flag) {
		case(4):
		case(1):	/* Field+Desig+[station] */ 
			pRec->Field=(isdigit(*s))? getPosNum() : 0;
			clearSpace;
			if (!(isalpha(*s)||*s=='#')) {		/* no Desig */
				errorAlert(MissingSegmentName);
				return(pRec->flag=0);
			}
		case(2):	/* Desig+[station] */
			for (i=0;i<DesigLen;i++)
				pRec->desig[i]=(isdesig(*s))? *s++ : 0;
			if(isdesig(*s)) {
				errorAlert(TooManyChars);
				while (isdesig(*s)) s++;
			}
			clearSpace;
		case(3):	/* station */
			if(isdigit(*s)) {
				pRec->this.n=getPosNum();
				if(*s=='\'') {
					while(*s=='\'') { s++; pRec->this.n += StationPrimeRadix;}
					if(pRec->this.n>15000)
						errorAlert(TooManyPrimes);
				} else if(isalpha(*s)) {
					pRec->this.n<<=6;	/* 26<2^5 */
					pRec->this.n += (tolower(*s)-'a');
					pRec->this.n |= 0x4000;	/* keeps it a pos number */
					while (isalpha(*s)) s++;
				}
				return(pRec->flag);	/* good return */
			}
			if(pRec->flag==4)
				return(pRec->flag);	/* missing station okay */
			if(pRec->desig[1]|| !isalpha(pRec->desig[0])) {
				pRec->flag=0;
				errorAlert(MissingStationNumber);
			} else {
				errorAlert(LetterSequence);
				pRec->this.n=0x4020+tolower(pRec->desig[0])-'a';
				pRec->desig[0]=0;
			}
			return(pRec->flag);
		default:
			_errorAlert(UnexpectedError,0,14);
			return(pRec->flag=0);
	}
}

FILE *getFile(char *name,char *buf,long length)
{
	FILE *theNewFile;
	char *p;
	int copyright=FALSE;
	static FILE dummyFile;
#ifdef MAC
	OSErr err;		
	FInfo fndrInfo;
#endif

	if(buf) {
		/* 	Case for several Shell implementations, including MDLView
			and Vectors. InBuf is a global that signifies that the input
			data are to come from a buffer in memory. */
		dummyFile._ptr=dummyFile._base=buf;
		dummyFile._cnt=length;
		theNewFile = &dummyFile;
		StreamIsFile = FALSE;
	} else {
		StreamIsFile = TRUE;
		theNewFile=fopen(name,"r");
	}
	if(theNewFile) {
		if(OutputFlag>0) {
			strcpy(Scratch,name);
			if(p=strrchr(Scratch,'.')) {
				copyright=!strncmp(p+1,"CRF",3) || !strncmp(p+1,"crf",3);
#ifndef MAC
			 	*p='\0'; /* Truncate extension. */
#endif
			}
			if(ExchangeFormat)
				strcat(Scratch,".sef");
			else
				strcat(Scratch,".out");
			if(!(OutFile=fopen(Scratch,"w"))) 
				errorAlert(FileOutErr);
			
			if(OutputFlag==1) 
				ErrFile=OutFile;
			OutputFlag=-OutputFlag;
#ifdef MAC
			err=GetFInfo(c2pstr(Scratch),0,&fndrInfo);
			if(err==noErr) {
/*				fndrInfo.fdType='TEXT';	*/
  				fndrInfo.fdCreator='MRPG';
				SetFInfo(Scratch,0,&fndrInfo);
			}
			p2cstr(Scratch);
#endif
		}
#ifdef VECTORS
		 else if(!OutputFlag) {
		 	OutFile=tmpfile();
			if(!OutFile)
				errorAlert(FileOutErr);
		 
		 }
#endif
		if(OutputFlag>-10) {
			OutputFlag-=10; /* -10 if stdout, less if not */
			if(!Silent) {
				fprintf(OutFile,"; %s, version %s\n",ProgramString,VersionString);
				fprintf(OutFile,"; %s",p=asctime(NewTime));
				if(copyright)
					fprintf(OutFile,"; Copyright %.4s %s\n",p+20,OwnerString);
			}
		}
	}
	return (theNewFile);
}

struct locnRec *getFixedData(struct segmentRec *seg,int sta)
{
	struct subSegRec *theSubSeg;
	LinkRecPtr theLink;
	struct locnRec *theLocn;
	int i;
	/*------------------ find the subsegment ------------------------*/
	for(theSubSeg=(struct subSegRec *) seg;;) {
		if(theSubSeg->first<=sta && sta<=theSubSeg->last) {
			theLocn=getData(theSubSeg->data+sta-theSubSeg->first);
			if((theLocn->code&_Point) && !(theLocn->code&_Rel))
				return(theLocn);
			else 
				return 0;
		} else if (theSubSeg->nextSubSeg) 
			theSubSeg=getSS(theSubSeg->nextSubSeg);
		else {
	/*===		errorAlert(NoSubSegment);	 temp warning ====*/
	/*------------------------ First try the links -------------------------------*/
			for(i=seg->links+seg->nLocns
				;i<seg->links+seg->nLocns+seg->nLinks
				;i++) {
				theLink=getLink(i);
				if(theLink->from.n == sta) {
					theLocn=getFixedData(getSR(theLink->to.segment),theLink->to.n);
					if((theLocn->code&_Point) && !(theLocn->code&_Rel))
						return(theLocn);
				}
			}
	/*----------------------- Next try the ties ---------------------------------*/
			for(i=seg->links+seg->nLocns+seg->nLinks
				;i<seg->links+seg->nLocns+seg->nLinks+seg->nTies
				;i++) {
				theLink=getLink(i);
				if(theLink->from.n==sta) {
					theLocn=getLocn(theLink->data);
					if((theLocn->code&_Point) && !(theLocn->code&_Rel))
						return((theLink->to.segment<0)?theLocn-1:theLocn);
				}
			}
			return 0;
		}
	}
}

struct locnRec *p2Planar(void)
{
	int thisStation;
	Previous = Left;
	if(!(State & 0x3000)) { /* Set up data space */
		thisStation=(LineOrder==ToVector)? Previous.this.n 
			: Previous.this.n+1;
		ThisContext->segment = Previous.this.segment;
		return (setUpDataSpace(thisStation,Previous.this.segment));
	}
	return 0;
}

#ifdef DEBUG
void printTrace(struct segmentRec *seg,LinkRecPtr li)
{
	struct segmentRec *su0;
	Str31 a,b;
	su0=getSR(abs(li->to.segment));
	fprintf(OutFile,"%s (%d) fixed from %s\n",printFullSta(abs(li->to.n),su0,a),
		su0->segment,printFullSta(abs(li->from.n),seg,b));
	return;
}
#endif

/*-------------------------------------------------------------------
	This is the p2 equivalent of updateMapping. There is not much to
	do beyond keeping track of the segment's length and resetting the
	first station in parse record.
---------------------------------------------------------------------*/

void updateString(int incr)
{
	struct segmentRec *theSegment;

	if((Previous.this.n+incr)==Previous.first)
		return;
		
	theSegment=getSR(Previous.this.segment);
	theSegment->length+=Length;

	Previous.first=Previous.this.n+incr;
	Length=0;
	
	/*	Used to allow calls to fixSegment() here--no more. */
	
	return;
}

struct context *newContext(int pass,char *fileName, FILE *file)
{
	/*	
		Contexts are created by main() and by p1 and p2 and are
		allocated on the heap by this routine. 
	*/
	struct context *this;
	if(this=ecalloc(1,sizeof(struct context))) {
		this->pass=pass;
		this->stationIncr=1;
		this->startLineCnt=LineCnt;
		this->fileName=fileName;
		this->s0=this->lastLine="...\n";
		this->file=file;
	}
	return this;
}

void averageDataLine(struct locnRec *avg)
{
	static int cnt;
	char *p;
	c_float delta;
	struct locnRec locn;
	
	if(!avg) {
		errorAlert(UnexpectedError);
		return;
	}
	/* If previous line starts with '+' then there is an ongoing average.	*/
	p=s;
	s=ThisContext->lastLine;
	clearSpace;
	cnt=(*s=='+')? cnt+1:2;
	s=p+1;
	parsePolarData(&locn);
	/* compare locn and avg	*/
	delta=abs(locn.length-avg->length);
	if(delta/locn.length > 0.05)
		errorAlert(ShotsDiffer);
	else {
		delta=locn.length/10.0;
		delta=(fabs(locn.data.cart.x=avg->data.cart.x)
				+fabs(locn.data.cart.y=avg->data.cart.y)
				+fabs(locn.data.cart.z=avg->data.cart.z))/delta;
		if(delta > 0.05)
			errorAlert(ShotsDiffer);
	}
	avg->data.cart.x=avg->data.cart.x*(cnt-1)/cnt + locn.data.cart.x/cnt;
	avg->data.cart.x=avg->data.cart.y*(cnt-1)/cnt + locn.data.cart.y/cnt;
	avg->data.cart.x=avg->data.cart.z*(cnt-1)/cnt + locn.data.cart.z/cnt;	
}

#ifdef MAC
#pragma segment fixsegment
#endif

int fixSegment(struct segmentRec *seg,struct locnRec *locn,short sta)
{
	register int i,lim,n;
	int j,fixingLink=0,col=0;
	int sefFlag=0;
	struct strRec *str=0;
	struct locnRec *theLocn,*extLocn,*theData;
	LinkRecPtr theLink,extLink;
	struct subSegRec *ss;
	struct segmentRec *su0;
	struct headerRec *head;
	int firstFixed,lastFixed,seeking,strLength;
	int numStringsToFix=0,numTiesToFix=0;
	c_float dx=0,dy=0,dz=0;
	struct nameRec *place,*person;
	struct dbRec *party;
	struct tm *segmentDate;
	char sep[2];
	Str31 a,b;
	
/*--------------------------------------------------------------------- 
	This if very complicated due to its attempt to fix gapped
	subSegments and all subSegments in a segment. Since most Segments
	don't have subSegments and even fewer are gapped, it does 
	operate efficiently, and fast, in most cases.
	
	If entered with locn==0, then the routine looks for more to do.
---------------------------------------------------------------------*/

	RotateCursor(seg->segment);
	if(ExchangeFormat) {
		if(seg->data || seg->nLocns)
			sefFlag=ExchangeFormat;
		else {	/* check if there are ties that would be printed for this segment. */
			for(lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies;!sefFlag && i<lim;i++) {
				theLink=getLink(i);
				theLocn=getLocn(theLink->data);
				if(theLocn[-1].length==seg->segment) 
					sefFlag=ExchangeFormat;
			}
		}
		if(sefFlag) {	
			sep[0]=Sep;
			sep[1]=0;
			if(seg->Field>0)
				fprintf(OutFile,"\n;------------------- %s $%d%.4s ------------------\n",SegmentName,seg->Field,seg->desig);
			else
				fprintf(OutFile,"\n;----------------- Literal Stations ----------------\n");
				
			head=(seg->header)? getHeader(seg->header):0;
			place=(head && head->object)? getPlace(head->object,NameList):0;
			
			if(seg->nLocns) {
				if(seg->Field>0)
					fprintf(OutFile,"#cpoint Survey $%d%.4s %s\n",seg->Field,seg->desig,(place)?place->name:"");
				else
					fprintf(OutFile,"#cpoint Literal Stations\n");
				if(Sep)
					fprintf(OutFile,"#data station%seast%snorth%selev\n",sep,sep,sep);
				for(i=seg->links;i<seg->links+seg->nLocns;i++) {
					theLink=getLink(i);
					theLocn=getLocn(theLink->data);
					if(Sep)
						fprintf(OutFile,"%s%s%.2f%s%.2f%s%.2f\n",printPureSta(theLink->from.n,seg,a),sep,
							theLocn->data.cart.x,sep,theLocn->data.cart.y,sep,theLocn->data.cart.z);
					else
						fprintf(OutFile,"     %-12s%16.1f%16.1f%10.1f\n",printPureSta(theLink->from.n,seg,a),
							theLocn->data.cart.x,theLocn->data.cart.y,theLocn->data.cart.z);
				}
				fprintf(OutFile,"#endcpoint\n");
			}
			if(seg->Field>0)
				fprintf(OutFile,"#ctsurvey Survey $%d%.4s %s\n",seg->Field,seg->desig,(place)?place->name:"");
			else
				fprintf(OutFile,"#ctsurvey Literal Stations\n");
			if(head) {
				if(head->date) {
					segmentDate=cmllocaltime(&head->date);
					fprintf(OutFile,"#date=%02d/%02d/%4d\n",
						segmentDate->tm_mon+1,segmentDate->tm_mday,segmentDate->tm_year+1900);
				} 
				if(place) {	/* contains #reconstruction (#object) field */
					fprintf(OutFile,";#c %s\n",place->name);
				}
				if(head->location) {
					place=getPlace(head->location,NameList);
					fprintf(OutFile,";#l %s\n",place->name);
				}
				if(head->personnel) {
					for(n=0;n<8;n++) {
						party=getDb(head->personnel+n);
						person=getName(party->what);
						fprintf(OutFile,"#person %s\n",person);
						if(party->where.item&(Leader|Book|Compass|Tape)) {
							fprintf(OutFile,"#duty");
							if(party->where.item&Leader)
								fprintf(OutFile," leader");
							if(party->where.item&Book)
								fprintf(OutFile," book");
							if(party->where.item&Compass)
								fprintf(OutFile," compass");
							if(party->where.item&Tape)
								fprintf(OutFile," tape");
							fprintf(OutFile,"\n");
						}
						if (party->where.item&LastInParty)
							break;
					}
				}
				if(head->compass) {
					place=getPlace(head->compass,NameList);
					fprintf(OutFile,"#fcid %s\n",place->name);
				}
				if(head->decl)
					fprintf(OutFile,"#decl=%.2f\n",(c_float)head->decl/100);
				if(head->fcc)
					fprintf(OutFile,"#fcc=%.2f\n",(c_float)head->fcc/100);
				if(head->bcc)
					fprintf(OutFile,"#bcc=%.2f\n",(c_float)head->bcc/100);
				if(head->inclinometer) {
					place=getPlace(head->inclinometer,NameList);
					fprintf(OutFile,"#fiid %s\n",place->name);
				}
				if(head->fic)
					fprintf(OutFile,"#fic=%.2f\n",(c_float)head->fic/100);
				if(head->bic)
					fprintf(OutFile,"#bic=%.2f\n",(c_float)head->bic/100);
			}
			if(Sep)
				fprintf(OutFile,"#data from%sto%sdist%sfazi%sbazi%sfinc%sbinc\
%sleft%sright%sceil%sfloor%svdist\n",sep,sep,sep,sep,sep,
				sep,sep,sep,sep,sep,sep);

			if(FixArray[seg->segment]<0) {
				theLink=getLink(-FixArray[seg->segment]);
				if(!(theLink->from.segment&Link) && theLink->to.segment && theLink->data) {
					/* Is a tie. Now does it belong to this Field (FSB or Section)? */
					theLocn=(theLink->to.segment<0)? locn:locn-1;
					if(!(theLocn->code&Move_to))
						_errorAlert(UnexpectedError,seg->segment,sta);
					if(theLocn->length==seg->segment) {
						/* Will have to report the fixing tie now. */
						for(lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies
							;!fixingLink && i<lim
							;i++) {
							extLink=getLink(i);
							if(extLink->data==theLink->data) 
								fixingLink=i;
						}
						if(!fixingLink)
							_errorAlert(UnexpectedError,seg->segment,sta);
						else {
							theLocn++;
							su0=getSR(abs(extLink->to.segment));
							if(extLink->to.segment>0) {
								if(Sep)
									fprintf(OutFile,"%s%s%s%s",printPureSta(extLink->from.n,seg,a),
										sep,printPureSta(extLink->to.n,su0,b),sep);
								else
									fprintf(OutFile,"     %-12s%-12s",printPureSta(extLink->from.n,seg,a),
										printPureSta(extLink->to.n,su0,b));
							} else {
								if(Sep)
									fprintf(OutFile,"%s%s%s%s",printPureSta(extLink->to.n,su0,a),
										sep,printPureSta(extLink->from.n,seg,b),sep);
								else
									fprintf(OutFile,"     %-12s%-12s",printPureSta(extLink->to.n,su0,a),
										printPureSta(extLink->from.n,seg,b));
							}
							if(fseek(TokFile,theLocn->data.tok.data,SEEK_SET))
								_errorAlert(DiskErr,seg->segment,extLink->from.n);
							if(sefFlag&2) {
								for(i=getc(TokFile),col=30;i&&(i!='\n');col++) {
									if(col>79) {
										fprintf(OutFile,"\n/");
										col=0;
									}
									putc(i,OutFile);
									i=getc(TokFile);
								}
							} else {
								for(i=getc(TokFile);i&&(i!='\n');) {
									putc(i,OutFile);
									i=getc(TokFile);
								}
							}
							putc('\n',OutFile);
						}
					}
				}
			}
		}
	}

	if(!locn) {
		seeking=TRUE;
		goto seekLocn;
	}
/*-------------------------- top of loop -----------------------------*/

	do {	/* Keep trying as long as we have a locn fix to. */
		if(locn->code&_Rel) 
			_errorAlert(UnfixedLocn,seg->segment,sta);
		seeking=FALSE;
		
		/*------------------ find the subsegment -----------------*/
		if(!seg->age) {
			ss=NULL;
			firstFixed=lastFixed=sta;
			goto fixLinks;
		}
		for(ss=(struct subSegRec *) seg;;) {
			if(ss->first<=sta && sta<=ss->last) 
				break;
			else if (ss->nextSubSeg)
				ss=getSS(ss->nextSubSeg);
			else {
				ss=NULL;
				firstFixed=lastFixed=sta;
				goto fixLinks;

				/*			
					If ss==0, then sta is not mapped; need only to fix
					any links and ties, and then look for more to do.
				*/
			}
		}

/*------------------- B. fix string (formally fixZone) ----------------
							(the main act)
----------------------------------------------------------------------*/
		
		firstFixed=lastFixed=sta;
		
	/* 1. calculate offset     */
		
		dx=dy=dz=0;
		n=sta;
		theData=getData(ss->data+sta-ss->first);
		while((theData->code&(_Point|_Rel))==Line_to_rel) {
			if(ExchangeFormat) {
				if(sefFlag) {
					if(Sep)
						fprintf(OutFile,"%s%s%s%s",printPureSta(n-1,seg,a),sep,printPureSta(n,seg,b),sep);
					else
						fprintf(OutFile,"     %-12s%-12s",printPureSta(n-1,seg,a),printPureSta(n,seg,b));
					if(fseek(TokFile,theData->data.tok.data,SEEK_SET))
						_errorAlert(DiskErr,seg->segment,sta);
					if(sefFlag&2) {
						for(i=getc(TokFile),col=30;i&&(i!='\n');col++) {
							if(col>79) {
								fprintf(OutFile,"\n/");
								col=0;
							}
							putc(i,OutFile);
							i=getc(TokFile);
						}
					} else {
						if(sefFlag&2) {
							for(i=getc(TokFile),col=30;i&&(i!='\n');col++) {
								if(col>79) {
									fprintf(OutFile,"\n/");
									col=0;
								}
								putc(i,OutFile);
								i=getc(TokFile);
							}
						} else {
							for(i=getc(TokFile);i&&(i!='\n');) {
								putc(i,OutFile);
								i=getc(TokFile);
							}
						}
					}
					putc('\n',OutFile);
				}
			} else {
				dx+=theData->data.cart.x;		
				dy+=theData->data.cart.y;		
				dz+=theData->data.cart.z;
			}
			theData--;
			n--;
		}
		if(firstFixed > n) 
			firstFixed = n;
		if(theData->code) {
/*			_errorAlert(NotAStringStart,seg->segment,sta);	*/
/*			goto seekLocn;	*/
			goto fixLinks;
		}
		
		theData->code=Move_to;
		if(!ExchangeFormat) {
			theData->data.cart.x=dx=locn->data.cart.x-dx;
			theData->data.cart.y=dy=locn->data.cart.y-dy;
			theData->data.cart.z=dz=locn->data.cart.z-dz;

			if(str=findMappedString(seg,firstFixed)) {
				if(str->first!=firstFixed)
					_errorAlert(UnexpectedError,seg->segment,sta);
				strLength=0;
				str->code|=FixedFlag;
			}

		}
		
	/* 2. fix string    */

		while(((++theData)->code&(_Point|_Rel))==Line_to_rel) {
			n++;
			theData->code = Line_to;
			if(ExchangeFormat) {
				if(sefFlag && n>sta) {
					if(Sep)
						fprintf(OutFile,"%s%s%s%s",printPureSta(n-1,seg,a),sep,printPureSta(n,seg,b),sep);
					else
						fprintf(OutFile,"     %-12s%-12s",printPureSta(n-1,seg,a),printPureSta(n,seg,b));
					if(fseek(TokFile,theData->data.tok.data,SEEK_SET))
						_errorAlert(DiskErr,seg->segment,sta);
					if(sefFlag&2) {
						for(i=getc(TokFile),col=30;i&&(i!='\n');col++) {
							if(col>79) {
								fprintf(OutFile,"\n/");
								col=0;
							}
							putc(i,OutFile);
							i=getc(TokFile);
						}
					} else {
						for(i=getc(TokFile);i&&(i!='\n');) {
							putc(i,OutFile);
							i=getc(TokFile);
						}
					}
					putc('\n',OutFile);
				}
			} else {
				if(str) {
					strLength+=theData->length;
					if(n==str->last) {
						str->length=strLength;
						strLength=0;
						if(str->code&ContigAtEnd) {
							str=((str->string+1)&BlockLo)? str+1:getStr(str->string+1);
							str->code|=FixedFlag;
						} else
							str=0;
					}
				}
				dx=theData->data.cart.x+=dx;
				dy=theData->data.cart.y+=dy;
				dz=theData->data.cart.z+=dz;

			}
		}
		if(n > lastFixed)
			lastFixed = n;
		
		
/*----------------------- C. fix the string's links and ties -------------------*/

fixLinks:
		/* 1. fix each tie.   */
		for(numTiesToFix=0,lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies
			;i<lim;i++) {
			theLink = getLink(i);
			theLocn = getLocn(theLink->data);
			if(theLink->from.n>=firstFixed && theLink->from.n<=lastFixed) {
				if(sefFlag) {
					if((i!=fixingLink)&&((theLocn-1)->length==seg->segment)) {
						su0=getSR(abs(theLink->to.segment));
						if(theLink->to.segment>0) {
							if(Sep)
								fprintf(OutFile,"%s%s%s%s",printPureSta(theLink->from.n,seg,a),
									sep,printPureSta(theLink->to.n,su0,b),sep);
							else
								fprintf(OutFile,"     %-12s%-12s",printPureSta(theLink->from.n,seg,a),
									printPureSta(theLink->to.n,su0,b));
						} else {
							if(Sep)
								fprintf(OutFile,"%s%s%s%s",printPureSta(theLink->to.n,su0,a),
									sep,printPureSta(theLink->from.n,seg,b),sep);
							else
								fprintf(OutFile,"     %-12s%-12s",printPureSta(theLink->to.n,su0,a),
									printPureSta(theLink->from.n,seg,b));
						}
						if(fseek(TokFile,theLocn->data.tok.data,SEEK_SET))
							_errorAlert(DiskErr,seg->segment,theLink->from.n);
						if(sefFlag&2) {
							for(n=getc(TokFile),col=30;n&&(n!='\n');col++) {
								if(col>79) {
									fprintf(OutFile,"\n/");
									col=0;
								}
								putc(n,OutFile);
								n=getc(TokFile);
							}
						} else {
							for(n=getc(TokFile);n&&(n!='\n');) {
								putc(n,OutFile);
								n=getc(TokFile);
							}
						}
						putc('\n',OutFile);
					}
				}
				if(theLocn->code&_Rel) {
					/* 1a. link not previously fixed. */
					theLocn[0].code&=~_Rel;
					theLocn[-1].code=Move_to;
					/* Note: the subSegment has not changed. */
					if(!ExchangeFormat) {
						/* 1b. fix it. */
						theData=(ss)? getData(ss->data+theLink->from.n-ss->first):locn;
						if(theLink->to.segment<0) {
							theLocn[-1].data.cart.x = theData->data.cart.x-theLocn[0].data.cart.x;
							theLocn[-1].data.cart.y = theData->data.cart.y-theLocn[0].data.cart.y;
							theLocn[-1].data.cart.z = theData->data.cart.z-theLocn[0].data.cart.z;
							theLocn[0].data=theData->data;	/* This segment's data. */
						} else {
							theLocn[-1].data=theData->data;	/* This segment's data. */
							theLocn[0].data.cart.x += theLocn[-1].data.cart.x;
							theLocn[0].data.cart.y += theLocn[-1].data.cart.y;
							theLocn[0].data.cart.z += theLocn[-1].data.cart.z;
						}
					}

					n=abs(theLink->to.segment);
					if(n!=seg->segment) {
						if(FixArray[n]==0 || (FixArray[n]>0 && !(FixArray[n]&FixedFlag))) {
							FixArray[n] = -i;
#ifdef DEBUG
							printTrace(seg,theLink);
#endif
						}  
					}
				} else if(!ExchangeFormat) { 
					if(theLocn->code&CloseShape) {
						theLocn[0].code|=Line_to;
						theLocn[-1].code=Move_to;
						/* 	CloseShape ties always run from the last station of a segment to
							the first station. They should be encountered from the first station,
							but I had better check. */
						if(theLink->from.n != 1) 
							theData = (ss)? getData(ss->data+theLink->from.n-ss->first):locn;
						else
							theData = (ss)? getData(ss->data+theLink->to.n-ss->first):locn;
						extLocn=(ss)? getData(ss->data):locn;
						theLocn[-1].data = theData->data;	/* This segment's data. */
						theLocn[0].data = extLocn->data;
					}
				}
			}
			if(theLocn->code&_Rel)
				numTiesToFix++;
		}
		
		/* 2. fix the links */
		for(lim=i=seg->links+seg->nLocns,lim+=seg->nLinks
			;i<lim;i++) {
			theLink = getLink(i);
			if(theLink->from.n>=firstFixed && theLink->from.n<=lastFixed) {
				if((n=abs(theLink->to.segment)!=seg->segment)) {
					if(FixArray[n]==0 || (FixArray[n]>0 && !(FixArray[n]&FixedFlag))) {
						FixArray[n] = -i;
#ifdef DEBUG
						printTrace(seg,theLink);
#endif
						/* 	Modify the link record, by adding an index to the fixed
						locn record i. This is necessary as the segment number of
						the tie (from.segment) will not be available. */
						theLink->from.segment |= Link;
						theLink->data=(ss)?ss->data+theLink->from.n-ss->first:0;
					/*==== Problem: if ss==0, then link is unmapped, yet need to point it
							to locn. How do we find out locn's index? =======*/
					}
				} 
			}
		}
			
		if(numTiesToFix) {	/* Check again. */
			for(numTiesToFix=0,lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies
				;i<lim;i++) {
				theLink = getLink(i);
				theLocn = getLocn(theLink->data);
				if(theLocn->code&_Rel) {
					numTiesToFix=1;
					break;
				}
			}
		}
		if(numTiesToFix) 
			seg->flags |= PartiallyFixed;	/* Have to check strings */
		else {
			if(ss) {
				/*	Is there any chance we have not gotten all of the subsegment? */
				if(ss->flags & Gap) 
					seg->flags |= PartiallyFixed;
				else {
					seg->flags |= FixedFlag;
					ss->flags |= FixedFlag;
					if(!seg->nextSubSeg) {
						/* It's an ungapped seg, and there are no subsegments. */
						seg->flags |= SegmentFixed;
						if(sefFlag) fprintf(OutFile,"#endctsurvey\n");
						return FixedFlag;	/* Most common exit. */
					}
				}
			} else if(!seg->age) {
				/* No subsurvey as there are no main data. */
				seg->flags |= (SegmentFixed|FixedFlag);
				if(sefFlag) fprintf(OutFile,"#endctsurvey\n");
				return FixedFlag;	/* Another common exit. */
			}
			/* 	The case with no ss in an aged segment requires checking
				the strings. */
			seg->flags|=PartiallyFixed;
		}
			
/*------------------------- Rare things follow ------------------------*/

/*-------------------------- D. check strings ------------------------*/
		
		for(i=seg->strings,numStringsToFix=0;i<seg->strings+seg->nStrings;i++) {
			str=getStr(i);
			if(str->code&FixedFlag)
				continue;
			if(str->code&PartiallyFixed)
				numStringsToFix++;
			else if(str->code&UnMapped)
				continue;
			if(str->first < firstFixed)
				numStringsToFix++;
			else if(str->last <= lastFixed) 	/* within fixed zone */
				str->code |= FixedFlag;
			else if(str->last > lastFixed)
				numStringsToFix++;
		}
		str=0;
		if(!numStringsToFix && !numTiesToFix) {
			seg->flags|=(SegmentFixed|FixedFlag);
			if(sefFlag) fprintf(OutFile,"#endctsurvey\n");
			return FixedFlag;
		}

		
/*------------------------ E. find more to do -------------------------*/

		/*
			A gap in a subsegment is very rare. It is best to try to Fix the
			subsegment while we have it. We can afford to go to elaborate
			measures. It seems best to use strings. First set the code of
			those that are fixed. This will be useful for subsequent calls;
			subSegment.flag set to PartiallyFixed is the key that the strings
			have been so set.
		*/
		
	/* 1. now try to fix unfixed strings. We are looking for from.n's in an unfixed string.
	   */
seekLocn:
		locn=extLocn=0;		/* locn is now be used as a local variable. */
		
		for(j=seg->strings,numStringsToFix=seg->nStrings;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);
			if(str->code & FixedFlag) {
				numStringsToFix--;
				continue;
			}
			if(str->code & PartiallyFixed) {
				/*	
					This is a KEY TEST. PartiallyFixed can only occur when
					CloseFlag is TRUE. The main data of this string was fixed
					during loop adjustment. Neither its links nor any dead-end
					parts of its subSegment were fixed.
				*/
				if(str->code&UnMapped) {
					str->code|=FixedFlag;
					firstFixed=lastFixed=str->first;
					if(str->nFirst>=0)
						_errorAlert(UnexpectedError,seg->segment,str->first);
					theLink=getLink(-(str->nFirst+1));	/* Cipher for crossing tie. */
					if(theLink->data) 
						/* We want the from-station. */
						locn=(theLink->to.segment>0)? getLocn(theLink->data-1):getLocn(theLink->data);
					else
						locn=getFixedData(getSR(theLink->to.segment),theLink->to.n);
					if(locn->code&_Rel)
						_errorAlert(UnexpectedError,seg->segment,str->first);
					ss=0;	
				} else
					ss=fixSubSegment(seg,str,&firstFixed,&lastFixed); 
				goto fixLinks;
			}
			if(str->code & UnMapped) {
				numStringsToFix--;
				continue;
			}
				
			/* a. Is there a second locn ?  */
				
			for(i=seg->links+1;i<seg->links+seg->nLocns;i++) {
				theLink = getLink(i);
				if(theLink->from.n==str->first 
						||theLink->from.n==str->last) {
					locn=getLocn(theLink->data);
					extLink=theLink;
					theLink->to.n |= FixedFlag;
					break;
				}
			}
			if(locn)
				break;
	
			/* b. is there a fixed tie?  */
														
			for(lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies
				;i<lim;i++) {
				theLink = getLink(i);
				if(theLink->from.n==str->first 
						||theLink->from.n==str->last) {
					theLocn = getLocn(theLink->data);
					if((theLocn->code&_Point) && !(theLocn->code&_Rel)) {
						/*	We have fixed data. Is it in the same segment? 
							If so, it receives priority. */
						if((abs(theLink->to.segment)==seg->segment)) {
							locn = (theLink->to.segment>0)? theLocn-1 : theLocn;
							extLink = theLink;
							break;
						} else if(!extLocn) {
							/*	This is a tie from a fixed segment, no priority.
								Save it, we might use it later. */
							extLocn = (theLink->to.segment>0)? theLocn-1 : theLocn;
							extLink = theLink;
						}
					}
				}
			}
			if(locn) 
				break;
				
			/* c. Is there a fixed link?   */
			
			for(lim=i=seg->links+seg->nLocns,lim+=seg->nLinks
				;i<lim;i++) {
				theLink = getLink(i);
				if(theLink->from.n==str->first 
						||theLink->from.n==str->last) {						
					if(theLink->to.segment==seg->segment) {
						if(locn=getFixedData(getSR(theLink->to.segment),theLink->to.n)) {
							extLink = theLink;
							break;
						}
					} else if(!extLocn && FixArray[theLink->to.segment]>0) {
						if(extLocn=getFixedData(getSR(theLink->to.segment),theLink->to.n))
							extLink = theLink;
					}
				}
			}
			if(locn)
				break;
		}	/* Bottom of seeking loop */
		
		str=0;
		if(seeking && !numStringsToFix)
			seeking=FALSE;
		
		if(locn=(locn)?locn:extLocn)
			sta=extLink->from.n;
		else if(numTiesToFix) {	
			/* Are there other fixed locations? */
			for(i=seg->links+1;i<seg->links+seg->nLocns;i++) {
				theLink = getLink(i);
				if(!(theLink->to.n & FixedFlag)) {
					locn=getLocn(theLink->data);
					sta=theLink->from.n;
					theLink->to.n |= FixedFlag;
					break;
				}
			}
			if(!locn) { /* No locn. Are there ties to multi-tie nodes to fix? */
				for(extLink=0,lim=i=seg->links+seg->nLocns+seg->nLinks,lim+=seg->nTies
					;i<lim;i++) {
					theLink=getLink(i);
					theLocn=getLocn(theLink->data);
					if((theLocn->code&_Point) && !(theLocn->code&_Rel)) {
						extLocn=theLocn;	/* Here is a fixed tie. */
						extLink=theLink;
						continue;		/* Save it and look ahead. */
					}
					if(extLink && (extLink->from.n == theLink->from.n)) {
						/* Do we have an unfixed ties ahead of a fixed one? */
						locn=(extLink->to.segment>0)? extLocn-1 : extLocn;
						sta=theLink->from.n;
						break;
					} else if(i<lim-1) {
						/* Is there a fixed tie ahead of an unfixed one? */
						extLink=getLink(i+1);
						extLocn=getLocn(extLink->data);
						if(extLink->from.n==theLink->from.n && (extLocn->code&_Point) && !(extLocn->code&_Rel)) {
							locn = (extLink->to.segment>0)? extLocn-1 : extLocn;
							sta = theLink->from.n;
							break;
						}
					}
					extLink=0;	/* not a preceeding fixed link */
				}
			}
		}
	} while(locn);	/* Bottom of loop */
	
	if(sefFlag) fprintf(OutFile,"#endctsurvey\n");
	if(seeking)
		return 0;
	return (numTiesToFix || numStringsToFix)? PartiallyFixed : FixedFlag;
}
			
struct  subSegRec *fixSubSegment(struct segmentRec *seg,struct strRec *str,
	int *firstFixed,int *lastFixed)
{
	register int n;
	struct locnRec *theData;
	struct strRec *str0;
	struct subSegRec *ss=0;
	int strLength;
	c_float dx,dy,dz;
	
	/*--------------------------------------------------------------------- 
		This works in cooperation with the very complicated older routine, 
		fixSegment. It is called to fix an entire subSegment based on one 
		partially fixed string.
		
		It is passed a stringRec, its segmentRec. It returns the first
		and last stations fixed. 
		It only receives mapped strings, and always the lowest fixed
		string in a subsegment. 
		It can receive an unMapped string but it will have already been
		fixed, just as a SuperStringMember.
	
	---------------------------------------------------------------------*/
			
					/* 
	1.	If there is a contiguous string before this one, then
		it is a dead-end one (i.e. the trunk of a tree). It has already
		been fixed, but we need to go throught it to get firstFixed.
		
					*/
					 	
	str0=str;
	while(str0->code&ContigAtStart)
	if(str->code&ContigAtStart)
		str0=((str0->string-1)&BlockLo)? str0-1:getStr(str0->string-1);
	*firstFixed=str0->first;
	str->code|=FixedFlag;
	
					/* 
	
	2.	If there is a contiguous string after this one, then
		it is a dead-end one (i.e. the trunk of a tree). 
		
		Look for it. Fix it, but note that the procedure differs.
					*/
		
	if(str->code&ContigAtEnd) {
		/* 
			We will use the strRecs, that are a mapping of the main
			data, to skip through all of the contiguous data that 
			already has absolute coordinates.
		*/
		
		str0=str;
		str0->code |= FixedFlag;
		while((str0->code&(ContigAtEnd|PartiallyFixed))==(ContigAtEnd|PartiallyFixed)) {
			/* 	Step through to end of partially fixed part of ss. This loop
				will not continue to the next segment. */
			str0=((str0->string+1)&BlockLo)? str0+1:getStr(str0->string+1);
			str0->code |= FixedFlag;
			*lastFixed=str0->last;
		}
		if(!(str0->code&PartiallyFixed)) {
			/* Still in same subSegment, at base of a tree. Fix it. */
			n=str0->first;
			theData=_getMainData(seg,n,&ss);
			dx=theData->data.cart.x;
			dy=theData->data.cart.y;
			dz=theData->data.cart.z;
			strLength=0;
			while(((++theData)->code&(_Point|_Rel))==Line_to_rel) {
				theData->code = Line_to;
				strLength+=theData->length;
				if((++n)==str0->last) {
					str0->length=strLength;
					strLength=0;
					if(str0->code&ContigAtEnd) {
						str0=((str0->string+1)&BlockLo)? str0+1:getStr(str0->string+1);
						str0->code |= FixedFlag;
					}
				} 
				dx=theData->data.cart.x+=dx;
				dy=theData->data.cart.y+=dy;
				dz=theData->data.cart.z+=dz;
			}
			if(n!=(*lastFixed=str0->last))
				_errorAlert(UnexpectedError,seg->segment,n);
		}
	} else
		*lastFixed=str->last;
			
	if(!ss)
		_getMainData(seg,str->first,&ss);

	return ss;
}

#ifdef MAC
#pragma segment extents
#endif

void findExtents(void)
{
	int i;
	struct segmentRec *seg;
	
	for(i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		if(seg->flags&FixedFlag) {
			if(!findSegmentBounds(seg))
				_errorAlert(UnexpectedError,i,0);
			UNE=seg->UNE;
			LSW=seg->LSW;
			gTopMost.segment=gBottomMost.segment=
				gWestMost.segment=gEastMost.segment=
				gNorthMost.segment=gSouthMost.segment=i;
			gTopMost.n=seg->top;
			gBottomMost.n=seg->bottom;
			gEastMost.n=seg->east;
			gWestMost.n=seg->west;
			gNorthMost.n=seg->north;
			gSouthMost.n=seg->south;
			break;
		}
	}
	for(i=2;i<=SegList->current;i++) { 
		seg=getSR(i);
		if(seg->flags&FixedFlag) {
			if(!findSegmentBounds(seg))
				_errorAlert(UnexpectedError,i,0);
				
			if (seg->UNE.x>UNE.x) { 
				UNE.x=seg->UNE.x; 
				gEastMost.segment=i;
				gEastMost.n=seg->east;
			}
			if (seg->UNE.y>UNE.y) { 
				UNE.y=seg->UNE.y; 
				gNorthMost.segment=i;
				gNorthMost.n=seg->north;
			}
			if (seg->UNE.z>UNE.z) { 
				UNE.z=seg->UNE.z; 
				gTopMost.segment=i;
				gTopMost.n=seg->top;
			}
			if (seg->LSW.x<LSW.x) { 
				LSW.x=seg->LSW.x; 
				gWestMost.segment=i;
				gWestMost.n=seg->west;
			}
			if (seg->LSW.y<LSW.y) { 
				LSW.y=seg->LSW.y; 
				gSouthMost.segment=i;
				gSouthMost.n=seg->south;
			}
			if (seg->LSW.z<LSW.z) { 
				LSW.z=seg->LSW.z; 
				gBottomMost.segment=i; 
				gBottomMost.n=seg->bottom; 
			}
		}
	}
	return;
}

static int findSegmentBounds(struct segmentRec *seg)
{
	int i,k,belongs;
	short sta;
	struct subSegRec *ss;
	struct locnRec *lo;
	LinkRecPtr li;
	c_float minx,miny,minz,maxx,maxy,maxz;
	short north,south,east,west,top,bottom;
	
	if(seg->data) {
		ss=(struct subSegRec *)seg;
		lo=getData(ss->data);
		maxx=minx=lo->data.cart.x;
		maxy=miny=lo->data.cart.y;
		maxz=minz=lo->data.cart.z;
		north=south=east=west=top=bottom=ss->first;
		do {
			for(lo++,sta=ss->first+1;sta<=ss->last;sta++,lo++) {
				if(lo->code) {
					if(minx>lo->data.cart.x) {
						minx=lo->data.cart.x;
						west=sta;
					} else if(maxx<lo->data.cart.x) {
						maxx=lo->data.cart.x;
						east=sta;
					} 	
					if(miny>lo->data.cart.y) {
						miny=lo->data.cart.y;
						south=sta;
					} else if(maxy<lo->data.cart.y) {
						maxy=lo->data.cart.y;
						north=sta;
					}
					if(minz>lo->data.cart.z) {
						minz=lo->data.cart.z;
						bottom=sta;
					} else if(maxz<lo->data.cart.z) {
						maxz=lo->data.cart.z;
						top=sta;
					} 
				}
			}
		} while(ss=(ss->nextSubSeg)?getSS(ss->nextSubSeg):0);
	} else if(seg->links) {
		/* Still need to fill maxx-minz. Take the first locn/link/tie. */
		li=getLink(seg->links);
		if(li->data && !(li->from.segment&Link)) {
			lo=getLocn(li->data);
			if(li->to.segment>0)	/* Tie and from-station is in this survey. */
				lo--;
		} else 						/* Link */
			lo=getMainData(getSR(li->to.segment),li->to.n);
			
		north=south=east=west=top=bottom=li->from.n;
		maxx=minx=lo->data.cart.x;
		maxy=miny=lo->data.cart.y;
		maxz=minz=lo->data.cart.z;

	} else 
		return FALSE;

	/* 	Now check every locn, link and tie, any of which may be to unmapped 
		strings. */
	k=seg->links+seg->nLocns+seg->nLinks;
	for(i=seg->links;i<k+seg->nTies;i++) {
		li=getLink(i);
		if(li->data && !(li->from.segment&Link)) {
			lo=getLocn(li->data);
			if(li->to.segment>0) {	/* Tie and from-station is in this survey. */
				lo--;
				belongs=lo->length==seg->segment;
			} else if(li->to.segment)
				belongs=lo[-1].length==seg->segment;
			else
				belongs=0;
		} else 						/* Link */
			lo=getMainData(getSR(li->to.segment),li->to.n);
		if(lo) {
			if(minx>lo->data.cart.x) {
				minx=lo->data.cart.x;
				west=li->from.n;
			} else if(maxx<lo->data.cart.x) {
				maxx=lo->data.cart.x;
				east=li->from.n;
			}
			if(miny>lo->data.cart.y) {
				miny=lo->data.cart.y;
				south=li->from.n;
			} else if(maxy<lo->data.cart.y) {
				maxy=lo->data.cart.y;
				north=li->from.n;
			}
			if(minz>lo->data.cart.z) {
				minz=lo->data.cart.z;
				bottom=li->from.n;
			}
			else if(maxz<lo->data.cart.z) {
				maxz=lo->data.cart.z;
				top=li->from.n;
			}
			if(i>=k && belongs) {	/* Tie belonging to this segment. */
				lo+=(li->to.segment>0)? 1 : -1;
				if(minx>lo->data.cart.x) {
					minx=lo->data.cart.x;
					west=li->from.n;
				} else if(maxx<lo->data.cart.x) {
					maxx=lo->data.cart.x;
					east=li->from.n;
				}
				if(miny>lo->data.cart.y) {
					miny=lo->data.cart.y;
					south=li->from.n;
				} else if(maxy<lo->data.cart.y) {
					maxy=lo->data.cart.y;
					north=li->from.n;
				}
				if(minz>lo->data.cart.z) {
					minz=lo->data.cart.z;
					bottom=li->from.n;
				} else if(maxz<lo->data.cart.z) {
					maxz=lo->data.cart.z;
					top=li->from.n;
				}
			}
		}
	}
#if 0
	if(seg->UNE.x!=maxx || seg->UNE.y!=maxy || seg->UNE.z!=maxz ||
		seg->LSW.x!=minx || seg->LSW.y!=miny || seg->LSW.z!=minz) {
		fprintf(OutFile,"Bad Bounds with %d%.4s (%d)\n",seg->Field,seg->desig,seg->segment);
		fprintf(OutFile,"\t%6.1fW to\t%6.1fE,\t%6.1fS to\t%6.1fN\n",
			seg->LSW.x,seg->UNE.x,seg->LSW.y,seg->UNE.y);
		fprintf(OutFile,"\t%6.1fW to\t%6.1fE,\t%6.1fS to\t%6.1fN\n",
			minx,maxx,miny,maxy);

	}
#endif
	seg->UNE.x=maxx;
	seg->UNE.y=maxy;
	seg->UNE.z=maxz;
	seg->LSW.x=minx;
	seg->LSW.y=miny;
	seg->LSW.z=minz;
	seg->top=top;
	seg->bottom=bottom;
	seg->west=west;
	seg->east=east;
	seg->north=north;
	seg->south=south;
	return TRUE;
}

#ifdef DOS

struct tm *cmllocaltime(const time_t *timer)
{
	static struct tm da;
	int i,j,leapYear;
	unsigned long temp,temp2;

	temp=*timer;
	da.tm_sec=temp%60;		/* Seconds after the minute -- [0, 61] */
	temp/=60;
	da.tm_min=temp%60; 		/* Minutes after the hour -- [0, 59] */
	temp/=60;
	da.tm_hour=temp%24;		/* Hours after midnight -- [0, 23] */
	temp/=24;				/* days since Jan 1, 1904 */
	da.tm_wday=(temp+5)%7;	/* Days since Sunday -- [0, 6] 1/1/04 was
								a Friday. */
	temp+=1460;				/* 4*365; Reference to 1900, which was not a leap year. */
	da.tm_year=temp2=temp/365;
	/* Calculate Days since January 1, taking account of leap years. */
	leapYear=(da.tm_year%4)? 0:1;
	if((da.tm_yday=leapYear+temp-temp2*365-temp2/4)<0) {
		da.tm_year= --temp2;
		leapYear=(da.tm_year%4)? 0:1;
		da.tm_yday=leapYear+temp-temp2*365-temp2/4;
	}

	/* 	We are through. 2000 A.D. is divisible by 400 and so
		is a leap year. */

	for(i=j=0;i<12 && (da.tm_yday>=DaysToMonth[i]+j);i++) 
		j=(i && leapYear)? 1:0;
	da.tm_mon=i-1;
	j=(i>2)? 1-j:1;
	da.tm_mday=da.tm_yday-DaysToMonth[da.tm_mon]+j;
	return &da;
}

char *cmltime(const time_t *timer)
{
	static Str31 buf;
	struct tm *da;
	
	da=cmllocaltime(timer);
	sprintf(buf,"%s %s %2d %.2d:%.2d:%.2d %4d\n",WeekDay[da->tm_wday],Month[da->tm_mon],
		da->tm_mday,da->tm_hour,da->tm_min,da->tm_sec,1900+da->tm_year);
	return buf;
}

#endif

#if defined(DOS) || defined UNIX
/* from Bill Purvis */
void RotateCursor(int n)
{
	static char chars[] = "\\|/-";
	n &= 3;
	putc(chars[n],stdout);
	putc(8, stdout);		/* backspace */
	fflush(stdout);
}
#endif

#ifdef MAC
#pragma segment steps
#endif

#ifdef MDLView
void RotateCursor(int)
{
	return;
}
#endif

#if defined(MDLView) || defined(VECTORS)

char *getLine(char *p, int n, FILE *stream)
{
	char *q;
	int cnt;
	
	if(StreamIsFile)
		return fgets(p,n,stream);
	q=p;
	cnt=stream->_cnt;
	while(n-- && cnt) {
		cnt--;
		if((*p++ = *stream->_ptr++)=='\n') {
			*p='\0';
			stream->_cnt = cnt;
			return q;
		}
	}
	stream->_cnt = cnt;
	return 0;
}
#endif

int nextNode(struct segmentRec *seg)
{
	static int sta,stri=0;
	static struct strRec theString;
	
	if(stri) {
		if(sta==theString.first && !(theString.code&UnMapped))
			return(sta=theString.last);
		for(++stri;stri<seg->strings+seg->nStrings;stri++) {
			theString=*getStr(stri);
			if(sta!=theString.first)
				return(sta=theString.first);
			else if(!(theString.code&UnMapped))
				return(sta=theString.last);
		}
	} else {
		/*	First call for this segment. Return the first string, if there is one. */
		stri=seg->strings;
		if(stri<seg->strings+seg->nStrings) {
			theString=*getStr(stri);
			return(sta=theString.first);
		}
	}
	/*	Else, nothing more for this survey. */
	stri=0;
	return(-1);
}

#ifndef CMLmake

#ifdef MAC
#pragma segment transforms
#endif

int processFids(LinkRecPtr liBuff)
{
	int i,j,n,masterFid,cnt;
	struct segmentRec *seg,*seg0;
	struct fieldRec *se,*se0;
	LinkRecPtr li,li0;
	struct locnRec *lo;
	struct cartData *lastFidData;
	c_float ang;
	
	/*
		This is called in p1.5, just after the first sort of the 
		linkSortBuffer, which is what is passed to processFids. This
		routine (1) finds the fid ties, points the fid segments to them,
		then (2) determines each sections master fid and complete the
		ties for each fid.
		
		The link records representing fids have been doubled earlier in p1.5,
		that is, there are twice as many records as fids. The nFids fields of
		the section record contains the number of fids. The first nFids records
		are place holders, having fictitious  station numbers of zere. The field
		of these records will be completely replaced now. The second nFids link
		records have fictitious  to fields that will be replaced now. The first
		link record will be the tie to the previous section; it will point down.
		The nFids'th link record points up to the next section.
	*/
		

	/*	
		1. Go through sections in their sorted order, finding the true master fid,
		linking it to fid0, then calculating the correction angle and offset. 
		
		The first section is handled simply. The first fid is master and there
		is no rotation.
	*/
	
	se0=getField(ThisContext->firstField);
	if(!se0->fidSegment) {
		errorAlert(NoFids);
		return 0;
	}
	seg0=getSR(se0->fidSegment);
	
	/*	The first true fid is nFids from the end. */
	li=liBuff+seg0->links+seg0->nTies-se0->nFids;
	se0->localDatum=li->from;
	lo=getLocn(li->data);
	se0->datumData=lo->data.cart;
	/*	Make it a zero-length tie (fid0->fid1) although it should be made into a tie. */
	/* lo->data.cart.x=lo->data.cart.y=lo->data.cart.z=0; */
		
	li0=liBuff+seg0->links;
	for(i=0;i<seg0->nTies;i++) {
		if((li0+i)->to.n && (li0+i)->to.segment>0) 	{
			lo=getLocn((li0+i)->data);
			lo->data.cart.x-=se0->datumData.x;
			lo->data.cart.y-=se0->datumData.y;
			lo->data.cart.z-=se0->datumData.z;
		}
	}
		
	/*	Loop through the next sections. */
	
	for(;se0->nextField;se0=se,seg0=seg) {
	
		se=getField(se0->nextField);
		seg=getSR(se->fidSegment);
		
		li0=li;
		li=liBuff+seg->links+seg->nTies-se->nFids;
		
		/* 1. Find the master fid by looking for the first matching from.n's
			in li (in this section) and li0 (in the preceeding section). */
			
		for(;li0<liBuff+seg0->links+seg0->nTies && li<liBuff+seg->links+seg->nTies
			;(n<0)? li0++:li++) {
			if(!(n=li0->from.n - li->from.n))
				break;
		}
		if(n) {
			errorAlert(NoMatchingFid);
			return 0;
		} else {
			masterFid=li->from.n;			/* First matching fid */
			se->localDatum=li->from;
			lo=getLocn(li->data);
			se->datumData=lo->data.cart;
				/*	Will make it a zero-length tie (fid0->fid1) although 
					it should be made into a tie. */
			lastFidData=&(se0->datumData);
		}
		
		
		/* 2. Calculate the rotation for this section by matching fids
		from the previous section. li and li0 are still pointing to
		the matching fids. */
	
		for(i=j=1,cnt=0,ang=0;
			i<se->nFids && j<se0->nFids;) {
			if(n=(li0+j)->from.n - (li+i)->from.n)
				(n<0)? j++:i++;
			else {
				lo=getLocn((li+i)->data);
				ang+=atan2(lo->data.cart.y-se->datumData.y,lo->data.cart.x-se->datumData.x);
				lo=getLocn((li0+j)->data);
				ang-=atan2(lo->data.cart.y,lo->data.cart.x);
				i++;
				j++;
				cnt++;
			}
		}
		ang/=(cnt)? cnt:1;
		se->corrAngle=ang;
		
		/*	2d. Rotate the present section's fids. */
		
		li=liBuff+seg->links;
		for(i=0;i<seg->nTies;i++) {
			if((li+i)->to.n && (li+i)->to.segment>0) {
				lo=getLocn((li+i)->data);
				transformCart(&(lo->data.cart),ang,&(se->datumData));
			}
		}

	}
	return 1;
}
 
void transformCart(struct cartData *data,c_float angle,struct cartData *offset)
{
	/* 2D transform */
	c_float x,y,theta,len;
	if(offset) {
		x=data->x-offset->x;
		y=data->y-offset->y;
		data->z-=offset->z;
	} else {
		x=data->x;
		y=data->y;
	}
	theta=atan2(y,x)-angle;
	len=hypot(y,x);
	data->x = len*cos(theta);
	data->y = len*sin(theta);
	return;
}

void sortSections(void)
{
	int i;
	size_t nSects;
	struct fieldRec *se,*se0;
	LinkRecPtr li;
	struct locnRec *lo;
	short *fieldSortList;
	
	/* 1. Determine the section order by sorting the elements of 
		a temporary array of indexs. */
	
	fieldSortList = ecalloc(FieldLList->current,2*sizeof(short));
	for(nSects=0,i=1;i<=FieldLList->current;i++) {
		se = getField(i);
		if(se->ThisField) {
			*(fieldSortList+nSects++)=se->plane;
			*(fieldSortList+nSects++)=i;
		}
	}
	
	/* The sort is base on Field or plane number. */
	
	qsort(fieldSortList,nSects/2,2*sizeof(short),FieldSortCompare);
	
	ThisContext->firstField = *(fieldSortList+1);
	se = getField(*(fieldSortList+1));
	se->lastField=-1;
	
	/* 2. Make a doubly linked list of the section records. 
		Also, make a tie for the master fids between section. */
	
	for(i=3;i<nSects;i+=2) {
		se0=se;
		se0->nextField = *(fieldSortList+i);
		se = getField(*(fieldSortList+i));
		se->lastField = se0->ThisField;
		if(se->fidSegment) {
			li=nextFromList(P1LList,linkCast);
			li->from.segment=se0->fidSegment;
			li->to.segment=se->fidSegment;
			li->to.n=li->from.n=0;	/* May have to be changed if the is a jog in the fids. */
				/* Don't split the two records of a tie accross a block. */
			lo=nextFromList(P1LocnList,locnCast);
			if((P1LocnList->current & BlockLo) == BlockLo) 
				/* Don't split the two records of a tie accross a block. */
				lo=nextFromList(P1LocnList,locnCast);
			lo->code=0;
		
			lo=nextFromList(P1LocnList,locnCast);
			li->data=P1LocnList->current;
			lo->data.cart.x=lo->data.cart.y=0;
			lo->data.cart.z=(se->ThisField-se->lastField)*se->dimension;
			lo->code=Line_to_rel|Virtual;
		} 
	}
	se->nextField = 0;
	
	free(fieldSortList);
	return;
}

int FieldSortCompare(short *a,short *b)
{
	/*
		The structure being searched is a 2*short array. a[0] is the plane
		of the section number whose number is contained in a[1].
	*/
	int n;
	if(n=*a - *b)
		return n;
	return *(a+1) - *(b+1);
}

int lscompare(short *a,LinkRecPtr b)
{
	return *a - b->from.segment;
}
#endif
