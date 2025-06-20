/**********************************************************************
 *	This file contains the functions which let the user determine 
 *	which file to change the creator and/or file types.
 **********************************************************************/

#include "Creator Changer.h"
#include "Creator Changer.file.h"


/**********************************************************************
 *	Function Pick_File(), this function lets the user pick the file
 *	that is to be changed.
 **********************************************************************/

Boolean Pick_File(FSSpec *the_file_spec, FInfo *file_info)
	{
	
	if(Old_File(the_file_spec, file_info)) return(TRUE);
	else return(FALSE);
	
	}



/**********************************************************************
 *	Function Old_File(), this function calls the file to have the 
 *	creator and / or file type changed.
 **********************************************************************/

Boolean Old_File(FSSpec *the_file_spec, FInfo *file_info)
	{
	
	StandardFileReply	reply;
	SFTypeList			my_types;
	
	StandardGetFile(NIL_PTR, -1, my_types, &reply);
	if(!reply.sfGood) return(FALSE);
	else
		{
		*the_file_spec=reply.sfFile;
		FSpGetFInfo(&reply.sfFile, file_info);
		return(TRUE);
		}
	
	}




/**********************************************************************
 *	Function p_Str_Copy(), this function copies one pascal string to
 *	another pascal string.
 **********************************************************************/

void p_Str_Copy(StringPtr p1, StringPtr p2)
	{
	
	register int len;
	
	len=*p2++=*p1++;
	while(--len>=0) *p2++=*p1++;
	
	}



/**********************************************************************
 *	Function Force_Finder_Update(), this function forces the Finder
 *	to update the window in which the file was changed in.
 **********************************************************************/

void Force_Finder_Update(FSSpec *the_file_spec)
	{
	
	OSErr			error;
	CInfoPBRec		lCBlk;
	
	if(the_file_spec->parID!=1) lCBlk.dirInfo.ioNamePtr=0L;
	lCBlk.dirInfo.ioVRefNum=the_file_spec->vRefNum;		
	lCBlk.dirInfo.ioDrDirID=the_file_spec->parID;
	lCBlk.dirInfo.ioFDirIndex=0;
	lCBlk.dirInfo.ioCompletion=0;

	error=PBGetCatInfoSync(&lCBlk);
	if(!error)
		{
		GetDateTime(&lCBlk.dirInfo.ioDrMdDat);
		lCBlk.dirInfo.ioDrDirID=the_file_spec->parID;
		PBSetCatInfoSync(&lCBlk);
		FlushVol(NIL_PTR, the_file_spec->vRefNum);
		}
	
	}