/*----------------------------------------------------------
#
#	NewsWatcher	- Macintosh NNTP Client Application
#
#-----------------------------------------------------------
#
#	kill.c
#
#	This module handles reading and writing the kill file
#	at the start and end of a NewsWatcher session.
#
#	John Werner (werner@soe.berkeley.edu)
#	May, 1993
#
#-----------------------------------------------------------*/

#include <string.h>

#include "filebuff.h"
#include "glob.h"
#include "kill.h"
#include "killdata.h"
#include "killfile.h"
#include "util.h"
#include "dlgutil.h"

/* kKillFileVersion
 * This number is written at the start of the killfile so that we know what
 * we're reading when the file is read later.  Whenever the data structures
 * written to the file change, this number should be incremented.
 */
#define	kKillFileVersion 1

/*-------------------------------------------------------------------------------------
 * WriteKillFile
 * Write the entire killfile out to disk.
 */
void
WriteKillFile()
{
	OSErr			err = noErr;
	BufferedFile	file;
	short			numKill, g, numEntries, e;
	
	if (!gKillFile)
		return;

	FailErr(BufOpen(&file, &gFilterFile,
					fsWrPerm, kFCreator, kPrefType), error);

	FailErr(BufWriteShort(&file, kKillFileVersion), error);

	/* Write the number of groups with kill info */
	numKill = (short)(GetHandleSize((Handle)gKillFile) / sizeof(**gKillFile));
	FailErr(BufWriteShort(&file, numKill), error);

	/* Loop through the groups */
	for (g = 0; g < numKill; g++) {
		TGroupKillHdl group = (*gKillFile)[g];
		TKillEntry **entries = (**group).entries;

		FailErr(BufWriteStrHdl(&file,(**group).groupName), error);	/* Group Name */
		
		FailErr(BufWriteShort(&file,(**group).noMatchKill), error);	/* Kill by default? */

		numEntries = (**group).numEntries;
		FailErr(BufWriteShort(&file, numEntries), error);			/* # of entries */
		HLockHi((Handle)entries);
		for (e = 0; e < numEntries; e++) {							/* All the entries */
			FailErr(BufWriteStrHdl(&file, (*entries)[e].string), error);
			FailErr(BufWriteStr(&file,
						KillHeaderName((*entries)[e].header)), error);
			FailErr(BufWriteChar(&file, (*entries)[e].match), error);
			FailErr(BufWriteChar(&file, (*entries)[e].action), error);
			FailErr(BufWriteULong(&file, (*entries)[e].expires), error);
			FailErr(BufWriteShort(&file, (*entries)[e].highlight), error);
			FailErr(BufWriteChar(&file, (*entries)[e].ignoreCase), error);
		}
		HUnlock((Handle)entries);
	}
	
	/* Write an empty group name to finish */
	FailErr(BufWriteStr(&file, ""), error);

	FailErr(BufClose(&file), error);

	return;

error:
	/* An error happend.  Try to close and delete the file. */
	BufClose(&file);
	FSpDelete(&gFilterFile);
	if (err != noErr)
		UnexpectedErrorMessage(err);
}

/*-------------------------------------------------------------------------------------
 * ReadKillFile
 * Write the killfile from disk.  If there is no killfile yet,
 * gKillFile is left set to NULL, and we return TRUE for success.
 */
Boolean				/* returns TRUE if successful, FALSE on error */
ReadKillFile()
{
	OSErr			err = noErr;
	BufferedFile	file;
	short			numKill=0, g;
	short			numEntries, e, tempShort, fileVers;
	unsigned long	tempULong;
	CStr255			tempStr;
	char			tempChar;
	TGroupKillHdl	group;
	TKillEntry		**entries;

	err = BufOpen(&file, &gFilterFile, fsRdPerm, 0, 0);
	FailErr(err, error);

	FailErr(BufReadShort(&file, &fileVers), error);			/* Version # of file format */
	
	FailErr(BufReadShort(&file, &numKill), error);				/* # of groups in file */

	/* Allocate the handle that holds all the killfile entries */
	FailNIL(gKillFile = (TGroupKillHdl **)MyNewHandle(numKill * sizeof(**gKillFile)), error);
	HLock((Handle)gKillFile);
	memset(*gKillFile, 0, numKill * sizeof(**gKillFile));
	HUnlock((Handle)gKillFile);

	for (g = 0; g < numKill; g++) {
		FailErr(BufReadStr(&file, tempStr, 256), error);		/* Group name */
		
		/* Allocate and initialize the kill info for this group */
		FailNIL(group = NewGroupKill(tempStr), error);
		(*gKillFile)[g] = group;

		FailErr(BufReadShort(&file, &tempShort), error);		/* Kill by default? */
		(**group).noMatchKill = tempShort;

		FailErr(BufReadShort(&file, &numEntries), error);		/* # entries for this group */
		(**group).numEntries = numEntries;

		/* Make room to hold all the entries */
		entries = (**group).entries;
		MySetHandleSize((Handle)entries, numEntries*sizeof(TKillEntry));
		FailMemErr(error);
		HLock((Handle)entries);
		memset(*entries, 0, numEntries*sizeof(TKillEntry));

		for (e = 0; e < numEntries; e++) {					/* All the entries */
			FailErr(BufReadStr(&file, tempStr, 256), error);
			FailErr(InitKillEntry(*entries + e, tempStr), error);
		
			FailErr(BufReadStr(&file, tempStr, 256), error);
			(*entries)[e].header = KillHeaderNum(tempStr);
			
			FailErr(BufReadChar(&file, &tempChar), error);
			(*entries)[e].match = tempChar;
			
			FailErr(BufReadChar(&file, &tempChar), error);
			(*entries)[e].action = tempChar;
			
			FailErr(BufReadULong(&file, &tempULong), error);
			(*entries)[e].expires = tempULong;		

			FailErr(BufReadShort(&file, &tempShort), error);
			(*entries)[e].highlight = tempShort;		

			FailErr(BufReadChar(&file, &tempChar), error);
			(*entries)[e].ignoreCase = tempChar;		
		}
		HUnlock((Handle)(**group).entries);
	}

	return TRUE;

error:
	BufClose(&file);							/* Close the file */
	if (gKillFile) {
		for (g = 0; g < numKill; g++) {			/* Get rid of whatever was read */
			/* TODO: something */
		}
		MyDisposHandle((Handle)gKillFile);
		gKillFile = NULL;
	}
	if (err != noErr && err != fnfErr)
		UnexpectedErrorMessage(err);

	return (err == noErr || err == fnfErr);
}
