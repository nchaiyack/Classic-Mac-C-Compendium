#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#ifdef CMLmake
#include "CML.h"
#include "CMLglobs.h"
#else
#include "MDL.h"
#include "MDLglobs.h"
#endif

/* #define CloseDEBUG 1  */

#ifdef MAC
	#pragma segment closure
#else
	#include <search.h>
#endif
#ifdef MPW
#include <CursorCtl.h>
#endif

/* #define atan2(a,b) eatan2((a),(b)) */
/* extern extended eatan2(extended y,extended x); */

extern void printDashes(void);

/*	functions in MDLclose.c	*/
void simClose(int,c_float);
void markDeadEnds(void);
void killStrings(struct strRec *,struct segmentRec *);
void initializeNetwork(void);
struct strRec *findLiveString(struct segmentRec *, int);
struct strRec *findString(struct segmentRec *seg, int sta);
struct strRec *findMappedString(struct segmentRec *seg, int sta);
LinkRecPtr findLiveTie(struct segmentRec *seg,int sta,struct locnRec **theLocn,int *liIndex);
LinkRecPtr findLiveLink(struct segmentRec *seg,int sta,int *liIndex);
LinkRecPtr findFixedLink(struct segmentRec *seg,int sta,struct locnRec **theLocn,int *liIndex);
struct strRec *reduceLiveString(struct segmentRec *seg, int sta);
struct strRec *findPassingString(struct segmentRec *seg,int sta);
LinkRecPtr findLocn(struct segmentRec *,int);
void traceSuperString(struct strRec *str,struct segmentRec *seg) ;
void printForwardLink(LinkRecPtr li);
struct strRec *crossTraverse(struct strRec *str,int *sta,
	struct segmentRec **seg, int *liIndex,struct stationRec *from);
void *initializeLink(LinkRecPtr,int,void *);
void *initializeTraverse(struct strRec *,struct segmentRec *,int,int,void *);
int reportLongTraverses(void);
void *makeTraLink(LinkRecPtr,int,void *);
void *makeTraTraverse(struct strRec *,struct segmentRec *,int,int,void *);
void *printLink(LinkRecPtr,int,void *);
void *printTraverse(struct strRec *,struct segmentRec *,int,int,void *) ;
void *plotLink(LinkRecPtr,int,void *) ;
void *plotTraverse(struct strRec *,struct segmentRec *,int,int,void *);
void *printTraverseCart(struct strRec *,struct segmentRec *,int,int,void *);
void *printLinkCart(LinkRecPtr,int,void *);
void *adjustLink(LinkRecPtr,int,void *) ;
void *adjustTraverse(struct strRec *,struct segmentRec *,int,int,void *);
static void fixStump(struct strRec *,struct locnRec *,void *);

void *traceTraverse(struct strRec *,struct segmentRec *,
	void *(*)(LinkRecPtr,int,void *),
	void *(*)(struct strRec *,struct segmentRec *,int,int,void *),void *);


int findRow(int,int);
int findEeRow(int,int);
struct colRec *findCol(struct rowRec *,indexType);
struct eeColRec *findEeCol(struct eeRowRec *,indexType,short);

static int fixTies(struct segmentRec *,struct cartData *,int);
void fixNetwork(void);
void fixTree(void);
void *findLengthTraverse(struct strRec *,struct segmentRec *,int,int,void *);
void *findLengthLink(LinkRecPtr,int,void *);
void removeColumn(TraRecPtr);
void removeEEColumn(TraRecPtr);
void removeClosedLoop(TraRecPtr);
void addColumn(TraRecPtr,short);

/*==========
void atsub(c_float x[],c_float v[]);
void asub(c_float x[],c_float v[]);
===========*/
int reportEulerLoops(void);
void EETransform(void);
void *makeEETraLink(LinkRecPtr,int,void *); 
void *makeEETraTraverse(struct strRec *,struct segmentRec *,int,int,void *) ;
struct eeColRec *searchForHighLevelLoop(struct eeRowRec *,int,int,int);
void mergeSeriesTraverses(struct eeRowRec *);
void removeDeadEnd(struct eeRowRec *);


/*------------------------- Simultaneous Closure Routines -----------------*/

