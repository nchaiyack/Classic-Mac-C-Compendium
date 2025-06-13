/* ==========================================

	fss_utl2.c	

	Copyright (c) 1993,1994,1995 Newport Software Development
			
   ========================================== */

#include <GestaltEqu.h>

#include "fss_utl2.proto.h"

/* ========================================== */

OSErr fss_Create( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag )
{
	OSErr			result;
	HParamBlockRec	block;
		
	if (gotFSSpec)
		return (FSpCreate(spec, creator, type, tag));
	else {
		block.fileParam.ioVRefNum = spec->vRefNum;
		block.fileParam.ioDirID = spec->parID;
		block.fileParam.ioNamePtr = (StringPtr) &(spec->name);
		block.fileParam.ioFVersNum = 0;
		return( PBHCreateSync(&block));
		}
}

/* ========================================== */

OSErr fss_CreateResFile( int gotFSSpec, const FSSpec *spec, OSType creator, OSType type, ScriptCode tag)
{
	OSErr			result;
	CInfoPBRec		pb;
	
	if (gotFSSpec) {
		result = ResError();
		FSpCreateResFile(spec, creator, type, tag);
		result = ResError();
		}
	else {
		HCreateResFile(spec->vRefNum, spec->parID, spec->name);
		result = ResError();
		if (!result) {
			// set creator and type
			pb.hFileInfo.ioVRefNum = spec->vRefNum;
			pb.hFileInfo.ioDirID = spec->parID;
			pb.hFileInfo.ioNamePtr = (StringPtr) &(spec->name);
			pb.hFileInfo.ioFDirIndex = 0;
			result = PBGetCatInfoSync(&pb);
			if (!result) {
				pb.hFileInfo.ioFlFndrInfo.fdCreator = creator;
				pb.hFileInfo.ioFlFndrInfo.fdType = type;
				pb.hFileInfo.ioDirID = spec->parID;
				result = PBSetCatInfoSync(&pb);
				}
			}
		}
		
	return( result );
}

/* ========================================== */

OSErr fss_Delete(int gotFSSpec, const FSSpec *spec)
{
	HParamBlockRec	pb;

	if (gotFSSpec)
		return (FSpDelete(spec));
	else {
		pb.ioParam.ioVRefNum = spec->vRefNum;
		pb.fileParam.ioDirID = spec->parID;
		pb.ioParam.ioNamePtr = (StringPtr) &(spec->name);
		pb.ioParam.ioVersNum = 0;
		return (PBHDeleteSync(&pb));
		}
}

/* ========================================== */
	
