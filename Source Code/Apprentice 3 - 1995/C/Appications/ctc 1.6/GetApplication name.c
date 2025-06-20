/*****************
	The following routine are based on DTS sample code.
	
*/

#include <Files.h>
OSErr GetApplicationName(OSType fCreator, FSSpec *file, short ioVRefNum);
OSErr VolHasOption( short vRefNum, Boolean *hasDesktop, short option );
OSErr VolHasDesktopDB( short vRefNum, Boolean *hasDesktop );
OSErr FindAppOnVolume( OSType sig, short vRefNum, FSSpec *file );
	/*************************************
	Get name of application that created it, if on volume being scanned!
	**************************************/
OSErr GetApplicationName(OSType fCreator, FSSpec *file, short ioVRefNum)
{		Boolean hasDesktop;
		OSErr rc;
		
		rc = VolHasDesktopDB( ioVRefNum, &hasDesktop);
		if (rc == noErr && hasDesktop) {
			rc = FindAppOnVolume(fCreator, ioVRefNum, file );
		} else
			if (rc == noErr) rc = -1;
return rc;
}

/* ////////////////////////////////////////////////////////////
  //	VolHasDesktopDB										//
 //			Check if a volume supports desktop DB calls	   //
//////////////////////////////////////////////////////////*/
OSErr VolHasDesktopDB( short vRefNum, Boolean *hasDesktop )
{
	return VolHasOption(vRefNum, hasDesktop, bHasDesktopMgr );
}



OSErr VolHasOption( short vRefNum, Boolean *hasDesktop, short option )
{	HParamBlockRec pb;
	GetVolParmsInfoBuffer info;	// Volume Info Rec; see IM-VI 25-32
	OSErr err;
	
{	long i, *ip;
	for( i=0, ip=(long*)&pb; i<sizeof(pb)/sizeof(long); i++ )		// Clear pb
		*ip++ = 0;
}
	pb.ioParam.ioCompletion = NULL;
	pb.ioParam.ioNamePtr = NULL;
	pb.ioParam.ioVRefNum = vRefNum;
	pb.ioParam.ioBuffer = (Ptr)&info;
	pb.ioParam.ioReqCount = sizeof(info);
	
	err= PBHGetVolParms(&pb,false);			// Get volume info

	*hasDesktop = err==noErr && (info.vMAttrib & (1L << option))!=0;
	return err;
}

/* ////////////////////////////////////////////////////////////
  //	FindAppOnVolume										//
 //			Ask vol's desktop db for application		   //
//////////////////////////////////////////////////////////*/
OSErr FindAppOnVolume( OSType sig, short vRefNum, FSSpec *file )
{	DTPBRec pb;
	OSErr err;
	short refNum;
	
{	long i, *ip;
	for( i=0, ip=(long*)&pb; i<sizeof(pb)/sizeof(long); i++ )		/* Clear pb */
		*ip++ = 0;
}
	pb.ioCompletion = NULL;
	pb.ioVRefNum = vRefNum;
	pb.ioNamePtr = NULL;
	err= PBDTGetPath(&pb);				/* Puts DT refnum into pb.ioDTRefNum */
	if( err ) return err;
	
	refNum = pb.ioDTRefNum;
	
	pb.ioCompletion = NULL;
	pb.ioDTRefNum = refNum;
	pb.ioIndex = 0;
	pb.ioFileCreator = sig;
	pb.ioNamePtr = (StringPtr)&file->name;
	err= PBDTGetAPPLSync(&pb);						/* Find the Application! */
	
	if( err == fnfErr )
		err = afpItemNotFound;						/* Bug in PBDTGetAPPL() */
	if( err )	return err;							/* Returns afpItemNotFound if app wasn't found. */

	file->vRefNum = vRefNum;
	file->parID = pb.ioAPPLParID;
	return noErr;
}	/* End of () */