void simClose(int count,c_float tolerence)
{
	int	i,j,cnt,temp,nProcessedRows,matrixSize;
	int dox,doy,doz;
	int totalLegs;
	struct segmentRec *seg;
	struct strRec *str;
	LinkRecPtr li;
	struct locnRec *lo;
	struct rowRec *row,*row0;
	struct colRec *col,*col0;
	TraRecPtr tra;
	c_float weightH,weightV,wx,wy,wz,omega,delta;
	c_float dx,dy,dz,one=1;
	c_float spih=0,spiv=0;
	c_float totalLength=0;
	Str31 a,b;

	/*---------------------- remove dead end strings ------------------------*/
	/*
		1. 	Each string has a count of junctions, set back in p1.5, for both 
			its begining and end stations. My use of this comes from Tom Kaye's
			version of CMAP, from which I have from both Tom and Bob Thrun. These
			counts are one less than the number of segment shots that have been
			made to or from the particular station.
		2.	This count is zero for a station that is a dead end. Dead ends have
			been marked between passes 1 and 2.
		3.	Note that all unexceptional string junctions are represented in the 
			list of string data structures. What is unexceptional is really that
			and will be treated below.
		4.	The tie data, then, contains ties between strings and spray shots. 
			The latter are dead ends.
		5.	Thus, dead ends are (a) strings with one junction count
			(->nFirst or ->nLast) equal to zero and (b) ties to a station that
			is not part of a string and to which no other tie has been made. 
			These have been found and flagged in p1.5.
	----------------------------------------------------------------
		The procedure is:
		
		1.	For each segment:
					
			find any (5a) (see above)
				mark it dead.
					find its live end and reduce that junction's count by one.
		2.	Crossing multiple unmapped ties is a headache, but possible.
		
	--------------------------------------------------------------------------*/

	if(CloseFlag==1 || CloseFlag==-1) {
	
		/*---------------------- Report Long Traverses ---------------------*/
		if(!Silent) 
			cnt=reportLongTraverses();
		
		/*-----------------------------------------------------------------
			Count junctions. Set up rowRecs. There are as many rows
			as junctions. Rows are junctions. Located rows are ones that are
			part of the network and will be adjusted. The property
			of being located propagates out from located locations.
		------------------------------------------------------------------*/
		if(!Silent)
			fprintf(OutFile,"Setting up network traverses...\n");
		
		RowList->other=0;	/* Count of located rows. */
		ColList->other=0;	/* Count of processed columns. */
		
		for(ColList->current=i=1;i<=SegList->current;i++) {
			seg=getSR(i);
			if(seg->nLocns) {
				for(j=seg->links;j<seg->links+seg->nLocns;j++) {
					li=getLink(j);
					if(str=findString(seg,li->from.n))
						str->code|=HasLocn;
					else
						_errorAlert(UnexpectedError,i,li->from.n);
				}
			}
			temp=RowList->current+1;
			seg->nJcts=0;
			for(j=seg->strings,row=0;j<seg->strings+seg->nStrings;j++) {
				str=getStr(j);
	
				/*------------------------------------------------------	
					The following criterion detects junctions by looking
					at vertex counts. In this way, rows are assigned to
					junctions in segment/station rank order. 
				-------------------------------------------------------*/
				
				if(str->nFirst>1 && str->nFirst<1000) {
					if(!row || (row->this.n!=str->first)) {
						/* i.e. str->first not in last row.  */
						row=getNextRow();
						row->cols=ColList->current;
						if((str->code&HasLocn) && (li=findLocn(seg,str->first))) {
							/* Is this is the traverse with the Fixed Locn? */
							ColList->current+=row->nCols=str->nFirst;
							lo=getLocn(li->data);
							row->locn=row->vector=lo->data.cart;
							col=getCol(row->cols);

							row->code=(HasLocn|FixedFlag|Located);
							RowList->other++;
							row->cols++;
						} else {
							ColList->current+=row->nCols=str->nFirst+1;
							row->code=0;
						}
						seg->nJcts++;
						row->this.n=str->first;
						row->this.segment=i;
					}
				}
				if(!(str->code&UnMapped)) {
					if(str->nLast>1 && str->nLast<1000) {
						row=getNextRow();
						row->cols= ColList->current;
						if((str->code&HasLocn) && (li=findLocn(seg,str->last))) {
							ColList->current+=row->nCols=str->nLast;
							lo=getLocn(li->data);
							row->locn=row->vector=lo->data.cart;
							col=getCol(row->cols);

							row->code=(HasLocn|FixedFlag|Located);
							RowList->other++;
							row->cols++;
						} else {
							ColList->current+=row->nCols=str->nLast+1;
							row->code=0;
						}
						seg->nJcts++;
						row->this.n=str->last;
						row->this.segment=i;
					}
				}
			}
			seg->rows=(seg->nJcts)? temp:0;
		}
		/*------------------------------------------------------------------
			At this point, all junctions have been created and there is a
			row record for each. RowList->other has counted located rows, and
			should be equal to the number of fixed locations in the data.
		-------------------------------------------------------------------*/
		if(!Silent) {
			fprintf(OutFile,"There %s %d complete traverse(s)\n",
					(cnt==2)? "is":"are",cnt-1);
			fprintf(OutFile,"There %s %d junction(s)\n",
					(RowList->current==1)? "is":"are",RowList->current);
			if(!RowList->current) {
				fprintf(OutFile,"Loop adjustment does not need to be made\n");
				return;
			}
			fprintf(OutFile,"Building matrix...\n");
		}
	
		/*------------------------- Build matrix -------------------------*/
		for(i=1;i<=SegList->current;i++) {
			seg=getSR(i);
			for(j=seg->strings;j<seg->strings+seg->nStrings;j++) {
				str=getStr(j);
				if(str->code&SuperStringEntry) {
					RotateCursor(j);
					/*------------------------------------------------------
						The SuperStringEntry flag was set in p1.5 as part 
						of the marking of merging traverses through trivial
						junctions (i.e. of order 1). This criterion produces
						an entry into all of the consolidated traverses that
						pass through the main data.
					-------------------------------------------------------*/
					tra=getNextTra();
					tra->self=LongTraList->current;
					tra->code=tra->nLegs=0;
					tra->entryStr=j;	/* index to entry string */
					tra->entrySegment=i;
					tra->traLength=0;
					/* This trace fills traLength, netLength.{x,y,z}, tra->nLegs 
						and sets fromVertex and toVertex with row indices. */
					traceTraverse(str,seg,makeTraLink,makeTraTraverse,tra);
					tra->effLength=sqrt(tra->effLength);
					if(tra->code&UnProcessed) 
						removeColumn(tra);
					else if(tra->fromVertex==tra->toVertex) 
						removeClosedLoop(tra);
					else 
						addColumn(tra,0);
				}
			}
			seg=getSR(i);	/* Re-get, just in case we are thrashing. */
			
			/*======= 
				#define reGetSR(seg,i) ((seg)->segment==(i))? seg : getSR(i);
			========*/
			
			/*-------------------------------------------------------------
				For the same segment, look through the ties for the one-shot
				traverses in them. 
			---------------------------------------------------------------*/
			
			for(j=seg->links+seg->nLocns+seg->nLinks
					;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
				li=getLink(j);
				if(li->to.segment<0) continue;
				lo=getLocn(li->data);
				if(lo->code&(SuperStringMember|DeadEnd)) continue;
				tra=getNextTra();
				tra->self=LongTraList->current;
				tra->code=0;
				tra->fromVertex=findRow(i,li->from.n);
				tra->toVertex=findRow(li->to.segment,li->to.n);
				if(!tra->fromVertex || !tra->toVertex) {
					_errorAlert(UnexpectedError,i,li->to.n);
				} else if(lo->code&_Rel) {
					tra->nLegs=1;
					tra->entryStr=-j;	/* Minus index to tie */
					tra->entrySegment=i;
					tra->effLength=tra->traLength=((c_float)lo->length)/10;
					tra->netLength=lo->data.cart;
					addColumn(tra,0);
				} else {
					tra->code|=UnProcessed;
					tra->nLegs=1;
					tra->entryStr=-j;	/* Minus index to tie */
					tra->entrySegment=i;
					removeColumn(tra);
				}
			}
		}
		
		/*----------------- Reset (Off-Diagonal) Column Indices -----------------*/
		for(j=1,nProcessedRows=0;j<=RowList->current;j++) {
			row=getRow(j);
			row->cols-=row->nCols;
			if(row->code&Processed)
				nProcessedRows++;
		}
	
		matrixSize=0;	
		
		/*------------------- Report Pre-adjustment closure errors ------------*/
		
		if(ListingFlag&4) {
			plotNetwork(2);
			if(Super3D)
				plotNetwork(1);
		}
	
	
		/*------------------------ Enter fixed locations ------------------------*/
			/*
				Already done. We can use the HasLocn flag to avoid processing
				fixed locns. There is no need to simplify the rest of the row;
				it is ignored.
			*/
	
	#ifdef DEBUG
		fprintf(OutFile,"\nPreadjustment Vertices\n\n");
		for(i=1;i<=RowList->current;i++) {
			row=getRow(i);
			fprintf(OutFile,"Row %d sta %4d:%-3d locn: %10.1f%10.1f%10.1f\n",
				i,row->this.segment,row->this.n,row->locn.x,row->locn.y,row->locn.z);
		}
	#endif
	
	} else {	/* Update matrix */
		/*	There has been a previous call to SimClose(), so the matrix is already
			set up and solved once. Rows that have active columns are considered to
			be fixed and are marked as such. */
			
		matrixSize=RowList->other;
		
		for(i=1,nProcessedRows=0;i<=RowList->current;i++) {
			row=getRow(i);
			if(row->code&Processed) {
				nProcessedRows++;		/* This is a recount. */
				if(row->code&Located) 
					row->code|=FixedFlag; /* RowList->other counts located rows. */
			} 
			if(!(row->code&FixedFlag)) {	/* All available columns have to be set up. */
				for(j=row->cols;j<row->cols+row->nCols;j++) {
					col=getCol(j);
					tra=getTra(col->traverse);
					if(tra->code&UnProcessed) {
						tra->code=tra->nLegs=0;
						tra->traLength=0;
						if(tra->entryStr>0) {
							traceTraverse(getStr(tra->entryStr),getSR(tra->entrySegment),makeTraLink,makeTraTraverse,tra);
							tra->effLength=sqrt(tra->effLength);
							if(!(tra->code&UnProcessed) && tra->fromVertex!=tra->toVertex) {
								if(!(row->code&Processed))
									nProcessedRows++;
								addColumn(tra,(short)j);
							}
						} else {	/* Single-tie traverse */
							li=getLink(-tra->entryStr);
							lo=getLocn(li->data);
							if(lo->code&_Rel) {
								if(!(row->code&Processed))
									nProcessedRows++;
								tra->nLegs=1;
								tra->effLength=tra->traLength=((c_float)lo->length)/10;
								tra->netLength=lo->data.cart;
								addColumn(tra,(short)j);
							} else				/* Still unprocessed. */
								tra->code|=UnProcessed;		
						}
					} 
				}
			}
		}			
	} 
	
	/*-------------------- Fill in any missing row locations  ------------------
		Cnt is the number of juctions (i.e. rows) that were not made part of
		the network in the first pass through the traverses. Unlocated rows 
		can also result from disconnected networks. 
	---------------------------------------------------------------------------*/
	
	if(CloseFlag<0) 
		nProcessedRows=RowList->current;
		
	if((cnt=nProcessedRows-RowList->other)<0) {
		cnt=0;
		errorAlert(CloseBeforeLocn);
	}
	while(cnt) {
		/* 
			The failure to fix cnt number of junctions resulted from a sweep forward
			through them, above. Lets alternate forward and backward sweeps, starting
			backwards.
		*/
		for(i=RowList->current;cnt&&i>0;i--) {
			row=getRow(i);
			if(row->code&Processed && !(row->code&Located)) {
				for(j=row->cols;j<row->cols+row->nCols;j++) {
					col=getCol(j);
					if(col->col>0) {
						row0=getRow(col->col);
						if(row0->code&Located) {
							cnt--;
							row->code|=Located;
							RowList->other++;
							tra=getTra(col->traverse);
							if(tra->fromVertex==i) {
								row->locn.x=row0->locn.x-tra->netLength.x;
								row->locn.y=row0->locn.y-tra->netLength.y;
								row->locn.z=row0->locn.z-tra->netLength.z;
							} else {
								row->locn.x=row0->locn.x+tra->netLength.x;
								row->locn.y=row0->locn.y+tra->netLength.y;
								row->locn.z=row0->locn.z+tra->netLength.z;
							}
							break;
						}
					}
				}
			}
		}
		for(i=1,temp=0;cnt&&(i<=RowList->current);i++) {
			row=getRow(i);
			if(row->code&Processed && !(row->code&Located)) {
				for(j=row->cols;j<row->cols+row->nCols;j++) {
					col=getCol(j);
					if(col->col>0) {
						row0=getRow(col->col);
						if(row0->code&Located) {
							cnt--;
							row->code|=Located;
							RowList->other++;
							tra=getTra(col->traverse);
							if(tra->fromVertex==i) {
								row->locn.x=row0->locn.x-tra->netLength.x;
								row->locn.y=row0->locn.y-tra->netLength.y;
								row->locn.z=row0->locn.z-tra->netLength.z;
							} else {
								row->locn.x=row0->locn.x+tra->netLength.x;
								row->locn.y=row0->locn.y+tra->netLength.y;
								row->locn.z=row0->locn.z+tra->netLength.z;
							}
							break;
						}
					}
				}
				if(!(row->code&Located)) {
					if(++temp>=cnt) {
						cnt--;
						if(CloseFlag<0) {
						/*	As many rows failed to fix on this iteration as 
							there are left to fix. This is the signal that 
							there is a disconnected network. */
							row->code|=Located;
							RowList->other++;
							_errorAlert(SeparateNetwork,row->this.segment,row->this.n);
						}
					}
				}
			}
		}
	}

	matrixSize=RowList->other-matrixSize;
	
	if(!Silent) {
		reportByVertex();
		reportByLongTraverse();
	}
	
	/*------------ Consolidate matrix: merge parallel traverse --------------*/
	for(i=1;i<=RowList->current;i++) {
		row=getRow(i);
		if(row->code&Located && !(row->code&FixedFlag)) {		/* Skip fixed locn rows. */
			for(j=row->cols;j<row->cols+row->nCols;j++) {
				col=getCol(j);
				if(col->col>0) {
					for(cnt=j+1;cnt<row->cols+row->nCols;cnt++) {
						col0=getCol(cnt);
						if(col->col==col0->col) {
							col->value.x+=col0->value.x;
							col->value.y+=col0->value.y;
							col->value.z+=col0->value.z;
							col0->col=0;	/* Keep the record around. */
						}
					}
				}
			}
		}
	}
		
	/*-------------------------- Solve Matrix -----------------------------*/
#ifdef DEBUG	
	omega = sqrt((RowList->current<135)? RowList->current:135);	/* This doesn't work. Bug reported to Apple. */
	fprintf(OutFile,"Rows: %d omega: %6.1f\n",RowList->current,omega);
#endif
	dx = (matrixSize<135)? matrixSize:135;
	omega = 2.0250-1.1/sqrt(dx);
	for(dox=doy=doz=temp=cnt=1;(cnt<=count)&&temp;cnt++) {
		dx=dy=dz=0;
		temp=0;
		RotateCursor(cnt<<3); 
		for(i=1;i<=RowList->current;i++) {
			row=getRow(i);
			if(row->code&Located && !(row->code&FixedFlag)) { 
				for(j=row->cols,wx=wy=wz=0,row0=0;j<row->cols+row->nCols;j++) {
					col=getCol(j);
					if(col->col>0) {
						row0=getRow(col->col);
						if(dox)
							wx+=col->value.x*row0->locn.x;
						if(doy)
							wy+=col->value.y*row0->locn.y;
						if(doz)
							wz+=col->value.z*row0->locn.z;
					}
				}
				if(row0) {	/* flag: at least one processed column. */
					if(dox) {
						delta=omega*((row->vector.x-wx)/row->diagonal.x-row->locn.x);
						row->locn.x+=delta;
						delta=fabs(delta);
		#ifdef CloseDEBUG
						fprintf(OutFile,"Deltas: cnt %d, row=%d, col=%d wx: %6.1f dx: %6.1f",cnt,i,j,wx,delta);
		#endif
						dx=(dx>delta)? dx:delta;
					}
					if(doy) {
						delta=omega*((row->vector.y-wy)/row->diagonal.y-row->locn.y);
						row->locn.y+=delta;
						delta=fabs(delta);
		#ifdef CloseDEBUG
						fprintf(OutFile," wy: %6.1f y: %6.1f",wy,delta);
		#endif
						dy=(dy>delta)? dy:delta;
					}
					if(doz) {
						delta=omega*((row->vector.z-wz)/row->diagonal.z-row->locn.z);
						row->locn.z+=delta;
						delta=fabs(delta);
		#ifdef CloseDEBUG
						fprintf(OutFile," wz: %6.1f z: %6.1f\n",wz,delta);
		#endif
						dz=(dz>delta)? dz:delta;
					}
				}
			}
		}
		if(!Silent) {
			if(!(cnt%10)) {
				delta=(dx>dy)?dx:dy;
				delta=(delta>dz)?delta:dz;
				fprintf(OutFile,"Converged to %f at %d interations\n",ForM(delta),cnt);
			}
			if(dox)
				if(!(dox=(dx>tolerence)))
					fprintf(OutFile,"x converged to %f in %d iterations\n",ForM(dx),cnt);
			if(doy)
				if(!(doy=(dy>tolerence)))
					fprintf(OutFile,"y converged to %f in %d iterations\n",ForM(dy),cnt);
			if(doz)
				if(!(doz=(dz>tolerence)))
					fprintf(OutFile,"z converged to %f in %d iterations\n",ForM(dz),cnt);
		}
		temp=dox||doy||doz;

	}

#ifdef DEBUG
	fprintf(OutFile,"\nPostadjustment Vertices\n\n");
	for(i=1;i<=RowList->current;i++) {
		row=getRow(i);
		fprintf(OutFile,"Row %d sta %4d:%-3d locn: %10.1f%10.1f%10.1f\n",
			i,row->this.segment,row->this.n,row->locn.x,row->locn.y,row->locn.z);
	}
#endif
/*---------------------------- Report Network ------------------------------*/
		
	if(!Silent) {
		putc('\n',OutFile);
		printDashes();
		fputs ("Closure Statistics\n"																,OutFile);
		fputs ("  The following table reports the amount of adjustment for each network tra-\n"		,OutFile);
		fputs ("  verse. The Amount of Adjustment is reported as a spherical vector (i.e. dis-\n"	,OutFile);
		fputs ("  tance, bearing in x-y, and inclination or vertical angle). The error ratios\n"	,OutFile);
		fputs ("  are the quotient 'adjustment distance'/'expected error'. Expected error is:\n\n"	,OutFile);
		fputs ("            (nLegs * (a + b * (traverseLength/nLegs)^2))^0.5\n\n"					,OutFile);
		fputs ("  where a = 0.1222 and b = 22.2784e-4 for x and y,\n"								,OutFile);
		fputs ("        a = 0.0076 and b = 41.8609e-4 for z.\n\n"									,OutFile);
		fputs ("  Error ratios greater than 1.5 are flagged with one asterisk. Ratios greater\n"	,OutFile);
		fputs ("  than 5.0 have two asterisks.\n"													,OutFile);
		fprintf(OutFile,"  Distances are in %s.\n",(MetersOut)?"meters":"feet");

		printDashes();
		fputs ("\n                      Table of Closure Adjustments on Traverses\n\n"				,OutFile); 
		fputs ("                   Traverses               Amount of Adjustment   Error ratios\n"	,OutFile);
		fputs ("       Begin      End       N   Length    Dist   Bearing  Incln   Horz   Vert\n\n"	,OutFile);

		for(i=1,totalLegs=0;i<=LongTraList->current;i++) {
			tra=getTra(i);
			row=getRow(tra->fromVertex);
			if(!(row->code&Located)) {
				if(tra->toVertex)
					row0=getRow(tra->toVertex);
				fprintf(OutFile,"%-5d%-10s%-10s   (not located)\n",
					i,printFullSta(row->this.n,getSR(row->this.segment),a),
					(tra->toVertex)? printFullSta(row0->this.n,getSR(row0->this.segment),b):"  loop");
			} else if(tra->code&UnProcessed) {
				if(tra->toVertex)
					row0=getRow(tra->toVertex);
				fprintf(OutFile,"%-5d%-10s%-10s   (not processed)\n",
					i,printFullSta(row->this.n,getSR(row->this.segment),a),
					(tra->toVertex)? printFullSta(row0->this.n,getSR(row0->this.segment),b):"  loop");
			} else {
				if(tra->toVertex) {
					row0=getRow(tra->toVertex);
					/*-----------------------------------------------------------------
						Compute net vector of string between adjusted end points.
					------------------------------------------------------------------*/
					dx = row0->locn.x - row->locn.x;
					dy = row0->locn.y - row->locn.y;
					dz = row0->locn.z - row->locn.z;
					/*------------------------------------------------------------------
						Compare with old vectors to find amount of adjustment.
					-------------------------------------------------------------------*/
					wx = dx - tra->netLength.x;
					wy = dy - tra->netLength.y;
					wz = dz - tra->netLength.z;
					delta = sqrt(wx*wx+wy*wy+wz*wz);
					/*------------------------------------------------------------------
						Sum number of strings, number of shots,length, and error.
					-------------------------------------------------------------------*/
					omega += delta;
			
					/*------------------------------------------------------------------
						tra->nLegs is number of shots in string
					-------------------------------------------------------------------*/
					totalLegs += tra->nLegs;
					totalLength += tra->traLength;
					
					spih += 2.*sqrt(wx*wx+wy*wy)*tra->nLegs*sqrt((c_float)tra->nLegs)/tra->traLength;
					spiv += 2.*fabs(wz)*tra->nLegs*sqrt((c_float)tra->nLegs)/tra->traLength;
		
				} else {	/* closed loop. Calculate some stats anyway. */
					wx = -tra->netLength.x;
					wy = -tra->netLength.y;
					wz = -tra->netLength.z;
					delta = sqrt(wx*wx+wy*wy+wz*wz);
				}
				/*-----------------------------------------------------------------
					weightH is error bearing, weightV is error inclination.
				-------------------------------------------------------------------*/
	#ifdef CMLmake
				weightH = atan2(wx,wy) * RtoD;
	#else
				weightH = atan2(wy,wx) * RtoD;
	#endif
				
				if ( wy<0) weightH += 180;
				if ( weightH<0) weightH += 360;
		
				weightV = (wy==0&&wz==0)? 0 : atan2(wz,sqrt(wx*wx+wy*wy)) * RtoD;
			 
				/*-----------------------------------------------------------------
					probable error is 1./weight
					error ratio is (amount of adjustment)/(probable error)
					error ratio is (amount of adjustment)*(weight)
				--------------------------------------------------------------------*/
				dx = wx / sqrt(one/tra->weight.x);
				dy = wy / sqrt(one/tra->weight.y);
				dx = sqrt(dx*dx+dy*dy);	/* errath */
				dz = fabs(wz/sqrt(one/tra->weight.z));
				/*-------------------------------------------------------------------
					Now print the results for this string.
				---------------------------------------------------------------------*/
				fprintf(OutFile,"%-5d%-10s%-10s%4d %8.1f %7.1f %8.1f %7.1f %6.2f %6.2f",
					i,printFullSta(row->this.n,getSR(row->this.segment),a),
					(tra->toVertex)? printFullSta(row0->this.n,getSR(row0->this.segment),b):"  loop",
					tra->nLegs,ForM(tra->traLength),
					ForM(delta),weightH,weightV,dx,dz);
				if(dx>1.5||dz>1.5) fprintf(OutFile,"*");
				if(dx>5||dz>5) fprintf(OutFile,"*");
				fprintf(OutFile,"\n");	
			}
		}	/* end of loop to print table of adjustments on strings */
	
/*----------------------------------------------------------------------------
	errt = errt/totalLength*100.
	spi = (spi/real(totalLegs)) * 52.67845
	Chi square for 68.26 percentage ("standard deviation")
-----------------------------------------------------------------------------*/
		if(totalLegs) {
			spih = (spih/totalLegs) * 53.55;
			spiv = (spiv/totalLegs) * 57.79;
		} else 
			spih=spiv=0;
		
		fprintf (OutFile,"\nTraverses in adjusted net  %d\n",LongTraList->current);
		fprintf (OutFile,"Junctions in adjusted net     %d\n",RowList->current);
		fprintf (OutFile,"Euler loops in net            %d\n",LongTraList->current-RowList->current+1);
		fprintf (OutFile,"Total %s legs in net      %d\n",segmentName,totalLegs);
		fprintf (OutFile,"Length of net             %6.1f ft, %6.1fm\n",totalLength,totalLength/MtoFt);
		fprintf (OutFile,"Compass standard deviation    %f\n",spih);
		fprintf (OutFile,"Clinometer standard deviation %f\n",spiv);
	}
	
#if 0
/*--------------------------------------------------------------------------
	Reset the coordinates of all the defined stations.
----------------------------------------------------------------------------*/
	for( i=1;i<=nc;i++) {
		sta(1,[conloc[i]] = constr[1,i];
		sta(2,[conloc[i]] = constr[2,i];
		sta(3,[conloc[i]] = constr[3,i];
	}

/*---------------------------------------------------------------------------
	Recompute station coordinates
	Reset istat for the benefit of print routine
---------------------------------------------------------------------------*/
	for(i=1;i<=n;i++) {
		istat[i] = i;
		if (i==lasta[i])
			continue;		/* Bypass the doubling of defined stations. */
		sta[1,i] = sta[1,lasta[i]]+delta[1,i];
		sta[2,i] = sta[2,lasta[i]]+delta[2,i];
		sta[3,i] = sta[3,lasta[i]]+delta[3,i];
	}

#endif	
	return;
}

/*---------------------------- killStrings -----------------------------
	
	Strings are dead when the count of effective links at one of its ends
	reaches zero. This routine marks a string already known to be dead 
	with the DeadEnd flag bit and then searches its non-dead end to see
	if other strings are to be killed.
	
------------------------------------------------------------------------*/

void markDeadEnds(void) {
	int i,j;
	struct segmentRec *seg;
	struct strRec *str;
	LinkRecPtr li;
	
	for(i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		if(seg->nLocns) {
			for(j=seg->links;j<seg->links+seg->nLocns;j++) {
				li=getLink(j);
				if(str=findString(seg,li->from.n))
					str->code|=HasLocn;
				else
					_errorAlert(UnexpectedError,i,li->from.n);
			}
		}
		for(j=seg->strings;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);
			if(str->code&DeadEnd) continue;
			if(!str->nFirst) {
				killStrings(str,seg);
				continue;
			}
			if(str->code&UnMapped) continue;
			if(!str->nLast)
				killStrings(str,seg);
		}
	}
	return;
}

void killStrings(struct strRec *deadStr,struct segmentRec *seg)
{
	int n=1,sta;
	int liIndex;
	struct strRec *str,*str0;
	LinkRecPtr li,li0;
	struct locnRec *lo;
	do {		/* while there is a deadStr to kill. */
		str=deadStr;
		deadStr=0;
		str->code|=DeadEnd;
		NumStrJct-=(str->code&ContigAtStart)? 1 : 2;
		if(str->code&UnMapped) {
			sta=str->first;
			if(str->code&HasTie) {
				if(li=findLiveTie(seg,str->first,&lo,&liIndex)) {
					lo->code|=DeadEnd;
					seg=getSR(abs(li->to.segment));
					deadStr=reduceLiveString(seg,li->to.n);
					continue;
				}
			}
			if(str->code&HasLink) {
				if(li=findLiveLink(seg,str->first,&liIndex)) {
					li->from.segment|=DeadEnd;
					seg=getSR(li->to.segment);
					if(li0=findLiveLink(seg,li->to.n,&liIndex)) 
						li0->from.segment|=DeadEnd;
					deadStr=reduceLiveString(seg,li->to.n);
					continue;
				}
			}
			if(str->code&HasLocn) {
				_errorAlert(UnexpectedError,seg->segment,sta);
				continue;
			}
		} else if(str->nFirst) {
			sta=str->first;
			n = --str->nFirst;	/* reduce count of live end */
			if(str->code&ContigAtStart) {
				str0=getStr(str->string-1);
				if(!(str0->nLast=n)) {	/* pass the count to it. If zero, */
					if(!(str0->code&DeadEnd)) { /* If not already dead, */
						deadStr=str0;		/* kill the contiguous string. */
						continue;
					}
				}
			} 
		} else if(str->nLast){
			sta=str->last;
			n = --str->nLast;	/* reduce count */
			if(str->code&ContigAtEnd) {
				str0=getStr(str->string+1);
				if(!(str0->nFirst=n)) {
					if(!(str0->code&DeadEnd)) {
						deadStr=str0;
						continue;
					}
				}
			} 
		} else
			return;	/* Both ends dead. No further action. */
		
		if(n<0)
			_errorAlert(UnexpectedError,seg->segment,sta);
			
		if(!n) {
			/*	This end was 1, and is now 0 and there is no contiguous string.
				There should be a live tie to kill. Sta is properly filled. */
				
			if(li=findLiveTie(seg,sta,&lo,&liIndex)) {
				lo->code|=DeadEnd;
				seg=getSR(abs(li->to.segment));
				deadStr=reduceLiveString(seg,li->to.n);
				continue;
			}
			if(li=findLiveLink(seg,sta,&liIndex)) {
				li->from.segment|=DeadEnd;
				seg=getSR(li->to.segment); /* Links have only positve to.segments. */
				deadStr=reduceLiveString(seg,li->to.n);
				continue;
			} 
		}  
	} while(deadStr);
	return;
}

void initializeNetwork(void)
{
	int i,j,cnt;
	struct segmentRec *seg;
	struct strRec *str,*str0;
	
	/* Put in traces through data. */
	
	for(i=1,cnt=0;i<=SegList->current;i++) {
		seg=getSR(i);
		for(j=seg->strings;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);
			if(str->code&SuperStringMember) continue;
			if(str->code&DeadEnd) {
				if(str->nFirst==1) {
					if(str->code&ContigAtStart) 
						continue;	/* Either already gotten or prev string is live. */
				} else if(str->nLast==1) {
					if(str->code&ContigAtEnd) {
						str0=getStr(str->string+1);
						if(!(str0->code&DeadEnd)) 
							continue;
					}
				} else /* neither junction count is 1(+1) */
					continue;
			}
			if(str->code&HasLocn && (str->nFirst+str->nLast)==1)
					continue;

			cnt++;
			str->code|=SuperStringEntry;
#ifdef DEBUG
			fprintf(OutFile,"%d	enters at $%d%.4s%d\n",cnt,seg->Field,seg->desig,str->first);
#endif
			traceTraverse(str,seg,initializeLink,initializeTraverse,0);
		}
	}
	if(CloseFlag) {
		RowList=lListAlloc(0,BlockLen,sizeof(struct rowRec),1+cnt/BlockLen,nRowList);
		ColList=lListAlloc(0,BlockLen,sizeof(struct colRec),1+(2*cnt/BlockLen),nColList);
	} 
	if(CloseFlag || EEFlag || LogFlag&128)
		LongTraList=lListAlloc(0,BlockLen,sizeof(struct longTraverseRec),1+cnt/BlockLen,nLongTraList);
}

struct strRec *findLiveString(struct segmentRec *seg, int sta)
{
	int i;
	struct strRec *str;
	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(str->first>sta) break;
		if(!(str->code&DeadEnd)) {
			if(str->first==sta) return str;
			if(str->code&UnMapped) continue;
			if(str->last==sta) return str;
		}
	}
	return 0;
}

