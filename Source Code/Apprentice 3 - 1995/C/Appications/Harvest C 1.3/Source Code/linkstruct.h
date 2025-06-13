/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains structures for the Harvest C linker.
 * 
 * 
 */


#ifndef LinkStruct_INTERFACE
#define LinkStruct_INTERFACE

struct Block_S {
	EString_t						mem;
    unsigned long                   size;
};
typedef struct Block_S          Block_t;
typedef Block_t P__H           *BlockVia_t;

struct Reference_S {
    unsigned short                  ID, ID2;
    char                            isA5REL;
    char                            isCOMPUTED;
    char                            patchsize;
    char                            patchkind;	/* 0 is code-code, 1 is
						 * code-data 2 is data-code 3
						 * is data-data */
    char                            RefCount;
    long P__H                      *Offsets;
    struct Module_S P__H           *WhichModule;
    struct Module_S P__H           *OtherModule;
    struct Reference_S P__H        *next;
};
typedef struct Reference_S      Reference_t;
typedef Reference_t P__H       *ReferenceVia_t;

struct Dictionary_S {
    unsigned short                  ID;
    struct Dictionary_S P__H       *next;
    char                            name[1];
};
typedef struct Dictionary_S     Dictionary_t;
typedef Dictionary_t P__H      *DictionaryVia_t;

struct EntryPoint_S {
    char                            isMAIN;
    char                            isEXTERN;
    unsigned short                  ID;
    unsigned long                   offset;
    struct EntryPoint_S P__H       *next;
};
typedef struct EntryPoint_S     EntryPoint_t;
typedef EntryPoint_t P__H      *EntryPointVia_t;

struct Module_S {
	long sizeBytes;
    EString_t                       Bytes;
    Reference_t P__H               *ReferenceList;
    struct Module_S P__H           *Entries;
    unsigned long                   modulesize;
    unsigned short                  ID;
    char                            externref;
    char                            isACTIVE;
    char                            isMAIN;
    struct Module_S P__H           *isENTRY;
    char                            isEXTERN;
    char                            isCODE;
    int                             segment;
    unsigned long                   segmentoffset;
    unsigned long                   offset;
    struct LinkerFile_S P__H       *file;
    long                            A5offset;
    struct Module_S P__H           *next;
};
typedef struct Module_S         Module_t;
typedef Module_t P__H          *ModuleVia_t;

struct LinkerFile_S {
    Module_t P__H                  *ModuleList;
    Dictionary_t P__H              *DictionaryList;
    int                             version;
    char                            isNESTED;
    char                            name[64];
    struct LinkerFile_S P__H       *next;
};
typedef struct LinkerFile_S     LinkerFile_t;
typedef LinkerFile_t P__H      *LinkerFileVia_t;

struct ModulePackage_S {
    Module_t P__H                  *themod;
    struct ModulePackage_S P__H    *next;
};
typedef struct ModulePackage_S  ModulePackage_t;
typedef ModulePackage_t P__H   *ModulePackageVia_t;

struct DATAZone_S {
    Block_t                         Image;
    unsigned long                   CurrentSize;
    unsigned long                   count;
    ModulePackage_t P__H           *ModuleList;
};
typedef struct DATAZone_S       DATAZone_t;

struct Segment_S {
    unsigned long                   CurrentSize;
    ModulePackage_t P__H           *ModuleList;
    unsigned short                  ID;
    EString_t                       name;
    unsigned short                  firstJT;
    unsigned short                  countJT;
    struct Segment_S P__H          *next;
    Block_t                         Image;
};
typedef struct Segment_S        Segment_t;
typedef Segment_t P__H         *SegmentVia_t;

#endif
