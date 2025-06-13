/* ==========================================

	fss_utl.c	

	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
		
   ========================================== */

#include <GestaltEqu.h>

#include "fss_utl.proto.h"

/* ========================================== */

// test this once and store it away somewhere safe

int fss_test( void )
{
	long	response;
	
	if ( Gestalt( gestaltFSAttr, &response ) )
		return(false);
		
	if ( response & ( 1L << gestaltHasFSSpecCalls ) )
		return(true);
	else
		return(false);
}

/* ========================================== */

// open data fork using FSSpec

OSErr fss_OpenDF(int gotFSSpec, const FSSpec *spec, char permission, short *refNum)
{
	OSErr			result;
	HParamBlockRec	pb;

	if (gotFSSpec)
		return (FSpOpenDF(spec, permission, refNum));
	else {
		pb.ioParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.ioParam.ioNamePtr = (StringPtr) &(spec->name);
		pb.ioParam.ioVersNum = 0;
		pb.ioParam.ioPermssn = permission;
		pb.ioParam.ioMisc = nil;
		result = PBHOpenSync(&pb);
		*refNum = pb.ioParam.ioRefNum;
		return (result);
		}
}

/* ========================================== */

OSErr fss_OpenRF(int gotFSSpec, const FSSpec *spec, char permission, short *refNum)
{
	OSErr			result;
	HParamBlockRec	pb;

	if (gotFSSpec)
		return (FSpOpenRF(spec, permission, refNum));
	else {
		pb.ioParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.ioParam.ioNamePtr = (StringPtr) &(spec->name);
		pb.ioParam.ioVersNum = 0;
		pb.ioParam.ioPermssn = permission;
		pb.ioParam.ioMisc = nil;
		result = PBHOpenRFSync(&pb);
		*refNum = pb.ioParam.ioRefNum;
		return (result);
		}
}

/* ========================================== */

// read the finder information block

OSErr fss_GetFInfo(int gotFSSpec, const FSSpec *spec, FInfo *fndrInfo)
{
	OSErr			result;
	HParamBlockRec	pb;

	if (gotFSSpec)
		return (FSpGetFInfo(spec, fndrInfo));
	else {
		pb.fileParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.fileParam.ioNamePtr = (StringPtr) &(spec->name);
		pb.fileParam.ioFVersNum = 0;
		pb.fileParam.ioFDirIndex = 0;
		result = PBHGetFInfoSync(&pb);
		*fndrInfo = pb.fileParam.ioFlFndrInfo;
		return (result);
		}
}

/* ========================================== */

OSErr fss_SetFInfo(int gotFSSpec, const FSSpec *spec, const FInfo *fndrInfo)
{
	OSErr			result;
	HParamBlockRec	pb;

	if (gotFSSpec)
		return (FSpSetFInfo(spec, fndrInfo));
	else {
		pb.fileParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.fileParam.ioNamePtr = (StringPtr) &(spec->name);
		pb.fileParam.ioFVersNum = 0;
		pb.fileParam.ioFDirIndex = 0;
		result = PBHGetFInfoSync(&pb);
		if (!result) {
			pb.fileParam.ioFlFndrInfo = *fndrInfo;
			pb.fileParam.ioDirID = spec->parID;
			result = PBHSetFInfoSync(&pb);
			}
		return (result);
		}
}

/* ========================================== */

OSErr fss_to_DirID(const FSSpec *spec, long *theDirID, Boolean *isDirectory)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = (StringPtr)spec->name;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;
	error = PBGetCatInfoSync(&pb);
	
	if (error) {
		*theDirID = 0;
		*isDirectory = 0;
		}
	else {
		*theDirID = pb.hFileInfo.ioDirID;
		*isDirectory = (pb.hFileInfo.ioFlAttrib & 0x10) != 0;
		}
		
	return (error);
}

/* ========================================== */

// advance the date of a parent directory, so the finder knows to redraw etc.

OSErr fss_wake_parent(const FSSpec *spec)
{
	CInfoPBRec pb;
	OSErr error;
	unsigned long old_secs;
	unsigned long new_secs;

	pb.hFileInfo.ioNamePtr = nil;
	pb.hFileInfo.ioVRefNum = spec->vRefNum;
	pb.hFileInfo.ioDirID = spec->parID;
	pb.hFileInfo.ioFDirIndex = 0;
	
	error = PBGetCatInfoSync(&pb);
	GetDateTime(&new_secs);
	
	if (error == noErr) {
		old_secs = pb.hFileInfo.ioFlMdDat;
		if (old_secs < new_secs)
			pb.hFileInfo.ioFlMdDat = new_secs;
		else
			pb.hFileInfo.ioFlMdDat++;
		pb.hFileInfo.ioDirID = spec->parID;
		error = PBSetCatInfoSync(&pb);
		}
		
	return (error);
}