struct strRec *findString(struct segmentRec *seg, int sta)
{
	int i;
	struct strRec *str;
	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(str->first>sta) break;
		if(str->first==sta || str->last==sta) return str;
	}
	return 0;
}

struct strRec *findPassingString(struct segmentRec *seg,int sta)
{
	int i;
	struct strRec *str;
	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(str->first>sta) break;
		if(str->first==sta) {
			if(str->nFirst==1 || str->nFirst<0)
				return str;
			else
				return 0;
		}
		if(str->last==sta) {
			if(str->nLast==1 || str->nLast<0)
				return str;
			else
				return 0;
		}
	}
	return 0;
}

struct strRec *findMappedString(struct segmentRec *seg, int sta)
{
	int i;
	struct strRec *str;
	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(str->first>sta) break;
		if(str->code&UnMapped) continue;
		if(str->first==sta) return str;
		if(str->last==sta) return str;
	}
	return 0;
}

struct strRec *reduceLiveString(struct segmentRec *seg, int sta)
{
	int i;
	struct strRec *str,*str0;
	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(str->first==sta) {
			if(str->code&DeadEnd)
				return str;
			else if(str->nFirst) {
				if (!(--str->nFirst)) 
					return str;
				else if(str->nFirst==1 && str->code&HasLocn)
					return 0;
				else
					return 0;
			} else {	/* Null nFirst, but not yet dead. Must be unmapped. */
				if(!(str->code&UnMapped)) 
					_errorAlert(UnexpectedError,seg->segment,str->last);
				str->code|=DeadEnd;
				return 0;
			}
		}
		if(str->last==sta) {
			if(str->code&DeadEnd)
				return str;
			else {
				if(--str->nLast<0)
					_errorAlert(UnexpectedError,seg->segment,str->last);
				if(str->code&ContigAtEnd) {
					str0=getStr(i+1);
					if((--str0->nFirst)!=str->nLast)
						_errorAlert(UnexpectedError,seg->segment,str->last);
					str0->nFirst=str->nLast;
				}
				if (!(str->nLast))
					return str;
				else if(str->nLast==1 && str->code&HasLocn)
					return 0;
				else
					return 0;
			} 
		}
	}
	return 0;
}

LinkRecPtr findLiveTie(struct segmentRec *seg,int sta,struct locnRec **theLocn,int *liIndex)
{
	/*	findLiveTie finds the first non-dead tie to station sta in the
		segment pointed to by seg. If seg is a null-pointer, then the search
		for live ties picks up from the last call to findLiveTie. 
		
		The routine lets the calling routine look up seg, in order not
		waste calls to getSR(). liIndex is used to fill the path fields
		when tracing traverses. */
		
	int i;
	static int startTi,endTi;
	LinkRecPtr li;
	struct locnRec *lo;
	
	if(seg) {
		startTi=seg->links+seg->nLocns+seg->nLinks;
		endTi=seg->links+seg->nLocns+seg->nLinks+seg->nTies;
	}
	for(i=startTi;i<endTi;i++) {
		li=getLink(i);
		if(li->from.n<sta) continue;
		if(li->from.n>sta) break;
		lo=getLocn(li->data);
		if(lo->code&DeadEnd) continue;
		*theLocn = lo;
		*liIndex = i;
		startTi=i+1;
		return li;
	}
	*liIndex=0;
	return 0;
}

LinkRecPtr findFixedLink(struct segmentRec *seg,int sta,struct locnRec **theLocn,int *liIndex)
{	
	int i,n;
	LinkRecPtr li;
	struct locnRec *lo;
	struct segmentRec *su0;
	for(i=seg->links+seg->nLocns+seg->nLinks;
		i<seg->links+seg->nLocns+seg->nLinks+seg->nTies;i++) {
		li=getLink(i);
		if(li->from.n==sta) {
			lo=getLocn(li->data);
			n=lo->code&(_Point|_Rel);		/* Mask ->code */
			if((n==Move_to)||(n==Line_to)) {
				*theLocn = lo;
				*liIndex = i;
				return li;
			}
		} else if(li->from.n>sta) break;
	}
	for(i=seg->links+seg->nLocns;
		i<seg->links+seg->nLocns+seg->nLinks;i++) {
		li=getLink(i);
		if(li->from.n==sta) {
			su0=getSR(li->to.segment);
			if(lo=getMainData(su0,li->to.n)) {
				n=lo->code&(_Point|_Rel);	/* Mask ->code */
				if((n==Move_to)||(n==Line_to)) {
					*theLocn = lo;
					*liIndex = i;
					return li;
				}
			}
		} else if(li->from.n>sta) break;
	}
	for(i=seg->links;i<seg->links+seg->nLocns;i++) {
		li=getLink(i);
		if(li->from.n==sta) {
			*theLocn=getLocn(li->data);
			*liIndex = i;
			return li;
		} else if(li->from.n>sta) break;
	}
	*liIndex=0;
	return 0;
}

LinkRecPtr findLiveLink(struct segmentRec *seg,int sta,int *liIndex)
{
	/*	findLiveLink finds the first non-dead tie to station sta in the
		segment pointed to by seg. If seg is a null-pointer, then the search
		for live Link picks up from the last call to findLiveLink. 
		
		The routine lets the calling routine look up seg, in order not
		waste calls to getSR(). liIndex can be used to fill the path
		fields when tracing traverses. */
		
	int i;
	static int startLi,endLi;
	LinkRecPtr li;
	
	if(seg) {
		startLi=seg->links+seg->nLocns;
		endLi=seg->links+seg->nLocns+seg->nLinks;
	}
	for(i=startLi;i<endLi;i++) {
		li=getLink(i);
		if(li->from.n<sta) continue;
		if(li->from.n>sta) break;
		if(li->from.segment&DeadEnd) continue;
		*liIndex = i;
		startLi=i+1;
		return li;
	}
	*liIndex=0;
	return 0;
}

/*-----------------------------------------------------------------------

	traceTraverse() finds the connections through a string, leaving a
	trail behind after the first time.
	
		nLast (or nFirst)
			0		DeadEnd--should never be seen here.
			1		Traverse continues to a link or tie. No trail yet left.
			-1		Traverse continues through contiguous string.
			<-1		Continues through a tie.
			>1000	Continues through a link.
			
	The routine returns a pointer to the next valid string. It can get passed a
	function to be executed for each intervening structure, such as to collect
	lengths, and number of stations. It can be recursive as super-traverses
	will never be unreasonably deep.
	
-------------------------------------------------------------------------------*/

#ifdef DEBUG

void traceSuperString(struct strRec *str,struct segmentRec *seg) 
{
	/* do string specific task. Just demo printing for now. */
	
	/*----------------------------------------------------------------
		This string traces backward and then forward, so is a protype
		for all future tracings. This is the routine that could have
		(*functForLinks)() and (*functForStrs)().
	-----------------------------------------------------------------*/
	
	indexType startStr,startSegment,endStr,entryStr,entrySegment;
	int sta,liIndex=0,startLink,startSta,cnt=0;
	LinkRecPtr li;
	struct segmentRec *su0;
	struct stationRec from;
	Str31 a,b;
	
	entryStr=str->string;
	entrySegment=seg->segment;
	sta=str->last;
	from.segment=0;
	
	if(sta<0) 	/* Unmapped traverse. */
		if((str->nFirst>1)&&(str->nFirst<1000))
			return;			/* is a vertex only */
	
	do {	/* keep crossing, saving the moving front. */
		startStr=str->string;
		startSegment=seg->segment;
		startLink=liIndex;
		startSta=sta;
		if(++cnt>50) {
			_errorAlert(LoopingInLongTraverse,seg->segment,sta);
			cnt=0;
		}
	} while(str=crossTraverse(str,&sta,&seg,&liIndex,&from));
	
	str=getStr(endStr=startStr);

	if(liIndex) {		/* There was a link */
		li=getLink(liIndex);	
		su0=getSR(abs(li->to.segment));
		from.n=li->to.n;
		from.segment=su0->segment;
		/* Print Link: "$123A10->" (to-station) */
		fprintf(OutFile,"%s%s",printFullSta(li->to.n,su0,a),(li->data)?"->":"=");
	} else
		from.n=from.segment=0;
	seg=getSR(startSegment);
	/* Print String, direction is always forward. */
	fprintf(OutFile,"%s-($%d%.4s)",printSta(str->first,a),seg->SECT,seg->desig);
	if(str->string==entryStr)
		fprintf(OutFile,"*");
	sta=str->first;
	while(str=crossTraverse(str,&sta,&seg,&liIndex,&from)) {
	

		if((endStr=str->string)==entryStr)
			fprintf(OutFile,"*");
		if(liIndex) {
			li=getLink(liIndex);
			/* Print Link: "-12->23" (from station, ending string) */
			fprintf(OutFile,"-%s%s%s",printSta(li->from.n,a),(li->data)?"->":"=",printSta(li->to.n,b));
		}
		if(str->nFirst!=-1)	/* non-contiguous. */
			/* Print String, bi-directional and is the !sta end. */
			fprintf(OutFile,"-($%d%.4s)",seg->SECT,seg->desig);
	}
	/* End stuff: */
	
	str=getStr(endStr);
	if(liIndex) {
		li=getLink(liIndex);	
		su0=getSR(abs(li->to.segment));
		/* Print Link: "3->$123A10"  */
		fprintf(OutFile,"-%s%s%s\n",printSta(li->from.n,a),(li->data)?"->":"=",printFullSta(li->to.n,su0,b));
	} else if(str->code&UnMapped)
		fprintf(OutFile,"\n");
	else
		/* Print final station, bi-directional. */
		fprintf(OutFile,"-%s\n",printSta(sta,a));
	return;
}

