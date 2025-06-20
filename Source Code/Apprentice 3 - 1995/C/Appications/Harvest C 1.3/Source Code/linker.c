/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains the Harvest C linker.  It is designed to be functionally
 * compatible with the MPW linker.  THIS FILE SHOULD BE IMPORTANT IN THE PORT
 * OF GCC.
 * 
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"

#include "linkstruct.h"
#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CHarvestOptions.h"
#include "CErrorLog.h"

extern CHarvestApp *gApplication;
extern CHarvestDoc *gProject;
extern	CErrorLog	*gErrs;

#ifdef HDEBUG
FILE *dumpFile;
#endif

FILE *fopenMAC(char *name,short vRefNum,long dirID,char *mode);

/* 
	patchkinds
	
	CODE->CODE 	1
	CODE->DATA	2
	DATA->CODE	3
	DATA->DATA	4
*/

#pragma segment LinkerSeg

int                             LinkErrorCount;

void
LinkerError(char *errstr)
{
	gErrs->Hprintf("%s",errstr);
    LinkErrorCount++;
}

void
LinkerErrorSYM(char *errstr, char *name)
{
    char                            err[128];
    strcpy(err, errstr);
    strcat(err, (name));
    LinkerError(err);
}

unsigned int
fgetw(FILE * thef)
{
    unsigned int                             wd;
    wd = getc(thef);
    wd = (wd << 8) | getc(thef);
    return wd;
}

unsigned long
fgetl(FILE * thef)
{
    unsigned long                   lg;
    lg = fgetw(thef);
    lg = (lg << 16) | fgetw(thef);
    return lg;
}

#define STARTMODSIZE 50000

void
ZeroBytes(char P__H * buf,  long bufsize)
/* Zero out a range of memory */
{
    register  long          ndx = 0;
    while (ndx < bufsize)
	Via(buf)[ndx++] = 0;
}

