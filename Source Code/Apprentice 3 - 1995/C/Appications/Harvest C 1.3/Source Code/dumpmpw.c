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
 * This file contains routines for dumping 68k assembly code records as MPW
 * Object records.
 * 
 * 
 * 
 */


#include "conditcomp.h"
#include <stdio.h>
#include <string.h>
#include "structs.h"

#pragma segment DumpOMF


/* While creating Contents records, the results are built here. */
static EString_t                DATAContentsBuffer = NULL;
static long                     DATAContentsIndex;
static EString_t                CODEContentsBuffer = NULL;
static long                     CODEContentsIndex;
static int                      CurMPWSegNum;

/* MPW Record routines */

static short                    MPWSeg;
static unsigned short           NextMPWID = 1;
MPWListVia_t                    GlobalRecords;
MPWDictListVia_t                OBJNameList;

MPWDictListVia_t
RawMPWDictList(void)
{
    MPWDictListVia_t                raw;
    raw = Ealloc(sizeof(MPWDictList_t));
    Via(raw)->head = NULL;
    Via(raw)->tail = NULL;
    Via(raw)->count = 0;
    return raw;
}

MPWDictVia_t
RawMPWDict(char *name)
{
    MPWDictVia_t                    raw;
    raw = Ealloc(sizeof(MPWDict_t) + strlen(name));
    Via(raw)->DictID = 0;
    Via(raw)->isEXTERNAL = 0;
    Via(raw)->next = NULL;
    strcpy(Via(raw)->name, name);
    return raw;
}

MPWListVia_t
RawMPWList(void)
{
    MPWListVia_t                    raw;
    raw = Ealloc(sizeof(MPWList_t));
    Via(raw)->head = NULL;
    Via(raw)->tail = NULL;
    Via(raw)->count = 0;
    return raw;
}

MPWRecordVia_t
RawMPWRecord(void)
{
    MPWRecordVia_t                  raw;
    raw = Ealloc(sizeof(MPWRecord_t));
    Via(raw)->RecordType = 0;
    Via(raw)->theRecord = NULL;
    Via(raw)->next = NULL;
    Via(raw)->recsize = 0;
    return raw;
}

void
KillMPWRecord(MPWRecordVia_t rec)
{
    if (rec) {
	Efree(Via(rec)->theRecord);
	Efree(rec);
    }
}

void
KillRecordsList(MPWListVia_t reclist)
{
    MPWRecordVia_t                  cur;
    MPWRecordVia_t                  nxt;
    if (reclist) {
	cur = Via(reclist)->head;
	while (cur) {
	    nxt = Via(cur)->next;
	    KillMPWRecord(cur);
	    cur = nxt;
	}
	Via(reclist)->head = NULL;
	Via(reclist)->tail = NULL;
	Via(reclist)->count = 0;
    }
}

void
KillMPWDict(MPWDictVia_t rec)
{
    if (rec) {
	Efree(rec);
    }
}

void
KillDictList(MPWDictListVia_t Dicts)
{
    MPWDictVia_t                    cur;
    MPWDictVia_t                    nxt;
    if (Dicts) {
	cur = Via(Dicts)->head;
	while (cur) {
	    nxt = Via(cur)->next;
	    KillMPWDict(cur);
	    cur = nxt;
	}
	Efree(Dicts);
    }
}

MPWDictVia_t
FindDict(char *name, MPWDictListVia_t Dicts)
{
    MPWDictVia_t                    cur;
    cur = Via(Dicts)->head;
    while (cur) {
	if (!strcmp(name, (Via(cur)->name)))
	    return cur;
	cur = Via(cur)->next;
    }
    return NULL;
}

void
DoWord(unsigned char *buf, long x)
{
    buf[0] = x >> 8;
    buf[1] = x;
}

void
DoLong(unsigned char *buf, long x)
{
    buf[0] = x >> 24;
    buf[1] = x >> 16;
    buf[2] = x >> 8;
    buf[3] = x;
}

void
AddMPWRec(MPWRecordVia_t rec, MPWListVia_t Records)
{
    if (Pass == 1) {
	if (rec)
	    KillMPWRecord(rec);
	return;
    }
    if (!rec)
	return;
    if (!Records)
	return;
    Via(rec)->next = NULL;
    if (Via(Records)->head) {
	Via(Records)->count++;
	Via(Via(Records)->tail)->next = rec;
	Via(Records)->tail = rec;
    } else {
	Via(Records)->head = rec;
	Via(Records)->tail = rec;
	Via(Records)->count = 1;
    }
}

