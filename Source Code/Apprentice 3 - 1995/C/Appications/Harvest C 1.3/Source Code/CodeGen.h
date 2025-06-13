/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file defines the interface for code generation.
 * 
 */

#ifndef CodeGen_INTERFACE
#define CodeGen_INTERFACE

typedef struct LocAM_S          LocAM_t;
typedef LocAM_t P__H           *LocAMVia_t;
typedef struct Inst_S           Inst_t;
typedef Inst_t P__H            *InstVia_t;
typedef struct InstList_S       InstList_t;
typedef InstList_t P__H        *InstListVia_t;
typedef struct SpillSlot_S      SpillSlot_t;
typedef SpillSlot_t P__H       *SpillSlotVia_t;
typedef struct RegisterRecord_S RegisterRecord_t;

#include "SymTable.h"
#include "ParseTree.h"
#include "Assem.h"

#define FOADD			 		0x0000
#define FOSUB			 		0x0002
#define FOMUL			 		0x0004
#define FODIV			 		0x0006
#define FOCMP			 		0x0008
#define FOCPX			 		0x000A
#define FOREM			 		0x000C
#define FOZ2X			 		0x000E
#define FOX2Z			 		0x0010
#define FOSQRT			 		0x0012
#define FORTI			 		0x0014
#define FOTTI			 		0x0016
#define FOSCALB 		 		0x0018
#define FOLOGB			 		0x001A
#define FOCLASS 		 		0x001C

#define FOSETENV		 		0x0001
#define FOGETENV		 		0x0003
#define FOSETHV 		 		0x0005
#define FOGETHV 		 		0x0007
#define FOD2B			 		0x0009
#define FOB2D			 		0x000B
#define FONEG			 		0x000D
#define FOABS			 		0x000F
#define FOCPYSGN		 		0x0011
#define FONEXT			 		0x0013
#define FOSETXCP		 		0x0015
#define FOPROCENTRY 	 		0x0017
#define FOPROCEXIT		 		0x0019
#define FOTESTXCP		 		0x001B

#define FFEXT			 		0x0000
#define FFDBL			 		0x0800
#define FFSGL			 		0x1000
#define FFINT			 		0x2000
#define FFLNG			 		0x2800
#define FFCOMP			 		0x3000

#define FCEXT			 		0x0000
#define FCDBL			 		0x4000
#define FCSGL			 		0x8000

/* Data structures for 68000 code generation. */

/* Addressing modes */

enum Size68 {
    M68sz_extended = -3,
    M68sz_double = -2,
    M68sz_single = -1,
    M68sz_byte = 1,
    M68sz_word = 2,
    M68sz_none = 3,		/* There is no data type with a size of 3 */
    M68sz_long = 4
    /*
     * All sizes greater than 4 are structs/unions that need special
     * handling.  Only the sizes given here can be generated directly as
     * assembler, and the negative ones only work with 68881.
     */
};

#define NOREG68 100

enum Dictation68 {
    M68dict_DReg = 1,
    M68dict_AReg
};

enum AddressingMode68 {
    M68am_DReg = 1,
    M68am_ARegDirect,
    M68am_ARegIndirect,
    M68am_ARegPostInc,
    M68am_ARegPreDec,
    M68am_ARegDisplace,
    M68am_ARegDispIndx,
    /* Above 7 are register modes */
    M68am_AbsShort,
    M68am_AbsLong,
    M68am_PCDisplace,
    M68am_PCLabelDisplace,
    M68am_PCDispIndx,
    /* Above 4 are absolute modes, mod=111 */
    M68am_Immediate,
    M68am_MultRegMove,
    M68am_SR,
    M68am_CCR,
    M68am_USP,
    M68am_Label,
    M68am_FReg,
    M68am_FSANEtemp,
    M68am_ARegDisplaceFIELD,
    M68am_ARegLabelDisplace,
    M68am_WhatModeIsThis,
    M68am_LargeGlobal,
    M68am_OtherFormat,
    M68am_CIdentifier,
    M68am_LabelOffset
};

enum RegisterState68 {
    M68rs_Available = 1,
    M68rs_Used
};

enum OperandState68 {
    M68os_valid = 1,
    M68os_spilled
};

enum SpillState68 {
    M68ss_free = 1,
    M68ss_used
};

struct Inst_S {
    Opcode_t                        OP;
    LocAM_t P__H                   *left;
    LocAM_t P__H                   *right;
    enum Size68                     SZ;
    struct Inst_S P__H             *prev;
    struct Inst_S P__H             *next;
    long                            Address;
    unsigned char                   Bytes[16];	/* QQQQ What should 16 be ? */
    short                           InstSize;
};

struct InstList_S {
    InstVia_t                       head;
    InstVia_t                       tail;
    int                             count;
    LabSYMVia_t                     PendingLabel;
};

struct SpillSlot_S {
    LocAM_t P__H                   *SlotLoc;
    short                           local;
    short                           SlotSize;
    enum SpillState68               status;
    struct SpillSlot_S P__H        *next;
};

struct RegisterRecord_S {
    enum RegisterState68            status;
    short                           waiting;
    short                           count;
    LocAM_t P__H                   *holder;
};

enum AddressingMode68
                                GetLocAM(LocAMVia_t loc);

enum Size68
                                GetLocSZ(LocAMVia_t loc);

enum OperandState68
                                GetLocStatus(LocAMVia_t loc);

SpillSlotVia_t
GetLocSlot(LocAMVia_t loc);

char
                                GetLocAReg(LocAMVia_t loc);

char
                                GetLocDReg(LocAMVia_t loc);

char
                                GetLocFReg(LocAMVia_t loc);

LabSYMVia_t
GetLocLabel(LocAMVia_t loc);

unsigned long
                                GetLocConstant(LocAMVia_t loc);

int
                                LocIsFloat(LocAMVia_t loc);

void
                                SetLocAM(LocAMVia_t loc, enum AddressingMode68 AM);

void
                                SetLocSZ(LocAMVia_t loc, enum Size68 SZ);

void
                                SetLocAReg(LocAMVia_t loc, char a);

struct TwoShorts
                                GetLocFieldBits(LocAMVia_t loc);

void
                                SetLocStatus(LocAMVia_t loc, enum OperandState68 s);

void
                                SetLocSlot(LocAMVia_t loc, SpillSlotVia_t s);

void
                                SetLocConstant(LocAMVia_t loc, unsigned long c);

void
                                SetLocIsFloat(LocAMVia_t loc, int val);

void
                                KillSomeLocations(void);

void
                                MakeLocGlobal(LocAMVia_t loc);

#endif