LinkerFileVia_t
ReadObjectFile(char *fname, short volrefnum, long dirID)
/*
 * Reads an object file, creating data structures which are returned. Returns
 * NULL on file not found
 */
{
    LinkerFileVia_t                 result = NULL;
    FILE *               thef;
    ModuleVia_t                     CurrentCodeModule = NULL;
    ModuleVia_t                     CurrentDataModule = NULL;
    int                             c;
    int				    c2;
    int                             done = 0;
    int                             recordcount = 0;
    long                            val;
    long                            len;
    OSErr                           bad;
    long                            sz;
    long                            startoffset;
    long                            repcount;
    long                            offset;
    long                            endoffset;
    long                            ndx;
    char                            tempname[256];
    char                            mesg[256];
    DictionaryVia_t                 newdict;
    FSSpec                          filespec;
    ReferenceVia_t                  newref;
    ModuleVia_t                     newmod;
    int                             ID;

#ifdef HDEBUG
    fprintf(dumpFile,"Reading object file: %s\n",fname);
#endif
    
    CurrentCodeModule = NULL;
    CurrentDataModule = NULL;
    result = Ealloc(sizeof(LinkerFile_t));
    Via(result)->ModuleList = NULL;
    Via(result)->DictionaryList = NULL;
    Via(result)->isNESTED = 0;
    Via(result)->version = 0;
    Via(result)->next = NULL;
    strcpy(Via(result)->name, fname);
    bad = FindOMFFile(fname, volrefnum, dirID, &filespec);
    if (bad)
	return NULL;
    thef = fopenMAC((char *) filespec.name, filespec.vRefNum, filespec.parID, "rb");
    while (!done) {
	/* done will be set true when we find endoffile */
	recordcount++;
    gApplication->SpinCursor();
	c = getc(thef);
	if (c == EOF)
	    done = 1;
	else
	    switch (c) {
	    case MPWRec_Pad:
		break;
	    case MPWRec_First:
		c = getc(thef);
		Via(result)->version = fgetw(thef);
		if (c & 1)
		    Via(result)->isNESTED = 1;
		break;
	    case MPWRec_Last:
		c = getc(thef);
		if (c)
		    LinkerError("Bad Last record");
		break;
	    case MPWRec_Comment:
		c = getc(thef);
		sz = fgetw(thef);
		sz -= 4;
		while (sz--)
		    getc(thef);
		break;
	    case MPWRec_Dictionary:
		c = getc(thef);	/* flags */
		sz = fgetw(thef);
		ID = fgetw(thef);
		sz -= 6;
		while (sz > 0) {
		    val = getc(thef);	/* length of next string */
		    sz--;
		    ndx = 0;
		    len = val;
		    while (val--) {
			tempname[ndx++] = getc(thef);
			sz--;
		    }
		    tempname[ndx] = 0;
		    newdict = Ealloc(sizeof(Dictionary_t) + strlen(tempname) + 1);
		    strcpy(Via(newdict)->name, tempname);
		    Via(newdict)->ID = ID++;
#ifdef HDEBUG
		    fprintf(dumpFile,"Dictionary: %s\t\t%d\n",tempname,Via(newdict)->ID);
#endif
		    Via(newdict)->next = Via(result)->DictionaryList;
		    Via(result)->DictionaryList = newdict;
		}
		break;
	    case MPWRec_Module:
		newmod = Ealloc(sizeof(Module_t));
		#ifdef OLDMEM
		HLock((Handle) newmod);
		#endif
		Via(newmod)->Bytes = NULL;
		Via(newmod)->sizeBytes = 0;
		Via(newmod)->file = result;
		Via(newmod)->ReferenceList = NULL;
		Via(newmod)->Entries = NULL;
		val = getc(thef);	/* flags */
		Via(newmod)->isACTIVE = (val & 128);
		Via(newmod)->isMAIN = (val & 16);
		Via(newmod)->isEXTERN = (val & 8);
		Via(newmod)->isENTRY = NULL;
		Via(newmod)->offset = 0;
		Via(newmod)->isCODE = !(val & 1);
		Via(newmod)->externref = 0;
		Via(newmod)->ID = fgetw(thef);
		if (Via(newmod)->isCODE) {
		    Via(newmod)->segment = fgetw(thef);
		    Via(newmod)->modulesize = 0;
		    CurrentCodeModule = newmod;
		} else {
		    Via(newmod)->modulesize = fgetw(thef);
		    Via(newmod)->segment = 0;
		    CurrentDataModule = newmod;
		}
#ifdef HDEBUG
		fprintf(dumpFile,"Module %d\n",Via(newmod)->ID);
#endif
		Via(newmod)->segmentoffset = 0;
		Via(newmod)->A5offset = 0;
		Via(newmod)->next = Via(result)->ModuleList;
		Via(result)->ModuleList = newmod;
		#ifdef OLDMEM
		HUnlock((Handle) newmod);
		#endif
		break;
	    case MPWRec_Entry_Point:
		newmod = Ealloc(sizeof(Module_t));
		Via(newmod)->Bytes = NULL;
		Via(newmod)->sizeBytes = 0;
		Via(newmod)->file = result;
		Via(newmod)->ReferenceList = NULL;
		Via(newmod)->Entries = NULL;
		#ifdef OLDMEM
		HLock((Handle) newmod);
		#endif
		val = getc(thef);
		ID = fgetw(thef);
		offset = fgetl(thef);
		Via(newmod)->ID = ID;
		Via(newmod)->offset = offset;
		Via(newmod)->modulesize = 0;
		Via(newmod)->isEXTERN = (val & 8);
		Via(newmod)->isMAIN = (val & 16);
		Via(newmod)->isCODE = !(val & 1);
		Via(newmod)->segmentoffset = 0;
		Via(newmod)->externref = 0;
		Via(newmod)->A5offset = 0;
		if (val & 1) {
		    Via(newmod)->next = Via(CurrentDataModule)->Entries;
		    Via(CurrentDataModule)->Entries = newmod;
		    Via(newmod)->segment = Via(CurrentDataModule)->segment;
		    Via(newmod)->isENTRY = CurrentDataModule;
		} else {
		    Via(newmod)->next = Via(CurrentCodeModule)->Entries;
		    Via(CurrentCodeModule)->Entries = newmod;
		    Via(newmod)->segment = Via(CurrentCodeModule)->segment;
		    Via(newmod)->isENTRY = CurrentCodeModule;
		}
		#ifdef OLDMEM
		HUnlock((Handle) newmod);
		#endif
		break;
	    case MPWRec_Size:
		val = getc(thef);
		sz = fgetl(thef);
		if (val & 1) {
		    if (sz > Via(CurrentDataModule)->modulesize)
			Via(CurrentDataModule)->modulesize = sz;
		} else {
		    if (sz > Via(CurrentCodeModule)->modulesize)
			Via(CurrentCodeModule)->modulesize = sz;
		}
		break;
	    case MPWRec_Contents:
		val = getc(thef);
		if (val & 1) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Data Contents:\n");
#endif
		    newmod = CurrentDataModule;
		}
		else {
#ifdef HDEBUG
		    fprintf(dumpFile,"Code Contents:\n");
#endif
		    newmod = CurrentCodeModule;
		}
		sz = fgetw(thef);
		sz -= 4;
		offset = 0;
		if (val & 8) {
		    sz -= 4;
		    offset = fgetl(thef);
		}
		repcount = 1;
		if (val & 16) {
		    sz -= 2;
		    repcount = fgetw(thef);
		}
#ifdef HDEBUG
		fprintf(dumpFile,"Contents:sz %d offset %ld repcount %d\n",sz,offset,repcount);
#endif
		if (Via(newmod)->Bytes) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Contents:Bytes already exists\n");
#endif
	    	    if (((sz * repcount) + offset) > Via(newmod)->sizeBytes) {
		    #ifdef OLDMEM
				SetHandleSize((Handle) Via(newmod)->Bytes, (sz * repcount) + offset + 10);
			#else
			{
				char *newBytes;
				newBytes = (char *) icemalloc((sz * repcount) + offset + 10);
				memcpy(newBytes,Via(newmod)->Bytes,Via(newmod)->sizeBytes);
				Via(newmod)->sizeBytes = ((sz * repcount) + offset + 10);
				icefree(Via(newmod)->Bytes);
				Via(newmod)->Bytes = newBytes;
			}
			#endif
				if (MemError()) {
				    LinkerError("Unable to allocate memory for second contents record");
				    return NULL;
				}
		    }
		} else {
		    Via(newmod)->Bytes = Ealloc((sz * repcount) + offset + 10);
		    Via(newmod)->sizeBytes = ((sz * repcount) + offset + 10);
		}
		{
			int count = 0;
			startoffset = offset;
#ifdef HDEBUG
			fprintf(dumpFile,"Contents values: \n");
#endif
			while (sz--) {
			    c = getc(thef);
			    count++;
			    if (!(val & 1)) {
#ifdef HDEBUG
			    	fprintf(dumpFile," %X",c);
			    	if (count==8) {fprintf(dumpFile,"\n");count = 0;}
#endif
			    }
			    Via(Via(newmod)->Bytes)[offset++] = c;
			}
#ifdef HDEBUG
			    fprintf(dumpFile,"\n");
			    if (!(val & 1) && newmod->ID == 529) {
   			    char buf[256];
			    sprintf(buf,"Hex dump of contents:");
			    hex_dump(dumpFile,buf,newmod->Bytes,newmod->sizeBytes);
			    Debugger();
			    }
#endif
		}
		endoffset = offset;
		while (--repcount) {
#ifdef HDEBUG
			if (!(val & 1)) fprintf(dumpFile,"Repeating\n");
#endif
		        ndx = startoffset;
		        while (ndx < endoffset) {
			     Via(Via(newmod)->Bytes)[offset++] = Via(Via(newmod)->Bytes)[ndx++];
		        }
		}
		if (Via(newmod)->modulesize < offset)
		    Via(newmod)->modulesize = offset;
		break;
	    case MPWRec_Reference:
		val = getc(thef);
		sz = fgetw(thef);
		newref = Ealloc(sizeof(Reference_t));
		#ifdef OLDMEM
		HLock((Handle) newref);
		#endif
		Via(newref)->isCOMPUTED = 0;
		Via(newref)->ID = fgetw(thef);
		Via(newref)->isA5REL = (val & 128);
		if (val & 16)
		    Via(newref)->patchsize = 16;
		else
		    Via(newref)->patchsize = 32;
		Via(newref)->WhichModule = NULL;
		Via(newref)->OtherModule = NULL;
		sz -= 6;
		if (val & 8)
		    Via(newref)->RefCount = sz / 4;
		else
		    Via(newref)->RefCount = sz / 2;
		Via(newref)->Offsets = Ealloc(sizeof(long) * Via(newref)->RefCount + 1);
		ndx = 0;
		while (ndx < Via(newref)->RefCount) {
		    if (val & 8)
			Via(Via(newref)->Offsets)[ndx] = fgetl(thef);
		    else
			Via(Via(newref)->Offsets)[ndx] = fgetw(thef);
#ifdef HDEBUG
		        fprintf(dumpFile,"Reference to %x size %d offset %x\n",
		    	Via(newref)->ID,Via(newref)->patchsize,Via(Via(newref)->Offsets)[ndx]);
#endif
		    ndx++;
		}
		if (val & 1)
		    newmod = CurrentDataModule;
		else
		    newmod = CurrentCodeModule;
		Via(newref)->next = Via(newmod)->ReferenceList;
		Via(newmod)->ReferenceList = newref;
		#ifdef OLDMEM
		HUnlock((Handle) newref);
		#endif
		break;
	    case MPWRec_Computed_Reference:
		val = getc(thef);
		sz = fgetw(thef);
		newref = Ealloc(sizeof(Reference_t));
		#ifdef OLDMEM
		HLock((Handle) newref);
		#endif
		Via(newref)->isCOMPUTED = 1;
		Via(newref)->ID = fgetw(thef);
		Via(newref)->ID2 = fgetw(thef);
		if (val & 16)
		    Via(newref)->patchsize = 16;
		else if (val & 32)
		    Via(newref)->patchsize = 8;
		else
		    Via(newref)->patchsize = 32;
		Via(newref)->WhichModule = NULL;
		Via(newref)->OtherModule = NULL;
		sz -= 8;
		if (val & 8)
		    Via(newref)->RefCount = sz / 4;
		else
		    Via(newref)->RefCount = sz / 2;
		Via(newref)->Offsets = Ealloc(sizeof(long) * Via(newref)->RefCount + 1);
		ndx = 0;
		while (ndx < Via(newref)->RefCount) {
		    if (val & 8)
			Via(Via(newref)->Offsets)[ndx] = fgetl(thef);
		    else
			Via(Via(newref)->Offsets)[ndx] = fgetw(thef);
		    ndx++;
		}
		if (val & 1)
		    newmod = CurrentDataModule;
		else
		    newmod = CurrentCodeModule;
		Via(newref)->next = Via(newmod)->ReferenceList;
		Via(newmod)->ReferenceList = newref;
		#ifdef OLDMEM
		HUnlock((Handle) newref);
		#endif
		break;
	    default:
		LinkerError("Unrecognized record type");
		break;
	    }
    }
	fclose(thef);
