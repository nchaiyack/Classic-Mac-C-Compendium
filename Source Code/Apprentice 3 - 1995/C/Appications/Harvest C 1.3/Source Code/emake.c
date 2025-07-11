/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * Built-in Make facility for Harvest C
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"
#include "CHarvestDoc.h"

extern CHarvestDoc *gProject;

#pragma segment EMake

/*
 * The built in make facility for Harvest C is based on the Harvest C project
 * file.  A project file for Harvest C is a list of C source files,
 * Libraries, and Resource files which must be included in a project.  The
 * project file itself is a text file, containing one UNIX command line on
 * each line of the file.
 */

unsigned long
GetFModDate(char *filename, short volrefnum, long dirID)
{
    HFileParam                      k;
    Str255                          nm;
    OSErr                           bad;
    strcpy((char *) nm, filename);
    c2pstr(nm);
    k.ioCompletion = NULL;
    k.ioNamePtr = nm;
    k.ioVRefNum = volrefnum;
    k.ioFDirIndex = 0;
    k.ioDirID = dirID;
    bad = PBHGetFInfo((HParmBlkPtr) & k, false);
    if (!bad)
	return k.ioFlMdDat;
    else
	return 0;
}

void
EMake(char *filename, short volrefnum, long dirID, int buildall)
/* This routine accepts the Project file */
{

    char                            comline[512];
    GenericFileVia_t                thefile;
    InFileVia_t                     FileList;
    InFileVia_t                     newFileList = NULL;
    InFileVia_t                     LinkList = NULL;
    FSSpec                          filespec;
    int                             argc = 1;
    OSErr                           bad;
    char                           *argv[50];
    int                             CountCompiled = 0;
    char                           *entry;
    unsigned long                   MaxOMFDate = 0;
    unsigned long                   AppDate;
    CurVolrefnum = volrefnum;
    CurDirID = dirID;
    thefile = OpenGenericFile(filename, volrefnum, dirID, "r", 0);
    if (thefile) {
	int                             donefile = 0;
	while (!donefile) {
	    GenericGetLine(thefile, comline);
	    argc = 1;
	    if (comline[0]) {
		/*
		 * Now, divide comline into parts separated by spaces,
		 * storing the parts into argv[][]
		 */
		entry = strtok(comline, " \n\r");
		if (entry) {
		    if (entry[0] == '#')
			entry = NULL;
		    else {
			argv[argc] = (char *) NewPtr(strlen(entry) + 1);
			strcpy(argv[argc++], entry);
		    }
		}
		while (entry) {
		    entry = strtok(NULL, " \n\r");
		    if (entry) {
			if (entry[0] == '#')
			    entry = NULL;
			else {
			    argv[argc] = (char *) NewPtr(strlen(entry) + 1);
			    strcpy(argv[argc++], entry);
			}
		    }
		}
		FileList = SetOpts(argc, argv);
		newFileList = NULL;
		while ((--argc) > 0) {
		    DisposPtr(argv[argc]);
		}
		/*
		 * Now, add all the files in the FileList, to the LinkList,
		 * then delete all the files from the FileList which are
		 * already up to date
		 */
		{
		    InFileVia_t                     templist = FileList;
		    InFileVia_t                     newnode;
		    InFileVia_t                     newnode2;
		    Str255                          pnm;
		    unsigned long                   dateSRC;
		    unsigned long                   dateOMF;
		    while (templist) {
			newnode = Ealloc(sizeof(InFile_t));
			strcpy((char *) pnm, Via(templist)->fname);
			strcpy(Via(newnode)->fname, (char *) pnm);
			switch (pnm[strlen((char *) pnm) - 2]) {
			case '.':
			    switch (pnm[strlen((char *) pnm) - 1]) {
			    case 'c':
			    case 'm':	/* For ObjC */
				strcat(Via(newnode)->fname, ".o");
				/* fall thru */
			    default:
				break;
			    }
			    break;
			case 'r':
			    switch (Via(newnode)->fname[strlen(Via(newnode)->fname) - 1]) {
			    case 'c':
				/* This is a .rsrc file */
				break;
			    default:
				break;
			    }
			    break;
			default:
			    break;
			}
			bad = FindCFile((char *) pnm, Via(templist)->volrefnum, Via(templist)->dirID, &filespec);
			if (!bad) {
			    Via(newnode)->volrefnum = filespec.vRefNum;
			    Via(newnode)->dirID = filespec.parID;
			    Via(newnode)->next = LinkList;
			    LinkList = newnode;
			    dateSRC = GetFModDate((char *) pnm, Via(newnode)->volrefnum, Via(newnode)->dirID);
			    HLock((Handle) newnode);
			    dateOMF = GetFModDate(Via(newnode)->fname, Via(newnode)->volrefnum, Via(newnode)->dirID);
			    if (dateOMF > MaxOMFDate)
				MaxOMFDate = dateOMF;
			    HUnlock((Handle) newnode);
			    if (buildall || (dateSRC > dateOMF)) {
				CountCompiled++;
				newnode2 = Ealloc(sizeof(InFile_t));
				strcpy(Via(newnode2)->fname, (char *) pnm);
				Via(newnode2)->volrefnum = filespec.vRefNum;
				Via(newnode2)->dirID = filespec.parID;
				Via(newnode2)->next = newFileList;
				newFileList = newnode2;
			    }
			} else
			    Efree(newnode);
			templist = Via(templist)->next;
		    }
		    #ifdef Undefined
		    MultiFile(newFileList);
		    #endif
		}
	    } else
		donefile = 1;
	}
	/* Now link */
	AppDate = GetFModDate((char *) gProject->StdAppName, gProject->StdAppVol, gProject->StdAppDir);
	if ((buildall || CountCompiled || (MaxOMFDate > AppDate)))
	    ELink(LinkList, (char *) gProject->StdAppName, gProject->StdAppVol, gProject->StdAppDir);
    } else {
	FatalError("Project file not found");
    }
}
