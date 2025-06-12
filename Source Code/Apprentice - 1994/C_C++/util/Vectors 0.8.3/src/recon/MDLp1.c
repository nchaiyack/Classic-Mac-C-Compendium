/*-------------------------------------------------------------------
	p1aFile
	
	Program control is by a State variable, determined for each line.
	This is, perhaps, an obtuse method, but it is fast. It is so
	deterministic that it may, as well, be easy to maintain, enhance,
	and alter.

	Action is determined by the sum of:
	
	parseRec			syntax				Previous
	State				State				State
	(Left.flag &		(Previous.flag)		(Tentative.flag)
	 Right.flag) 
____________________________________________________________________

	 Field+A#	1 		:		0x100	 	Previous	0x1000
	 A#			2		=		0x200	 	Tentative	0x2000
	 #			3		->		0x300	 	  (both)	
	 blank		4		..		0x400		none		0
	 lit		5		,		0x800
	 dot		6		error	0
	 locn		7
	 label		8
	 error		0
____________________________________________________________________

Station Increment and Line Order.

Station increment is always non-zero and, by default, set to 1. Its only 
other possible setting  is -1. A prospective station increment is calculated 
after each link or tie (0x2000 cases) and for each data line that contains a 
numbered station. For the 0x2000 cases, a increment of zero can result, in 
which case the line increment is set to the default value of 1. A subsequent 
data line with a numbered station can set it to -1. For the 0x1000 cases, a 
value of zero is an error, and an absolute value greater than 1 is an implicit 
tie.

Line order is known by the first (i.e. A10: ..) or second data line (the first 
in which main storage is needed). 

Reversed blocks are the only special situation. They are set up by a -1 
station increment in a 0x2000 or 0x1000 data line.

----------------------------------------------------------------------------*/


#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#ifdef CMLmake
#include "CML.h"
#include "CMLglobs.h"
#else
#include "MDL.h"
#include "MDLglobs.h"
#endif

#ifdef MAC
	#pragma segment pass1
#else
	#include <search.h>
#endif
#ifdef MPW
	#include <CursorCtl.h>
#endif


/*	functions in MDLp1.c	*/
void doP1half(void);
int p1aFile(char *theFileName);
int tryToResolve(struct parseRec *parsed,struct parseRec *active);
void p1Planar(int dl);