#endif

void printForwardLink(LinkRecPtr li)
{
	struct segmentRec *seg;
	Str31 a;
	seg=getSR(abs(li->to.segment));
	if(li->data) 
		fprintf(OutFile,"-%d->%s",li->from.n,printFullSta(li->to.n,seg,a));
	else
		fprintf(OutFile,"-%d=%s",li->from.n,printFullSta(li->to.n,seg,a));
	return;
}

/*--------------------------- crossTraverse ----------------------------

	If str is part of a superstring, crossTraverse crosses through the
	adjoining link to the next string. The direction is away from the
	*sta end. *sta is replaced with the farthest known end of the 
	present superstring. This is:
	
		far end of str if there is no connecting link.
		far end of connecting link if it ends in a multi-node.
		near end of connecting string.
		
	*seg is updated to the segment of the connecting string, but not
	to the segment of the far end of intervening link.
		
	Thus *sta and *seg are properly set up for the next call to 
	crossTraverse. Note that seg is not thoroughly safe since it
	is a pointer. Passing index, as liIndex, is absolutely safe in 
	MDL's list-indexed virtual memory environment.
	
------------------------------------------------------------------------*/

struct strRec *crossTraverse(struct strRec *str,int *sta,
		struct segmentRec **seg,int *liIndex,struct stationRec *from)
{

/*======= N.B. from could be replaced by an internal (auto) data
			structure. if(liIndex) { li0=getLink(liIndex; 
			if(li->t0.. = li0->from..; .. } ================*/
	
	int whichEnd,farCnt,farSta,encipher=1;
	struct strRec *str0;
	LinkRecPtr li;
	struct locnRec *lo;
	struct segmentRec *su0;
	short *farPtr;
		
	/*-------------------------------------------------------------------
	 	whichEnd of +1 means forward. For UnMapped strings, .last is
		<0, so can never match a station; .nFirst and .first are the
		valid fields, the same as whichEnd -1. For DeadEnd live strings,
		*sta is the end to be interogated, so direction is reversed: 
	---------------------------------------------------------------------*/
	
	if(str->code&UnMapped) {
		farCnt=str->nFirst;
		if((str->code&HasLocn)&&(farCnt==2)) encipher=0,farCnt--;
		whichEnd=-1;
	} else {
		whichEnd=(str->last==*sta)? -1:1;
		if(str->code&DeadEnd) whichEnd = -whichEnd;
		farCnt=(whichEnd>0)? str->nLast:str->nFirst;
	}
	if((*sta!=str->first)&&(*sta!=str->last)) {
		_errorAlert(UnexpectedError,(*seg)->segment,*sta);
		fprintf(ErrFile,"Station doesn't match traverse %d-%d\n",str->first,str->last);
	}
	/* Flick sta to the other end */
	*sta=(whichEnd>0)? str->last:str->first;
	if(farCnt>1 && farCnt<1000) {
		*liIndex=0;
		return 0;
	}
	/* 	Can't cross as the jct is too alive. (It has more
		than two field observations to or from it.) It 
		will be a parameter in the segment net. There has
		been no intervening linkRec. */
		
	if(farCnt==-1) {	/* Contiguous. An uncomplicated return. */
		*liIndex=0;
		return getStr(str->string+whichEnd);
	}	
	if(farCnt<0) {	/* Tie */
		li=getLink(-(farCnt+1));
		if(str->code&(DeadEnd|UnMapped)) {	
			*liIndex=-(farCnt+1);
			if((li->to.n==from->n)&&(abs(li->to.segment)==from->segment)) {
				/* We are going back along the same path. Get 2nd linkRec. */
				farSta=(whichEnd>0)? str->last:str->first;
				findLiveTie(*seg,farSta,&lo,liIndex);
				if(!(li=findLiveTie(0,farSta,&lo,liIndex)))
					li=findLiveLink(*seg,farSta,liIndex);
			}
			/*	N.B. farCnt indexes the lowest connecting link. Thus, a
				second findLiveTie, working up, will return the one we want.
				Else findLiveLink.  */
		} else
			*liIndex=-(farCnt+1);
		from->n=li->from.n;		/* or *sta */
		from->segment=(*seg)->segment;
		*sta=li->to.n;
								/*==== which station do we return? and why
									doesn't it match the segment? =====*/
									
		if(li->from.segment&StringEnd)
			return 0;			/* The tie is to a (multi-node) junction. */
			
		if(str0=findLiveString(su0=getSR(abs(li->to.segment)),*sta)) {
			farCnt=(str0->last==*sta)? str0->nLast:str0->nFirst;
			if(farCnt>1 && farCnt<1000) {	
				li->from.segment|=StringEnd;	
				return 0;
			}
			return *seg=su0,str0;
		}
		if(str0=findMappedString(su0,*sta)) {
			farCnt=(str0->last==*sta)? str0->nLast:str0->nFirst;
			if((farCnt==1)||(farCnt<0)||(farCnt>1000))
				return *seg=su0,str0;
			else {
				li->from.segment|=StringEnd;
				return 0;
			}
		}
		_errorAlert(UnexpectedError,abs(li->to.segment),*sta);
		return 0;
	}
	
	if(farCnt>1000) {	/* Link */
		li=getLink(*liIndex=farCnt-1000);
		if(str->code&(DeadEnd|UnMapped)) {
			if((li->to.n==from->n)&&(li->to.segment==from->segment)) {
				farSta=(whichEnd>0)? str->last:str->first;
				findLiveLink(*seg,farSta,liIndex);
				if(!(li=findLiveLink(0,farSta,liIndex)))
					li=findLiveTie(*seg,farSta,&lo,liIndex);
			}
		}
		from->n=li->from.n;
		from->segment=(*seg)->segment;
		*sta=li->to.n;
		if(li->from.segment&StringEnd)
			return 0;
		else {
			if(str0=findLiveString(su0=getSR(abs(li->to.segment)),*sta))
				*seg=su0;
			return str0;
		}
	}
	
	/*-------------------------------------------------------------------
		The shorthand paths through consolidated traverses have not been
		set up. .nFirst and .nLast fields == 1 (after Dead Ends have been
		removed) indicate a the continuation of a traverse that, since it
		has no junctions with a loop, should be consolidated.
	---------------------------------------------------------------------*/
	
	if(farCnt==1) {	/* nothing set up. */
	
		/*---------------------------------------------------------------
			1. Is there a contiguous live string?
				UnMapped strings can never test TRUE here.
				If true for a DeadEnd string, it is not the superstring's
				path. Last is rare, so is tested 2nd.
		----------------------------------------------------------------*/
		
		if(str->code&((whichEnd>0)? ContigAtEnd:ContigAtStart)) {
			if(!(str->code&DeadEnd)) {		/* Rare, so tested 2nd. */
				str0=getStr(str->string+whichEnd);
				if(str0->last<0) {
					_errorAlert(BadTraverseTrace,(*seg)->segment,*sta);
					*liIndex=0;
					return 0;
				}
				if(!(str0->code&DeadEnd)) {
					if(whichEnd>0)
						str->nLast=str0->nFirst=-1;
					else
						str->nFirst=str0->nLast=-1;
					from->n=li->from.n;
					from->segment=(*seg)->segment;
					*liIndex=0;
					return str0;
				}
			}
		}
		
		farPtr=(whichEnd>0)? &str->nLast:&str->nFirst;
		farSta=*(farPtr-2);
		
		if(li=findLiveTie(*seg,farSta,&lo,liIndex)) {
			if(encipher)
				*farPtr=-((*liIndex)+1);		/* cipher for tie */
			lo->code|=SuperStringMember;
			if(str->code&(DeadEnd|UnMapped)) {	
				if((li->to.n==from->n)&&(abs(li->to.segment)==from->segment)) {
					if(li=findLiveTie(0,farSta,&lo,liIndex))
						lo->code|=SuperStringMember;
					else
						li=findLiveLink(*seg,farSta,liIndex);
				}
				/*	N.B. farCnt points to the lowest link index.  */
			} 
			from->n=li->from.n;
			from->segment=(*seg)->segment;
			*sta=li->to.n;
			if(li->from.segment&StringEnd)	
				return 0;	
			if(str0=findLiveString(su0=getSR(abs(li->to.segment)),*sta)) {
/*=====		if(str0->string==str->string)
					_errorAlert(SameString,abs(li->to.segment),li->to.n); */
				farCnt=(str0->last==*sta)? str0->nLast:str0->nFirst;
				if(farCnt>1 && farCnt<1000) {	
					li->from.segment|=StringEnd;	
					return 0;
				}
				return *seg=su0,str0;
			}
			if(str0=findMappedString(su0,*sta)) {
				farCnt=(str0->last==*sta)? str0->nLast:str0->nFirst;
				if((farCnt==1)||(farCnt<0)||(farCnt>1000))
					return *seg=su0,str0;
				else {
					li->from.segment|=StringEnd;	
					return 0;
				}
			}
			_errorAlert(UnexpectedError,abs(li->to.segment),*sta);
			return 0;
		}
		if(li=findLiveLink(*seg,farSta,liIndex)) {
			*farPtr= (*liIndex)+1000;	/* cipher for link */
			if(str->code&(DeadEnd|UnMapped)) {
				/* There wil be a second link or tie. */
				if((li->to.n==from->n)&&(abs(li->to.segment)==from->segment)) {
					if(!(li=findLiveLink(0,farSta,liIndex)))
						li=findLiveTie(*seg,farSta,&lo,liIndex);
				}
				/*	N.B. farCnt points to the lowest link index.  */
			} 
			from->n=li->from.n;
			from->segment=(*seg)->segment;
			*sta=li->to.n;
			if(li->from.segment&StringEnd)	
				return 0;					
			str0=findLiveString(su0=getSR(li->to.segment),*sta);	
			farCnt=(str0->last==*sta)? str0->nLast:str0->nFirst;
			if(farCnt>1 && farCnt<1000) {	
				li->from.segment|=StringEnd;	
				return 0;
			}
			return *seg=su0,str0;
		}
	}
		
/*	if(str->nLast==0)
		errorAlert(UnexpectedError); */
	return 0;
}

#ifdef MAC
void *initializeLink(LinkRecPtr li,int flag,void *)
#else
void *initializeLink(LinkRecPtr li,int flag,void *ptr)
#endif
{
	struct locnRec *lo;
	
	/*	Links (Aliases) require no action. */
	if(li->data && !(li->from.segment&Link)) {
		lo=getLocn(li->data);
		lo->code|=SuperStringMember;
		switch (flag) {
			case 0:
				lo->code|=StringBegin;
				break;
			case 4:
				lo->code|=StringEnd;
				break;
			default:
				break;
		}
	}
	return 0;
}

#ifdef MAC
void *initializeTraverse(struct strRec *str,struct segmentRec *,int,int,void *)
#else
void *initializeTraverse(struct strRec *str,struct segmentRec *seg,int x,int y,void *z)
#endif
{
	str->code|=SuperStringMember;
/*=====
	switch (flag) {
		case 0:
			str->code|=StringBegin;
			break;
		case 4:
			str->code|=StringEnd;
			break;
		default:
			break;
	}
======*/
	return 0;
}

/*----------------------------------------------------------------------
	This routine traces backward and then forward through a series of
	connected traverses, so long as the vertex order is 2. It thus
	trace through the entire length of a COMPLETE TRAVERSE. This is 
	the master routine for doing all--or nearly all--edge-related
	tasks. The routines, fLink and fTraverse, passed to it and
	a data structure pointed to by pB, are called at stages 
	throught the trace. 
	
	Code indicates what stage we are at.
	
	For links:
	0	Traverse begins with link, to-station->from-station.
	2	Links is in mid-traverse, from-station->to->station.
	4	Traverse ends with link, from-station->to->station.
	
	For stringRecs
	0	Traverse begins with string.
	1
	2	Contiguous mid-traverse string.
	3
	4	Traverse ends with string.
	5	String before end link.

	Note on synonyms used so far:
	
	A TRAVERSE is a set of contiguous shots between two vertices within 
	a single segment. They are represented by strRecs.
	
			They have been called:	strings
									traverses
	
	COMPLETE TRAVERSEs are the series of shots that join vertices of 
	order 3 or higher. They may be made up of a single strRec, a single
	tie, or any number of these, in any combination.
	
			They have been called:	strings
									traverses
									super-strings
									long traverses
									consolodated traverses.
		
-----------------------------------------------------------------------*/

void *traceTraverse(struct strRec *str,struct segmentRec *seg,
		void *(*fLink)(LinkRecPtr,int,void *), 
		void *(*fTraverse)(struct strRec *,struct segmentRec *,int,int,void *),void *pB) 
{
		
	indexType startStr,startSegment,endStr;
	int sta,liIndex=0,startLink,startSta,cnt=0;
	LinkRecPtr li;
	struct stationRec from;
	
	from.segment=0;
	
	if(str->last<0) {	/* Unmapped traverse. */
		if((str->nFirst>1)&&(str->nFirst<1000)) {
			str->code&= ~SuperStringEntry;
			return 0;			/* is a vertex only */
		}
		sta=str->first;
	} else if(str->code&DeadEnd) 
		sta=(str->nFirst)? str->first:str->last;
	else
		sta=str->last;
	
	do {	/* keep crossing, saving the moving front. */
		startStr=str->string;
		startSegment=seg->segment;
		startLink=liIndex;
		startSta=sta;
		if(++cnt>50) {
			_errorAlert(LoopingInLongTraverse,seg->segment,sta);
			cnt=0;
		}
	} while(str=crossTraverse(str,&sta,&seg,&liIndex,&from));

	str=getStr(endStr=startStr);
	seg=getSR(startSegment);
/*	sta=str->first; */
	
	if(liIndex) {	/* Traverse begins with a link. */
		li=getLink(liIndex);
		from.n=li->to.n;
		from.segment=abs(li->to.segment);
		sta=li->from.n;
		if(fLink) fLink(li,0,pB);
		if(fTraverse) fTraverse(str,seg,sta,1,pB);
	} else {
		from.n=from.segment=0;
		if(fTraverse) fTraverse(str,seg,sta,0,pB);
	}

	while(str=crossTraverse(str,&sta,&seg,&liIndex,&from)) {
		endStr=str->string;
		if(liIndex) {
			if(fLink) fLink(getLink(liIndex),2,pB);
			if(fTraverse) fTraverse(str,seg,sta,3,pB);
		} else 
			if(fTraverse) fTraverse(str,seg,sta,2,pB);
	}
	/* End stuff: */
	
	str=getStr(endStr);
	if(liIndex) {
		if(fTraverse) fTraverse(str,seg,sta,5,pB);
		if(fLink) fLink(getLink(liIndex),4,pB);
	} else
		if(fTraverse) fTraverse(str,seg,sta,4,pB);
	return 0;
}

