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
	#pragma segment reporting
#else
	#include <search.h>
#endif

#define ColWidth 75
#define putLine(x) {x=0;fputc('\n',OutFile);}

/* #define atan2(a,b) eatan2((a),(b))				*/
/* extern extended eatan2(extended y,extended x);	*/

void printDashes(void);
void newPage(void);
void printLog(int);
c_float logField(int i,char *lines[],FILE **inFile);
static int exportTextBlock(indexType,FILE **);
static int printTextBlock(indexType,char *,FILE **,int,int,int);
static int printFill(char *,int,int);
void printAttributes(struct dbRec *);
void reportMemoryUsage(void);
int printCart(void);
int printSurveyCart(struct segmentRec *);
void put32(long, FILE *);
int doGraphicsFile(int);
int printSegmentDXF(struct segmentRec *,int,FILE *);
int printSegmentS3D(struct segmentRec *,int,FILE *);
int printSegmentSVX(struct segmentRec *,int,FILE *);
int printSegmentPS(struct segmentRec *,int,FILE *);
int reportSegment(struct segmentRec *seg, char *buf,int *tieLength);
void flushReport(char *lines[], int nCols);
int printTies(struct segmentRec *seg, char *buf, int sta, int desLen, int *tieLength);
void printFull(char *fileName, int how);
int prettyPrintTabs(int,unsigned int,int,int);
int prettyPrintData(int,unsigned int,int,int);
void dumpDB(int flag);
void reportSegmentChains(void);
int reportChain(int *,int);
void printSubSegment(struct segmentRec *);
void reportURS(void);
void printBounds(struct segmentRec *seg);
void reportByVertex(void);
void reportByLongTraverse(void);
void printConstrained(void);
char *printSta(int sta,char *buf);

char *printFullSta(int sta,struct segmentRec *seg,char *buf);
char *printPureSta(int sta,struct segmentRec *seg,char *buf);
char *printDesigSta(int sta,struct segmentRec *seg,char *buf);
char *printFullDesig(int sta,struct segmentRec *seg,char *buf);
char *printFullSegment(struct segmentRec *seg,char *buf);
void plotNetwork(int);
void printSummaries(int how);
extern void makeSkematic(void);
extern struct cartData *minExtent(struct cartData *least, struct cartData *data);
extern struct cartData *maxExtent(struct cartData *most, struct cartData *data);
static void checkSegmentConsistency(void);
void printTripReport(int,FILE **);
void exportTripReport(int,FILE **);

#ifndef CMLmake
void reportMatches(void);
struct segmentRec *matchNeighbors(struct segmentRec *,struct fieldRec *);
void doEllipse(void);
struct cartData *ellipse(struct segmentRec *seg,struct cartData *data);
#endif

/*----------------------------- Logsheets ---------------------------
	
	Logs can be just a schematic or a true logsheet.  A true logsheet
	includes Field header information, such as date, reconstruction, 
	personnel, etc.
	
--------------------------------------------------------------------*/

void newPage(void)
{
	fprintf(OutFile,"\n(&FF)\n");
	return;
}

void printDashes(void)
{
	fprintf(OutFile,
	"-------------------------------------------------------------------------------\n");
}

void printLog(int flag)
{
	int i,k;
	char *lines[5];	/* Array of pointers. 5 is Max number of columns+1 */
	FILE *inFile=0;
	struct segmentRec *seg;
	c_float totalLength=0;
		
	if(flag&1) {
		/* List of survey books or sections */
		if(!Silent)
			fprintf(OutFile,"\n;-------------------- %ss --------------------\n\n",FieldName);
		for(i=k=0;i<=MaxField-MinField;i++) {
			if(FieldList[i]) {
				fprintf(OutFile,"%d",i+MinField);
				if(FieldList[i+1]) {
					k++;
					for(++i;FieldList[i+1] && (i<=MaxField-MinField);i++) ;
					fprintf(OutFile,"-%d",i+MinField);
				}
				if(++k>9) {
					k=0;
					putc('\n',OutFile);
				} else
					putc(' ',OutFile);
			}
		}
		fprintf(OutFile,"\n\n");
	}
	if(flag&2) {
		lines[0] = emalloc(4000*sizeof(char));
		if(LogList) {
			for(i=1;i<=*LogList;i++) {
				k=abs(LogList[i]);
				if(k<=MaxField-MinField) {
					if(FieldList[k]) {
						FieldList[k]&=0x3FFF;			/* Clear any marks. */
						totalLength+=logField(k,lines,&inFile);
						FieldList[k]|=0x4000;			/* Marked as logged. */
					}
					if(LogList[i]<0) {
						/* Log each book/section up to the next one in LogList. */
						for(++k;k<LogList[i+1] && k<=MaxField-MinField;k++) {
							if(FieldList[k]) {
								FieldList[k]&=0x3FFF;			/* Clear any marks. */
								totalLength+=logField(k,lines,&inFile);
								FieldList[k]|=0x4000;			/* Marked as logged. */
							}
						}
					}
				}
			}
			/* Go back and log the neighboring fields (books or sections). */
			free(LogList);
			LogList=0;
			for(i=0;i<=MaxField-MinField;i++) {
				if(FieldList[i]&0x4000) {
					FieldList[i]&=0x3FFF;	/* Erase flag bits. */
					totalLength+=logField(i,lines,&inFile);
				}
				FieldList[i]&=0x3FFF;	/* Erase flag bits. */
			}
		} else {
			for(i=0;i<=MaxField-MinField;i++) {
				if(FieldList[i]) 
					totalLength+=logField(i,lines,&inFile);
			}
			if(Literals) {
				fprintf(OutFile,";---------------- Literal Stations ------------------\n\n");
				seg=getSR(Literals);
				lines[1] = lines[0]+reportSegment(seg,lines[0],&k);
				flushReport(lines,1);
			}
		}
		if(inFile)
			fclose(inFile);
		free(lines[0]);
		if(DoP2)
			fprintf(OutFile,"\nSum of shot distances: %.1f feet, %.1f meters, %.2f miles\n",totalLength,
				totalLength/MtoFt,totalLength/5280);
	}
	if(flag&4) 
		reportSegmentChains();
	if(flag&8)
		checkSegmentConsistency();
	if(flag&16 && HeaderList) {
		for(i=1;i<=HeaderList->current;i++) 
			if(flag&32)
				exportTripReport(i,&inFile);
			else
				printTripReport(i,&inFile);
		if(inFile)
			fclose(inFile);
	
	}
	return;
}

c_float logField(int i,char *lines[],FILE **inFile)
{
	int k=0,n,j,nSegs=0;	/* Number of segments */
	int nBPtr=0,neighboringFields[64],lastMin,thisMin;
	long fieldLength=0;
	int tieLength=0;
	struct segmentRec *seg,*su0;
	struct headerRec *head;
	LinkRecPtr li;
	struct nameRec *place,*person;
	struct dbRec *party,*db;
	c_float length,ties;
	char *p;
	
	newPage();
	fprintf(OutFile,";-------------------- %s $%d --------------------\n\n",FieldName,i+MinField);
	seg = getSR(FieldList[i]);
	if(head=(ProcessHeaders && seg->header)? getHeader(seg->header):0) {
		if(head->accession) {
			place=getPlace(head->accession,NameList);
			fprintf(OutFile," #Accession %s\n",place->name);
		}
		if(head->date) {
			p=cmltime(&head->date);
			fprintf(OutFile," #Date %.10s%s",p,p+19);
		}
		if(head->object) {
			place=getPlace(head->object,NameList);
			fprintf(OutFile," #Cave %s\n",place->name);
		}
		if(head->location) {
			place=getPlace(head->location,NameList);
			fprintf(OutFile," #Area %s\n",place->name);
		}
		if(n=head->personnel) {
			fprintf(OutFile," #Personnel");
			do {
				party=getDb(n);
				n++;
				person=getName(party->what);
				fprintf(OutFile," %s",person->name);
				if(party->where.item&Leader)
					fprintf(OutFile,"*");
				if(party->where.item&Job) {
					fprintf(OutFile," (");
					if(party->where.item&Book)
						fprintf(OutFile,"b");
					if(party->where.item&Compass)
						fprintf(OutFile,"c");
					if(party->where.item&PointPerson)
						fprintf(OutFile,"p");
					if(party->where.item&Sketch)
						fprintf(OutFile,"s");
					if(party->where.item&Backsights)
						fprintf(OutFile," bs ");
					if(party->where.item&Foresights)
						fprintf(OutFile," fs ");
					if(party->where.item&Tape)
						fprintf(OutFile,"t");
					fprintf(OutFile,")");
				}
			} while (!(party->where.item&LastInParty));
			putc('\n',OutFile);
		}
		putc('\n',OutFile);
	}
	for(nSegs=1,nBPtr=fieldLength=tieLength=k=0;seg;
		seg = (seg->next)? nSegs++,getSR(seg->next):0) {
		k+=reportSegment(seg,lines[0]+k,&tieLength);
		if (k>=3500)
			errorAlert(OutOfMemory);	/*======== Poor: need to be able to realloc in reportSegment. */
		lines[nSegs] = lines[0]+k;
		fieldLength+=seg->length;
		if(nSegs == 4) {
			flushReport(lines,nSegs);
			nSegs=0;
			k=0;
		}
		/* Make a list of neighboring Fields. */
		for(j=seg->links+seg->nLocns
			;j<seg->links+seg->nLinks+seg->nTies;j++) {
			li=getLink(j);
			su0=getSR(abs(li->to.segment));
			if(su0->Field!=i) {
				neighboringFields[nBPtr++]=su0->Field;
				if(nBPtr>63) nBPtr=63;
			}
		}
	}
	flushReport(lines,nSegs);
	k=0;
	if(nBPtr) {
		fprintf(OutFile,"Neighboring %s(s):\n",FieldName);
		thisMin=n=0;
		for(;;) {
			lastMin=thisMin;
			for(thisMin=j=0;j<nBPtr;j++) {
				if(neighboringFields[j]>lastMin) {
					if(thisMin)
						thisMin=(neighboringFields[j]<thisMin)?neighboringFields[j]:thisMin;
					else
						thisMin=neighboringFields[j];
				}
			}
			if(!thisMin) break;
			fprintf(OutFile,"%5d%s",thisMin,(++n % 8)? " ":"\n");
			if(LogList) /* Mark the neighbor, if it hasn't been reported so far. */
				FieldList[thisMin-MinField]|=(FieldList[thisMin-MinField]&0x4000)? 0 : 0x8000;
		}
		putc('\n',OutFile);
	}
	if(DoP2) {
		length=fieldLength;
		length/=10;
		ties=tieLength;
		ties/=10;
		fprintf(OutFile,"%.1f%straverse + %.1f ties = %.1f %s total\n",ForM(length),
			(HorizontalDistanceFlag)? " horizontal ":" ", ForM(ties),ForM(length+ties),
			(MetersOut)?"meters":"feet");
	}
	if(head) {
		if(head->notes)
			printTextBlock(head->notes,"\n #Notes:\n",inFile,0,ColWidth,1);
		if(head->attributes) {
			fprintf(OutFile,"\n #Attributes:\n");
			for(db=getDb(head->attributes);db;db=(db->link)?getDb(db->link):0) 
				printAttributes(db);
		}
	}
	return length+ties;
}


/*------------------------- TextBlock routines ---------------------------
	Text Blocks are found in some file, between some markers:
		file:		db->what
		markers:	db->where to db->finish
					If db->finish.item is NULL then the first \n trips out.
	exportTextBlock() turns the TextBlock into a byte stream with newlines
		and tabs replaced smartly with spaces.
	
-------------------------------------------------------------------------*/

static int exportTextBlock(indexType link,FILE **inFile)
{
	int ch,cnt,spaceFlag=FALSE;
	int how;
	FILE *theFile;	/* local copy for speed. */
	struct dbRec *db;
	
	if(link) {
		db=getDb(link);
		exportTextBlock(db->link,inFile);
	
		if(FileListPosn!=db->what) {
			if(*inFile)
				fclose(*inFile);
			*inFile=getFile(IFileList[FileListPosn=db->what],InBuf,NChars);
		}
		if(fseek(theFile=*inFile,db->where.item,SEEK_SET))
			errorAlert(DiskErr);
			
		/*	
			This loop steps, byte-wise, through the block of text 
			contained in the input file, filtering text as it goes.
			A more complex version is in printTextBlock(). If the 
			text block is on just one line, then cnt is always 
			negative and we trip out.
		*/
		if(how=(LogFlag&64))
			fputc('"',OutFile);
		for(cnt=(db->finish.item)? db->finish.item-db->where.item-1:-1;cnt;cnt--) {
			ch=getc(theFile);
			if(how) {
				if(ch=='\n' && cnt<0) {
					fputc('"',OutFile);
					return 0;
				}
				fputc(ch,OutFile);
				if(ch=='"')
					fputc('"',OutFile);
			} else {
				if(ch=='\n') {
					if(cnt<0) 
						return 0;
					if(!spaceFlag)
						fputc(' ',OutFile);
				} else if(isspace(ch)) {
					fputc(' ',OutFile);
					spaceFlag=TRUE;
				} else {
					fputc(ch,OutFile);
					spaceFlag=FALSE;
				}
			}
		}
		if(how)
			fputc('"',OutFile);
	}
	return 0;
}

