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

/* Harvest C Copyright 1991 Eric W. Sink  All rights reserved. */
/* This file contains the data tables for the assembler */
/* It was generated automatically by perl */
#include <stdio.h>
#include "conditcomp.h"
#include "structs.h"

struct tmpl     template[] =
{
    {bu,
     {DN, DN}, RXRY, 0xC100, 0x0},	/* 0 = abcd */
    {bu,
     {PREDEC, PREDEC}, RXRY, 0xC108, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0600, 0x0},	/* 2 = add */
    {wlu,
     {ANYEA, DN}, EAREGS, 0xD000, 0x0},
    {B,
     {DATA, DN}, EAREGS, 0xD000, 0x0},
    {bwlu,
     {DN, ALTMEM}, REGEAS, 0xD100, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0xD0C0, 0x0},
    {L,
     {ANYEA, AN}, EAREG, 0xD1C0, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0xD0C0, 0x0},	/* 8 = adda */
    {L,
     {ANYEA, AN}, EAREG, 0xD1C0, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0600, 0x0},	/* 10 = addi */
    {bwlu,
     {IMMED, ALTER}, QUKEA, 0x5000, 0x0},	/* 11 = addq */
    {bwlu,
     {DN, DN}, RXRYS, 0xD100, 0x0},	/* 12 = addx */
    {bwlu,
     {PREDEC, PREDEC}, RXRYS, 0xD108, 0x0},
    {bwlu,
     {DATA, DN}, EAREGS, 0xC000, 0x0},	/* 14 = and */
    {bwlu,
     {DN, ALTMEM}, REGEAS, 0xC100, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0200, 0x0},
    {bu,
     {IMMED, CCR}, IMMB, 0x023C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x027C, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0200, 0x0},	/* 19 = andi */
    {bu,
     {IMMED, CCR}, IMMB, 0x023C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x027C, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE1C0, 0x0},/* 22 = asl */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE120, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE100, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE0C0, 0x0},/* 25 = asr */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE020, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE000, 0x0},
    {bwlu,
     {EXPR}, BCC, 0x6400, 0x0},	/* 28 = bcc */
    {bwlu,
     {EXPR}, BCC, 0x6500, 0x0},	/* 29 = bcs */
    {bwlu,
     {EXPR}, BCC, 0x6700, 0x0},	/* 30 = beq */
    {bwlu,
     {EXPR}, BCC, 0x6C00, 0x0},	/* 31 = bge */
    {bwlu,
     {EXPR}, BCC, 0x6E00, 0x0},	/* 32 = bgt */
    {bwlu,
     {EXPR}, BCC, 0x6200, 0x0},	/* 33 = bhi */
    {bwlu,
     {EXPR}, BCC, 0x6F00, 0x0},	/* 34 = ble */
    {bwlu,
     {EXPR}, BCC, 0x6300, 0x0},	/* 35 = bls */
    {bwlu,
     {EXPR}, BCC, 0x6D00, 0x0},	/* 36 = blt */
    {bwlu,
     {EXPR}, BCC, 0x6B00, 0x0},	/* 37 = bmi */
    {bwlu,
     {EXPR}, BCC, 0x6600, 0x0},	/* 38 = bne */
    {bwlu,
     {EXPR}, BCC, 0x6A00, 0x0},	/* 39 = bpl */
    {bwlu,
     {EXPR}, BCC, 0x6800, 0x0},	/* 40 = bvc */
    {bwlu,
     {EXPR}, BCC, 0x6900, 0x0},	/* 41 = bvs */
    {bwlu,
     {EXPR}, BCC, 0x6000, 0x0},	/* 42 = bra */
    {bwlu,
     {EXPR}, BCC, 0x6100, 0x0},	/* 43 = bsr */
    {lu,
     {DN, DN}, REGEA, 0x0140, 0x0},	/* 44 = bchg */
    {lu,
     {IMMED, DN}, BIT, 0x0840, 0x0},
    {bu,
     {DN, ALTMEM}, REGEA, 0x0140, 0x0},
    {bu,
     {IMMED, ALTMEM}, BIT, 0x0840, 0x0},
    {lu,
     {DN, DN}, REGEA, 0x0180, 0x0},	/* 48 = bclr */
    {lu,
     {IMMED, DN}, BIT, 0x0880, 0x0},
    {bu,
     {DN, ALTMEM}, REGEA, 0x0180, 0x0},
    {bu,
     {IMMED, ALTMEM}, BIT, 0x0880, 0x0},
    {lu,
     {DN, DN}, REGEA, 0x01C0, 0x0},	/* 52 = bset */
    {lu,
     {IMMED, DN}, BIT, 0x08C0, 0x0},
    {bu,
     {DN, ALTMEM}, REGEA, 0x01C0, 0x0},
    {bu,
     {IMMED, ALTMEM}, BIT, 0x08C0, 0x0},
    {lu,
     {DN, DN}, REGEA, 0x0100, 0x0},	/* 56 = btst */
    {lu,
     {IMMED, DN}, BIT, 0x0800, 0x0},
    {bu,
     {DN, DATA}, REGEA, 0x0100, 0x0},
    {bu,
     {IMMED, DATA}, BIT, 0x0800, 0x0},
    {U,
     {DATALT, FIELD}, BITFLD, 0xEAC0, 0x0},	/* 60 = bfchg */
    {U,
     {DATALT, FIELD}, BITFLD, 0xECC0, 0x0},	/* 61 = bfclr */
    {U,
     {DN, FIELD, DN}, BITFLD2, 0xEBC0, 0x0},	/* 62 = bfexts */
    {U,
     {CONTROL, FIELD, DN}, BITFLD2, 0xEBC0, 0x0},
    {U,
     {DN, FIELD, DN}, BITFLD2, 0xE9C0, 0x0},	/* 64 = bfextu */
    {U,
     {CONTROL, FIELD, DN}, BITFLD2, 0xE9C0, 0x0},
    {U,
     {DN, FIELD, DN}, BITFLD2, 0xEDC0, 0x0},	/* 66 = bfffo */
    {U,
     {CONTROL, FIELD, DN}, BITFLD2, 0xEDC0, 0x0},
    {U,
     {DN, DATALT, FIELD}, BITFLD2, 0xEFC0, 0x0},	/* 68 = bfins */
    {U,
     {DATALT, FIELD}, BITFLD, 0xEEC0, 0x0},	/* 69 = bfset */
    {U,
     {DN, FIELD}, BITFLD, 0xE8C0, 0x0},	/* 70 = bftst */
    {U,
     {CONTROL, FIELD}, BITFLD, 0xE8C0, 0x0},
    {U,
     {IMMED}, IMM3, 0x4848, 0x0},	/* 72 = bkpt */
    {U,
     {IMMED, CONTROL}, CALLM, 0x06C0, 0x0},	/* 73 = callm */
    {bwlu,
     {DN, DN, ALTMEM}, CAS, 0x08C0, 0x0},	/* 74 = cas */
    {wlu,
     {DPAIR, DPAIR, RPAIR}, CAS2, 0x08FC, 0x0},	/* 75 = cas2 */
    {wlu,
     {DATA, DN}, CHK, 0x4100, 0x0},	/* 76 = chk */
    {bwlu,
     {CONTROL, RN}, CHK2, 0x00C0, 0x0800},	/* 77 = chk2 */
    {bwlu,
     {DATALT}, EAS, 0x4200, 0x0},	/* 78 = clr */
    {bwlu,
     {IMMED, DATA}, IMMEAS, 0x0C00, 0x0},	/* 79 = cmp */
    {wlu,
     {ANYEA, DN}, EAREGS, 0xB000, 0x0},
    {B,
     {DATA, DN}, EAREGS, 0xB000, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0xB0C0, 0x0},
    {L,
     {ANYEA, AN}, EAREG, 0xB1C0, 0x0},
    {bwlu,
     {PSTINC, PSTINC}, RXRYS, 0xB108, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0xB0C0, 0x0},	/* 85 = cmpa */
    {L,
     {ANYEA, AN}, EAREG, 0xB1C0, 0x0},
    {bwlu,
     {IMMED, DATA}, IMMEAS, 0x0C00, 0x0},	/* 87 = cmpi */
    {bwlu,
     {PSTINC, PSTINC}, RXRYS, 0xB108, 0x0},	/* 88 = cmpm */
    {bwlu,
     {CONTROL, RN}, CHK2, 0x00C0, 0x0000},	/* 89 = cmp2 */
    {U,
     {DN, EXPR}, DBCC, 0x54C8, 0x0},	/* 90 = dbcc */
    {U,
     {DN, EXPR}, DBCC, 0x55C8, 0x0},	/* 91 = dbcs */
    {U,
     {DN, EXPR}, DBCC, 0x57C8, 0x0},	/* 92 = dbeq */
    {U,
     {DN, EXPR}, DBCC, 0x51C8, 0x0},	/* 93 = dbf */
    {U,
     {DN, EXPR}, DBCC, 0x5CC8, 0x0},	/* 94 = dbge */
    {U,
     {DN, EXPR}, DBCC, 0x5EC8, 0x0},	/* 95 = dbgt */
    {U,
     {DN, EXPR}, DBCC, 0x52C8, 0x0},	/* 96 = dbhi */
    {U,
     {DN, EXPR}, DBCC, 0x5FC8, 0x0},	/* 97 = dble */
    {U,
     {DN, EXPR}, DBCC, 0x53C8, 0x0},	/* 98 = dbls */
    {U,
     {DN, EXPR}, DBCC, 0x5DC8, 0x0},	/* 99 = dblt */
    {U,
     {DN, EXPR}, DBCC, 0x5BC8, 0x0},	/* 100 = dbmi */
    {U,
     {DN, EXPR}, DBCC, 0x56C8, 0x0},	/* 101 = dbne */
    {U,
     {DN, EXPR}, DBCC, 0x5AC8, 0x0},	/* 102 = dbpl */
    {U,
     {DN, EXPR}, DBCC, 0x50C8, 0x0},	/* 103 = dbt */
    {U,
     {DN, EXPR}, DBCC, 0x58C8, 0x0},	/* 104 = dbvc */
    {U,
     {DN, EXPR}, DBCC, 0x59C8, 0x0},	/* 105 = dbvs */
    {U,
     {DN, EXPR}, DBCC, 0x51C8, 0x0},	/* 106 = dbra */
    {wu,
     {DATA, DN}, EAREG, 0x81C0, 0x0},	/* 107 = divs */
    {L,
     {DATA, DN}, MULDIV, 0x4C40, 0x0800},
    {L,
     {DATA, DPAIR}, MULDIV, 0x4C40, 0x0800},
    {lu,
     {DATA, DPAIR}, MULDIV, 0x4C40, 0x0C00},	/* 110 = divsl */
    {wu,
     {DATA, DN}, EAREG, 0x80C0, 0x0},	/* 111 = divu */
    {L,
     {DATA, DN}, MULDIV, 0x4C40, 0x0000},
    {L,
     {DATA, DPAIR}, MULDIV, 0x4C40, 0x0000},
    {lu,
     {DATA, DPAIR}, MULDIV, 0x4C40, 0x0400},	/* 114 = divul */
    {bwlu,
     {DN, DATALT}, REGEAS, 0xB100, 0x0},	/* 115 = eor */
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0A00, 0x0},
    {bu,
     {IMMED, CCR}, IMMB, 0x0A3C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x0A7C, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0A00, 0x0},	/* 119 = eori */
    {bu,
     {IMMED, CCR}, IMMB, 0x0A3C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x0A7C, 0x0},
    {lu,
     {DN, DN}, RXRY, 0xC140, 0x0},	/* 122 = exg */
    {lu,
     {AN, AN}, RXRY, 0xC148, 0x0},
    {lu,
     {DN, AN}, RXRYR, 0xC188, 0x0},
    {lu,
     {AN, DN}, RXRY, 0xC188, 0x0},
    {wu,
     {DN}, REG, 0x4880, 0x0},	/* 126 = ext */
    {L,
     {DN}, REG, 0x48C0, 0x0},
    {lu,
     {DN}, REG, 0x49C0, 0x0},	/* 128 = extb */
    {U,
     {EMPTY}, INH, 0x4AFC, 0x0},/* 129 = illegal */
    {U,
     {CONTROL}, EA, 0x4EC0, 0x0},	/* 130 = jmp */
    {U,
     {CONTROL}, EA, 0x4E80, 0x0},	/* 131 = jsr */
    {lu,
     {CONTROL, AN}, EAREG, 0x41C0, 0x0},	/* 132 = lea */
    {wu,
     {AN, IMMED}, REGIMM, 0x4E50, 0x0},	/* 133 = link */
    {L,
     {AN, IMMED}, REGIMM, 0x4808, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE3C0, 0x0},/* 135 = lsl */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE128, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE108, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE2C0, 0x0},/* 138 = lsr */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE028, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE008, 0x0},
    {wu,
     {ANYEA, DATALT}, MOVE, 0x3000, 0x0},	/* 141 = move */
    {B,
     {DATA, DATALT}, MOVE, 0x1000, 0x0},
    {L,
     {ANYEA, DATALT}, MOVE, 0x2000, 0x0},
    {wu,
     {ANYEA, AN}, MOVE, 0x3000, 0x0},
    {L,
     {ANYEA, AN}, MOVE, 0x2000, 0x0},
    {wu,
     {CCR, DATALT}, EAREV, 0x42C0, 0x0},
    {wu,
     {DATA, CCR}, EA, 0x44C0, 0x0},
    {wu,
     {SR, DATALT}, EAREV, 0x40C0, 0x0},
    {wu,
     {DATA, SR}, EA, 0x46C0, 0x0},
    {lu,
     {CN, AN}, MOVEU, 0x4E68, 0x0},
    {lu,
     {AN, CN}, MOVEU, 0x4E60, 0x0},
    {wu,
     {ANYEA, AN}, MOVE, 0x3000, 0x0},	/* 152 = movea */
    {L,
     {ANYEA, AN}, MOVE, 0x2000, 0x0},
    {lu,
     {CN, RN}, MOVEC, 0x4E7A, 0x0},	/* 154 = movec */
    {lu,
     {RN, CN}, MOVEC, 0x4E7B, 0x0},
    {wu,
     {RN, CALTPR}, MOVEMO, 0x4880, 0x0},	/* 156 = movem */
    {L,
     {RN, CALTPR}, MOVEMO, 0x48C0, 0x0},
    {wu,
     {RLIST, CALTPR}, MOVEMO, 0x4880, 0x0},
    {L,
     {RLIST, CALTPR}, MOVEMO, 0x48C0, 0x0},
    {wu,
     {CTLPST, RN}, MOVEMI, 0x4C80, 0x0},
    {L,
     {CTLPST, RN}, MOVEMI, 0x4CC0, 0x0},
    {wu,
     {CTLPST, RLIST}, MOVEMI, 0x4C80, 0x0},
    {L,
     {CTLPST, RLIST}, MOVEMI, 0x4CC0, 0x0},
    {wu,
     {DN, INDEX}, MOVEPO, 0x0188, 0x0},	/* 164 = movep */
    {L,
     {DN, INDEX}, MOVEPO, 0x01C8, 0x0},
    {wu,
     {INDEX, DN}, MOVEPI, 0x0108, 0x0},
    {L,
     {INDEX, DN}, MOVEPI, 0x0148, 0x0},
    {lu,
     {IMMED, DN}, MOVEQ, 0x7000, 0x0},	/* 168 = moveq */
    {bwlu,
     {RN, ALTMEM}, MOVES, 0x0E00, 0x0800},	/* 169 = moves */
    {bwlu,
     {ALTMEM, RN}, MOVES, 0x0E00, 0x0000},
    {wu,
     {DATA, DN}, EAREG, 0xC1C0, 0x0},	/* 171 = muls */
    {L,
     {DATA, DN}, MULDIV, 0x4C00, 0x0800},
    {L,
     {DATA, DPAIR}, MULDIV, 0x4C00, 0x0C00},
    {wu,
     {DATA, DN}, EAREG, 0xC0C0, 0x0},	/* 174 = mulu */
    {L,
     {DATA, DN}, MULDIV, 0x4C00, 0x0000},
    {L,
     {DATA, DPAIR}, MULDIV, 0x4C00, 0x0400},
    {bu,
     {DATALT}, EA, 0x4800, 0x0},/* 177 = nbcd */
    {bwlu,
     {DATALT}, EAS, 0x4400, 0x0},	/* 178 = neg */
    {bwlu,
     {DATALT}, EAS, 0x4000, 0x0},	/* 179 = negx */
    {U,
     {EMPTY}, INH, 0x4E71, 0x0},/* 180 = nop */
    {bwlu,
     {DATALT}, EAS, 0x4600, 0x0},	/* 181 = not */
    {bwlu,
     {DATA, DN}, EAREGS, 0x8000, 0x0},	/* 182 = or */
    {bwlu,
     {DN, ALTMEM}, REGEAS, 0x8100, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0000, 0x0},
    {bu,
     {IMMED, CCR}, IMMB, 0x003C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x007C, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0000, 0x0},	/* 187 = ori */
    {bu,
     {IMMED, CCR}, IMMB, 0x003C, 0x0},
    {wu,
     {IMMED, SR}, IMMW, 0x007C, 0x0},
    {U,
     {PREDEC, PREDEC, IMMED}, RXRYP, 0x8148, 0x0000},	/* 190 = pack */
    {U,
     {DN, DN, IMMED}, RXRYP, 0x8140, 0x0000},
    {lu,
     {CONTROL}, EA, 0x4840, 0x0},	/* 192 = pea */
    {U,
     {EMPTY}, INH, 0x4E70, 0x0},/* 193 = reset */
    {wu,
     {ALTMEM}, EA, 0xE7C0, 0x0},/* 194 = rol */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE138, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE118, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE6C0, 0x0},/* 197 = ror */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE038, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE018, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE5C0, 0x0},/* 200 = roxl */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE130, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE110, 0x0},
    {wu,
     {ALTMEM}, EA, 0xE4C0, 0x0},/* 203 = roxr */
    {bwlu,
     {DN, DN}, RSHIFT, 0xE030, 0x0},
    {bwlu,
     {IMMED, DN}, QSHIFT, 0xE010, 0x0},
    {U,
     {IMMED}, IMMWS, 0x4E74, 0x0},	/* 206 = rtd */
    {U,
     {EMPTY}, INH, 0x4E73, 0x0},/* 207 = rte */
    {U,
     {AN}, REG, 0x06C8, 0x0},	/* 208 = rtm */
    {U,
     {DN}, REG, 0x06C0, 0x0},
    {U,
     {EMPTY}, INH, 0x4E77, 0x0},/* 210 = rtr */
    {U,
     {EMPTY}, INH, 0x4E75, 0x0},/* 211 = rts */
    {bu,
     {DN, DN}, RXRY, 0x8100, 0x0},	/* 212 = sbcd */
    {bu,
     {PREDEC, PREDEC}, RXRY, 0x8108, 0x0},
    {bu,
     {DATALT}, EA, 0x54C0, 0x0},/* 214 = scc */
    {bu,
     {DATALT}, EA, 0x55C0, 0x0},/* 215 = scs */
    {bu,
     {DATALT}, EA, 0x57C0, 0x0},/* 216 = seq */
    {bu,
     {DATALT}, EA, 0x51C0, 0x0},/* 217 = sf */
    {bu,
     {DATALT}, EA, 0x5CC0, 0x0},/* 218 = sge */
    {bu,
     {DATALT}, EA, 0x5EC0, 0x0},/* 219 = sgt */
    {bu,
     {DATALT}, EA, 0x52C0, 0x0},/* 220 = shi */
    {bu,
     {DATALT}, EA, 0x5FC0, 0x0},/* 221 = sle */
    {bu,
     {DATALT}, EA, 0x53C0, 0x0},/* 222 = sls */
    {bu,
     {DATALT}, EA, 0x5DC0, 0x0},/* 223 = slt */
    {bu,
     {DATALT}, EA, 0x5BC0, 0x0},/* 224 = smi */
    {bu,
     {DATALT}, EA, 0x56C0, 0x0},/* 225 = sne */
    {bu,
     {DATALT}, EA, 0x5AC0, 0x0},/* 226 = spl */
    {bu,
     {DATALT}, EA, 0x50C0, 0x0},/* 227 = st */
    {bu,
     {DATALT}, EA, 0x58C0, 0x0},/* 228 = svc */
    {bu,
     {DATALT}, EA, 0x59C0, 0x0},/* 229 = svs */
    {U,
     {IMMED}, IMMW, 0x4E72, 0x0},	/* 230 = stop */
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0400, 0x0},	/* 231 = sub */
    {wlu,
     {ANYEA, DN}, EAREGS, 0x9000, 0x0},
    {B,
     {DATA, DN}, EAREGS, 0x9000, 0x0},
    {bwlu,
     {DN, ALTMEM}, REGEAS, 0x9100, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0x90C0, 0x0},
    {L,
     {ANYEA, AN}, EAREG, 0x91C0, 0x0},
    {wu,
     {ANYEA, AN}, EAREG, 0x90C0, 0x0},	/* 237 = suba */
    {L,
     {ANYEA, AN}, EAREG, 0x91C0, 0x0},
    {bwlu,
     {IMMED, DATALT}, IMMEAS, 0x0400, 0x0},	/* 239 = subi */
    {bwlu,
     {IMMED, ALTER}, QUKEA, 0x5100, 0x0},	/* 240 = subq */
    {bwlu,
     {DN, DN}, RXRYS, 0x9100, 0x0},	/* 241 = subx */
    {bwlu,
     {PREDEC, PREDEC}, RXRYS, 0x9108, 0x0},
    {wu,
     {DN}, REG, 0x4840, 0x0},	/* 243 = swap */
    {bu,
     {DATALT}, EA, 0x4AC0, 0x0},/* 244 = tas */
    {U,
     {IMMED}, IMM4, 0x4E40, 0x0},	/* 245 = trap */
    {U,
     {EMPTY}, INH, 0x54FC, 0x0},/* 246 = trapcc */
    {wl,
     {IMMED}, TRAPCC, 0x54F8, 0x0},
    {U,
     {EMPTY}, INH, 0x55FC, 0x0},/* 248 = trapcs */
    {wl,
     {IMMED}, TRAPCC, 0x55F8, 0x0},
    {U,
     {EMPTY}, INH, 0x57FC, 0x0},/* 250 = trapeq */
    {wl,
     {IMMED}, TRAPCC, 0x57F8, 0x0},
    {U,
     {EMPTY}, INH, 0x51FC, 0x0},/* 252 = trapf */
    {wl,
     {IMMED}, TRAPCC, 0x51F8, 0x0},
    {U,
     {EMPTY}, INH, 0x5CFC, 0x0},/* 254 = trapge */
    {wl,
     {IMMED}, TRAPCC, 0x5CF8, 0x0},
    {U,
     {EMPTY}, INH, 0x5EFC, 0x0},/* 256 = trapgt */
    {wl,
     {IMMED}, TRAPCC, 0x5EF8, 0x0},
    {U,
     {EMPTY}, INH, 0x52FC, 0x0},/* 258 = traphi */
    {wl,
     {IMMED}, TRAPCC, 0x52F8, 0x0},
    {U,
     {EMPTY}, INH, 0x5FFC, 0x0},/* 260 = traple */
    {wl,
     {IMMED}, TRAPCC, 0x5FF8, 0x0},
    {U,
     {EMPTY}, INH, 0x53FC, 0x0},/* 262 = trapls */
    {wl,
     {IMMED}, TRAPCC, 0x53F8, 0x0},
    {U,
     {EMPTY}, INH, 0x5DFC, 0x0},/* 264 = traplt */
    {wl,
     {IMMED}, TRAPCC, 0x5DF8, 0x0},
    {U,
     {EMPTY}, INH, 0x5BFC, 0x0},/* 266 = trapmi */
    {wl,
     {IMMED}, TRAPCC, 0x5BF8, 0x0},
    {U,
     {EMPTY}, INH, 0x56FC, 0x0},/* 268 = trapne */
    {wl,
     {IMMED}, TRAPCC, 0x56F8, 0x0},
    {U,
     {EMPTY}, INH, 0x5AFC, 0x0},/* 270 = trappl */
    {wl,
     {IMMED}, TRAPCC, 0x5AF8, 0x0},
    {U,
     {EMPTY}, INH, 0x50FC, 0x0},/* 272 = trapt */
    {wl,
     {IMMED}, TRAPCC, 0x50F8, 0x0},
    {U,
     {EMPTY}, INH, 0x58FC, 0x0},/* 274 = trapvc */
    {wl,
     {IMMED}, TRAPCC, 0x58F8, 0x0},
    {U,
     {EMPTY}, INH, 0x59FC, 0x0},/* 276 = trapvs */
    {wl,
     {IMMED}, TRAPCC, 0x59F8, 0x0},
    {U,
     {EMPTY}, INH, 0x4E76, 0x0},/* 278 = trapv */
    {wlu,
     {ANYEA}, EAS, 0x4A00, 0x0},/* 279 = tst */
    {B,
     {DATA}, EAS, 0x4A00, 0x0},
    {U,
     {AN}, REG, 0x4E58, 0x0},	/* 281 = unlk */
    {U,
     {PREDEC, PREDEC, IMMED}, RXRYP, 0x8188, 0x0},	/* 282 = unpk */
    {U,
     {DN, DN, IMMED}, RXRYP, 0x8180, 0x0},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0018},	/* 284 = fabs */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4018},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0018},
    {xu,
     {FN}, FMONAD, 0xF000, 0x001C},	/* 287 = facos */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x401C},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x001C},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4022},	/* 290 = fadd */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0022},
    {xu,
     {FN}, FMONAD, 0xF000, 0x000C},	/* 292 = fasin */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x400C},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x000C},
    {xu,
     {FN}, FMONAD, 0xF000, 0x000A},	/* 295 = fatan */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x400A},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x000A},
    {xu,
     {FN}, FMONAD, 0xF000, 0x000D},	/* 298 = fatanh */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x400D},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x000D},
    {wlu,
     {EXPR}, FBCC, 0xF081, 0x0000},	/* 301 = fbeq */
    {wlu,
     {EXPR}, FBCC, 0xF080, 0x0000},	/* 302 = fbf */
    {wlu,
     {EXPR}, FBCC, 0xF093, 0x0000},	/* 303 = fbge */
    {wlu,
     {EXPR}, FBCC, 0xF096, 0x0000},	/* 304 = fbgl */
    {wlu,
     {EXPR}, FBCC, 0xF097, 0x0000},	/* 305 = fbgle */
    {wlu,
     {EXPR}, FBCC, 0xF092, 0x0000},	/* 306 = fbgt */
    {wlu,
     {EXPR}, FBCC, 0xF095, 0x0000},	/* 307 = fble */
    {wlu,
     {EXPR}, FBCC, 0xF094, 0x0000},	/* 308 = fblt */
    {wlu,
     {EXPR}, FBCC, 0xF08E, 0x0000},	/* 309 = fbne */
    {wlu,
     {EXPR}, FBCC, 0xF09C, 0x0000},	/* 310 = fbnge */
    {wlu,
     {EXPR}, FBCC, 0xF099, 0x0000},	/* 311 = fbngl */
    {wlu,
     {EXPR}, FBCC, 0xF098, 0x0000},	/* 312 = fbngle */
    {wlu,
     {EXPR}, FBCC, 0xF09D, 0x0000},	/* 313 = fbngt */
    {wlu,
     {EXPR}, FBCC, 0xF09A, 0x0000},	/* 314 = fbnle */
    {wlu,
     {EXPR}, FBCC, 0xF09B, 0x0000},	/* 315 = fbnlt */
    {wlu,
     {EXPR}, FBCC, 0xF083, 0x0000},	/* 316 = fboge */
    {wlu,
     {EXPR}, FBCC, 0xF086, 0x0000},	/* 317 = fbogl */
    {wlu,
     {EXPR}, FBCC, 0xF082, 0x0000},	/* 318 = fbogt */
    {wlu,
     {EXPR}, FBCC, 0xF085, 0x0000},	/* 319 = fbole */
    {wlu,
     {EXPR}, FBCC, 0xF084, 0x0000},	/* 320 = fbolt */
    {wlu,
     {EXPR}, FBCC, 0xF087, 0x0000},	/* 321 = fbor */
    {wlu,
     {EXPR}, FBCC, 0xF08F, 0x0000},	/* 322 = fbra */
    {wlu,
     {EXPR}, FBCC, 0xF091, 0x0000},	/* 323 = fbseq */
    {wlu,
     {EXPR}, FBCC, 0xF090, 0x0000},	/* 324 = fbsf */
    {wlu,
     {EXPR}, FBCC, 0xF09E, 0x0000},	/* 325 = fbsne */
    {wlu,
     {EXPR}, FBCC, 0xF09F, 0x0000},	/* 326 = fbst */
    {wlu,
     {EXPR}, FBCC, 0xF08F, 0x0000},	/* 327 = fbt */
    {wlu,
     {EXPR}, FBCC, 0xF089, 0x0000},	/* 328 = fbueq */
    {wlu,
     {EXPR}, FBCC, 0xF08B, 0x0000},	/* 329 = fbuge */
    {wlu,
     {EXPR}, FBCC, 0xF08A, 0x0000},	/* 330 = fbugt */
    {wlu,
     {EXPR}, FBCC, 0xF08D, 0x0000},	/* 331 = fbule */
    {wlu,
     {EXPR}, FBCC, 0xF08C, 0x0000},	/* 332 = fbult */
    {wlu,
     {EXPR}, FBCC, 0xF088, 0x0000},	/* 333 = fbun */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4038},	/* 334 = fcmp */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0038},
    {xu,
     {FN}, FMONAD, 0xF000, 0x001D},	/* 336 = fcos */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x401D},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x001D},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0019},	/* 339 = fcosh */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4019},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0019},
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0001},	/* 342 = fdbeq */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0000},	/* 343 = fdbf */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0013},	/* 344 = fdbge */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0016},	/* 345 = fdbgl */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0017},	/* 346 = fdbgle */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0012},	/* 347 = fdbgt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0015},	/* 348 = fdble */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0014},	/* 349 = fdblt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000E},	/* 350 = fdbne */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001C},	/* 351 = fdbnge */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0019},	/* 352 = fdbngl */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0018},	/* 353 = fdbngle */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001D},	/* 354 = fdbngt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001A},	/* 355 = fdbnle */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001B},	/* 356 = fdbnlt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0003},	/* 357 = fdboge */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0006},	/* 358 = fdbogl */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0002},	/* 359 = fdbogt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0005},	/* 360 = fdbole */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0004},	/* 361 = fdbolt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0007},	/* 362 = fdbor */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0000},	/* 363 = fdbra */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0011},	/* 364 = fdbseq */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0010},	/* 365 = fdbsf */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001E},	/* 366 = fdbsne */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x001F},	/* 367 = fdbst */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000F},	/* 368 = fdbt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0009},	/* 369 = fdbueq */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000B},	/* 370 = fdbuge */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000A},	/* 371 = fdbugt */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000D},	/* 372 = fdbule */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x000C},	/* 373 = fdbult */
    {U,
     {DN, EXPR}, FDBCC, 0xF048, 0x0008},	/* 374 = fdbun */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4020},	/* 375 = fdiv */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0020},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0010},	/* 377 = fetox */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4010},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0010},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0008},	/* 380 = fetoxm1 */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4008},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0008},
    {xu,
     {FN}, FMONAD, 0xF000, 0x001E},	/* 383 = fgetexp */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x401E},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x001E},
    {xu,
     {FN}, FMONAD, 0xF000, 0x001F},	/* 386 = fgetman */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x401F},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x001F},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0001},	/* 389 = fint */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4001},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0001},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0003},	/* 392 = fintrz */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4003},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0003},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0015},	/* 395 = flog10 */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4015},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0015},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0016},	/* 398 = flog2 */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4016},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0016},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0014},	/* 401 = flogn */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4014},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0014},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0006},	/* 404 = flognp1 */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4006},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0006},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4021},	/* 407 = fmod */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0021},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4000},	/* 409 = fmove */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0000},
    {bwlsdxp,
     {FN, DATALT}, FMOVE, 0xF000, 0x6000},
    {P,
     {FN, DATALT, DYNK}, FMOVE, 0xF000, 0x7C00},
    {P,
     {FN, DATALT, STATK}, FMOVE, 0xF000, 0x6C00},
    {lu,
     {ANYEA, FCN}, FMOVEMCI, 0xF000, 0x8000},
    {lu,
     {FCN, ALTER}, FMOVEMCO, 0xF000, 0xA000},
    {xu,
     {IMMED, FN}, FMOVECR, 0xF000, 0x5C00},	/* 416 = fmovecr */
    {X,
     {FLIST, CALTPR}, FMOVEMO, 0xF000, 0xE000},	/* 417 = fmovem */
    {X,
     {DN, CALTPR}, FMOVEMO, 0xF000, 0xE800},
    {X,
     {CTLPST, FLIST}, FMOVEMI, 0xF000, 0xD000},
    {X,
     {CTLPST, DN}, FMOVEMI, 0xF000, 0xD800},
    {lu,
     {ANYEA, FCLIST}, FMOVEMCI, 0xF000, 0x8000},
    {lu,
     {FCLIST, ALTER}, FMOVEMCO, 0xF000, 0xA000},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4023},	/* 423 = fmul */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0023},
    {xu,
     {FN}, FMONAD, 0xF000, 0x001A},	/* 425 = fneg */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x401A},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x001A},
    {U,
     {EMPTY}, FINH, 0xF080, 0x0000},	/* 428 = fnop */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4025},	/* 429 = frem */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0025},
    {U,
     {CTLPST}, FEA, 0xF140, 0x0000},	/* 431 = frestore */
    {U,
     {CALTPR}, FEA, 0xF100, 0x0000},	/* 432 = fsave */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4026},	/* 433 = fscale */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0026},
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0001},	/* 435 = fseq */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0000},	/* 436 = fsf */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0013},	/* 437 = fsge */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0016},	/* 438 = fsgl */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0017},	/* 439 = fsgle */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0012},	/* 440 = fsgt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0015},	/* 441 = fsle */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0014},	/* 442 = fslt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000E},	/* 443 = fsne */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001C},	/* 444 = fsnge */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0019},	/* 445 = fsngl */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0018},	/* 446 = fsngle */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001D},	/* 447 = fsngt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001A},	/* 448 = fsnle */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001B},	/* 449 = fsnlt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0003},	/* 450 = fsoge */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0006},	/* 451 = fsogl */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0002},	/* 452 = fsogt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0005},	/* 453 = fsole */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0004},	/* 454 = fsolt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0007},	/* 455 = fsor */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0011},	/* 456 = fsseq */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0010},	/* 457 = fssf */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001E},	/* 458 = fssne */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x001F},	/* 459 = fsst */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000F},	/* 460 = fst */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0009},	/* 461 = fsueq */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000B},	/* 462 = fsuge */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000A},	/* 463 = fsugt */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000D},	/* 464 = fsule */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x000C},	/* 465 = fsult */
    {bu,
     {DATALT}, FSCC, 0xF040, 0x0008},	/* 466 = fsun */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4024},	/* 467 = fsgldiv */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0024},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4027},	/* 469 = fsglmul */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0027},
    {xu,
     {FN}, FMONAD, 0xF000, 0x000E},	/* 471 = fsin */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x400E},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x000E},
    {bwlsdxp,
     {DATA, FPAIR}, FEAPAIR, 0xF000, 0x4030},	/* 474 = fsincos */
    {X,
     {FN, FPAIR}, FREGPAIR, 0xF000, 0x0030},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0002},	/* 476 = fsinh */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4002},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0002},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0004},	/* 479 = fsqrt */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4004},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0004},
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4028},	/* 482 = fsub */
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0028},
    {xu,
     {FN}, FMONAD, 0xF000, 0x000F},	/* 484 = ftan */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x400F},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x000F},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0009},	/* 487 = ftanh */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4009},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0009},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0012},	/* 490 = ftentox */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4012},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0012},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0001},	/* 493 = ftrapeq */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0001},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0000},	/* 495 = ftrapf */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0000},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0013},	/* 497 = ftrapge */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0013},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0016},	/* 499 = ftrapgl */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0016},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0017},	/* 501 = ftrapgle */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0017},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0012},	/* 503 = ftrapgt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0012},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0015},	/* 505 = ftraple */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0015},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0014},	/* 507 = ftraplt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0014},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000E},	/* 509 = ftrapne */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000E},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001C},	/* 511 = ftrapnge */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001C},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0019},	/* 513 = ftrapngl */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0019},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0018},	/* 515 = ftrapngle */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0018},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001D},	/* 517 = ftrapngt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001D},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001A},	/* 519 = ftrapnle */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001A},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001B},	/* 521 = ftrapnlt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001B},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0003},	/* 523 = ftrapoge */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0003},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0006},	/* 525 = ftrapogl */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0006},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0002},	/* 527 = ftrapogt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0002},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0005},	/* 529 = ftrapole */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0005},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0004},	/* 531 = ftrapolt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0004},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0007},	/* 533 = ftrapor */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0007},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0011},	/* 535 = ftrapseq */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0011},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0010},	/* 537 = ftrapsf */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0010},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001E},	/* 539 = ftrapsne */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001E},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x001F},	/* 541 = ftrapst */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x001F},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000F},	/* 543 = ftrapt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000F},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0009},	/* 545 = ftrapueq */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0009},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000B},	/* 547 = ftrapuge */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000B},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000A},	/* 549 = ftrapugt */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000A},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000D},	/* 551 = ftrapule */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000D},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x000C},	/* 553 = ftrapult */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x000C},
    {U,
     {EMPTY}, FINH, 0xF07C, 0x0008},	/* 555 = ftrapun */
    {wl,
     {IMMED}, FTRAPCC, 0xF078, 0x0008},
    {bwlsdxp,
     {DATA}, FTSTEA, 0xF000, 0x403A},	/* 557 = ftest */
    {X,
     {FN}, FTSTREG, 0xF000, 0x003A},
    {xu,
     {FN}, FMONAD, 0xF000, 0x0011},	/* 559 = ftwotox */
    {bwlsdxp,
     {DATA, FN}, FEAREG, 0xF000, 0x4011},
    {xu,
     {FN, FN}, FREGREG, 0xF000, 0x0011},
    {U,
     {EXPR}, FEQU, 0x0, 0x0},	/* 562 = fequ */
    {U,
     {EMPTY}, FOPT, 0x0, 0x0},	/* 563 = fopt */
    {wlu,
     {EXPR}, PBCC, 0xF080, 0x0},/* 564 = pbbs */
    {wlu,
     {EXPR}, PBCC, 0xF082, 0x0},/* 565 = pbls */
    {wlu,
     {EXPR}, PBCC, 0xF084, 0x0},/* 566 = pbss */
    {wlu,
     {EXPR}, PBCC, 0xF086, 0x0},/* 567 = pbas */
    {wlu,
     {EXPR}, PBCC, 0xF088, 0x0},/* 568 = pbws */
    {wlu,
     {EXPR}, PBCC, 0xF08A, 0x0},/* 569 = pbis */
    {wlu,
     {EXPR}, PBCC, 0xF08C, 0x0},/* 570 = pbgs */
    {wlu,
     {EXPR}, PBCC, 0xF08E, 0x0},/* 571 = pbcs */
    {wlu,
     {EXPR}, PBCC, 0xF081, 0x0},/* 572 = pbbc */
    {wlu,
     {EXPR}, PBCC, 0xF083, 0x0},/* 573 = pblc */
    {wlu,
     {EXPR}, PBCC, 0xF085, 0x0},/* 574 = pbsc */
    {wlu,
     {EXPR}, PBCC, 0xF087, 0x0},/* 575 = pbac */
    {wlu,
     {EXPR}, PBCC, 0xF089, 0x0},/* 576 = pbwc */
    {wlu,
     {EXPR}, PBCC, 0xF08B, 0x0},/* 577 = pbic */
    {wlu,
     {EXPR}, PBCC, 0xF08D, 0x0},/* 578 = pbgc */
    {wlu,
     {EXPR}, PBCC, 0xF08F, 0x0},/* 579 = pbcc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0000},	/* 580 = pdbbs */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0002},	/* 581 = pdbls */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0004},	/* 582 = pdbss */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0006},	/* 583 = pdbas */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0008},	/* 584 = pdbws */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000A},	/* 585 = pdbis */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000C},	/* 586 = pdbgs */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000E},	/* 587 = pdbcs */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0001},	/* 588 = pdbbc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0003},	/* 589 = pdblc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0005},	/* 590 = pdbsc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0007},	/* 591 = pdbac */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x0009},	/* 592 = pdbwc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000B},	/* 593 = pdbic */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000D},	/* 594 = pdbgc */
    {wu,
     {DN, EXPR}, PDBCC, 0xF048, 0x000F},	/* 595 = pdbcc */
    {U,
     {EMPTY}, PINH, 0xF000, 0x2400},	/* 596 = pflusha */
    {U,
     {PEA1, STATK}, PFLUSH, 0xF000, 0x3000},	/* 597 = pflush */
    {U,
     {PEA1, STATK, CTLALT}, PFLUSH, 0xF000, 0x3800},
    {U,
     {PEA1, EXPR}, PFLUSH, 0xF000, 0x3400},	/* 599 = pflushg */
    {U,
     {PEA1, EXPR, CTLALT}, PFLUSH, 0xF000, 0x3C00},
    {U,
     {MEMORY}, PSCC, 0xF000, 0xA000},	/* 601 = pflushr */
    {U,
     {PEA1, CTLALT}, PLOAD, 0xF000, 0x2200},	/* 602 = ploadr */
    {U,
     {PEA1, CTLALT}, PLOAD, 0xF000, 0x2000},	/* 603 = ploadw */
    {bwlu,
     {ANYEA, PN}, PMOVEI, 0xF000, 0x0000},	/* 604 = pmove */
    {bwlu,
     {PN, ALTER}, PMOVEO, 0xF000, 0x0200},
    {wlu,
     {ANYEA, PN}, PMOVEIF, 0xF000, 0x0100},	/* 606 = pmovefd */
    {U,
     {CTLPST}, PEA, 0xF140, 0x0},	/* 607 = prestore */
    {U,
     {CALTPR}, PEA, 0xF100, 0x0},	/* 608 = psave */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0000},	/* 609 = psbs */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0002},	/* 610 = psls */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0004},	/* 611 = psss */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0006},	/* 612 = psas */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0008},	/* 613 = psws */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000A},	/* 614 = psis */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000C},	/* 615 = psgs */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000E},	/* 616 = pscs */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0001},	/* 617 = psbc */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0003},	/* 618 = pslc */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0005},	/* 619 = pssc */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0007},	/* 620 = psac */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x0009},	/* 621 = pswc */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000B},	/* 622 = psic */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000D},	/* 623 = psgc */
    {bu,
     {DATALT}, PSCC, 0xF040, 0x000F},	/* 624 = pscc */
    {U,
     {PEA1, CTLALT, EXPR}, PTEST, 0xF000, 0x8200},	/* 625 = ptestr */
    {U,
     {PEA1, CTLALT, EXPR, AN}, PTEST, 0xF000, 0x8200},
    {U,
     {PEA1, CTLALT, EXPR}, PTEST, 0xF000, 0x8000},	/* 627 = ptestw */
    {U,
     {PEA1, CTLALT, EXPR, AN}, PTEST, 0xF000, 0x8000},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0000},	/* 629 = ptrapbs */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0000},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0002},	/* 631 = ptrapls */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0002},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0004},	/* 633 = ptrapss */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0004},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0006},	/* 635 = ptrapas */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0006},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0008},	/* 637 = ptrapws */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0008},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000A},	/* 639 = ptrapis */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000A},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000C},	/* 641 = ptrapgs */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000C},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000E},	/* 643 = ptrapcs */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000E},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0001},	/* 645 = ptrapbc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0001},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0003},	/* 647 = ptraplc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0003},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0005},	/* 649 = ptrapsc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0005},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0007},	/* 651 = ptrapac */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0007},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x0009},	/* 653 = ptrapwc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x0009},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000B},	/* 655 = ptrapic */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000B},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000D},	/* 657 = ptrapgc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000D},
    {U,
     {EMPTY}, PINH, 0xF07C, 0x000F},	/* 659 = ptrapcc */
    {wl,
     {IMMED}, PTRAPCC, 0xF078, 0x000F},
    {lu,
     {PN, CTLALT}, PVALID, 0xF000, 0x2400},	/* 661 = pvalid */
    {lu,
     {AN, CTLALT}, PVALID, 0xF000, 0x2C00},
    {bwlu,
     {EXPR, EXPR, EXPR}, CPBCC, 0xF080, 0x0},	/* 663 = cpbcc */
    {wu,
     {EXPR, DN, EXPR}, CPDBCC, 0xF048, 0x0},	/* 664 = cpdbcc */
    {U,
     {EXPR, EXPR, ANYEA}, CPGEN, 0xF000, 0x0},	/* 665 = cpgen */
    {U,
     {EXPR, CTLPST}, CPEA, 0xF140, 0x0},	/* 666 = cprestore */
    {U,
     {EXPR, CALTPR}, CPEA, 0xF100, 0x0},	/* 667 = cpsave */
    {bu,
     {EXPR, EXPR, DATALT}, CPSCC, 0xF040, 0x0},	/* 668 = cpscc */
    {U,
     {EXPR, EXPR}, CPINH, 0xF07C, 0x0},	/* 669 = cptrapcc */
    {wl,
     {EXPR, EXPR, IMMED}, CPTRAPCC, 0xF078, 0x0},
    {bwlu,
     {EXPR}, CODEW, 0x0, 0x0},	/* 671 = codew */
    {U,
     {EMPTY}, INH, 0xA9EB, 0x0},/* 672 = fp68k */
};