int reportLongTraverses(void)
{
	int cnt,i,j;
	struct segmentRec *seg,*su0;
	struct strRec *str;
	LinkRecPtr li;
	struct locnRec *lo;
	Str31 a,b;
	
	fprintf(OutFile,"\nTraverses\n");
	for(cnt=i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		
		for(j=seg->strings;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);
			if(str->code&SuperStringEntry) {
				fprintf(OutFile,"%-4d",cnt++);
				traceTraverse(str,seg,printLink,printTraverse,&i);
			}
		}
		seg=getSR(i);
		for(j=seg->links+seg->nLocns+seg->nLinks;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			li=getLink(j);
			if(li->to.segment<0) continue;
			lo=getLocn(li->data);
			if(lo->code&(SuperStringMember|DeadEnd)) continue;
			su0=getSR(li->to.segment);
			fprintf(OutFile,"%-4d",cnt++);
			fprintf(OutFile,"%s->%s\n",printFullSta(li->from.n,seg,a),printFullSta(li->to.n,su0,b));
		}
	}
	return cnt;
}

void *makeTraLink(LinkRecPtr li,int code,void *lTra) 
{
	struct locnRec *lo;
	
	lo=(li->data && !(li->from.segment&Link))?getLocn(li->data):0;
	if(code==0) {	/* First link of traverse */
		if(lo) {
			((TraRecPtr)lTra)->fromVertex=findRow(abs(li->to.segment),li->to.n);
			((TraRecPtr)lTra)->traLength=((c_float)lo->length)/10;
			((TraRecPtr)lTra)->effLength=
				((TraRecPtr)lTra)->traLength*((TraRecPtr)lTra)->traLength;
			((TraRecPtr)lTra)->nLegs=1;
			if(!(((TraRecPtr)lTra)->code&UnProcessed)) {
				if(lo->code&_Rel) {
					if(li->to.segment<0) {
						((TraRecPtr)lTra)->netLength = lo->data.cart;
					} else {
						((TraRecPtr)lTra)->netLength.x = -lo->data.cart.x;
						((TraRecPtr)lTra)->netLength.y = -lo->data.cart.y;
						((TraRecPtr)lTra)->netLength.z = -lo->data.cart.z;
					}
				} else {
					((TraRecPtr)lTra)->code|=UnProcessed;
					((TraRecPtr)lTra)->netLength.x
						=((TraRecPtr)lTra)->netLength.y
						=((TraRecPtr)lTra)->netLength.z=0;
				}
			}
		}
		return lTra;
	} else if(code==4) 		/* Last link of traverse */
		((TraRecPtr)lTra)->toVertex=findRow(abs(li->to.segment),li->to.n);
	if(!(((TraRecPtr)lTra)->code&UnProcessed)) {
		if(lo) {
			if(lo->code&_Rel) {
				((TraRecPtr)lTra)->traLength+=((c_float)lo->length)/10;
				((TraRecPtr)lTra)->effLength+=((c_float)lo->length)*((c_float)lo->length)/100;
				((TraRecPtr)lTra)->nLegs++;
				if(li->to.segment>0) {
					((TraRecPtr)lTra)->netLength.x += lo->data.cart.x;
					((TraRecPtr)lTra)->netLength.y += lo->data.cart.y;
					((TraRecPtr)lTra)->netLength.z += lo->data.cart.z;
				} else {
					((TraRecPtr)lTra)->netLength.x -= lo->data.cart.x;
					((TraRecPtr)lTra)->netLength.y -= lo->data.cart.y;
					((TraRecPtr)lTra)->netLength.z -= lo->data.cart.z;
				}
			} else {
				((TraRecPtr)lTra)->code|=UnProcessed;
				((TraRecPtr)lTra)->netLength.x
					=((TraRecPtr)lTra)->netLength.y
					=((TraRecPtr)lTra)->netLength.z = 0;
			}
		}
	}
	return lTra;
}

void *adjustLink(LinkRecPtr li,int code,void *locn) 
{
	struct locnRec *lo;
	c_float dx,dy,dz,length;

	if(li->data && !(li->from.segment&Link)) {	/* Tie */
		lo=getLocn(li->data);
		length=lo->length/10.0;
		dx=((struct c_locn *)locn)[1].x*length;
		dy=((struct c_locn *)locn)[1].y*length;
		dz=((struct c_locn *)locn)[1].z*length;
		/*====== No -> code mask in the following. =====*/
		if((code==0)? li->to.segment>0:li->to.segment<0) {
			lo[-1].data.cart.x=((struct c_locn *)locn)->x-lo->data.cart.x+dx;
			lo[-1].data.cart.y=((struct c_locn *)locn)->y-lo->data.cart.y+dy;
			lo[-1].data.cart.z=((struct c_locn *)locn)->z-lo->data.cart.z+dz;
			lo->data.cart.x=((struct c_locn *)locn)->x;
			lo->data.cart.y=((struct c_locn *)locn)->y;
			lo->data.cart.z=((struct c_locn *)locn)->z;
			((struct c_locn *)locn)->x=lo[-1].data.cart.x;
			((struct c_locn *)locn)->y=lo[-1].data.cart.y;
			((struct c_locn *)locn)->z=lo[-1].data.cart.z;
		} else {
			lo[-1].data.cart.x=((struct c_locn *)locn)->x;
			lo[-1].data.cart.y=((struct c_locn *)locn)->y;
			lo[-1].data.cart.z=((struct c_locn *)locn)->z;
			((struct c_locn *)locn)->x += (lo->data.cart.x+dx);
			((struct c_locn *)locn)->y += (lo->data.cart.y+dy);
			((struct c_locn *)locn)->z += (lo->data.cart.z+dz);
			lo->data.cart.x=((struct c_locn *)locn)->x;
			lo->data.cart.y=((struct c_locn *)locn)->y;
			lo->data.cart.z=((struct c_locn *)locn)->z;
		}
		lo[-1].code=Move_to;
		lo->code&=~_Rel;
	}
	return locn;
}

