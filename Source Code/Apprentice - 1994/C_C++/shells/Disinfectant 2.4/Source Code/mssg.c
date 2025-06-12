/*______________________________________________________________________

	mssg.c - Virus Message Module.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This module contains the scan begin and end routines, and a variety
	of helper routines for issuing messages to the report.  It also
	maintains the three counters in the main window.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rep.h"
#include "vol.h"
#include "rez.h"
#include "glob.h"
#include "mssg.h"

#pragma segment mssg


/*______________________________________________________________________

	Global Types and Variables.
_____________________________________________________________________*/


typedef struct TagTableEntry {
	short		reportLineNum;
	short		tag;
} TagTableEntry;


static TagTableEntry		(**TagTable)[] = nil;	/* handle to tag table */
static short				NTags = 0;					/* number of tags */
static short				NAlloc = 0;					/* number of allocated
																	tag table entries */
static short				CounterTop;					/* top coord of counters */
static short				CounterRight;				/* right coord of counters */
static Str255				Line1;						/* unplugged message line */
static Str255				Line2;						/* plugged message line */


/*______________________________________________________________________

	RecordTag - Record a New Tag.
	
	Entry:	tag = tag to be recorded.  If the tag is zero or if it
				does not appear in the TAG resource it is not
				recorded.
					
	Exit:		current report line number and corresponding tag
				recorded in table.
_____________________________________________________________________*/


static void RecordTag (short tag)

{
	TagTableEntry		tagEntry;	/* new entry for tag table */
	
	if (!tag || !rep_Tag(tagID, tag)) return;
	tagEntry.reportLineNum = rep_GetSize(Report);
	tagEntry.tag = tag;
	
	/* Allocate more space in tag table if necessary. */
	
	if (NTags >= NAlloc) {
		if (!TagTable) {
			TagTable = (TagTableEntry(**)[])NewHandle(10*sizeof(TagTableEntry));
			NAlloc = 10;
		} else {
			SetHandleSize((Handle)TagTable, 
				GetHandleSize((Handle)TagTable) + 10*sizeof(TagTableEntry));
			NAlloc += 10;
		};
	};
	
	/* Record new tag table entry. */
	
	(**TagTable)[NTags] = tagEntry;
	NTags++;
}

/*______________________________________________________________________

	mssg_LookupTag - Lookup a Tag.
	
	Entry:	repLine = line number in report of error message.
					
	Exit:		function result = tag of doc text of error
				message description, or -1 if the report line number is
				not an error message.
_____________________________________________________________________*/


short mssg_LookupTag (short repLine)

{
	short			i;
	
	for (i = 0; i < NTags; i++) {
		if ((**TagTable)[i].reportLineNum == repLine) 
			return (**TagTable)[i].tag;
	};
	return -1;
}

/*______________________________________________________________________

	mssg_ClearTags - Clear the Tag Table.
	
	This routine should be called whenever the report is cleared.
_____________________________________________________________________*/


void mssg_ClearTags (void)

{
	if (TagTable) {
		DisposHandle((Handle)TagTable);
		TagTable = nil;
	};
	NTags = NAlloc = 0;
}

/*______________________________________________________________________

	mssg_M0 - Issue a Report Message with 0 Parameters.
	mssg_M1 - Issue a Report Message with 1 Parameter.
	mssg_M2 - Issue a Report Message with 2 Parameters.
	mssg_M2_Tag - Issue a Report Message with 2 Parameters and a tag.
	mssg_Continue - Issue a Report Message Continuation Line.
	mssg_Blank - Issue a Blank Line to the Report.
	
	Entry:		strInd = index in STR# of message template.
					p0 = pointer to param 1.
					p1 = pointer to param 2.
					msgNum = message number = index in STR# of first line
						of error message.
					tag = document tag.
_____________________________________________________________________*/


void mssg_M0 (short strInd)

{
	GetIndString(Line1, strListID, strInd);
	RecordTag(strInd);
	rep_Append(Report, Line1, true, true);
}


void mssg_M1 (short strInd, Str255 p0)

{
	GetIndString(Line1, strListID, strInd);
	utl_PlugParams(Line1, Line2, p0, nil, nil, nil);
	RecordTag(strInd);
	rep_Append(Report, Line2, true, true);
}


void mssg_M2 (short strInd, Str255 p0, Str255 p1)

{
	GetIndString(Line1, strListID, strInd);
	utl_PlugParams(Line1, Line2, p0, p1, nil, nil);
	RecordTag(strInd);
	rep_Append(Report, Line2, true, true);
}


void mssg_M2_Tag (short strInd, Str255 p0, Str255 p1, short tag)

{
	GetIndString(Line1, strListID, strInd);
	utl_PlugParams(Line1, Line2, p0, p1, nil, nil);
	RecordTag(tag);
	rep_Append(Report, Line2, true, true);
}


