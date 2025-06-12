#define FILE_NUM 13
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * various useful functions related to the filesystem
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Lib
#define FILL(pb,name,vRef,dirId) do { 									\
	(pb).ioNamePtr = name;																\
	(pb).ioVRefNum = vRef;																\
	(pb).ioDirID = dirId; 																\
	} while (0)

/**********************************************************************
 * GetMyVR - get a volume ref number
 **********************************************************************/
short GetMyVR(UPtr name)
{
	HVolumeParam vInfo;
	
	FILL(*((HFileInfo*)&vInfo),name,0,0);
	vInfo.ioVolIndex = -1;
	if (PBHGetVInfo(&vInfo,FALSE))
		return(0);
	return(vInfo.ioVRefNum);
}

/**********************************************************************
 * get a dir ID, given a vrefnum
 **********************************************************************/
long GetMyDirID(short refNum)
{
	WDPBRec wdBlock;		/* to turn a working dir id into a dir id */
	Byte name[32];			/* we gotta give it some room to write the name */
	
	/*
	 * HFS.  get working directory info
	 */
	wdBlock.ioVRefNum = wdBlock.ioWDVRefNum = refNum;
	wdBlock.ioWDIndex = 0;
	wdBlock.ioWDProcID = nil;
	wdBlock.ioNamePtr = name;
	if (PBGetWDInfo(&wdBlock,0)!=noErr) /* get working directory info */
		return(0);
	else
		return(wdBlock.ioWDDirID);
}

/**********************************************************************
 * get a name, given a vRefNum
 **********************************************************************/
short GetDirName(UPtr volName,short vRef, long dirId,UPtr name)
{
	CInfoPBRec catBlock;	/* to get the name of a directory */
	short err;
	short oldVol;
	
	GetVol(nil,&oldVol);
	if (err=SetVol(volName,vRef)) return(err);
	GetVol(nil,&vRef);
	
	*name = 0;
	
	/*
	 * get name of directory
	 */
	catBlock.dirInfo.ioDrDirID = dirId;
	catBlock.dirInfo.ioNamePtr = name;
	catBlock.dirInfo.ioVRefNum = vRef;
	catBlock.dirInfo.ioFDirIndex = -1;		/* use ioDirID */
	if ((err=PBGetCatInfo(&catBlock,0))!=noErr) /* get working directory info */
		*name = 0;
	SetVol(nil,oldVol);
	return(err);
}

/**********************************************************************
 * get a volume name, given a vRefNum
 **********************************************************************/
UPtr GetMyVolName(short refNum,UPtr name)
{
	VolumeParam myParam;
		
	*name = 0;		/* empty string */
	
	myParam.ioNamePtr = name;
	myParam.ioVRefNum = refNum;
	myParam.ioVolIndex = 0; 				/* use refnum only */
	PBGetVInfo(&myParam,0); 				/* get the volume info */

	return(name);
}

/**********************************************************************
 * find the DirID of the blessed (system) folder
 **********************************************************************/
int BlessedDirID(long *sysDirIDPtr)
{
	HVolumeParam myPB;
	SysEnvRec env;
	int err;

	/*
	 * does HFS exist?
	 */
	if (*(short *)FSFCBLen <= 0) return(-1);
	
	/*
	 * find the blessed dir
	 */
	SysEnvirons(ENVIRONS_VERSION,&env);
	myPB.ioNamePtr = nil;
	myPB.ioVRefNum = env.sysVRefNum;
	myPB.ioVolIndex = 0;
	if (err=PBHGetVInfo(&myPB,False)) return(err);
	if (myPB.ioVSigWord == 0xd2d7) return(myPB.ioVSigWord); /* MFS */
	*sysDirIDPtr = myPB.ioVFndrInfo[1];
	return(0);
}

/**********************************************************************
 * MakeResFile - create a resource file in a given directory
 **********************************************************************/
int MakeResFile(UPtr name,int vRef,long dirId,long creator,long type)
{
	int err;
	
	(void) HCreate(vRef,dirId,name,creator,type);
	(void) HSetVol(nil,vRef,dirId);
	CreateResFile(name);
	err=ResError();
	return (err=dupFNErr ? noErr : err);
}

/**********************************************************************
 * turn a DirID into a working directory reference number
 **********************************************************************/
short GetMyWD(short vRef,long dirID)
{
	WDPBRec myPB;
	short err;
	
	myPB.ioNamePtr = nil;
	myPB.ioVRefNum = vRef;
	myPB.ioWDProcID = nil;
	myPB.ioWDDirID = dirID;
	
	if (err=PBOpenWD(&myPB,0))
		return(0);
	
	return (myPB.ioVRefNum);
}

