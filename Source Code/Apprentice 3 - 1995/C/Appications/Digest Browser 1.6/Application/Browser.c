/*
 * Browser.c
 *
 * 9/5/91
 * Manuel A. Perez
 *
 * 9/5/91 Added a simple mac front end to the skim-digest program
 * from sumex.
 *
 * 9/6/91 Liked it so much, that I decided this was worth making
 * a TCL application out of it.
 *
 * 2/29/92 Reorganized the parser.  More error checks.  It now
 * detects Date, From, and Subject.  All three of them are kept
 * in the directory structure.  Also, this file can be tested
 * without the full application.
 *
 * 3/9/92 Fixed problem with BuildHeadMessage.
 * Based on: skim-digest.c (stored in info-mac)
 * version 1.0, 28 Feb 91, by Mike Gleason, NCEMRSoft.
 *
 * 8/29/92 Added the functionality to mark items
 */
 
 
#include <stdio.h>
#include <string.h>				// str-stuff
#include "Browser.h"
#include <Exceptions.h>

// Constants
#define KEY "From:"
#define KEYLEN 5

// Prototypes
int select_index_line(void);
int line_type(char *line);
void copystr(register char *to, register char *from);
Boolean find_first_header(FILE *fp);
Boolean find_next_header(FILE *fp, fpos_t *last_blank_line);
void get_article_info(BrowserItemPtr p);
Boolean BuildHeadMessage(BrowserDir *dir);

// Uncomment the next line to test this program without TCL.
//#define MAIN

#ifdef MAIN
#define REPORT_ERROR(MSG)		fprintf(stderr, MSG)
#define REPORT_F_NOT_OPENED(FNAME) 		fprintf(stderr, "skim-digest: \"%s\" could not be opened.\n", FNAME)
#define REPORT_EOF() fprintf(stderr, "skim-digest: Unexpected EOF\n")
#define REPORT_MEMORY_ALOC() fprintf(stderr, "skim-digest: error allocating memory\n")
#define REPORT_IMPROPER_FMT()	fprintf(stderr, "skim-digest: improper file format\n")
#else

#define REPORT_ERROR(MSG)			FailMemError()
#define REPORT_F_NOT_OPENED(FNAME)	Failure(1000, SpecifyMsg(1025, 1))
#define REPORT_EOF() 				Failure(1001, SpecifyMsg(1025, 2))
#define REPORT_MEMORY_ALOC() 		FailMemError()
#define REPORT_IMPROPER_FMT()		Failure(1000, SpecifyMsg(1025, 3))

#endif
 
#ifdef MAIN
main()
{
BrowserDir	dir;
SFTypeList typeList;
SFReply theReply;
Point at = {40, 40};
BrowserItemPtr list, p, q;


	typeList[0] = 'TEXT';
	SFGetFile(at, "\pGet File", NULL, 1, typeList, NULL, &theReply);
	while (theReply.good) {
		brInitDir(&dir);
		PtoCstr(theReply.fName);
		strcpy(dir.fname, (char *)&theReply.fName);
		CtoPstr(theReply.fName);
		dir.vRefNum = theReply.vRefNum;
	
		if (BuildBrowserIndex(&dir)) {

			p = dir.topItem;
			while (p) {
				printf("Date: [%s]\n", p->date);
				printf("From: [%s]\n", p->from);
				printf("Subject: [%s]\n", p->subject);
				printf("\n\n");
				p = p->next;
			}
			//fclose(dir->fp);
		}
		SFGetFile(at, "\pGet File", NULL, 1, typeList, NULL, &theReply);
	}
}
#endif

/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
void	brInitDir(BrowserDirPtr dir)
{
	if (dir) {
		dir->fname[0] = '\0';
		dir->fp = NULL;
		dir->numArticles = 0;
		dir->markArticles = 0;
		dir->topItem = NULL;
	}
}

/*----------------------------------------------------------------------------*/
long	brItemCount(BrowserDirPtr dir)
{
	if (dir)
		return dir->numArticles;
	else
		return 0;
}

/*----------------------------------------------------------------------------*/
long	brMarkCount(BrowserDirPtr dir)
{
	if (dir)
		return dir->markArticles;
	else
		return 0;
}

/*----------------------------------------------------------------------------*/
void	brInitItem(BrowserItemPtr item)
{
	if (item) {
		item->next = NULL;
		item->owner = NULL;
		item->fp = NULL;
		item->date[0] = '\0';
		item->from[0] = '\0';
		item->subject[0] = '\0';
		item->composite[0] = '\0';
		item->startAt = -1;
		item->endAt = -1;
		item->marked = false;
	}
}