int printTextBlock(indexType block,char *title,FILE **inFile,int left,int right,int nCRs)
{
	int k,i,cnt,width;
	int wrapped=FALSE;
	struct dbRec *db;
	FILE *theFile;	/* local copy for speed. */
	char *fill;
	
	if(block) {
		db=getDb(block);
		k=printTextBlock(db->link,title,inFile,left,right,nCRs);

		if(FileListPosn!=db->what) {
			if(*inFile)
				fclose(*inFile);
			*inFile=getFile(IFileList[FileListPosn=db->what],InBuf,NChars);
		}
		if(fseek(theFile=*inFile,db->where.item,SEEK_SET))
			errorAlert(DiskErr);
			
		cnt=(db->finish.item)? db->finish.item-db->where.item-1:-1;
		fill=Scratch;
		i=0;
		if(!k) 
			while(i<left)
				*(fill+i++)=' ';
		width=right-k;
		for(;cnt;cnt--,i++) {
			*(fill+i)=getc(theFile);
			if(*(fill+i)=='\n') {
/*=================			
				fprintf(OutFile,"\ncnt: %d, i: %d char: %d\n",cnt,i,(int)*(fill+i));
===================*/
				*(fill+i+((cnt>0)?1:0))=0;
				fputs(fill,OutFile);
				if(cnt<0) {			/* No db->finish: One line text block. */
					if(nCRs) {
						for(;nCRs;nCRs--)
							fputc('\n',OutFile);
						return 0;
					}
					return (wrapped)?i:k+i;
				}
				for(i=0;i<left;i++)
					*(fill+i)=' ';
				i--;
				width=right;
			} else if(i>width) {
/*				fprintf(OutFile,"\ncnt: %d, i: %d char: %d\n",cnt,i,(int)*(fill+i));	*/
				wrapped=TRUE;
				width=right;
				i=printFill(fill,i,left);
			}
		}
		if(i>left) {
			*(fill+i)=0;
			fputs(fill,OutFile);
			k=i;
		}
		if(nCRs) {
			for(;nCRs;nCRs--)
				fputc('\n',OutFile);
			return 0;
		}
		return k;
	} else {
		k=fprintf(OutFile,"%s",title);
		return (*(title+k-1)=='\n')? 0:k;
	}
}

static int printFill(char *buf,int cnt,int indent)
{
	int i,j;
	
	for(j=cnt;j>=0;j--) {	/* look for last separator */
		if(isspace(*(buf+j))||*(buf+j)=='-')
			break;
	}
	j=(j)?j+1:cnt;	/* if no sep found then print whole buf */
	for(i=0;i<j;i++)
		putc(*(buf+i),OutFile);
	putc('\n',OutFile);
	for(i=0;i<indent;i++)
		*(buf+i)=' ';
	for(;j<=cnt;i++,j++) 
		*(buf+i)=*(buf+j);
	return i-1;		/* Number of characters remaining in buf. */
}

void printAttributes(struct dbRec *db)
{
	int n;
	struct segmentRec *seg;
	struct nameRec *class;
	Str31 a;

	class=getName(db->what);
	fprintf(OutFile," %s",class->name);
	if(n=db->finish.sta.segment) {
		class=getName(n);
		fprintf(OutFile,": %s",class->name);	/* Sub-class name */
	}
	if(n=db->where.sta.segment) {
		seg=getSR(n);
		fprintf(OutFile," at %s",printDesigSta(db->where.sta.n,seg,a));
	} else
		fprintf(OutFile," at %s",printSta(db->where.sta.n,a));
	if(n=db->finish.sta.n)
		fprintf(OutFile," to %s",printDesigSta(n,seg,a));
	putc('\n',OutFile);
			
	return;
}

void reportMemoryUsage(void)
{
	int i;
	struct lList *theList;
	c_float totalMem=0;

	if(!Silent) {
		fprintf(OutFile,"\n;------------------------- Memory Usage -------------------\n\n");
		fprintf(OutFile,"List  Hits  Faults  pc    Size(by)  StruSize  Items  Buffers  Name\n");
		for(i=0;i<nMasters;i++) {
			if(MastersArray[i].p) {
				theList=(MastersArray+i)->p;
				totalMem+=((c_float)theList->size)*theList->nbuffers;
				fprintf(OutFile,"%2d%9lu%6d%6.1f%8d%10d%10d%7d   %s\n",i+1,theList->thisHit,
					theList->faults,
					(theList->thisHit)?(c_float)theList->faults*100/(c_float)theList->thisHit:0,
					theList->size,theList->nItems,theList->current,
					theList->nbuffers,*(LListNames+i));
			} 
		}
		fprintf(OutFile,"\nTotal Writes %d\nTotal Reads %d\n",NrTempWrites,NrTempReads);
		fprintf(OutFile,"\nTotal Memory Used %6.1fK\n\n",totalMem/1024);
	}
}

int printCart(void)
{ 
	int i,n;
	struct segmentRec *seg;
	Str31 a;
	
	fprintf(OutFile,"\n----------------- Bounds ----------------\n");
	for(i=0;i<=MaxField-MinField;i++) {
		for(n=FieldList[i];n;n=seg->next) {
			seg=getSR(n);
			fprintf(OutFile,"%s:\t",printFullSegment(seg,a));
			if(MetersOut)
				fprintf(OutFile,"%6.2fW to\t%6.2fE,\t%6.2fS to\t%6.2fN\n",
								seg->LSW.x,seg->UNE.x,seg->LSW.y,seg->UNE.y);
			else
				fprintf(OutFile,"%6.1fW to\t%6.1fE,\t%6.1fS to\t%6.1fN\n",
								seg->LSW.x,seg->UNE.x,seg->LSW.y,seg->UNE.y);
		}
	}
	fprintf(OutFile,"\n\n--------------- Coordinates --------------\n");
	for(i=0;i<=MaxField-MinField;i++) {
		for(n=FieldList[i];n;n=seg->next) 
			printSurveyCart(seg=getSR(n));
	}
	if(Literals) {
		printSurveyCart(getSR(Literals));
	}
	return 1;
}