/**********************************************************************
 * DirIterate - iterate over the files in a directory.
 **********************************************************************/
short DirIterate(short vRef,long dirId,HFileInfo *hfi)
{
	hfi->ioFDirIndex++;
	hfi->ioDirID = dirId;
	hfi->ioVRefNum = vRef;
	return(PBGetCatInfo(hfi,False));
}

/**********************************************************************
 * CopyFBytes - copy bytes from one file to another
 **********************************************************************/
int CopyFBytes(short fromRefN,long fromOffset,long length,short toRefN,long toOffset)
{
	int err;
	UHandle buffer;
	long size;
	long count;
	
	if (err = SetFPos(fromRefN,fsFromStart,fromOffset)) return (err);
	if (err = SetFPos(toRefN,fsFromStart,toOffset)) return (err);
	
	size = OPTIMAL_BUFFER > length ? length : OPTIMAL_BUFFER;
	if ((buffer=NuHandle(size))==nil) return(MemError());
	
	LDRef(buffer);
	do
	{
		count = size > length ? length : size;
		if (err = FSRead(fromRefN,&count,*buffer)) break;
		if (err = FSWrite(toRefN,&count,*buffer)) break;
		length -= count;
	}
	while (length);
	DisposHandle(buffer);
	return(err);
}

/************************************************************************
 * MySFGetFile - get a file with SFGetFile
 * Returns True if a file was got.
 ************************************************************************/
Boolean MySFGetFile(ProcPtr filter, long *dirIdPtr, short *vRefPtr, UPtr namePtr,short typeCount,SFTypeList tl)
{
	SFReply sfr;
	Str255 junk;
	Point where;
	
	StdFileSpot(&where,SFGETFILE_ID);
	*junk = 0;
	SFPGetFile(where,junk,filter,typeCount,tl,nil,&sfr,SFGETFILE_ID,DlgFilter);
	
	if (!sfr.good) return(False);
	BlockMove(sfr.fName,namePtr,*sfr.fName+1);
	*dirIdPtr = GetMyDirID(sfr.vRefNum);
	*vRefPtr = sfr.vRefNum;
	return(True);
}

/************************************************************************
 * StdFilespot - figure out where a stdfile dialog should go
 ************************************************************************/
void StdFileSpot(Point *where, short id)
{
	Rect r,in;
	DialogTHndl dTempl; 			
	if ((dTempl=(DialogTHndl)GetResource('ALRT',id)) ||
			(dTempl=(DialogTHndl)GetResource('DLOG',id)))
	{
		r = (*dTempl)->boundsRect;
		in = qd.screenBits.bounds;
		in.top += GetMBarHeight();
		ThirdCenterRectIn(&r,&in);
		where->h = r.left;
		where->v = r.top;
	}
	else
	{
		where->h = 100;
		where->v = 100;
	}
}

/************************************************************************
 * FSHOpen - like FSOpen, but takes a dirId and permissions, too.
 ************************************************************************/
short FSHOpen(UPtr name,short vRefN,long dirId,short *refN,short perm)
{
	HIOParam pb;
	int err;
	Str255 newName;
	
	PCopy(newName,name);
	if (err=MyResolveAlias(&vRefN,&dirId,newName,nil)) return(err);
	pb.ioNamePtr = newName;
	pb.ioVRefNum = vRefN;
	pb.ioPermssn = perm;
	pb.ioMisc = nil;
	((HFileParam *)&pb)->ioDirID = dirId;
	err = PBHOpen(&pb,False);
	if (!err) *refN = pb.ioRefNum;
	return(err);
}

/************************************************************************
 * RFHOpen - like RFOpen, but with dirId and permissions
 ************************************************************************/
short RFHOpen(UPtr name,short vRefN,long dirId,short *refN,short perm)
{
	HIOParam pb;
	int err;
	
	pb.ioCompletion = nil;
	pb.ioNamePtr = name;
	pb.ioVRefNum = vRefN;
	pb.ioVersNum = 0;
	pb.ioPermssn = perm;
	pb.ioMisc = nil;
	((HFileParam *)&pb)->ioDirID = dirId;
	err = PBHOpenRF(&pb,False);
	if (!err) *refN = pb.ioRefNum;
	return(err);
}

/************************************************************************
 * MyAllocate - do a PBAllocate call
 ************************************************************************/
int MyAllocate(short refN,long size)
{
	IOParam pb;
	
	pb.ioCompletion = nil;
	pb.ioRefNum = refN;
	pb.ioReqCount = size;
	return(PBAllocate(&pb,False));
}