/* 418 mnemonics, 673 templates  */

struct mne      mnemonic[] =
{
    "abcd", M68op_ABCD, 2, &template[0],
    "add", M68op_ADD, 6, &template[2],
    "adda", M68op_ADDA, 2, &template[8],
    "addi", M68op_ADDI, 1, &template[10],
    "addq", M68op_ADDQ, 1, &template[11],
    "addx", M68op_ADDX, 2, &template[12],
    "and", M68op_AND, 5, &template[14],
    "andi", M68op_ANDI, 3, &template[19],
    "asl", M68op_ASL, 3, &template[22],
    "asr", M68op_ASR, 3, &template[25],
    "bcc", M68op_BCC, 1, &template[28],
    "bchg", M68op_BCHG, 4, &template[44],
    "bclr", M68op_BCLR, 4, &template[48],
    "bcs", M68op_BCS, 1, &template[29],
    "beq", M68op_BEQ, 1, &template[30],
    "bfchg", M68op_BFCHG, 1, &template[60],
    "bfclr", M68op_BFCLR, 1, &template[61],
    "bfexts", M68op_BFEXTS, 2, &template[62],
    "bfextu", M68op_BFEXTU, 2, &template[64],
    "bfffo", M68op_BFFFO, 2, &template[66],
    "bfins", M68op_BFINS, 1, &template[68],
    "bfset", M68op_BFSET, 1, &template[69],
    "bftst", M68op_BFTST, 2, &template[70],
    "bge", M68op_BGE, 1, &template[31],
    "bgt", M68op_BGT, 1, &template[32],
    "bhi", M68op_BHI, 1, &template[33],
    "bkpt", M68op_BKPT, 1, &template[72],
    "ble", M68op_BLE, 1, &template[34],
    "bls", M68op_BLS, 1, &template[35],
    "blt", M68op_BLT, 1, &template[36],
    "bmi", M68op_BMI, 1, &template[37],
    "bne", M68op_BNE, 1, &template[38],
    "bpl", M68op_BPL, 1, &template[39],
    "bra", M68op_BRA, 1, &template[42],
    "bset", M68op_BSET, 4, &template[52],
    "bsr", M68op_BSR, 1, &template[43],
    "btst", M68op_BTST, 4, &template[56],
    "bvc", M68op_BVC, 1, &template[40],
    "bvs", M68op_BVS, 1, &template[41],
    "callm", M68op_CALLM, 1, &template[73],
    "cas", M68op_CAS, 1, &template[74],
    "cas2", M68op_CAS2, 1, &template[75],
    "chk", M68op_CHK, 1, &template[76],
    "chk2", M68op_CHK2, 1, &template[77],
    "clr", M68op_CLR, 1, &template[78],
    "cmp", M68op_CMP, 6, &template[79],
    "cmp2", M68op_CMP2, 1, &template[89],
    "cmpa", M68op_CMPA, 2, &template[85],
    "cmpi", M68op_CMPI, 1, &template[87],
    "cmpm", M68op_CMPM, 1, &template[88],
    "codew", M68op_CODEW, 1, &template[671],
    "cpbcc", M68op_CPBCC, 1, &template[663],
    "cpdbcc", M68op_CPDBCC, 1, &template[664],
    "cpgen", M68op_CPGEN, 1, &template[665],
    "cprestore", M68op_CPRESTORE, 1, &template[666],
    "cpsave", M68op_CPSAVE, 1, &template[667],
    "cpscc", M68op_CPSCC, 1, &template[668],
    "cptrapcc", M68op_CPTRAPCC, 2, &template[669],
    "dbcc", M68op_DBCC, 1, &template[90],
    "dbcs", M68op_DBCS, 1, &template[91],
    "dbeq", M68op_DBEQ, 1, &template[92],
    "dbf", M68op_DBF, 1, &template[93],
    "dbge", M68op_DBGE, 1, &template[94],
    "dbgt", M68op_DBGT, 1, &template[95],
    "dbhi", M68op_DBHI, 1, &template[96],
    "dble", M68op_DBLE, 1, &template[97],
    "dbls", M68op_DBLS, 1, &template[98],
    "dblt", M68op_DBLT, 1, &template[99],
    "dbmi", M68op_DBMI, 1, &template[100],
    "dbne", M68op_DBNE, 1, &template[101],
    "dbpl", M68op_DBPL, 1, &template[102],
    "dbra", M68op_DBRA, 1, &template[106],
    "dbt", M68op_DBT, 1, &template[103],
    "dbvc", M68op_DBVC, 1, &template[104],
    "dbvs", M68op_DBVS, 1, &template[105],
    "divs", M68op_DIVS, 3, &template[107],
    "divsl", M68op_DIVSL, 1, &template[110],
    "divu", M68op_DIVU, 3, &template[111],
    "divul", M68op_DIVUL, 1, &template[114],
    "eor", M68op_EOR, 4, &template[115],
    "eori", M68op_EORI, 3, &template[119],
    "exg", M68op_EXG, 4, &template[122],
    "ext", M68op_EXT, 2, &template[126],
    "extb", M68op_EXTB, 1, &template[128],
    "fabs", M68op_FABS, 3, &template[284],
    "facos", M68op_FACOS, 3, &template[287],
    "fadd", M68op_FADD, 2, &template[290],
    "fasin", M68op_FASIN, 3, &template[292],
    "fatan", M68op_FATAN, 3, &template[295],
    "fatanh", M68op_FATANH, 3, &template[298],
    "fbeq", M68op_FBEQ, 1, &template[301],
    "fbf", M68op_FBF, 1, &template[302],
    "fbge", M68op_FBGE, 1, &template[303],
    "fbgl", M68op_FBGL, 1, &template[304],
    "fbgle", M68op_FBGLE, 1, &template[305],
    "fbgt", M68op_FBGT, 1, &template[306],
    "fble", M68op_FBLE, 1, &template[307],
    "fblt", M68op_FBLT, 1, &template[308],
    "fbne", M68op_FBNE, 1, &template[309],
    "fbnge", M68op_FBNGE, 1, &template[310],
    "fbngl", M68op_FBNGL, 1, &template[311],
    "fbngle", M68op_FBNGLE, 1, &template[312],
    "fbngt", M68op_FBNGT, 1, &template[313],
    "fbnle", M68op_FBNLE, 1, &template[314],
    "fbnlt", M68op_FBNLT, 1, &template[315],
    "fboge", M68op_FBOGE, 1, &template[316],
    "fbogl", M68op_FBOGL, 1, &template[317],
    "fbogt", M68op_FBOGT, 1, &template[318],
    "fbole", M68op_FBOLE, 1, &template[319],
    "fbolt", M68op_FBOLT, 1, &template[320],
    "fbor", M68op_FBOR, 1, &template[321],
    "fbra", M68op_FBRA, 1, &template[322],
    "fbseq", M68op_FBSEQ, 1, &template[323],
    "fbsf", M68op_FBSF, 1, &template[324],
    "fbsne", M68op_FBSNE, 1, &template[325],
    "fbst", M68op_FBST, 1, &template[326],
    "fbt", M68op_FBT, 1, &template[327],
    "fbueq", M68op_FBUEQ, 1, &template[328],
    "fbuge", M68op_FBUGE, 1, &template[329],
    "fbugt", M68op_FBUGT, 1, &template[330],
    "fbule", M68op_FBULE, 1, &template[331],
    "fbult", M68op_FBULT, 1, &template[332],
    "fbun", M68op_FBUN, 1, &template[333],
    "fcmp", M68op_FCMP, 2, &template[334],
    "fcos", M68op_FCOS, 3, &template[336],
    "fcosh", M68op_FCOSH, 3, &template[339],
    "fdbeq", M68op_FDBEQ, 1, &template[342],
    "fdbf", M68op_FDBF, 1, &template[343],
    "fdbge", M68op_FDBGE, 1, &template[344],
    "fdbgl", M68op_FDBGL, 1, &template[345],
    "fdbgle", M68op_FDBGLE, 1, &template[346],
    "fdbgt", M68op_FDBGT, 1, &template[347],
    "fdble", M68op_FDBLE, 1, &template[348],
    "fdblt", M68op_FDBLT, 1, &template[349],
    "fdbne", M68op_FDBNE, 1, &template[350],
    "fdbnge", M68op_FDBNGE, 1, &template[351],
    "fdbngl", M68op_FDBNGL, 1, &template[352],
    "fdbngle", M68op_FDBNGLE, 1, &template[353],
    "fdbngt", M68op_FDBNGT, 1, &template[354],
    "fdbnle", M68op_FDBNLE, 1, &template[355],
    "fdbnlt", M68op_FDBNLT, 1, &template[356],
    "fdboge", M68op_FDBOGE, 1, &template[357],
    "fdbogl", M68op_FDBOGL, 1, &template[358],
    "fdbogt", M68op_FDBOGT, 1, &template[359],
    "fdbole", M68op_FDBOLE, 1, &template[360],
    "fdbolt", M68op_FDBOLT, 1, &template[361],
    "fdbor", M68op_FDBOR, 1, &template[362],
    "fdbra", M68op_FDBRA, 1, &template[363],
    "fdbseq", M68op_FDBSEQ, 1, &template[364],
    "fdbsf", M68op_FDBSF, 1, &template[365],
    "fdbsne", M68op_FDBSNE, 1, &template[366],
    "fdbst", M68op_FDBST, 1, &template[367],
    "fdbt", M68op_FDBT, 1, &template[368],
    "fdbueq", M68op_FDBUEQ, 1, &template[369],
    "fdbuge", M68op_FDBUGE, 1, &template[370],
    "fdbugt", M68op_FDBUGT, 1, &template[371],
    "fdbule", M68op_FDBULE, 1, &template[372],
    "fdbult", M68op_FDBULT, 1, &template[373],
    "fdbun", M68op_FDBUN, 1, &template[374],
    "fdiv", M68op_FDIV, 2, &template[375],
    "fequ", M68op_FEQU, 1, &template[562],
    "fetox", M68op_FETOX, 3, &template[377],
    "fetoxm1", M68op_FETOXM1, 3, &template[380],
    "fgetexp", M68op_FGETEXP, 3, &template[383],
    "fgetman", M68op_FGETMAN, 3, &template[386],
    "fint", M68op_FINT, 3, &template[389],
    "fintrz", M68op_FINTRZ, 3, &template[392],
    "flog10", M68op_FLOG10, 3, &template[395],
    "flog2", M68op_FLOG2, 3, &template[398],
    "flogn", M68op_FLOGN, 3, &template[401],
    "flognp1", M68op_FLOGNP1, 3, &template[404],
    "fmod", M68op_FMOD, 2, &template[407],
    "fmove", M68op_FMOVE, 7, &template[409],
    "fmovecr", M68op_FMOVECR, 1, &template[416],
    "fmovem", M68op_FMOVEM, 6, &template[417],
    "fmul", M68op_FMUL, 2, &template[423],
    "fneg", M68op_FNEG, 3, &template[425],
    "fnop", M68op_FNOP, 1, &template[428],
    "fopt", M68op_FOPT, 1, &template[563],
    "fp68k", M68op_FP68K, 1, &template[672],
    "frem", M68op_FREM, 2, &template[429],
    "frestore", M68op_FRESTORE, 1, &template[431],
    "fsave", M68op_FSAVE, 1, &template[432],
    "fscale", M68op_FSCALE, 2, &template[433],
    "fseq", M68op_FSEQ, 1, &template[435],
    "fsf", M68op_FSF, 1, &template[436],
    "fsge", M68op_FSGE, 1, &template[437],
    "fsgl", M68op_FSGL, 1, &template[438],
    "fsgldiv", M68op_FSGLDIV, 2, &template[467],
    "fsgle", M68op_FSGLE, 1, &template[439],
    "fsglmul", M68op_FSGLMUL, 2, &template[469],
    "fsgt", M68op_FSGT, 1, &template[440],
    "fsin", M68op_FSIN, 3, &template[471],
    "fsincos", M68op_FSINCOS, 2, &template[474],
    "fsinh", M68op_FSINH, 3, &template[476],
    "fsle", M68op_FSLE, 1, &template[441],
    "fslt", M68op_FSLT, 1, &template[442],
    "fsne", M68op_FSNE, 1, &template[443],
    "fsnge", M68op_FSNGE, 1, &template[444],
    "fsngl", M68op_FSNGL, 1, &template[445],
    "fsngle", M68op_FSNGLE, 1, &template[446],
    "fsngt", M68op_FSNGT, 1, &template[447],
    "fsnle", M68op_FSNLE, 1, &template[448],
    "fsnlt", M68op_FSNLT, 1, &template[449],
    "fsoge", M68op_FSOGE, 1, &template[450],
    "fsogl", M68op_FSOGL, 1, &template[451],
    "fsogt", M68op_FSOGT, 1, &template[452],
    "fsole", M68op_FSOLE, 1, &template[453],
    "fsolt", M68op_FSOLT, 1, &template[454],
    "fsor", M68op_FSOR, 1, &template[455],
    "fsqrt", M68op_FSQRT, 3, &template[479],
    "fsseq", M68op_FSSEQ, 1, &template[456],
    "fssf", M68op_FSSF, 1, &template[457],
    "fssne", M68op_FSSNE, 1, &template[458],
    "fsst", M68op_FSST, 1, &template[459],
    "fst", M68op_FST, 1, &template[460],
    "fsub", M68op_FSUB, 2, &template[482],
    "fsueq", M68op_FSUEQ, 1, &template[461],
    "fsuge", M68op_FSUGE, 1, &template[462],
    "fsugt", M68op_FSUGT, 1, &template[463],
    "fsule", M68op_FSULE, 1, &template[464],
    "fsult", M68op_FSULT, 1, &template[465],
    "fsun", M68op_FSUN, 1, &template[466],
    "ftan", M68op_FTAN, 3, &template[484],
    "ftanh", M68op_FTANH, 3, &template[487],
    "ftentox", M68op_FTENTOX, 3, &template[490],
    "ftest", M68op_FTEST, 2, &template[557],
    "ftrapeq", M68op_FTRAPEQ, 2, &template[493],
    "ftrapf", M68op_FTRAPF, 2, &template[495],
    "ftrapge", M68op_FTRAPGE, 2, &template[497],
    "ftrapgl", M68op_FTRAPGL, 2, &template[499],
    "ftrapgle", M68op_FTRAPGLE, 2, &template[501],
    "ftrapgt", M68op_FTRAPGT, 2, &template[503],
    "ftraple", M68op_FTRAPLE, 2, &template[505],
    "ftraplt", M68op_FTRAPLT, 2, &template[507],
    "ftrapne", M68op_FTRAPNE, 2, &template[509],
    "ftrapnge", M68op_FTRAPNGE, 2, &template[511],
    "ftrapngl", M68op_FTRAPNGL, 2, &template[513],
    "ftrapngle", M68op_FTRAPNGLE, 2, &template[515],
    "ftrapngt", M68op_FTRAPNGT, 2, &template[517],
    "ftrapnle", M68op_FTRAPNLE, 2, &template[519],
    "ftrapnlt", M68op_FTRAPNLT, 2, &template[521],
    "ftrapoge", M68op_FTRAPOGE, 2, &template[523],
    "ftrapogl", M68op_FTRAPOGL, 2, &template[525],
    "ftrapogt", M68op_FTRAPOGT, 2, &template[527],
    "ftrapole", M68op_FTRAPOLE, 2, &template[529],
    "ftrapolt", M68op_FTRAPOLT, 2, &template[531],
    "ftrapor", M68op_FTRAPOR, 2, &template[533],
    "ftrapseq", M68op_FTRAPSEQ, 2, &template[535],
    "ftrapsf", M68op_FTRAPSF, 2, &template[537],
    "ftrapsne", M68op_FTRAPSNE, 2, &template[539],
    "ftrapst", M68op_FTRAPST, 2, &template[541],
    "ftrapt", M68op_FTRAPT, 2, &template[543],
    "ftrapueq", M68op_FTRAPUEQ, 2, &template[545],
    "ftrapuge", M68op_FTRAPUGE, 2, &template[547],
    "ftrapugt", M68op_FTRAPUGT, 2, &template[549],
    "ftrapule", M68op_FTRAPULE, 2, &template[551],
    "ftrapult", M68op_FTRAPULT, 2, &template[553],
    "ftrapun", M68op_FTRAPUN, 2, &template[555],
    "ftwotox", M68op_FTWOTOX, 3, &template[559],
    "illegal", M68op_ILLEGAL, 1, &template[129],
    "jmp", M68op_JMP, 1, &template[130],
    "jsr", M68op_JSR, 1, &template[131],
    "lea", M68op_LEA, 1, &template[132],
    "link", M68op_LINK, 2, &template[133],
    "lsl", M68op_LSL, 3, &template[135],
    "lsr", M68op_LSR, 3, &template[138],
    "move", M68op_MOVE, 11, &template[141],
    "movea", M68op_MOVEA, 2, &template[152],
    "movec", M68op_MOVEC, 2, &template[154],
    "movem", M68op_MOVEM, 8, &template[156],
    "movep", M68op_MOVEP, 4, &template[164],
    "moveq", M68op_MOVEQ, 1, &template[168],
    "moves", M68op_MOVES, 2, &template[169],
    "muls", M68op_MULS, 3, &template[171],
    "mulu", M68op_MULU, 3, &template[174],
    "nbcd", M68op_NBCD, 1, &template[177],
    "neg", M68op_NEG, 1, &template[178],
    "negx", M68op_NEGX, 1, &template[179],
    "nop", M68op_NOP, 1, &template[180],
    "not", M68op_NOT, 1, &template[181],
    "or", M68op_OR, 5, &template[182],
    "ori", M68op_ORI, 3, &template[187],
    "pack", M68op_PACK, 2, &template[190],
    "pbac", M68op_PBAC, 1, &template[575],
    "pbas", M68op_PBAS, 1, &template[567],
    "pbbc", M68op_PBBC, 1, &template[572],
    "pbbs", M68op_PBBS, 1, &template[564],
    "pbcc", M68op_PBCC, 1, &template[579],
    "pbcs", M68op_PBCS, 1, &template[571],
    "pbgc", M68op_PBGC, 1, &template[578],
    "pbgs", M68op_PBGS, 1, &template[570],
    "pbic", M68op_PBIC, 1, &template[577],
    "pbis", M68op_PBIS, 1, &template[569],
    "pblc", M68op_PBLC, 1, &template[573],
    "pbls", M68op_PBLS, 1, &template[565],
    "pbsc", M68op_PBSC, 1, &template[574],
    "pbss", M68op_PBSS, 1, &template[566],
    "pbwc", M68op_PBWC, 1, &template[576],
    "pbws", M68op_PBWS, 1, &template[568],
    "pdbac", M68op_PDBAC, 1, &template[591],
    "pdbas", M68op_PDBAS, 1, &template[583],
    "pdbbc", M68op_PDBBC, 1, &template[588],
    "pdbbs", M68op_PDBBS, 1, &template[580],
    "pdbcc", M68op_PDBCC, 1, &template[595],
    "pdbcs", M68op_PDBCS, 1, &template[587],
    "pdbgc", M68op_PDBGC, 1, &template[594],
    "pdbgs", M68op_PDBGS, 1, &template[586],
    "pdbic", M68op_PDBIC, 1, &template[593],
    "pdbis", M68op_PDBIS, 1, &template[585],
    "pdblc", M68op_PDBLC, 1, &template[589],
    "pdbls", M68op_PDBLS, 1, &template[581],
    "pdbsc", M68op_PDBSC, 1, &template[590],
    "pdbss", M68op_PDBSS, 1, &template[582],
    "pdbwc", M68op_PDBWC, 1, &template[592],
    "pdbws", M68op_PDBWS, 1, &template[584],
    "pea", M68op_PEA, 1, &template[192],
    "pflush", M68op_PFLUSH, 2, &template[597],
    "pflusha", M68op_PFLUSHA, 1, &template[596],
    "pflushg", M68op_PFLUSHG, 2, &template[599],
    "pflushr", M68op_PFLUSHR, 1, &template[601],
    "ploadr", M68op_PLOADR, 1, &template[602],
    "ploadw", M68op_PLOADW, 1, &template[603],
    "pmove", M68op_PMOVE, 2, &template[604],
    "pmovefd", M68op_PMOVEFD, 1, &template[606],
    "prestore", M68op_PRESTORE, 1, &template[607],
    "psac", M68op_PSAC, 1, &template[620],
    "psas", M68op_PSAS, 1, &template[612],
    "psave", M68op_PSAVE, 1, &template[608],
    "psbc", M68op_PSBC, 1, &template[617],
    "psbs", M68op_PSBS, 1, &template[609],
    "pscc", M68op_PSCC, 1, &template[624],
    "pscs", M68op_PSCS, 1, &template[616],
    "psgc", M68op_PSGC, 1, &template[623],
    "psgs", M68op_PSGS, 1, &template[615],
    "psic", M68op_PSIC, 1, &template[622],
    "psis", M68op_PSIS, 1, &template[614],
    "pslc", M68op_PSLC, 1, &template[618],
    "psls", M68op_PSLS, 1, &template[610],
    "pssc", M68op_PSSC, 1, &template[619],
    "psss", M68op_PSSS, 1, &template[611],
    "pswc", M68op_PSWC, 1, &template[621],
    "psws", M68op_PSWS, 1, &template[613],
    "ptestr", M68op_PTESTR, 2, &template[625],
    "ptestw", M68op_PTESTW, 2, &template[627],
    "ptrapac", M68op_PTRAPAC, 2, &template[651],
    "ptrapas", M68op_PTRAPAS, 2, &template[635],
    "ptrapbc", M68op_PTRAPBC, 2, &template[645],
    "ptrapbs", M68op_PTRAPBS, 2, &template[629],
    "ptrapcc", M68op_PTRAPCC, 2, &template[659],
    "ptrapcs", M68op_PTRAPCS, 2, &template[643],
    "ptrapgc", M68op_PTRAPGC, 2, &template[657],
    "ptrapgs", M68op_PTRAPGS, 2, &template[641],
    "ptrapic", M68op_PTRAPIC, 2, &template[655],
    "ptrapis", M68op_PTRAPIS, 2, &template[639],
    "ptraplc", M68op_PTRAPLC, 2, &template[647],
    "ptrapls", M68op_PTRAPLS, 2, &template[631],
    "ptrapsc", M68op_PTRAPSC, 2, &template[649],
    "ptrapss", M68op_PTRAPSS, 2, &template[633],
    "ptrapwc", M68op_PTRAPWC, 2, &template[653],
    "ptrapws", M68op_PTRAPWS, 2, &template[637],
    "pvalid", M68op_PVALID, 2, &template[661],
    "reset", M68op_RESET, 1, &template[193],
    "rol", M68op_ROL, 3, &template[194],
    "ror", M68op_ROR, 3, &template[197],
    "roxl", M68op_ROXL, 3, &template[200],
    "roxr", M68op_ROXR, 3, &template[203],
    "rtd", M68op_RTD, 1, &template[206],
    "rte", M68op_RTE, 1, &template[207],
    "rtm", M68op_RTM, 2, &template[208],
    "rtr", M68op_RTR, 1, &template[210],
    "rts", M68op_RTS, 1, &template[211],
    "sbcd", M68op_SBCD, 2, &template[212],
    "scc", M68op_SCC, 1, &template[214],
    "scs", M68op_SCS, 1, &template[215],
    "seq", M68op_SEQ, 1, &template[216],
    "sf", M68op_SF, 1, &template[217],
    "sge", M68op_SGE, 1, &template[218],
    "sgt", M68op_SGT, 1, &template[219],
    "shi", M68op_SHI, 1, &template[220],
    "sle", M68op_SLE, 1, &template[221],
    "sls", M68op_SLS, 1, &template[222],
    "slt", M68op_SLT, 1, &template[223],
    "smi", M68op_SMI, 1, &template[224],
    "sne", M68op_SNE, 1, &template[225],
    "spl", M68op_SPL, 1, &template[226],
    "st", M68op_ST, 1, &template[227],
    "stop", M68op_STOP, 1, &template[230],
    "sub", M68op_SUB, 6, &template[231],
    "suba", M68op_SUBA, 2, &template[237],
    "subi", M68op_SUBI, 1, &template[239],
    "subq", M68op_SUBQ, 1, &template[240],
    "subx", M68op_SUBX, 2, &template[241],
    "svc", M68op_SVC, 1, &template[228],
    "svs", M68op_SVS, 1, &template[229],
    "swap", M68op_SWAP, 1, &template[243],
    "tas", M68op_TAS, 1, &template[244],
    "trap", M68op_TRAP, 1, &template[245],
    "trapcc", M68op_TRAPCC, 2, &template[246],
    "trapcs", M68op_TRAPCS, 2, &template[248],
    "trapeq", M68op_TRAPEQ, 2, &template[250],
    "trapf", M68op_TRAPF, 2, &template[252],
    "trapge", M68op_TRAPGE, 2, &template[254],
    "trapgt", M68op_TRAPGT, 2, &template[256],
    "traphi", M68op_TRAPHI, 2, &template[258],
    "traple", M68op_TRAPLE, 2, &template[260],
    "trapls", M68op_TRAPLS, 2, &template[262],
    "traplt", M68op_TRAPLT, 2, &template[264],
    "trapmi", M68op_TRAPMI, 2, &template[266],
    "trapne", M68op_TRAPNE, 2, &template[268],
    "trappl", M68op_TRAPPL, 2, &template[270],
    "trapt", M68op_TRAPT, 2, &template[272],
    "trapv", M68op_TRAPV, 1, &template[278],
    "trapvc", M68op_TRAPVC, 2, &template[274],
    "trapvs", M68op_TRAPVS, 2, &template[276],
    "tst", M68op_TST, 2, &template[279],
    "unlk", M68op_UNLK, 1, &template[281],
    "unpk", M68op_UNPK, 2, &template[282],
    {0}
};
int             Nmne = (sizeof(mnemonic) / sizeof(mnemonic[0])) - 1;
