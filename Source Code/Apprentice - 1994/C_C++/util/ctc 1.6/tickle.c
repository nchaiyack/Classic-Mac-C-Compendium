/*
** From: dickie@schaefer.math.wisc.edu (Garth Dickie)
** Newsgroups: comp.sys.mac.programmer
** Subject: Re: HOW do I get the finder to update its file information..
** Message-ID: <1992May22.014427.9977@schaefer.math.wisc.edu>
** Date: 22 May 92 01:44:27 GMT
** References: <1992May21.231246.23090@crash.cts.com>
** Organization: Univ. of Wisconsin Dept. of Mathematics
** Lines: 58
** 
** You need to change the modification date of the parent directory.
** The routine TickleParent below works for me.  In case you are not
** using the FSSpec structure, the routine MakeWDSpec will convert
** a working directory / filename pair to a psuedo-FSSpec.  I use
** this when the new file manager calls are not available, as the rest
** of my code passes around FSSpecs.  The code compiles under THINK C
** 5.0, with the usual headers.
** 
** BTW (mild flame) this is simple enough that it's pretty annoying
** when people don't do it.  Most of the nifty little 'drag-and-drop'
** utilities that have come around recently forget to do this.  Feel
** free to use the code below in any way you see fit.
*/
#include <Files.h>

OSErr TickleParent( FSSpec * child ) {
    CInfoPBRec    pb;
    OSErr         err = noErr;
 
    pb.dirInfo.ioCompletion = 0;
    pb.dirInfo.ioNamePtr = 0;
    pb.dirInfo.ioVRefNum = child->vRefNum;
    pb.dirInfo.ioFDirIndex = -1;              // get info based on the DirID
    pb.dirInfo.ioDrDirID = child->parID;
    
    if(( err = PBGetCatInfoSync( &pb )) != noErr ) return err;
 
    GetDateTime( &pb.dirInfo.ioDrMdDat );
 
    err = PBSetCatInfoSync( &pb );
 
    return err;
}

//	err = FSMakeFSSpec(reply.sfFile.vRefNum, reply.sfFile.parID, reply.sfFile.name, &reply.sfFile);
	
OSErr MakeWDSpec( FSSpec *spec, short vRefNum, Str63 name ) {
    WDPBRec		parameter;
    Size		length;
    OSErr		err;
    short		nameLen;
    
    parameter.ioCompletion = 0L;
    parameter.ioNamePtr = 0L;
    parameter.ioVRefNum = vRefNum;
    parameter.ioWDIndex = 0;
    parameter.ioWDProcID = 0;
    parameter.ioWDVRefNum = 0;
 
    err = PBGetWDInfoSync( &parameter );
 	
	spec->vRefNum = parameter.ioWDVRefNum;
	spec->parID = parameter.ioWDDirID;
   	nameLen = name[0];
    length = (( nameLen < sizeof spec->name - 1 ) ? nameLen : sizeof spec->name -1);
    BlockMove( name + 1, spec->name + 1, length );
    *spec->name = length;
    
    return err;
}