int printSurveyCart(struct segmentRec *seg)
{
	int sta,j;
	struct segmentRec *su0;
	struct subSegRec *ss;
	struct locnRec *theData;
	LinkRecPtr theLink;
	Str31 a;
	
	if(seg->segment==Literals)
		fprintf(OutFile,"\n;--------- Literals ------- (%d)\n",Literals);
	else
		fprintf(OutFile,"\n;--------- %s %d%.4s ------- (%d)\n",SegmentName,seg->Field,seg->desig,seg->segment);
	ss = (struct subSegRec *)seg;
	while(ss && ss->data) {
		theData = getData(ss->data);
		for(sta=ss->first;sta<=ss->last;sta++,theData++) {
			if(theData->code) {
				fprintf(OutFile,"%s:\t",printDesigSta(sta,seg,a));
				if(MetersOut)
					fprintf(OutFile,"%6.2f\t%6.2f\t%6.2f\n",
						theData->data.cart.x/MtoFt,theData->data.cart.y/MtoFt,theData->data.cart.z/MtoFt);
				else
					fprintf(OutFile,"%6.1f\t%6.1f\t%6.1f\n",
						theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else
				putc('\n',OutFile);
		}
		ss=(ss->nextSubSeg)?getSS(ss->nextSubSeg):NULL; 
	}
	fprintf(OutFile,";------------ Links ---------------\n");
	for(j=seg->links;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
		theLink = getLink(j);
		if(!theLink->data || theLink->from.segment&Link)
			continue;
		theData = getLocn(theLink->data);
		if(theLink->to.segment) {
			sta=(theLink->to.segment>0)? -1: 0;
			su0=(seg->segment==abs(theLink->to.segment)?seg:getSR(abs(theLink->to.segment)));
			fprintf(OutFile,"%s:\t",printDesigSta(theLink->from.n,seg,a));
			if(MetersOut)
				fprintf(OutFile,"%6.2f\t%6.2f\t%6.2f\n",
					theData[sta].data.cart.x/MtoFt,theData[sta].data.cart.y/MtoFt,theData[sta].data.cart.z/MtoFt);
			else
				fprintf(OutFile,"%6.1f\t%6.1f\t%6.1f\n",
					theData[sta].data.cart.x,theData[sta].data.cart.y,theData[sta].data.cart.z);
				
			fprintf(OutFile,"%s%s:\t",(sta)?"->":"<-",printFullSta(theLink->to.n,su0,a));
			sta=(theLink->to.segment>0)? 0: -1;
		} else {
			sta=0;
			fprintf(OutFile,"%s:\t",printDesigSta(theLink->from.n,seg,a));
		}
		if(MetersOut)
			fprintf(OutFile,"%6.2f\t%6.2f\t%6.2f\n",
					theData[sta].data.cart.x/MtoFt,theData[sta].data.cart.y/MtoFt,theData[sta].data.cart.z/MtoFt);
		else
			fprintf(OutFile,"%6.1f\t%6.1f\t%6.1f\n",
					theData[sta].data.cart.x,theData[sta].data.cart.y,theData[sta].data.cart.z);
	}
	return 1;
}

int reportSegment(struct segmentRec *seg, char *buf,int *tieLength)
{
	struct strRec *theString;
	LinkRecPtr theLink; 
	int i,j,desLen,k=0,lastLast=-1;
	Str31 a;
	
	if(seg->Field>0)
		k+=1+sprintf(buf+k,"%s $%d%1.4s (%d)",SegmentName,seg->Field,seg->desig,seg->segment);

	for(i=seg->strings;i<seg->strings+seg->nStrings;i++) {
		theString = getStr(i);
		if(theString->code&UnMapped) {
			k+=1+sprintf(buf+k," ");
			k+=desLen=sprintf(buf+k,"%s ",printDesigSta(theString->first,seg,a));
			k+=printTies(seg,buf+k,theString->first,desLen,tieLength);
			lastLast = theString->first;
		} else {
			if(theString->first<seg->first) {
				_errorAlert(UnexpectedError,0,8);	/*===== Should never happen ====*/
				lastLast=theString->last;
				continue;
			}
			if(!(theString->code&ContigAtStart)) {
				/* Print any unmapped links before this string. These can only occur
					if string is not contiguous at start. */
				for(j=seg->links;j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
					theLink = getLink(j);
					if(theLink->from.n>lastLast && theLink->from.n<theString->first) {
						k+=1+sprintf(buf+k," ");
						k+=desLen=sprintf(buf+k,"%1.4s%s ",seg->desig,printSta(theLink->from.n,a));
						k+=printTies(seg,buf+k,theLink->from.n,desLen,tieLength);
						lastLast = theLink->from.n;
					}
				}
				/* Print the first station of the string and any ties. There is no preceeding
					contiguous string. */
				k+=1+sprintf(buf+k," ");
				k+=desLen=sprintf(buf+k,"%1.4s%s ",seg->desig,printSta(theString->first,a));
				if(theString->code & NodeAtStart) 
					k+=printTies(seg,buf+k,theString->first,desLen,tieLength);
				else
					k++;
			}

			/* Print the vertical bar for the string itself. It has two flavors, dead end
				and not dead end. */
			if(theString->code&DeadEnd)
				k+=1+sprintf(buf+k," |*");
			else
				k+=1+sprintf(buf+k," |");
			
			if(theString->length) {
				k--;
				if(MetersOut)
					k+=1+sprintf(buf+k," %.2f",((c_float)theString->length)/(10*MtoFt));
				else
					k+=1+sprintf(buf+k," %d.%d",theString->length/10,theString->length%10);
			}
			
			/*	Print the last station and any links. */
			k+=desLen=sprintf(buf+k,"%1.4s%s ",seg->desig,
					printSta(theString->last,a));
			if(theString->code & NodeAtEnd) 
				k+=printTies(seg,buf+k,theString->last,desLen,tieLength);
			else
				k++;
			
			/*	Get ready for next string. */
			lastLast=theString->last;
		}
	}
	return k;
}

void flushReport(char *lines[],int nCols)
{
	char *currentLines[4];	/* Max number of columns. */
	int i,activeCols;
	activeCols = nCols;
	for(i=0;i<nCols;i++)
		currentLines[i]=lines[i];
	while(activeCols) {
		for(i=activeCols=0;i<nCols;i++) {
			if(currentLines[i]<lines[i+1]) {
				fprintf(OutFile,"%-20s",currentLines[i]);
				currentLines[i]+=(strlen(currentLines[i])+1);
				activeCols++;
			} else
				fprintf(OutFile,"%-20s"," ");
		}
		putc('\n',OutFile);
	}
	putc('\n',OutFile);
	return;
}


/*--------------------------------- printTies -------------------------------
	Prints all the links to a station into *buf. Returns number of characters
	printed. Comes with the desig+station (i.e."A15" already printed).
	  
	A15	= [3400,1233,444]
	 .	= $476X10
	 .  -> $1500C1
	A15 <- $1502D15
----------------------------------------------------------------------------*/

int printTies(struct segmentRec *seg, char *buf, int sta, int desLen, int *tieLength)
{
	int i,lim,k=0;
	int fi=-1,la;
	LinkRecPtr theLink;
	struct locnRec *theLocn;
	struct segmentRec *su0;
	int tieDataBelongs,internalTie;
	Str31 a,b;
	
	for(lim=i=seg->links,lim+=seg->nLocns+seg->nTies+seg->nLinks;i<lim;i++) {
		if((theLink=getLink(i))->from.n==sta) {
			fi=(fi<0)? i : fi;
			la = i;
		}
	}
	if(fi<0) {
#ifdef DEBUG
		fprintf(ErrFile,"### Warning: Link to $%d%1.4s%d not found.###",
			seg->Field,seg->desig,sta);
#endif
		return 0;
	}
	for(i=fi;i<=la;i++) {
		if((theLink=getLink(i))->from.n == sta) {
			if(i != fi) {
				if(i == la)
					k+=sprintf(buf+k,"%s ",printDesigSta(sta,seg,a));
				else
					k+=sprintf(buf+k," .%*s",desLen-1," ");
			}
			if(!theLink->to.segment) {	/* = [123,456,789] */
				theLocn = getLocn(theLink->data);
				/* Adding 1 includes the terminal '\0', ending the string. */
				k+=(1+sprintf(buf+k,"= [%.1f,%.1f,%.1f]",theLocn->data.cart.x,
						theLocn->data.cart.y,theLocn->data.cart.z));
			} else if(!theLink->data || theLink->from.segment & Link) { /* Link */
				su0 = getSR(theLink->to.segment);
				if(su0->Field==seg->Field)
					k+=1+sprintf(buf+k,"= %s",printFullDesig(theLink->to.n,su0,b));
				else
					k+=1+sprintf(buf+k,"= %s",printFullSta(theLink->to.n,su0,b));
				if(theLink->from.segment&DeadEnd)
					k+=sprintf(buf+k,"*");
			} else {	/* Tie */
				if(internalTie=abs(theLink->to.segment)==seg->segment)
					su0=seg;
				else
					su0 = getSR(abs(theLink->to.segment));
				theLocn = getLocn(theLink->data);
				tieDataBelongs=theLocn[-1].length==seg->segment;
				if(theLink->to.segment > 0) {
					/* 	Original tie; the from-station is in this segment, 
						but is it part of this segment's data? */
					if(tieDataBelongs) 
						*tieLength+=theLocn->length;
					else 
						k+=sprintf(buf+k,"(");
					k+=sprintf(buf+k,"-> ");
				} else {
					/* This is the reversed tie. The orignal to-station (the
						from station of this link) in this segment. Is it in
						the segment's data? */
					if(!internalTie) {
						/*	Reversed internal ties do not contribute to length but
							they do get normal notation. */
						if(tieDataBelongs)
							*tieLength+=theLocn->length;
						else
							k+=sprintf(buf+k,"(");
					}
					k+=sprintf(buf+k,"<- ");
				}
				if(su0->Field==seg->Field)
					k+=sprintf(buf+k,"%s",printFullDesig(theLink->to.n,su0,b));
				else
					k+=sprintf(buf+k,"%s",printFullSta(theLink->to.n,su0,b));

				if(!tieDataBelongs)
					k+=sprintf(buf+k,")");
				if(theLocn->code&DeadEnd)
					k+=sprintf(buf+k,"*");
				if(CloseFlag&&theLink->to.segment&&theLink->data) {	/* ties only */
					/* str lengths only calculated in building matrix. */
					if(!tieDataBelongs||theLocn->code&DeadEnd)
						k+=sprintf(buf+k,"%d.%d",theLocn->length/10,theLocn->length%10);
					else
						k+=sprintf(buf+k," %d.%d",theLocn->length/10,theLocn->length%10);
				
				}
				k++;	/* end of line */
			}
		}
	}
	return k;
}

void printFull(theFileName,how)
char *theFileName;
int how;
{
	int i,sep,canBlankStation;
	FILE *theFile;
	struct context *then;
	struct lineRec *theLine;
	struct locnRec *theData;
	
	if(!(theFile=getFile(theFileName,InBuf,NChars))) {
		errorAlert(FileInErr);
		return ; 				/* Fatal Err, no input file, so quit */
	}
	
	then=ThisContext;
	ThisContext=newContext(1,theFileName,theFile);

	if(how&1) {
		sep=' ';
/*		fprintf(OutFile,";------------------- Full Listing -------------------\n#fullListing\n");	*/
	} else
		sep='\t';
		
	if(LineCnt)
		theLine=(struct lineRec *)getLListBuf(LineCnt,LineList)+(LineCnt&MedBlockLo);
	else 
		theLine=(struct lineRec *)getLListBuf(1,LineList);
		
	canBlankStation=FALSE;
	while((s=getLine(ThisContext->lastLine,MaxInLine,theFile)) != NULL) {
		ThisContext->lastLine=ThisContext->s0;
		ThisContext->s0=s;
		i=1;
		LineCnt++;
		ContinuationLines=0;
		
		if(LineCnt & MedBlockLo)
			theLine++;
		else
			theLine=(struct lineRec *)getLListBuf(LineCnt,LineList)+(LineCnt&MedBlockLo);
		/*-------------------------------------------------
		theLine->state
		0				data line with no station
		1				directive
		2				comment
		3				skipped block (#nop, #beginnote, etc.)
		4-9				unused
		&0x8000			first line of an implicit tie
		else			parse state
		
		--------------------------------------------------*/

		if(!theLine->state || (theLine->state==0x8000)) 
			i=(how&2)? prettyPrintTabs(--i,theLine->fields,1,5):prettyPrintData(i,theLine->fields,12,6);
		else if((theLine->state&0x300)==0x100) {
			if(canBlankStation && (theLine->state&0xF000)==0x1000 && (theLine->sta.n%5)) {
				while(*s != ':') {s++;i++;}
				s++;
			} else
				while(*s != ':') {putc(*s++,OutFile);i++;}
			putc(*s++,OutFile);i++;
			putc(sep,OutFile);i++;
			i=(how&2)? prettyPrintTabs(1,theLine->fields,1,5):prettyPrintData(i,theLine->fields,12,6);
		} else if((theLine->state&0x300)==0x300) {	/* explicit tie */
			while(*s != '>') {putc(*s++,OutFile);i++;}
			while(*s != ':') {putc(*s++,OutFile);i++;}
			putc(*s++,OutFile);i++;
			i+=fprintf(OutFile,"   \\");	/* continuation symbol */
			if(ListingFlag) {
				while(i<54) { putc(' ',OutFile);i++;}
				theData=getLocn(theLine->data-1);
				fprintf(OutFile,"| %8.1f %8.1f %7.1f\n",theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else
				putc('\n',OutFile);
			i=(how&2)? prettyPrintTabs(0,theLine->fields,1,5):prettyPrintData(1,theLine->fields,12,6);
		} else if(theLine->state&0x400) {	/* .. line */
			while(*s != '.')  {
				if(*s=='\t') {
					putc(' ',OutFile);
					s++;
				} else 
					putc(*s++,OutFile);
				i++;
			}
			while(i<12) {
				putc(' ',OutFile);
				i++;
			}
			putc(*s++,OutFile);
			putc(*s++,OutFile);
			i+=2;
			if(how&2) {
				_clearSpace;
				fprintf(OutFile,"\t\t\t\t");
			} else {
				_clearSpace;
				while(i<42) {
					putc(' ',OutFile);
					i++;
				}
			}
		}
		if(how&1) {
			while(*s && (*s!='\n')) {
				if(*s=='\t') {
					s++;
					putc(' ',OutFile);i++;
					while(i % 4) {
						putc(' ',OutFile);
						i++;
					}
				} else {
					putc(*s++,OutFile);
					i++;
				}
			}
		} else {
			while(*s && (*s!='\n'))
				putc(*s++,OutFile);
		
		}	
		if(!ListingFlag || (theLine->state && theLine->state<10)) {
			putc('\n',OutFile);
		} else {
			while(i<54) 
				{ putc(' ',OutFile); i++; }
			if(i>54) 
				fprintf(OutFile,"   \\\n%53s"," ");
				
			if(theLine->state&0x8000) {	/* from-station of an implicit tie */
				theData=getLocn(theLine->data-1);
				fprintf(OutFile,"| %8.1f %8.1f %7.1f\n",theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else if(theLine->state&0x4000) {	/* to-station of an implicit tie */
				theData++;	/* still valid, from from-station */
				fprintf(OutFile,"| %8.1f %8.1f %7.1f\n",theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else if((theLine->state&0x300)==0x300) {	/* explicit tie, theData already points to LocnList locn. */
				theData++;
				fprintf(OutFile,"| %8.1f %8.1f %7.1f\n",theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else if((theLine->state == 0)
						||((theLine->state&0x700)==0x100)
						||((theLine->state&0x700)==0x400)) {
				theData = getData(theLine->data);
				fprintf(OutFile,"| %8.1f %8.1f %7.1f\n",theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			} else
				putc('\n',OutFile);
		}
		canBlankStation= how&4 && (theLine->state&0xF000)==0x1000;
		if(ContinuationLines) {
			theLine=(struct lineRec *)getLListBuf(LineCnt,LineList)+(LineCnt&MedBlockLo);
		}
	}	/* end of main loop. */
	fclose(theFile);
	
	if(ThisContext) free(ThisContext);
	ThisContext=then;
	return;
}

int prettyPrintTabs(int tabCount,unsigned int fields,int start,int end)
{
	int j;
	
	while(tabCount<start) {putc('\t',OutFile);tabCount++;}
	_clearSpace;
	if(*s=='('||*s=='\\')
		printComment();
	while(isfdigit) {putc(*s,OutFile);s++;}
	_clearSpace;
	if(*s=='('||*s=='\\')
		printComment();
	putc('\t',OutFile); tabCount++;
	for(j=0;j<5;j++,fields>>=1) {
		if(fields&1) {
			_clearSpace;
			if(*s=='('||*s=='\\')
				printComment();
			if(j<2) {
				while(isbdigit) {putc(*s,OutFile);s++;}
				if(*s=='*')  {putc(*s,OutFile);s++;}
			} else {
				while(isfdigit) {putc(*s,OutFile);s++;}
				if(*s=='*')  {putc(*s,OutFile);s++;}
			}
			_clearSpace;
			if(*s=='('||*s=='\\') 
				printComment();
			if(*s==',') {
				putc(*s,OutFile);
				s++;
				_clearSpace;
				if(*s=='('||*s=='\\') 
					printComment();
			} else {
				putc('\t',OutFile);
				tabCount++;
			}
		}
	}
	while(tabCount<end) {
		putc('\t',OutFile);
		tabCount++;
	}
	
	return tabCount;
}

int prettyPrintData(int col,unsigned int fields,int start,int spacing)
{
	
	int j,scol;
	char *p;
	
	while(col<=start-1) {putc(' ',OutFile);col++;}
	_clearSpace;
	while(isfdigit) {putc(*s,OutFile);s++;col++;}
	_clearSpace;
	if(*s=='('||*s=='\\') 
		col+=printComment();
	putc(' ',OutFile);col++;
	while(col<=start+spacing-1) {putc(' ',OutFile);col++;}
	
	for(j=0,scol=col;j<5;j++,fields>>=1) {
		while(col < scol) {putc(' ',OutFile);col++;}
		if(fields&1) {
			if(*s==',') {putc(*s,OutFile);s++;col++;}
			_clearSpace;
			if(*s=='('||*s=='\\') 
				col+=printComment();
			if(j>1 && isdigit(*s)) {putc(' ',OutFile);col++;}
			if(j<2) {
				while(isbdigit) {putc(*s,OutFile);s++;col++;}
				if(*s=='*')  {putc(*s,OutFile);s++;}
			} else {
				while(isfdigit) {putc(*s,OutFile);s++;col++;}
				if(*s=='*')  {putc(*s,OutFile);s++;}
			}
			_clearSpace;
			if(*s=='('||*s=='\\') 
				col+=printComment();
			putc(' ',OutFile);col++;
		}
		scol+=(j%2)?spacing+1:spacing;
		if(j>1) scol--;
	}
	if(RotateDimensions) {
		if(isfdigit) {
			p=s;
			for(j=0;j<RotateDimensions;s++,col++)
				if(*s==',') j++;
			while(*s && (*s!='\n')) {putc(*s,OutFile);s++;col++;}
			putc(',',OutFile);
			for(j=0;j<RotateDimensions;) {
				putc(*p,OutFile);
				if(*++p==',') j++;
			}
		}
	}
	return col;	
}

void dumpDB(int flag)
{
	int i,n;
	struct nameRec *place;
	if(flag==0 || NameList==0)
		return;
	for(i=0;i<37;i++) {
		n=*(NameHashArray+i);
		while(n) {
			place=getName(n);
			fprintf(OutFile,"%s\n",place->name);
			n=place->next;
		}
	}
	return;
}

void reportSegmentChains(void)
{
	int i,j,n,cnt;
	int *segmentTable,segmentTableMax=512;
	int desigTable[53];
	struct segmentRec *seg;
	
	/* Fill hash table */
	segmentTable=emalloc(segmentTableMax*sizeof(int));
	fprintf(OutFile,"\n;------------------------- %s Chains ----------------------------\n",SegmentName);
	for(i=0;i<53;i++) *(desigTable+i)=0;	/*=== is this necessary? */
	/* Make hash table. */
	for(i=MaxField-MinField;i>=0;i--) {
		j=FieldList[i];
		while(j) {
			seg=getSR(j);
			n=(isalpha(seg->desig[0]))? 2*(tolower(seg->desig[0])-'a'):52;
			if(seg->desig[1]) n++;	/* Multiple-letter segments in different category */
			seg->other=*(desigTable+n);
			*(desigTable+n)=j;
			j=seg->next;
		}
	}
	
	/* For each letter */
	for(i=0;i<53;i+=2) {
		cnt=0;
		
		/* 	Fill a segmentTable with a list of all the segments of this letter. */
		for(j=*(desigTable+i);j;j=seg->other) {
			seg=getSR(j);
			segmentTable[cnt++]=j;
			if(cnt==segmentTableMax) 
				segmentTable=erealloc(segmentTable,sizeof(int)*(segmentTableMax+=128));
		}
		segmentTable[cnt]=-1;
		
		/*	Now make the report, if we have a cnt. */
		if(cnt) {
			fprintf(OutFile,";---------------- %d %c survey%c -----------------\n",
				cnt,(i<52)?'A'+i/2:'#',(cnt==1)? '\0':'s');	/* i.e. "-- 3 X surveys --" */
			for(j=0;j<cnt;j++) {
				if(*(segmentTable+j)>0) {
					n=reportChain(segmentTable+j,i%2);
					fprintf(OutFile," [%d junctions]\n",n);
				}
			}
		}
	}
	putc('\n',OutFile);

	free(segmentTable);	/* Thanks. Good job. */
	return;
}

/*----------------------------------------------------------------------
	Check for a chain of like-lettered segments and print it out. Mode
	is TRUE when reporting multi-letter desigs.
----------------------------------------------------------------------*/

int reportChain(int *sT,int mode)
{
	int i,*sTP,n=1,toSegment;
	struct segmentRec *seg,*su0;
	LinkRecPtr li;
	
	seg=getSR(*sT);		/* 	Get the sTarting segment, null its entry back in the */
	*(sT++)=0;			/*	calling segmentTable, and point sT to the next like seg. */
	printSubSegment(seg);
	do {
		su0=0;
		n+=seg->nStrings;	/*	The number of strings is roughly the number of nodes in a segment. */
		for(i=seg->links+seg->nLocns;
			i<seg->links+seg->nLocns+seg->nLinks+seg->nTies; i++) {
			li=getLink(i);
			if((toSegment=abs(li->to.segment))!=seg->segment) {
				/*	For every non-selfreferential link: */
				for(sTP=sT;*sTP>=0;sTP++) {
					/*	For every like segment, if it links... */
					if(*sTP==toSegment) {
						su0=getSR(toSegment);
						if((mode)? desigCmp(seg->desig,su0->desig):1) {
							*sTP=0;
							fprintf(OutFile,"-");
							printSubSegment(su0);
						} else
							su0=0;	/* Unsuccessful desigCmp (only done when mode==TRUE)
										means su0 is no good. */
					}
				}
			}
		}
	} while(seg=su0);
	return n;
}

void printSubSegment(struct segmentRec *seg)
{
	struct subSegRec *ss;
	struct strRec *str;
	Str31 a,b;
	
	fprintf(OutFile,"$%d%.4s(",seg->Field,seg->desig);
	if(seg->nStations) {
		ss=(struct subSegRec *)seg;
		for(;;) {
			if(ss->first!=ss->last) 
				fprintf(OutFile,"%s-%s",printSta(ss->first,a),printSta(ss->last,b));
			else
				fprintf(OutFile,"%s",printSta(ss->first,a));
			if(ss->nextSubSeg) {
				fprintf(OutFile,",");
				ss=getSS(ss->nextSubSeg);
			} else {
				fprintf(OutFile,")");
				return;
			}
		}
	} else if(seg->strings) {
		str=getStr(seg->strings);
		fprintf(OutFile,"%s",printSta(str->first,a));
		if(seg->nStrings>1) {
			str=getStr(seg->strings+seg->nStrings-1);
			fprintf(OutFile,",%s",printSta(str->first,b));
		}
		fprintf(OutFile,")");
	}
}

void reportURS(void)
{
	int i,n,cnt;
	int desigTable[27];
	struct unResSegRec *urs;
	struct segmentRec *seg,*su0;
	fprintf(OutFile,";--------------------- Warning -----------------------\n");
	
	fprintf(OutFile,"\n; There %s %d unresolved segment(s)\n",
		(UnResSegList->current>1)?"are":"is",UnResSegList->current); 
	/* Fill hash table */
	for(i=0;i<27;i++) *(desigTable+i)=0;
	for(i=SegList->current;i>0;i--) {
		seg=getSR(i);
		n=(isalpha(seg->desig[0]))?tolower(seg->desig[0])-'a':26;
		seg->other=*(desigTable+n);
		*(desigTable+n)=i;
	}
	for(i=1;i<=UnResSegList->current;i++) {
		urs=getURS(i);
		if(urs->resSegment<1) {
			cnt=0;
			seg=getSR(urs->segment);
			printBounds(seg);
			n=*(desigTable+((isalpha(seg->desig[0]))?tolower(seg->desig[0])-'a':26));
			while(n) {
				su0=getSR(n);
				if((n!=urs->segment) && desigCmp(seg->desig,su0->desig)) {
					fprintf(OutFile,"  %-4d ",++cnt);
					printBounds(su0);
				}
				n=su0->other;
			}
		}
	}	
}

void printBounds(struct segmentRec *seg)
{
	int first,last,k;
	struct strRec *str;
	Str31 a,b;
	
	str=getStr(seg->strings);
	first=str->first;
	if(seg->nStrings>1) 
		str=getStr(seg->strings+seg->nStrings-1);
	last=(str->code&UnMapped)?str->first:str->last;
	k=fprintf(OutFile,"%s",printFullSta(first,seg,a));
	if(first!=last) 
		fprintf(OutFile,"-%-*s",12-k,printSta(last,b));
	else 
		fprintf(OutFile,"%*s",13-k," ");
	if(DoP2 && !ExchangeFormat) {
		fprintf(OutFile," at [%6.1f,%6.1f,%6.1f]",
					ForM(seg->LSW.x),ForM(seg->LSW.y),ForM(seg->LSW.z));
		if(first!=last) 
			fprintf(OutFile," to [%6.1f,%6.1f,%6.1f]",
					ForM(seg->UNE.x),ForM(seg->UNE.y),ForM(seg->UNE.z));
	}
	putc('\n',OutFile);
	return;
}

void reportByVertex(void)
{
	int i,j,sta;
	c_float dx,dy,dz,errX,errY,errZ,totalErr,fact,pcErr;
	struct rowRec *row,*row0,*row1;
	struct colRec *col;
	TraRecPtr tra;
	char *prep;
	Str31 a;

/*------------------- Report Pre-adjustment closure errors ------------*/
	
	putc('\n',OutFile);
	printDashes();
	fputs("Network Vertices\n",OutFile);
	fputs("  This table lists each node (junction) in the closure network and describes\n",OutFile);
	fputs("  the traverses connecting to it. An asterisk (*) means that there is an error,\n",OutFile); 
	fputs("  based on the geometric sum of shot lengths, larger than 2.0%. All errors\n",OutFile); 
	fputs("  larger than 0.5% are reported as \"Err:\" followed by 1/2 the error in x,y, and\n",OutFile); 
	fputs("  z, then the error length and percent error, in parentheses (length, error).\n",OutFile);
	printDashes();
	putc('\n',OutFile);
	for(i=1;i<=RowList->current;i++) {
		row=getRow(i);
		fprintf(OutFile,"%-4d %s ",i,printFullSta(row->this.n,getSR(row->this.segment),a));
		if(row->code&HasLocn) {
			if(MetersOut)
				fprintf(OutFile,"[%6.2f,%6.2f,%6.2f]\n",row->locn.x/MtoFt,row->locn.y/MtoFt,row->locn.z/MtoFt);
			else
				fprintf(OutFile,"[%6.1f,%6.1f,%6.1f]\n",row->locn.x,row->locn.y,row->locn.z);
		} else { 
			if(MetersOut)
				fprintf(OutFile,"(%6.2f,%6.2f,%6.2f)\n",row->locn.x/MtoFt,row->locn.y/MtoFt,row->locn.z/MtoFt);
			else
				fprintf(OutFile,"(%6.1f,%6.1f,%6.1f)\n",row->locn.x,row->locn.y,row->locn.z);
		}
		for(j=row->cols;j<row->cols+row->nCols;j++) {
			col=getCol(j);
			if(col->traverse) {
				tra=getTra(col->traverse);
				/* Which way does this traverse run? */
				if(i==tra->fromVertex) {
					row1=getRow(sta=tra->toVertex);
					row0=row;	/* From-vertex */
					prep=" to ";
					fact = 2;
				} else {
					row1=row;	/* To-vertex */
					row0=getRow(sta=tra->fromVertex);
					prep="from";
					fact = -2;
				}
				if(tra->code&UnProcessed) {
					fprintf(OutFile,"  tra %-3d%s jct %-3d",col->traverse,prep,sta);
				} else {
					/* Calculate distance between vertices. */
					dx=row1->locn.x-row0->locn.x;
					dy=row1->locn.y-row0->locn.y;
					dz=row1->locn.z-row0->locn.z;
					/* Calculate short-fall of traverse*/
					errX=dx-tra->netLength.x;
					errY=dy-tra->netLength.y;
					errZ=dz-tra->netLength.z;
					totalErr=hypot3(errX,errY,errZ);
				/*	pcErr=(totalErr!=0)?tra->traLength/totalErr:0; */
					pcErr=(totalErr!=0)?100*totalErr/tra->effLength:0;
					fprintf(OutFile,"%2s",(pcErr>0.5)?"*":" "); 
					fprintf(OutFile,"tra %-3d %s jct %-3d ",col->traverse,prep,sta);
					if(MetersOut)
						fprintf(OutFile,"%6.2fm in %2d leg%s",tra->traLength/MtoFt,tra->nLegs,
									(tra->nLegs==1)?" ":"s");
					else
						fprintf(OutFile,"%6.1f ft in %2d leg%s",tra->traLength,tra->nLegs,
									(tra->nLegs==1)?" ":"s");
					if(pcErr>0.2) {
						if(MetersOut)
							fprintf(OutFile," Err: %5.2f,%5.2f,%5.2f ",
									(errX/fact)/MtoFt,(errY/fact)/MtoFt,(errZ/fact)/MtoFt);
						else
							fprintf(OutFile," Err: %5.1f,%5.1f,%5.1f ",errX/fact,errY/fact,errZ/fact);
							
						if(MetersOut)
							fprintf(OutFile,"(%4.2f,%3.1f)",totalErr/MtoFt,pcErr);
						else
							fprintf(OutFile,"(%4.1f,%3.1f)",totalErr,pcErr);
					}
				}	
				putc('\n',OutFile);
			}
		}
	}
	return;
}

void reportByLongTraverse(void)
{
	int i;
	TraRecPtr tra;
	struct rowRec *row;
	Str31 a,b;
	c_float angle;
	
	putc('\n',OutFile);
	putc('\n',OutFile);
	printDashes();
	fputs("Network Traverses\n",OutFile);
	fputs("  This table lists each traverse in the closure network. The number in paren-\n",OutFile);
	fputs("  theses that follows each station name is the junction (jct) number used in\n",OutFile);
	fputs("  the preceding table. The pair of numbers in parentheses is the total tra-\n",OutFile); 
	fputs("  verse length and net bearing. The triplet in braces is the net vector,\n",OutFile);
	fputs("  i.e. {x,y,z}.\n",OutFile);
	printDashes();
	putc('\n',OutFile);
	for(i=1;i<=LongTraList->current;i++) {
		tra=getTra(i);
		row=getRow(tra->fromVertex);
		fprintf(OutFile,"%-3d%9s(%3d) - ",i,
			printFullSta(row->this.n,getSR(row->this.segment),a),tra->fromVertex);
		if(tra->toVertex) {
			row=getRow(tra->toVertex);
			fprintf(OutFile,"%-10s(%3d)",printFullSta(row->this.n,getSR(row->this.segment),b),
				tra->toVertex);
		} else 
			fprintf(OutFile,"  loop         ");
		if(tra->code&UnProcessed) {
			putc('\n',OutFile);
		} else {
#ifdef CMLmake
			if(tra->netLength.x||tra->netLength.y) {
				angle=atan2(tra->netLength.x,tra->netLength.y)*RtoD;
				if(angle<0) angle+=360;
			} else
				angle=0;
#else
			angle=atan2(tra->netLength.y,tra->netLength.x)*RtoD;
#endif
			fprintf(OutFile," (%6.1f,%3.0f) %3d legs {%6.1f,%6.1f,%5.1f}\n",
				ForM(tra->traLength), angle, tra->nLegs,
				ForM(tra->netLength.x),ForM(tra->netLength.y),ForM(tra->netLength.z));
		}
	}
	putc('\n',OutFile);
	return;
}

void printConstrained(void)
{
	int i,j;
	struct segmentRec *seg;
	LinkRecPtr li;
	struct locnRec *lo;
	Str31 a;
	
	if(!FixArray[SegmentWithFirstLocn])
		return;
	fprintf(OutFile,"Fixed Locations\n");
	for(i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		for(j=seg->links;j<seg->links+seg->nLocns;j++) {
			li=getLink(j);
			lo=getLocn(li->data);
			fprintf(OutFile,"%s\t%6.1f\t%6.1f\t%6.1f\n",printFullSta(li->from.n,seg,a),
				ForM(lo->data.cart.x),ForM(lo->data.cart.y),ForM(lo->data.cart.z));
		}
	}
	return;
}

void plotNetwork(int flag)
{
	
	int i;
	TraRecPtr tra;
	struct rowRec *row0,*row1;
	c_float dx,dy,dz;
	struct c_locn locn;
	Str31 a;
	
	for(i=1;i<=LongTraList->current;i++) {
		tra=getTra(i);
		if(tra->toVertex) {
			row0=getRow(tra->fromVertex);
			row1=getRow(tra->toVertex);
			dx=row1->locn.x-row0->locn.x;
			dy=row1->locn.y-row0->locn.y;
			dz=row1->locn.z-row0->locn.z;
			locn.x=row0->locn.x+(dx-tra->netLength.x)/2;
			locn.y=row0->locn.y+(dy-tra->netLength.y)/2;
			locn.z=row0->locn.z+(dz-tra->netLength.z)/2;
			if(tra->entryStr>0) {
				if(flag&1)
					traceTraverse(getStr(tra->entryStr),getSR(tra->entrySegment),
						plotLink,plotTraverse,(void *)&locn);
				else if(flag&2)
					traceTraverse(getStr(tra->entryStr),getSR(tra->entrySegment),
						printLinkCart,printTraverseCart,(void *)&locn);
				else {	/* plot as single line. */
					fprintf(OutFile,"START %6.1f %6.1f %6.1f\n",
							ForM(locn.x),ForM(locn.y),ForM(locn.z));
					fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",
							ForM(locn.x+tra->netLength.x),
							ForM(locn.y+tra->netLength.y),ForM(locn.z+tra->netLength.z));
				}
			} else {	/* Just a single tie. */
				if(flag&2) {
					fprintf(OutFile,"\n%s: %6.1f %6.1f %6.1f\n",printFullSta(row0->this.n,getSR(row0->this.segment),a),
						ForM(locn.x),ForM(locn.y),ForM(locn.z));
					fprintf(OutFile,"->%s: %6.1f %6.1f %6.1f\n",printFullSta(row1->this.n,getSR(row1->this.segment),a),
						ForM(locn.x+tra->netLength.x),ForM(locn.y+tra->netLength.y),ForM(locn.z+tra->netLength.z));
				} else {
					fprintf(OutFile,"START %6.1f %6.1f %6.1f\n",ForM(locn.x),ForM(locn.y),ForM(locn.z));
					fprintf(OutFile,"LINE %6.1f %6.1f %6.1f\n",ForM(locn.x+tra->netLength.x),
					ForM(locn.y+tra->netLength.y),ForM(locn.z+tra->netLength.z));
				}
			}
		}
	}
	return;
}

char *printSta(int sta,char *buf)
{
	int k=0;
	if(sta!=-1) {
		if(sta>=StationPrimeRadix) {
			if (sta&0x4000) {	/* Alpha sequence */
				if(sta&0x20)
					k+=sprintf(buf,"%c",'a'+(sta&0x1F));
				else
					k+=sprintf(buf,"%d%c",(sta&0x3FFF)>>6,'a'+(sta&0x1F));				
			} else {
				k+=sprintf(buf,"%d",sta%StationPrimeRadix);
				while((sta-=StationPrimeRadix)>=0) 
					k+=sprintf(buf+k,"\'");
			}
		} else
			k+=sprintf(buf,"%d",sta);
	}
	*(buf+k)='\0';
	return buf;
}

char *printDesigSta(int sta,struct segmentRec *seg,char *buf)
{
	int k=0;
	struct nameRec *lit;

	if(seg->Field>0) {
		k=sprintf(buf,"%.4s",seg->desig);
		printSta(sta,buf+k);
	} else if(seg->Field==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",seg->Field,seg->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

char *printFullSta(int sta,struct segmentRec *seg,char *buf)
{
	int k=0;
	struct nameRec *lit;

	if(seg->Field>0) {
		k=sprintf(buf,"$%d%.4s",seg->Field,seg->desig);
		printSta(sta,buf+k);
	} else if(seg->Field==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",seg->Field,seg->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

char *printPureSta(int sta,struct segmentRec *seg,char *buf)
{
	char *a;
	a=printFullSta(sta,seg,buf);
	if(seg->Field==-1) 
		a[strlen(a)-1]=0;
	return a+1;
}

char *printFullDesig(int sta,struct segmentRec *seg,char *buf)
{
	int k=0;
	struct nameRec *lit;

	if(seg->Field>0) {
		k=sprintf(buf,"%.4s",seg->desig);
		printSta(sta,buf+k);
	} else if(seg->Field==-1) {	/* lit */
		lit=getName(sta);
		k=sprintf(buf,"\'%s\'",lit->name);
	} else {	/* unresolved */
		k=sprintf(buf,"$(%d)%.4s",seg->Field,seg->desig);
		printSta(sta,buf+k);
	}
	return buf;
}

char *printFullSegment(struct segmentRec *seg,char *buf)
{
	if(seg->Field>0)
		sprintf(buf,"$%d%.4s",seg->Field,seg->desig);
	else if(seg->Field==-1) 	/* lit */
		sprintf(buf,"Literals");
	else 	/* unresolved */
		sprintf(buf,"$(%d)%.4s",seg->Field,seg->desig);
	return buf;
}

/*------------------------ Print summaries ----------------------*/

void printSummaries(int how)
{
	int i;
	struct segmentRec *seg;
	Str31 a;
	
	if(Silent)
		return;
	if(how==1) {
		fprintf(OutFile,"\nThe location list has %d elements.\n",P1LocnList->current);
		fprintf(OutFile,"There are %d total links and ties\n",P1LList->current-P1LList->other);
		fprintf(OutFile,"and %d fixed locations.\n",P1LList->other);
		fprintf(OutFile,"There are %d %ss.\n",SegList->current,SegmentName);
		if(UnResSegList->current)
			fprintf(OutFile,"There are %d unresolved %ss.\n",UnResSegList->current,SegmentName);
		putc('\n',OutFile);
	}
	if(how==2 && LogFlag&1) {
		fprintf(OutFile,"There are %d traverses.\n",StringList->current - StringList->other);
		fprintf(OutFile,"There are %d unmapped nodes.\n",StringList->other);	
		fprintf(OutFile,"The size of the data space is: %d\n",DataList->current);
		fprintf(OutFile,"\n;--------------------------- Index of %ss --------------------------\n",SegmentName);
		fprintf(OutFile,"   (locations, links, ties, sub%ss, strings, stations, data storage)\n",SegmentName);
		fprintf(OutFile,"\nn    %-7s     lo li ti ss str sta  data\n",SegmentName);
		for(i=1;i<=SegList->current;i++) {
			seg=getSR(i);
			fprintf(OutFile,"%-4d %-10s %3d%3d%3d%s%4d%4d%6ld\n",i,
				printFullSegment(seg,a),
				seg->nLocns,seg->nLinks,seg->nTies,
				(seg->nextSubSeg)? " * ":"   ",
				seg->nStrings,seg->nStations,seg->data);
		}
		putc('\n',OutFile);
	}
	return;
}

struct cartData *minExtent(struct cartData *least, struct cartData *data) {
	least->x = (data->x<least->x)? data->x : least->x;
	least->y = (data->y<least->y)? data->y : least->y;
	least->z = (data->z<least->z)? data->z : least->z;
	return least;
}

struct cartData *maxExtent(struct cartData *most, struct cartData *data) {
	most->x = (data->x>most->x)? data->x : most->x;
	most->y = (data->y>most->y)? data->y : most->y;
	most->z = (data->z>most->z)? data->z : most->z;
	return most;
}

void makeSkematic(void)
{
	int i;
	struct cartData min,max;
	struct fieldRec *bo;
	struct segmentRec *seg;
	struct locnRec *locn;
	struct skCell *cell,*cell0;
	int minCellX,minCellY,maxCellX,maxCellY,overlaps;
	int cellX,cellY,sta;
	indexType *cellCols,*cellRows;
	indexType *nextPtr;
	c_float grid=50.0;

	if(FieldLList) {
		bo=getField(1);
		min=bo->LSW;
		max=bo->UNE;
		for(i=2;i<=FieldLList->current;i++) {
			bo=getField(i);
			minExtent(&min,&(bo->LSW));
			maxExtent(&max,&(bo->UNE));
		}
	} else {
		for(i=1;i<=SegList->current;i++) {
			seg=getSR(i);
			if(seg->flags&FixedFlag) {
				min=seg->LSW;
				max=seg->UNE;
				if(min.x==0)
					fprintf(OutFile,"Zero x in survey %d\n",i);
				break;
			}
		}
		for(++i;i<=SegList->current;i++) {
			seg=getSR(i);
			if(seg->flags&FixedFlag) {
				minExtent(&min,&(seg->LSW));
				maxExtent(&max,&(seg->UNE));
				if(min.x==0)
					fprintf(OutFile,"Zero x in survey %d\n",i);
			}
		}
	}

	minCellX=min.x/grid;
	minCellY=min.y/grid;
	maxCellX=max.x/grid;
	maxCellY=max.y/grid;

	cellRows=ecalloc(maxCellY-minCellY+1,sizeof(indexType ));
	cellCols=ecalloc(maxCellX-minCellX+1,sizeof(indexType ));
	CellList=lListAlloc(3,BlockLen,sizeof(struct skCell),4,nCellList);


	for(i=1,overlaps=0;i<=SegList->current;i++) {
		seg=getSR(i);
/*		fprintf(stderr,"%d\n",i);	*/
		while((sta=nextNode(seg))>=0) {
			if(locn=getFixedData(seg,sta)) {
				cellX=(locn->data.cart.x/grid)-minCellX;
				if((cellX+minCellX)>maxCellX)
					_errorAlert(UnexpectedError,0,9);
				cellY=(locn->data.cart.y/grid)-minCellY;
				if((cellY+minCellY)>maxCellY)
					_errorAlert(UnexpectedError,0,10);
/*				fprintf(stderr," (%d,%d) ",cellX,cellY); */
				if(cellRows[cellY]) {
					/* Already something on that row; have to see if cell
						already taken. */
					for(nextPtr=cellRows+cellY;*nextPtr;) {
						cell0=getCell(*nextPtr);
						if(cell0->col==cellX) {		/* already ocuppied, insert here */
							overlaps++;
/*							fprintf(stderr,"[+%d] ",overlaps); */
							break;
						} else if(cell0->col>cellX)	{ /* Gone past, insert here */
							break;
						}
						nextPtr=&(cell0->next);
					}
					/* insertCell */
					CellList->current++;
					cell=getCell(CellList->current);
					cell->next=*nextPtr;
					*nextPtr=CellList->current;
					cell->row=cellY;
					cell->col=cellX;
					cell->this.n=sta;
					cell->this.segment=seg->segment;
					cellCols[cellX]++;
				} else {
					cellRows[cellY]=++CellList->current;
					cellCols[cellX]++;
					cell=getCell(CellList->current);
					cell->next=0;
					cell->row=cellY;
					cell->col=cellX;
					cell->this.n=sta;
					cell->this.segment=seg->segment;
				}
			}
		}
	}
	
	fprintf(OutFile,"\nThe schematic grid is %d by %d, with ranges %d to %d and %d to %d\n",
		maxCellX-minCellX,maxCellY-minCellY,minCellX,maxCellX,minCellY,maxCellY);
	fprintf(OutFile,"There are %d nodes with %d overlaps.\n",CellList->current,overlaps);
	for(sta=i=0;i<maxCellX-minCellX;i++) {
		if(!cellCols[i])
			sta++;
	}
	fprintf(OutFile,"%d columns and ",sta);
	for(sta=i=0;i<maxCellY-minCellY;i++) {
		if(!cellRows[i])
			sta++;
	}
	fprintf(OutFile,"%d rows are empty.\n",sta);

	free(cellCols);
	free(cellRows);
	return;
}

#ifndef CMLmake

void doEllipse(void)
{

	int i;
	struct segmentRec *seg;
	
	for(i=1;i<=SegList->current;i++) {
		seg=getSR(i);
		if(seg->data)
			ellipse(seg,&(seg->center));
	}
	return;
}

struct cartData *ellipse(struct segmentRec *seg,struct cartData *data)
{
	/*	Calculate the area and center of gravity of a segment, assuming that it forms
		a closed figure. The result is returned in data. Each pair of points describes
		a trapezoid consiting of a rectangle topped by a right triangle. The center of 
		gravity, in both x and y, of a right triangle is 1/3 the distance from its
		heavy end. */
		
	int i;
	struct locnRec *lo0,*lo1,*heavy,*light;
	c_float dx,dy,xsums=0,ysums=0,area=0,ymin,unitAreaTri,unitAreaRect;
	
	/* Start with the wrap from last point to first point. */
	lo0 = getData(seg->data+seg->nStations-1);
	lo1 = getData(seg->data);
	for(i=0;i<seg->nStations;lo0=lo1++,i++) {
		dx=lo1->data.cart.x-lo0->data.cart.x;
		dy=fabs(lo1->data.cart.y-lo0->data.cart.y);
		heavy=(lo1->data.cart.y>lo0->data.cart.y)? light=lo0,lo1:light=lo1,lo0;
		ymin=light->data.cart.y;
		unitAreaRect=ymin*dx;
		unitAreaTri=dy*dx/2;
		area+=unitAreaRect+unitAreaTri;
		ysums+=(ymin+dy/3)*unitAreaTri+ymin*unitAreaRect/2;
		xsums+=(lo0->data.cart.x+dx/2)*unitAreaRect
			+unitAreaTri*(2*heavy->data.cart.x+light->data.cart.x)/3;
	}
	data->x=xsums/area;
	data->y=ysums/area;
	data->z=area;
	/*==============*/
	fprintf(OutFile,"$%d%.4s: Area: %6.1f Xm: %6.1f Ym: %6.1f\n",seg->Field,seg->desig,area,data->x,data->y);
	return data;
}

void reportMatches(void)
{
	struct segmentRec *seg,*seg0;
	struct fieldRec *se,*se0;
	c_float dx,dy,dz,thd;

	for(se0=getField(ThisContext->firstField);se0->nextField;se0=se) {
		se=getField(se0->nextField);
	
		for(seg0=(FieldList[se0->ThisField-MinField])?getSR(FieldList[se0->ThisField-MinField]):0;seg0;
				 seg0=(seg0->next)?getSR(seg0->next):0) {
			dz=se->z-se0->z;
			if((seg0->segment!=se0->fidSegment) && !desigCmp(seg0->desig,"A")) {
				for(seg=matchNeighbors(seg0,se);seg;seg=matchNeighbors(0,se)) {
					dx=seg->center.x-seg0->center.x;
					dy=seg->center.y-seg0->center.y;
					thd=hypot(dx,dy);
					fprintf(OutFile,"$%d%.4s1\':\t%6.1f\t%6.1f\t%6.1f\n",seg0->Field,seg0->desig,
						hypot(thd,dz),atan2(dy,dx)*RtoD,atan2(dz,thd)*RtoD);
					fprintf(OutFile,".={%6.1f,%6.1f,0}\n",seg0->center.x,seg0->center.y);
					fprintf(OutFile,"$%d%.4s1\':\t..\n",seg->Field,seg->desig);
				}
			}
		}
	}
}

struct segmentRec *matchNeighbors(struct segmentRec *seg0,struct fieldRec *se)
{
	/*	Find the next match for segment seg in section se. If seg is zero
		then contine the search from where left off. */
	
	static c_float x,y;
	static struct segmentRec *seg;
	
	if(seg0) {
		x=seg0->center.x;
		y=seg0->center.y;
		seg=getSR(FieldList[se->ThisField-MinField]);
	} else
		seg=(seg->next)?getSR(seg->next):0;
		
	for(;seg;seg=(seg->next)?getSR(seg->next):0) {
		if(!desigCmp(seg->desig,"A") && seg->segment!=se->fidSegment && x>=seg->LSW.x 
			&& x<=seg->UNE.x && y>=seg->LSW.y && y<=seg->UNE.y)
			return seg;
	}
	return 0;
}
#endif

static void checkSegmentConsistency(void)
{
	int i,j,err;
	struct segmentRec *seg, *su0;
	LinkRecPtr li;
	Str31 a;
	
	/*----------------------------------------
		Check for inconsistencies:
		1. no main data (links only)
		2. first station > 1 and 
			a. there is no tie to it
			b. the tie is not from a chain
	----------------------------------------*/
	
	fprintf(OutFile,"\n;---------------------- Consistency Check -------------------\n\n");
	for(i=1;i<=SegList->current;i++) {
		if(i!=Literals) {
			seg=getSR(i);
			if(!seg->data)
				fprintf(OutFile,"%s has no data\n",printFullSegment(seg,a));
			else if(seg->first>1) {
				err=1;
				for(j=seg->links+seg->nLocns;j<seg->links+seg->nLocns+seg->nLinks;j++) {
					li=getLink(j);
					if(li->from.n==seg->first) {
						if(li->to.n=li->from.n-1) {
							su0=getSR(li->to.segment);
							if(desigCmp(seg->desig,su0->desig)) {
								err=0;
								break;
							}
						}
					} else if(li->from.n>seg->first)
						break;
				}
				if(err) {
					for(j=seg->links+seg->nLocns+seg->nLinks;
						j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
						li=getLink(j);
						if(li->from.n==seg->first) {
							if(li->to.n=li->from.n-1) {
								su0=getSR(abs(li->to.segment));
								if(desigCmp(seg->desig,su0->desig)) {
									err=0;
									break;
								}
							}
						} else if(li->from.n>seg->first)
							break;
					}
					if(err) {
						fprintf(OutFile,"%s appears to be floating\n",printFullSta(seg->first,seg,a));
					
					}
				}
			}
		}
	}
	putc('\n',OutFile);
	return;
}

void printTripReport(int headIndex,FILE **inFile)
{
	int k,n;
	struct headerRec *head;
	struct nameRec *place,*person;
	struct dbRec *party;
	struct tm *rep;
	char *p;
	
	head=getHeader(headIndex);
	newPage();
	fprintf(OutFile,"Trip Report");
	if(head->report) {
		rep=localtime(&head->report);
		fprintf(OutFile,"%48s%.2d%.2d%.2d.%d","",rep->tm_year,rep->tm_mon+1,
			rep->tm_mday,rep->tm_sec);
	}
	fprintf(OutFile,"\nCave Research Foundation\n\n");
	if(head->accession) {
		place=getPlace(head->accession,NameList);
		fprintf(OutFile,"%59sFSB: %s\n","",place->name);
	} else if(head->FSB) 
		fprintf(OutFile,"%59sFSB: %d\n","",head->FSB);

	if(head->date) {
		p=cmltime(&head->date);
		fprintf(OutFile,"%59s%.10s%.5s\n","",p,p+19);
	}
	k=0;
	if(head->object) {
		place=getPlace(head->object,NameList);
		k=fprintf(OutFile,"Cave: %s",place->name);
	}
	if(head->location) {
		place=getPlace(head->location,NameList);
		fprintf(OutFile,"%*sArea: %s\n",39-k," ",place->name);
	} else
		putLine(k);

	if(n=head->personnel) {
		k=fprintf(OutFile,"Personnel:");
		do {
			party=getDb(n);
			n++;
			person=getName(party->what);
			if(k>(ColWidth-strlen(person->name))) {
				k=5;
				fputs("\n     ",OutFile);
			}
			k+=fprintf(OutFile," %s",person->name);
			if(party->where.item&Leader) {
				k++;
				putc('*',OutFile);
			}
			if(party->where.item&Job) {
				putc('(',OutFile);
				if(party->where.item&Book)
					{k++;putc('b',OutFile);}
				if(party->where.item&Compass)
					{k++;putc('c',OutFile);}
				if(party->where.item&PointPerson)
					{k++;putc('p',OutFile);}
				if(party->where.item&Sketch)
					{k++;putc('s',OutFile);}
				if(party->where.item&Backsights)
					k+=fprintf(OutFile," bs ");
				if(party->where.item&Foresights)
					k+=fprintf(OutFile," fs ");
				if(party->where.item&Tape)
					{k++;putc('t',OutFile);}
				fprintf(OutFile,")");
				k+=2;
			}
		} while (!(party->where.item&LastInParty));
	}
	
	/*	Project Supported:					New Survey:	*/
	if(head->project) {
		place=getPlace(head->project,NameList);
		k=fprintf(OutFile,"\nProject Supported: %s",place->name)-1;
	} else
		putLine(k);
	fprintf(OutFile,"%*sNew Survey: %.2f\n",59-k,"",((c_float)head->newlength)/10.0);
	
	/*	Special Equipment:					Resurvey:	*/
	k=0;
	if(head->equipment)
		k=printTextBlock(head->equipment,"Special Equipment: ",inFile,5,ColWidth-20,0);
	fprintf(OutFile,"%*sResurvey: %.2f\n",59-k,"",head->replacedlength/10.0);
	
	/*	Special Skills:						Time In:	*/
	k=0;
	if(head->skills)
		k=printTextBlock(head->skills,"Special Skills: ",inFile,5,ColWidth-20,0);
	fprintf(OutFile,"%*sTime In: %d:%2.2d\n",59-k,"",head->in/60,head->in%60);
	
	/*	Special Hazards:					Time Out:	*/
	k=0;
	if(head->hazards)
		k=printTextBlock(head->hazards,"Special Hazards: ",inFile,5,ColWidth-20,0);
	fprintf(OutFile,"%*sTime Out: %d:%.2d\n",59-k,"",head->out/60,head->out%60);
	
	/*	Route:								PHUG:	*/
	k=0;
	if(head->route)
		k=printTextBlock(head->route,"Route: ",inFile,7,ColWidth-20,0);
	if(head->PMUG)
		fprintf(OutFile,"%*sPHUG: %.2f\n",59-k,"",head->PMUG/60.0);
	else 
		putLine(k);
	
	if(head->data)
		printTextBlock(head->data,"Survey Data:\n",inFile,5,ColWidth,1);
	if(head->objective)
		printTextBlock(head->objective,"Objectives: ",inFile,5,ColWidth,1);
	if(head->description)
		printTextBlock(head->description,"\n\nDescription:\n",inFile,0,ColWidth,1);
	if(head->personnelreport)
		printTextBlock(head->personnelreport,"\n\nPersonnel Report:\n",inFile,0,ColWidth,1);
	return;
}

void exportTripReport(int headIndex,FILE **inFile)
{
	int n;
	struct headerRec *head;
	struct nameRec *place,*person;
	struct dbRec *party;
	char *p;

	head=getHeader(headIndex);

		/* [1] */
	if(head->date) {
		p=cmltime(&head->date);
		fprintf(OutFile,"%.6s%.5s",p+4,p+19);
	}
	putc('\t',OutFile);
		/* [2] */
	if(head->object) {
		place=getPlace(head->object,NameList);
		fprintf(OutFile,"%s",place->name);
	}
	putc('\t',OutFile);
		/* [3] */
	if(head->location) {
		place=getPlace(head->location,NameList);
		fprintf(OutFile,"%s",place->name);
	} 
	putc('\t',OutFile);
		/* [4] */
	if(head->project) {
		place=getPlace(head->project,NameList);
		fprintf(OutFile,"%s",place->name);
	} 
	putc('\t',OutFile);
		/* [5] */
	if(head->accession) {
		place=getPlace(head->accession,NameList);
		fprintf(OutFile,"%s",place->name);
	} else if(head->FSB) 
		fprintf(OutFile,"%d",head->FSB);
	putc('\t',OutFile);
		/* [6] */
	if(n=head->personnel) {
		do {
			party=getDb(n);
			n++;
			person=getName(party->what);
			
			fprintf(OutFile," %s",person->name);
			if(party->where.item&Leader) 
				putc('*',OutFile);

			if(party->where.item&Job) {
				putc('(',OutFile);
				if(party->where.item&Book)
					putc('b',OutFile);
				if(party->where.item&Compass)
					putc('c',OutFile);
				if(party->where.item&PointPerson)
					putc('p',OutFile);
				if(party->where.item&Sketch)
					putc('s',OutFile);
				if(party->where.item&Backsights)
					fprintf(OutFile," bs ");
				if(party->where.item&Foresights)
					fprintf(OutFile," fs ");
				if(party->where.item&Tape)
					putc('t',OutFile);
				fprintf(OutFile,")");
			}
		} while (!(party->where.item&LastInParty));
	}
	putc('\t',OutFile);
		/* [7] */
	fprintf(OutFile,"%.2f\t",((c_float)head->newlength)/10.0);
		/* [8] */
	if(head->equipment)
		exportTextBlock(head->equipment,inFile);
	putc('\t',OutFile);
		/* [9] */
	fprintf(OutFile,"%.2f\t",head->replacedlength/10.0);
		/* [10] */
	if(head->skills)
		exportTextBlock(head->skills,inFile);
	putc('\t',OutFile);
		/* [11] Time in */
	fprintf(OutFile,"%d:%2.2d\t",head->in/60,head->in%60);
		/* [12] Special Hazards */
	if(head->hazards)
		exportTextBlock(head->hazards,inFile);
	putc('\t',OutFile);
		/* [13] Time Out */
	fprintf(OutFile,"%d:%.2d\t",head->out/60,head->out%60);
		/* [14] Route */
	if(head->route)
		exportTextBlock(head->route,inFile);
	putc('\t',OutFile);
		/* [15] PHUG */
	if(head->PMUG)
		fprintf(OutFile,"%.2f",head->PMUG/60.0);
	putc('\t',OutFile);
		/* [16] Survey Data */
	if(head->data)
		exportTextBlock(head->data,inFile);
	putc('\t',OutFile);
		/* [17] Objectives */
	if(head->objective)
		exportTextBlock(head->objective,inFile);
	putc('\t',OutFile);
		/* [18] Description */
	if(head->description)
		exportTextBlock(head->description,inFile);
	putc('\t',OutFile);
		/* [19] Illustrations */
	putc('\t',OutFile);
		/* [20] Description */
	if(head->personnelreport)
		exportTextBlock(head->personnelreport,inFile);
	putc('\n',OutFile);
	return;
}

#ifdef MAC
#pragma segment graphics
#endif

void put32(long w, FILE *fh)
{
	fputc( (char)(w     ), fh);
	fputc( (char)(w>> 8l), fh);
	fputc( (char)(w>>16l), fh);
	fputc( (char)(w>>24l), fh);
}

int doGraphicsFile(int how)
{
	int currentField=-1,ch;
	struct segmentRec *seg;
	struct fieldRec *fr;
	FILE *GraphFile,*temp;
	Str63 a;
	char *p,*extension,*method;
	
	if(!how)
		return 0;
	extension=(how&128)? ".ps":(how&64)? ".DXF":".3d";
	if(how&4) 
		GraphFile=OutFile;
	else {
		strcpy(Scratch,IFileList[0]);
		if(p=strrchr(Scratch,'.')) 
			*p='\0'; /* Truncate extension. */
		strcat(Scratch,extension);
		p=strrchr(Scratch,'.');
		method=(how&512)? "wb":"w";
		if(!(how&32)) {
			if(!(GraphFile=fopen(Scratch,method))) {
				GraphFile=OutFile;
				errorAlert(FileOutErr);
			}
		}
	}
	/* Any introduction? */
	if(how&128) {
		fputs ("/#copies 1 def\n",GraphFile);
		fputs ("/landscape false def\n\n",GraphFile);
		fprintf(GraphFile,"/mapscale %f def\n",PSScale);
		fputs ("% /mapscale 0.288 def		% 1"" = 200' \n",GraphFile);
		fputs ("% /mapscale 0.144 def		% 1"" = 500' \n",GraphFile);
		fputs ("% /mapscale 0.072 def		% 1"" = 1000' \n",GraphFile);
		fputs ("% /mapscale 0.036 def		% 1"" = 2000' \n",GraphFile);
		fputs ("% /mapscale 0.018 def		% 1"" = 5000' \n\n",GraphFile);
		fputs("/swap {2 1 roll} def\n",GraphFile);
		fputs ("/inch {72 mul} def\n",GraphFile);
		fputs ("/sinch {inch mapscale div} def\n\n",GraphFile);
		fprintf(GraphFile,"/bbleft %.1f def\n",LSW.x);
		fprintf(GraphFile,"/bbright %.1f def\n",UNE.x);
		fprintf(GraphFile,"/bbbottom %.1f def\n",LSW.y);
		fprintf(GraphFile,"/bbtop %.1f def\n",UNE.y);
		fprintf(GraphFile,"/maxz %.1f def\n",UNE.z);
		fprintf(GraphFile,"/minz %.1f def\n\n",LSW.z);
		fputs("/zrange maxz minz sub def\n\n",GraphFile);
		fputs("/boxw 7.5 sinch def /boxh 10 sinch def\n",GraphFile);
		fputs("landscape { /boxw boxh /boxh boxw def def } if\n\n",GraphFile);
		fputs("/boxl bbleft bbright add 2 div boxw 2 div sub def\n",GraphFile);
		fputs("/boxb bbtop bbbottom add 2 div boxh 2 div sub def\n\n",GraphFile);
		fputs("/llx boxl def\n",GraphFile);
		fputs("/lly boxb def\n\n",GraphFile);
		fputs("/boxr boxl boxw add def\n",GraphFile);
		fputs("/boxt boxb boxh add def\n\n",GraphFile);
		fputs("% /color {minz sub zrange div 3.99 mul truncate 1 add 0.25 mul neg 1 add setgray} def\n",GraphFile);
		fputs("/color {pop} def\n",GraphFile);
		fputs("/pswap {4 2 roll} def\n",GraphFile);
		fputs("/pdup {1 index 1 index} def\n",GraphFile);
		fputs("/pp {pop pop} def\n",GraphFile);
		fputs("% /dw {newpath pdup moveto 8 2 roll pdup lineto 8 2 roll pswap lineto lineto closepath fill} def\n",GraphFile);
		fputs("/dw {pdup moveto 8 2 roll pswap lineto pdup moveto 6 2 roll lineto stroke} def\n",GraphFile);
		fputs("/dt {pswap moveto pdup lineto stroke} def\n\n",GraphFile);
		fputs("landscape {8 inch} {0.5 inch} ifelse 0.5 inch transform\n",GraphFile);
		fputs("mapscale dup scale\n",GraphFile);
		fputs("landscape {90 rotate} if\n",GraphFile);
		fputs("itransform lly sub swap llx sub swap translate\n\n",GraphFile);
		fputs("0 setlinewidth\n",GraphFile);
		fputs ("newpath\n",GraphFile);
		if(temp=fopen("MapFrame.ps","r")) {
			while ((ch=getc(temp)) != EOF)
				fputc(ch,GraphFile);
			fclose(temp);
		}
	} else if(how&64)
		fputs("  0\nSECTION\n  2\nENTITIES\n",GraphFile);
	else if(how&(256|512)) {
		fputs( "Survex 3D Image File\n",GraphFile);			/* File identifier */
		if(how&512)
			fputc('B',GraphFile);
		fputs( "v0.01\n",GraphFile); 							/* file format version number */
		for(ch=0;ch<NFiles;) {
			fputs(IFileList[ch],GraphFile); 			/* List of files in cmd line) */
			if(++ch<NFiles)
				fputc(' ',GraphFile);
		}
		fputc('\n',GraphFile);
		strftime(a,40,"%a,%Y.%m.%d %H:%M:%S %Z",NewTime);
		fputs(a,GraphFile);
		fputc('\n',GraphFile);
	}
		
	/* Sequence by FSB. */
	fr=(FieldLList)? getField(ThisContext->firstField):0;
	if(Literals) {
		seg=getSR(Literals);
		if(seg->flags&FixedFlag) {
			if(how&128)
				printSegmentPS(seg,how,GraphFile);
			else if(how&64) 
				printSegmentDXF(seg,how,GraphFile);
			else if(how&(256|512))
				printSegmentSVX(seg,how,GraphFile);
			else
				printSegmentS3D(seg,how,GraphFile);
		}
	}
	for(seg=nextSegment(0,&fr);seg;seg=nextSegment(seg,&fr)) {
		if(seg->flags&FixedFlag) {
			/* Any special treatment at the start of a Field? */
			if(currentField!=seg->Field) {	
				currentField=seg->Field;
				if(how&32) {
					/* With this option, each field is going into a separate file. */
					sprintf(a,"%s%d",extension,seg->Field);
					*p='\0';
					strcat(Scratch,a);
					if(!(GraphFile=fopen(Scratch,"w"))) {
						GraphFile=OutFile;
						errorAlert(FileOutErr);
					}
					if(!(how&64)) {		/* not DXF */
						sprintf(a,"%s %d",FieldName,seg->Field);
						fprintf(GraphFile,"START -3000 5000 0\n");
						fprintf(GraphFile,"TEXTRECT %d 9 %d\n",6*strlen(a),strlen(a));
						fprintf(GraphFile,"FONT 1 9 1\nSTYLE 0 0 0\n");
						fprintf(GraphFile,"TEXT %s\n",a);
					}
				}
			}
			if(!fr || seg->segment!=fr->fidSegment) {
				if(how&128)
					printSegmentPS(seg,how,GraphFile);
				else if(how&64) 
					printSegmentDXF(seg,how,GraphFile);
				else if(how&(256|512))
					printSegmentSVX(seg,how,GraphFile);
				else
					printSegmentS3D(seg,how,GraphFile);
			}
			if(how&32) 
				fclose(GraphFile);
		}
	}
	/* Any closing action */
	if(how&128)
		fputs("showpage\n",GraphFile);
	else if(how&64)
		fputs("  0\nENDSEC\n  0\nEOF\n",GraphFile);
	else if(how&512)
		put32(-1L,GraphFile); 				/* End of list marker */
	if(GraphFile!=OutFile)
		fclose(GraphFile);
	return 1;
}

#define SVXtest 1
#ifndef SVXtest
int printSegmentSVX(struct segmentRec *seg,int how,FILE *GraphFile)
{
	int sta,j;
	struct subSegRec *ss;
	struct strRec *st;
	struct locnRec *theData,*theData2;
	LinkRecPtr theLink;
	c_float ft2cm;
	long temp;

	ft2cm=100/MtoFt;
	if(seg->strings) {
		for(j=0;j<seg->nStrings;j++) {
			st=getStr(seg->strings+j);
			if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
				sta=st->first;
				for(ss = (struct subSegRec *)seg,theData=0;!theData;) {
					if(ss->first<=sta && sta<=ss->last) 
						theData=getData(ss->data+sta-ss->first);
					else if (ss->nextSubSeg)
						ss=getSS(ss->nextSubSeg);
					else
						errorAlert(NoSubSegment);	/* Fatal */
				}
				for(++sta,++theData;sta<=st->last;sta++,theData++) {
					if(!(theData->code&Virtual)) {
						if(how&256)
							 fprintf(GraphFile,"line %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
								(theData-1)->data.cart.x*ft2cm,(theData-1)->data.cart.y*ft2cm,(theData-1)->data.cart.z*ft2cm,
								theData->data.cart.x*ft2cm,theData->data.cart.y*ft2cm,theData->data.cart.z*ft2cm);
						else {
							put32(4L,GraphFile);
							temp=(long)((theData-1)->data.cart.x*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)((theData-1)->data.cart.y*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)((theData-1)->data.cart.z*ft2cm);
     						put32(temp,GraphFile);
    						put32(5L,GraphFile);
							
  							temp=(long)(theData->data.cart.x*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData->data.cart.y*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData->data.cart.z*ft2cm);
     						put32(temp,GraphFile);
 						}	
					}
				}
			}
		}
		for(j=seg->links+seg->nLocns+seg->nLinks;
			j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			theLink = getLink(j);
			if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
				theData2 = getLocn(theLink->data);
				if(!(theData2->code&Virtual)) {
					if((theData2->code&(_Point|_Rel))==Line_to) {
						theData = theData2-1;
						if(how&256)
							 fprintf(GraphFile,"line %9.2f %9.2f %9.2f %9.2f %9.2f %9.2f\n",
								theData->data.cart.x*ft2cm,theData->data.cart.y*ft2cm,theData->data.cart.z*ft2cm,
								theData2->data.cart.x*ft2cm,theData2->data.cart.y*ft2cm,theData2->data.cart.z*ft2cm);
						else {
							put32(4L,GraphFile);
  							temp=(long)(theData->data.cart.x*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData->data.cart.y*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData->data.cart.z*ft2cm);
     						put32(temp,GraphFile);

    						put32(5L,GraphFile);
							
  							temp=(long)(theData2->data.cart.x*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData2->data.cart.y*ft2cm);
     						put32(temp,GraphFile);
							temp=(long)(theData2->data.cart.z*ft2cm);
     						put32(temp,GraphFile);
						}
					}
				}
			}
		}
	}

	return 1;
}

#else

int printSegmentSVX(struct segmentRec *seg,int how,FILE *GraphFile)
{
  int sta,j;
  struct subSegRec *ss;
  struct strRec *st;
  struct locnRec *theData,*theData2;
  LinkRecPtr theLink;
  c_float ft2cm;
  long temp;
  Str31 buf;

  ft2cm=1.0/MtoFt;
  if(seg->strings)
    {
      for(j=0;j<seg->nStrings;j++)
        {
          /*
            The only reason we go through the string records is
            to check some flags that are only set there.
           */
          st=getStr(seg->strings+j);
          if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag)
            {
              /*
                i.e. Fixed (no longer relative coordinates),
                Mapped (not just a dummy string put there for a tie),
                and not Virtual (plot wanted)
                */
              sta=st->first;
              /*
                Now we have to find the data and that means going
                to the subsegment (subsurvey) records. They point
                to the main data.
                */
              for(ss = (struct subSegRec *)seg,theData=0;!theData;)
                {
                  if(ss->first<=sta && sta<=ss->last)
                    theData=getData(ss->data+sta-ss->first);
                  /* This is the right one. */
                  else if (ss->nextSubSeg)
                    ss=getSS(ss->nextSubSeg);
                  else
                    errorAlert(NoSubSegment);   /* Fatal */
                }

              /*----------------- Loop 1: through the Main Data ---------------
                The Main Data consists of sequential stations. Most data
                is here. They are stored in numberical station order, grouped
                by survey. X,y,z triplets are floats. Bits in the code field
                categorize each point as a move_to, line_to, move_to_relative,
                or line_to_relative. There should be no relative shots at
                this point (they are caused by floating surveys) but we check
                anyway.
                ----------------------------------------------------------------*/

              for(;sta<=st->last;sta++,theData++)
                {
                  if(!(theData->code&Virtual))
                    {
                      if(how&256)
                        {               /* Ascii SVX File */
                          if((theData->code&(_Point|_Rel))==Move_to)
                            fputs("move ",GraphFile);
                          else if((theData->code&(_Point|_Rel))==Line_to)
                            fputs("line ",GraphFile);
                          else
                            continue;
                          fprintf(GraphFile,"%9.2f %9.2f %9.2f\n",
                                  theData->data.cart.x*ft2cm,
                                  theData->data.cart.y*ft2cm,
                                  theData->data.cart.z*ft2cm);
                          printFullSta(sta,seg,buf);
                          fprintf(GraphFile,"name %s %9.2f %9.2f %9.2f\n",
                                  buf,
                                  theData->data.cart.x*ft2cm,
                                  theData->data.cart.y*ft2cm,
                                  theData->data.cart.z*ft2cm);
                        }
                      else
                        {                       /* This is the old binary format
 */
                          put32(4L,GraphFile);
                          temp=(long)((theData-1)->data.cart.x*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)((theData-1)->data.cart.y*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)((theData-1)->data.cart.z*ft2cm);
                          put32(temp,GraphFile);
                          put32(5L,GraphFile);
                          temp=(long)(theData->data.cart.x*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData->data.cart.y*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData->data.cart.z*ft2cm);
                          put32(temp,GraphFile);
                        }
                    }
                }
            }
        }
      /*----------------- Loop 2: through the tie data ---------------
        Ties between surveys result in a second set of coordinates for
        each tie station. Also some stations are only represented by
        ties, such as spray shots, or screwy survey practices where
        sequential numbering is not used. An example of the latter would
        be A1->A1'->A2->A2', etc. Tie data is stored separately from the
        main data and is accessed through link records.
        ----------------------------------------------------------------*/
      for(j=seg->links+seg->nLocns+seg->nLinks;
          j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++)
        {
          /* Note: starting j as shown instead of j=seg->links skips
             over the fixed locations. */
          theLink = getLink(j);
          if(theLink->to.segment>0 && theLink->data &&
             !(theLink->from.segment&Link))
            {
              theData2 = getLocn(theLink->data);
              if(!(theData2->code&Virtual))
                {
                  if((theData2->code&(_Point|_Rel))==Line_to)
                    {
                      theData = theData2-1;
                      if(how&256)
                        {       /* Ascii SVX output wanted */
                          /*
                            The from-station of a tie is always in the calling survey.
                            */
                          fprintf(GraphFile,"move %9.2f %9.2f %9.2f\n",
                                  theData->data.cart.x*ft2cm,
                                  theData->data.cart.y*ft2cm,
                                  theData->data.cart.z*ft2cm);
                          /*
                            The to-station of a tie can be in any survey. Get it
.
                            */
                          fprintf(GraphFile,"draw %9.2f %9.2f %9.2f\n",
                                  theData2->data.cart.x*ft2cm,
                                  theData2->data.cart.y*ft2cm,
                                  theData2->data.cart.z*ft2cm);
                          printFullSta(theLink->from.n,seg,buf);
                          fprintf(GraphFile,"name %s %9.2f %9.2f %9.2f\n",
                                  buf,
                                  theData->data.cart.x*ft2cm,
                                  theData->data.cart.y*ft2cm,
                                  theData->data.cart.z*ft2cm);
                          printFullSta(theLink->to.n,getSR(theLink->to.segment),buf);
                          fprintf(GraphFile, "name %s %9.2f %9.2f %9.2f\n",
                                  buf,
                                  theData2->data.cart.x*ft2cm,
                                  theData2->data.cart.y*ft2cm,
                                  theData2->data.cart.z*ft2cm);
                        }
                      else
                        {               /* Old style binary format */
                          put32(4L,GraphFile);
                          temp=(long)(theData->data.cart.x*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData->data.cart.y*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData->data.cart.z*ft2cm);
                          put32(temp,GraphFile);
                          put32(5L,GraphFile);
                          temp=(long)(theData2->data.cart.x*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData2->data.cart.y*ft2cm);
                          put32(temp,GraphFile);
                          temp=(long)(theData2->data.cart.z*ft2cm);
                          put32(temp,GraphFile);
                        }
                    }
                }
            }
        }
    }
  return 1;
}
#endif

int printSegmentS3D(struct segmentRec *seg,int how,FILE *GraphFile)
{
	int sta,j;
	struct subSegRec *ss;
	struct strRec *st;
	struct locnRec *theData,*theData2;
	LinkRecPtr theLink;

	if(seg->strings) {
		if(how&8) {
			/* Output Bounding Box. */
			fprintf(GraphFile,"START %6.1f %6.1f 2000\n",seg->LSW.x,seg->LSW.y);
			fprintf(GraphFile,"LINE  %6.1f %6.1f 2000\n",seg->LSW.x,seg->UNE.y);
			fprintf(GraphFile,"LINE  %6.1f %6.1f 2000\n",seg->UNE.x,seg->UNE.y);
			fprintf(GraphFile,"LINE  %6.1f %6.1f 2000\n",seg->UNE.x,seg->LSW.y);
			fprintf(GraphFile,"LINE  %6.1f %6.1f 2000\n",seg->LSW.x,seg->LSW.y);
		}
		if(how&16) {
			/* Output Labels. */
			fprintf(GraphFile,"START %6.1f %6.1f 0\n",(seg->LSW.x+seg->UNE.x)/2,(seg->LSW.y+seg->UNE.y)/2);
			fprintf(GraphFile,"TEXTRECT %d 6 %d\n",4*strlen(seg->desig),strlen(seg->desig));
			fprintf(GraphFile,"FONT 1 9 1\nSTYLE 0 0 0\n");
			fprintf(GraphFile,"TEXT %.4s\n",seg->desig);
		}
		
		for(j=0;j<seg->nStrings;j++) {
			st=getStr(seg->strings+j);
			if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
				sta=st->first;
				for(ss = (struct subSegRec *)seg,theData=0;!theData;) {
					if(ss->first<=sta && sta<=ss->last) 
						theData=getData(ss->data+sta-ss->first);
					else if (ss->nextSubSeg)
						ss=getSS(ss->nextSubSeg);
					else
						errorAlert(NoSubSegment);	/* Fatal */
				}
				if(how&2 && st->code & DeadEnd) 
					fprintf(GraphFile,"PEN 65535 65535 0\n");	/* Yellow */
				else if(how&2)
					fprintf(GraphFile,"PEN 0 0 0\n");
				fprintf(GraphFile,"START %6.1f %6.1f %6.1f\n",
					theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
				for(++sta,++theData;sta<=st->last;sta++,theData++)
					fprintf(GraphFile,"LINE  %6.1f %6.1f %6.1f\n",
						theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			}
		}
		for(j=seg->links+seg->nLocns+seg->nLinks;
			j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			theLink = getLink(j);
			if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
				theData2 = getLocn(theLink->data);
				if(!(theData2->code&Virtual)) {
					if(theData2->code&CloseShape) {
						fprintf(GraphFile,"LINE  ");
					} else if((theData2->code&(_Point|_Rel))==Line_to) {
						theData = theData2-1;
						if(how&2 && theData2->code&DeadEnd)
							fprintf(GraphFile,"PEN 65535 32768 0\n");	/* Orange */
						else if(how&2)
							fprintf(GraphFile,"PEN 65535 0 0\n");	/* Red, for ties. */
						fprintf(GraphFile,"START %6.1f %6.1f %6.1f\nLINE  ",
							theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
					}
					fprintf(GraphFile,"%6.1f %6.1f %6.1f\n",
						theData2->data.cart.x,theData2->data.cart.y,theData2->data.cart.z);
				}
			}
		}
	}
	return 1;
}

int printSegmentPS(struct segmentRec *seg,int how,FILE *GraphFile)
{
	int sta,j;
	int x0,y0,x1,y1,d,draw;
	int empty,lastAzi=0,pop;
	struct subSegRec *ss;
	struct strRec *st;
	struct locnRec *theData,*theData2;
	LinkRecPtr theLink;
	struct dimData *theDims,*theDims2;
	c_float xAzi,xAzi2;
	
	fprintf(GraphFile,"%% Survey $%d%.4s\n",seg->Field,seg->desig);
	fprintf(GraphFile,"%.1f color\n",(seg->UNE.z+seg->LSW.z)/2);
	if(seg->strings) {
		if(how&8) {
			/* Output Bounding Box. */
#if 0
			x0=seg->LSW.x*PSScale;
			y0=seg->LSW.y*PSScale;
			y1=seg->UNE.y*PSScale;
			x1=seg->UNE.x*PSScale;
#endif
			x0=seg->LSW.x;
			y0=seg->LSW.y;
			y1=seg->UNE.y;
			x1=seg->UNE.x;
			fprintf(GraphFile,"%d %d moveto ",x0,y0);
			fprintf(GraphFile,"%d %d lineto ",x0,y1);
			fprintf(GraphFile,"%d %d lineto ",x1,y1);
			fprintf(GraphFile,"%d %d lineto ",x1,y0);
			fprintf(GraphFile,"%d %d lineto\n",x0,y0);
		}
		if(how&16) {
			/* Output Labels. */

			fprintf(GraphFile,"START %6.1f %6.1f 0\n",(seg->LSW.x+seg->UNE.x)/2,(seg->LSW.y+seg->UNE.y)/2);
			fprintf(GraphFile,"TEXTRECT %d 6 %d\n",4*strlen(seg->desig),strlen(seg->desig));
			fprintf(GraphFile,"FONT 1 9 1\nSTYLE 0 0 0\n");
			fprintf(GraphFile,"TEXT %.4s\n",seg->desig);
		}
		if(DimList) {
			empty=0;
			for(j=0;j<seg->nStrings;j++) {
				st=getStr(seg->strings+j);
				if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
					sta=st->first;
					empty=1;
					pop=0;
					for(ss = (struct subSegRec *)seg,theData=0;!theData;) {
						if(ss->first<=sta && sta<=ss->last) {
							theData=getData(ss->data+sta-ss->first);
							theDims=getDims(d=LocnList->current+ss->data+sta-ss->first);
						} else if (ss->nextSubSeg)
							ss=getSS(ss->nextSubSeg);
						else
							errorAlert(NoSubSegment);	/* Fatal */
					}
					
					lastAzi=theDims->azi;
					fprintf(GraphFile,"%% Stations %d - %d\n",st->first,st->last);
				
					xAzi=(theDims->azi/10.0 + 90.0)*DtoR;
#if 0
					/* moveto right wall */
					fprintf(GraphFile,"%d %d moveto",
						(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi)),
						(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi)));
					/* lineto left wall */
					fprintf(GraphFile," %d %d lineto",
						(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi)),
						(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi)));
					/* moveto station */
					fprintf(GraphFile," %d %d moveto",
						(int)theData->data.cart.x,(int)theData->data.cart.y);
#endif
					if(draw = theDims->r+theDims->l) {
						/* save right wall */
						x0=(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi));
						y0=(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi));
						/* save left wall */
						x1=(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi));
						y1=(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi));
						fprintf(GraphFile," %d %d %d %d",x0,y0,x1,y1);
					} else {
						x0=x1=theData->data.cart.x;
						y0=y1=theData->data.cart.y;
						fprintf(GraphFile," %d %d",x0,y0);
					}
#if 0
					if(draw = theDims->r+theDims->l) {
						/* save right wall */
						x0=(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi));
						y0=(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi));
						/* save left wall */
						x1=(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi));
						y1=(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi));
					} else {
						x0=x1=theData->data.cart.x;
						y0=y1=theData->data.cart.y;
					}
#endif
					for(++sta,++theData,++d;sta<=st->last;sta++,theData++,d++)
						if(!(theData->code&Virtual)) {
							theDims=getDims(d);
							/* calculate half-angle */
							xAzi=(abs(lastAzi-theDims->azi)<1800)? 
									(lastAzi+theDims->azi)/20.0 :
									(lastAzi+theDims->azi)/20.0 +180 ;
							xAzi=(xAzi + 90.0)*DtoR;
							lastAzi=theDims->azi;
#if 0
							/* lineto station, drawing survey line. */
							fprintf(GraphFile," %d %d lineto",
								(int)theData->data.cart.x,(int)theData->data.cart.y);
							/* moveto right wall */
							fprintf(GraphFile," %d %d moveto",
								(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi)),
								(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi)));
							/* lineto left wall */
							fprintf(GraphFile," %d %d lineto",
								(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi)),
								(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi)));
							/* moveto station */
							fprintf(GraphFile," %d %d moveto",
								(int)theData->data.cart.x,(int)theData->data.cart.y);
#endif
							if(draw || (theDims->l+theDims->r)) {
								/* draw right wall. */
								if(!draw)
									fputs(" pdup",GraphFile);
								fprintf(GraphFile," %d %d",
									x0=(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi)),
									y0=(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi)));
							
								/* draw left wall */
								fprintf(GraphFile," %d %d dw",
									x1=(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi)),
									y1=(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi)));
								pop=1;
							} else {
								/* moveto station */
								fprintf(GraphFile," %d %d dt",
									x0=x1=(int)theData->data.cart.x,y0=y1=(int)theData->data.cart.y);
							}
							draw=theDims->l+theDims->r;