#define RecType(x) { Via(rec)->RecordType =x; Via(Via(rec)->theRecord)[0] = x; }
#define RecSize(x) {Via(rec)->recsize = x; Via(rec)->theRecord = Ealloc(x); }

void
MPW_Comment(char *comment, MPWListVia_t Records)
{
    int                             len;
    MPWRecordVia_t                  rec;
    int                             ndx;

    rec = RawMPWRecord();
    len = 4 + strlen(comment) + 1;
    RecSize(len);
    RecType(MPWRec_Comment);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = 0;
    DoWord(&(Via(Via(rec)->theRecord)[2]), len);
    ndx = 4;
    while (*comment) {
	Via(Via(rec)->theRecord)[ndx++] = *comment;
	comment++;
    }
    Via(Via(rec)->theRecord)[ndx] = 0;
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

void
MPW_Last(MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    rec = RawMPWRecord();
    RecSize(2);
    RecType(MPWRec_Last);

    Via(Via(rec)->theRecord)[1] = 0;
    AddMPWRec(rec, Records);
}

void
MPW_First(unsigned char FlagsByte, unsigned short Version, MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    rec = RawMPWRecord();
    RecSize(4);
    RecType(MPWRec_First);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoWord(&(Via(Via(rec)->theRecord)[2]), Version);
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

void
AddMPWDict(char *name, unsigned short ID, int isdefined, MPWDictListVia_t Dicts)
{
    MPWDictVia_t                    rec;
    if (Pass == 1)
	return;
    rec = RawMPWDict(name);
    Via(rec)->DictID = ID;
    Via(rec)->isEXTERNAL = isdefined;
    if (!Dicts)
	return;
    Via(rec)->next = NULL;
    if (Via(Dicts)->head) {
	Via(Dicts)->count++;
	Via(Via(Dicts)->tail)->next = rec;
	Via(Dicts)->tail = rec;
    } else {
	Via(Dicts)->head = rec;
	Via(Dicts)->tail = rec;
	Via(Dicts)->count = 1;
    }
}

void
MPW_DoDictionary(unsigned char FlagsByte, char *name, int Defined,
		 unsigned short ID,
		 MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    int                             len;
    int                             ndx;
    if (FindDict(name, OBJNameList))
	return;
    rec = RawMPWRecord();
    len = 7 + strlen((name));
    RecSize(len);
    RecType(MPWRec_Dictionary);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoWord(&(Via(Via(rec)->theRecord)[2]), Via(rec)->recsize);
    DoWord(&(Via(Via(rec)->theRecord)[4]), ID);
    Via(Via(rec)->theRecord)[6] = strlen((name));
    ndx = 0;
    while (ndx < strlen((name))) {
	Via(Via(rec)->theRecord)[7 + ndx] = (name)[ndx];
	ndx++;
    }
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
    AddMPWDict(name, NextMPWID - 1, Defined, OBJNameList);
}

void
MPW_Dictionary(unsigned char FlagsByte, char *name, int Defined,
	       MPWListVia_t Records)
{
    /*
     * Generate a dictionary record, AND generate a dict entry for
     * OBJNameList
     */
    MPW_DoDictionary(FlagsByte, name, Defined, NextMPWID++, Records);
}

void
MPW_Module(unsigned char FlagsByte, unsigned short ModID,
	   unsigned short ModSize, MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    rec = RawMPWRecord();
    RecSize(6);
    RecType(MPWRec_Module);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoWord(&(Via(Via(rec)->theRecord)[2]), ModID);
    DoWord(&(Via(Via(rec)->theRecord)[4]), ModSize);
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

void
MPW_Size(unsigned char FlagsByte, long sizevalue, MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    rec = RawMPWRecord();
    RecSize(6);
    RecType(MPWRec_Size);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoLong(&(Via(Via(rec)->theRecord)[2]), sizevalue);
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

void
MPW_Contents(unsigned char FlagsByte, EString_t Buffer, long
	     BufferSize, MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    int                             ndx, ndx2;
    rec = RawMPWRecord();
    RecSize(4 + BufferSize);
    RecType(MPWRec_Contents);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoWord(&(Via(Via(rec)->theRecord)[2]), Via(rec)->recsize);
    ndx = 0;
    ndx2 = 4;
    while (ndx < BufferSize) {
	Via(Via(rec)->theRecord)[ndx2++] = Via(Buffer)[ndx++];
    }
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

void
MPW_Reference(unsigned char FlagsByte, unsigned short RefID, long
	      offset, MPWListVia_t Records)
{
    MPWRecordVia_t                  rec;
    rec = RawMPWRecord();
    RecSize(8);
    RecType(MPWRec_Reference);
#ifdef OLDMEM
    HLock((Handle) rec);
#endif

    Via(Via(rec)->theRecord)[1] = FlagsByte;
    DoWord(&(Via(Via(rec)->theRecord)[2]), Via(rec)->recsize);
    DoWord(&(Via(Via(rec)->theRecord)[4]), RefID);
    DoWord(&(Via(Via(rec)->theRecord)[6]), offset);
#ifdef OLDMEM
    HUnlock((Handle) rec);
#endif
    AddMPWRec(rec, Records);
}

unsigned short
NameID(MPWDictListVia_t Dicts, char *name, unsigned char FlagsByte, int
       isdefined, MPWListVia_t Records)
{
    /*
     * If the name does not exist, generate a Dictionary record for it
     */
    MPWDictVia_t                    dict;
    dict = FindDict(name, Dicts);
    if (!dict) {
	MPW_Dictionary(FlagsByte, name, isdefined, Records);
	dict = FindDict(name, Dicts);
    }
    if (dict)
	return Via(dict)->DictID;
    else
	return 0;
}

unsigned char
GetModuleFlags(char *name)
{
    MPWDictVia_t                    dict;
    dict = FindDict(name, OBJNameList);
    if (dict) {
	if (Via(dict)->isEXTERNAL)
	    return 8;
	else
	    return 0;
    } else {
	return 0;
    }
}

void
WordAlignContents(void)
{
    if (MPWSeg) {		/* data */
		if (DATAContentsIndex % 2)
		    if (Pass == 2)
			DATAContentsIndex++;
		Pc++;
    } else {
		if (CODEContentsIndex % 2)
		    if (Pass == 2)
			CODEContentsIndex++;
		Pc++;
    }
}

void
AddContentsByte(unsigned char bt)
{
    if (Pass == 1)
	return;
    if (MPWSeg) {		/* data */
	Via(DATAContentsBuffer)[DATAContentsIndex++] = bt;
    } else {
	Via(CODEContentsBuffer)[CODEContentsIndex++] = bt;
    }
}

void
AddContentsWord(unsigned short wd)
{
    AddContentsByte(hibyte(wd));
    AddContentsByte(lobyte(wd));
}

void
AddContentsLong(unsigned long wd)
{
    AddContentsWord(hiword(wd));
    AddContentsWord(loword(wd));
}

void
FlushCode(MPWListVia_t Records)
{
    if (CODEContentsIndex) {
	MPW_Contents(0, CODEContentsBuffer, CODEContentsIndex, Records);
	CODEContentsIndex = 0;
    }
}

void
FlushData(MPWListVia_t Records)
{
    if (DATAContentsIndex) {
	MPW_Contents(1, DATAContentsBuffer, DATAContentsIndex, Records);
	DATAContentsIndex = 0;
    }
}

void
CodeModule(unsigned char FlagsByte, unsigned short ModID,
	   unsigned short ModSize, MPWListVia_t Records)
{
    FlushCode(Records);
    MPW_Module(FlagsByte, ModID, ModSize, Records);
    MPWSeg = 0;
    Pc = 0;
}

void
DataModule(unsigned char FlagsByte, unsigned short ModID,
	   unsigned short ModSize, MPWListVia_t Records)
{
    FlushData(Records);
    MPW_Module(FlagsByte | 1, ModID, ModSize, Records);
    MPWSeg = 1;
}

void
AddtoContents(InstVia_t inst)
{
    int                             ndx;
    if (Via(inst)->OP == M68op_DELETED) return;
    ndx = 0;
    if (Via(inst)->InstSize < 0)
	return;
    while (ndx < Via(inst)->InstSize) {
	AddContentsByte(Via(inst)->Bytes[ndx]);
	ndx++;
    }
}

void
DumpCodeList(InstListVia_t Codes, MPWListVia_t Records)
{
    InstVia_t                       cur;
    unsigned short                  RecordID;
    char                            LastCodeSymbol[64];
    int                             ndx;
    unsigned char                   FlagsByte = 0;
    MPWSeg = 0;
    CODEContentsBuffer = Ealloc(MAXCONTENTS);
    CODEContentsIndex = 0;
    DATAContentsBuffer = Ealloc(MAXCONTENTS);
    DATAContentsIndex = 0;
    if (!(Codes && Records)) {
	CodegenError("Bad instruction list");
    }
    /*
     * Process every instruction in Codes, generating MPW OBJ records as
     * necessary, inserting them into Records.
     */
    Pass = 1;
    while (Pass <= 2) {
	Pc = 0;
	cur = Via(Codes)->head;
	while (cur) {
	    switch (Via(cur)->OP) {
	    case M68op_STARTCOMMENT:
	    case M68op_TEXTCOMMENT:
	    case M68op_PLAINTEXTCOMMENT:
	    case M68op_COMMENT:
	    case M68op_CRCOMMENT:
		/* We currently do nothing with comments */
		break;
	    case M68op_MPWSEG:
		CurMPWSegNum = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 1, Records);
		break;
	    case M68op_CODELABEL:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		RecordID = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 1, Records);
		strcpy(LastCodeSymbol, Via(GetLocLabel(Via(cur)->left))->name);
		FlagsByte = GetModuleFlags(Via(GetLocLabel(Via(cur)->left))->name);
		CodeModule(FlagsByte, RecordID, CurMPWSegNum, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		break;
	    case M68op_STRINGLABEL:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		RecordID = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 1, 1, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		FlagsByte = 1;
		DataModule(FlagsByte, RecordID, 0, Records);
		break;
	    case M68op_DATALABEL:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		RecordID = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 1, Records);
		FlagsByte = GetModuleFlags(Via(GetLocLabel(Via(cur)->left))->name);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		DataModule(FlagsByte, RecordID, 0, Records);	/* 0 is size
								 * specification - it
								 * will be set later
								 * using a Size record
								 * generated from op_DS */
		break;
	    case M68op_LCOMM:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		RecordID = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 0, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		DataModule(1, RecordID, GetLocConstant(Via(cur)->right), Records);
		break;
	    case M68op_COMM:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		RecordID = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 1, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		DataModule(9, RecordID, GetLocConstant(Via(cur)->right), Records);
		break;
	    case M68op_XREF:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		MPW_Dictionary(FlagsByte, Via(GetLocLabel(Via(cur)->left))->name, 0, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		/*
		 * The 0 is for xref as opposed to Defined
		 */
		break;
	    case M68op_DEFSEG:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		MPW_Dictionary(FlagsByte, Via(GetLocLabel(Via(cur)->left))->name, 1, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		/*
		 * The 1 is for Defined as opposed to xref
		 */
		break;
	    case M68op_XDEF:
#ifdef OLDMEM
		HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
		MPW_Dictionary(FlagsByte, Via(GetLocLabel(Via(cur)->left))->name, 1, Records);
#ifdef OLDMEM
		HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
		/*
		 * The 1 is for Defined as opposed to xref
		 */
		break;
	    case M68op_BSEG:
	    case M68op_DSEG:
		MPWSeg = 1;
		break;
	    case M68op_CSEG:
		MPWSeg = 0;
		break;
	    case M68op_DS:
		MPW_Size(MPWSeg, GetLocConstant(Via(cur)->left), Records);
		break;
	    case M68op_DC:
		Via(cur)->Address = Pc;
		if (GetLocAM(Via(cur)->left) == M68am_AbsLong) {
		    switch (Via(cur)->SZ) {
		    case M68sz_byte:
			AddContentsByte(GetLocConstant(Via(cur)->left));
			Pc++;
			break;
		    case M68sz_word:
			AddContentsWord(GetLocConstant(Via(cur)->left));
			Pc += 2;
			break;
		    case M68sz_long:
			AddContentsLong(GetLocConstant(Via(cur)->left));
			Pc += 4;
			break;
		    default:
			Gen68Error("Bad DC size");
			break;
		    }
		} else {
		    int                             id;
		    /*
		     * We need to put a Reference record.  We need the ID of
		     * the module we are referencing.  We also need the
		     * offset, so we need the value of the correct
		     * ContentsIndex, and we need to decide what the flags
		     * will be.
		     */
		    switch (Via(cur)->SZ) {
		    case M68sz_byte:
			AddContentsByte(0);
			Pc++;
			break;
		    case M68sz_word:
#ifdef OLDMEM
			HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
			id = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 0, Records);
#ifdef OLDMEM
			HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
			MPW_Reference(128 + 16 + MPWSeg, id, MPWSeg ? CODEContentsIndex : DATAContentsIndex, Records);
			AddContentsWord(0);
			Pc += 2;
			break;
		    case M68sz_long:
#ifdef OLDMEM
			HLock((Handle) GetLocLabel(Via(cur)->left));
#endif
			id = NameID(OBJNameList, Via(GetLocLabel(Via(cur)->left))->name, 0, 0, Records);
#ifdef OLDMEM
			HUnlock((Handle) GetLocLabel(Via(cur)->left));
#endif
			MPW_Reference(128 + MPWSeg, id, MPWSeg ? CODEContentsIndex : DATAContentsIndex, Records);
			AddContentsLong(0);
			Pc += 4;
			break;
		    default:
			Gen68Error("Bad DC size");
			break;
		    }
		}
		break;
	    case M68op_MBG:
		/* Here we add MacsBug symbol information */
		AddContentsByte(0x80 + strlen(LastCodeSymbol));
		Pc++;
		ndx = 0;
		while (LastCodeSymbol[ndx]) {
		    AddContentsByte(LastCodeSymbol[ndx++]);
		    Pc++;
		}
		if (CODEContentsIndex % 2)
			AddContentsByte(0);	/* align even */
		AddContentsByte(0);	/* size of literals */
		AddContentsByte(0);
		Pc += 3;
		break;
	    case M68op_EVEN:
		WordAlignContents();
		break;
	    default:
		/*
		 * The default here is an instruction of some kind.  We pass
		 * this to another routine to handle all instructions that
		 * come in sequence, generating Contents and Reference
		 * records appropriately.  When that routine finds a
		 * directive, control comes back to this routine.
		 */
		process(cur);
		AddtoContents(cur);
		break;
	    }
	    if (cur)
		cur = Via(cur)->next;
	}
	Pass++;
    }
    FlushCode(Records);
    FlushData(Records);
    Efree(CODEContentsBuffer);
    Efree(DATAContentsBuffer);
}

OSType
MakeOSType(char *d)
{
    OSType                          result;
    result = d[0] << 24;
    result |= d[1] << 16;
    result |= d[2] << 8;
    result |= d[3];
    return result;
}

short 
InitOMF(InstListVia_t Codes, char *fname, short volrefnum, long dirID)
{
    MPWRecordVia_t                  cur;
    int                             totalwritten;
    OSErr                           bad;
    char                            OBJName[64];
    short                           filenum;
    strcpy(OBJName, fname);
    c2pstr(OBJName);
    OBJName[OBJName[0]] = 'o';
    totalwritten = 0;
    bad = HDelete(volrefnum, dirID, OBJName);
    if (bad != -43)
		UserFileError(bad);
    Pass = 0;
    if (NumErrors)
	return 0;
    GlobalRecords = RawMPWList();
    OBJNameList = RawMPWDictList();
    MPW_First(0, 2, GlobalRecords);	/* the 2 will be a 3 if SADE info is
					 * included */
    MPW_Comment("Created by Harvest C", GlobalRecords);
    bad = HCreate(volrefnum, dirID, OBJName, MakeOSType("Jn15"), MakeOSType("OBJ "));
    UserFileError(bad);
    if (!bad) {
	bad = HOpen(volrefnum, dirID, OBJName, 2, &filenum);
	UserFileError(bad);
    }
    return filenum;
}

void
FlushOMF(short filenum)
{
    MPWRecordVia_t                  cur;
    int                             totalwritten = 0;
    OSErr                           bad;
    if (GlobalRecords) {
	cur = Via(GlobalRecords)->head;
	while (cur) {
	    long                            count;
	    count = Via(cur)->recsize;
#ifdef OLDMEM
	    HLock((Handle) Via(cur)->theRecord);
#endif
	    bad = FSWrite(filenum, &count, (char *) Via(Via(cur)->theRecord));
#ifdef OLDMEM
	    HUnlock((Handle) Via(cur)->theRecord);
#endif
	    UserFileError(bad);
	    totalwritten += count;
	    if (totalwritten % 2) {
		char                            padbyte[2];
		long                            count2;
		totalwritten++;
		count2 = 1;
		padbyte[0] = 0;
		bad = FSWrite(filenum, &count2, padbyte);
		UserFileError(bad);
	    }
	    cur = Via(cur)->next;
	}
	KillRecordsList(GlobalRecords);
    }
}

void
FinishOMF(short filenum)
{
    MPWRecordVia_t                  cur;
    int                             totalwritten;
    OSErr                           bad;
    MPW_Last(GlobalRecords);

    FlushOMF(filenum);

    bad = FSClose(filenum);
    UserFileError(bad);

    Efree(GlobalRecords);
    KillDictList(OBJNameList);
}

void
GenerateOBJ(InstListVia_t Codes, char *fname, short volrefnum, long dirID)
{
    short                           filenum;
    filenum = InitOMF(Codes, fname, volrefnum, dirID);
    DumpCodeList(Codes, GlobalRecords);
    FlushOMF(filenum);
    FinishOMF(filenum);
}
