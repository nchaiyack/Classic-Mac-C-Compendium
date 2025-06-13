/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for type records.
 * 
 */

#ifndef TypeRecord_INTERFACE
#define TypeRecord_INTERFACE

#include "Param.h"

typedef struct TypeRecord       TypeRecord_t;
typedef unsigned long           TypeRecordVia_t;

enum TypeRecordCode {
    TRC_char = 1,
    TRC_short,
    TRC_int,
    TRC_long,
    TRC_bitfield,
    TRC_float,
    TRC_double,
    TRC_longdouble,
    TRC_enum,
    TRC_void,
    TRC_array,
    TRC_ANSIfunction,
    TRC_ANSIELLIPSISfunction,
    TRC_OLDfunction,
    TRC_NOARGSfunction,
    TRC_pointer,
    TRC_struct,
    TRC_union,
    TRC_typedef,
    TRC_comp
};

enum StorageClassCode {
    SCC_auto = 1,
    SCC_register,
    SCC_static,
    SCC_const,
    SCC_volatile,
    SCC_enum,
    SCC_extern,
    SCC_typedef,
    SCC_normal,
    SCC_pascal
};

enum SignCode {
    SGN_signed = 1,
    SGN_unsigned,
    SGN_unknown
};

#include "CodeGen.h"
#include "SymTable.h"

void
                                InitTypes(void);

void
                                FreeTypeRecords(void);

TypeRecordVia_t
StripTypedef(TypeRecordVia_t);

int
                                SameType(TypeRecordVia_t, TypeRecordVia_t);

TypeRecordVia_t
isPointerType(TypeRecordVia_t);

int
                                isIncompleteType(TypeRecordVia_t);

int
                                isVoidType(TypeRecordVia_t);

int
                                isVoidPointerType(TypeRecordVia_t);

int
                                isIntegralType(TypeRecordVia_t);

int
                                isFloatingType(TypeRecordVia_t);

int
                                isExtendedFloatingType(TypeRecordVia_t);

int
                                isLongFloatingType(TypeRecordVia_t);

int
                                isShortFloatingType(TypeRecordVia_t);

int
                                isArithmeticType(TypeRecordVia_t);

int
                                isBooleanType(TypeRecordVia_t);

SymListVia_t
isStructUnionType(TypeRecordVia_t);

TypeRecordVia_t
isArrayType(TypeRecordVia_t);

int
                                isLongType(TypeRecordVia_t);

int
                                isSignedType(TypeRecordVia_t);

int
                                isUnsignedType(TypeRecordVia_t);

int
                                isBitFieldType(TypeRecordVia_t);

int
                                isEnumType(TypeRecordVia_t);

TypeRecordVia_t
GetFuncType(TypeRecordVia_t);

int
                                CountFuncArgs(TypeRecordVia_t);

TypeRecordVia_t
isFunctionType(TypeRecordVia_t);

SYMVia_t
isMemberOf(TypeRecordVia_t, char *);

int
                                StartLValue(TypeRecordVia_t);

TypeRecordVia_t
InheritTypeRec(TypeRecordVia_t);

TypeRecordVia_t
CopyTypeRecord(TypeRecordVia_t);

SymListVia_t
GetTPMembers(TypeRecordVia_t);

void
                                SetTPMembers(TypeRecordVia_t, SymListVia_t);

TypeRecordVia_t
GetTPBase(TypeRecordVia_t);

void
                                SetTPBase(TypeRecordVia_t, TypeRecordVia_t);

int
                                GetTPSize(TypeRecordVia_t);

int
                                GetTPKind(TypeRecordVia_t);

int
                                GetTPFlags(TypeRecordVia_t);

enum SignCode
                                GetTPSign(TypeRecordVia_t);

ParamRecVia_t
GetTPParam(TypeRecordVia_t);

void
                                SetTPSize(TypeRecordVia_t, int);

void
                                SetTPFlags(TypeRecordVia_t, int);

TypeRecordVia_t
BuildTypeRecord(TypeRecordVia_t, enum TypeRecordCode, enum SignCode);

TypeRecordVia_t
BuildTagTypeRecord(TypeRecordVia_t, enum TypeRecordCode, char *);

void
                                GetTPName(TypeRecordVia_t t, char *nm);

ParseTreeVia_t
GetTPTrap(TypeRecordVia_t t);

enum StorageClassCode
                                GetTPQual(TypeRecordVia_t t);

void
                                SetTPQual(TypeRecordVia_t t, enum StorageClassCode q);

void
                                SetTPParam(TypeRecordVia_t t, ParamRecVia_t p);

void
                                SetTPTrap(TypeRecordVia_t t, ParseTreeVia_t p);

void
                                SetTPSign(TypeRecordVia_t t, enum SignCode s);

void
                                SetTPKind(TypeRecordVia_t t, int k);

AbsStringID
GetTPNameID(TypeRecordVia_t t);

#endif