#ifdef HDEBUG
	fprintf(dumpFile,"Done reading %s (%d records)\n",fname,recordcount);
#endif
    return result;
}

ModuleVia_t
FindModuleID(unsigned short ID, LinkerFileVia_t file)
/*
 * Given the data structures for a single OBJ file, search for a Module with
 * the given ID.
 */
{
    ModuleVia_t                     cur = NULL;
    ModuleVia_t                     ecur;
    if (file)
	cur = Via(file)->ModuleList;
    while (cur) {
	if (Via(cur)->ID == ID)
	    return cur;
	ecur = Via(cur)->Entries;
	while (ecur) {
	    if (Via(ecur)->ID == ID) {
		return ecur;
	    }
	    ecur = Via(ecur)->next;
	}
	cur = Via(cur)->next;
    }
    return NULL;
}

DictionaryVia_t
FindStrName(char *name, LinkerFileVia_t file)
/*
 * Given the data structures for a single OBJ file, search for a Dictionary
 * entry with the given name.
 */
{
    DictionaryVia_t                 cur;
    cur = Via(file)->DictionaryList;
    while (cur) {
	if (!strcmp((name), (Via(cur)->name)))
	    return cur;
	cur = Via(cur)->next;
    }
    return NULL;
}

ModuleVia_t
FindNameAll(char *name, LinkerFileVia_t file)
/* Given a name, search all files for a module with that name */
{
    DictionaryVia_t                 cur;
    ModuleVia_t                     mod;
    while (file) {
	cur = Via(file)->DictionaryList;
	while (cur) {
	    if (!strcmp((name), (Via(cur)->name))) {
		mod = FindModuleID(Via(cur)->ID, file);
		if (mod)
		    return mod;
	    }
	    cur = Via(cur)->next;
	}
	file = Via(file)->next;
    }
    return NULL;
}

DictionaryVia_t
FindIDName(unsigned short ID, LinkerFileVia_t file)
/*
 * Given the data structures for a single OBJ file, search for a Dictionary
 * entry with the given ID.
 */
{
    DictionaryVia_t                 cur = NULL;
    if (file)
	cur = Via(file)->DictionaryList;
    while (cur) {
	if (Via(cur)->ID == ID)
	    return cur;
	cur = Via(cur)->next;
    }
    return NULL;
}

ModuleVia_t
FindModuleExternal(unsigned short ID, LinkerFileVia_t localfile, DictionaryVia_t * dict,
		   LinkerFileVia_t * foundfile, LinkerFileVia_t allfiles)
/*
 * Given a single OBJ file and an ID within that file, look up the name for
 * that ID.  Then, search all other files (given the file list), for a Module
 * with that name.  Return the module found.
 */
{
    ModuleVia_t                     result = NULL;
    DictionaryVia_t                 namerec;
    namerec = FindIDName(ID, localfile);
    if (namerec) {
	DictionaryVia_t                 testname;
	LinkerFileVia_t                 curfile;
	curfile = allfiles;
#ifdef OLDMEM
	HLock((Handle) namerec);
#endif
	while (curfile) {
	    if (curfile != localfile) {
		testname = FindStrName(Via(namerec)->name, curfile);
		if (testname) {
		    result = FindModuleID(Via(testname)->ID, curfile);
		    if (result) {
			if (Via(result)->isEXTERN) {
			    *dict = testname;
			    *foundfile = curfile;
			    return result;
			}
			else {
			    result = NULL;
			}
		    }
		}
	    }
	    curfile = Via(curfile)->next;
	}
#ifdef OLDMEM
	HUnlock((Handle) namerec);
#endif
    }
    return result;
}

int
ResolveAllReferences(LinkerFileVia_t files)
/* Resolves all references, returns the number of unresolved */
{
    int                             unresolved = 0;
    LinkerFileVia_t                 curfile = files;
    LinkerFileVia_t                 otherfile = NULL;
    DictionaryVia_t                 modname = NULL;
    ModuleVia_t                     curmod;
    ModuleVia_t                     doubdef;
    ReferenceVia_t                  curref;
    char                            err[256];

    while (curfile) {
#ifdef HDEBUG
    	fprintf(dumpFile,"Resolving references for %s\n",curfile->name);
#endif
	curmod = Via(curfile)->ModuleList;
	while (curmod) {
	    ModuleVia_t                     referenced = NULL;
	    gApplication->SpinCursor();
	    /* First, we check for double definitions. */
	    modname = NULL;
	    otherfile = NULL;
	    doubdef = FindModuleExternal(Via(curmod)->ID, curfile, &modname, &otherfile, files);
	    if (doubdef && modname) {
		DictionaryVia_t                 dict;
		if (doubdef->isEXTERN && curmod->isEXTERN) {
			dict = FindIDName(Via(curmod)->ID, curfile);
			#ifdef OLDMEM
			HLock((Handle) modname);
			HLock((Handle) otherfile);
			HLock((Handle) curfile);
			HLock((Handle) dict);
			#endif
			sprintf(err, "# Doubly defined : %s(%d) in %s found as %s(%d) in file %s",
				Via(dict)->name, Via(dict)->ID,
				Via(curfile)->name, Via(modname)->name, Via(modname)->ID, Via(otherfile)->name);
			#ifdef OLDMEM
			HUnlock((Handle) dict);
			HUnlock((Handle) modname);
			HUnlock((Handle) otherfile);
			HUnlock((Handle) curfile);
			#endif
			LinkerError(err);
		}
	    }
	    curref = Via(curmod)->ReferenceList;
	    while (curref) {
		if (referenced = FindModuleID(Via(curref)->ID, curfile)) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Reference to %d found within file\n",Via(curref)->ID);
#endif
		    Via(referenced)->isACTIVE = 1;
		    if (Via(curmod)->isCODE) {
			if (Via(referenced)->isCODE) {
			    Via(curref)->patchkind = 1;
			} else {
			    Via(curref)->patchkind = 2;
			}
		    } else {
			if (Via(referenced)->isCODE) {
			    Via(curref)->patchkind = 3;
			} else {
			    Via(curref)->patchkind = 4;
			}
		    }
		    if (Via(referenced)->isENTRY) {
			Via(Via(referenced)->isENTRY)->isACTIVE = 1;
		    }
		    Via(curref)->WhichModule = referenced;
		} else if (referenced =
			   FindModuleExternal(Via(curref)->ID, curfile, &modname, &otherfile, files)) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Reference to %s(%d) found in %s as %d\n",
		    	modname->name,Via(curref)->ID,otherfile->name,referenced->ID);
#endif
		    Via(referenced)->externref = 1;
		    if (Via(curmod)->isCODE) {
			if (Via(referenced)->isCODE) {
			    Via(curref)->patchkind = 1;
			} else {
			    Via(curref)->patchkind = 2;
			}
		    } else {
			if (Via(referenced)->isCODE) {
			    Via(curref)->patchkind = 3;
			} else {
			    Via(curref)->patchkind = 4;
			}
		    }
		    if (Via(referenced)->isENTRY) {
			Via(Via(referenced)->isENTRY)->isACTIVE = 1;
		    }
		    Via(referenced)->isACTIVE = 1;
		    Via(curref)->WhichModule = referenced;
		} else {
		    DictionaryVia_t                 dict;
		    dict = FindIDName(Via(curref)->ID, curfile);
		    if (dict) {
		    #ifdef OLDMEM
			HLock((Handle) dict);
		#endif
			sprintf(err, "# Unresolved : %s, referenced in %s",
				Via(dict)->name,
				Via(curfile)->name);
			LinkerError(err);
		    #ifdef OLDMEM
			HUnlock((Handle) dict);
			#endif
		    }
		    unresolved++;
		    Via(curref)->WhichModule = NULL;
		}
		if (Via(curref)->isCOMPUTED) {
		    if (referenced = FindModuleID(Via(curref)->ID2, curfile)) {
			Via(referenced)->isACTIVE = 1;
			Via(curref)->OtherModule = referenced;
			if (Via(referenced)->isENTRY) {
			    Via(Via(referenced)->isENTRY)->isACTIVE = 1;
			}
		    } else if (referenced =
			       FindModuleExternal(Via(curref)->ID2, curfile, &modname, &otherfile, files)) {
			Via(referenced)->isACTIVE = 1;
			if (Via(referenced)->isENTRY) {
			    Via(Via(referenced)->isENTRY)->isACTIVE = 1;
			}
			Via(referenced)->externref = 1;
			Via(curref)->OtherModule = referenced;
		    } else {
			DictionaryVia_t                 dict;
			dict = FindIDName(Via(curref)->ID2, curfile);
			if (dict) {
			#ifdef OLDMEM
			    HLock((Handle) dict);
			    #endif
			    sprintf(err, "# Unresolved : %s, referenced in %s",
				    Via(dict)->name,
				    Via(curfile)->name);
			    LinkerError(err);
			    #ifdef OLDMEM
			    HUnlock((Handle) dict);
			    #endif
			}
			unresolved++;
			Via(curref)->OtherModule = NULL;
		    }
		}
		curref = Via(curref)->next;
	    }
	    curmod = Via(curmod)->next;
	}
	curfile = Via(curfile)->next;
    }
    return unresolved;
}

