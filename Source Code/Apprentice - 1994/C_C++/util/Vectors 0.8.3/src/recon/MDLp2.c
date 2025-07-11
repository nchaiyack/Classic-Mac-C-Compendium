/*--------------------------- Pass 2 ------------------------------------------

	Since strings and subSegments contain contiguous shots, once in
	one, data can be filled by just incrementing the data pointer.
	The update segment that ended a string in p1 will do so in p2
	as well. (Not quite, so need to count when keeping track of 
	Length. This is getting to be a big overhead. Maybe split strings
	should get special handling. Most strings will map onto p1
	mapRecs pretty closely.
	
	Or, the order that mapRecs were created is the same in p1 and p2.
	If serial order is remembered, then an array of lengths would be
	enough, they could be matched up at the end of p2.
	
-------------------------------------------------------------------------------*/

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
	#pragma segment pass2
#else
	#include <search.h>
#endif
#ifdef MPW
	#include <CursorCtl.h>
#endif


/*	functions in MDLp2.c	*/
int p2aFile(char *theFileName);
int unadjustedFix(void);

#if 0
	int getUnresolvedSegment(struct parseRec *pR, struct segmentRec **sur);
#endif

int p2aFile(char *theFileName)
{
	FILE *theFile;
	int temp,n;
	int thisStation;
	int reverse=FALSE;
	char *p;
	struct context *then;
	LinkRecPtr theLink;
	struct locnRec *theData,*theLocn;
	struct segmentRec *theSegment;
	struct lineRec *theLine;
	struct dimData *theDims;
	int lastDataLineIndex=0;
		
	if(!(theFile=getFile(theFileName,InBuf,NChars))) {
		errorAlert(FileInErr);
		return (1); 				/* Fatal Err, no input file, so quit */
	}

	if(!Silent)
		fprintf(OutFile,"; Processing %s in Pass 2\n",theFileName);

	Length=0;
	ShotFlag=Line_to_rel;
	LineOrder=0;
	then=ThisContext;
	ThisContext=newContext(2,theFileName,theFile);
	
	Previous.flag=Tentative.flag=Left.flag=Right.flag=0;
	ThisContext->s0=Scratch2;
	*(ThisContext->s0)=0;
	ThisContext->lastLine=Scratch;
	TheOpenField=0;
	
	EEFlag=(EEFlag>0)? 1 : (EEFlag<0)? -1 : 0 ;

	if(FullListing) {
		if(LineCnt)
			theLine=(struct lineRec *)getLListBuf(LineCnt,LineList)+(LineCnt&MedBlockLo);
		else {
			theLine=(struct lineRec *)nextFromLList(LineList);
			LineList->current--;
		}
	}
	
	
	/*-----------------------------------------------------------------
		Main loop of p2aFile follows
			Data are placed:
					0x1000 data lines.
			Data runs are started:
					0x0000 data lines,
					0x2000 data lines.
			Implicit ties treated now.
			
		Parsing is as in Pass 1.
	-----------------------------------------------------------------*/
	
	while ((s=getLine(ThisContext->lastLine,MaxInLine,theFile)) != NULL) {
		ThisContext->lastLine=ThisContext->s0;
		ThisContext->s0=s;
		LineCnt++;
/*		ContinuationLines=0;  */
		
		RotateCursor(LineCnt);
#if 0		
		if(NoProcessFlag) {
			clearSpace;
			if(*s=='#') {
				s++;
				processDirective(4);
			}
			State=3;
			goto p2Bottom;
		}
#endif
		Left.flag=Right.flag=Previous.flag=0;
		clearSpace;
		switch (*s) {	
			case '#':
				s++;
				temp=processDirective(2);
				State=1;
				/* if skippedBlock, need to register this copy of theLine */
				if(LineList) {
					if(LineCnt>1) {
						LineList->current=LineCnt-1;
						theLine=(struct lineRec *)getLListBuf(LineList->current,LineList)+((LineList->current)&MedBlockLo);
					} else {
						LineList->current=1;
						theLine=(struct lineRec *)getLListBuf(1,LineList)+1;
						theLine--;
						LineList->current=0;
					}
				}
				if(temp)
					goto p2Bottom;
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
				State=2;
				goto p2Bottom;
			case '+':
				if(Tentative.flag & 0x3000)  
					averageDataLine(theData);
				State=2;
				goto p2Bottom;
			case '\'':		/* a literal station name */
				Left.flag=5;
				lookUpLit(&Left);
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
						break;
					}
doLength:				
					if(!(Tentative.flag & 0x3000))  
						goto p2Bottom;
					s=p;
					Previous.this.n+=ThisContext->stationIncr;
					Tentative.flag=0x1000;
	/*------------------------- place data ---------------------------*/
					Length+=parsePolarData(theData+=ThisContext->stationIncr);
					if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
						/* Data must be reversed in a reverse block. */
						theData->data.cart.x = -theData->data.cart.x;
						theData->data.cart.y = -theData->data.cart.y;
						theData->data.cart.z = -theData->data.cart.z;
					}
					goto p2Bottom;					
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
				goto p2Bottom;
		}
		
