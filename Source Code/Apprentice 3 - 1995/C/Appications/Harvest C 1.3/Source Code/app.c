/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved. This software is not
 * freely distributable nor is it public domain.
 * 
 * Routines for the application version of Harvest C for the Mac.  These are
 * based on TransSkel 2.6.
 * 
 * 
 * 
 */

#ifdef OLDCODE
#include "conditcomp.h"
#include <string.h>
#include <stdio.h>
#include "structs.h"

#pragma segment AppStuff

enum FileMenuItem {		/* File menu item numbers */
    new = 1,			/* begin new window */
    open,			/* open existing file */
    close,			/* close file */
    /* --- */
    save = 5,			/* save file */
    saveAs,			/* save under another name */
    saveCopy,			/* save a copy w/o switching file binding */
    revert,			/* revert to version on disk */
    /* --- */
    quit = 10
};


enum EditMenuItem {		/* Edit menu item numbers */
    undo = 1,
    /* --- */
    cut = 3,
    copy,
    paste,
    clear
};

enum OptionsMenuItem {
    includepath = 1,
    libpath,
    apppath,
    codegen,
    general,
    saveopts = 7,
    readopts,
    defaultname,
    warnings
};

enum ProjectMenuItem {
    compilefile = 1,
    domake,
    dolink,
    smartmake,
    debuggeritem,
};

MenuHandle                      fileMenu;
MenuHandle                      editMenu;
MenuHandle                      codegenMenu;
MenuHandle                      generalMenu;
MenuHandle                      optionsMenu;
MenuHandle                      projectMenu;
MenuHandle                      warningsMenu;

void
UpdateOptionsMenu(void)
{
    int                             ndx;
    if (codegenMenu && generalMenu) {
#ifndef FREECC
	CheckItem(codegenMenu, 1, (Boolean) Opts.BigGlobals);
	CheckItem(codegenMenu, 2, (Boolean) Opts._mc68020);
	CheckItem(codegenMenu, 3, (Boolean) Opts._mc68881);
#endif
	CheckItem(codegenMenu, 4, (Boolean) Opts.SignedChars);
	CheckItem(codegenMenu, 5, (Boolean) (!Opts.MPWoutput));
#ifndef FREECC
	CheckItem(codegenMenu, 6, (Boolean) Opts.Annotate);
#endif
	CheckItem(codegenMenu, 7, (Boolean) Opts.MBGSymbols);

	CheckItem(generalMenu, 1, (Boolean) Opts.useTrigraphs);
#ifndef FREECC
	CheckItem(generalMenu, 2, (Boolean) Opts.metrics);
	CheckItem(generalMenu, 3, (Boolean) Opts.oopecc);
#endif
	CheckItem(generalMenu, 4, (Boolean) Opts.allwarnings);
	CheckItem(generalMenu, 5, (Boolean) Opts.nowarnings);
	CheckItem(generalMenu, 6, (Boolean) Opts.progress);
	CheckItem(generalMenu, 7, (Boolean) Opts.GiveTime);
	ndx = 1;
	while (ndx < 34) {
	    CheckItem(warningsMenu, ndx, (Boolean) Opts.Warnings[ndx]);
	    ndx++;
	}
    }
}

static SysEnvRec                theWorld;

void
SaveOptsFile(void)
{
    WDPBRec                         pb;
    Str255                          s;
    OSErr                           bad;
    short                           refnum;
    long                            dirID;
    int                             volrefnum;
    long                            count;

    SysEnvirons(curSysEnvVers, &theWorld);

    pb.ioNamePtr = nil;
    pb.ioVRefNum = theWorld.sysVRefNum;
    pb.ioWDIndex = 0;
    pb.ioWDProcID = 0;
    PBGetWDInfo(&pb, false);

    dirID = pb.ioWDDirID;
    volrefnum = pb.ioWDVRefNum;

    bad = HDelete(volrefnum, dirID, "\pHarvest C Options");
    if (bad != -43)
	UserFileError(bad);
    bad = HCreate(volrefnum, dirID, "\pHarvest C Options", 'Jn15', '????');
    UserFileError(bad);
    bad = HOpen(volrefnum, dirID, "\pHarvest C Options", 2, &refnum);
    UserFileError(bad);
    if (!bad) {
	count = sizeof(Opts);
	bad = FSWrite(refnum, &count, (char *) &Opts);
	DefaultOpts = Opts;
	UserFileError(bad);
	FSClose(refnum);
    }
}

int                             LostOptsFile;