#define STARTINGDATAOFFSET (-128)
#define A5TOJT 32
#define MAINOFFSET (A5TOJT)
#define JTENTRYSIZE 8
#define STARTINGJTOFFSET (MAINOFFSET+JTENTRYSIZE)
/*
 * The 32 is the distance from A5 to the jumptable.  The 8 is the length of
 * one jump table entry, since the first one is reserved for MAIN.
 */

long                            JTEntries = 1;
long                            DataBlockSize = 0;
long                            NextCodeOffset = STARTINGJTOFFSET;
long                            NextDataOffset = STARTINGDATAOFFSET;

SegmentVia_t                    SegmentList = NULL;

SegmentVia_t
FindSegmentID(unsigned short ID)
/*
 * Given an ID for a segment, search the global segment list for a segment
 * with that ID.
 */
{
    SegmentVia_t                    cur;
    cur = SegmentList;
    while (cur) {
	if (Via(cur)->ID == ID)
	    return cur;
	cur = Via(cur)->next;
    }
    return NULL;
}

SegmentVia_t
FindSegmentName(char *name)
{
    SegmentVia_t                    cur;
    cur = SegmentList;
    while (cur) {
    #ifdef OLDMEM
	HLock((Handle) Via(cur)->name);
	#endif
	if (!strcmp(name, Via(Via(cur)->name))) {
		#ifdef OLDMEM
	    HUnlock((Handle) Via(cur)->name);
	    #endif
	    return cur;
	}
	#ifdef OLDMEM
	HUnlock((Handle) Via(cur)->name);
	#endif
	cur = Via(cur)->next;
    }
    return NULL;
}

unsigned short                  NextSegID;

SegmentVia_t
AddSegment(char *name, LinkerFileVia_t file)
/*
 * Given an ID for a segment, add a new segment to the global segment list
 * with that ID.
 */
{
    SegmentVia_t                    raw;
    char                            nm[64];
    raw = Ealloc(sizeof(Segment_t));
    Via(raw)->ID = NextSegID++;
    Via(raw)->name = Ealloc(strlen(name) + 1);
    #ifdef OLDMEM
    HLock((Handle) Via(raw)->name);
    #endif
    strcpy(Via(Via(raw)->name), name);
    #ifdef OLDMEM
    HUnlock((Handle) Via(raw)->name);
    #endif

    Via(raw)->ModuleList = NULL;
    Via(raw)->firstJT = 0;
    Via(raw)->countJT = 0;
    Via(raw)->CurrentSize = 0;
    Via(raw)->next = SegmentList;
    SegmentList = raw;
    return raw;
}

DATAZone_t                      theDATA;
Segment_t                       theJT;

void
AddToDATA(ModuleVia_t themod)
/* Given a module, add it to the DATA block */
{
    ModulePackageVia_t              modp;

    if (Via(themod)->isACTIVE) {
	modp = Ealloc(sizeof(ModulePackage_t));
	Via(modp)->next = theDATA.ModuleList;
	Via(modp)->themod = themod;
	theDATA.ModuleList = modp;
	theDATA.count++;
	theDATA.CurrentSize += Via(themod)->modulesize;
	if ((Via(themod)->modulesize) % 2)
	    theDATA.CurrentSize++;
    }
}

 char                  *
AddImage(ModulePackageVia_t modp,  char *buf)
{
    int                             ndx;
    if (modp) {
	if (Via(modp)->next) {
	    buf = AddImage(Via(modp)->next, buf);
	}
	if (Via(Via(modp)->themod)->isACTIVE) {
	    if (((unsigned long) buf) % 2)
		buf++;
	    ndx = 0;
	    while (ndx < Via(Via(modp)->themod)->modulesize) {
		*buf++ = Via(Via(Via(modp)->themod)->Bytes)[ndx++];
	    }
	}
    }
    return buf;
}

 char                  *
AddDataImage(ModulePackageVia_t modp,  char *buf)
{
    int                             ndx;
    if (modp) {
	if (Via(modp)->next) {
	    buf = AddDataImage(Via(modp)->next, buf);
	}
	if (Via(Via(modp)->themod)->isACTIVE) {
	    ndx = 0;
	    while (ndx < Via(Via(modp)->themod)->modulesize) {
		*buf++ = Via(Via(Via(modp)->themod)->Bytes)[ndx++];
	    }
	}
    }
    return buf;
}

