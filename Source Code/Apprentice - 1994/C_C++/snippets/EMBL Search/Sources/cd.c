/*
*********************************************************************
*	
*	cd.c
*	Initialization of CD-ROM
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*
*/

#include <Packages.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"

/*
******************************* Prototypes ***************************
*/

#include "cd.h"
#include "pstr.h"
#include "util.h"
#include "window.h"

static short GetWDFromPath(StringPtr dirName, short vRefNum, StringPtr volName);
static short GetWDFromSpec(DirSpec *dirSpec);
static Boolean ProcessDBFiles(short dbcode);
static Boolean CheckHeader(StringPtr fName, short fd, short dbcode, long rec_size, u_long *nrec, u_short *recsize);

/*
******************************** Global variables *****************
*/

extern VolInfo		gCurrentCD;
extern DBInfo		gDBInfo[DB_NUM];
extern IndexFiles	gFileList;
extern Str255		gVolLabel;
extern char			gError[256];
extern Prefs		gPrefs;

/**************************************
*	Startup check for an attached EMBL CD-ROM.
*	We compare the volume name to the value stored in a STR# resource
*	(variable gVolLabel).
*/

void InitCD()
{
	register short	i;
	HParamBlockRec	myPB;
	Str255			volName;
	Boolean			foundCD=FALSE;
	OSErr				err;
	DialogPtr		myDialog;
	
	*gCurrentCD.volName=EOS;			/* clear volume name. A valid volume name
													indicates that we�ve found a CD-ROM		*/
	
	/* now check every attached device */
	myPB.volumeParam.ioNamePtr=volName;
	for(i=1,err=noErr;err == noErr;++i) {
		myPB.volumeParam.ioVolIndex=i;
		err=PBHGetVInfo(&myPB,FALSE);					/* get volume information		*/
		if(err == noErr) {								/* error or no more volumes	*/
			/* Check volume name */
			if(!pstrncmp(volName,gVolLabel,*gVolLabel)) {
				/* it looks like an EMBL CD-ROM, now try to initialize it */
				StartWaitCursor();
				CenterDA('DLOG',INITWAIT_DLG,50);
				ShowWindow(myDialog=GetNewDialog(INITWAIT_DLG,NULL,(WindowPtr)-1));
				DrawDialog(myDialog);
				foundCD=InitDB(myPB.volumeParam.ioVRefNum,volName);
				DisposDialog(myDialog);
				InitCursor();
				if( foundCD == TRUE )
					break;
			}
		}
	}
	
	/* In case no EMBL CD-ROM is attached or something went wrong, we exit */
	
	if(! foundCD)
		FatalErrorMsg(ERR_NOCD);
}