/* 	Now get verb, can be '=' or '->'. ':' can intervene or be alone. */

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
				} else 
					continue; 
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
				continue;
		}
	
		/* 	End Verb */
		/*	Get Right. It can be '$','[','"','\'', digit, or alpha */
		
		clearSpace;
		switch (*s) { 
			case '$':
				s++;
				Right.flag=1;
				parseSegment(&Right);
				Right.this.segment=assignSegment(Right,Field,NoP1Resolve);
				break;
			case '[':
			case '{':
				Right.flag = 7;
				break;
			case '"':
				Right.flag = 8;	/* Comment */
				break;
			case '\'':			/* Literal */
				Right.flag = 5;
				lookUpLit(&Right);
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
		}
			
decision:
		State = Previous.flag | (Left.flag<<4) | Right.flag | Tentative.flag;
		
/*------------------ This is the main jump table. -------------------------*/
		
		switch(State) {
		
/*-------------------------------------------------------------------------
		First data line, no Tentative or Previous stations. 
		0x400 cases are handled separately. If fromVector,
		no action taken. If toVector, same as 0x100, but no data to parse.
 --------------------------------------------------------------------------*/

				/* 	Note that the error producing cases, 
						430, 160, 237, 267, etc.
					are missing. */
					
			case 0x410:							/*	$123A10: ..	*/
			case 0x420:							/*	A10: ..	*/
			case 0x450:							/*	'TT8W': ..	*/
				LineOrder=ToVector;
				if(DimList)
					parseDims(findDims(theData));
				goto FirstLine;

			case 0x810:	case 0x2810:			/*	$123A10,$123A11: */
			case 0x820:	case 0x2820:			/*	A10:,A11:		*/
			case 0x830:	case 0x2830:			/*	10:,11:			*/
				LineOrder=ToFromVector;
				Tentative=Right;
				ThisContext->stationIncr=temp;
				goto FirstLine1;

			case 0x110: 						/* $123A10: data */
			case 0x120: 						/* A10: data */
			case 0x150: 						/* 'TT8W': data */
			/* p1 without error messages--much distilled, eh? */
				LineOrder=FromVector;
FirstLine:		ThisContext->stationIncr=1;
FirstLine1:		Previous=Left;
				Previous.first=Previous.this.n;
				Tentative.flag = 0x1000;
/*				if(!ThisContext->segment) */
					ThisContext->segment = Previous.this.segment;
				

		/*--------------------- Set up data space ---------------------*/
				theData=setUpDataSpace(Previous.this.n,Previous.this.segment);
				if(!(State&0x600)) {				/* This includes 100 & 800 */
					if(theData!=&TempLocn) 
						theData++;
					Length=parsePolarData(theData); /* Previous.this.n+1 */
				}
				break;
		/*--------------------- End data space setup ---------------------*/
					
/*-------------------------------------------------------------------------
	The Thousands in to-from format.
	There are both Previous and Tentative stations. 
-------------------------------------------------------------------------*/

			case 0x1810: case 0x1820:
			case 0x1830:
				if(Left.this.segment!=Tentative.this.segment ||
						temp!=ThisContext->stationIncr) {
					updateString(0);		/* break in the data stream */
					LineOrder=ToFromVector;
					Tentative=Right;
					ThisContext->stationIncr=temp;
					goto FirstLine1;
				}
				Previous.this.n+=temp;
				Tentative.this.n+=temp;
				Length+=parsePolarData(theData+=ThisContext->stationIncr);
				if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
					/* Data must be reversed in a reverse block. */
					theData->data.cart.x = -theData->data.cart.x;
					theData->data.cart.y = -theData->data.cart.y;
					theData->data.cart.z = -theData->data.cart.z;
				}
				
				break;