Block_t
ImageDATA(void)
{
    EString_t						result;
    Block_t                         theb;
    ModulePackageVia_t              cur;
    ReferenceVia_t                  curref;
    int                             ndx3;
    ModuleVia_t                     curmod;
    char                            msg[256];
    unsigned long                   lastoff, nextbyte;
    unsigned long                   thisoff;
    unsigned long                   delta;
    unsigned long                   relocs;
    unsigned long                   ndx;

    result = (EString_t) Ealloc(theDATA.CurrentSize + theDATA.count * 10 + 4096);

    theb.mem = ( EString_t) result;

    lastoff = 0;
    nextbyte = 32;
    cur = theDATA.ModuleList;
    while (cur) {
	curmod = Via(cur)->themod;
	if (Via(curmod)->isACTIVE) {
	    if (Via(curmod)->Bytes) {
		thisoff = Via(curmod)->A5offset - Via(Via(theDATA.ModuleList)->themod)->A5offset;
		delta = thisoff - lastoff;
		lastoff = thisoff + Via(curmod)->modulesize;
		Via(result)[nextbyte++] = 0;
		Via(result)[nextbyte++] = 224;
		Via(result)[nextbyte++] = Via(curmod)->modulesize >> 24;
		Via(result)[nextbyte++] = Via(curmod)->modulesize >> 16;
		Via(result)[nextbyte++] = Via(curmod)->modulesize >> 8;
		Via(result)[nextbyte++] = Via(curmod)->modulesize;
		Via(result)[nextbyte++] = 224;
		Via(result)[nextbyte++] = delta >> 24;
		Via(result)[nextbyte++] = delta >> 16;
		Via(result)[nextbyte++] = delta >> 8;
		Via(result)[nextbyte++] = delta;
		ndx = 0;
		while (ndx < Via(curmod)->modulesize) {
		    Via(result)[nextbyte++] = Via(Via(curmod)->Bytes)[ndx++];
		}
	    }
	}
	cur = Via(cur)->next;
    }
    Via(result)[nextbyte++] = 0x20;
    Via(result)[nextbyte++] = 0x00;

    relocs = nextbyte;

    lastoff = 0;
    cur = theDATA.ModuleList;
    while (cur) {
	curmod = Via(cur)->themod;
	if (Via(curmod)->isACTIVE) {
	    if (Via(curmod)->ReferenceList) {
		curref = Via(curmod)->ReferenceList;
		while (curref) {
		    if (!Via(curref)->isA5REL) {
			ndx3 = 0;
			while (ndx3 < Via(curref)->RefCount) {
			    thisoff = Via(curmod)->A5offset - Via(Via(theDATA.ModuleList)->themod)->A5offset +
				Via(Via(curref)->Offsets)[ndx3];
			    delta = thisoff - lastoff;
			    lastoff = thisoff;
			    delta = (delta >> 1) | 0x80000000;
			    Via(result)[nextbyte++] = 0;
			    Via(result)[nextbyte++] = (delta >> 24);
			    Via(result)[nextbyte++] = (delta >> 16);
			    Via(result)[nextbyte++] = (delta >> 8);
			    Via(result)[nextbyte++] = (delta);
			    ndx3++;
			}
		    }
		    curref = Via(curref)->next;
		}
	    }
	}
	cur = Via(cur)->next;
    }
    Via(result)[nextbyte++] = 0x00;
    Via(result)[nextbyte++] = 0x00;
    theb.size = nextbyte;

    Via(result)[0] = (-NextDataOffset) >> 24;
    Via(result)[1] = (-NextDataOffset) >> 16;
    Via(result)[2] = (-NextDataOffset) >> 8;
    Via(result)[3] = (-NextDataOffset);

    Via(result)[4] = 0;
    Via(result)[5] = 1;

    Via(result)[6] = 0;
    Via(result)[7] = 0;

    Via(result)[8] = 0;
    Via(result)[9] = 0;
    Via(result)[10] = 0;
    Via(result)[11] = 32;

    Via(result)[12] = relocs >> 24;
    Via(result)[13] = relocs >> 16;
    Via(result)[14] = relocs >> 8;
    Via(result)[15] = relocs;

    Via(result)[16] = 0;
    Via(result)[17] = 0;
    Via(result)[18] = 0;
    Via(result)[19] = 0;

    return theb;
}

void
SetFirstJT(SegmentVia_t seg)
{
    long                            offset = 9999999;
    ModulePackageVia_t              modp;
    modp = Via(seg)->ModuleList;
    while (modp) {
	if (Via(Via(modp)->themod)->A5offset < offset) {
	    offset = Via(Via(modp)->themod)->A5offset;
	}
	modp = Via(modp)->next;
    }
    Via(seg)->firstJT = offset - 2 - MAINOFFSET;
}

Block_t
ImageCODE(SegmentVia_t seg)
{
    EString_t						result;
    Block_t                         theb;
    unsigned long                   sz;
    SetFirstJT(seg);
    sz = Via(seg)->CurrentSize + 4;
    if (sz % 2)
	sz++;
    result = ( EString_t) Ealloc(sz);
    theb.mem = result;
    theb.size = Via(seg)->CurrentSize + 4;
    #ifdef OLDMEM
    HLock((Handle) result);
    #endif
    AddImage(Via(seg)->ModuleList, Via(result) + 4);
    Via(result)[0] = Via(seg)->firstJT >> 8;
    Via(result)[1] = Via(seg)->firstJT;
    Via(result)[2] = Via(seg)->countJT >> 8;
    Via(result)[3] = Via(seg)->countJT;
    #ifdef OLDMEM
    HUnlock((Handle) result);
    #endif
    return theb;
}

SegmentVia_t
GetSeg(int SegID, LinkerFileVia_t file)
{
    DictionaryVia_t                 segname;
    char                            nm[64];
    segname = FindIDName(SegID, file);
    if (segname) {
	strcpy(nm, Via(segname)->name);
    } else {
	strcpy(nm, "%?Anon");
    }
    return FindSegmentName(nm);
}

void
AddToSegment(unsigned short SegID, ModuleVia_t themod, LinkerFileVia_t file)
{
    SegmentVia_t                    theseg;
    ModulePackageVia_t              modp;
    DictionaryVia_t                 segname;
    DictionaryVia_t                 modname;
    char                            nm[64];
    char                            msg[256];

    if (!Via(themod)->isACTIVE)
	return;
    segname = FindIDName(SegID, file);
    if (segname) {
	strcpy(nm, Via(segname)->name);
    } else {
	strcpy(nm, "%?Anon");
    }
    theseg = FindSegmentName(nm);
    if (!theseg) {
	theseg = AddSegment(nm, file);
    }
    if (Via(theseg)->CurrentSize % 2)
	Via(theseg)->CurrentSize++;
    modp = Ealloc(sizeof(ModulePackage_t));
    Via(modp)->next = Via(theseg)->ModuleList;
    Via(modp)->themod = themod;
    Via(theseg)->ModuleList = modp;
    Via(themod)->segmentoffset = Via(theseg)->CurrentSize;
    Via(theseg)->CurrentSize += Via(themod)->modulesize;
}

ModuleVia_t                     MAINmodule = NULL;

char                            recerr[128];

void
AssignModules(ModuleVia_t curmod, LinkerFileVia_t curfile)
{
    ModuleVia_t                     curmod2;
    DictionaryVia_t                 dict;
    if (!curmod) return;
    if (Via(curmod)->next) {
	AssignModules(Via(curmod)->next, curfile);
    }
    if (Via(curmod)->isACTIVE) {
	if (Via(curmod)->isCODE) {
	    /* Add this routine to its segment. */
	    AddToSegment(Via(curmod)->segment, curmod, curfile);
	} else {		/* isDATA */
	    if ((Via(curmod)->modulesize) % 2)
		Via(curmod)->modulesize++;
	    NextDataOffset -= Via(curmod)->modulesize;
	    if (!Via(curmod)->modulesize) {
		dict = FindIDName(Via(curmod)->ID, curfile);
		if (dict) {
		#ifdef OLDMEM
		    HLock((Handle) dict);
		    #endif
		    sprintf(recerr, "# Zero sized data module : %s in %s",
			    Via(dict)->name,
			    Via(curfile)->name);
		    LinkerError(recerr);
		    #ifdef OLDMEM
		    HUnlock((Handle) dict);
		    #endif
		}
	    }
	    Via(curmod)->A5offset = NextDataOffset;
	    AddToDATA(curmod);
	}
	if (Via(curmod)->Entries) {
	    curmod2 = Via(curmod)->Entries;
	    while (curmod2) {
		Via(curmod2)->segmentoffset = Via(curmod)->segmentoffset + Via(curmod2)->offset;
		if (!Via(curmod2)->isCODE) {
#ifdef Undefined
		    NextDataOffset -= Via(curmod2)->modulesize;
		    Via(curmod2)->A5offset = NextDataOffset;
#else
		    Via(curmod2)->A5offset = Via(curmod)->A5offset + Via(curmod2)->offset;
#endif
		}
		curmod2 = Via(curmod2)->next;
	    }
	}
    }
}