/**************************************
*	Check files on CD for correctness
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

Boolean InitDB(short vRefNum,StringPtr volName)
{
	Str255 name;

	/* create working directory for EMBL sequence directory */
	pstrcpy(name,gDBInfo[DB_EMBL].SeqDName);
	if(!(gDBInfo[DB_EMBL].SeqWDRefNum  = GetWDFromPath(name,vRefNum,volName)))
		return(FALSE);
		
	/* create working directory for Swissprot sequence directory */
	pstrcpy(name,gDBInfo[DB_SWISS].SeqDName);
	if(!(gDBInfo[DB_SWISS].SeqWDRefNum = GetWDFromPath(name,vRefNum,volName)))
		return(FALSE);
		
	/* Try to create working directory for EMBL index directory according to specs
		stored in Prefs file */
	if(*gPrefs.inxDirSpec[DB_EMBL].volName != EOS) {
		if(!(gDBInfo[DB_EMBL].InxWDRefNum  = GetWDFromSpec(&gPrefs.inxDirSpec[DB_EMBL]))) {
			sprintf(gError,LoadErrorStr(ERR_PNF,FALSE),"EMBL");
			ErrorMsg(0);
			*gPrefs.inxDirSpec[DB_EMBL].volName = EOS;	/* reset name on error */
		}
	}
	
	/* create working directory for EMBL index directory */
	if(*gPrefs.inxDirSpec[DB_EMBL].volName == EOS) {
		pstrcpy(name,volName);
		pstrcat(name,"\p:");
		pstrcat(name,gDBInfo[DB_EMBL].InxDName);
		if(!(gDBInfo[DB_EMBL].InxWDRefNum  = GetWDFromPath(name,vRefNum,volName)))
			return(FALSE);
	}
	
	/* Try to create working directory for Swissprot index directory according to specs
		stored in Prefs file */
	if(*gPrefs.inxDirSpec[DB_SWISS].volName != EOS) {
		if(!(gDBInfo[DB_SWISS].InxWDRefNum  = GetWDFromSpec(&gPrefs.inxDirSpec[DB_SWISS]))) {
			sprintf(gError,LoadErrorStr(ERR_PNF,FALSE),"SWISS");
			ErrorMsg(0);
			*gPrefs.inxDirSpec[DB_SWISS].volName = EOS;	/* reset name on error */
		}
	}
	
	/* create working directory for Swissprot index directory */
	if(*gPrefs.inxDirSpec[DB_SWISS].volName == EOS) {
		pstrcpy(name,volName);
		pstrcat(name,"\p:");
		pstrcat(name,gDBInfo[DB_SWISS].InxDName);
		if(!(gDBInfo[DB_SWISS].InxWDRefNum = GetWDFromPath(name,vRefNum,volName)))
			return(FALSE);
	}

	/* now check individual files */
	if( !ProcessDBFiles(DB_EMBL) || !ProcessDBFiles(DB_SWISS) )
		return(FALSE);

	/* Fill gCurrentCd and thus make current CD valid */
	pstrcpy(gCurrentCD.volName,volName);
	gCurrentCD.vRefNum=vRefNum;
	return(TRUE);
}


/**************************************
*	Create a working directory from a full path specification
*	Returns working directory refnum, or 0 if an error occurred
*/

static short GetWDFromPath(StringPtr dirName,short vRefNum,StringPtr volName)
{
	CInfoPBRec	myPB;
	OSErr			err;
	short			wdRefNum;

	/* Get DirID */
	myPB.dirInfo.ioNamePtr=dirName;
	myPB.dirInfo.ioVRefNum=vRefNum;
	myPB.dirInfo.ioFDirIndex=0;
	myPB.dirInfo.ioDrDirID=0;
	if((err = PBGetCatInfo(&myPB,FALSE)) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_NOCDDIR,FALSE),
					PtoCstr(dirName), PtoCstr(volName));
		CtoPstr((char *)volName);
		return(ErrorMsg(0));
	}
	
	/* Create a working directory */
	if( (err=OpenWD(vRefNum,myPB.dirInfo.ioDrDirID,kApplSignature,&wdRefNum))
			!= noErr) {
		sprintf(gError,LoadErrorStr(ERR_CREATEWD,FALSE),err );
		return(ErrorMsg(0));
	}
	
	return(wdRefNum);
}

/**************************************
*	Create a working directory from a DirSpec
*	Returns working directory refnum, or 0 if an error occurred
*/

static short GetWDFromSpec(DirSpec *dirSpec)
{
	OSErr			err;
	short			wdRefNum;
	short			vRefNum;
	
	if(!GetVRefNumFromName(dirSpec->volName,&vRefNum))
		return(0);
			
	/* Create a working directory */
	if( (err=OpenWD(vRefNum,dirSpec->dirID,kApplSignature,&wdRefNum)) != noErr) {
		sprintf(gError,LoadErrorStr(ERR_CREATEWD,FALSE),err );
		return((short)ErrorMsg(0));
	}
	
	return(wdRefNum);
}

/**************************************
*	Analyse index files on CD-ROM
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*/