void mssg_Continue (short strInd, short msgNum)

{
	GetIndString(Line1, strListID, strInd);
	RecordTag(msgNum);
	rep_Append(Report, Line1, true, true);
}


void mssg_Blank (void)
{
	rep_Append(Report, "\p", true, true);
}

/*______________________________________________________________________

	PrintPath - Print Indented Path Name.
	
	Entry:	folderList = handle to folder list.
					
	Exit:		level = indentation level for next element of path.
				tag = document tag to be associated with each line of the
					path name list in the report, or 0 if none.
	
	This function calls itself recursively to output the folder list
	in reverse order.
_____________________________________________________________________*/


static void PrintPath (scn_FListElHandle folderList, short *level, short tag)

{
	short				myLevel;			/* my level */
	char				*p;				/* pointer into Line1 */
	short				len;				/* length of folder name */
	
	if (*(**folderList).name) {
		PrintPath((**folderList).next, &myLevel, tag);
		*level = myLevel + 3;
		p = Line1+1;
		while (myLevel--) *p++ = ' ';
		len = *(**folderList).name;
		if (p + len >= Line1 + 256) len = Line1 + 256 - p;
		memcpy(p, (**folderList).name + 1, len);
		*Line1 = p - Line1 - 1 + len;
		RecordTag(tag);
		rep_Append(Report, Line1, true, true);
	} else {
		*level = 0;
	}
}		

/*______________________________________________________________________

	mssg_PrintFileName - Print the File Name.
	
	Entry:	folderList = handle to folder list.
				pBlock = pointer to PBGetCatInfo param block.
				tag = document tag to be associated with each line of the
					path name list in the report, or 0 if none.
_____________________________________________________________________*/


void mssg_PrintFileName (scn_FListElHandle folderList, 
	CInfoPBRec *pBlock, short tag)

{
	short					indentLevel;		/* indentation level */
	char					*pLine;				/* pointer into message line */
	short					fileNameLen;		/* length of file name */

	if (CurScanKind == fileScan) return;
	rep_Append(Report, "\p--------------------------------------------", 
		true, true);
	PrintPath(folderList, &indentLevel, tag);
	pLine = Line1+1;
	while (indentLevel--) *pLine++ = ' ';
	fileNameLen = *pBlock->hfileInfo.ioNamePtr;
	if (pLine + fileNameLen >= Line1 + 256)
		fileNameLen = Line1 + 256 - pLine;
	memcpy(pLine, pBlock->hfileInfo.ioNamePtr + 1, fileNameLen);
	*Line1 = pLine - Line1 - 1 + fileNameLen;
	RecordTag(tag);
	rep_Append(Report, Line1, true, true);
}

/*______________________________________________________________________

	PrintSum - Print Summary Line.
	
	Entry:		theNum = longword summary counter.
					theMsg = index in STR# resource of plural form of message, 
						followed by singular form of message. 
					thePlug = pointer to ^1 plug, or nil if none.
_____________________________________________________________________*/


static void PrintSum (long theNum, short theMsg, Str255 thePlug)

{
	Str255			decNum;			/* converted number */
	
	NumToString(theNum, decNum);
	mssg_M2(theMsg + ((theNum==1) ? 1 : 0), decNum, thePlug);
}

/*______________________________________________________________________

	mssg_CheckAccess - Check Directory Access Privileges.
	
	Entry:	accessRights = ioACUser field from PBGetCatInfo param
					block.  (see IM V-391).
				folderList = handle to folder list to be printed, or
					nil to inhibit printing of folder list.
				pBlock = pointer to PBGetCatInfo param block
					if folderList != nil.
_____________________________________________________________________*/


void mssg_CheckAccess (char accessRights, 
	scn_FListElHandle folderList, CInfoPBRec *pBlock)
	
{
	short			firstMsg;			/* index of first message */

	/* Return if user has both See Files and See Folders access
		privileges. */

	if (!(accessRights &= 3)) return;
	
	/* Print folder path if folderList != nil. */
	
	firstMsg = noPrivs1 + 3*(accessRights-1);
	if (folderList) {
		mssg_PrintFileName(folderList, pBlock, firstMsg);
	};
	
	/* Issue error message. */
	
	mssg_M0(firstMsg);
	mssg_Continue(firstMsg+1, firstMsg);
	mssg_Continue(firstMsg+2, firstMsg);
	TotErrors++;
	mssg_BumpCounter(2);
	TotNoAccess++;
}

/*______________________________________________________________________

	mssg_Begin - Begin a Scan.
	
	Entry			disinfect = true if disinfection run.
					dirID = directory id of folder if folder scan.
					fName = pointer to file name if file scan.
					fVRefNum = vol or wd ref num of folder containing file
						if file scan.
					volRefNum = vol ref num of vol if volume scan, or vol ref
						num of vol containing file or folder if file or folder 
						scan.
					counterTop = top coord of main window counters.
					counterRight = right coord of main window counters.
_____________________________________________________________________*/