#if 0
							if(draw || (theDims->l+theDims->r)) {
								/* draw right wall. */
								fprintf(GraphFile," %d %d moveto",x0,y0);
								fprintf(GraphFile," %d %d lineto",
									x0=(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi)),
									y0=(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi)));
							
								/* draw left wall */
								fprintf(GraphFile," %d %d moveto",x1,y1);
								fprintf(GraphFile," %d %d lineto",
									x1=(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi)),
									y1=(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi)));
							} else {
								/* moveto station */
								fprintf(GraphFile," %d %d moveto",x0,y0);
								fprintf(GraphFile," %d %d lineto",
									x0=x1=(int)theData->data.cart.x,y0=y1=(int)theData->data.cart.y);
							}
							draw=theDims->l+theDims->r;
#endif
						}
					if(pop)
						fprintf(GraphFile," pp pp\n");
					else
						fputs("\n",GraphFile);
				}
			}
			for(j=seg->links+seg->nLocns+seg->nLinks;
				j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
				theLink = getLink(j);
				if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
					theData2 = getLocn(theLink->data);
					theDims2 = getDims(theLink->data);
					if(!(theData2->code&Virtual)) {
						empty=1;
						if(theData2->code&CloseShape) {
							;
						} else if((theData2->code&(_Point|_Rel))==Line_to) {
							theData = theData2-1;
							theDims = getDims(theLink->data-1);
									
							if(theDims->r||theDims->l||theDims2->l||theDims2->r) {
								xAzi=(theDims->azi/10.0 + 90.0)*DtoR;
								xAzi2=(abs(theDims2->azi-theDims->azi)<1800)? 
										(theDims2->azi+theDims->azi)/20.0 :
										(theDims2->azi+theDims->azi)/20.0 +180 ;
								xAzi2=(xAzi2 + 90.0)*DtoR;
								/* push from-station, right then left wall */
								fprintf(GraphFile,"%d %d %d %d",
									(int)(theData->data.cart.x+(theDims->r/10)*sin(xAzi)),
									(int)(theData->data.cart.y+(theDims->r/10)*cos(xAzi)),
									(int)(theData->data.cart.x+(theDims->l/10)*-sin(xAzi)),
									(int)(theData->data.cart.y+(theDims->l/10)*-cos(xAzi)));
								/* push to-station, right then left wall */
								fprintf(GraphFile," %d %d %d %d dw pp pp\n",
									(int)(theData2->data.cart.x+(theDims2->r/10)*sin(xAzi2)),
									(int)(theData2->data.cart.y+(theDims2->r/10)*cos(xAzi2)),
									(int)(theData2->data.cart.x+(theDims2->l/10)*-sin(xAzi2)),
									(int)(theData2->data.cart.y+(theDims2->l/10)*-cos(xAzi2)));
							} else {
								fprintf(GraphFile,"%d %d moveto",
									(int)theData->data.cart.x,(int)theData->data.cart.y);
								fprintf(GraphFile," %d %d lineto\n",
									(int)theData2->data.cart.x,(int)theData2->data.cart.y);
							}
						}
					}
				}
			}