static Boolean ProcessDBFiles(short dbcode)
{
	Str255			fName;
	short				fd;
	short				i;
	Header			hdr;
	DivisionRec		divRec;
	long				size;
	unsigned long	secs;
	DateTimeRec		date;
	Boolean			ret;
	SignedByte		oldHState;
	u_long			temp;
	u_short			recsize;
	short				rel;
	
	/* Process division information file */
	pstrcpy(fName,gFileList.divFName);
	if( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd, TRUE) != noErr )
		return(FALSE);
	
	/* check header */
	size=sizeof(Header);
	if ( ReadMacFile(fd,&size,&hdr,fName, TRUE) ) {
		FSClose(fd);
		return(FALSE);
	}

	/* store db name and release number */
	strncpy(gDBInfo[dbcode].DBName,hdr.db_name,DBNAMELEN);
	gDBInfo[dbcode].DBName[DBNAMELEN]=EOS;
	rtrim(gDBInfo[dbcode].DBName);

	strncpy(gDBInfo[dbcode].DBRelNum,hdr.db_relnum,DBRELNUMLEN);
	gDBInfo[dbcode].DBRelNum[DBRELNUMLEN]=EOS;
	rtrim(gDBInfo[dbcode].DBRelNum);
	
	/* check for compatibility of software with CD-ROM
	if(dbcode == DB_EMBL) {
		rel=atoi( gDBInfo[dbcode].DBRelNum );
		if (rel < 29) {
			sprintf(gError,LoadErrorStr(ERR_TOOOLD,FALSE));
			return(ErrorMsg(0));
		}
	}
	
	/* get release date and convert it to a string */
	date.year=1900+hdr.db_reldate.year;
	date.month=hdr.db_reldate.month;
	date.day=hdr.db_reldate.day;
	date.hour=date.minute=date.second=0;
	Date2Secs(&date,&secs);
	IUDateString(secs,shortDate,(StringPtr)gDBInfo[dbcode].DBRelDate);
	PtoCstr((StringPtr)gDBInfo[dbcode].DBRelDate);
	
	/* check record size */
	if(ConvertShort(&hdr.record_size) != sizeof(DivisionRec)){
		FSClose(fd);
		sprintf(gError,LoadErrorStr(ERR_INVRECSIZE,FALSE),PtoCstr(fName));
		CtoPstr((char *)fName);
		return(ErrorMsg(0));
	}
	
	/* allocate memory for division information */ 
	gDBInfo[dbcode].ndiv = (short)ConvertLong(&hdr.nrecords);
	size= (sizeof(Str255) * (gDBInfo[dbcode].ndiv+1));
	if( !(gDBInfo[dbcode].gDivNames = 
		(char **)NewHandleClear((DIVNAMELEN+1) * (gDBInfo[dbcode].ndiv+1))) ) {
		FSClose(fd);
		return(ErrorMsg(ERR_MEMORY));
	}
	
	/* read sequence file names and division codes*/
	oldHState = LockHandleHigh(gDBInfo[dbcode].gDivNames);
	for(i=1;i<=gDBInfo[dbcode].ndiv;++i) {
		size=sizeof(DivisionRec);
		if( ReadMacFile(fd,&size,&divRec,fName,TRUE) ) {
			FSClose(fd);
			DisposHandle((Handle)gDBInfo[dbcode].gDivNames);
			gDBInfo[dbcode].gDivNames = NULL;
			return(FALSE);
		}
		if(ConvertShort(&divRec.div_code) != (u_short)i) {
			FSClose(fd);
			DisposHandle((Handle)gDBInfo[dbcode].gDivNames);
			gDBInfo[dbcode].gDivNames = NULL;
			sprintf(gError,LoadErrorStr(ERR_DIVCODEORDER,FALSE),PtoCstr(fName));
			CtoPstr((char *)fName);
			return(ErrorMsg(0));
		}
		else {
			strncpy( *gDBInfo[dbcode].gDivNames + i*(DIVNAMELEN+1),divRec.filename,
						DIVNAMELEN);
			(*gDBInfo[dbcode].gDivNames + i*(DIVNAMELEN+1))[DIVNAMELEN]=EOS;
			rtrim(*gDBInfo[dbcode].gDivNames + i*(DIVNAMELEN+1));
			strcat(*gDBInfo[dbcode].gDivNames + i*(DIVNAMELEN+1),";1");
			CtoPstr(*gDBInfo[dbcode].gDivNames + i*(DIVNAMELEN+1));
		}
	}
	HSetState((Handle)gDBInfo[dbcode].gDivNames,oldHState);
	FSClose(fd);
	
	/* Check short description file */
	pstrcpy(fName,gFileList.briefIdxFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum, &fd, TRUE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(DescRec),&temp,&recsize);
		FSClose(fd);
	}
	if(ret == FALSE) return(FALSE);

	/* Check entry name index */
	pstrcpy(fName,gFileList.enameIdxFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,TRUE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(EnameRec),
								&gDBInfo[dbcode].ename_nrec, &recsize);
		FSClose(fd);
	}
	if(ret == FALSE) return(FALSE);
	
	/* Compare nrec of entryname and shortdir indexes */
	if (gDBInfo[dbcode].ename_nrec != temp)
		return(ErrorMsg(ERR_WRONGNREC));
	
	/* Check accession number hit file */
	pstrcpy(fName,gFileList.acnumHitFName);
	if ((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,TRUE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(u_long),&gDBInfo[dbcode].achit_nrec,
								&recsize);
		FSClose(fd);
	}
	if(ret == FALSE) return(FALSE);

	/* Check accession number target list */
	pstrcpy(fName,gFileList.acnumTrgFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,TRUE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,0L,&gDBInfo[dbcode].actrg_nrec,&recsize);
		FSClose(fd);
	}
	if(ret)
		gDBInfo[dbcode].actrg_recsize=recsize;
	else
		return(FALSE);
		
	/* Check keyword hit file */
	pstrcpy(fName,gFileList.keywHitFName);
	if ((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,FALSE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(u_long),
								&gDBInfo[dbcode].kwhit_nrec, &recsize);
		FSClose(fd);
	}
	if(ret == FALSE) 
		gDBInfo[dbcode].kwtrg_recsize=0;
	else {
		/* Check keyword target list */
		pstrcpy(fName,gFileList.keywTrgFName);
		if((ret=(OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,FALSE) == noErr))) {
			ret = CheckHeader(fName,fd,dbcode,0L,&gDBInfo[dbcode].kwtrg_nrec,&recsize);
			FSClose(fd);
		}
		if(ret == FALSE)
			gDBInfo[dbcode].kwtrg_recsize=0;
		else
			gDBInfo[dbcode].kwtrg_recsize=recsize;
	}

	/* Check free text hit file */
	pstrcpy(fName,gFileList.textHitFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,FALSE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(u_long),
								&gDBInfo[dbcode].texthit_nrec, &recsize);
		FSClose(fd);
	}
	if(ret == FALSE)	/* no error, we simply disable free text queries
									(might be a pre-Rel.29 CD-ROM */
		gDBInfo[dbcode].texttrg_recsize=0;
	else {
		/* Check free text target list */
		pstrcpy(fName,gFileList.textTrgFName);
		if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd, FALSE) == noErr))) {
			ret = CheckHeader(fName,fd,dbcode,0L,&gDBInfo[dbcode].texttrg_nrec,
									&recsize);
			FSClose(fd);
		} 
		if(ret == FALSE)	/* no error, we simply disable free text queries
									(might be a pre-Rel.29 CD-ROM */
			gDBInfo[dbcode].texttrg_recsize=0;
		else
			gDBInfo[dbcode].texttrg_recsize=recsize;
	}
	
	/* Check author hit file */
	pstrcpy(fName,gFileList.authorHitFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,FALSE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(u_long),
								&gDBInfo[dbcode].authorhit_nrec, &recsize);
		FSClose(fd);
	}
	if(ret == FALSE)	/* no error, we simply disable author queries
									(might be a pre-Rel.30 CD-ROM */
		gDBInfo[dbcode].authortrg_recsize=0;
	else {
		/* Check author target list */
		pstrcpy(fName,gFileList.authorTrgFName);
		if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd, FALSE) == noErr))) {
			ret = CheckHeader(fName,fd,dbcode,0L,&gDBInfo[dbcode].authortrg_nrec,
									&recsize);
			FSClose(fd);
		} 
		if(ret == FALSE)	/* no error, we simply disable author queries
									(might be a pre-Rel.30 CD-ROM */
			gDBInfo[dbcode].authortrg_recsize=0;
		else
			gDBInfo[dbcode].authortrg_recsize=recsize;
	}
	
	/* Check taxonomy hit file */
	pstrcpy(fName,gFileList.taxonHitFName);
	if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd,FALSE) == noErr))) {
		ret = CheckHeader(fName,fd,dbcode,sizeof(u_long),
								&gDBInfo[dbcode].taxonhit_nrec, &recsize);
		FSClose(fd);
	}
	if(ret == FALSE)	/* no error, we simply disable taxonomy queries
									(might be a pre-Rel.30 CD-ROM */
		gDBInfo[dbcode].taxontrg_recsize=0;
	else {
		/* Check taxonomy target list */
		pstrcpy(fName,gFileList.taxonTrgFName);
		if((ret=( OpenMacFileReadOnly(fName,gDBInfo[dbcode].InxWDRefNum,&fd, FALSE) == noErr))) {
			ret = CheckHeader(fName,fd,dbcode,0L,&gDBInfo[dbcode].taxontrg_nrec,
									&recsize);
			FSClose(fd);
		} 
		if(ret == FALSE)	/* no error, we simply disable taxonomy queries
									(might be a pre-Rel.30 CD-ROM */
			gDBInfo[dbcode].taxontrg_recsize=0;
		else
			gDBInfo[dbcode].taxontrg_recsize=recsize;
	}

	return(TRUE);
}