void
AssignA5Offsets(SegmentVia_t curseg)
{
    ModulePackageVia_t              modp;
    ModuleVia_t                     curmod;
    ModuleVia_t                     curmod2;
    modp = Via(curseg)->ModuleList;
    while (modp) {
	curmod = Via(modp)->themod;
	if (Via(curmod)->isACTIVE) {
	    if (Via(curmod)->isMAIN) {
		Via(curmod)->A5offset = MAINOFFSET + 2;
	    } else {
		Via(curmod)->A5offset = NextCodeOffset + 2;
		Via(curseg)->countJT++;
		JTEntries++;
		NextCodeOffset += JTENTRYSIZE;
	    }
	    if (Via(curmod)->Entries) {
		curmod2 = Via(curmod)->Entries;
		while (curmod2) {
		    if (Via(curmod2)->isMAIN) {
			Via(curmod2)->A5offset = MAINOFFSET + 2;
		    } else {
			Via(curmod2)->A5offset = NextCodeOffset + 2;
			Via(curseg)->countJT++;
			JTEntries++;
			NextCodeOffset += JTENTRYSIZE;
		    }
		    curmod2 = Via(curmod2)->next;
		}
	    }
	}
	modp = Via(modp)->next;
    }
}

void
AssignMPWOffsets(LinkerFileVia_t files)
{
    LinkerFileVia_t                 curfile = files;
    ModuleVia_t                     curmod;
    ModuleVia_t                     curmod2;
    SegmentVia_t                    theseg;
    SegmentVia_t                    curseg;
    ModulePackageVia_t              modp;
    DictionaryVia_t                 segname;
    DictionaryVia_t                 modname;
    char                            nm[64];
    char                            msg[256];

    if (!MAINmodule) {
	MAINmodule = FindNameAll("%__MAIN", files);
	if (MAINmodule) {
	    Via(MAINmodule)->A5offset = MAINOFFSET + 2;
	    Via(MAINmodule)->isACTIVE = 1;
	    Via(MAINmodule)->isMAIN = 1;
	    if (Via(MAINmodule)->isENTRY) {
		Via(Via(MAINmodule)->isENTRY)->isACTIVE = 1;
	    }
	}
	MAINmodule = NULL;
    }
    while (curfile) {
		curmod = Via(curfile)->ModuleList;
		AssignModules(curmod, curfile);
		curfile = Via(curfile)->next;
    }
    if (!MAINmodule) {
	MAINmodule = FindNameAll("%__MAIN", files);
	if (MAINmodule) {
	    Via(MAINmodule)->A5offset = MAINOFFSET + 2;
	    Via(MAINmodule)->isACTIVE = 1;
	    Via(MAINmodule)->isMAIN = 1;
	    if (Via(MAINmodule)->isENTRY) {
			Via(Via(MAINmodule)->isENTRY)->isACTIVE = 1;
	    }
	}
    }
    if (!MAINmodule) {
		LinkerError("No main module");
    }
    /*
     * We need to find the main module, find out what segment it is in, and
     * make that segment # 1, and assign all its MPW offsets first
     */

	segname = NULL;
	if (MAINmodule)
   		segname = FindIDName(Via(MAINmodule)->segment, Via(MAINmodule)->file);
    if (segname) {
		strcpy(nm, Via(segname)->name);
    } else {
		strcpy(nm, "%?Anon");
    }
    theseg = FindSegmentName(nm);
    if (theseg) {
    	Via(theseg)->ID = 1;
    	AssignA5Offsets(theseg);
    	Via(theseg)->countJT++;
	    curseg = SegmentList;
	    while (curseg) {
			if (curseg != theseg) {
			    AssignA5Offsets(curseg);
			}
			curseg = Via(curseg)->next;
	    }
	}
}

Block_t
ConcatImage(Block_t a, Block_t b, LinkerFileVia_t files)
{
    Block_t                         theb;
    EString_t						result;
    long                            ndx1, ndx2;
    ModuleVia_t                     a5init3;
    unsigned long                   datainit;
    int                             asz;

    a5init3 = FindNameAll("_A5Init3", files);
    datainit = Via(a5init3)->segmentoffset + 4;
    asz = a.size + b.size;
    while (asz % 4)
	asz++;

    result = Ealloc(asz + 8);
    theb.mem = result;
    theb.size = asz + 8;
    ndx1 = 0;
    ndx2 = 0;
    while (ndx2 < a.size) {
	Via(result)[ndx1++] = Via(a.mem)[ndx2++];
    }
    ndx1 = datainit;
    ndx2 = 0;
    while (ndx2 < b.size) {
	Via(result)[ndx1++] = Via(b.mem)[ndx2++];
    }
    ndx1 = asz;
    Via(result)[ndx1++] = datainit >> 24;
    Via(result)[ndx1++] = datainit >> 16;
    Via(result)[ndx1++] = datainit >> 8;
    Via(result)[ndx1++] = datainit;

    Via(result)[ndx1++] = 'm';
    Via(result)[ndx1++] = 'p';
    Via(result)[ndx1++] = 'w';
    Via(result)[ndx1++] = 'd';
    return theb;
}

Block_t
ImageJT(void)
{
    SegmentVia_t                    curseg;
    Block_t                         theb;
    DictionaryVia_t                 segname;
    ModulePackageVia_t              curmod;
    ModuleVia_t                     curmod2;
    EString_t						result;
    long                            offset;
    char                            nm[64];

    if (!MAINmodule) {
	theb.mem = NULL;
	theb.size = 0;
	return theb;
    }
    result = Ealloc(16 + (JTEntries) * 8);
    theb.mem = result;
    theb.size = 16 + (JTEntries * 8);
    curseg = SegmentList;
    while (curseg) {
	curmod = Via(curseg)->ModuleList;
	while (curmod) {
	    if (Via(Via(curmod)->themod)->isACTIVE) {
		offset = Via(Via(curmod)->themod)->A5offset - 16;
		offset -= 2;
		Via(result)[offset] = Via(Via(curmod)->themod)->segmentoffset >> 8;
		Via(result)[offset + 1] = Via(Via(curmod)->themod)->segmentoffset;
		Via(result)[offset + 2] = 0x3f;
		Via(result)[offset + 3] = 0x3c;
		Via(result)[offset + 4] = Via(curseg)->ID >> 8;
		Via(result)[offset + 5] = Via(curseg)->ID;
		Via(result)[offset + 6] = 0xa9;
		Via(result)[offset + 7] = 0xf0;
		if (Via(Via(curmod)->themod)->Entries) {
		    curmod2 = Via(Via(curmod)->themod)->Entries;
		    while (curmod2) {
			offset = Via(curmod2)->A5offset - 16;
			offset -= 2;
			Via(result)[offset] = Via(curmod2)->segmentoffset >> 8;
			Via(result)[offset + 1] = Via(curmod2)->segmentoffset;
			Via(result)[offset + 2] = 0x3f;
			Via(result)[offset + 3] = 0x3c;
			Via(result)[offset + 4] = Via(curseg)->ID >> 8;
			Via(result)[offset + 5] = Via(curseg)->ID;
			Via(result)[offset + 6] = 0xa9;
			Via(result)[offset + 7] = 0xf0;
			curmod2 = Via(curmod2)->next;
		    }
		}
	    }
	    curmod = Via(curmod)->next;
	}
	curseg = Via(curseg)->next;
    }
    /* Now we store the header values */
    Via(result)[0] = (theb.size + 16) >> 24;
    Via(result)[1] = (theb.size + 16) >> 16;
    Via(result)[2] = (theb.size + 16) >> 8;
    Via(result)[3] = (theb.size + 16);
    Via(result)[4] = (-NextDataOffset) >> 24;
    Via(result)[5] = (-NextDataOffset) >> 16;
    Via(result)[6] = (-NextDataOffset) >> 8;
    Via(result)[7] = (-NextDataOffset);
    Via(result)[8] = (theb.size - 16) >> 24;
    Via(result)[9] = (theb.size - 16) >> 16;
    Via(result)[10] = (theb.size - 16) >> 8;
    Via(result)[11] = (theb.size - 16);
    Via(result)[12] = A5TOJT >> 24;
    Via(result)[13] = A5TOJT >> 16;
    Via(result)[14] = A5TOJT >> 8;
    Via(result)[15] = A5TOJT;
    return theb;
}

