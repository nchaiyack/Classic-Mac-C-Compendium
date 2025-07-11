/*----------------------------------------------------------------------------

	kill.c

	This module handles killing/filtering of articles after they are
	fetched from the server.

----------------------------------------------------------------------------*/

#include <Packages.h>	/* For the intl string comparison functions */
#include <string.h>
#include <stdio.h>

#include "glob.h"
#include "dlgutil.h"
#include "kill.h"
#include "killdata.h"
#include "nntp.h"
#include "util.h"

/*----------------------------------------------------------------------------
 * MatchStrings
 * Utility routine for comparing strings.
 *
 */
static Boolean
MatchStrings(
	const char	*string,		/* String we're searching in */
	const char	*toFind)		/* What we're looking for */
{
	Boolean match = FALSE;
	short strLen, patLen;
	short index;

	strLen = strlen(string);
	patLen = strlen(toFind);
 
  	if (patLen <= strLen) {
  		/* TODO: This is incredibly slow */
  		for (index = 0; index <= strLen - patLen && !match; index++) {
	  		match = !strncasecmp(toFind, string+index, patLen);
  		}
  	}

	return match;
}

/*----------------------------------------------------------------------------
 * KillStatus
 * Display or change the status window while the killfile is processed.
 * Displays the current message count in the window.
 * If count==-1, the lastCount variable is reset, and the window is not changed
 */
static void KillStatus(short count)
{
	static short lastCount = 0;
	if (count >= 0 && count != lastCount) {
		char str[64];
		sprintf(str, "Filtering articles. %d", count);
		StatusWindow(str);
	}
	lastCount = count;
}

/*----------------------------------------------------------------------------
 * MarkArticle
 * Mark one article in a subject array as read or unread, and also
 * highlight it if necessary.
 *
 * The starting index for the search is passed in, and the index where
 * the search stopped is returned.  This lets us take advantage of the fact
 * that the subjectArray is sorted by article number.
 *
 */
static short MarkArticle(
	TSubject	**subjectArray,		/* Array of subjects to look for article in */
	short		index,				/* Index in subjectArray to start search */
	short		numSubjects,		/* Total size of subjectArray */
	long		article,			/* The article ID we're looking for */
	EKillAction	action,				/* What to do: Kill, Keep, or Highlight */
	short		highlight,			/* The highlight color if needed */
	short		*count)				/* Is adjusted by -+ one if killed or kept */
{
	Boolean read;

	while (index < numSubjects && (*subjectArray)[index].number <= article) {
		if ((*subjectArray)[index].number == article) {
			switch (action) {
			  case kKeep:
			  	/*
			  	 * Keep and Kill are almost the same, except that Keep
			  	 * might have a highlight color attached, but Kill can't.
			  	 */
			  	if (highlight) {
			  		(*subjectArray)[index].highlight = highlight;
			  	}
			  	/* fallthrough */
			  case kKill:
			  	/* Killed articles are marked as read; Kept articles as unread */
			  	read = (action == kKill);
				if ((*subjectArray)[index].read != read) {
					(*subjectArray)[index].read = read;
					/* Adjust the running count and display it */
					*count += read ? -1 : 1;
					KillStatus(*count);
				}
				break;
			  case kHighlight:
			  	/* Just change the article's highlight color */
			  	(*subjectArray)[index].highlight = highlight;
			  	break;
			}
		}
		index++;
	}
	return index;	/* Where to start search for next article */
}

static Boolean	gHasXPAT = true;

/*---------------------------------------------------------------------------------
 * KillArticles
 * This is the main externally-visible routine used to kill articles in a group.
 * It goes through subjectArray and decides whether each article needs killing.
 * 
 * Killed articles are treated in two possible ways:
 *	If gPrefs.showKilledArticles is TRUE, then kill articles are marked as read
 *	but left in the subject array.
 *
 *	If it is FALSE, the killed articles are removed from the subject array and
 *	numSubjects is adjusted.
 */