/************************************************************************
 * SFPutOpen - open a file for write, using stdfile
 ************************************************************************/
short SFPutOpen(UPtr name,short *vRef,long creator,long type,short *refN,ProcPtr filter,short id)
{
	Str255 scratch;
	Point where;
	SFReply sfr;
	short err;
	
	/*
	 * do the standard file stuff
	 */
	if (!MommyMommy(ATTENTION,nil)) return(1);
	StdFileSpot(&where,SFPUTFILE_ID);
	GetRString(scratch,SAVEAS_PROMPT);
	SFPPutFile(where,scratch,name,nil,&sfr,id,filter);
	if (!sfr.good) return(1);
	BlockMove(sfr.fName,name,*sfr.fName+1);
	*vRef = sfr.vRefNum;
	
	/*
	 * create && open the file
	 */
	if (err=MakeResFile(name,*vRef,0,creator,type))
	{
		FileSystemError(COULDNT_SAVEAS,name,err);
		return(err);
	}

	if (err=FSOpen(name,*vRef,refN))
	{
		FileSystemError(COULDNT_SAVEAS,name,err);
		FSDelete(name,*vRef);
		return(err);
	}

	if (err=SetEOF(*refN,0))
	{
		FileSystemError(COULDNT_SAVEAS,name,err);
		FSDelete(name,*vRef);
		return(err);
	}
	
	return(noErr);
}

/************************************************************************
 * IsText - is a file of type TEXT or not?
 ************************************************************************/
Boolean IsText(UPtr volName,long dirId,UPtr name)
{
	FInfo info;
	Str255 scratch;
	short oldVol;
	short newVol;
	
	GetVol(scratch,&oldVol);
	SetVol(volName,0);
	GetVol(scratch,&newVol);
	
	/*
	 * find the file, and get info
	 */
	HGetFInfo(newVol,dirId,name,&info);
	
	SetVol(nil,oldVol);
	return(info.fdType=='TEXT');	
}

/************************************************************************
 * SpinOn - spin until a return code is not inProgress or cacheFault
 ************************************************************************/
short SpinOn(short *rtnCodeAddr,long maxTicks)
{
	long ticks=TickCount();
	long startTicks=ticks+120;
	long now;
	EventRecord event;
	extern ConnHandle CnH;
	Boolean oldCommandPeriod = CommandPeriod;
	
	CommandPeriod = False;
	do
	{
		now = TickCount();
		if (now>startTicks && now-ticks>10) {CyclePendulum();ticks=now;}
		if (WNE(everyEvent,&event,0))
		{
			(void) MiniMainLoop(&event);
			if (CommandPeriod) return(userCancelled);
		}
		if (!UseCTB) ClearICMP();
		if (CnH) MyCMIdle();
		if (maxTicks && startTicks+maxTicks < now+120) break;
	}
	while (*rtnCodeAddr == inProgress || *rtnCodeAddr == cacheFault);
	CommandPeriod = oldCommandPeriod;
	return(*rtnCodeAddr);
}

/************************************************************************
 * IsItAFolder - is the specified file a folder?
 ************************************************************************/
Boolean IsItAFolder(short vRef,long inDirId,UPtr name)
{
	HFileInfo hfi;
	short err;
	
	hfi.ioCompletion=nil;
	hfi.ioNamePtr=name;
	hfi.ioVRefNum=vRef;
	hfi.ioDirID=inDirId;
	hfi.ioFDirIndex=0;
	if (err=PBGetCatInfo(&hfi,False)) return(err);
	return(0!=(hfi.ioFlAttrib&0x10));
}

/************************************************************************
 * FolderFileCount - count the files in a folder
 ************************************************************************/
short FolderFileCount(long inDirId,UPtr name)
{
	HFileInfo hfi;
	short err;
	
	FILL(hfi,name,MyVRef,inDirId);
	hfi.ioFDirIndex=0;
	if (err=PBGetCatInfo(&hfi,False)) return(-1);
	return(hfi.ioFlStBlk);
}

/************************************************************************
 * Move - Move a file or directory
 ************************************************************************/
short HMove(short vRef,long fromDirId,UPtr fromName,long toDirId,UPtr toName)
{
	CMovePBRec pb;
	
	pb.ioNamePtr = fromName;
	pb.ioDirID = fromDirId;
	pb.ioNewDirID = toDirId;
	pb.ioNewName = toName;
	pb.ioVRefNum = vRef;
				
	return(PBCatMove(&pb,False));
}