void
AdjustOneRef(ModuleVia_t curmod)
{
    ReferenceVia_t                  curref;
    ModuleVia_t                     curmod2;
    long                            theoffset;
    int                             ndx;
    if (!Via(curmod)->Bytes) {
    	return;
    }
    if (!Via(curmod)->sizeBytes) {
    	return;
    }
#ifdef OLDMEM
    HLock((Handle) Via(curmod)->Bytes);
#endif

#ifdef HDEBUG
    fprintf(dumpFile,"Adjusting references for module %d\n",curmod->ID);
    if (curmod->ID == 529) Debugger();
#endif
    if (Via(curmod)->isACTIVE) {
	curref = Via(curmod)->ReferenceList;
	while (curref) {
	    switch (Via(curref)->patchkind) {
	    case 1:
		if (Via(curref)->isCOMPUTED) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Next ref computed\n");
#endif
		    theoffset = Via(Via(curref)->WhichModule)->segmentoffset -
			Via(Via(curref)->OtherModule)->segmentoffset;
		    ndx = 0;
		    while (ndx < Via(curref)->RefCount) {
			char                           *thepoint;
			thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 1 at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
			switch (Via(curref)->patchsize) {
			case 32:
			    *((long *) thepoint) += theoffset;
			    break;
			case 16:
			    *((short *) thepoint) += theoffset;
			    break;
			case 8:
			    *((char *) thepoint) += theoffset;
			    break;
			default:
			    break;
			}
			ndx++;
		    }
		} else {
		    if (Via(curref)->isA5REL) {
			theoffset = Via(Via(curref)->WhichModule)->A5offset;
			ndx = 0;
			while (ndx < Via(curref)->RefCount) {
			    char                           *thepoint;
			    thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 1 at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
			    switch (Via(curref)->patchsize) {
			    case 32:
				*((long *) thepoint) += theoffset;
				break;
			    case 16:
				*((short *) thepoint) += theoffset;
				break;
			    case 8:
				*((char *) thepoint) += theoffset;
				break;
			    default:
				break;
			    }
			    ndx++;
			}
		    } else {
			/* This case requires instruction editing */
			SegmentVia_t                    thisseg;
			SegmentVia_t                    thatseg;
			thisseg = GetSeg(Via(curmod)->segment, Via(curmod)->file);
			thatseg = GetSeg(Via(Via(curref)->WhichModule)->segment, Via(Via(curref)->WhichModule)->file);
			if (thisseg != thatseg) {
			    theoffset = Via(Via(curref)->WhichModule)->A5offset;
			    ndx = 0;
			    while (ndx < Via(curref)->RefCount) {
				char                           *thepoint;
				 char                   j;
				thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
				j = *(thepoint - 1);	// preceding byte
				j = j & (128 + 64);	// strip out low 6 bits
				j = j | 0x2d;		// set to A5
				*(thepoint - 1) = j;
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 1 (nonA5) at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
				switch (Via(curref)->patchsize) {
				case 32:
				    *((long *) thepoint) += theoffset;
				    break;
				case 16:
				    *((short *) thepoint) += theoffset;
				    break;
				case 8:
				    *((char *) thepoint) += theoffset;
				    break;
				default:
				    break;
				}
				ndx++;
			    }
			} else {
			    ndx = 0;
			    while (ndx < Via(curref)->RefCount) {
				char                           *thepoint;
				 char                   j;
				thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
				theoffset = Via(Via(curref)->WhichModule)->segmentoffset -
				    (Via(curmod)->segmentoffset + Via(Via(curref)->Offsets)[ndx]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
				j = *(thepoint - 1);	// previous byte
				j = j & (128 + 64);	// strip out low 6 bits
				j = j | 0x3a;		// set to PC rel
				*(thepoint - 1) = j;
#ifdef HDEBUG
			        fprintf(dumpFile,"Patchkind 1 (nonA5) at %x size %d value %x\n",
			        	Via(Via(curref)->Offsets)[ndx],
			        	Via(curref)->patchsize,
			        	theoffset);
#endif
				switch (Via(curref)->patchsize) {
				case 32:
				    *((long *) thepoint) += theoffset;
				    break;
				case 16:
				    *((short *) thepoint) += theoffset;
				    break;
				case 8:
				    *((char *) thepoint) += theoffset;
				    break;
				default:
				    break;
				}
				ndx++;
			    }
			}
		    }
		}
		break;
	    case 2:
		if (Via(curref)->isCOMPUTED) {
#ifdef HDEBUG
		    fprintf(dumpFile,"Next ref computed\n");
#endif
		    theoffset = Via(Via(curref)->WhichModule)->A5offset - Via(Via(curref)->OtherModule)->A5offset;
		    ndx = 0;
		    while (ndx < Via(curref)->RefCount) {
			char                           *thepoint;
			thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 2 at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
			switch (Via(curref)->patchsize) {
			case 32:
			    *((long *) thepoint) += theoffset;
			    break;
			case 16:
			    *((short *) thepoint) += theoffset;
			    break;
			case 8:
			    *((char *) thepoint) += theoffset;
			    break;
			default:
			    break;
			}
			ndx++;
		    }
		} else {
		    if (!Via(curref)->isA5REL)
			LinkerError("# Code to data refs must be a5 rel");
		    theoffset = Via(Via(curref)->WhichModule)->A5offset;
		    ndx = 0;
		    while (ndx < Via(curref)->RefCount) {
			char                           *thepoint;
			thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 2 at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
			switch (Via(curref)->patchsize) {
			case 32:
			    *((long *) thepoint) += theoffset;
			    break;
			case 16:
			    *((short *) thepoint) += theoffset;
			    break;
			case 8:
			    *((char *) thepoint) += theoffset;
			    break;
			default:
			    break;
			}
			ndx++;
		    }
		}
		break;
	    case 3:
	    case 4:
		theoffset = Via(Via(curref)->WhichModule)->A5offset;
		if (Via(curref)->isCOMPUTED)
		    theoffset -= Via(Via(curref)->OtherModule)->A5offset;
		ndx = 0;
		while (ndx < Via(curref)->RefCount) {
		    char                           *thepoint;
		    thepoint = &(Via(Via(curmod)->Bytes)[Via(Via(curref)->Offsets)[ndx]]);
#ifdef HDEBUG
			if ((Via(Via(curref)->Offsets)[ndx]) > Via(curmod)->modulesize) {
			    char                            tmp[128];
			    sprintf(tmp, "AdjustOneRef: offset out of range %d (size %d)",
			      Via(Via(curref)->Offsets)[ndx],
			      Via(curmod)->modulesize);
			    c2pstr(tmp);
			    DebugStr(tmp);
			}
#endif
#ifdef HDEBUG
		        fprintf(dumpFile,"Patchkind 3/4 at %x size %d value %x\n",
		        	Via(Via(curref)->Offsets)[ndx],
		        	Via(curref)->patchsize,
		        	theoffset);
#endif
		    switch (Via(curref)->patchsize) {
		    case 32:
			*((long *) thepoint) += theoffset;
			break;
		    case 16:
			*((short *) thepoint) += theoffset;
			break;
		    case 8:
			*((char *) thepoint) += theoffset;
			break;
		    default:
			break;
		    }
		    ndx++;
		}
		break;
	    default:
		assert(0);
		break;
	    }
	    curref = Via(curref)->next;
	}
    }
    #ifdef OLDMEM
    HUnlock((Handle) Via(curmod)->Bytes);
    #endif
}