int p1aFile(char *theFileName)
{
	FILE *theFile;
	int temp;
	int reverse=FALSE;
	int dataLocn;
	char *p;
	struct context *then;
	LinkRecPtr theLink;
	struct locnRec *theLocn;
	struct lineRec *theLine;
	
	/* 	
		Between the data in startLineCnt, LineCnt, and s-Scratch, an MPW type
		of error message can be printed. A 2nd parameter must be passed to 
		errorAlert.
	*/
	
	if(!(theFile=getFile(theFileName,InBuf,NChars))) {
		errorAlert(FileInErr);
		return (1); 				/* Fatal Err, no input file, so quit */
	}

	if(!Silent)
		fprintf(OutFile,"; Processing %s\n",theFileName);
		
	ShotFlag=Line_to_rel;
	LineOrder=0;
	then=ThisContext;
	ThisContext=newContext(1,theFileName,theFile);
	
	/*	
		To allow errorAlert messages that reference the line number
		in the file currently being processes. Char place is s-Scratch.
	*/
		
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	ThisContext->s0=Scratch2;
	*(ThisContext->s0)=0;
	ThisContext->lastLine=Scratch;
	TheOpenField=0;

	/*--------------------- Main loop of p1aFile follows -------------------*/
	
	while ((s=getLine(ThisContext->lastLine,MaxInLine,theFile)) != NULL) {
		ThisContext->lastLine=ThisContext->s0;
		ThisContext->s0=s;
		LineCnt++;	 	/* Global long that is the running linecount */
		ContinuationLines=0;

		RotateCursor(LineCnt);
#if 0
		if(NoProcessFlag) {
			clearSpace;
			if(*s=='#') {
				s++;
				processDirective(4);
			}
			continue;
		}
#endif
		/* Get Left */
		dataLocn=0;		/* will go into linkRec to point to a locnRec */
		Left.flag=Right.flag=Previous.flag=0;
		clearSpace;
		switch (*s) {	/* Look at initial non-whitespace char. */
			case '#':
				s++;
				if(processDirective(1))
					continue;
				else
					goto decision;
			case '.':
				if(isdigit(*(s+1)))
					goto doLength;
				s++;
				Left=Previous;
				Left.flag=6;
				break;
			case ';': case '\0':			
				/* no action needed, ignore the line. */
				continue;
			case '+':		/* line to be averaged with last */
				if(Tentative.flag & 0x3000)
					averageDataLine(theLocn);
				continue;
			case '\'':		/* a literal station name */
				Left.flag=5;
				findLit(&Left);
				break;
			case '$':
				s++;
				Left.flag=1;
				parseSegment(&Left);
				Left.this.segment=assignSegment(Left,Field,NoP1Resolve);
					/* If Field==0 in a 10, then cannot be resolved in p1. */
				break;
			default:
				if (isdigit(*s)) {	/* The most common case requires speedy handling */
					p=s++;
					while(isdigit(*s)) s++;	/* so leap ahead to see if a Left */
					_clearSpace;
					if(*s=='-') {	/* Could be "12 ->" or "12 -90" */
						s++;
						_clearSpace;
					}
					if (*s==':' || *s=='=' || *s=='>' || *s=='\'') {
						/* The digits are part of a segment station. Must check it. */
						Left.flag=3;
						s=p;
						Left.this.n=getPosNum();
						while(*s=='\'') { s++; Left.this.n += StationPrimeRadix;}
						if(Left.this.n>15000)
							errorAlert(TooManyPrimes);
						break;	/* to get verb. */
					}
					/*
						From here on: No station #, so have encountered a Length.
						Normal Data Line; there is no verb. 
						The following is the only action necessary. 
					*/
doLength:			if(Tentative.flag & 0x3000) { 
						Previous.this.n+=ThisContext->stationIncr;
						Tentative.flag=0x1000;	/* Extinguish any Tentative branch. */
						continue;
					} 
					errorAlert(BaldShot);	/* i.e., station data with no antecedent. */
					continue;
				}
				if (isalpha(*s)) {
					Left.flag=2;
					parseSegment(&Left);
					if(Tentative.flag == 0x1000) {
						if(desigCmp(Left.desig,Previous.desig)) {
							Left.Field=Previous.Field;
							Left.this.segment=Previous.this.segment;
						} else
							Left.this.segment=assignSegment(Left,Field=TheOpenField,Resolve);
					} else if(Tentative.flag == 0x2000) {
						if(desigCmp(Left.desig,Tentative.desig)) {
							Left.Field=Tentative.Field;
							Left.this.segment=Tentative.this.segment;
						} else if(desigCmp(Left.desig,Previous.desig)) {
							Left.Field=Previous.Field;
							Left.this.segment=Previous.this.segment;
						} else
							Left.this.segment=assignSegment(Left,Field=TheOpenField,Resolve);

					} else
						Left.this.segment=assignSegment(Left,Field=TheOpenField,Resolve);
					break;
				}
				errorAlert(BaldShot);
				continue;
		}
		
/* 	Now get verb, can be '=', '->', or a ','. ':' can intervene or be alone. */

		clearSpace;
		if(*s==':') {
			s++;
			clearSpace;
		}
		switch (*s) {
			case '.':
				s++;
				_clearSpace;
				if(*s=='.') {	/* Empty field */
					s++;
					Previous.flag=0x400;
					goto decision;
				}
				Previous.flag=0x100;	/* Must be a number. */
				goto decision;
			case '-':			/* a tie */
				s++; 
				_clearSpace;	/* comments not allowed here */
				if (*s == '>') {
					s++;
					Previous.flag=0x300;
					break;
				} else { 
					errorAlert(ArrowHeadExpected); 
					continue; 
				}
			case '=':
				s++;
				Previous.flag=0x200;
				break;
			case ',':
				s++;
				Previous.flag=0x800;
				break;
			default:
				if(isfdigit) {
					Previous.flag=0x100;
					goto decision;
				}
				errorAlert(Syntax);
				continue;
				
		}
	
		/*------------------------------------------------------------------ 	
			End Verb. At this point, we have a link, a tie, a location 
			assignment or a to-from station line.
		
			Get Right. It can be '$','[','"','\'', digit, or alpha. 
		------------------------------------------------------------------*/
		
		clearSpace;
		switch (*s) { 
			case '$':
				s++;
				Right.flag=1;
				parseSegment(&Right);
				Right.this.segment=assignSegment(Right,Field,NoP1Resolve);
				break;
			case '[':		/* Fixed location */
				if(!(dataLocn=parsePlanar(nextFromList(P1LocnList,locnCast)))) 
					continue;
				Right.flag = 7;
				break;
			case '{':		/* Relative locn. This is represented internally as a tie. */
				theLocn=nextFromList(P1LocnList,locnCast);
				if((P1LocnList->current & BlockLo) == BlockLo) 
					/* Don't split the two records of a tie accross a block. */
					theLocn=nextFromList(P1LocnList,locnCast);
				theLocn->code=0;	/* This is the header rec, not yet active. */
				if(!(dataLocn= -parsePlanar(nextFromList(P1LocnList,locnCast)))) 
					continue;
				Right.flag = 7;
				break;
			case '"':
				Right.flag = 8;	/* Comment string assigned to a station. Ignored. */
				break;
			case '\'':			/* Literal station name. */
				Right.flag = 5;
				findLit(&Right);
				break;
			default:
				if (isalpha(*s)) {
					Right.flag = 2;
					parseSegment(&Right);
					if(Tentative.flag == 0x1000) {
						if(desigCmp(Right.desig,Previous.desig)) {
							Right.Field=Previous.Field;
							Right.this.segment=Previous.this.segment;
						} else
							Right.this.segment=assignSegment(Right,Field=TheOpenField,Resolve);

					} else if(Tentative.flag == 0x2000) {
						if(desigCmp(Right.desig,Tentative.desig)) {
							Right.Field=Tentative.Field;
							Right.this.segment=Tentative.this.segment;
						} else if(desigCmp(Right.desig,Previous.desig)) {
							Right.Field=Previous.Field;
							Right.this.segment=Previous.this.segment;
						} else
							Right.this.segment=assignSegment(Right,Field=TheOpenField,Resolve);

					} else
						Right.this.segment=assignSegment(Right,Field=TheOpenField,Resolve);
					break;
				}
				if (isdigit(*s)) {
					Right.flag = 3;
					Right.this.n = getPosNum();
					break;
				}
				errorAlert(Syntax);
				Right.flag=4;		/* blank */
		}
		
		/*	Some special treatment for cases when there is both a from- and a
			to-station on a line. */
			
		if(Previous.flag==0x800) {				/* For now, treat to-froms as ties. */
			if(Left.this.segment!=Right.this.segment || 
					(abs(temp=Right.this.n-Left.this.n)>1 && Right.flag!=5))
				Previous.flag=0x300; 			/* Treat as a tie */
			else
				Right.flag=0;					/* Simplify, as both stations normal */
		} else if(WarningFlag && Previous.flag==0x300
					&& Left.this.segment==Right.this.segment
					&& Right.this.n==Left.this.n+ThisContext->stationIncr)
						errorAlert(TrivialTie);
		
decision:
		State = Previous.flag | (Left.flag<<4) | Right.flag | Tentative.flag;
		
/*------------------ this is the main jump table --------------------------*/
		
		switch(State) {
		
			/*	Data lines:
				Key actions here are:
				Check sequence. If out of order, declare an implicit tie.
					Only data lines create implicit tie. Implicit ties
					do not give rise to a Tentative parseRec (pR).
				As part of above, look for a reverse sequence.
				10 and 20 segments can be continuations of the Tentative pR.
				30 segments get filled with the Previous data and are 
					treated as 10s.
				Data lines always leave a Previous, never a Tentative.
			*/

/*--------------------------------------------------------------------------
	First data line in a segment. There are no Tentative or Previous stations.
	This is one place where the station order form is decided. The same 
	procedure is used for datalines, i-lines. Stations are 10's, 20's,
	30's, or 50's (lits). 
--------------------------------------------------------------------------*/

		/* First, a few simple errors: */
		
			case 0x160:							/* 	. data	*/
			case 0x130:	case 0x430:				/*	10:		*/
			case 0x237:							/*	10: = [5677,45,9]	*/
				errorAlert(BaldShot);
				continue;
				
		/* The data lines, including i-lines (..): */
				
			case 0x410:							/*	$123A10: ..	*/
			case 0x420:							/*	A10: ..	*/
			case 0x450:							/*	'TT8W': ..	*/
				LineOrder=ToVector;
				goto FirstLine;

			case 0x810:	case 0x2810:			/*	$123A10,$123A11: */
			case 0x820:	case 0x2820:			/*	A10:,A11:		*/
			case 0x830:	case 0x2830:			/*	10:,11:			*/
				LineOrder=ToFromVector;
				Tentative=Right;
				ThisContext->stationIncr=temp;
				goto FirstLine1;

			case 0x110: 						/*	$123A10: data	*/
			case 0x120: 						/*	A10: data	*/
			case 0x150: 						/*	'TT8W': data	*/
				LineOrder=FromVector;
FirstLine:		ThisContext->stationIncr=1;	/* default, can be changed later. */
FirstLine1:		Previous=Left;
				Previous.first=Previous.this.n;
				Tentative.flag = 0x1000;
				ThisContext->segment = Previous.this.segment;
				break;
				
				
/*-------------------------------------------------------------------------
	The Thousands in to-from format.
	There are both Previous and Tentative stations. 
-------------------------------------------------------------------------*/

			case 0x1810: case 0x1820:
			case 0x1830:
				if(Left.this.segment!=Tentative.this.segment ||
						temp!=ThisContext->stationIncr) {
					updateMapping(0);		/* break in the data stream */
					LineOrder=ToFromVector;
					ThisContext->stationIncr=temp;
					Tentative=Right;
					goto FirstLine1;
				}
				Previous.this.n+=temp;
				Tentative.this.n+=temp;
				break;

/*-------------------------------------------------------------------------
	The Thousands:
	Data lines in mid-stream (also possible after an implicit tie). There 
	is only a Previous station. 
-------------------------------------------------------------------------*/

			case 0x1130:					/*	10: 	*/
				if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr))  
					break;	/* most common case */
				Previous.this.n-=ThisContext->stationIncr;
				if (abs(temp=Left.this.n-Previous.this.n)>1) {
					Left.Field=Previous.Field;
					desigCpy(Previous.desig,Left.desig);
					Left.this.segment = Previous.this.segment;
					goto implicitTie;
				}
				goto changeBlock;
				
			case 0x1110:					/*	$123A10: 	*/
			case 0x1120:					/*	A10: 		*/
				if(Left.this.segment==Previous.this.segment) {
					if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr))  /* most common case */
						break;
					Previous.this.n-=ThisContext->stationIncr;
					if(abs(temp=Left.this.n-Previous.this.n)<2) {
changeBlock:			ThisContext->stationIncr=temp;
						if(ThisContext->stationIncr==1) {
							Previous.this.n=Left.this.n;
						} else if(ThisContext->stationIncr==-1) {
							Previous.this.n=Left.this.n;
							errorAlert(ReverseBlock);	/* Warning. */
						} else {
							errorAlert(DoubleData);	/* Error. */
							ThisContext->stationIncr=1;
						}
						break;
					}
				}
			case 0x1150:			

