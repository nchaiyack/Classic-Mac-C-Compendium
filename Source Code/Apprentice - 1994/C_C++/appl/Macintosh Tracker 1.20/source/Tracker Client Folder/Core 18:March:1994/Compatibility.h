/* Compatibility.h */

#pragma once


typedef struct
	{
		ushort	AppleEvents : 1;
		ushort	MathCoprocessor : 1;
		ushort	VirtualMemory : 1;
		ushort	GoodTempMem : 1;
		ushort	MC68020orBetter : 1;
		ushort	StandardFile58 : 1;
		ushort	FSSpecCallsPresent : 1;
		ushort	FindFolderPresent : 1;
		ushort	GreyishTextOr : 1;
		ushort	AliasManager : 1;
		ushort	HelpManager : 1;
		ushort	PowerManager : 1;
	} CompatibleRec;


#ifndef COMPILING_COMPATIBILITY_C
	extern CompatibleRec	Compatibility;
#endif

#define HasMathCoprocessor (Compatibility.MathCoprocessor)
#define HasAppleEvents (Compatibility.AppleEvents)
#define HasVirtualMemory (Compatibility.VirtualMemory)
#define HasGoodTemporaryMemory (Compatibility.GoodTempMem)
#define Has020orBetter (Compatibility.MC68020orBetter)
#define HasFSSpecStandardFile (Compatibility.StandardFile58)
#define HasFSSpec (Compatibility.FSSpecCallsPresent)
#define HasFindFolder (Compatibility.FindFolderPresent)
#define HasGreyishTextOr (Compatibility.GreyishTextOr)
#define HasAliasManager (Compatibility.AliasManager)
#define HasHelpManager (Compatibility.HelpManager)
#define HasPowerManager (Compatibility.PowerManager)

void		InitCompatibility(void);