void KillArticles(
	char		*groupName,				/* Name of the group we're dealing with */
	TSubject	**subjectArray,			/* List of articles to worry about */
	short		*numSubjects, 			/* # of articles; # killed is subtracted */
	Handle		subStrings)				/* All of the subject strings */
{
	OSErr		err = noErr;
	short		k, hdr, a, subIndex, count, num;
	long		first, last, nextStringOffset;
	char		*toFind;
	Handle		strings = nil;
	THeader		**headers = nil;
	TKillEntry	**entries = nil;
	char		status[64];
	Boolean		read;

	/*
	 * This killHeaders structure is used to store headers that we fetch
	 * from the server if we have to do the string matching ourselves.
	 * They are cached here until the end of the function
	 * in case we need to search the same header again.
	 */
	struct {
		short	num;
		THeader	**headers;
		Handle	strings;
	} killHeaders[kNumKillHeaders];

	/* First, we need to find the killfile record corresponding to this newsgroup */
	TGroupKillHdl aKill = FindGroupKill(groupName, &k);
	if (!aKill)
		return;	/* No killfile; just return */

	/* Initialize the array for storing headers */
	for (k = 0; k < kNumKillHeaders; k++) {
		killHeaders[k].num = 0;
		killHeaders[k].headers = nil;
		killHeaders[k].strings = nil;
	}

	first = (*subjectArray)[0].number;
	last = (*subjectArray)[*numSubjects - 1].number;

	/*
	 * If the default for this group is to kill everying first, do it
	 */
	if ((**aKill).noMatchKill) {
		for (a = 0; a < *numSubjects; a++)
			(*subjectArray)[a].read = true;
		count = 0;
	} else {
		count = *numSubjects;
	}

	/* Initialize and display the status window with a count of articles */
	KillStatus(-1);
	KillStatus(count);

	/*
	 * Loop through each of the killfile entries and find the
	 * articles that match it.  Matched articles are marked as read
	 * or unread depending on the type of killfile entry.
	 */
	entries = (**aKill).entries;
	for (k = 0; k < (**aKill).numEntries; k++) {
		hdr = (*entries)[k].header;
		HLockHi((*entries)[k].string);
		toFind = *((*entries)[k].string);
		if (hdr == kKillSubject) {
			/*
			 * Killing based on the subject; get it from subjectStrings
			 */
			 HLock(subStrings);
			 for (a = 0; a < *numSubjects; a++) {
			 	long offset = (*subjectArray)[a].subjectOffset;
				if (MatchStrings(*subStrings + offset, toFind)) {
					/* This article matches the string pattern */
					switch ((*entries)[k].action) {
					  case kKill:
					  case kKeep:
					  	/* Change the read/unread state of the article */
					  	read = (*entries)[k].action == kKill;
						if ((*subjectArray)[a].read != read) {
							(*subjectArray)[a].read = read;
							count += read ? -1 : 1;
							KillStatus(count);
						}
						if ((*entries)[k].action == kKill && (*entries)[k].highlight) {
							/* kKeep can also affect the highlight color */
						  	(*subjectArray)[a].highlight = (*entries)[k].highlight;
						}
						break;
					  case kHighlight:
					  	/* Just change the highlight color */
					  	(*subjectArray)[a].highlight = (*entries)[k].highlight;
					  	break;
					}
				}
				if (!(a % 5) && !GiveTime()) {
					FailErr(-1, exit);
				}
			 }
			 HUnlock(subStrings);
		} else {
			/*
			 * Killing based on another header. 
			 * First try to do the string matching on the server
			 */
			if (gHasXPAT && gPrefs.useXPAT) {
				headers = nil;
				err = SearchHeaders(groupName, KillHeaderName(hdr), first, last, toFind,
									&headers, &num);
				if (err == 2) {
					err = noErr;		/* XPAT is not implemented on this server */
					gHasXPAT = false;
				} else if (err != 0) {
					goto exit;			/* Some other error */
				} else {
					subIndex = 0;		/* Index into subject array */
					for (a = 0; a < num; a++) {
						/*
						 * Mark as read or unread as appropriate
						 * subIndex is used to take advantage of the fact that
						 * 'articles' and 'subjectArray' are both sorted by article#
						 */
						subIndex = MarkArticle(subjectArray, subIndex, *numSubjects,
												(*headers)[a].number, (*entries)[k].action,
												(*entries)[k].highlight, &count);
						if (!(a % 5) && !GiveTime()) {
							DisposHandle((Handle) headers);
							headers = nil;
							FailErr(-1, exit);
						}
					}
					DisposHandle((Handle)headers);
					headers = nil;
					continue;
				}
			}
			/*
			 * No XPAT; we have to do the string matching ourselves.
			 * First make sure we have fetched this header from the server.
			 */
			if (!killHeaders[hdr].strings) {
				killHeaders[hdr].strings = NewHandle(10000);
				nextStringOffset = 0;
				FailNIL( killHeaders[hdr].strings, exit);
				FailErr( GetHeaders(groupName, KillHeaderName(hdr), first, last,
								killHeaders[hdr].strings, &nextStringOffset, nil, 255,
								&(killHeaders[hdr].headers), &(killHeaders[hdr].num)),
								exit);
			}
			/*
			 * Now we've got the appropriate header.
			 * Loop through all of the articles for which we have the header
			 * and see if any of them match.
			 */
			strings = killHeaders[hdr].strings;
			headers = killHeaders[hdr].headers;
			num = killHeaders[hdr].num;
			HLock(strings);
			subIndex = 0;	/* Start at beginning of subject array too */
			for (a = 0; a < num; a++) {
				if (MatchStrings(*strings + (*headers)[a].offset, toFind)) {
					subIndex = MarkArticle(subjectArray, subIndex, *numSubjects,
												(*headers)[a].number, (*entries)[k].action,
												(*entries)[k].highlight, &count);
				}
				if (!(a % 5) && !GiveTime())
					FailErr(-1, exit);
			}
			HUnlock(strings);
		}
		HUnlock((*entries)[k].string);
	}

	/*
	 * Now that we've got all the killed articles marked as "read",
	 * clobber them out of the subjects array.
	 * TODO: This is about the slowest way this could be done.
	 * I'm not sure if it really matters, though.
	 */
	if (!gPrefs.showKilledArticles) {
		num = *numSubjects;
		for (a = 0; a < num; /**/ ) {
			if ((*subjectArray)[a].read) {
				short numMove = num - a - 1;
				if (numMove > 0) {
					BlockMove(*subjectArray + a + 1, *subjectArray + a,
								numMove * sizeof(**subjectArray));
				}
				num -= 1;
			} else {
				a++;
			}
		}
		for (a = 0; a < num; a++) {
			(*subjectArray)[a].myIndex = a;
		}
		*numSubjects = num;
		SetHandleSize((Handle)subjectArray, num * sizeof(**subjectArray));
	}

exit:
	if (err) {
		/* Put all the articles back the way they were */
		for (a = 0; a < *numSubjects; a++) {
			(*subjectArray)[a].read = false;
		}
	}
	/* Unlock everything */
	if (aKill) {
		for (k = 0; k < (**aKill).numEntries; k++) {
			HUnlock((*entries)[k].string);
		}
	}
	HUnlock(subStrings);
	/* Dispose the headers we fetched from the server */
	for (hdr = 0; hdr < kNumKillHeaders; hdr++) {
		if (killHeaders[hdr].headers)
			DisposHandle((Handle)killHeaders[hdr].headers);
		if (killHeaders[hdr].strings)
			DisposHandle(killHeaders[hdr].strings);
	}
	if (err)
		UnexpectedErrorMessage(err);
}