implicitTie:	if(WarningFlag) {
					if(Previous.this.segment==Left.this.segment && 
						Previous.this.n+ThisContext->stationIncr-Previous.first==
							Previous.first-Left.this.n)
						errorAlert(BadStationIncr);	/* warning */
					}

				P1LocnList->other++;	/* Counting implicit ties. */
				updateMapping(0);
				
				theLink=nextFromList(P1LList,linkCast);
				theLink->from=Previous.this;
				theLink->to=Left.this;
				/* 	Implicit ties have no shot information--the vector may be on
					the present line or the Previous, depending on short order.
					Wait until p2. */
					
				theLocn=nextFromList(P1LocnList,locnCast);
				if((P1LocnList->current & BlockLo) == BlockLo) 
					/* Don't split the two records of a tie across a block. */
					theLocn=nextFromList(P1LocnList,locnCast);
				theLocn->code=0;	/* This is the header rec, not yet active. */
				theLocn=nextFromList(P1LocnList,locnCast);
				theLocn->code=0;	/* In p2, this will be a move_to to the 
											from.station of the tie. */
				theLink->data = P1LocnList->current;

				/* 	Encode whom the link belongs to. The Length field of the from-
					station is free to use as a flag. Just placing the current
					segment number there. */
					
				if(TheOpenField && Previous.Field==TheOpenField)
					theLocn[-1].length=Previous.this.segment;
				else if(TheOpenField && Left.Field==TheOpenField)
					theLocn[-1].length=Left.this.segment;
				else	/*	Use logic: The data are in the segment with the higher Field. */
					theLocn[-1].length=(Left.Field>Previous.Field)? Left.this.segment:Previous.this.segment;

				Previous=Left;
				Previous.first=Previous.this.n;
				ThisContext->stationIncr=1;
				ThisContext->segment=Previous.this.segment; /* Implicit forces a new segment. */
				break;
	
/*--------------------------------------------------------------------------

	First data line after a link, both Tentative and Previous valid.
		Scope:
			An incomplete reference, as 10: or A10: is checked against the
			current context, i.e. Previous and Tentative.
		Match criteria:
			Although links and ties terminate traverses, data lines do look
			back at the two possibly open stations in order to (1) reset 
			LineOrder and (2) to try resolve any unresolved station name.
			Regarding LineOrder, a preceeding link or tie is like a pre-
			ceeding i-line for establishing to-vector format. All other
			conditions (stations match, or no match at all) are treated
			the same as an 0x100 or 0x400 line.
			
--------------------------------------------------------------------------*/
		 	
			case 0x2110: 				/*$123A10: data... 	*/
				if(Left.this.segment == Tentative.this.segment &&
						abs(ThisContext->stationIncr=(Left.this.n-Tentative.this.n))<=1) {
					Previous=Tentative;
					goto setUp2000;
				}	/* no match. Check Previous. */
				if(Left.this.segment == Previous.this.segment &&
						abs(ThisContext->stationIncr=(Left.this.n-Previous.this.n))<=1)
					goto setUp2000;
				/*	No match anywhere. */
				errorAlert(SequentialTies);
				goto FirstLine;
				
			case 0x2120:				/*	A10: data... 	*/
				if(Left.this.segment == Tentative.this.segment &&
					abs(ThisContext->stationIncr=(Left.this.n-Tentative.this.n))<=1) {
					Previous=Tentative;
					goto setUp2000;
				}
				/* no match. Check Previous. */
				if(Left.this.segment == Previous.this.segment &&
					abs(ThisContext->stationIncr=(Left.this.n-Previous.this.n))<=1) {
					goto setUp2000;
				}
				
				/*	No match anywhere. Now, check for an openField. This is
					done now so as not to mask an immediate context. */

				errorAlert(SequentialTies);
				goto FirstLine;
				
			case 0x2130: 				/*	10: data... 	*/
				if(abs(ThisContext->stationIncr=Left.this.n-Tentative.this.n)<=1) {
					Previous=Tentative;
					goto setUp2000;
				}
				if(abs(ThisContext->stationIncr=Left.this.n-Previous.this.n)<=1) 
					goto setUp2000;
					
				/* No match */
				errorAlert(SequentialTies);
				temp=Left.this.n;
				Left=Previous;
				Left.this.n=temp;
				goto FirstLine;
				
setUp2000:		if(!LineOrder)
					LineOrder = (ThisContext->stationIncr)? ToVector:FromVector;
				Tentative.flag = 0x1000;
				ThisContext->stationIncr=(ThisContext->stationIncr)? ThisContext->stationIncr:1;
				Previous.this.n=Left.this.n;
				/* Data after a tie establishes a new transit. */
				ThisContext->segment = Previous.this.segment;
				break;
					
/*-------------------------------------------------------------------------
	0x1400s: Empty data lines in mid-stream (possible after an implicit 
	tie). This is a normal termination of a FromVector (default) string
	and an error in ToVector. In FromVector, an implicit tie can be made.
--------------------------------------------------------------------------*/

			case 0x1410:				/*	$123A10: ..	*/
			case 0x1420:				/*	A10: ..		*/
				if(LineOrder==ToVector) {
					updateMapping(0);
					goto FirstLine;
				}
				if(Left.this.segment==Previous.this.segment) 
					goto seq400;
				/* no match */
				Tentative.flag=0;
				goto implicitTie;
				
			case 0x1430:				/*	10: ..	*/
				if(LineOrder==ToVector) {
					updateMapping(0);
					Left.Field=Previous.Field;
					desigCpy(Previous.desig,Left.desig);
					Left.this.segment = Previous.this.segment;
					goto FirstLine;
				}