short HGetFileInfo(short vRef,long dirId,UPtr name,HFileInfo *hfi)
{
	short err;
	Str255 newName;
	
	PCopy(newName,name);
	if (err=MyResolveAlias(&vRef,&dirId,newName,nil)) return(err);
	WriteZero(hfi,sizeof(*hfi));
	FILL(*hfi,newName,vRef,dirId);
	return(PBHGetFInfo(hfi,False));
}

short HSetFileInfo(short vRef,long dirId,UPtr name,HFileInfo *hfi)
{
	short err;
	Str255 newName;
	
	PCopy(newName,name);
	if (err=MyResolveAlias(&vRef,&dirId,newName,nil)) return(err);
	FILL(*hfi,newName,vRef,dirId);
				return(PBHSetFInfo(hfi,False));
}

/************************************************************************
 * I am indebted to Tim Maroney (tim@toad.com) for the following routines.
 ************************************************************************/
static Boolean good, noSys, needWrite, allowFloppy, allowDesktop;
static SFReply reply;

pascal Boolean
FolderFilter(FileParam *pb);
pascal Boolean
FolderFilter(FileParam *pb)
{
#pragma unused(pb)
				return true;
}

pascal short
FolderItems(short item,DialogPtr dlog);
pascal short
FolderItems(short item,DialogPtr dlog)
{
#pragma unused(dlog)
				if (item == 2) {
								good = true;
								item = 3;
				}
				return item;
}

pascal void
FolderEvents(DialogPtr dialog, EventRecord *event,short *item);
pascal void
FolderEvents(DialogPtr dialog, EventRecord *event,short *item)
{
#pragma unused(event,item)
				ControlHandle ch;
				short type;
				Rect r;
				HVolumeParam vp;
				
				/* disable if a directory is selected in the list */
				GetDItem(dialog, 2, &type, &ch, &r);
#ifdef BOY_DO_PEOPLE_HATE_THIS	/* scd, 2/15/90 */
				if (reply.fType) {
								HiliteControl(ch, 255);
								return;
				}
#endif

				/* get information on the volume */
				vp.ioNamePtr = (StringPtr)0;
				vp.ioVRefNum = -*(short *)SFSaveDisk;
				vp.ioVolIndex = 0;
				if (PBHGetVInfo(&vp, false))
								HiliteControl(ch, 255);
				else if (vp.ioVSigWord != 0x4244) 									/* HFS? */
								HiliteControl(ch, 255);
				else if (vp.ioVDRefNum >= 0 || vp.ioVDrvInfo == 0)	/* ejected? */
								HiliteControl(ch, 255);
				else if (needWrite && (vp.ioVAtrb & 0x8080))				/* locked? */
								HiliteControl(ch, 255);
				else if (!allowFloppy && vp.ioVDRefNum == -5) 			/* floppy? */
								HiliteControl(ch, 255);
				else if (!allowDesktop && *(long *)CurDirStore == 2)				/* desktop? */
								HiliteControl(ch, 255);
				else if (noSys && *(long *)CurDirStore == vp.ioVFndrInfo[0]) /* blessed? */
								HiliteControl(ch, 255);
				else		HiliteControl(ch, 0);
}

Boolean
GetFolder(char *name,short *volume,long *folder,Boolean writeable,Boolean system,Boolean floppy,Boolean desktop)
{
				short oldvol = -*(short *)SFSaveDisk;
				long oldfolder = *(long *)CurDirStore;
				Point where;
				StdFileSpot(&where,GETFOLDER_DLOG); /* sd 1/91 */
				good = false;
				if (*volume && *folder) {
								*(short *)SFSaveDisk = -*volume;
								*(long *)CurDirStore = *folder;
				}
				needWrite = writeable, noSys = !system, allowFloppy = floppy;
				allowDesktop = desktop;
				SFPGetFile(where, (char *)0, FolderFilter, -1, 0, FolderItems,
									 &reply, GETFOLDER_DLOG, FolderEvents);
				if (!good) return false;
				*volume = -*(short *)SFSaveDisk;
				*folder = *(long *)CurDirStore;
				/*FullFileName(name, "", -*(short *)SFSaveDisk, *(long *)CurDirStore); sd 1/91 */
				GetMyVolName(*volume,name); 		/* sd 1/91 */
				*(short *)SFSaveDisk = -oldvol;
				*(long *)CurDirStore = oldfolder;
				return true;
}

/************************************************************************
 * CopyRFork - copy the resource fork from one file to another
 ************************************************************************/