/*			fputs("0 setlinewidth\n",GraphFile);	*/
		} else {
			empty=0;
			for(j=0;j<seg->nStrings;j++) {
				st=getStr(seg->strings+j);
				if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
					sta=st->first;
					empty=1;
					for(ss = (struct subSegRec *)seg,theData=0;!theData;) {
						if(ss->first<=sta && sta<=ss->last) 
							theData=getData(ss->data+sta-ss->first);
						else if (ss->nextSubSeg)
							ss=getSS(ss->nextSubSeg);
						else
							errorAlert(NoSubSegment);	/* Fatal */
					}
					fprintf(GraphFile,"%d %d moveto",
						(int)theData->data.cart.x,(int)theData->data.cart.y);
					for(++sta,++theData;sta<=st->last;sta++,theData++)
						if(!(theData->code&Virtual))
							fprintf(GraphFile," %d %d lineto",
								(int)theData->data.cart.x,(int)theData->data.cart.y);
					fprintf(GraphFile,"\n");
				}
			}
			for(j=seg->links+seg->nLocns+seg->nLinks;
				j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
				theLink = getLink(j);
				if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
					theData2 = getLocn(theLink->data);
					if(!(theData2->code&Virtual)) {
						empty=1;
						if(theData2->code&CloseShape) {
							;
						} else if((theData2->code&(_Point|_Rel))==Line_to) {
							theData = theData2-1;
							fprintf(GraphFile,"%d %d moveto",
								(int)theData->data.cart.x,(int)theData->data.cart.y);
							fprintf(GraphFile," %d %d lineto\n",
								(int)theData2->data.cart.x,(int)theData2->data.cart.y);
						}
					}
				}
			}
		}
		if(empty)
			fprintf(GraphFile,"stroke\n");
	}
	return 1;
}