/*----------------------------------------------------------------------------*/
void	brSetOwner(BrowserItemPtr item, BrowserDirPtr dir)
{
	if (item)
		item->owner = dir;
}

/*----------------------------------------------------------------------------*/
BrowserDirPtr	brGetOwner(BrowserItemPtr item)
{
	if (item)
		return (BrowserDirPtr)item->owner;
	else
		return NULL;
}

/*----------------------------------------------------------------------------*/
void	brSetFP(BrowserItemPtr item, FILE *file)
{
BrowserDirPtr owner;

	if (item) {
		item->fp = file;
		
		// update owner
		if (owner = brGetOwner(item))
			owner->fp = file;
	}
}

/*----------------------------------------------------------------------------*/
FILE	*brGetFP(BrowserItemPtr item)
{
	if (item)
		return item->fp;
	else
		return NULL;
}

/*----------------------------------------------------------------------------*/
void	brToggleMark(BrowserItemPtr item)
{
Boolean old;
BrowserDirPtr owner;

	if (item) {
		old = item->marked;
		item->marked = !(item->marked);

		// update count on directory
		owner = brGetOwner(item);
		if (old) {
			// decrement count
			owner->markArticles--;
		}
		else {
			// increment count
			owner->markArticles++;
		}
	}
}

/*----------------------------------------------------------------------------*/
void	brSetMark(BrowserItemPtr item, Boolean mark)
{
Boolean old;
BrowserDirPtr owner;

	if (item) {
		old = item->marked;
		item->marked = mark;

		// update count on directory
		if (old && !mark) {
			// decrement count
			owner->markArticles--;
		}
		else if (!old && mark) {
			// increment count
			owner->markArticles++;
		}
	}
}

/*----------------------------------------------------------------------------*/
Boolean	brGetMark(BrowserItemPtr item)
{
	if (item)
		return item->marked;
	else
		return false;
}

/*----------------------------------------------------------------------------*/
void	brSetStart(BrowserItemPtr item, long start)
{
	if (item)
		item->startAt = start;
}

/*----------------------------------------------------------------------------*/
long	brGetStart(BrowserItemPtr item)
{
	if (item)
		return item->startAt;
	else
		return -1;
}

/*----------------------------------------------------------------------------*/
void	brSetEnd(BrowserItemPtr item, long end)
{
	if (item)
		item->endAt = end;
}

/*----------------------------------------------------------------------------*/
long	brGetEnd(BrowserItemPtr item)
{
	if (item)
		return item->endAt;
	else
		return -1;
}

/*----------------------------------------------------------------------------*/
void	brSetNext(BrowserItemPtr item, BrowserItemPtr nxt)
{
	if (item)
		item->next = nxt;
}

/*----------------------------------------------------------------------------*/
BrowserItemPtr	brGetNext(BrowserItemPtr item)
{
	if (item)
		return item->next;
}

/*----------------------------------------------------------------------------*/
Boolean equalstr(register char *s, register char *t, int n)
{
register int i;

	for (i = 0; i < n; i++, s++, t++)
		if (*s != *t)
			return false;
		else if (*s == '\0' || *t == '\0')
			return false;

	return true;
}   /* equalstr */

/*----------------------------------------------------------------------------*/
int select_index_line(void)
{
	return line_type("From:");
}

/*----------------------------------------------------------------------------*/
int line_type(char *line)
{
register int i = 1;
Str255 string;

	do {
		GetIndString(string, 132, i);
		if (string[0] && equalstr((char *)&string[1], line, string[0]))
			return i;
	} while (string[0] != 0);
	return 0;
}

/*----------------------------------------------------------------------------*/
void copystr(register char *to, register char *from)
{

	if (!(to && from)) return;

	// advance string pointer until ':'
	for (; *from && *from != ':'; from++)
		;/* nothing */

	// skip over white spaces
	if (*from)
		for (from++; *from && *from == ' '; from++)
			;/* nothing */

	strcpy(to, from);
	if (*from)
		to[strlen(to) - 1] = '\0';		// remove nl at end of line
}

/*----------------------------------------------------------------------------*/
/*
 * The format of the file has:
 *
 * A message from the Info-mac administrator at the top,
 * Followed by messages with the following format:
 *
 *	<blank line>
 *	Date:
 *	From:
 *	Subject:		<< this line seems optional
 *	<blank line>
 *	<message>		<< will include blank lines
 *	<blank line>
 *	------			<< dash line separates the messages
 *
 * But, we can only be garanteed that the 'From' appears at the
 * beginning of the line only when it is part of the header.
 *
 * So to parse the header we will do the following:
 *  1) Search for a line beginning with From, while
 *	   remembering the last blank line seen.
 *	2) Once a From line is found, look ahead for the
 *	   next blank line.
 *	3) The text between the first blank line, and the
 *	   second blank line contains the header for the
 *	   mail message.  Search this chunck of text to
 *	   get the Date, From, and Subject.
 *
 */