/*-------------------------------------------------------------------------
	The Thousands:
	Data lines in mid-stream (also possible after an implicit tie). There 
	is only a Previous station. 
-------------------------------------------------------------------------*/

			case 0x1130:				/* 10: */
				if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr)) { /* most common case */
					Length+=parsePolarData(theData+=ThisContext->stationIncr);
					if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
						/* Data must be reversed in a reverse block. */
						theData->data.cart.x = -theData->data.cart.x;
						theData->data.cart.y = -theData->data.cart.y;
						theData->data.cart.z = -theData->data.cart.z;
					}
					break;
				}
				Previous.this.n-=ThisContext->stationIncr;
				if(abs(temp=Left.this.n-Previous.this.n)>1) {
					Left.Field=Previous.Field;
					desigCpy(Previous.desig,Left.desig);
					Left.this.segment = Previous.this.segment;
					goto implicitTie;
				}
				goto changeBlock;
				
			case 0x1110:		/* $123A10: */
			case 0x1120:				/* A10: */
				if(Left.this.segment==Previous.this.segment) {
					if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr))  { /* most common case */
					Length+=parsePolarData(theData+=ThisContext->stationIncr);
					if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
						/* Data must be reversed in a reverse block. */
						theData->data.cart.x = -theData->data.cart.x;
						theData->data.cart.y = -theData->data.cart.y;
						theData->data.cart.z = -theData->data.cart.z;
					}
					break;
					}
					Previous.this.n-=ThisContext->stationIncr;
					if(abs(temp=Left.this.n-Previous.this.n)<2) {
	changeBlock:		ThisContext->stationIncr=temp;
						if(ThisContext->stationIncr==1) {
							Previous.this.n=Left.this.n;
							Length+=parsePolarData(theData+=ThisContext->stationIncr);
						} else if(ThisContext->stationIncr==-1) {
							Previous.this.n=Left.this.n;
							if(LineOrder==ToVector) {
								thisStation=Left.this.n+1;
								theData=setUpDataSpace(thisStation,Left.this.segment);
							} else {  /* LineOrder is FromVector, or default. Since
										there has been a shift to reversed station 
										order, the data for the Previous station are
										in the wrong place. */
								if(flattenPtr(theData)==flattenPtr(&TempLocn)) {
#ifdef DEBUG
									fprintf(OutFile,"Recalculating theData\n");
#endif
									thisStation=Left.this.n+1;
									theData=setUpDataSpace(thisStation,Left.this.segment);
									if(FullListing) {
										theLine->data = DataList->currentList*DataList->nItems
												+(theData - (struct locnRec *)DataList->list[DataList->currentList].ptr);
									}
								} else
									theData--;
								if(TempLocn.code) {
									*theData=TempLocn;
									TempLocn.code=0;
								} else {
									*theData=*(theData+1);
									(theData+1)->code=0;
								}
								if(!ExchangeFormat) {
									/* Data must be reversed in a reverse block. */
									theData->data.cart.x = -theData->data.cart.x;
									theData->data.cart.y = -theData->data.cart.y;
									theData->data.cart.z = -theData->data.cart.z;
								}
								theData--;
							}
							Length+=parsePolarData(theData);
							if(!ExchangeFormat) {
								theData->data.cart.x = -theData->data.cart.x;
								theData->data.cart.y = -theData->data.cart.y;
								theData->data.cart.z = -theData->data.cart.z;
							}
						} else if(!ThisContext->stationIncr) {
							ThisContext->stationIncr=1;
							break;	/* error. */
						}
						break;
					}
				}
			case 0x1150:
implicitTie:
				/*	We want to find the link that matches Previous 
					and Left stations. */
				theSegment=getSR(Previous.this.segment);
				for(theLocn=0,temp=n=theSegment->links+theSegment->nLocns+theSegment->nLinks
					;temp<=n+theSegment->nTies && !theLocn
					;temp++) {
					theLink=getLink(temp);
					if(theLink->from.n==Previous.this.n 
						&& theLink->to.segment==Left.this.segment
						&& theLink->to.n==Left.this.n)	{	
						theLocn=getLocn(theLink->data);
						if(theLocn->code&_Point) {
							theLocn=0;	/* Already filled. There must be another. */
							errorAlert(DoubleInImplicitTie);
						}
					}
				}
				if(!theLocn)
					errorAlert(NoP2Locn);	/* fatal */
					
				State |= 0x4000;
				if(LineOrder==ToVector) {
					parsePolarData(theLocn);
					if(DimList) {
						/* 	parsePolarData will have put the dims for the to-station
							in the correct place. The azi has to be changed, though. */
						theDims=getDims(theLink->data);
						theDims[-1].azi=theDims->azi;
					}
				} else { /* FromVector (====For Now, ToFrom later===). The tie vector 
							has already been parsed and is either
							in *theData or the global TempLocn. */
					if(TempLocn.code&_Rel) {
						thisStation=theLocn->code;	/* int being used as temp. */
						*theLocn=TempLocn;
						theLocn->code|=thisStation;
						TempLocn.code=0;
						if(DimList) {
							if(DimList->current<theLink->data)
								DimList->current=theLink->data;
							theDims=getDims(theLink->data-1);
							*theDims=TempDims;
							theDims=getDims(theLink->data);
						}
					} else {
						thisStation=theLocn->code;	/* int being used as temp. */
						*theLocn=*theData;
						theLocn->code|=thisStation;
						if(DimList) {
							theDims=findDims(theData);
							TempDims=*theDims;
							if(DimList->current<theLink->data)
								DimList->current=theLink->data;
							theDims=getDims(theLink->data-1);
							*theDims=TempDims;
							theDims=getDims(theLink->data);
						}
						theData->code=0;
					}
					if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
						/* Data must be re-reversed in a reverse block. */
						theLocn->data.cart.x = -theLocn->data.cart.x;
						theLocn->data.cart.y = -theLocn->data.cart.y;
						theLocn->data.cart.z = -theLocn->data.cart.z;
					}
					if((Length-=theLocn->length)<0)
						_errorAlert(UnexpectedError,0,7);
				}
				
				updateString(0);
				if(!(State&0x100))	{	/* A10: ..	, can't call parsePolardata.cart. */
					Tentative.flag=0;
					break;
				}

				Previous=Left;
				Previous.first=Previous.this.n;
				ThisContext->stationIncr=1;
				ThisContext->segment=Previous.this.segment; /* Implicit forces a new segment. */
					
/*--------------------- Set up data space -----------------------*/

				thisStation=(LineOrder==ToVector)? Previous.this.n 
					: Previous.this.n+1;
				theData=setUpDataSpace(thisStation,Previous.this.segment);
				if(LineOrder==ToVector) 
					Length=0;
				else {
					Length= parsePolarData(theData);
					if(DimList) 
						*theDims=*(findDims(theData));
				}
				break;

/*--------------------- End data space setup ---------------------*/