int printSegmentDXF(struct segmentRec *seg,int how,FILE *DXFFile)
{
	int sta,j;
	struct subSegRec *ss;
	struct strRec *st;
	struct locnRec *theData,*theData2;
	LinkRecPtr theLink;

	if(seg->strings) {
		for(j=0;j<seg->nStrings;j++) {
			st=getStr(seg->strings+j);
			if((st->code&(FixedFlag|UnMapped|Virtual))==FixedFlag) { /* FixedFlag, Mapped, and not Virtual */
				sta=st->first;
				for(ss = (struct subSegRec *)seg,theData=0;!theData;) {
					if(ss->first<=sta && sta<=ss->last) 
						theData=getData(ss->data+sta-ss->first);
					else if (ss->nextSubSeg)
						ss=getSS(ss->nextSubSeg);
					else
						errorAlert(NoSubSegment);	/* Fatal */
				}
				fprintf(DXFFile,"  0\nLINE\n  8\n0\n 10\n%.1f\n 20\n%.1f\n 30\n%.1f\n",
					theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
				for(++sta,++theData;sta<st->last;sta++,theData++) {
					fprintf(DXFFile," 11\n%.1f\n 21\n%.1f\n 31\n%.1f\n",
						theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
					fprintf(DXFFile,"  0\nLINE\n  8\n0\n 10\n%.1f\n 20\n%.1f\n 30\n%.1f\n",
						theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
				}
				fprintf(DXFFile," 11\n%.1f\n 21\n%.1f\n 31\n%.1f\n",
					theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
			}
		}
		for(j=seg->links+seg->nLocns+seg->nLinks;
			j<seg->links+seg->nLocns+seg->nLinks+seg->nTies;j++) {
			theLink = getLink(j);
			if(theLink->to.segment>0 && theLink->data && !(theLink->from.segment&Link)) {
				theData2 = getLocn(theLink->data);
				if(!(theData2->code&Virtual)) {
					if((theData2->code&(_Point|_Rel))==Line_to) {
						theData = theData2-1;
						fprintf(DXFFile,"  0\nLINE\n  8\n0\n 10\n%.1f\n 20\n%.1f\n 30\n%.1f\n",
							theData->data.cart.x,theData->data.cart.y,theData->data.cart.z);
						fprintf(DXFFile," 11\n%.1f\n 21\n%.1f\n 31\n%.1f\n",
							theData2->data.cart.x,theData2->data.cart.y,theData2->data.cart.z);
					}
				}
			}
		}
	}

	return 1;
}