// Look from the beginning of the file and determine if the
// first few lines contains what looks like a header for this
// type of file. (Looking at the previous comments, the first
// header should not have the leading blank lines).

Boolean find_first_header(FILE *fp)
{
char buffer[200];

	fseek(fp, 0, SEEK_SET);		// set file position
	while (fgets(buffer, sizeof(buffer), fp)) {

		if (equalstr(buffer, "\n", 1))	// found an empty line
			//return false;
			continue;

		else if (equalstr(buffer, KEY, KEYLEN))
			// found a From:
			return true;

// JRB addition - allow first index to start with anything
		else // if (equalstr(buffer, "C.S.M.P. Digest", 15))
			return true;
// end JRB addition
	}

	return false;
}

// Look ahead on the file for the beginning of the next
// message file.  Return the file position of where the
// next message begins.
Boolean find_next_header(FILE *fp, fpos_t *last_blank_line)
{
fpos_t next_message;
char buffer[200];
Boolean found_blank_line;
Boolean empty_line;
Boolean found_header;

	fgetpos(fp, &next_message);		// save file position
	*last_blank_line = next_message;

	found_blank_line = false;
	found_header = false;
	while (fgets(buffer, sizeof(buffer), fp)) {

		empty_line = equalstr(buffer, "\n", 1);
		if (!found_blank_line && empty_line) {	// found an empty line
			*last_blank_line = next_message;
			found_blank_line = true;
		}

		else if (found_blank_line && equalstr(buffer, KEY, KEYLEN)) {
			// found a From: after a blank line and before another
			// blank line
			found_header = true;
			break;			// get out
		}

		else if (found_blank_line && empty_line) {
			*last_blank_line = next_message;
			found_blank_line = true;
		}

		fgetpos(fp, &next_message);
	}

	return found_header;
}

/*----------------------------------------------------------------------------*/
void get_article_info(BrowserItemPtr p)
{
char  buffer[128];
fpos_t start;
// JRB addition
char *compositePtr;
char separator[10] = "; ";
char firstLine[MAX_STRING];
short subLength,dateLength,fromLength,sepLength;
// end JRB addition

	fgetpos(brGetFP(p), &start);

	p->from[0] = p->date[0] = p->subject[0] = '\0';
// JRB addition - initialize string for new index option
	p->composite[0] = '\0';

	// skip over blank lines
	do {
		fgets(buffer, sizeof(buffer), brGetFP(p));
	} while(equalstr(buffer, "\n", 1));

// JRB addition - save first line in case we want to use it for the index if we find nothing better
	strcpy(firstLine,buffer);	
	firstLine[strlen(firstLine) - 1] = '\0';		
// end JRB addition


	// process lines until another blank line comes along
	while (!equalstr(buffer, "\n", 1))  {
		if (equalstr(buffer, "From:", 5))
			copystr(p->from, buffer);				// Copy From:
		else if (equalstr(buffer, "Date:", 5))
			copystr(p->date, buffer);				// Copy Date:
		else if (equalstr(buffer, "Subject:", 8))
			copystr(p->subject, buffer);			// Copy Subject:
		fgets(buffer, sizeof(buffer), brGetFP(p));
	}

// JRB addition - allow first index to be something other than "From:"
	if ((p->from[0] == '\0') && (p->date[0] == '\0') && (p->subject[0] == '\0')) 
		strcpy(p->subject,firstLine);
// end JRB addition

	if (p->from[0] == '\0')		strcpy(p->from, "-");
	if (p->date[0] == '\0')		strcpy(p->date, "-");
	if (p->subject[0] == '\0') 	strcpy(p->subject, "-");

// JRB addition - new index option
	sepLength = strlen(separator);
	if ((subLength=strlen(p->subject))+sepLength < MAX_STRING) {
		strcpy(compositePtr=p->composite, p->subject);
		if (subLength+(fromLength=strlen(p->from))+sepLength < MAX_STRING) {
			if (fromLength>1) {
				strcpy(compositePtr+=subLength, separator);
				strcpy(compositePtr+=sepLength, p->from);
			}
			if (subLength+fromLength+(dateLength=strlen(p->date))+2*sepLength < MAX_STRING) {
				if (dateLength>1) {
					strcpy(compositePtr+=fromLength, separator);
					strcpy(compositePtr+=sepLength, p->date);
				}
			}
		}
	}
// end JRB addition
}