/*------------------------------------------------------------------------------------
	First data line after a link, both Tentative and Previous valid.
		Match criteria:
			desigCmp is true; station numbers are withing +- 1 of each other.
			===	This is bad. There is an allowed increment, dependent upon State.
				It should be looked to.
		Actions:
------------------------------------------------------------------------------------*/

			case 0x2110: 		/* $123A10: */
				if(Left.this.segment == Tentative.this.segment &&
					abs(ThisContext->stationIncr=(Left.this.n-Tentative.this.n))<=1) {
					Previous=Tentative;
					goto setUp2000;
				}	/* no match. Check Previous. */
				if(Left.this.segment == Previous.this.segment  &&
					abs(ThisContext->stationIncr=(Left.this.n-Previous.this.n))<=1)
					goto setUp2000;
				/* no match anywhere. */
				goto FirstLine;

			case 0x2120:				/* A10: */
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

				goto FirstLine;
									
			case 0x2130:
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
					LineOrder=(ThisContext->stationIncr)? ToVector:FromVector;
				Tentative.flag = 0x1000;
				ThisContext->stationIncr=(ThisContext->stationIncr)? ThisContext->stationIncr:1;
				Previous.this.n=Left.this.n;
				/* Data after a tie establishes a new transit. */
				ThisContext->segment = Previous.this.segment;
					
/*--------------------- Set up data space ---------------------*/

				if(ThisContext->stationIncr==1)
					thisStation=(LineOrder==ToVector)? Previous.this.n 
					: Previous.this.n+ThisContext->stationIncr;
				else
					thisStation=(LineOrder==ToVector)? Previous.this.n+ThisContext->stationIncr 
					: Previous.this.n;
				Length=parsePolarData(theData=setUpDataSpace(thisStation,Previous.this.segment));
				
				if((ThisContext->stationIncr==-1)&&(!ExchangeFormat)) {
					/* Data must be reversed in a reverse block. */
					theData->data.cart.x = -theData->data.cart.x;
					theData->data.cart.y = -theData->data.cart.y;
					theData->data.cart.z = -theData->data.cart.z;
				}
				break;
/*----------------------- End data space setup --------------------------*/
							
							
/*-------------------------------------------------------------------------
	0x1400s: Empty data lines in mid-stream (possible after an implicit 
	tie). This is a normal termination of a FromVector (default) string
	and an error in ToVector. In FromVector, an implicit tie can be made.
--------------------------------------------------------------------------*/

			case 0x1410:		/* $123A10: .. */
			case 0x1420:				/* A10: .. */
				if(LineOrder==ToVector) {
					updateString(0);
					goto FirstLine;
				}
				if(Left.this.segment==Previous.this.segment) 
						goto seq400;
				/* no match */
				Tentative.flag=0;
				goto implicitTie;

			case 0x1430:				/* 10: .. */
				if(LineOrder==ToVector) {
					updateString(0);
					Left.Field=Previous.Field;
					desigCpy(Previous.desig,Left.desig);
					Left.this.segment = Previous.this.segment;
					goto FirstLine;
				}
seq400:			if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr)) {
					updateString(0);
					Tentative.flag=0;
					theData+=ThisContext->stationIncr;
					if(DimList) 
						parseDims(findDims(theData));
					break;
				}
				Previous.this.n-=ThisContext->stationIncr;
				if(abs(Left.this.n-Previous.this.n)<=1) {
					errorAlert(ReverseBlock);
				}
				Left.Field=Previous.Field;
				desigCpy(Previous.desig,Left.desig);
				Left.this.segment = Previous.this.segment;
			case 0x1450:				/* 'TT3W' .. */
				Tentative.flag=0;
				goto implicitTie;

/*-----------------------------------------------------------------------
	Empty data line after a link or tie.  
------------------------------------------------------------------------*/
					
			case 0x2410:		/* $123A10: .. */
			case 0x2420:		/*	A10: .. */
				if(Left.this.segment==Tentative.this.segment &&
						Left.this.n==Tentative.this.n) {
					Tentative.flag=0;
					break;
				}	/* no match. Check Previous. */
				if(Left.this.segment==Previous.this.segment &&
						Left.this.n==Previous.this.n) {
					Tentative.flag=0;
					break;
				}	/* no match at all. */
				goto FirstLine;
				
			case 0x2430:		/* 10: .. */
				if((Left.this.n==Tentative.this.n) 
					|| (Left.this.n==Previous.this.n)) {
					Tentative.flag=0;
					break;
				}
				temp=Left.this.n;
				Left=Previous;
				Left.this.n=temp;
				goto FirstLine;
		