int
ReadOptsFile(void)
{
    WDPBRec                         pb;
    Str255                          s;
    OSErr                           bad;
    short                           refnum;
    long                            fcnt;
    long                            dirID;
    int                             volrefnum;
    long                            count;

    SysEnvirons(curSysEnvVers, &theWorld);
    LostOptsFile = 0;

    pb.ioNamePtr = nil;
    pb.ioVRefNum = theWorld.sysVRefNum;
    pb.ioWDIndex = 0;
    pb.ioWDProcID = 0;
    PBGetWDInfo(&pb, false);

    dirID = pb.ioWDDirID;
    volrefnum = pb.ioWDVRefNum;

    bad = HOpen(volrefnum, dirID, "\pHarvest C Options", 1, &refnum);

    if (!bad) {
	count = sizeof(DefaultOpts);
	bad = GetEOF(refnum, &fcnt);
	if (bad) {
	    SysBeep(1);
	    LostOptsFile = 1;
	    FSClose(refnum);
	    return 0;
	}
	if (count != fcnt) {
	    SysBeep(1);
	    FSClose(refnum);
	    LostOptsFile = 1;
	    return 0;
	}
	bad = FSRead(refnum, &count, (char *) &DefaultOpts);
	Opts = DefaultOpts;
	if (bad) {
	    SysBeep(1);
	    LostOptsFile = 1;
	    FSClose(refnum);
	    return 0;
	}
	bad = FSClose(refnum);
	if (bad) {
	    SysBeep(1);
	    return 0;
	}
	UpdateOptionsMenu();
	return 1;
    }
    SysBeep(1);
    LostOptsFile = 1;
    return 0;
}

long                            MyCurDir;
Boolean                         CurDirValid;
char                            junknm[64];
int                             sfinlibs;

SFReply                         dreply;

pascal short
SFHookF(item, dPtr)
    short                           item;
    DialogPtr                       dPtr;
{
    /* Equates for the itmes that I've added */
#define getDirButton 11
#define getDirNowButton 12
#define getDirMessage 13

    Str255                          messageTitle;
    short                           kind;
    Handle                          h;
    Rect                            r;

    switch (item) {
    case -1:

	/* Read in the prompt string from the resource fork, and initialize */
	/* CurDirValid to FALSE. */

	GetIndString(&messageTitle, 6695, 1);
	GetDItem(dPtr, getDirMessage, &kind, &h, &r);
	SetIText(h, &messageTitle);
	CurDirValid = false;
	return item;
	break;
    case getDirButton:
	if (dreply.fType != 0) {
	    MyCurDir = dreply.fType;
	    CurDirValid = true;
	    return (getCancel);
	};
	break;
    case getDirNowButton:
	MyCurDir = *(long *) CurDirStore;
	CurDirValid = true;
	return (getCancel);
    };
    return (item);		/* By default, return the item passed to us. */
};


pascal short
SFHookG(item, dPtr)
    short                           item;
    DialogPtr                       dPtr;
{
    /* Equates for the itmes that I've added */
#define getDirButton 11
#define getDirNowButton 12
#define getDirMessage 13

    Str255                          messageTitle;
    short                           kind;
    Handle                          h;
    Rect                            r;

    switch (item) {
    case -1:

	/* Read in the prompt string from the resource fork, and initialize */
	/* CurDirValid to FALSE. */
	GetIndString(&messageTitle, 6696, 1);
	GetDItem(dPtr, getDirMessage, &kind, &h, &r);
	SetIText(h, &messageTitle);
	CurDirValid = false;
	return item;
	break;
    case getDirButton:
	if (dreply.fType != 0) {
	    MyCurDir = dreply.fType;
	    CurDirValid = true;
	    return (getCancel);
	};
	break;
    case getDirNowButton:
	MyCurDir = *(long *) CurDirStore;
	CurDirValid = true;
	return (getCancel);
    };
    return (item);		/* By default, return the item passed to us. */
};


pascal short
SFHookH(item, dPtr)
    short                           item;
    DialogPtr                       dPtr;
{
    /* Equates for the itmes that I've added */
#define getDirButton 11
#define getDirNowButton 12
#define getDirMessage 13

    Str255                          messageTitle;
    short                           kind;
    Handle                          h;
    Rect                            r;

    switch (item) {
    case -1:

	/* Read in the prompt string from the resource fork, and initialize */
	/* CurDirValid to FALSE. */
	GetIndString(&messageTitle, 6697, 1);
	GetDItem(dPtr, getDirMessage, &kind, &h, &r);
	SetIText(h, &messageTitle);
	CurDirValid = false;
	return item;
	break;
    case getDirButton:
	if (dreply.fType != 0) {
	    MyCurDir = dreply.fType;
	    CurDirValid = true;
	    return (getCancel);
	};
	break;
    case getDirNowButton:
	MyCurDir = *(long *) CurDirStore;
	CurDirValid = true;
	return (getCancel);
    };
    return (item);		/* By default, return the item passed to us. */
};