/*----------------------------------------------------------------------------*/
Boolean BuildBrowserIndex(BrowserDir *dir)
{
char  buffer[128];
short articleNum = 0;
BrowserItemPtr p, q;
fpos_t this_article;
fpos_t next_article;
short saveVol;

	GetVol(NULL, &saveVol);
	SetVol(NULL, dir->vRefNum);
	if (!(dir->fp = fopen(dir->fname, "r"))) {
		SetVol(NULL, saveVol);
		REPORT_F_NOT_OPENED(dir->fname);
		return false;
	}
	SetVol(NULL, saveVol);

	dir->topItem = NULL;	// initialize head pointer

	if (!BuildHeadMessage(dir))	// if error, return
		return false;

	q = p = dir->topItem;	// initialize temp pointers (q, p)
	articleNum = 1;			// we already have one article

	// Find the beginning of the next article as an indicator
	// of the end of this one. 'find_next_header' returns TRUE
	// if it finds the beginning of the next header
	this_article = dir->topItem->endAt;
	fsetpos(dir->fp, &this_article);
	while (find_next_header(dir->fp, &next_article)) {

		// found new one; store new information
		if (q = Allocate(BrowserItem)) {

			brInitItem(q);
			brSetOwner(q, dir);
			// keep file pointer at every node, duplication of effort
			brSetFP(q, dir->fp);	//q->fp = dir->fp;	// but worth it (see CDisplayText)

			// Set header to the beginning of this article, then
			// get the header information, and find the end of it
			fsetpos(dir->fp, &this_article);			// start
			get_article_info(q);	// store Date, From and Subject
			find_next_header(dir->fp, &next_article);

			brSetStart(q, this_article);	//q->startAt = this_article;
			brSetEnd(q, next_article);		//q->endAt = next_article;
			brSetMark(q, false);			//q->marked = false;
			brSetNext(q, NULL);				//q->next = NULL;

			// set info for previous node
			if (p) {
				//p->endAt = q->startAt;	// ends at beginning of current one
				brSetEnd(p, brGetStart(q));
				//p->next = q;			// set sequential link
				brSetNext(p, q);
			}

			// copy pointer over
			p = q;

			// increment article count
			articleNum++;
			this_article = next_article;
			fsetpos(dir->fp, &next_article);
		}

		else {
			REPORT_ERROR("skim-digest: error allocating memory\n");
			return false;
		}
	}

	// when done, set the end of the link
	if (p) {
		brSetEnd(p, next_article);		//p->endAt = next_article;
		brSetNext(p, NULL);				//p->next = NULL;
	}
	dir->numArticles = articleNum;

	return true;
}  /* BuildBrowserIndex */

/*----------------------------------------------------------------------------*/
Boolean BuildHeadMessage(BrowserDir *dir)
{
char  buffer[128];
fpos_t start;
fpos_t end;
Boolean found_new_line = false;

	// Go through the digest until a line KEY follows a blank line
	fseek(dir->fp, 0, SEEK_SET);	// set file position
	fgetpos(dir->fp, &start);		// save file position
	if (!find_first_header(dir->fp)) {
		REPORT_IMPROPER_FMT();
		return false;
	}
	else if (dir->topItem = Allocate(BrowserItem)) {
		brInitItem(dir->topItem);
		brSetOwner(dir->topItem, dir);
		brSetFP(dir->topItem, dir->fp);	// keep file pointer at every node,
			// duplication of effort, but worth it (see CDisplayText)

		fsetpos(dir->fp, &start);	// set file position
		find_next_header(dir->fp, &end);	// get end of article

		fsetpos(dir->fp, &start);	// set file position
		get_article_info(dir->topItem);		// get article info

		// this is a hack to allow blank lines at the top of the
		// file.
		if (start == end)
			find_next_header(dir->fp, &end);	// get real end of article
	
		brSetStart(dir->topItem, start);		//dir->topItem->startAt = start;
		brSetEnd(dir->topItem, end);			//dir->topItem->endAt = end;
		brSetMark(dir->topItem, false);			//dir->topItem->marked = false;
		brSetNext(dir->topItem, NULL);			//dir->topItem->next = NULL;
		
		// leave pointer at the end of the header
		fsetpos(dir->fp, &end);
		return true;
	}
	else {
		REPORT_MEMORY_ALOC();
		return false;
	}

}  /* BuildHeadMessage */