/*-------------------------------------------------------------------------
	The friendly fixed locations, treatment is much like the empty data
	lines.
--------------------------------------------------------------------------*/
				
			case 0x1217: 				/*	$123A10 = [56,78,90] */
				if(Left.this.segment==Previous.this.segment) 
					goto seq1217;
				/* no match */
			case 0x1257:
				theData=p2Planar();
				goto implicitTie;
			
			case 0x1227: 				/* A10 = [56,78,90] */
				if(Left.this.segment==Previous.this.segment) {
					theData=p2Planar();
					goto implicitTie;
				}
			case 0x1237:				/* 10: = [5677,45,9] */
seq1217:		if(Left.this.n==(Previous.this.n+=ThisContext->stationIncr)) {
					updateString(0);
					Left=Previous;
					break;
				}
				Previous.this.n-=ThisContext->stationIncr;
				temp=Left.this.n;
				Left=Previous;
				Left.this.n=temp;
				theData=p2Planar();
				if(Left.this.n!=Previous.this.n)
					goto implicitTie;
			case 0x1267: 				/*	. = [56,78,90] */
				updateString(0);
				break;
						
/*---------------------------------- Links: -----------------------------------------*/

			case 0x1262:				/* . = B14 */
			case 0x1265: 				/* . = 'TT8W' */
			case 0x1261: 				/* . = $456B14 */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotLink;

			case 0x1222:
			case 0x1252:				/* 'TT8W' = B14 */
			case 0x1225:				/*  B14 = 'TT8W' */
			case 0x1221: 
			case 0x1255:				/* 'TT8W' = 'Nail' */
			case 0x1211: 				/* $123A10 = $456B14 */
			case 0x1212:				/* $123A10 = B14 */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
	
			case 0x222:	case 0x2222:	/*	A10 = B14  */
			case 0x252: case 0x2252:
			case 0x212: case 0x2212:
			case 0x225: case 0x2225:
			case 0x221: case 0x2221:	/*	$A10 = $456B14 */
			case 0x255: case 0x2255:
			case 0x211:	case 0x2211:	/*	$123A10 = $456B14 */
				Previous=Left;
				Previous.first=Previous.this.n;
				
			case 0x2262:	case 0x262:	
			case 0x265:	case 0x2265:	/* . = 'TT8W' */
			case 0x261:	case 0x2261:	/* . = $456B14 */
makeDotLink:	Tentative=Right;
				Tentative.flag = 0x2000;
				break;
				
/*---------------------------------- Ties: -----------------------------------------*/

			case 0x1362: 				/*	. -> B14 */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotTie;
				
			case 0x362:	case 0x2362:
				Left.this.n += (LineOrder==ToVector)? 0:ThisContext->stationIncr;
				goto makeDotTie;

			case 0x1363: 			/* . -> 6: */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x2363:
				temp=Right.this.n;
				Right=Previous;
				Right.this.n=temp;
				goto makeDotTie;

			case 0x1365: case 0x2365:		/* . -> 'TT8W': */
			case 0x1361: case 0x2361: 		/* . -> $456B14: */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
				goto makeDotTie;

			case 0x1353:					/* 'TT8W' -> 14: */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x353: case 0x2353:
				temp=Right.this.n;
				Right=Previous;            /* not Right=Left */
				Right.this.n=temp;	
				goto makeTie;

			case 0x1313:					/* $123A10 -> 14: */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);
			case 0x313:	case 0x2313:		/* $123A10 -> 14: */
				temp=Right.this.n;
				Right=Previous;				/* not Right=Left */
				Right.this.n=temp;	
				goto makeTie;
	
			case 0x1322: 				/*	A10 -> B14 */
			case 0x1352: 				/*	'TT8W' -> B14 */
			case 0x1312: 				/*	$123A10 -> B14 */
			case 0x1325: 				/*	 B14 -> 'TT8W' */
			case 0x1321: 				/*	$A10 -> $456B14 */
			case 0x1351: 				/* 'TT8W' -> $456B14: */
			case 0x1315: 				/* $456B14 -> 'TT8W' */
			case 0x1311: 				/* $123A10 -> $456B14: */
				updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);

			case 0x322:	case 0x2322:	/*	A10 -> B14 */
			case 0x352:	case 0x2352:	/*	'TT8W' -> B14 */
			case 0x312:	case 0x2312:	/*	$123A10 -> B14 */
			case 0x325:	case 0x2325:	/*	 B14 -> 'TT8W' */
			case 0x321:	case 0x2321:	/*	$A10 -> $456B14 */
			case 0x351:	case 0x2351:	/* 'TT8W' -> $456B14 */
			case 0x315:	case 0x2315:	/* $456B14 -> 'TT8W' */
			case 0x311:	case 0x2311:	/* $123A10 -> $456B14 */
			case 0x355: case 0x2355:		
			case 0x1355:
makeTie:		Previous=Left;
				Previous.first=Previous.this.n;
makeDotTie:		Tentative=Right;
				Tentative.flag = 0x2000;
				clearSpace;
				if(*s==':') {
					s++;
					clearSpace;
				}
				break;

				
/*----------------------- Location (Planar) definitions ----------------------

	Treatment here is identical to that in p1.
	
-----------------------------------------------------------------------------*/

			case 0x227:
			case 0x217:					/*	$123A10 = [56,78,90] */
			case 0x257:		/*	'TT28AZ' = [5677,45,9] */
				Previous=Left;
				Previous.first=Previous.this.n;
				Tentative.flag = 0x1000;
				ThisContext->segment = Previous.this.segment;
				LineOrder=ToVector;	/*==== Attention */
				ThisContext->stationIncr=1;
	/*--------------------- Set up data space ---------------------*/
				ThisContext->segment = Previous.this.segment;
				theData=setUpDataSpace(Previous.this.n,Previous.this.segment);
				break;
	/*--------------------- End data space setup ---------------------*/

			case 0x2227: 				/* A10 = [56,78,90] */
			case 0x2217: 				/*	$123A10 = [56,78,90] */
			case 0x2257:
				Tentative.flag = 0x1000;
				Previous = Left;
				break;

			case 0x2267:		/*	. = [56,78,90] */
				break;
												
			default:
				;
		}	/* End of switch. */
p2Bottom:				
		if(FullListing) {	/* Scott's Listing */
			if(State & 0x4000) {
			/*	Implicit tie encountered. Need to replace the data reference of the from-station
				with a reference to the tie location data. theLink->data is still valid. */
				if((lastDataLineIndex>>MedBlockHi)==(LineList->current>>MedBlockHi)) {
					theLine[n=lastDataLineIndex-LineList->current].data = theLink->data; /* locnRec index */
					theLine[n].state |= 0x8000;				/* last line. Implicit tie from. */
				} else { 			/* Already paged out. Let's just write these out here, */
						 			/*	instead of making a macro. */
					theLine=(struct lineRec *)getLListBuf(lastDataLineIndex,LineList)
								+(lastDataLineIndex&MedBlockLo);
					theLine->data=theLink->data;
					theLine->state |= 0x8000;	/* marks from-station */
					/* Restore things. */
					theLine=(struct lineRec *)getLListBuf(LineList->current,LineList)
							+(LineList->current&MedBlockLo);
				}
			}
			/* Here is how to use a lList efficiently in serial retrieval. This could replace sLists entirely. */
			if(++LineList->current & MedBlockLo)
				theLine++;
			else
				theLine=getLListBuf(LineList->current,LineList);
				
			if((State&0x300)==0x300 || State&0x800)	/* Explicit tie or ToFrom. */
				lastDataLineIndex = 0;
			else {
				theLine->sta = Previous.this;
				theLine->state = State;
				theLine->fields = ThisContext->fields;
			}
			ThisContext->fields=0;
			if(State&0x4000) {
				theLine->data=theLink->data;			/* Just like from-station. */
				lastDataLineIndex = LineList->current;	/* in case it will be another implicit tie */;
			} else if(!State || ((State&0x300)==0x100) || State&0x400) {	/* normal data line */
				theLine->data = DataList->currentList*DataList->nItems
					+(theData - (struct locnRec *)DataList->list[DataList->currentList].ptr);
				if((LineOrder!=ToVector)&&(ThisContext->stationIncr==1))
					theLine->data--;
				lastDataLineIndex = LineList->current;	/* in case it will be an implicit tie */
			} 
			while(LineList->current<LineCnt) {
				if(++LineList->current & MedBlockLo)
					theLine++;
				else
					theLine=getLListBuf(LineList->current,LineList);
				theLine->state = -1;
			}
			State = 0;
		}

	} 	/* End of p2 loop. */

	if(TheOpenField)
		closeField();
	else if(Tentative.flag==0x1000) 
		updateString((LineOrder==FromVector)?ThisContext->stationIncr:0);

	if(theFile)
		fclose(theFile);
	if(ThisContext) free(ThisContext);
	ThisContext=then;	/* restore calling context */
	return 0;
}