seq400:			if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr)) {
					updateMapping(0);
					Tentative.flag=0;
					break;
				}
				Previous.this.n-=ThisContext->stationIncr;
				Left.Field=Previous.Field;
				desigCpy(Previous.desig,Left.desig);
				Left.this.segment = Previous.this.segment;
			case 0x1450:				/*	'TT3W' ..	*/
				Tentative.flag=0;
				goto implicitTie;

/*-----------------------------------------------------------------------
	Empty data line after a link or tie.  
------------------------------------------------------------------------*/
					
			case 0x2410:				/*	$123A10: ..	*/
			case 0x2420:				/*	A10: ..		*/
				if(Left.this.segment==Tentative.this.segment &&
						Left.this.n==Tentative.this.n) {
					Tentative.flag=0;
					if(DimList)
						parseDims(getDims(P1LocnList->current));
					break;
				}	/* no match. Check Previous. */
				if(Left.this.segment==Previous.this.segment &&
						Left.this.n==Previous.this.n) {
					Tentative.flag=0;
					break;
				}	/* no match at all. */
				LineOrder=ToVector;
				goto FirstLine;
				
			case 0x2430:				/*	10: ..	*/
				if(Left.this.n==Tentative.this.n) {
					Tentative.flag=0;
					if(DimList)
						parseDims(getDims(P1LocnList->current));
					break;
				}
				if (Left.this.n==Previous.this.n) {
					Tentative.flag=0;
					break;
				}
				temp=Left.this.n;
				Left=Previous;
				Left.this.n=temp;
				LineOrder=ToVector;
				goto FirstLine;

/*-------------------------------------------------------------------------
	The friendly fixed locations, treatment is much like the empty data
	lines.
--------------------------------------------------------------------------*/
				
			case 0x1217:				/*	$123A10: = [5677,45,9] */
				if(Left.this.segment==Previous.this.segment)
					goto seq1217;
				/* no match */
			case 0x1257:				/*	'TT28AZ' = [5677,45,9] */
				p1Planar(dataLocn);
				goto implicitTie;
				
			case 0x1227:				/*	A10: = [5677,45,9] */
				if(Left.this.segment==Previous.this.segment) {
					p1Planar(dataLocn);
					goto implicitTie;
				}
			case 0x1237:				/*	10: = [5677,45,9] */
seq1217:		if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr)) {
					updateMapping(0);
					Left=Previous;
					p1Planar(dataLocn);
					break;
				}
				Previous.this.n-=ThisContext->stationIncr;
				temp=Left.this.n;
				Left=Previous;			/* make sure Left info all there, */
				Left.this.n=temp;		/* preserving Left.this.n. */
				Tentative.flag=0;
				if(Left.this.n!=Previous.this.n) {
					p1Planar(dataLocn);
					goto implicitTie;
				}
			case 0x1267: 				/*	. = [56,78,90] */
				updateMapping(0);
				p1Planar(dataLocn);
				break;
				
/*---------------------------------- Links: -----------------------------------------
				Links are aliases: two stations having the same name.
				They are not too common, so the code can be written to be compact.
				The key actions are:
					Update (close) the Previous segment.
					There is not checking for sequential stations as no implicit
						ties are made to links (or to ties). 
					Create new Previous and Tentative pRs, with all fields filled:
						->this.segment
						->Field
						->desig
						->start
					
------------------------------------------------------------------------------------ */
										
			case 0x1262:				/*	. = B14 	*/
			case 0x1265: 				/*	. = 'TT8W' 	*/
			case 0x1261: 				/*	. = $456B14 */
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotLink;
				
			case 0x1222:				/*	A10 = B14 	*/
			case 0x1252:				/*	'TT8W' = B14 	*/
			case 0x1225:				/*	B14 = 'TT8W' 	*/
			case 0x1221: 				/*	A10 = $456B14 	*/
			case 0x1255:				/*	'TT8W' = 'Nail' 	*/
			case 0x1211: 				/*	$123A10 = $456B14 	*/
			case 0x1212:				/*	$123A10 = B14 		*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);

			case 0x222: case 0x2222:	/*	A10 = B14 		*/
			case 0x252: case 0x2252:	/*	'TT8W' = B14 	*/
			case 0x212: case 0x2212:	/*	$123A10 = B14 	*/
			case 0x225: case 0x2225:	/*	B14 = 'TT8W' 	*/
			case 0x221: case 0x2221:	/*	A10 = $456B14 	*/
			case 0x255: case 0x2255:	/*	$123A10 = $456B14 */
			case 0x211:	case 0x2211:	/*	$123A10 = $456B14 */
				Previous=Left;
				Previous.first=Previous.this.n;
				
			case 0x2262: case 0x262:	/*	. = B14 	*/
			case 0x265:	case 0x2265: 	/*	. = 'TT8W' 	*/
			case 0x261:	case 0x2261:	/*	. = $456B14 */
makeDotLink:	Tentative=Right;
				Tentative.flag = 0x2000;
				Tentative.first=Tentative.this.n;
				theLink=nextFromList(P1LList,linkCast);
				theLink->from  = Left.this;
				theLink->to = Right.this;
				theLink->data=0;
				break;
				
/*-------------------------------- Ties: ------------------------------------
				Ties are not too common, so the code is written to be compact.
				The key actions are:
					Update (close) the Previous segment.
					There is not checking for sequential stations as no implicit
						ties are made to ties (or to links). 
					Create new Previous and Tentative pRs, with all fields filled:
						->this.segment
						->Field
						->desig
						->start
					Unresolved Segments can be made Tentative and Previous.
					The polar data is parsed at this time.
				Note that dots already have Left substituted so 6x is like 1x.
------------------------------------------------------------------------------*/

			case 0x1362: 				/*	. -> B14	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotTie;

			case 0x362:	case 0x2362:
				Left.this.n += (LineOrder==ToVector)? 0:ThisContext->stationIncr;
				goto makeDotTie;
				
			case 0x1363: 				/*	. -> 6:	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x2363:				/*	. -> 6:	*/
				Left=Previous;
				temp=Right.this.n;
				Right=Previous;
				Right.this.n=temp;
				goto makeDotTie;

			case 0x1365: case 0x2365:	/*	. -> 'TT8W':	*/
			case 0x1361: case 0x2361:	/*	. -> $456B14:	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotTie;

			case 0x1353:				/*	'TT8W' -> 14:	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x353: case 0x2353:	/*	'TT8W' -> 14:	*/
				temp=Right.this.n;
				Right=Previous;			/*	not Right=Left
				Right.this.n=temp;	
				goto makeTie;

			case 0x1313:				/*	$123A10 -> 14:	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x313:	case 0x2313:	/*	$123A10 -> 14:	*/
				temp=Right.this.n;
				Right=Left;
				Right.this.n=temp;	
				goto makeTie;
				
			case 0x1322: 				/*	A10 -> B14	*/
			case 0x1352: 				/*	'TT8W' -> B14	*/
			case 0x1312: 				/*	$123A10 -> B14	*/
			case 0x1325: 				/*	B14 -> 'TT8W'	*/
			case 0x1321: 				/*	A10 -> $456B14	*/
			case 0x1351: 				/*	'TT8W' -> $456B14	*/
			case 0x1315: 				/*	$456B14 -> 'TT8W'	*/
			case 0x1311: 				/*	$123A10 -> $456B14:	*/
				updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
				
			case 0x322:	case 0x2322:	/*	A10 -> B14	*/
			case 0x352:	case 0x2352:	/*	'TT8W' -> B14	*/
			case 0x312:	case 0x2312:	/*	$123A10 -> B14	*/
			case 0x325:	case 0x2325:	/*	B14 -> 'TT8W'	*/
			case 0x321:	case 0x2321:	/*	A10 -> $456B14	*/
			case 0x351:	case 0x2351:	/*	'TT8W' -> $456B14	*/
			case 0x315:	case 0x2315:	/*	$456B14 -> 'TT8W'	*/
			case 0x311:	case 0x2311:	/*	$123A10 -> $456B14	*/
			case 0x355: case 0x2355:		
			case 0x1355:
makeTie:		Previous=Left;
				Previous.first=Previous.this.n;
makeDotTie:		Tentative=Right;
				Tentative.first=Tentative.this.n;
				Tentative.flag = 0x2000;
				theLink=nextFromList(P1LList,linkCast);
				theLink->from = Left.this;
				theLink->to = Right.this;
				clearSpace;
				if(*s==':') {
					s++;
					clearSpace;
				}
				theLocn=nextFromList(P1LocnList,locnCast);
				if((P1LocnList->current & BlockLo) == BlockLo) 
					/* Don't split the two records of a tie accross a block. */
					theLocn=nextFromList(P1LocnList,locnCast);
				theLocn->code=0;	/* This is the header rec, not yet active. */
				
				/* 	Encode to whom the link belongs. The Length field of the from-
					station is free to use as a flag. */
				/* Changed 8/92 as ThisContext->segment is often not up to date here. */
				if(TheOpenField && Left.Field==TheOpenField)	
					theLocn->length=theLink->from.segment;
				else if(TheOpenField && Right.Field==TheOpenField)
					theLocn->length=theLink->to.segment;
				else 	/*	Use logic: The data are in the segment with the higher Field. */
					theLocn->length=(Right.Field>Left.Field)? theLink->to.segment:theLink->from.segment;
					
				parsePolarData(nextFromList(P1LocnList,locnCast));
				theLink->data = P1LocnList->current;
				if(FullListing) {
					if(!LineList) 
						LineList=lListAlloc(2,MedBlockLen,sizeof(struct lineRec),1+(LineCnt/MedBlockLen),nLineList);
					LineList->current=LineCnt-ContinuationLines;
					theLine=(struct lineRec *)getLListBuf(LineList->current,LineList)+(LineList->current&MedBlockLo);
					theLine->data=theLink->data;
					theLine->sta=Previous.this;
					theLine->state=State;
					theLine->fields=ThisContext->fields;
				}
				break;
				
/*------------------------- Fixed Locations ----------------------------

	The link record and locn data have already been made. The job here 
	is station sequence keeping. Fixed locations are friendly, like
	i-lines (i.e. A10: ..), which means that they can be part of an
	implicit tie.
	
-----------------------------------------------------------------------*/

			case 0x227: 				/*	A10 = [56,78,90] */
			case 0x217:					/*	$123A10 = [56,78,90] */
			case 0x257:					/*	'TT28AZ' = [5677,45,9] */
				Previous=Left;
				Previous.first=Previous.this.n;
				Tentative.flag = 0x1000;
				if(!ThisContext->segment)
					ThisContext->segment = Previous.this.segment;
				LineOrder=ToVector; 		/*======== Attention =========*/
				ThisContext->stationIncr=1;	/* default, can be changed later. */
				p1Planar(dataLocn);
				break;

			case 0x2227: 				/*	A10 = [56,78,90] */
			case 0x2217: 				/*	$123A10 = [56,78,90] */
			case 0x2257:				/*	'TT28AZ' = [5677,45,9] */
				Previous = Left;
				p1Planar(dataLocn);
				break;

			case 0x267:					/*	. = [56,78,90] */
				if(!Previous.this.segment) {
					errorAlert(BaldShot);
					break;
				}
			case 0x2267:				/*	. = [56,78,90] */
				Previous.first=Previous.this.n;
				p1Planar(dataLocn);
				break;
				
			default:
				fprintf(ErrFile,"State %x not handled\n",State);
				errorAlert(Syntax);
		}	/* End of switch. */	

	} 	/* End of p1 loop. */
	
	if(TheOpenField)
		closeField();
	else if(Tentative.flag==0x1000) 
		updateMapping((LineOrder==FromVector)?ThisContext->stationIncr:0);
	
	if(theFile)
		fclose(theFile);
	if(ThisContext) free(ThisContext);
	ThisContext=then;	/* restore calling context */
	return 0;
}

/*---------------------------------------------------------------------------------

					Pass 1.5

---------------------------------------------------------------------------------*/

static int lltcompare(void *a, void *b)
	/* Segregates the 3 types, ordering them Locn, Links, Ties within
		each segment. */
{
	int n;
	if(n=((LinkRecPtr)a)->from.segment - ((LinkRecPtr)b)->from.segment)	/* most common case */
		return n;
	if(((LinkRecPtr)a)->to.segment && ((LinkRecPtr)b)->to.segment) {	/* neither is a locn */
		if(!((LinkRecPtr)a)->data == !((LinkRecPtr)b)->data)		/* both same type */
			return ((LinkRecPtr)a)->from.n - ((LinkRecPtr)b)->from.n ;
		else		/* one is link, one is tie */
			return (((LinkRecPtr)a)->data)? 1 : -1 ;
	}
	/* comes here if one is a locn, which gets floated to top */
	if(!((LinkRecPtr)a)->to.segment == !((LinkRecPtr)b)->to.segment)	/* both locns */
		return ((LinkRecPtr)a)->from.n - ((LinkRecPtr)b)->from.n ;
	return (((LinkRecPtr)a)->to.segment)? 1 : -1 ;
}

static int linkcompare(void *a, void *b)
	/*	Sorts by from segment and station, only. */
{
	int n;
	if(n=((LinkRecPtr)a)->from.segment - ((LinkRecPtr)b)->from.segment)	/* most common case */
		return n;
	if(n=((LinkRecPtr)a)->from.n - ((LinkRecPtr)b)->from.n)
		return n;
	return ((LinkRecPtr)a)->to.segment - ((LinkRecPtr)b)->to.segment;
}

