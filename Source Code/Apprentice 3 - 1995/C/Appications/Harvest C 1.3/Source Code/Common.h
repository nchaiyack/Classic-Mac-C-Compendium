/*
 * Harvest C
 * 
 * Copyright 1991 Eric W. Sink   All rights reserved.
 * 
 * This file contains all the data structure definitions for Harvest C.
 * Generally each structure is declared, and then its type is assigned to a
 * typedef.
 * 
 * 
 */

#ifndef Common_INTERFACE
#define Common_INTERFACE

#include "conditcomp.h"

#define MAXINITDATA 32770
#define MAXARRAYDIMS 25
#define MAXIDLENGTH 1024
#define MAXERRORMSG 512
#define MAXNESTIFDEF 128
#define MAXMACROLENGTH 4096
#define MAXEXTRA 128
#define MAXNESTBLOCKS 1024
#define MAXTOKENPUTBACK 16
#define KEYWORDTABLESIZE 127
#define MAXSTRINGLITERAL 1024
#define MAXCONTENTS 50000
#define MAXUNSIGNEDCHAR 256
#define MAXELIFEXPR 1024
#define NUMOFTEMPDATAREGS 3
#define NUMOFTEMPADDRREGS 2
#define NUMOFTEMPFLOATREGS 4
#define MAXTEMPDATAREG (NUMOFTEMPDATAREGS - 1)
#define MAXTEMPFLOATREG (NUMOFTEMPFLOATREGS - 1)
#define MAXTEMPADDRREG (NUMOFTEMPADDRREGS - 1)
#define MAXFILENAME 128

#define BITFIELDUNIT 32

typedef void                   *MemReq_t;

#endif