#ifdef MAC
#pragma segment fixMethods
#endif

int unadjustedFix(void)
{
	int i,flag,temp;
	struct segmentRec *seg;
	LinkRecPtr theLink;
	struct locnRec *theLocn;

/*----------------------------- fix loops ----------------------------*/

	/* Can reach here without updateString having found any segments to fix. */
	if(!FixArray[SegmentWithFirstLocn]) {
		seg=getSR(SegmentWithFirstLocn);
		theLink=getLink(seg->links);
		theLocn=getLocn(theLink->data);
		FixArray[SegmentWithFirstLocn]=-seg->links;	/* So fixSegment can access linkRec */
		FixArray[SegmentWithFirstLocn]=fixSegment(seg,theLocn,theLink->from.n);
	}
	
	do {
		for(i=1,flag=0;i<=SegList->current;i++) {
			if(FixArray[i]<0) {
				theLink=getLink(-FixArray[i]);
				seg=getSR(i);
				if(theLink->data) {
					theLocn=(theLink->from.segment & Link) ? 
						(getData(theLink->data)) : (getLocn(theLink->data));
					if(theLink->to.segment<0) theLocn--;	/* Locn wanted is in other segment. */
				} else {
					theLocn=getMainData(seg,theLink->to.n);
				}
				flag|=FixArray[i]=fixSegment(seg,theLocn,theLink->to.n);
			}
		}
	} while(flag);	/* convenient flag, signals sillCanFix. */
	
	/*	All fixed that can be fixed. Start asking to fixSegment to seek
		stations to fix on. */
		
	do {
		for(i=1,flag=0;i<=SegList->current;i++) {
			if(FixArray[i]<0) {
				theLink=getLink(-FixArray[i]);
				seg=getSR(i);
				if(theLink->data) {
					theLocn=(theLink->from.segment & Link) ? 
						(getData(theLink->data)) : (getLocn(theLink->data));
					if(theLink->to.segment<0) theLocn--;	/* Locn wanted is in other segment. */
				} else {
					theLocn=getMainData(seg,theLink->to.n);
				}
				flag|=FixArray[i]=fixSegment(seg,theLocn,theLink->to.n);
			} else if(FixArray[i]!=FixedFlag) {
				if(temp=fixSegment(getSR(i),0,0)) {
					FixArray[i]=temp;
					flag++;
				}
			}
		}
	} while(flag);
	
	for(flag=0,i=1;i<=SegList->current;i++) {
		if(FixArray[i] == FixedFlag)
			flag++;
		else {
			_errorAlert(MayFloat,-i,0);
			seg=getSR(i);
			TempLocn.data.cart.x=TempLocn.data.cart.y=TempLocn.data.cart.z=0;
			fixSegment(seg,&TempLocn,seg->first);
			seg->flags &= ~FixedFlag;
		}
	}
	return 0;
}