void mssg_Begin (Boolean disinfect, long dirID, Str255 fName, 
	short fVRefNum, short volRefNum, short counterTop, short counterRight)

{
	unsigned long	secs;					/* current date/time */
	Str255			nowDate;				/* current date */
	Str255			nowTime;				/* current time */
	scn_FListElHandle	folderList;		/* handle to first el in folder list */
	scn_FListElHandle	newEl;			/* handle to new el for folder list */
	scn_FListElHandle	lastEl;			/* handle to last el in folder list */
	WDPBRec			wdBlock;				/* working directory info block */
	long				curDir;				/* directory id */
	Str255			dirName;				/* directory name */
	CInfoPBRec		dBlock;				/* directory info block */
	short				level;				/* indentation level */
	Boolean			firstDir;			/* true if first directory */
	char				accessRights;		/* directory access rights */

	/* Initialize global variables. */
	
	TotFiles = TotErrors = TotInfected = TotNoAccess = 0;
	CounterTop = counterTop;
	CounterRight = counterRight;
	
	/* Write the report header lines. */
	
	mssg_Blank();
	rep_Append(Report, "\p===========================================",
		true, true);
	mssg_Blank();
	
	/* Write the volume name, folder path list, or file path list.
		For folders and files we build a temporary linked list of
		folder names for use by PrintPath. */
	
	if (utl_VolIsMFS(volRefNum)) {
		vol_GetName(Line1);
		rep_Append(Report, Line1, true, true);
		if (CurScanKind == fileScan) {
			*Line1 = *fName+3;
			*(Line1+1) = ' ';
			*(Line1+2) = ' ';
			*(Line1+3) = ' ';
			memcpy(Line1+4, fName+1, *fName);
			rep_Append(Report, Line1, true, true);
		};
	} else {
		folderList = lastEl = nil;
		if (CurScanKind == fileScan) {
			wdBlock.ioNamePtr = nil;
			wdBlock.ioVRefNum = fVRefNum;
			wdBlock.ioWDIndex = 0;
			wdBlock.ioWDProcID = 0;
			wdBlock.ioWDVRefNum = 0;
			(void) PBGetWDInfo(&wdBlock, false);
			curDir = wdBlock.ioWDDirID;
			newEl = (scn_FListElHandle)NewHandle(sizeof(scn_FListEl));
			(**newEl).next = nil;
			utl_CopyPString((**newEl).name, fName);
			folderList = lastEl = newEl;
		} else if (CurScanKind == foldScan) { 
			curDir = dirID;
		} else {
			curDir = fsRtDirID;
		};
		firstDir = true;
		while (true) {
			dBlock.dirInfo.ioNamePtr = &dirName;
			dBlock.dirInfo.ioVRefNum = volRefNum;
			dBlock.dirInfo.ioFDirIndex = -1;
			dBlock.dirInfo.ioDrDirID = curDir;
			/* dBlock.dirInfo.ioACUser = 0; */
			*(&dBlock.dirInfo.ioFlAttrib+1) = 0;
			(void) PBGetCatInfo((CInfoPBPtr)&dBlock, false);
			if (firstDir) {
				/* accessRights = dBlock.dirInfo.ioACUser; */
				accessRights = *(&dBlock.dirInfo.ioFlAttrib+1);
				firstDir = false;
			};
			newEl = (scn_FListElHandle)NewHandle(sizeof(scn_FListEl));
			(**newEl).next = nil;
			utl_CopyPString((**newEl).name, dirName);
			if (folderList) {
				(**lastEl).next = newEl;
				lastEl = newEl;
			} else {
				folderList = lastEl = newEl;
			};
			if (curDir == fsRtDirID) break;
			curDir = dBlock.dirInfo.ioDrParID;
		};
		newEl = (scn_FListElHandle)NewHandle(sizeof(scn_FListEl));
		(**newEl).next = nil;
		*(**newEl).name = 0;
		if (lastEl) {
			(**lastEl).next = newEl;
		} else {
			folderList = newEl;
		};
		PrintPath(folderList, &level, 0);
		while (folderList) {
			newEl = (**folderList).next;
			DisposHandle((Handle)folderList);
			folderList = newEl;
		};
		if (CurScanKind != fileScan) 
			mssg_CheckAccess(accessRights, nil, nil);
	};
	
	/* Write the final header messages. */
	
	if (CurScanKind == foldScan) {
		mssg_M0(disinfect ? fdStartStr : fsStartStr);
	} else if (CurScanKind == fileScan) {
		mssg_M0(disinfect ? xdStartStr : xsStartStr);
	} else {
		mssg_M0(disinfect ? ddStartStr : dsStartStr);
	}
	GetDateTime(&secs);
	IUDateString(secs, shortDate, &nowDate);
	IUTimeString(secs, true, &nowTime);
	mssg_M2(scanDateStr, &nowDate, &nowTime);
}