void *plotLink(LinkRecPtr li,int code,void *locn) 
{
	struct locnRec *lo;
	int test;
	
	if(li->data && !(li->from.segment&Link)) {
		lo=getLocn(li->data);
		test=li->to.segment>0;
		if(code==0) {
			fprintf(OutFile,"START %6.1f %6.1f %6.1f\n",((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
			test = !test;
		}
		if(test) 
			fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",
				((struct c_locn *)locn)->x += lo->data.cart.x,
				((struct c_locn *)locn)->y += lo->data.cart.y,
				((struct c_locn *)locn)->z += lo->data.cart.z);
		else 
			fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",
				((struct c_locn *)locn)->x -= lo->data.cart.x,
				((struct c_locn *)locn)->y -= lo->data.cart.y,
				((struct c_locn *)locn)->z -= lo->data.cart.z);
	}
	return locn;
}

void *makeTraTraverse(struct strRec *str,struct segmentRec *seg,int sta,
	int code,void *lTra) 
{
	static int lastSegment;
	int i,n,len=0;
	struct locnRec *lo;
	c_float x=0,y=0,z=0,len2=0;
	
	if(code==0) {	/* First string of traverse */
		((TraRecPtr)lTra)->fromVertex=findRow(seg->segment,sta);
		((TraRecPtr)lTra)->nLegs=
			((TraRecPtr)lTra)->traLength=
			((TraRecPtr)lTra)->effLength=0;
	} else if(code==4) {
		((TraRecPtr)lTra)->toVertex=findRow(lastSegment,sta);
		return lTra;
	} else if(code==5)
		return lTra;
	
	lastSegment=seg->segment;
	if(str->code&(UnMapped|DeadEnd)) 
		return lTra;
	((TraRecPtr)lTra)->nLegs+=(n=str->last-str->first);
	lo=getMainData(seg,str->first);
	if(!((lo+1)->code&_Rel)) {
		((TraRecPtr)lTra)->code|=UnProcessed;
		((TraRecPtr)lTra)->netLength.x
			=((TraRecPtr)lTra)->netLength.y
			=((TraRecPtr)lTra)->netLength.z=0;
	} else if(!(((TraRecPtr)lTra)->code&UnProcessed)) {
		for(i=0;i<n;i++) {
			len+=(++lo)->length;
			len2+=((c_float)lo->length)*((c_float)lo->length);
			x+=(lo)->data.cart.x;
			y+=(lo)->data.cart.y;
			z+=(lo)->data.cart.z;
		}
		str->length=len;
		((TraRecPtr)lTra)->traLength+=((c_float)str->length)/10;
		((TraRecPtr)lTra)->effLength+=len2/100;
		if(sta==str->first) {
			((TraRecPtr)lTra)->netLength.x += x;
			((TraRecPtr)lTra)->netLength.y += y;
			((TraRecPtr)lTra)->netLength.z += z;
		} else {
			((TraRecPtr)lTra)->netLength.x -= x;
			((TraRecPtr)lTra)->netLength.y -= y;
			((TraRecPtr)lTra)->netLength.z -= z;
		}
	}
	return lTra;
}

void *plotTraverse(struct strRec *str,struct segmentRec *seg,int sta,
	int code,void *locn)
{
	int i,n;
	struct locnRec *lo;
	if((code>3)||(str->code&(UnMapped|DeadEnd)))
		return locn;
	lo=getMainData(seg,sta);
	n=str->last-str->first;
	if(code==0)
		fprintf(OutFile,"START %6.1f %6.1f %6.1f\n",
				((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
	if(str->first==sta) {
		for(i=0;i<n;i++) {
			lo++;
			fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",
				((struct c_locn *)locn)->x+=lo->data.cart.x,
				((struct c_locn *)locn)->y+=lo->data.cart.y,
				((struct c_locn *)locn)->z+=lo->data.cart.z);
		}
	} else {
		for(i=0;i<n;i++) {
			fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",
				((struct c_locn *)locn)->x-=lo->data.cart.x,
				((struct c_locn *)locn)->y-=lo->data.cart.y,
				((struct c_locn *)locn)->z-=lo->data.cart.z);
			lo--;
		}
	}
	return locn;
}

void *printTraverseCart(struct strRec *str,struct segmentRec *seg,int sta,int code,void *locn)
{
	int i,n;
	struct locnRec *lo;
	Str31 a;
		
	if(code>3)
		return locn;
	if((str->code&(UnMapped|DeadEnd))&&code>1) {
		fprintf(OutFile,"%s: %6.1f %6.1f %6.1f\n",printFullSta(sta,seg,a),((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
		return locn;
	}
	lo=getMainData(seg,sta);
	n=str->last-str->first;
	if(code==0)
		fprintf(OutFile,"%s","\n");
	if(code<2)
		fprintf(OutFile,"%s: %6.1f %6.1f %6.1f\n",printFullSta(sta,seg,a),((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
	if(str->first==sta) {
		for(i=0;i<n;i++) {
			lo++;
			fprintf(OutFile,"%s: %6.1f %6.1f %6.1f\n",printDesigSta(sta+i+1,seg,a),
				((struct c_locn *)locn)->x+=lo->data.cart.x,
				((struct c_locn *)locn)->y+=lo->data.cart.y,
				((struct c_locn *)locn)->z+=lo->data.cart.z);
		}
	} else {
		for(i=0;i<n;i++) {
			fprintf(OutFile,"%s: %6.1f %6.1f %6.1f\n",printDesigSta(sta-i-1,seg,a),
				((struct c_locn *)locn)->x-=lo->data.cart.x,
				((struct c_locn *)locn)->y-=lo->data.cart.y,
				((struct c_locn *)locn)->z-=lo->data.cart.z);
			lo--;
		}
	}
	return locn;
}


void *adjustTraverse(struct strRec *str,struct segmentRec *seg,int sta,
	int place,void *locn)
{
	int i,n;
	struct locnRec *data;
	c_float length;
	struct c_locn original;
	struct c_locn temp;
	
	if(place>3)	/* if 4 or 5, this is an ending str and has already been treated. */
		;
	else if(str->code&UnMapped) {
		str->code|=PartiallyFixed;
		seg->flags|=PartiallyFixed;
		FixArray[seg->segment]=PartiallyFixed;
		return locn;
	} else {
		/* 	The mapped string could be a half DeadEnd, with sta being
			the live end. It gets fixed anyway, even though its not
			necessary for building the net right now.
		*/
		original=*((struct c_locn *)locn);

		str->code|=PartiallyFixed;
		seg->flags|=PartiallyFixed;
		FixArray[seg->segment]=PartiallyFixed;
		data=getMainData(seg,sta);
		n=str->last-str->first;
		if(str->first==sta) {	/* Going through in forward direction */
			if(!data->code) {
				/* First station of a sub-segment. */
				data->code=Move_to;
				data->data.cart.x=((struct c_locn *)locn)->x;
				data->data.cart.y=((struct c_locn *)locn)->y;
				data->data.cart.z=((struct c_locn *)locn)->z;
			} else if(str->code&ContigAtStart) 
				/* 	must fix all of subsurvey in this preceeding dead end, since
						we are about to loose the data for the str->first station. */
				fixStump(str,data,((struct c_locn *)locn));

			for(i=0;i<n;i++) {
				/* 	This junction is mid-survey. It is fixed by the 
					adjacent loop. */
				data++;
				data->code&=(~_Rel);
				length=data->length/10.0;
				((struct c_locn *)locn)->x+=(data->data.cart.x+((struct c_locn *)locn)[1].x*length);
				((struct c_locn *)locn)->y+=(data->data.cart.y+((struct c_locn *)locn)[1].y*length);
				((struct c_locn *)locn)->z+=(data->data.cart.z+((struct c_locn *)locn)[1].z*length);
				data->data.cart.x=((struct c_locn *)locn)->x;
				data->data.cart.y=((struct c_locn *)locn)->y;
				data->data.cart.z=((struct c_locn *)locn)->z;
			}
		} else {
			for(i=0;i<n;i++) {
				data->code&=(~_Rel);
				length=data->length/10.0;
				temp=*((struct c_locn *)locn);
				((struct c_locn *)locn)->x-=(data->data.cart.x-((struct c_locn *)locn)[1].x*length);
				((struct c_locn *)locn)->y-=(data->data.cart.y-((struct c_locn *)locn)[1].y*length);
				((struct c_locn *)locn)->z-=(data->data.cart.z-((struct c_locn *)locn)[1].z*length);
				data->data.cart.x=temp.x;
				data->data.cart.y=temp.y;
				data->data.cart.z=temp.z;
				data--;
			}
			if(!data->code) {
				/* First station of a sub-segment. */
				data->code=Move_to;
				data->data.cart.x=((struct c_locn *)locn)->x;
				data->data.cart.y=((struct c_locn *)locn)->y;
				data->data.cart.z=((struct c_locn *)locn)->z;
			} else if(str->code&ContigAtStart)
				fixStump(str,data,((struct c_locn *)locn));
		}
		if(str->code&DeadEnd) {
			*((struct c_locn *)locn)=original;	/* reset locn to live end of traverse */
			/* Calculate string length--it's not done elsewhere. */
			data=getMainData(seg,str->first);
			for(str->length=i=0;i<n;i++,data++) 
				str->length+=data->length;
		}
	}
	return locn;
}

static void fixStump(struct strRec *str,struct locnRec *theData,void *locn)
{
	c_float dx=0,dy=0,dz=0;
	int n,i,strLength;
	struct strRec *str0;
	
	str0=getStr(str->string-1);
	if(str0->code&DeadEnd) {
		/* 
			There is a (dead-end) tree rooted to str, which is a strRec
			in the closure network. locn is str->first's coordinates and
			str->first is still a Line_to_Rel.
	
			We'll find this the subsegment's beginning by interogating
			the main data, since we have to accumulate the offset anyway.
		*/
		for(n=str->first;(theData->code&(_Point|_Rel))==Line_to_rel;theData--,n--) {
			dx+=theData->data.cart.x;		
			dy+=theData->data.cart.y;		
			dz+=theData->data.cart.z;
		}
	
		theData->code=Move_to;
		theData->data.cart.x=dx=((struct c_locn *)locn)->x-dx;
		theData->data.cart.y=dy=((struct c_locn *)locn)->y-dy;
		theData->data.cart.z=dz=((struct c_locn *)locn)->z-dz;
					
		while(str0->first > n) {
			str0=getStr(str0->string-1);
		} 
		str0->code|=FixedFlag;
		
		/* Fix the preceeding strings. */

		for(i=n,strLength=0;i<str->first;i++) {
			(++theData)->code = Line_to;
			strLength+=theData->length;
			if(i==str0->last) {
				str0->length=strLength;
				strLength=0;
				str0=getStr(str0->string+1);
				str0->code |= FixedFlag;
			} 
			dx=theData->data.cart.x+=dx;
			dy=theData->data.cart.y+=dy;
			dz=theData->data.cart.z+=dz;
		}
		str0->length=strLength;
	}
	return;
}

void *printLinkCart(LinkRecPtr li,int code,void *locn)
{
	char *ligand;
	int test;
	struct locnRec *lo;
	Str31 a;

	if(li->data && !(li->from.segment&Link)) {
		lo=getLocn(li->data);
		ligand="->";
		test=li->to.segment>0;
	} else {
		lo=0;
		ligand="=";
	}
	if(code==0) {	/* Traverse begins with link, to-station->from-station */
		fprintf(OutFile,"\n%s%s",printFullSta(li->to.n,getSR(abs(li->to.segment)),a),ligand);
		test=!test;
		if(lo) 
			fprintf(OutFile,": %6.1f %6.1f %6.1f\n",((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
	}
	if(lo) {
		if(test) {
			((struct c_locn *)locn)->x += lo->data.cart.x;
			((struct c_locn *)locn)->y += lo->data.cart.y;
			((struct c_locn *)locn)->z += lo->data.cart.z;
		} else {
			((struct c_locn *)locn)->x -= lo->data.cart.x;
			((struct c_locn *)locn)->y -= lo->data.cart.y;
			((struct c_locn *)locn)->z -= lo->data.cart.z;
		}
	}
	if(code==2 || code==4) {
		fprintf(OutFile,"%s%s",ligand,printFullSta(li->to.n,getSR(abs(li->to.segment)),a));
		if(lo)
			fprintf(OutFile,": %6.1f %6.1f %6.1f\n",((struct c_locn *)locn)->x,((struct c_locn *)locn)->y,((struct c_locn *)locn)->z);
		else
			fprintf(OutFile,"\n");
	}
	return locn;
}

#ifdef MAC
void *printLink(LinkRecPtr li,int code,void *)
#else
void *printLink(LinkRecPtr li,int code,void *v)
#endif
{
	Str31 a,b;
	char *ligand;
	ligand=(li->data)? "->":"=";
	if(code==0) 
		fprintf(OutFile,"%s%s",printFullSta(li->to.n,getSR(abs(li->to.segment)),a),ligand);
	else if((code==2)||(code==4))
		fprintf(OutFile,"-%s%s%s",printSta(li->from.n,a),ligand,
			printFullSta(li->to.n,getSR(abs(li->to.segment)),b));
	if(code==4)
		fprintf(OutFile,"\n");
	return 0;
}

#ifdef MAC
void *printTraverse(struct strRec *,struct segmentRec *seg,int sta,int code,void *)
#else
void *printTraverse(struct strRec *str,struct segmentRec *seg,int sta,int code,void *v)
#endif
{
	Str31 a;
	if(code==0 || code==1)
		fprintf(OutFile,"%s",printFullSta(sta,seg,a));
	else if(code==4)
		fprintf(OutFile,"-%s\n",printSta(sta,a));
	return 0;
}

int findRow(int segment,int sta) {
	int i;
	struct rowRec *row;
	struct segmentRec *seg;
	seg=getSR(segment);
	for(i=seg->rows;i<seg->rows+seg->nJcts;i++) {
		row=getRow(i);
		if(row->this.n==sta)
			return i;
	}
	_errorAlert(UnexpectedError,seg->segment,sta);
	return 0;
}

struct colRec *findCol(struct rowRec *row,indexType traNumber)
{
	int i;
	struct colRec *col;
	
	for(i=row->cols;i<row->cols+row->nCols;i++) {
		col=getCol(i);
		if(col->traverse==traNumber)
			return col;
	}
	errorAlert(UnexpectedError);
	return 0;
}

LinkRecPtr findLocn(struct segmentRec *seg, int sta) {
	int i;
	LinkRecPtr li;
	for(i=seg->links;i<seg->links+seg->nLocns;i++) {
		li=getLink(i);
		if(li->from.n==sta) 
			return li;
		else if(li->from.n>sta) break;
	}
	return 0;
}

static int fixTies(struct segmentRec *seg,struct cartData *locn,int sta)
{
	int i,n,numToFix=0;
	LinkRecPtr li;
	struct locnRec *lo;
	struct strRec *str;
	
	for(i=seg->links+seg->nLocns+seg->nLinks;
		i<seg->links+seg->nLocns+seg->nLinks+seg->nTies;
		i++) {
		li = getLink(i);
		lo = getLocn(li->data);
		if(li->from.n==sta) {
			if(lo->code&_Rel) {		/* link not previously fixed. */
				lo[0].code&=~_Rel;
				lo[-1].code=Move_to;
				if(li->to.segment<0) {
					lo[-1].data.cart.x = locn->x-lo[0].data.cart.x;
					lo[-1].data.cart.y = locn->y-lo[0].data.cart.y;
					lo[-1].data.cart.z = locn->z-lo[0].data.cart.z;
					lo[0].data.cart = *locn;
				} else {
					lo[-1].data.cart = *locn;
					lo[0].data.cart.x += lo[-1].data.cart.x;
					lo[0].data.cart.y += lo[-1].data.cart.y;
					lo[0].data.cart.z += lo[-1].data.cart.z;
				}

				n=abs(li->to.segment);
				if(n!=seg->segment) {
					if(FixArray[n]==0 || (FixArray[n]>0 && !(FixArray[n]&FixedFlag))) 
						FixArray[n] = -i;
				}
			}
		} else if(lo->code&_Rel) {
			numToFix++;
			if(li->from.n>sta)
				break;
		}
	}
	if(numToFix) {
		seg->flags |= PartiallyFixed;	/* Have to check strings */
		return 0;
	}

	for(i=seg->strings,numToFix=0;i<seg->strings+seg->nStrings;i++) {
		str=getStr(i);
		if(!str->code&UnMapped) {
			seg->flags |= PartiallyFixed;
			return 0;
		}
	}
	seg->flags|=FixedFlag;
	return 0;
}

void fixNetwork(void)
{
	int i;
	TraRecPtr tra;
	struct rowRec *row0,*row1;
	c_float dx,dy,dz;
	struct c_locn locn[2];
	struct segmentRec *seg;
	struct strRec *str;
	
	for(i=1;i<=LongTraList->current;i++) {
		RotateCursor(i);
		tra=getTra(i);
		if(tra->code&UnProcessed)
			traceTraverse(getStr(tra->entryStr),getSR(tra->entrySegment),
				findLengthLink,findLengthTraverse,tra);
		row0=getRow(tra->fromVertex);
		row1=(tra->toVertex)?getRow(tra->toVertex):row0;
		
		/* Store starting locn. */
		locn[0].x=row0->locn.x;
		locn[0].y=row0->locn.y;
		locn[0].z=row0->locn.z;
		dx=row1->locn.x-locn[0].x;
		dy=row1->locn.y-locn[0].y;
		dz=row1->locn.z-locn[0].z;
		
		/* Calculated and store error per unit length. */
		locn[1].x=(dx-tra->netLength.x)/tra->traLength;
		locn[1].y=(dy-tra->netLength.y)/tra->traLength;
		locn[1].z=(dz-tra->netLength.z)/tra->traLength;
		if(tra->entryStr>0)
			traceTraverse(getStr(tra->entryStr),getSR(tra->entrySegment),
					adjustLink,adjustTraverse,locn);
		else 	/* Just a single tie. */
			adjustLink(getLink(-tra->entryStr),1,locn);
#if 0			
		/*	Added March 5, 1993 to fix problem of unmapped nodes not being fixed. */
		str=findString(seg=getSR(row0->this.segment),row0->this.n);
		if(str->code&UnMapped) 
			FixArray[row0->this.segment]=fixTies(seg,&row0->locn,(int)row0->this.n);
		
		str=findString(seg=getSR(row1->this.segment),row1->this.n);
		if(str->code&UnMapped) 
			FixArray[row1->this.segment]=fixTies(seg,&row1->locn,(int)row1->this.n);
#endif
	}
	/* Added April 23, 1993 to fix March 5 problem again. */
	for(i=1;i<=RowList->current;i++) {
		row0=getRow(i);
		str=findString(seg=getSR(row0->this.segment),row0->this.n);
		if(str->code&UnMapped) {
			FixArray[row0->this.segment]=fixTies(seg,&row0->locn,(int)row0->this.n);
		}
	}
	return;
}

void fixTree(void)
{
	int i,flag,floating;
	struct segmentRec *seg;
	LinkRecPtr theLink;
	struct locnRec *theLocn;

	for(i=1;i<=SegList->current;i++) {
		if(FixArray[i]==PartiallyFixed) {
			seg=getSR(i);
			FixArray[i]=fixSegment(seg,0,0);
		}
	}
	do {
		for(i=1,floating=flag=0;i<=SegList->current;i++) {
			if(FixArray[i]<0) {
				theLink=getLink(-FixArray[i]);
				theLocn=(theLink->from.segment & Link) ? 
					(getData(theLink->data)) : (getLocn(theLink->data));
				if(theLink->to.segment<0) theLocn--;	/* Locn wanted is in other segment. */
				flag|=(FixArray[i]=fixSegment(getSR(i),theLocn,theLink->to.n));
/*				fprintf(OutFile,"Fixed su %d, Result: %d\n",i,FixArray[i]); */
			} else if(FixArray[i]==0) {
/*				flag|=(FixArray[i]=fixSegment(getSR(i),0,0)); */
/*				if(!FixArray[i]) */
					floating++;
			}
		}
	} while(flag&FixedFlag);	/* convenient flag, signals sillCanFix. */
	if(floating) {
		do	{
			floating=0;
			for(i=1,flag=0;i<=SegList->current;i++) {
				if(FixArray[i]<0) {
					theLink=getLink(-FixArray[i]);
					theLocn=(theLink->from.segment & Link) ? 
						(getData(theLink->data)) : (getLocn(theLink->data));
					if(theLink->to.segment<0) theLocn--;	/* Locn wanted is in other segment. */
					flag|=(FixArray[i]=fixSegment(getSR(i),theLocn,theLink->to.n));
/*					fprintf(OutFile,"2Fixed su %d, Result: %d\n",i,FixArray[i]); */
				} else if(FixArray[i]==0) {
					flag|=(FixArray[i]=fixSegment(getSR(i),0,0));
/*					fprintf(OutFile,"0-Fixed su %d, Result: %d\n",i,FixArray[i]); */
					if(FixArray[i]!=FixedFlag) {
						_errorAlert(MayFloat*100,-i,0);
						floating++;
					}
				}
			}
		} while(flag&FixedFlag);
	}
	if(floating) { 
		do {
			for(i=1,flag=0;i<=SegList->current;i++) {
				if(!(FixArray[i]&FixedFlag))
					flag|=(FixArray[i]=fixSegment(getSR(i),0,0));
			}
		} while(flag&FixedFlag);
	}
	return;
}

void removeColumn(TraRecPtr tra)
{
	struct rowRec *row,*row0;
	struct colRec *col,*col0;

	if(tra->fromVertex==tra->toVertex) {
		tra->toVertex=0;	/* This flags a closed loop. */
		if(tra->fromVertex>RowList->current)
			tra->fromVertex=0;
		row=getRow(tra->fromVertex);
		if((row->nCols-=2)<2)
			_errorAlert(NewDeadEnd,row->this.segment,row->this.n);
	} else {
		row=getRow(tra->fromVertex);
		row0=getRow(tra->toVertex);
		col=getCol(row->cols);
		col0=getCol(row0->cols);
		row->cols++;
		row0->cols++;
		col->col=-col->col;
		col0->col=-col0->col;	/* Signals an unprocessed traverse */
		col->traverse=col0->traverse=tra->self;
	}
	return;
}

void removeEEColumn(TraRecPtr tra)
{
	struct eeRowRec *row,*row0;
	struct eeColRec *col,*col0;

	if(tra->fromVertex==tra->toVertex) {
		tra->toVertex=0;						/* This flags a closed loop. */
		if(tra->fromVertex>RowList->current)
			tra->fromVertex=0;
		row=getEeRow(tra->fromVertex);
		if((row->nCols-=2)<2)
			_errorAlert(NewDeadEnd,row->this.segment,row->this.n);
	} else {
		row=getEeRow(tra->fromVertex);
		row0=getEeRow(tra->toVertex);
		col=getEeCol(row->cols);
		col0=getEeCol(row0->cols);
		row->cols++;
		row0->cols++;
		col->farEnd=-col->farEnd;
		col0->farEnd=-col0->farEnd;				/* Signals an unprocessed traverse */
		col->traverse=col0->traverse=tra->self;
	}
	return;
}

void *findLengthLink(LinkRecPtr li,int code,void *lTra) 
{
	struct locnRec *lo;
	
	lo=(li->data && !(li->from.segment&Link))?getLocn(li->data):0;
	if(code==0) {	/* First link of traverse */
		if(lo) {
			((TraRecPtr)lTra)->traLength=((c_float)lo->length)/10;
			((TraRecPtr)lTra)->nLegs=1;
			if(lo->code&_Rel) {
				if(li->to.segment<0) {
					((TraRecPtr)lTra)->netLength = lo->data.cart;
				} else {
					((TraRecPtr)lTra)->netLength.x = -lo->data.cart.x;
					((TraRecPtr)lTra)->netLength.y = -lo->data.cart.y;
					((TraRecPtr)lTra)->netLength.z = -lo->data.cart.z;
				}
			}
		}
	} else if(lo) {
		if(lo->code&_Rel) {
			((TraRecPtr)lTra)->traLength+=((c_float)lo->length)/10;
			((TraRecPtr)lTra)->nLegs++;
			if(li->to.segment>0) {
				((TraRecPtr)lTra)->netLength.x += lo->data.cart.x;
				((TraRecPtr)lTra)->netLength.y += lo->data.cart.y;
				((TraRecPtr)lTra)->netLength.z += lo->data.cart.z;
			} else {
				((TraRecPtr)lTra)->netLength.x -= lo->data.cart.x;
				((TraRecPtr)lTra)->netLength.y -= lo->data.cart.y;
				((TraRecPtr)lTra)->netLength.z -= lo->data.cart.z;
			}
		}
	}
	return lTra;
}

void *findLengthTraverse(struct strRec *str,struct segmentRec *seg,int sta,
	int code,void *lTra) 
{
	int i,n,len=0;
	struct locnRec *lo;
	c_float x=0,y=0,z=0;
	
	if(code==0) {	/* First string of traverse */
		((TraRecPtr)lTra)->nLegs=((TraRecPtr)lTra)->traLength=0;
	} else if(code==4) {
		return lTra;
	} else if(code==5)
		return lTra;
	
	if(str->code&(UnMapped|DeadEnd)) 
		return lTra;
	((TraRecPtr)lTra)->nLegs+=(n=str->last-str->first);
	lo=getMainData(seg,str->first);
	if((lo+1)->code&_Rel) {
		for(i=0;i<n;i++) {
			len+=(++lo)->length;
			x+=(lo)->data.cart.x;
			y+=(lo)->data.cart.y;
			z+=(lo)->data.cart.z;
		}
		str->length=len;
		((TraRecPtr)lTra)->traLength+=((c_float)str->length)/10;
		if(sta==str->first) {
			((TraRecPtr)lTra)->netLength.x += x;
			((TraRecPtr)lTra)->netLength.y += y;
			((TraRecPtr)lTra)->netLength.z += z;
		} else {
			((TraRecPtr)lTra)->netLength.x -= x;
			((TraRecPtr)lTra)->netLength.y -= y;
			((TraRecPtr)lTra)->netLength.z -= z;
		}
	}
	return lTra;
}

void removeClosedLoop(TraRecPtr tra)
{
	struct rowRec *row;
	c_float temp;
#ifdef DEBUG
	fprintf(OutFile,"Tra %d is a closed loop\n",LongTraList->current);
#endif
	/*	Traverses beginning and ending at the same same station
		do not contribute to the net. */
	tra->toVertex=0;	/* This flags a closed loop. */
	
	/* Still calculate a weight */
	temp=tra->traLength/tra->nLegs;	/* temp */
	temp*=temp;	/* square it */
	tra->weight.x=tra->weight.y=1.0/(tra->nLegs*(0.1222+temp*22.2784e-4));
	tra->weight.z=1.0/(tra->nLegs*(0.0076+temp*41.8609e-4));
	
	if(tra->fromVertex>RowList->current)
		tra->fromVertex=0;
	row=getRow(tra->fromVertex);
	if((row->nCols-=2)<2)
		_errorAlert(NewDeadEnd,row->this.segment,row->this.n);
}

void addColumn(TraRecPtr tra,short callingCol)
{
	int n;
	struct rowRec *row,*row0;
	struct colRec *col,*col0;
	c_float weightH,weightV,wx,wy,wz;

#if 0
	fprintf(OutFile,"LTraverse %d fr: %d to: %d nLeg: %d Le: %6.1f\n",
		LongTraList->current,tra->fromVertex,tra->toVertex,tra->nLegs,
		tra->traLength);
#endif

	if(tra->fromVertex>RowList->current)
		tra->fromVertex=0;	/* Forces a fatal error in a safe way. */
	if(tra->toVertex>RowList->current)
		tra->toVertex=0;
		
	row=getRow(tra->fromVertex);
	row0=getRow(tra->toVertex);
	row->code|=Processed;
	row0->code|=Processed;
	
	if((n=row->code&Located)!=(row0->code&Located)) {
		/* one is fixed, the other is not. */
		RowList->other++;	/* count of located rows. */
		if(n) {	/* row is the one fixed. */
			row0->code|=Located;
			row0->locn.x=row->locn.x+tra->netLength.x;
			row0->locn.y=row->locn.y+tra->netLength.y;
			row0->locn.z=row->locn.z+tra->netLength.z;
		} else {
			row->code|=Located;
			row->locn.x=row0->locn.x-tra->netLength.x;
			row->locn.y=row0->locn.y-tra->netLength.y;
			row->locn.z=row0->locn.z-tra->netLength.z;
		}
	}
	ColList->other+=2;
	if(CloseFlag==1 || CloseFlag==-1) {
		col=getCol(row->cols);
		col0=getCol(row0->cols);
		row->cols++;
		row0->cols++;
		col->col=tra->toVertex;
		col0->col=tra->fromVertex;
		col->traverse=col0->traverse=tra->self;
	} else {
		if(callingCol>=row->cols && callingCol<(row->cols+row->nCols)) {
			col=getCol(callingCol);
			col0=findCol(row0,tra->self);
		} else {
			col0=getCol(callingCol);
			col=findCol(row,tra->self);
		}
		col->col=tra->toVertex;
		col0->col=tra->fromVertex;
	}
	
		/* weight is a function of mean shot length */
	wx=tra->traLength/tra->nLegs;	/* temp */
	wx*=wx;	/* square it */
	weightH=1.0/(tra->nLegs*(0.1222+wx*22.2784e-4));
	weightV=1.0/(tra->nLegs*(0.0076+wx*41.8609e-4));
#if 0
	weightH=1.0/sqrt(tra->nLegs*(0.1222+wx*22.2784e-4));
	weightV=1.0/sqrt(tra->nLegs*(0.0076+wx*41.8609e-4));
#endif
	/* typical values are 0.05 for x,y and 0.03 for z */
	tra->weight.x = tra->weight.y = weightH;
	tra->weight.z = weightV;
	row->diagonal.x += weightH;
	row->diagonal.y += weightH;
	row->diagonal.z += weightV;
	row0->diagonal.x += weightH;
	row0->diagonal.y += weightH;
	row0->diagonal.z += weightV;
	row->vector.x -= (wx=weightH*tra->netLength.x);
	row->vector.y -= (wy=weightH*tra->netLength.y);
	row->vector.z -= (wz=weightV*tra->netLength.z);
	row0->vector.x += wx;
	row0->vector.y += wy;
	row0->vector.z += wz;
	col->value.x -= weightH;
	col->value.y -= weightH;
	col->value.z -= weightV;
	col0->value.x -= weightH;
	col0->value.y -= weightH;
	col0->value.z -= weightV;
}

#if 0

/*------------------------ Conjugate gradient s/rs------------------*/
/* after Press et al., p80 */

void asub(x,v)
c_float x[],v[];
{
	register int i,m;
	int a1,a2;
	for(i=0;i<N;i++) {
		v[i]=diag[i]*x[i];
		a1=row[i];
		a2=row[i+1];
		if(a1=a2) continue;	/* no off diagonal elements */
		for(m=a1;m<a2-1;m++) /* index columns for this row,
								col[m] is the column number */
			v[i] += off[m]*x[col[m]];
	}
}

void atsub(x,v)
c_float x[],v[];
{
	register int j,m;
	int a1,a2;
	for(j=0;j<N;j++)
		v[j]=diag[j]*x[j];
	for(j=0;j<N;j++) {
		a1=row[j];
		a2=row[j+1];
		if(a1=a2) continue;	/* no off diagonal elements */
		for(m=a1;m<a2-1;m++) 
			v[col[m]] += off[m]*x[j];
	}
}
#endif

#if 0
	/*------------------ Example fLink -----------------------------*/
	void fLink(int liIndex,int flag,void *pB) {
		LinkRecPtr li;
		li=getLink(liIndex);
		switch flag {
			case 0:	/* first in traverse */
				(struct parameters *) pB->firstSta=li->to.n;
				(struct parameters *) pB->firstSu=abs(li->to.segment);
				(struct parameters *) pB->length=li->length;
				(struct parameters *) pB->nLegs=1;
				break;
			case 2:
				(struct parameters *) pB->lastSta=li->to.n;
				(struct parameters *) pB->lastSu=abs(li->to.segment);
			case 1:	/* in mid-traverse */
				(struct parameters *) pB->length+=li->length;
				(struct parameters *) pB->nLegs+=1;
				break;
			default:
				_errorAlert(UnexpectedError,abs(li->to.segment),li->to.n);
				break;
		}
		return;
	}
#endif

#ifdef MAC
#pragma segment loops
#endif

struct eeColRec *findEeCol(struct eeRowRec *row,indexType traNumber,short farNode)
/* 	First tries to match by traverse. If that doesn't work, matches by
	opposite node. */
{
	int i,hit;
	struct eeColRec *col;
	
	for(hit=0,i=row->cols;i<row->cols+row->nCols;i++) {
		col=getEeCol(i);
		if(col->traverse==traNumber)
			return col;
		if(col->farEnd==farNode && !hit)
			hit=i;
	}
	if(!hit)
		errorAlert(UnexpectedError);
	return getEeCol(hit);;
}

int findEeRow(int segment,int sta) {
	int i;
	struct eeRowRec *row;
	struct segmentRec *seg;
	seg=getSR(segment);
	for(i=seg->rows;i<seg->rows+seg->nJcts;i++) {
		row=getEeRow(i);
		if(row->this.n==sta)
			return i;
	}
	_errorAlert(UnexpectedError,seg->segment,sta);
	return 0;
}

void EETransform(void)
{
	int i;
	c_float x,y;
	struct locnRec *locn;

	if(EEFlag) {
		locn=getData(1);
		locn--;
		for(i=1;i<=DataList->current;i++) {
			if(i&BigBlockLo)
				locn++;
			else
				locn=getData(i);
			if(locn->code&Line_to) {
				x=locn->data.cart.x;
				y=locn->data.cart.y;
				x=sqrt(x*x+y*y);
				locn->data.cart.x=(locn->code&EELeftFlagBit)? -x:x;
				locn->data.cart.y=locn->data.cart.z;
			}
		}
		for(i=1;i<=LocnList->current;i++) {
			locn=getLocn(i);
			if((locn->code&(_Point|_Rel))==Line_to_rel) {
				x=locn->data.cart.x;
				y=locn->data.cart.y;
				x=sqrt(x*x+y*y);
				locn->data.cart.x=(locn->code&EELeftFlagBit)? -x:x;
				locn->data.cart.y=locn->data.cart.z;
			}
		}
	}
}

void *makeEETraTraverse(struct strRec *str,struct segmentRec *seg,int sta,
	int code,void *lTra) 
{
	static int lastSegment;
	int n,len=0;
	
	if(code==0) {	/* First string of traverse */
		((TraRecPtr)lTra)->fromVertex=findEeRow(seg->segment,sta);
		((TraRecPtr)lTra)->nLegs=((TraRecPtr)lTra)->traLength=0;
	} else if(code==4) {
		((TraRecPtr)lTra)->toVertex=findEeRow(lastSegment,sta);
		return lTra;
	} else if(code==5)
		return lTra;
	
	lastSegment=seg->segment;
	if(str->code&(UnMapped|DeadEnd)) 
		return lTra;
	((TraRecPtr)lTra)->nLegs+=(n=str->last-str->first);

	return lTra;
}

void *makeEETraLink(LinkRecPtr li,int code,void *lTra) 
{
	struct locnRec *lo;
	
	lo=(li->data && !(li->from.segment&Link))?getLocn(li->data):0;
	if(code==0) {	/* First link of traverse */
		if(lo) {
			((TraRecPtr)lTra)->fromVertex=findEeRow(abs(li->to.segment),li->to.n);
			((TraRecPtr)lTra)->nLegs=1;
		}
		return lTra;
	} else if(code==4) 		/* Last link of traverse */
		((TraRecPtr)lTra)->toVertex=findEeRow(abs(li->to.segment),li->to.n);
		
	((TraRecPtr)lTra)->nLegs++;
	
	return lTra;
}

void removeEEClosedLoop(TraRecPtr tra)
{
	struct eeRowRec *row;

	/*	Traverses beginning and ending at the same same station
		do not contribute to the net. */
	tra->toVertex=0;	/* This flags a closed loop. */
		
	if(tra->fromVertex>EeRowList->current)
		tra->fromVertex=0;
	row=getEeRow(tra->fromVertex);
	row->nVertices-=2;
	if(row->nVertices<=2)
		row->code&=~IsStillNode;
	row->nCols-=2;
/*	if((row->nCols-=2)<2)	*/
/*		_errorAlert(NewDeadEnd,row->this.segment,row->this.n); */
}

void addEEColumn(TraRecPtr tra)
{
	struct eeRowRec *row,*row0;
	struct eeColRec *col,*col0;

	if(tra->fromVertex>EeRowList->current)
		tra->fromVertex=0;		/* Forces a fatal error in a safe way. */
	if(tra->toVertex>EeRowList->current)
		tra->toVertex=0;
		
	row=getEeRow(tra->fromVertex);
	row->code|=IsStillNode;
	col=getEeCol(row->cols);
	row->cols++;
	col->farEnd=tra->toVertex;
	col->traverse=tra->self;
	
	row0=getEeRow(tra->toVertex);
	row0->code|=IsStillNode;
	col0=getEeCol(row0->cols);
	col0->farEnd=tra->fromVertex;
	col0->traverse=tra->self;
	/* Don't remove closed loops yet. Just remove the col cnt */
	row0->cols++;
}

void reportClosedLoop(TraRecPtr tra)
{
	struct eeRowRec *row;
	
	row=getEeRow(tra->fromVertex);
	fprintf(OutFile,"Traverse %d forms a closed loop at ",tra->self);
	printTraverse(0,getSR(row->this.segment),row->this.n,0,0);
	fputc('\n',OutFile);
}

int reportEulerLoops(void)
{
	int nLE,i,j,temp,k,nEulerLoops=0,depth=0;
	struct segmentRec *seg;
	LinkRecPtr li;
	struct locnRec *lo;
	struct strRec *str;
	struct eeRowRec *row,*row0;
	struct eeColRec *col,*col0,*col1;
	TraRecPtr tra;
	Str31 a,b;
	
	nLE=(CloseFlag)?ColList->nLE:2;
	EeRowList=lListAlloc(0,BlockLen,sizeof(struct eeRowRec),nLE,-1);
	EeColList=lListAlloc(0,BlockLen,sizeof(struct eeColRec),2*nLE,-2);
	
	for(EeColList->current=i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		if(seg->nLocns) {
			for(j=seg->links;j<seg->links+seg->nLocns;j++) {
				li=getLink(j);
				if(str=findString(seg,li->from.n))
					str->code|=HasLocn;
				else
					_errorAlert(UnexpectedError,i,li->from.n);
			}
		}
		temp=EeRowList->current+1;
		seg->nJcts=0;
		for(j=seg->strings,row=0;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);

			/*------------------------------------------------------	
				The following criterion detects junctions by looking
				at vertex counts. In this way, rows are assigned to
				junctions in segment/station rank order. 
			-------------------------------------------------------*/
			
			if(str->nFirst>1 && str->nFirst<1000) {
				if(!row || (row->this.n!=str->first)) {
					/* i.e. str->first not in last row.  */
					row=getNextEeRow();
					row->self=EeRowList->current;
					row->cols=EeColList->current;
					if((str->code&HasLocn) && (li=findLocn(seg,str->first))) {
						/* Is this is the traverse with the Fixed Locn? */
						EeColList->current+=row->nCols=row->nVertices=str->nFirst;
						col=getEeCol(row->cols);
						row->code=(HasLocn|FixedFlag);
						row->cols++;
					} else {
						EeColList->current+=row->nCols=row->nVertices=str->nFirst+1;
						row->code=0;
					}
					seg->nJcts++;
					row->this.n=str->first;
					row->this.segment=i;
				}
			}
			if(!(str->code&UnMapped)) {
				if(str->nLast>1 && str->nLast<1000) {
					row=getNextEeRow();
					row->self=EeRowList->current;
					row->cols= EeColList->current;
					if((str->code&HasLocn) && (li=findLocn(seg,str->last))) {
						EeColList->current+=row->nCols=row->nVertices=str->nLast;
						col=getEeCol(row->cols);
						row->code=(HasLocn|FixedFlag);
						row->cols++;
					} else {
						EeColList->current+=row->nCols=row->nVertices=str->nLast+1;
						row->code=0;
					}
					seg->nJcts++;
					row->this.n=str->last;
					row->this.segment=i;
				}
			}
		}
		seg->rows=(seg->nJcts)? temp:0;
	}
	
	if(!EeRowList->current) {
		fputs("\nThere are no loops\n",OutFile);
		purgeLL(EeColList);
		purgeLL(EeRowList);
		return 0;
	}
		
	reportLongTraverses();
	
	
	/*------------------------------------------------------------------
		At this point, all junctions have been created and there is a
		row record for each. 
	-------------------------------------------------------------------*/

	/*------------------------- Build matrix -------------------------*/
	for(i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		for(j=seg->strings;j<seg->strings+seg->nStrings;j++) {
			str=getStr(j);
			if(str->code&SuperStringEntry) {
				RotateCursor(j);
				/*------------------------------------------------------
					The SuperStringEntry flag was set in p1.5 as part 
					of the marking of merging traverses through trivial
					junctions (i.e. of order 1). This criterion produces
					an entry into all of the consolidated traverses that
					pass through the main data.
				-------------------------------------------------------*/
				tra=getNextTra();
				tra->self=LongTraList->current;
				tra->code=tra->nLegs=0;
				tra->entryStr=j;	/* index to entry string */
				tra->entrySegment=i;
				tra->traLength=0;
				/* This trace fills traLength, netLength.{x,y,z}, tra->nLegs 
					and sets fromVertex and toVertex with row indices. */
				traceTraverse(str,seg,makeEETraLink,makeEETraTraverse,tra);
				addEEColumn(tra);
			}
		}
		seg=getSR(i);	/* Re-get, just in case we are thrashing. */
		
		/*-------------------------------------------------------------
			For the same segment, look through the ties for the one-shot
			traverses in them. 
		---------------------------------------------------------------*/

		for(j=seg->links+seg->nLocns+seg->nLinks
				;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			li=getLink(j);
			if(li->to.segment<0) continue;
			lo=getLocn(li->data);
			if(lo->code&(SuperStringMember|DeadEnd)) continue;
			tra=getNextTra();
			tra->self=LongTraList->current;
			tra->code=0;
			tra->fromVertex=findEeRow(i,li->from.n);
			tra->toVertex=findEeRow(li->to.segment,li->to.n);
			if(!tra->fromVertex || !tra->toVertex) {
				_errorAlert(UnexpectedError,i,li->to.n);
			} else {
				tra->nLegs=1;
				tra->entryStr=-j;	/* Minus index to tie */
				tra->entrySegment=i;
				addEEColumn(tra);
			}
		}
	}
		
	/*----------------- Reset (Off-Diagonal) Column Indices -----------------*/
	for(j=1;j<=EeRowList->current;j++) {
		row=getEeRow(j);
		row->cols-=row->nCols;
	}
	
	/* List Nodes */
	fputs("\nNodes\n",OutFile);
	for(i=1;i<=EeRowList->current;i++) {
		row=getEeRow(i);
		fprintf(OutFile,"%-4d%-15s joins traverses:",i,printFullSta(row->this.n,getSR(row->this.segment),a));
		for(temp=0,j=row->cols;j<row->cols+row->nCols;j++) {
			col=getEeCol(j);
			if(col->traverse)
				fprintf(OutFile," %d",col->traverse);
			else {
				temp++;
				row->nVertices--;	/* effectively remove fixed location. */
			}
		}
		if(temp)
			fputs(" and a fixed location",OutFile);
		fputc('\n',OutFile);
	}
	fputc('\n',OutFile);
	
	printDashes();
	fputs("Network Loops\n",OutFile);
	fputs("   In the following, the traverses and nodes (junctions) making up the closure\n",OutFile);
	fputs("   network are repeatedly searched until all loops have been found. The nodes\n",OutFile);
	fputs("   and traverses of the loops discovered are appropriately merged or removed\n",OutFile);
	fputs("   from the net and the search repeated. Traverses are prefixed with a \'t\' and\n",OutFile);
	fputs("   nodes with an \'n\'.\n",OutFile);
	printDashes();
	
	/* Now start looking for additional loops */
	depth=0;
	do {
		temp=0;
		/*------------ Merge parallel traverse --------------*/
		for(i=1;i<=EeRowList->current;i++) {
			row=getEeRow(i);
			if(row->code&IsStillNode) {
				for(j=row->cols;j<row->cols+row->nCols;j++) {
					col=getEeCol(j);
					if(col->farEnd>0) {
						if(col->farEnd==i) {
							temp++;
							row->nVertices--;
							col->originalFarEnd=col->farEnd;
							col->farEnd=0;
							fprintf(OutFile,"Closed Loop: t%d at %s(n%d)\n",col->traverse,
									printFullSta(row->this.n,getSR(row->this.segment),a),row->self);
							/* There is a second representation of this loop. */
							for(k=j+1;k<row->cols+row->nCols;k++) {
								col0=getEeCol(k);
								if(col0->farEnd==i) {
									row->nVertices--;
									col0->originalFarEnd=col0->farEnd;
									col0->farEnd=0;
									break;
								}
							}
						} else {
							for(k=j+1;k<row->cols+row->nCols;k++) {
								col0=getEeCol(k);
								if(col->farEnd==col0->farEnd) {
									/*	Here are parallel cols (traverses). Merge them by removing
										col0 and its mirror. */
									row0=getEeRow(col0->farEnd);
									col0->originalFarEnd=col0->farEnd;
									col0->farEnd=0;
									row->nVertices--;
							
									col1=findEeCol(row0,col0->traverse,(short)i); 
									col1->originalFarEnd=col1->farEnd;
									col1->farEnd=0;
									row0->nVertices--;
									fprintf(OutFile,"Loop: t%d and t%d between %s(n%d) and %s(n%d), (t%d=t%d||t%d)\n",
												col->traverse,col0->traverse,
												printFullSta(row->this.n,getSR(row->this.segment),a),row->self,
												printFullSta(row0->this.n,getSR(row0->this.segment),b),row0->self,
												col->traverse,col->traverse,col0->traverse);
									temp++;
								}
							}
						}
					}
				}
			}
		}
	
		/* If no loops have been found, do a high level search */
		for(j=3;!temp && j<10;j++) {
			for(i=1;!temp && i<=EeRowList->current;i++) {
				row=getEeRow(i);
				if(row->code&IsStillNode) {
					temp = (searchForHighLevelLoop(row,j,i,0))? 1:0;
					if(temp)
						putc('\n',OutFile);
				}
			}
		}
		
		/*--------------------------------------------------------
			Now need to remove rows that have vertex counts of 2.
		----------------------------------------------------------*/
		for(i=1;i<=EeRowList->current;i++) {
			row=getEeRow(i);
			if(row->code&IsStillNode) {
				if(row->nVertices==1)
					removeDeadEnd(row);
				else if(row->nVertices==2)
					mergeSeriesTraverses(row);
			}
		}
		nEulerLoops+=temp;
		if(temp) {
			printDashes();
			fprintf(OutFile,"    At depth %d, %d loop(s) found for a total of %d of %d.\n",
								++depth,temp,
								nEulerLoops,LongTraList->current-EeRowList->current+1);
		
			fputs("    Nodes(count) remaining:",OutFile);
			temp=27;
			for(i=1;i<=EeRowList->current;i++) {
				if(temp>72) {
					fputs("\n    ",OutFile);
					temp=4;
				}
				row=getEeRow(i);
				if(row->code&IsStillNode)
					temp+=fprintf(OutFile," n%d(%d)",i,row->nVertices);
			}
			fputc('\n',OutFile);
			printDashes();
		}

	} while(temp) ;
	fprintf(OutFile,"There are %d Euler Loops.\n",LongTraList->current-EeRowList->current+1);
	fprintf(OutFile,"%d have been found.\n",nEulerLoops);
	
	purgeLL(EeColList);
	purgeLL(EeRowList);
	
	return nEulerLoops;
}

struct eeColRec *searchForHighLevelLoop(struct eeRowRec *row,int order,int match,int enteringTra)
{
	int i;
	struct eeRowRec *row0;
	struct eeColRec *col,*col0;
	Str31 a;
	
	if(order==1) {
		for(i=row->cols;i<row->cols+row->nCols;i++) {
			col=getEeCol(i);
			if(col->farEnd==match && col->traverse!=enteringTra) {
				/*	Here are parallel cols (traverses). Merge them by removing
							col0 and its mirror. */
				row0=getEeRow(col->farEnd);
				col->originalFarEnd=col->farEnd;
				col->farEnd=0;
				row->nVertices--;
				
				col0=findEeCol(row0,col->traverse,i); 
				col0->originalFarEnd=col0->farEnd;
				col0->farEnd=0;
				row0->nVertices--;

				fprintf(OutFile,"Loop: t%d between %s(n%d)",
							col->traverse,
							printFullSta(row0->this.n,getSR(row0->this.segment),a),row0->self);
				fprintf(OutFile," and %s(n%d) forms a loop with\n     ",
							printFullSta(row->this.n,getSR(row->this.segment),a),row->self);
				return col;
			}
		}
	} else {
		for(i=row->cols;i<row->cols+row->nCols;i++) {
			col=getEeCol(i);
			if(col->farEnd && col->traverse!=enteringTra) {
				/* For the far end of each col from row, look for a match. */
				row0=getEeRow(col->farEnd);
				if(col0=searchForHighLevelLoop(row0,order-1,match,col->traverse)) {
					fprintf(OutFile," t%d from %s(n%d)",col->traverse,
							printFullSta(row->this.n,getSR(row->this.segment),a),
							row->self);
					return col;
				}
			}
		}
	}
	return 0;
}

void mergeSeriesTraverses(struct eeRowRec *row)
{
	int j,k;
	struct eeColRec *col,*col1,*col2;
	struct eeRowRec *row1,*row2;

	row->code&=~IsStillNode;

	/*	1. Get the two cols */
	
	for(col1=0,j=row->cols;!col1 && j<row->cols+row->nCols;j++) {
		col=getEeCol(j);
		if(col->farEnd) 
			col1=col;
	}
	for(col2=0,k=j;!col2 && k<row->cols+row->nCols;k++) {
		col=getEeCol(k);
		if(col->farEnd)
			col2=col;
	}
	if(!col2) 
		errorAlert(UnexpectedError);
		
	fprintf(OutFile,"    Node n%d removed, merging t%d and t%d between n%d and n%d, (t%d=t%d--t%d)\n",
								row->self,col1->traverse,col2->traverse,
								col1->farEnd,col2->farEnd,col1->traverse,col1->traverse,col2->traverse);
	/*	2. Get the other two rows. */
	
	row1=getEeRow(col1->farEnd);
	row2=getEeRow(col2->farEnd);

	/*	3. Get the cols that point to the disappearing jct (row) */
	col=findEeCol(row1,col1->traverse,row->self);
	col->originalFarEnd=row->self;
	col->farEnd=col2->farEnd;
	col->traverse=col1->traverse;
	
	col=findEeCol(row2,col2->traverse,row->self);
	col->originalFarEnd=row->self;
	col->farEnd=col1->farEnd;
	col->traverse=col1->traverse;
}

void removeDeadEnd(struct eeRowRec *row)
{
	int j;
	struct eeColRec *col,*col1;
	struct eeRowRec *row1;
	
	row->code&=~IsStillNode;
	
	/* Get the col */
	for(col1=0,j=row->cols;!col1 && j<row->cols+row->nCols;j++) {
		col=getEeCol(j);
		if(col->farEnd) 
			col1=col;
	}
	if(!col1) 
		errorAlert(UnexpectedError);
	/* Get the other row */
	row1=getEeRow(col1->farEnd);
	
	/* Get the mirror col */
	col=findEeCol(row1,col1->traverse,row->self);
	
	fprintf(OutFile,"    Dead End Node n%d and t%d to n%d removed\n",row->self,col1->traverse,col1->farEnd);
	/* Now we have everything. Null them */
	col->originalFarEnd=col->farEnd;
	col->farEnd=0;
	col1->originalFarEnd=col1->farEnd;
	col1->farEnd=0;
	if(--row1->nVertices == 2)
		mergeSeriesTraverses(row1);

}