void
AdjustAllReferences(LinkerFileVia_t files)
{
    LinkerFileVia_t                 curfile = files;
    ModuleVia_t                     curmod;
    ReferenceVia_t                  curref;
    char buf[256];

    while (curfile) {
	curmod = Via(curfile)->ModuleList;
	while (curmod) {
#ifdef HDEBUG
	    if (curmod->ID == 529) {
		    sprintf(buf,"Dump of module %d in %s",curmod->ID,curfile->name);
		    hex_dump(dumpFile,buf,curmod->Bytes,curmod->modulesize);
	    }
#endif
	    AdjustOneRef(curmod);
#ifdef HDEBUG
	    if (curmod->ID == 529) {
		    sprintf(buf,"SecondDump of module %d in %s",curmod->ID,curfile->name);
		    hex_dump(dumpFile,buf,curmod->Bytes,curmod->modulesize);
	    }
#endif
	    curmod = Via(curmod)->next;
	}
	curfile = Via(curfile)->next;
    }
}

#ifndef OLDMEM
void
AddIceResource(char *mem,long sz,OSType resType,short theID,char *name)
{
	Handle h;
	h = NewHandleClear(sz);
	memcpy(*h,mem,sz);
	AddResource(h,resType,theID,name);
}

#endif

int
FlushLink(char *name, short volrefnum, long dirID, LinkerFileVia_t files)
/* The argument is the name of the destination file */
{
    struct FInfo                    ftype;
    SegmentVia_t                    curseg;
    int                             thef;
    OSErr                           bad;
    char                            segname[256];
    int                             datainit = 0;
    Str255                          pname;
    if (!MAINmodule) {
		LinkerError("No main entry point !");
		return 0;
    }
    if (!LinkErrorCount) {
		theDATA.Image = ImageDATA();
		theJT.Image = ImageJT();
		theJT.ID = 0;
		theJT.name = NULL;
		theJT.CurrentSize = 0;	/* unused field for this struct */
		theJT.ModuleList = NULL;
		theJT.next = NULL;
    }
    if (!LinkErrorCount) {
	strcpy((char *) pname, name);
	c2pstr(pname);
	bad = HDelete(volrefnum, dirID, pname);
	HCreateResFile(volrefnum, dirID, pname);
	thef = HOpenResFile(volrefnum, dirID, pname, 2);
	if (thef == -1) {
	    LinkerError("Couldn't open app file for writing");
	    return 0;
	}
	curseg = SegmentList;
	while (curseg) {
	    Via(curseg)->Image = ImageCODE(curseg);
	    #ifdef OLDMEM
	    HLock((Handle) Via(curseg)->name);
	    #endif
	    if (!strcmp(Via(Via(curseg)->name), "%A5Init")) {
		Via(curseg)->Image = ConcatImage(Via(curseg)->Image, theDATA.Image, files);
		datainit = 1;
	    }
	    strcpy(segname, Via(Via(curseg)->name));
	    c2pstr(segname);
	    #ifdef OLDMEM
	    HUnlock((Handle) Via(curseg)->name);
	    AddResource((Handle) Via(curseg)->Image.mem, MakeOSType("CODE"), Via(curseg)->ID, segname);
	    #endif
	    AddIceResource(Via(curseg)->Image.mem,Via(curseg)->Image.size,MakeOSType("CODE"), Via(curseg)->ID, segname);
	    curseg = Via(curseg)->next;
	}
	#ifdef OLDMEM
	AddResource((Handle) theJT.Image.mem, MakeOSType("CODE"), 0, NULL);
	#endif
	AddIceResource(theJT.Image.mem,theJT.Image.size, MakeOSType("CODE"), 0, NULL);
	HGetFInfo(volrefnum, dirID, pname, &ftype);
	ftype.fdType = 'APPL';
	ftype.fdCreator = gProject->itsSignature;
	HSetFInfo(volrefnum, dirID, pname, &ftype);
	/* Now we add a SIZE resource */
	{
		Handle siz;
		long *lptr;
		siz = NewHandle(10);
		*((short *) *siz) = gProject->itsSizeFlags;
		*((long *) ((*((char **) siz))+2)) = gProject->itsPartition;
		*((long *) ((*((char **) siz))+6)) = gProject->itsPartition;
	    AddResource(siz, 'SIZE', -1, gProject->StdAppName);
	}
	CloseResFile(thef);
	if (!datainit) {
	    LinkerError("No data initialization code present !");
	    return 0;
	}
    } else {
    	LinkerError("No application generated");
    	return 0;
	}
    return 1;
}

void
KillRefs(ReferenceVia_t head)
{
    if (head) {
	if (Via(head)->next)
	    KillRefs(Via(head)->next);
	if (Via(head)->Offsets)
	    Efree(Via(head)->Offsets);
	Efree(head);
    }
}

void
KillModules(ModuleVia_t head)
{
    if (head) {
	if (Via(head)->next)
	    KillModules(Via(head)->next);
	KillModules(Via(head)->Entries);
	KillRefs(Via(head)->ReferenceList);
	if (Via(head)->Bytes)
	    Efree(Via(head)->Bytes);
	Efree(head);
    }
}

void
KillDicts(DictionaryVia_t head)
{
    if (head) {
	if (Via(head)->next)
	    KillDicts(Via(head)->next);
	Efree(head);
    }
}


void
KillModPacks(ModulePackageVia_t head)
{
    if (head) {
	if (Via(head)->next)
	    KillModPacks(Via(head)->next);
	Efree(head);
    }
}

void
KillImage(Block_t im)
{
    Efree(im.mem);
}

void
KillSegments(SegmentVia_t head)
{
    if (head) {
	if (Via(head)->next)
	    KillSegments(Via(head)->next);
	KillModPacks(Via(head)->ModuleList);
    }
}

void
KillLinkFiles(LinkerFileVia_t head)
{
    if (head) {
	if (Via(head)->next) {
	    KillLinkFiles(Via(head)->next);
	}
	KillModules(Via(head)->ModuleList);
	KillDicts(Via(head)->DictionaryList);
	Efree(head);
    }
}

