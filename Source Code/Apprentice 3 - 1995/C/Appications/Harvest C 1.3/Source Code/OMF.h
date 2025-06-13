/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for OMF records.
 * 
 */

#ifndef OMF_INTERFACE
#define OMF_INTERFACE

typedef struct MPWRecord_S      MPWRecord_t;
typedef MPWRecord_t P__H       *MPWRecordVia_t;
typedef struct MPWDict_S        MPWDict_t;
typedef MPWDict_t P__H         *MPWDictVia_t;
typedef struct MPWDictList_S    MPWDictList_t;
typedef MPWDictList_t P__H     *MPWDictListVia_t;
typedef struct MPWList_S        MPWList_t;
typedef MPWList_t P__H         *MPWListVia_t;

enum MPWRecords_E {
    MPWRec_Pad,
    MPWRec_First,
    MPWRec_Last,
    MPWRec_Comment,
    MPWRec_Dictionary,
    MPWRec_Module,
    MPWRec_Entry_Point,
    MPWRec_Size,
    MPWRec_Contents,
    MPWRec_Reference,
    MPWRec_Computed_Reference,
    MPWRec_Filename,
    MPWRec_SourceStatement,
    MPWRec_ModuleBegin,
    MPWRec_ModuleEnd,
    MPWRec_BlockBegin,
    MPWRec_BlockEnd,
    MPWRec_LocalIdentifier,
    MPWRec_LocalLabel,
    MPWRec_LocalType
};

struct MPWRecord_S {
    enum MPWRecords_E               RecordType;
    unsigned char P__H             *theRecord;
    short                           recsize;
    MPWRecordVia_t                  next;
};

struct MPWDict_S {
    unsigned short                  DictID;
    short                           isEXTERNAL;	/* This is a flag */
    MPWDictVia_t                    next;
    char                            name[1];
};

struct MPWDictList_S {
    MPWDictVia_t                    head;
    MPWDictVia_t                    tail;
    int                             count;
};

struct MPWList_S {
    MPWRecordVia_t                  head;
    MPWRecordVia_t                  tail;
    int                             count;
};

#endif
