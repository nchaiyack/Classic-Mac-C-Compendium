/*
 *	Selectors.c
 *
 *	WASTE PROJECT
 *	WEGetInfo / WESetInfo
 *
 *	Copyright (c) 1993-1994 Marco Piovanelli
 *	All Rights Reserved
 *
 */

typedef struct FieldDescriptor {
	short offset;
	short length;
} FieldDescriptor;

pascal void _WELookupSelector(OSType selector, FieldDescriptor *desc);

pascal void _WELookupSelector(OSType selector, FieldDescriptor *desc)
{
	asm {
			bra @entry

@table		dc.l	'clik', 0x00640004
			dc.l	'line', 0x00080004
			dc.l	'port', 0x00000004
			dc.l	'post', 0x00900004
			dc.l	'pre ', 0x008C0004
			dc.l	'refc', 0x007C0004
			dc.l	'runa', 0x00100004
			dc.l	'scrl', 0x00740004
			dc.l	'styl', 0x000C0004
			dc.l	'text', 0x00040004
			dc.l	'tsmd', 0x00800004
			dc.l	     0, 0x00000000

@entry		move.l selector, d0			; d0 = selector code to search
			lea @table, a0				; a0 = start of table
			move.l desc, a1				; a1 = ptr to desc variable
			clr.l (a1)					; 

@loop		cmp.l (a0)+, d0				; selector found?
			beq @found					; 
			tst.l (a0)+					; at the end of the table?
			bne.s @loop					; 
			bra.s @exit					; 

@found		move.l (a0), (a1)			; copy descriptor record

@exit
	}
}