pascal                          Boolean
FoldersOnly(p)
    ParmBlkPtr                      p;
{
    /* Normally, folders are ALWAYS shown, and aren't even passed to		 */
    /* this file filter for judgement. Under such circumstances, it is		 */
    /* only necessary to blindly return TRUE (allow no files whatsoever).	 */
    /* However, Standard File is not documented in such a manner, and		 */
    /* this feature may not be TRUE in the future. Therefore, we DO check	 */
    /* to see if the entry passed to us describes a file or a directory.	 */

    if ((p->fileParam.ioFlAttrib & 0x10) != 0)
	return (false);
    return (true);
};



InFileVia_t
FileSelectorF(void)
{
    /*
     * The objective here is to call SFGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    Point                           where =
    {64, 64};
    Str255                          s;

    *(long *) CurDirStore = Opts.StdIncludeDir;
    *(short *) SFSaveDisk = -(Opts.StdIncludeVol);

    SFPGetFile(where,		/* location */
	       "\pSpace for Rent",	/* vestigial string */
	       FoldersOnly,	/* fileFilter */
	       -1,		/* numtypes; -1 means all */
	       &typeList,	/* array to types to show */
	       SFHookF,		/* dlgHook */
	       &dreply,		/* record for returned values */
	       6694,
	       NULL);
    if (CurDirValid) {
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = MyCurDir;
	Via(result)->volrefnum = -(*(short *) SFSaveDisk);
	p2cstr(dreply.fName);
	strcpy(Via(result)->fname, dreply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

InFileVia_t
FileSelectorG(void)
{
    /*
     * The objective here is to call SFGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    Point                           where =
    {64, 64};
    Str255                          s;

    *(long *) CurDirStore = Opts.StdLibDir;
    *(short *) SFSaveDisk = -(Opts.StdLibVol);

    SFPGetFile(where,		/* location */
	       "\pSpace for Rent",	/* vestigial string */
	       FoldersOnly,	/* fileFilter */
	       -1,		/* numtypes; -1 means all */
	       &typeList,	/* array to types to show */
	       SFHookG,		/* dlgHook */
	       &dreply,		/* record for returned values */
	       6694,
	       NULL);
    if (CurDirValid) {
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = MyCurDir;
	Via(result)->volrefnum = -(*(short *) SFSaveDisk);
	p2cstr(dreply.fName);
	strcpy(Via(result)->fname, dreply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

InFileVia_t
FileSelectorH(void)
{
    /*
     * The objective here is to call SFGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    Point                           where =
    {64, 64};
    Str255                          s;

    *(long *) CurDirStore = Opts.StdAppDir;
    *(short *) SFSaveDisk = -(Opts.StdAppVol);

    SFPGetFile(where,		/* location */
	       "\pSpace for Rent",	/* vestigial string */
	       FoldersOnly,	/* fileFilter */
	       -1,		/* numtypes; -1 means all */
	       &typeList,	/* array to types to show */
	       SFHookH,		/* dlgHook */
	       &dreply,		/* record for returned values */
	       6694,
	       NULL);
    if (CurDirValid) {
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = MyCurDir;
	Via(result)->volrefnum = -(*(short *) SFSaveDisk);
	p2cstr(dreply.fName);
	strcpy(Via(result)->fname, dreply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

InFileVia_t
FileSelector(OSType filetype)
{
    /*
     * The objective here is to call SFGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    typeList[0] = filetype;
    SFGetFile(where, "\pnothing", NULL, 1, &typeList, NULL, &reply);
    if (reply.good) {
	WDPBRec                         blk;
	long                            dirID;
	short                           volrefnum;
	blk.ioNamePtr = NULL;
	blk.ioVRefNum = reply.vRefNum;
	blk.ioWDIndex = 0;
	blk.ioWDProcID = 0;
	PBGetWDInfo(&blk, 0);
	dirID = blk.ioWDDirID;
	volrefnum = blk.ioWDVRefNum;
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = dirID;
	Via(result)->volrefnum = volrefnum;
	p2cstr(reply.fName);
	strcpy(Via(result)->fname, reply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

pascal short
SFHookE(item, theDialog)
    short                           item;
    DialogPtr                       theDialog;
{
    Handle                          itemToChange;
    Rect                            itemBox;
    short                           itemType;

    switch (item) {
    default:
	return item;
	break;
    }
    return item;
}

InFileVia_t
FileSelectorE()
{
    /*
     * The objective here is to call SFPPutFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    char                            pname[256];
    strcpy(pname, Opts.StdAppName);
    c2pstr(pname);
    *(long *) CurDirStore = Opts.StdAppDir;
    *(short *) SFSaveDisk = -(Opts.StdAppVol);

    SFPPutFile(where, "\pEnter name of app", pname, SFHookE, &reply, 6693, NULL);
    if (reply.good) {
	WDPBRec                         blk;
	long                            dirID;
	short                           volrefnum;
	blk.ioNamePtr = NULL;
	blk.ioVRefNum = reply.vRefNum;
	blk.ioWDIndex = 0;
	blk.ioWDProcID = 0;
	PBGetWDInfo(&blk, 0);
	dirID = blk.ioWDDirID;
	volrefnum = blk.ioWDVRefNum;
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = dirID;
	Via(result)->volrefnum = volrefnum;
	p2cstr(reply.fName);
	strcpy(Via(result)->fname, reply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

int
FileSelectorK()
{
    InFileVia_t                     result;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    char                            pname[256];
    strcpy(pname, Opts.StdAppName);
    c2pstr(pname);
    SFPPutFile(where, "\pDefault app name", pname, SFHookE, &reply, 6700, NULL);
    if (reply.good) {
	p2cstr(reply.fName);
	strcpy(Opts.StdAppName, reply.fName);
	return 1;
    } else
	return 0;
}

pascal short
SFHookB(item, theDialog)
    short                           item;
    DialogPtr                       theDialog;
{
    Handle                          itemToChange;
    Rect                            itemBox;
    short                           itemType;
    char                            buttonTitle[256];

    switch (item) {
    case -1:
	GetIndString((char *) buttonTitle, 6688, 1);
	if (buttonTitle[0] != 0) {
	    GetDItem(theDialog, 1, &itemType, &itemToChange, &itemBox);
	    SetCTitle((ControlHandle) itemToChange, buttonTitle);
	}
	return item;
	break;
	break;
    default:
	return item;
	break;
    }
    return item;
}

InFileVia_t
FileSelectorB(OSType filetype)
{
    /*
     * The objective here is to call SFPGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    typeList[0] = filetype;
    *(long *) CurDirStore = Opts.StdAppDir;
    *(short *) SFSaveDisk = -(Opts.StdAppVol);

    SFPGetFile(where, "\pnothing", NULL, 1, &typeList, SFHookB, &reply, 6687, NULL);
    if (reply.good) {
	WDPBRec                         blk;
	long                            dirID;
	short                           volrefnum;
	blk.ioNamePtr = NULL;
	blk.ioVRefNum = reply.vRefNum;
	blk.ioWDIndex = 0;
	blk.ioWDProcID = 0;
	PBGetWDInfo(&blk, 0);
	dirID = blk.ioWDDirID;
	volrefnum = blk.ioWDVRefNum;
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = dirID;
	Via(result)->volrefnum = volrefnum;
	p2cstr(reply.fName);
	strcpy(Via(result)->fname, reply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

pascal short
SFHookC(item, theDialog)
    short                           item;
    DialogPtr                       theDialog;
{
    Handle                          itemToChange;
    Rect                            itemBox;
    short                           itemType;
    char                            buttonTitle[256];

    switch (item) {
    case -1:
	GetIndString((char *) buttonTitle, 6689, 1);
	if (buttonTitle[0] != 0) {
	    GetDItem(theDialog, 1, &itemType, &itemToChange, &itemBox);
	    SetCTitle((ControlHandle) itemToChange, buttonTitle);
	}
	return item;
	break;
	break;
    default:
	return item;
	break;
    }
    return item;
}

InFileVia_t
FileSelectorC(OSType filetype)
{
    /*
     * The objective here is to call SFPGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    typeList[0] = filetype;
    *(long *) CurDirStore = Opts.StdAppDir;
    *(short *) SFSaveDisk = -(Opts.StdAppVol);

    SFPGetFile(where, "\pnothing", NULL, 1, &typeList, SFHookC, &reply, 6687, NULL);
    if (reply.good) {
	WDPBRec                         blk;
	long                            dirID;
	short                           volrefnum;
	blk.ioNamePtr = NULL;
	blk.ioVRefNum = reply.vRefNum;
	blk.ioWDIndex = 0;
	blk.ioWDProcID = 0;
	PBGetWDInfo(&blk, 0);
	dirID = blk.ioWDDirID;
	volrefnum = blk.ioWDVRefNum;
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = dirID;
	Via(result)->volrefnum = volrefnum;
	p2cstr(reply.fName);
	strcpy(Via(result)->fname, reply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

pascal short
SFHookD(item, theDialog)
    short                           item;
    DialogPtr                       theDialog;
{
    Handle                          itemToChange;
    Rect                            itemBox;
    short                           itemType;
    char                            buttonTitle[256];

    switch (item) {
    case -1:
	GetIndString((char *) buttonTitle, 6690, 1);
	if (buttonTitle[0] != 0) {
	    GetDItem(theDialog, 1, &itemType, &itemToChange, &itemBox);
	    SetCTitle((ControlHandle) itemToChange, buttonTitle);
	}
	if (sfinlibs) {
	    sfinlibs = 1;

	    *(long *) CurDirStore = Opts.StdLibDir;
	    *(short *) SFSaveDisk = -(Opts.StdLibVol);

	    GetDItem(theDialog, 11, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 1);
	    GetDItem(theDialog, 12, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 0);
	} else {
	    sfinlibs = 0;
	    *(long *) CurDirStore = Opts.StdAppDir;
	    *(short *) SFSaveDisk = -(Opts.StdAppVol);

	    GetDItem(theDialog, 11, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 0);
	    GetDItem(theDialog, 12, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 1);
	}
	return 101;
	break;
    case 11:
	if (!sfinlibs) {
	    sfinlibs = 1;

	    *(long *) CurDirStore = Opts.StdLibDir;
	    *(short *) SFSaveDisk = -(Opts.StdLibVol);

	    GetDItem(theDialog, 11, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 1);
	    GetDItem(theDialog, 12, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 0);
	    return 101;
	}
	return item;
	break;
    case 12:
	if (sfinlibs) {
	    sfinlibs = 0;
	    *(long *) CurDirStore = Opts.StdAppDir;
	    *(short *) SFSaveDisk = -(Opts.StdAppVol);

	    GetDItem(theDialog, 11, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 0);
	    GetDItem(theDialog, 12, &itemType, &itemToChange, &itemBox);
	    SetCtlValue((ControlHandle) itemToChange, 1);
	    return 101;
	}
	return item;
	break;
    case 13:
	return getCancel;
	break;
	break;
    default:
	return item;
	break;
    }
    return item;
}

InFileVia_t
FileSelectorD(OSType filetype)
{
    /*
     * The objective here is to call SFPGetFile, munge the result into a
     * InFileVia structure and return it.  This involves translation of the
     * wdrefnum to a volrefnum, dirID pair, etc...
     */
    InFileVia_t                     result;
    SFTypeList                      typeList;
    SFReply                         reply;
    Point                           where =
    {64, 64};
    typeList[0] = filetype;

    SFPGetFile(where, "\pnothing", NULL, 1, &typeList, SFHookD, &reply, 6691, NULL);
    if (reply.good) {
	WDPBRec                         blk;
	long                            dirID;
	short                           volrefnum;
	blk.ioNamePtr = NULL;
	blk.ioVRefNum = reply.vRefNum;
	blk.ioWDIndex = 0;
	blk.ioWDProcID = 0;
	PBGetWDInfo(&blk, 0);
	dirID = blk.ioWDDirID;
	volrefnum = blk.ioWDVRefNum;
	result = Ealloc(sizeof(InFile_t));
	Via(result)->dirID = dirID;
	Via(result)->volrefnum = volrefnum;
	p2cstr(reply.fName);
	strcpy(Via(result)->fname, reply.fName);
	Via(result)->next = NULL;
	return result;
    } else
	return NULL;
}

void
UpdateFileMenu(void)
{
    WindowPtr                       theWind;
    DisableItem(fileMenu, close);
    DisableItem(fileMenu, save);
    DisableItem(fileMenu, saveAs);
    DisableItem(fileMenu, saveCopy);
    DisableItem(fileMenu, revert);
    if ((theWind = FrontWindow()) != 0) {
	EnableItem(fileMenu, close);
    }
    if (IsTWindow(theWind)) {
	EnableItem(fileMenu, saveAs);
	EnableItem(fileMenu, saveCopy);
	if (!GetTWindowFile(theWind, NULL)) {
	    EnableItem(fileMenu, save);
	} else if (IsTWindowDirty(theWind)) {
	    EnableItem(fileMenu, save);
	    EnableItem(fileMenu, revert);
	}
    }
}

WindowPtr                       lastFront;

void
CheckFront(void)
{
    if (lastFront != FrontWindow()) {
	UpdateFileMenu();
	lastFront = FrontWindow();
    }
}

WindowPtr
BuildWindow(int bindtofile, char *name)
{
    Rect                            r;
    static int                      count = 0;
    int                             offset;
    if (!FrontWindow())
	count = 0;
    SetRect(&r, 0, 0, 400, 275);
    offset = 50 + 25 * (count++ % 4);
    OffsetRect(&r, offset, offset);
    return NewTWindow(&r, name, 1, ((void *) -1), 1, 0L, bindtofile);
}

/*
 * File menu handler
 */

void
DoFileMenu(item)
    short                           item;
{
    WindowPtr                       theWind;
    theWind = FrontWindow();
    switch (item) {
    case new:
	BuildWindow(0, NULL);
	break;

    case open:
	BuildWindow(1, NULL);
	break;

    case close:
	if (IsTWindow(theWind))
	    TWindowClose(theWind);
	else
	    CloseDeskAcc(((WindowPeek) theWind)->windowKind);
	break;

    case save:
	TWindowSave(theWind);
	break;

    case saveAs:
	TWindowSaveAs(theWind);
	break;

    case saveCopy:
	TWindowSaveCopy(theWind);
	break;

    case revert:
	TWindowRevert(theWind);
	break;

    case quit:
	if (ClobberTWindows())
	    SkelWhoa();
	break;

    }
    UpdateFileMenu();
}


/*
 * Handle Options menu items
 */

void
DoOptionsMenu(item)
    short                           item;
{
    InFileVia_t                     whatfile;
    switch (item) {
    case includepath:
	whatfile = FileSelectorF();
	if (whatfile) {
	    /*
	     * We now set the default include path to the path to the chosen
	     * file
	     */
	    Opts.StdIncludeVol = Via(whatfile)->volrefnum;
	    Opts.StdIncludeDir = Via(whatfile)->dirID;
	    Efree(whatfile);
	}
	break;
    case libpath:
	whatfile = FileSelectorG();
	if (whatfile) {
	    /*
	     * We now set the default include path to the path to the chosen
	     * file
	     */
	    Opts.StdLibVol = Via(whatfile)->volrefnum;
	    Opts.StdLibDir = Via(whatfile)->dirID;
	    Efree(whatfile);
	}
	break;
    case apppath:
	whatfile = FileSelectorH();
	if (whatfile) {
	    /*
	     * We now set the default include path to the path to the chosen
	     * file
	     */
	    Opts.StdAppVol = Via(whatfile)->volrefnum;
	    Opts.StdAppDir = Via(whatfile)->dirID;
	    Efree(whatfile);
	}
	break;
    case saveopts:
	SaveOptsFile();
	break;
    case readopts:
	ReadOptsFile();
	break;
    case defaultname:
	FileSelectorK();
    default:
	break;
    }
}


/*
 * Handle code gen menu items
 */

void
DoCodeGenMenu(item)
    short                           item;
{
    switch (item) {
#ifndef FREECC
    case 1:
	Opts.BigGlobals = !Opts.BigGlobals;
	break;
    case 2:
	Opts._mc68020 = !Opts._mc68020;
	break;
    case 3:
	Opts._mc68881 = !Opts._mc68881;
	break;
#endif
    case 4:
	Opts.SignedChars = !Opts.SignedChars;
	break;
    case 5:
	Opts.MPWoutput = !Opts.MPWoutput;
	break;
#ifndef FREECC
    case 6:
	Opts.Annotate = !Opts.Annotate;
	break;
#endif
    case 7:
	Opts.MBGSymbols = !Opts.MBGSymbols;
	break;
    default:
	break;
    }
    UpdateOptionsMenu();
}

void
DoWarningsMenu(item)
    short                           item;
{
    Opts.Warnings[item] = !Opts.Warnings[item];
    UpdateOptionsMenu();
}

/*
 * Handle general menu items
 */

void
DoGeneralMenu(item)
    short                           item;
{
    switch (item) {
    case 1:
	Opts.useTrigraphs = !Opts.useTrigraphs;
	break;
#ifndef FREECC
    case 2:
	Opts.metrics = !Opts.metrics;
	break;
    case 3:
	Opts.oopecc = !Opts.oopecc;
	break;
#endif
    case 4:
	Opts.allwarnings = !Opts.allwarnings;
	if (Opts.allwarnings)
	    Opts.nowarnings = 0;
	break;
    case 5:
	Opts.nowarnings = !Opts.nowarnings;
	if (Opts.nowarnings)
	    Opts.allwarnings = 0;
	break;
    case 6:
	Opts.progress = !Opts.progress;
	break;
    case 7:
	Opts.GiveTime = !Opts.GiveTime;
	break;
    default:
	break;
    }
    UpdateOptionsMenu();
}

void
DoCompileFile(void)
{
    InFileVia_t                     whatfile;
    /* Put up a file dialog, and compile the given file */
    whatfile = FileSelectorB('TEXT');
    if (whatfile) {
	MoveHHi((Handle) whatfile);
	HLock((Handle) whatfile);
	SingleFile(Via(whatfile)->fname,
		   Via(whatfile)->volrefnum, Via(whatfile)->dirID);
	HUnlock((Handle) whatfile);
	Efree(whatfile);
    }
}

/*
 * Handle Project menu items
 */

void
DoProjectMenu(item)
    short                           item;
{
    InFileVia_t                     whatfile;
    InFileVia_t                     firstfile;
    InFileVia_t                     destfile;
    InFileVia_t                     nextfile;
    switch (item) {
    case compilefile:
	DoCompileFile();
	break;
    case domake:
	/* Put up a file dialog, and 'make' with the given file */
	whatfile = FileSelectorC('TEXT');
	if (whatfile) {
	    Opts.StdBuildDir = Via(whatfile)->dirID;
	    Opts.StdBuildVol = Via(whatfile)->volrefnum;
	    MoveHHi((Handle) whatfile);
	    HLock((Handle) whatfile);
	    EMake(Via(whatfile)->fname,
		  Via(whatfile)->volrefnum, Via(whatfile)->dirID, 1);
	    HUnlock((Handle) whatfile);
	    Efree(whatfile);
	}
	break;
    case dolink:

	sfinlibs = 0;
	nextfile = firstfile = FileSelectorD('OBJ ');
	if (nextfile) {
	    whatfile = nextfile;
	    while (nextfile) {
		nextfile = FileSelectorD('OBJ ');
		if (nextfile) {
		    Via(nextfile)->next = whatfile;
		    whatfile = nextfile;
		}
	    }
	    destfile = FileSelectorE();
	    if (destfile) {
		HLock((Handle) destfile);
		/* Call a link routine with this set of files */
		ELink(whatfile, Via(destfile)->fname, Via(destfile)->volrefnum, Via(destfile)->dirID);
		HUnlock((Handle) destfile);
	    }
	}
	break;
    case smartmake:
	/* Put up a file dialog, and 'make' with the given file */
	whatfile = FileSelectorC('TEXT');
	if (whatfile) {
	    Opts.StdBuildDir = Via(whatfile)->dirID;
	    Opts.StdBuildVol = Via(whatfile)->volrefnum;
	    MoveHHi((Handle) whatfile);
	    HLock((Handle) whatfile);
	    EMake(Via(whatfile)->fname,
		  Via(whatfile)->volrefnum, Via(whatfile)->dirID, 0);
	    HUnlock((Handle) whatfile);
	    Efree(whatfile);
	}
	break;
    case debuggeritem:
	Debugger();
	break;
    default:
	break;
    }
}


void
WindowActivate(Boolean x)
{
}


void
CloseIt(void)
{
    WindowPtr                       theWind;
    GetPort(&theWind);
    TWindowClose(theWind);
    UpdateFileMenu();
}

void
SetUpWarningsMenu(void)
{
    warningsMenu = NewMenu(224, "\pWarnings");

    AppendMenu(warningsMenu, "\pEmpty expression stmt;Multi-character constant;Redundant cast;\
Equivalence test of floating type");

    AppendMenu(warningsMenu, "\pDiscarded function result;Assignment as if conditional;\
Non-void function has no return statement;Constant expression as if condition");

    AppendMenu(warningsMenu, "\pComparison of pointer and integer;Assignment of nonequivalent type to a pointer;\
Return of nonequivalent type to a pointer");

    AppendMenu(warningsMenu, "\pPass of nonequivalent type to a pointer;\
Constant expression as switch condition");

    AppendMenu(warningsMenu, "\pConstant expression as while loop condition;\
Constant expression as do-while loop condition");

    AppendMenu(warningsMenu, "\pConstant expression as for loop condition;\
Switch expression not of integral type;\
Volatile is not handled by this compiler;\
(-");

    AppendMenu(warningsMenu, "\pUnused variable ;\
Dead code;\
Implicit decl;\
Re#definition;\
Possible nested comment");

    AppendMenu(warningsMenu, "\p(-;\
goto statement found;\
Multiple function returns;\
Empty compound statement");

    AppendMenu(warningsMenu, "\pMissing function return type - default to int;\
Trigraph found;\
(-;\
pascal keyword found;\
(-;\
Semicolon after function body");

    SkelMenu(warningsMenu, DoWarningsMenu, NULL, true, false);
}

#pragma segment AppInit

void
ApplicationInit(void)
{
    SkelInit(NULL, 450);

    InitDefaultOpts();
    InitOpts();

    SkelApple("\pAbout Harvest C...", DoAbout);

#ifdef Undefined
    fileMenu = NewMenu(1000, "\pFile");
    AppendMenu(fileMenu, "\pNew/N;Open.../O;Close/W;(-;Save/S;Save As...");
    AppendMenu(fileMenu, "\pSave a Copy As...;Revert/R;(-;Quit/Q");
#endif
    fileMenu = GetMenu(1000);
    SkelMenu(fileMenu, DoFileMenu, NULL, false, false);

#ifdef Undefined
    editMenu = NewMenu(1001, "\pEdit");
    AppendMenu(editMenu, "\p(Undo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
#endif
    editMenu = GetMenu(1001);
    SkelMenu(editMenu, TWindowEditOp, NULL, false, false);

    codegenMenu = NewMenu(222, "\pBig globals");
#ifdef FREECC
    AppendMenu(codegenMenu, "\p(Big globals;(mc68020;(mc68881;signed char;68k source;(Annotate;MacsBug symbols");
#else
    AppendMenu(codegenMenu, "\pBig globals;mc68020;mc68881;signed char;68k source;Annotate;MacsBug symbols");
#endif
    SkelMenu(codegenMenu, DoCodeGenMenu, NULL, true, false);

    generalMenu = NewMenu(223, "\pTrigraphs");
#ifdef FREECC
    AppendMenu(generalMenu, "\pTrigraphs;(Metrics;(OOP;All warnings;No warnings;Progress");
#else
    AppendMenu(generalMenu, "\pTrigraphs;Metrics;OOP;All warnings;No warnings;Progress");
#endif
    SkelMenu(generalMenu, DoGeneralMenu, NULL, true, false);

    SetUpWarningsMenu();

    UpdateOptionsMenu();

    optionsMenu = NewMenu(1002, "\pOptions");
    AppendMenu(optionsMenu, "\pInclude Path...;Lib Path...;App Path...;Code generation;General;(-;Save defaults;Read Defaults;Default app name...;Warnings");
    SkelMenu(optionsMenu, DoOptionsMenu, NULL, false, false);
    SetItemCmd(optionsMenu, codegen, 0x1B);
    SetItemMark(optionsMenu, codegen, (char) 222);
    SetItemCmd(optionsMenu, general, 0x1B);
    SetItemMark(optionsMenu, general, (char) 223);
    SetItemCmd(optionsMenu, warnings, 0x1B);
    SetItemMark(optionsMenu, warnings, (char) 224);

#ifdef Undefined
    projectMenu = NewMenu(1003, "\pProject");
    AppendMenu(projectMenu, "\pCompile File.../K;Build.../M;Link...;Make...;Debug/G;");
#endif
    projectMenu = GetMenu(1003);
    SkelMenu(projectMenu, DoProjectMenu, NULL, false, true);

    UpdateFileMenu();

    SetTWindowCreator('ALFA');
    SetTWindowStyle(NULL, 3, 9, 0, teJustLeft);
    SetTWindowProcs(NULL, UpdateFileMenu, WindowActivate, CloseIt);

    if (LostOptsFile) {
	LostOptsFile = 0;
	FakeAlert("\pCouldn't read ", "\pConfig file in ", "\pSystem Folder.", "\p",
		  1, 1, "\pOk", "\p", "\p");
    }
}

#pragma segment AppStuff

int
GiveTime(void)
{
    EventRecord                     tossMe;
    if (Opts.GiveTime) {
	(void) WaitNextEvent(keyDownMask, &tossMe, 0, NULL);
	if (tossMe.what == keyDown)
	    if (((tossMe.message & keyCodeMask) >> 8) == 0x2f)
		if (tossMe.modifiers & 0x0100)
		    return 1;
    }
    return 0;
}

OSErr
FindOMFFile(char *name, short volrefnum, long dirID, FSSpec * f)
{
    FInfo                           junk;
    OSErr                           bad;
    Str255                          pname;

    strcpy(f->name, name);
    strcpy(pname, name);
    c2pstr(pname);

    /* Search path : current app dir, lib dir, etc... */

    bad = HGetFInfo(f->vRefNum = volrefnum, f->parID = dirID, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdAppVol, f->parID = Opts.StdAppDir, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdBuildVol, f->parID = Opts.StdBuildDir, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdLibVol, f->parID = Opts.StdLibDir, pname, &junk);
    if (!bad)
	return bad;

    return bad;
}

OSErr
FindCFile(char *name, short volrefnum, long dirID, FSSpec * f)
{
    FInfo                           junk;
    OSErr                           bad;
    Str255                          pname;

    strcpy(f->name, name);
    strcpy(pname, name);
    c2pstr(pname);

    /* Search path : current app dir, lib dir, etc... */

    bad = HGetFInfo(f->vRefNum = volrefnum, f->parID = dirID, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdAppVol, f->parID = Opts.StdAppDir, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdBuildVol, f->parID = Opts.StdBuildDir, pname, &junk);
    if (!bad)
	return bad;

    bad = HGetFInfo(f->vRefNum = Opts.StdLibVol, f->parID = Opts.StdLibDir, pname, &junk);
    if (!bad)
	return bad;

    return bad;
}
#endif