short CopyRFork(short vRef,long dirId,UPtr name,short fromVRef,
								short fromDirId,Uptr fromName)
{
	short err;
	short fromRef,toRef;
	UPtr buffer=NuPtr(OPTIMAL_BUFFER);
	long bSize=OPTIMAL_BUFFER;
	long eof=0;
	
	if (!buffer) return(MemError());
	if (!(err=RFHOpen(fromName,fromVRef,fromDirId,&fromRef,fsRdPerm)))
	{
		if (!(err=RFHOpen(name,vRef,dirId,&toRef,fsRdWrPerm)))
		{
			GetEOF(fromRef,&eof);
			for (bSize=MIN(OPTIMAL_BUFFER,eof);!err&&eof;bSize=(OPTIMAL_BUFFER,eof))
			{
				if (!(err=FSRead(fromRef,&bSize,buffer)))
				{
					eof -= bSize;
					err = FSWrite(toRef,&bSize,buffer);
				}
			}
			FSClose(toRef);
		}
		FSClose(fromRef);
	}
	DisposPtr(buffer);
	return(err);	
}

/************************************************************************
 * CopyFInfo - copy the file info from one file to another
 ************************************************************************/
short CopyFInfo(short vRef,long dirId,UPtr name,short fromVRef,
								short fromDirId,Uptr fromName)
{
	short err;
	FInfo info;
	if (!(err=HGetFInfo(fromVRef,fromDirId,fromName,&info)))
		err=HSetFInfo(vRef,dirId,name,&info);
	return(err);
}

/************************************************************************
 * MyResolveAlias - resolve an alias
 ************************************************************************/
short MyResolveAlias(short *vRef,long *dirId,UPtr name,Boolean *wasAlias)
{
	FSSpec theSpec;
	Boolean folder;
	long haveAlias;
	short err=noErr;
	Boolean wasIt;
	
	if (wasAlias) *wasAlias = False;
	if (!Gestalt(gestaltAliasMgrAttr,&haveAlias) && haveAlias&0x1)
	{
		if (!(err=FSMakeFSSpec(*vRef,*dirId,name,&theSpec)) &&
				!(err=ResolveAliasFile(&theSpec,True,&folder,&wasIt)))
		{
			if (wasIt)
			{
				*vRef = theSpec.vRefNum;
				*dirId = theSpec.parID;
				PCopy(name,theSpec.name);
				name[*name+1] = 0;
				if (wasAlias) *wasAlias = True;
			}
		}
	}
	return(err);
}

/************************************************************************
 * FSWriteP - write a Pascal string
 ************************************************************************/
short FSWriteP(short refN,UPtr pString)
{
	long count = *pString;
	return(FSWrite(refN,&count,pString+1));
}


/************************************************************************
 * GetFileByRef - figure out the name & vol of a file from an open file
 ************************************************************************/
short GetFileByRef(short refN,short *vRef,long *dirId,UPtr name)
{
	FCBPBRec fcb;
	short err;
	
	fcb.ioCompletion = nil;
	fcb.ioVRefNum = nil;
	fcb.ioRefNum = refN;
	fcb.ioFCBIndx = 0;
	fcb.ioNamePtr = name;
	if (err=PBGetFCBInfo(&fcb,False)) return(err);
	
	*vRef = fcb.ioFCBVRefNum;
	*dirId = fcb.ioFCBParID;
	return(noErr);
}

/************************************************************************
 * VolumeFree - return the free space on a volume
 ************************************************************************/
long VolumeFree(short vRef)
{
	Str31 name;
	long vFree;
	
	*name = 0;
	if (GetVInfo(0,name,&vRef,&vFree)) return(0);
	return(vFree);
}

/************************************************************************
 * FSTabWrite - write, expanding tabs
 ************************************************************************/
short FSTabWrite(short refN,long *count,UPtr buf)
{
	UPtr p;
	UPtr end = buf+*count;
	long written=0;
	short err=noErr;
	long writing;
	static short charsOnLine=0;
	UPtr nl;
	short stops=0;
	
	if (!FakeTabs) return(FSZWrite(refN,count,buf));
	for (p=buf;p<end;p=buf=p+1)
	{
		nl = buf-charsOnLine-1;
		while (p<end && *p!=tabChar)
		{
			if (*p=='\n') nl=p;
			p++;
		}
		writing = p-buf;
		err=FSZWrite(refN,&writing,buf);
		written += writing;
		if (err) break;
		charsOnLine = p-nl-1;
		if (p<end)
		{
			if (!stops) stops = GetRLong(TAB_DISTANCE);
			writing = stops-(charsOnLine)%stops;
			charsOnLine = 0;
			err = FSZWrite(refN,&writing,"              ");
			written += writing;
			if (err) break;
		}
	}
	*count = written;
	return(err);
}