static int mrcompare(void *a,void *b)
{
	int n;
	if(n=((MapRecPtr)a)->segment - ((MapRecPtr)b)->segment)
		return n;
	if(n=((MapRecPtr)a)->first - ((MapRecPtr)b)->first)
		return n;
	return ((MapRecPtr)a)->last - ((MapRecPtr)b)->last ;
}

void doP1half(void)
{
		
	long dataCounter;
	indexType linkCounter;
	int (*nodeTable)[2],(*node)[2],nodeTableSize=BlockLen;
	int i,n,m,contig,fidCount=0;
	struct segmentRec *theSegment;
	LinkRecPtr theLink,linkSortBuff,li;
	struct locnRec *theLocn;
	struct subSegRec *theSubSeg; 
	MapRecPtr mapSortBuff, mapSPtr;
	struct strRec *theString;

	printSummaries(1);

/*---------------------- Take care of links ------------------------*/

	if(P1LList->other == 0) {	/* No fixed locations. We have to make one. */
		theLink=nextFromList(P1LList,linkCast);
		theLink->from  = FirstAged;
		theLink->to.segment =  0;
		theLink->to.n =  0;
		theLocn=nextFromList(P1LocnList,locnCast);
		theLocn->code = 0;
		theLocn->data.cart.x = theLocn->data.cart.y = theLocn->data.cart.z = 0;
		theLink->data=P1LocnList->current;
		P1LList->other = 1;
		SegmentWithFirstLocn = theLink->from.segment;
	}
	
	storeList(MapList);

	/*	Sort Sections by tissue plane. */
#ifndef CMLmake
	sortSections();
#endif
	
	storeList(P1LocnList);	/* Short List */
	storeList(P1LList);		/* Short List */

	n=2*(P1LList->current - P1LList->other)+P1LList->other;
		/*
			->other has been used to count Locn's. Links and Tie get
			duplicated, so that each of their segments has a record.
		*/
		
	if(LineList) {
		freeLL(LineList);
		LineList->other=LineList->current;
		LineList->current=0;
	}
	
		
	LocnList=lListAlloc(0,BlockLen,sizeof(struct locnRec),(P1LocnList->current>>BlockHi)+1,nLocnList);
	LocnList->current=P1LocnList->current;
	LocnList->other=P1LocnList->other;	/* Number of implicit ties. */
	for(i=0;i<LocnList->nLE;i++) 
		LocnList->list[i].position=P1LocnList->position[i];
	free(P1LocnList->buffer);
	free(P1LocnList->position);
	free(P1LocnList);
	
	LinkList = lListAlloc((int) 0,BlockLen,sizeof(LinkRec),(n>>BlockHi)+1,nLinkList);
		/*	
			Allocate an unitialized lList for the Link Records. This will be mapped
			to the linkSortBuff, with the tail of the data moved to the old
			P1LList->buffer. 
		*/
	StringList=lListAlloc(2,BlockLen,sizeof(struct strRec),(MapList->current/BlockLen)+1,nStringList);
	SubSegList=lListAlloc(1,BlockLen,sizeof(struct subSegRec),(SegList->current/BlockLen)+1,nSubSegList);
	linkSortBuff=emalloc((n+1)*sizeof(LinkRec)); 
	
	LinkList->current=n;		/* 	Store this now, even though the LinkList is still
								inactive */
	/*	This is the way to retrieve information from an sList. */
	tempRead(P1LList->buffer,P1LList->position[0],P1LList->size);
	theLink=linkSortBuff+1;		/*19-sep-90 and 20-nov-90, fixed sort bufs */

	for(i=1;i<=P1LList->current;i++) {
		if (!(i&BlockLo)) 
			tempRead(P1LList->buffer,P1LList->position[i>>BlockHi],P1LList->size);
		*theLink = *((linkCast P1LList->buffer)+(i&BlockLo));
		if(theLink->to.segment) {	/* Link or Tie: make reversed copy. */
			theLink++;
			/*	Here, we define that link to.segments not sign reversed--an archane,
				but historically important, point whose context is all but forgotten. */
			theLink->to.n = theLink[-1].from.n;
			theLink->from=theLink[-1].to;
			theLink->data=theLink[-1].data;
			theLink->to.segment = (theLink->data)? 
				-theLink[-1].from.segment
				: theLink[-1].from.segment;
		}
		theLink++;
	}

	qsort((void *) (linkSortBuff+1),(size_t) LinkList->current,sizeof(LinkRec),linkcompare);
		/* 	
		N.B. This is the first sort of the links. It orders them by segment and
		station, so as to form an ordered list of string ends.
		There will be a later sort, so that Locn's, Links, and Ties will be 
		segregated within each segment. Note that the first element, which is 
		a dummy record so that no link has an index of 0 (like segments), is 
		excluded from the sort. 
		*/


/*---------------- Prepare links for p2 -----------------*/

/*---------------- Prepare allocation maps ------------------
	
		Two data structures result:
			1. 	Strings are a map of all segment strings.
			2.	SubSegments map large gaps in station sequence.
			
------------------------------------------------------------*/
		
	mapSortBuff=emalloc((MapList->current+BlockLen)*sizeof(MapRec));
		/* 
		N.B. List->current is index of the currently valid member of the list.
		the number of elements in the list is ->current+1. So the following
		test clause. Watch it. */
		
	for(i=0;i<=MapList->current;i+=BlockLen)
		tempRead(mapSortBuff+i,MapList->position[i>>BlockHi],MapList->size);

	mapSPtr=mapSortBuff+1;	/* 0th record is garbage. */
	qsort((void *) mapSPtr,(size_t) MapList->current,sizeof(MapRec),mrcompare);
	mapSPtr[MapList->current].segment=SegList->current+1;	/* a tripwire */
		
	
	freeSL(MapList);
	free(MapList->position);
	free(MapList);
	
	FixArray=ecalloc(SegList->current+1,sizeof(short));
	nodeTable=emalloc(2*BlockLen*sizeof(int));

/*----------------------------------------------------------------------
	Everything is now in place to prepare the p2 data structures.
		The linkSortBuff is still memory resident.
		Segment records are in an lList.
		MapRecs, soon to be disposed of, are memory resident and sorted
			by segment and station.
		From all of this a plan for any conceivable future memory allocation 
			for data can be generated--stored in sRs and subSRs.
		StringRecs, which describe all segment strings, are also generated
			now. Strings are contiguous series of segment shots. Their
			ends are passage junctions, passage ends, or fixed locations.

------------------------------------------------------------------------*/

	
	theLink=linkSortBuff+1;
	SegList->other=dataCounter=0;
	linkCounter=1;
	
/*----------------------------------------------------------------------------- 
	For Each Segment:
		A single loop takes us all the way through the sRs.
------------------------------------------------------------------------------*/

	for(i=1,**nodeTable=0,node=nodeTable;i<=SegList->current;i++) {
		RotateCursor(i);
		theSegment = getSR(i);
			
		if(mapSPtr->segment!=i) {
			theSubSeg=NULL;
			theSegment->nStrings=0;
			theSegment->strings=0;
			theSegment->data=0;
			theSegment->flags|=UnMapped;
		} else
			theSubSeg = (struct subSegRec *)theSegment;

/*-------- Loop 1: Build node table and sift through sorted locn records. --------*/
		
		while(theLink->from.segment<i) {
			theLink++;
			linkCounter++;
		}
						
		if(theLink->from.segment != i) {
			_errorAlert(FloatingSegment,i,-1);
			*nodeTable[0]=-1; /* -1 always trips out any node-dependant actions */
		} else {
			*nodeTable[0]=theLink->from.n;
			nodeTable[0][1]=0;
			/*------ Pointer compare, though a pretty safe one. --------*/
/*			theSegment->links=theLink-linkSortBuff; */
			
			theSegment->links=linkCounter;

			for(n=0;theLink->from.segment==i;theLink++,linkCounter++) {
				if(*nodeTable[n]!=theLink->from.n) {
					if(++n==nodeTableSize) {
						nodeTable=erealloc(nodeTable,2*(n+BlockLen)*sizeof(int));
						nodeTableSize+=BlockLen;
					}
					*nodeTable[n]=theLink->from.n;
					nodeTable[n][1]=1;
				} else {
					if(theLink->to.n==(theLink-1)->to.n) {
						/* possible error */
						if((abs(theLink->to.segment)==abs((theLink-1)->to.segment)) &&
							(theLink->from.segment==(theLink-1)->from.segment)) 
							_errorAlert(DuplicateTies,i,theLink->from.n);
					}
					nodeTable[n][1]++; /* Multilinks count. */
				}
				if(theLink->to.segment) {	/* link or tie */
					if(theLink->data) {
						theSegment->nTies++;
						if(theLink->to.segment>0)
							theSegment->ownTies++;
					} else
						theSegment->nLinks++;
				} else {
					theSegment->nLocns++;
					FixArray[i]=-theSegment->links;
					theLink->to.n=theLink->from.n;	/* Kluge */
					nodeTable[n][1]++;		/*	CMAP device to assure that this node
												gets into the closure matrix. */
				}
			}
			*nodeTable[++n] = -1;
		}

		if(!theSubSeg) {	
			/* i.e., an unmapped segment, no contiguous data at all. 
				This is not so unusual, arising, for instance, when one
				or more segments are tied to a station that is from a
				discarded segment or a reference point that is not a
				segment at all. */
			for(n=0,li=linkSortBuff+theSegment->links
				,theSegment->strings=StringList->current+1
				;*nodeTable[n]>=0
				;n++) {
				theString=(stringCast nextFromLList(StringList))+(StringList->current&BlockLo);	
				theSegment->nStrings++;
				StringList->other++;
				theString->string=StringList->current;
				theString->length=theString->nLast=0;
				theString->last=-1;
				theString->first=*nodeTable[n];
				theString->nFirst=nodeTable[n][1]-1;
				theString->code=UnMapped;
				while((li->from.segment==i)&&(li->from.n<*nodeTable[n])) li++;
				while(li->from.n==*nodeTable[n]) {
					if(li->data && li->to.segment)
						theString->code|=HasTie;
					else if(li->to.segment)
						theString->code|=HasLink;
					else
						theString->code|=HasLocn;
					li++;
				} 
			}
			continue;
		}

/*-------------- Loop 2: Build stringRecs from mapRecs. --------------*/
		
		for(n=0,node=nodeTable,theSegment->nStrings=0,theString=NULL,
					theSegment->strings=StringList->current+1,
					li=linkSortBuff+theSegment->links
				;mapSPtr[n].segment==i
				;n++) {		/* n is now index for mapSPtr[]. */
			contig=0;

			/*---------------------------------------------------------
				Are there nodes not pointing to strings (i.e. with a 
				station number that is not a mapRec->first)?
								
				The specifics are that, for some reason, one or more 
				station numbers in a sequence have been skipped. In 
				addition, a tie could be being made into that gap. Both 
				conditions have to be tested for. 
			---------------------------------------------------------- */
			if(**node<mapSPtr[n].first) {
			/*	if((**node>=0)&&(n>0)) node++; */
				if(theString && (theString->last==**node)) node++;
				for(;(**node>=0) && (**node<mapSPtr[n].first);node++) {
					theSegment->flags |= UnMapped;
					theString=(stringCast nextFromLList(StringList))+(StringList->current&BlockLo);	
					theSegment->nStrings++;
					StringList->other++;
					theString->string=StringList->current;
					theString->length=theString->nLast=0;
					theString->last=-1;
					theString->first=**node;
					theString->nFirst=*(*node+1)-1;
					theString->code=UnMapped;
					while((li->from.segment==i)&&(li->from.n<**node)) li++;
					while(li->from.n==**node) {
						if(li->data && li->to.segment)
							theString->code|=HasTie;
						else if(li->to.segment)
							theString->code|=HasLink;
						else
							theString->code|=HasLocn;
						li++;
					}
					theString=NULL;		/* So as not to interfer with real traverses. */
				}
			}
						
			/* Are there loose ends regarding the last string?
			
					1. Is the mapRec totally enclosed by Previous one(s)?
						This comes from double representation of data. */
						
			if(theString) {
				if(theString->last>=mapSPtr[n].last) 
					continue;	/* Skip this mapRec. */

				/*	2. Is there overlap with the next mapRec or 
					3. Contiguous strings? */
				
				if(theString->last>=mapSPtr[n].first) {
					if(theString->nLast) {	/* Node at End */
						theString->code|=ContigAtEnd;
						theString->nLast++;		/* for the contiguous shot */
						mapSPtr[n].first=theString->last;
						contig=ContigAtStart;	
						/* Note that node is still set to the contiguous station, 
							so that the next string will be properly handled. */ 
					} else {	/*	Not terminated. This comes from premature 
									updateMapping. Extend string. */
						theString->last=mapSPtr[n].last;
						goto Job3;
					}
				}
			}
			
			/* Now start: New strings made either here or by splitting. */
			theString=(stringCast nextFromLList(StringList))+(StringList->current&BlockLo);	
			theSegment->nStrings++;
			theString->string=StringList->current;
			if(theString->code=contig) {
				theString->nFirst=1;
				NumStrJct += 2;
			} else {
				theString->nFirst=0;
				NumStrJct++;
			}
			theString->first=mapSPtr[n].first;
			theString->last=mapSPtr[n].last;

	 		/* Second job for the new string: Get the corresponding start node. */
			if(**node == theString->first) {
				theString->code |= NodeAtStart;
				theString->nFirst += *(*node+1);
				node++;
			}
			/* Third job: see if the string(s) are too long, according to the nodes. */
Job3:		while(theString->first<**node && **node<theString->last) {
				/* Yes, split the string(s) */
				theString->code |= (NodeAtEnd|ContigAtEnd);
				theString->last=**node;
				theString->nLast=(*(*node+1))+1;	/* +1 as Contig. Ditto below. */
				theString= stringCast nextFromLList(StringList)+(StringList->current & BlockLo);
				theString->string=StringList->current;
				theString->nFirst=(*(*node+1))+1;
				theString->code=(NodeAtStart|ContigAtStart);
				theString->first = **node++;	/* N.B. node bumped. */
				theString->last=mapSPtr[n].last;
				theSegment->nStrings++;
				NumStrJct++;
			}
			/* Fourth job: add information about end of string, terminating it. */
			if(**node==theString->last) {
				theString->code |= NodeAtEnd;
				theString->nLast = *(*node+1);
			}
		}
		
		/* Have looped through all the mapRecs in this segment. Are
			there nodes Left? */
		if(theString && (theString->last==**node)) node++;
		for(;**node>=0;node++) {
			theSegment->flags |= UnMapped;
			theString=(stringCast nextFromLList(StringList))+(StringList->current&BlockLo);	
			theSegment->nStrings++;
			StringList->other++;
			theString->string=StringList->current;
			theString->length=theString->nLast=0;
			theString->last=-1;
			theString->first=**node;
			theString->nFirst=*(*node+1)-1;
			theString->code=UnMapped;
			while((li->from.segment==i)&&(li->from.n<**node)) li++;
			while(li->from.n==**node) {
				if(li->data && li->to.segment)
					theString->code|=HasTie;
				else if(li->to.segment)
					theString->code|=HasLink;
				else
					theString->code|=HasLocn;
				li++;
			} 
			theString=NULL;
		}
		
/*------------- Loop 3: Build subSegments from sorted mapRecs. -----------*/
		
		theSubSeg->nextSubSeg=0;
		theSubSeg->data=(dataCounter+=DataHeaderSize);
		theSubSeg->first=mapSPtr->first;
		theSubSeg->last=mapSPtr->last;
		theSegment->nStations=theSegment->last-theSegment->first+1;
		dataCounter+=theSegment->nStations;
		while((++mapSPtr)->segment==i) {
			if(theSubSeg->last>=mapSPtr->last)
				continue;
			m=mapSPtr->first-theSubSeg->last;
			n=mapSPtr->last-mapSPtr->first;
			if(m<GapThreshold) {
				if(m) {	/* There is a gap */
					theSubSeg->flags |= Gap;
					theSegment->nStations++;
				}
				dataCounter+=(m+n);	/* Total increment is m-1+n+1 */
			} else {	/* make a new subSegment */
				theSubSeg->nextSubSeg=SubSegList->current+1;	/* linked list */
				/* Now check if the data will cross a data block boundary. */
				if((theSubSeg->data-DataHeaderSize)>>BigBlockHi != dataCounter>>BigBlockHi) {
					m=dataCounter-theSubSeg->data+DataHeaderSize;	/* total space */
					theSubSeg->data=(((theSubSeg->data>>BigBlockHi)+1)<<BigBlockHi)+DataHeaderSize;
					dataCounter=theSubSeg->data+m;
				}
				theSubSeg = subSegCast nextFromLList(SubSegList)+(SubSegList->current & BlockLo);
				theSubSeg->nextSubSeg=0;
				theSubSeg->flags=0;
				theSubSeg->data=(dataCounter+=DataHeaderSize);
				theSubSeg->first=mapSPtr->first;
				dataCounter+=(++n);
			}
			theSubSeg->last=mapSPtr->last;
			theSegment->nStations+=n;
		}
		if((theSubSeg->data-DataHeaderSize)>>BigBlockHi != dataCounter>>BigBlockHi) {
			m=dataCounter-theSubSeg->data+DataHeaderSize;	/* total space */
			theSubSeg->data=(((theSubSeg->data>>BigBlockHi)+1)<<BigBlockHi)+DataHeaderSize;
			dataCounter=theSubSeg->data+m;
		}
		/* 
			This, and the instance above, guarantee that the data space for
			any subSegment does not cross a block boundary. This simple check
			allows simple incrementing, without checking for overflow, of 
			pointers into the data space during p2.
		*/
	}
	
	free((void *)nodeTable);
	free((void *)mapSortBuff);				/* bye */
	
	/*---------------- Process the fiduciary surveys -----------------*/

#ifndef CMLmake
	if(UsingFids)
		processFids(linkSortBuff);
#endif

	/*-----------------------------------------------------------------------
		 Now set up p2LinkList from the linkSortBuff. This will have 
		 converted a sList into an lList.
	-----------------------------------------------------------------------*/
	qsort((void *) (linkSortBuff+1),(size_t) LinkList->current,sizeof(LinkRec),lltcompare);

	
	/* Null theLink->from.segment, which is used as a fixed flag in p2. */
	for(i=1;i<=LinkList->current;i++) 
		linkSortBuff[i].from.segment = 0;
		
	/*----------------- Move it to the LinkList buffers. ---------------------
	
		This would work for a generic SListToLList routine. Note, however, that
		handles are not created. If they are needed for export of data, then
		they will have to be created at export time.
		
	------------------------------------------------------------------------*/
	for(i=0;i<LinkList->nLE-1;i++)
		LinkList->list[i].ptr=linkSortBuff+(i*BlockLen);
	LinkList->list[i].ptr=P1LList->buffer;
	memcpy(P1LList->buffer,linkSortBuff+(i*BlockLen),
			(LinkList->nItems)*(LinkList->size/LinkList->nItems));

	flushLL(LinkList); 
	free(P1LList->buffer);
	free(P1LList->position);		/* bye */
	free(P1LList);

	free(linkSortBuff);	/* bye */
	markDeadEnds();
/*==============================*/
	if(DoP2) 
		initializeNetwork();
		
	DataList=lListAlloc(1,BigBlockLen,sizeof(struct locnRec),
		(dataCounter/BigBlockLen)+1,nDataList);
	DataList->current=dataCounter;
		
	printSummaries(2);

	return ;
}	/* end P1half */

