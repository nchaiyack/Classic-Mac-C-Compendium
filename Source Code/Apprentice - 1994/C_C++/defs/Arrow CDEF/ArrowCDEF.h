/******************************************************************
	FILE:			ArrowCDEF.h
	DESCRIPTION:	Some useful constants for the ArrowCDEF
	AUTHOR:		David Hay
	
	Copyright � 1994 David Hay
******************************************************************/

#pragma once

#define	kArrowResType	'APIC'	/*	Resource type to define which PICT's to use	*/
#define	kArrowResID		500		/*	If you change the CDEF id, change this too		*/
#define	kArrowProc		( kArrowResID * 16 )

enum
{
	kPlainArrow = 0,
	kUpArrow,
	kDownArrow,
	kInactiveArrow
};