/**************************************
*	Check index file header
*	Return value:	TRUE, if successful
*						FALSE, if an error occurred
*	Side-effect:	*nrec contains number of records, *recsize the size of a record
*/

static Boolean CheckHeader(StringPtr fName, short fd, short dbcode,
									long rec_size, u_long *nrec, u_short *recsize)
{
	long		size;
	Header	hdr;
	char		dbname[DBNAMELEN+1];
	char		dbrelnum[DBRELNUMLEN+1];
	
	*nrec=0L;
	
	/* rewind file */
	if(SetFPos(fd,fsFromStart,0L) != noErr)
		return(FALSE);
	
	/* read header */
	size=sizeof(Header);
	if ( ReadMacFile(fd,&size,&hdr,fName,TRUE) )
		return(FALSE);

	/* check record size; for generic record structures we pass rec_size of 0 and
		do not check */
	if(rec_size != 0L) {
		if(ConvertShort(&hdr.record_size) != rec_size){
			sprintf(gError,LoadErrorStr(ERR_INVRECSIZE,FALSE),fName);
			return(ErrorMsg(0));
		}
	}

	/* check dbcode name and version (we don't check the date) */
	strncpy(dbname,hdr.db_name,DBNAMELEN);
	dbname[DBNAMELEN]=EOS;
	rtrim(dbname);
	
	strncpy(dbrelnum,hdr.db_relnum,DBRELNUMLEN);
	dbrelnum[DBRELNUMLEN]=EOS;
	rtrim(dbrelnum);
	
	if(strcmp(dbname,gDBInfo[dbcode].DBName) ||
		strcmp(dbrelnum,gDBInfo[dbcode].DBRelNum)) {
		sprintf(gError,LoadErrorStr(ERR_HEADINC,FALSE),fName);
		return(ErrorMsg(0));
	}	

	*nrec=ConvertLong(&hdr.nrecords);
	*recsize = ConvertShort(&hdr.record_size);
	return(TRUE);
}