int tryToResolve(struct parseRec *parsed,struct parseRec *active)
{
	struct unResSegRec *urs;
	struct segmentRec *sr;
	if(active->Field==0 && parsed->Field>0) {
		/* Resolve Right now. */
		sr=getSR(active->this.segment);
		urs=getURS(-(sr->Field-1));
		if(urs->resSegment<0) {	/* 0 flags a forced URS. */
			sr->Field=active->Field=parsed->Field;
			addToFieldList(sr);
			urs->resSegment=active->this.segment;
			ResolvedSegments++;
		}
	}
	return(active->Field==parsed->Field);
}

void p1Planar(int dl)
{
	LinkRecPtr li;
	li=nextFromList(P1LList,linkCast);
	if(dl>0) {	/* = [23,44,5] */
		li->from = Left.this;
		li->to.segment = 0;
		li->to.n = 0;
		li->data = dl;
		P1LList->other++;	/* counting these */
		if(!SegmentWithFirstLocn)
			SegmentWithFirstLocn=Previous.this.segment;
	} else {	/* = {23,44,5} */
		if(!ThisContext->Field.localDatum.segment)
			errorAlert(NoLocalDatum);
		li->to = Left.this;
		li->from = ThisContext->Field.localDatum;
		li->data = -dl;
	}
	if(!(State & 0x3000)) 
		ThisContext->segment = Previous.this.segment;
	return;
}