/*______________________________________________________________________

	mssg_End - End a Scan.
	
	Entry:		disinfect = true if disinfection run.
					canceled = true if scan canceled.
					
	Exit:			*infected = true if infected file found.
					*sysInfected = true if infected file found in currently
						active system folder.
_____________________________________________________________________*/


void mssg_End (Boolean disinfect, 
	Boolean canceled, Boolean *infected, Boolean *sysInfected)

{
	unsigned long	secs;					/* current date/time */
	Str255			date;					/* date */
	Str255			time;					/* time */
	
	/* Set the infected and sysinfected flag. */
	
	*infected = TotInfected > 0;
	*sysInfected = false;

	/* If scan canceled write canceled message. */
	
	if (canceled) {
		mssg_Blank();
		mssg_M0(cancelStr);
		return;
	};

	/* Write the "scan complete" report lines. */
	
	if (TotInfected || TotErrors) {
		rep_Append(Report, "\p--------------------------------------------", 
			true, true);
	};
	if (CurScanKind == foldScan) {
		mssg_M0(disinfect ? fdEndStr : fsEndStr);
	} else if (CurScanKind == fileScan) {
		mssg_M0(disinfect ? xdEndStr : xsEndStr);
	} else {
		mssg_M0(disinfect ? ddEndStr : dsEndStr);
	}
	GetDateTime(&secs);
	IUDateString(secs, shortDate, &date);
	IUTimeString(secs, true, &time);
	mssg_M2(scanDateStr, &date, &time);
	
	/* Write the summary report lines. */
	
	if (TotInfected || TotErrors) {
		mssg_Blank();
		mssg_M0(summaryStr);
		mssg_Blank();
		PrintSum(TotFiles, totFilesStr, nil);
		PrintSum(TotErrors, totErrorsStr, nil);
		PrintSum(TotInfected, totInfectedStr, nil);
		if (TotNoAccess) {
			mssg_Blank();
			mssg_M0(noPrivs13);
			mssg_Continue(noPrivs14, noPrivs13);
			mssg_Continue(noPrivs15, noPrivs13);
		};
	} else {
		if (CurScanKind == fileScan) {
		} else { 
			PrintSum(TotFiles, totFilesStr, nil);
			mssg_Blank();
		};
	};
	
	/* If any error messages were issued refer the user to the
		document for more details. */
		
	if (TotErrors) {
		mssg_Blank();
		mssg_M0(errNoteStr1);
		mssg_Continue(errNoteStr2, errNoteStr1);
		mssg_Continue(errNoteStr3, errNoteStr1);
	};
}

/*______________________________________________________________________

	mssg_BumpCounter - Increment a counter.
	
	Entry:		counter = 0 to increment number of files scanned.
					counter = 1 to increment number of infected files.
					counter = 2 to increment number of errors.
_____________________________________________________________________*/


void mssg_BumpCounter (short counter)

{
	long				x;				/* counter value to be redrawn */
	short				h;				/* horizontal coord of counter */
	short				v;				/* vertical coord of counter */
	Str255			str;			/* string to be drawn */
	short				resFile;		/* saved current resource file */

	switch (counter) {
		case 0:
			NumScanned++;
			x = NumScanned;
			break;
		case 1:
			NumInfected++;
			x = NumInfected;
			break;
		case 2:
			NumErrors++;
			x = NumErrors;
			break;
	};
	v = CounterTop + 12*(counter+1);
	h = CounterRight + 5;
	MoveTo(h, v);
	NumToString(x, str);
	resFile = CurResFile();
	UseResFile(0);
	TextMode(srcCopy);
	DrawString(str);
	TextMode(srcOr);
	UseResFile(resFile);
}

/*______________________________________________________________________

	mssg_ClearCounters - Clear counters.
	
	Entry:		counterTop = top coord of main window counters.
					counterRight = right coord of main window counters.
_____________________________________________________________________*/


void mssg_ClearCounters (short counterTop, short counterRight)

{
	Rect			inval;			/* rectangle to be invalidated */

	NumScanned = NumInfected = NumErrors = 0;
	SetRect(&inval, counterRight+5, counterTop, counterRight+55, counterTop+36);
	InvalRect(&inval);
}

/*______________________________________________________________________

	mssg_BadDisk - Issue Bad Disk Error Message.
_____________________________________________________________________*/


void mssg_BadDisk (void)

{
	mssg_Blank();
	rep_Append(Report, "\p===========================================",
		true, true);
	mssg_Blank();
	mssg_M0(badDiskStr1);
	mssg_Continue(badDiskStr2, badDiskStr1);
}