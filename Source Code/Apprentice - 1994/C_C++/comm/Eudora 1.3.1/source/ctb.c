#define FILE_NUM 9
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions for i/o over a pseudo-telnet ctb stream
 * these functions are oriented toward CONVENIENCE, not performance
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment CTBTrans
#include <stdio.h>
static short	CTBErr;

/************************************************************************
 * private functions
 ************************************************************************/
Boolean CTBExists(void);
short ConfigureCTB(Boolean makeUser);
short CTBNavigateString(Uptr string);
short MyCMOpen(ConnHandle cn,long timeout);
short MyCMClose(ConnHandle cn);
short MyCMWrite(ConnHandle cn,UPtr buffer,long *size);
short MyCMBreak(ConnHandle cn,long howLong);
short MyCMRead(ConnHandle cn,UPtr buffer,long *size,long timeout);
short GetSecondPass(UPtr pass);
pascal void CTBComplete(ConnHandle cn);
long CTBDataCount(void);
typedef struct
{
	short pending;
	short errCode;
	CMBufferSizes asyncCount;
} AsyncStuff, *AsyncStuffPtr;
Boolean SilentCTB=False;
#define CTBTrouble(err) CTBTR(err,FILE_NUM,__LINE__)
short CTBTR(short err,short file,short line);
#define ESC_AUXUSR 1
#define ESC_AUXUSRERR 2
#define ESC_AUXPW 4
#define ESC_USR 8
#define ESC_PW 16
short EscapeCopy(PStr to,PStr from,PStr expect);

/************************************************************************
 * InitCTB - initialize the CommToolbox
 ************************************************************************/
Boolean InitCTB(Boolean makeUser)
{
	static int CTBInitted=0;
	
	CTBInitted = MIN(CTBInitted,4);
	CTBSilenceTrans(False);
	switch (CTBInitted)
	{
		case 0:
			if (!CTBExists())
			{
				WarnUser(NO_CTB,0);
				return(False);
			}
			CTBInitted++;
		case 1:
			if (CTBErr = InitCTBUtilities())
			{
				WarnUser(NO_CTBU,CTBErr);
				return(False);
			}
			CTBInitted++;
		case 2:
			if (CTBErr = InitCRM())
			{
				WarnUser(NO_CTBRM,CTBErr);
				return(False);
			}
			CTBInitted++;
		case 3:
			if (CTBErr = InitCM())
			{
				WarnUser(NO_CTBCM,CTBErr);
				return(False);
			}
			CTBInitted++;
		case 4:
			if (CTBErr = ConfigureCTB(makeUser))
				return(False);
			CTBInitted++;
	}
	return(True);
}

/************************************************************************
 * CTBExists - is the CTB installed?
 ************************************************************************/
Boolean CTBExists(void)
{
#define CTBTrap 0x8b
#define UnimplTrap 0x9f
	return(NGetTrapAddress(UnimplTrap,OSTrap)!=NGetTrapAddress(CTBTrap,OSTrap));
}

/************************************************************************
 * DialThePhone - create a CTB connection
 ************************************************************************/
short DialThePhone(void)
{
	Str255 scratch;
	
	Progress(NoChange,GetRString(scratch,INIT_CTB));
	if (!InitCTB(False)) return (CTBErr?CTBErr:cmGenericError);

	RcvSpot = -1;
	if ((RcvBuffer=NuHandle(GetRLong(RCV_BUFFER_SIZE)))==nil)
	{
		CTBErr=MemError();
		WarnUser(MEM_ERR,CTBErr);
		return(CTBErr);
	}
	Progress(NoChange,GetRString(scratch,MAKE_CONNECT));
	if (CTBErr = MyCMOpen(CnH,60*GetRLong(OPEN_TIMEOUT)))
	{
		CMDispose(CnH);
		CnH = nil;
		return(CTBErr);
	}
	return(CTBNavigateSTRN(NAVIN));
}
	
/************************************************************************
 * CTBConnectTrans - connect to the remote host.	This version uses the CTB.
 ************************************************************************/
int CTBConnectTrans(UPtr serverName, short port)
{
	Str127 scratch;
	
#ifdef DEBUG
	if (BUG12) port += 10000;
#endif
	ComposeRString(scratch,TS_CONNECT_FMT,serverName,(uLong) port);
	Progress(NoBar,scratch);
	CTBSendTrans(1,scratch+1,*scratch);
		
	return(CTBErr);
}

/************************************************************************
 * CTBSendTrans - send some text to the remote host.	This version uses the CTB.
 ************************************************************************/
int CTBSendTrans(short count, UPtr text,long size, ...)
{
	long bSize;
	if (size==0) return(noErr); 	/* allow vacuous sends */
	CycleBalls();
	
	bSize = size;
	if (CTBErr=MyCMWrite(CnH,text,&bSize))
		return(CTBErr);
	if (--count>0)
	{
		Uptr buffer;
		va_list extra_buffers;
		va_start(extra_buffers,size);
		while (count--)
		{
			CycleBalls();
			buffer = va_arg(extra_buffers,UPtr);
			bSize = va_arg(extra_buffers,int);
			if (CTBSendTrans(1,buffer,bSize)) break;
		}
		va_end(extra_buffers);
	}
	return(CTBErr);
}

/************************************************************************
 * GenSendWDS - send a lot of text to the remote host.
 ************************************************************************/
int GenSendWDS(wdsEntry *theWDS)
{
	short err=0;

	CycleBalls();
	for (;theWDS->length;theWDS++)
		if (err=SendTrans(1,theWDS->ptr,theWDS->length)) break;
	
	return(err);
}

/************************************************************************
 * CTBRecvTrans - get some text from the remote host.  This version uses the CTB.
 ************************************************************************/
int CTBRecvTrans(UPtr line,long *size)
{
	long gotThis;
	Str31 tmStr;
	short factor = 60;
	UPtr spot = line;
	long want = *size;
	
	NumToString(CTBTimeout,tmStr);
	MyCMIdle();
	do
	{
		CTBErr = SpinOn(&CTBHasChars,CTBTimeout*factor);
		if (CTBErr)
			CTBErr = cmTimeOut;
		else
		{
			factor = 10;
			gotThis = CTBDataCount();
			gotThis = MIN(gotThis,want);
			CTBErr=MyCMRead(CnH,line,&gotThis,0);
			if (!CTBErr) {spot += gotThis; want -= gotThis;}
		}
	}
	while (want && spot==line && !SilentCTB && !CommandPeriod &&
				 AlertStr(TIMEOUT_ALRT,Caution,tmStr)==1);

	*size = spot - line;
	if (!CTBErr && *size==0) CTBErr = cmTimeOut;
	return(CTBErr);
}

/************************************************************************
 *
 ************************************************************************/
void HangUpThePhone(void)
{
	if (CnH)
	{
		Str127 msg;
		(void) CTBNavigateSTRN(NAVOUT);
		Progress(NoChange,GetRString(msg,CTB_CLOSING));
		if (!PrefIsSet(PREF_NO_HANGUP)) (void) MyCMClose(CnH);
	}
	if (CnH)
	{
		CMDispose(CnH);
		CnH = nil;
	}
	if (RcvBuffer) ZapHandle(RcvBuffer);
}

/************************************************************************
 * CTBDisTrans - disconnect from the remote host.  This version uses the CTB.
 ************************************************************************/
int CTBDisTrans(void)
{
	return(noErr);
}

/************************************************************************
 * CTBDestroyTrans - destory the connection.
 ************************************************************************/
int CTBDestroyTrans(void)
{
	return(noErr);
}

/************************************************************************
 * CTBTransError - report our most recent error
 ************************************************************************/
int CTBTransError(void)
{
	return(CTBErr);
}

/************************************************************************
 * CTBSilenceTrans - turn off error reports from tcp routines
 ************************************************************************/
void CTBSilenceTrans(Boolean silence)
{
	SilentCTB = silence;
}

/************************************************************************
 * ConfigureCTB - configure the communications toolbox
 ************************************************************************/
short ConfigureCTB(Boolean makeUser)
{
	Str63 toolName;
	short procID;
	CMBufferSizes bs;
	Point pt;
	short result;
	Handle cfigH;
	Boolean needConfig = True;
	
	CTBErr = 0;
	
	/*
	 * find the tool to use
	 */
	if (!*GetRStr(toolName,CTB_TOOL_STR)) return(1);
	
	if ((procID=CMGetProcID(toolName))<0)
	{
		CRMGetIndToolName(classCM,1,toolName);
		if (!*toolName)
			return(WarnUser(NO_CTB_TOOLS,1));
		else if ((procID=CMGetProcID(toolName))<0)
			return(WarnUser(COULDNT_GET_TOOL,1));
	}
	
	/*
	 * create a connection record
	 */
	WriteZero(bs,sizeof(bs));
	if (!(CnH = CMNew(procID,
			cmNoMenus|(PrefIsSet(PREF_AUTO_DISMISS)?cmQuiet:0),bs,nil,nil)))
	{
		WarnUser(MEM_ERR,MemError());
		return(1);
	}
	
	/*
	 * try to load the old configuration
	 */
	if (cfigH = GetNamedResource(CTB_CFIG_TYPE,toolName))
	{
		if (GetPtrSize((*CnH)->config)==GetHandleSize(cfigH))
		{
			BlockMove(*cfigH,(*CnH)->config,GetPtrSize((*CnH)->config));
			BlockMove(*cfigH,(*CnH)->oldConfig,GetPtrSize((*CnH)->config));
			needConfig = CMValidate(CnH);
		}
		ReleaseResource(cfigH);
	}
	
	/*
	 * let the user have a look at things
	 */
	if (makeUser || needConfig)
	{
		pt.h = 10; pt.v = GetMBarHeight()+10;
		switch (result = CMChoose(&CnH,pt,nil))
		{
			case chooseDisaster:
			case chooseFailed:
			case chooseAborted:
				CTBErr = result;
				break;
			case chooseOKMajor:
			case chooseOKMinor:
				/* save the name string */
				CMGetToolName((*CnH)->procID,toolName);
				BlockMove((*CnH)->config,(*CnH)->oldConfig,GetPtrSize((*CnH)->config));
				if (CTBErr = SettingsPtr('STR ',nil,CTB_TOOL_STR,toolName,*toolName+1))
					WarnUser(WRITE_SETTINGS,CTBErr);
				else if (CTBErr = SettingsPtr(CTB_CFIG_TYPE,toolName,CTB_CFIG_ID,
																		(*CnH)->config,GetPtrSize((*CnH)->config)))
					WarnUser(WRITE_SETTINGS,CTBErr);
				break;
		}
	}
	
	/*
	 * just configuring?
	 */
	if (CnH)
		if (makeUser) {CMDispose(CnH); CnH = nil;}
	
	return(CTBErr);
}

/************************************************************************
 * CTBNavigateSTRN - navigate a serial connection by means of an STRN resource
 ************************************************************************/
short CTBNavigateSTRN(short id)
{
	short count,n;
	Str63 name;
	Uhandle strnH;
	Str255 string;
	ResType theType;
	Boolean oldSilence = SilentCTB;
	short err=noErr;
	
	GetRString(name,id);
	CMGetToolName((*CnH)->procID,string);
	PCatC(string,' ');
	PCat(string,name);
	strnH = GetNamedResource('STR#',string);
	if (!strnH)
		strnH = GetNamedResource('STR#',name);
	if (!strnH)
		strnH = GetResource('STR#',id==NAVIN ? CTB_NAV_IN_STRN :
			(id==NAVMID ? CTB_NAV_MID_STRN : CTB_NAV_OUT_STRN));
	
	if (strnH)
	{
		CTBTimeout = GetRLong(SHORT_TIMEOUT);
		count = (*strnH)[0]*256 + (*strnH)[1];
		GetResInfo(strnH,&id,&theType,name);
		ReleaseResource(strnH);
		ComposeLogR(LOG_NAV,nil,NAV_BEGIN,name,id,count);
		
		CTBSilenceTrans(True);
		for (n=1; n<=count; n++)
		{
			GetIndString(string,id,n);
			if (err=CTBNavigateString(string)) break;
		}
		CTBSilenceTrans(oldSilence);
		CTBTimeout = GetRLong(RECV_TIMEOUT);
		if (err!=cmTimeOut) CTBTrouble(err);
	}
		
	return(err==cmTimeOut ? noErr : err);
}

/************************************************************************
 * CTBNavigateString - navigate a serial connection by a string
 ************************************************************************/
short CTBNavigateString(Uptr string)
{
	Str255 data, scratch, expect;
	long bSize;
	long tics;
	UPtr exSpot,dSpot;
	Boolean found=False,brk=False,wait=False;
	short escFlags;
	
	escFlags = EscapeCopy(data,string,expect);
	for (exSpot=expect+*expect;exSpot>expect;exSpot--) *expect &= 0x7f;	/* strip parity */
	
	if (CommandPeriod) return(userCancelled);
	if (escFlags & ESC_AUXUSRERR)
	{
		WarnUser(NO_AUXUSR,0);
		return(userCancelled);
	}

	if (*data && data[1]==bulletChar)
	{
		Progress(NoChange,GetRString(scratch,SECRET));
		BlockMove(data+2,data+1,*data-1);
		--*data;
	}
	else if (*data && ((brk=data[1]==betaChar) || (wait=data[1]==deltaChar)))
	{
		BlockMove(data+2,data+1,*data);
		--*data;
		tics = atoi(data+1);
		if (wait) tics *= 60;
		Progress(NoChange,ComposeRString(scratch,brk?BREAKING:WAITING,tics));
	}
	else
	{
		Progress(NoChange,data);
	}

	CarefulLog(LOG_NAV,LOG_SENT,data+1,*data);
	if (brk)
	{
		if (CTBErr=MyCMBreak(CnH,tics)) return(CTBErr);
	}
	else if (wait)
	  for (tics+=TickCount();TickCount()<tics;) GiveTime();
	else if (CTBErr=CTBSendTrans(1,data+1,*data)) return(CTBErr);
	if (*expect)
	{
		CarefulLog(LOG_NAV,LOG_EXPECT,expect+1,*expect);
		tics = TickCount()+GetRLong(RECV_TIMEOUT)*60;
		exSpot = expect+1;
	}
	do
	{
		bSize = sizeof(data)-1;
		CTBErr = RecvLine(data+1,&bSize);
		if (bSize)
		{
			data[0] = bSize;
			Progress(NoChange,data);
			CarefulLog(LOG_NAV,LOG_GOT,data+1,*data);
			if (*expect)
			{
				for(dSpot=data+1;dSpot<=data+*data;dSpot++)
					if ((*dSpot&0x7f)==*exSpot)
					{
						if (++exSpot>expect+*expect)
						{
							ComposeLogR(LOG_NAV,nil,LOG_FOUND);
							found = True;
							break;
						}
					}
					else
						exSpot = expect+1;
			}
		}
		if (*expect && TickCount()>tics)
		{
			GetRString(scratch,RECV_TIMEOUT);
			CTBSilenceTrans(False);
			if (AlertStr(TIMEOUT_ALRT,Caution,scratch)==1)
				tics = TickCount()+60*GetRLong(RECV_TIMEOUT);
			else
			{
				ComposeLogR(LOG_NAV,nil,LOG_NOTFOUND);
				CTBErr = userCancelled;
			}
			CTBSilenceTrans(True);
		}
	}
	while (!CommandPeriod && (!CTBErr || CTBErr==cmTimeOut&&*expect) && !found);
	if (CTBErr==cmTimeOut) CTBErr = 0;
	if (*expect && !found)
	{
		CTBErr = userCancelled;
		InvalidatePasswords(0==(escFlags&ESC_PW),0==(escFlags&ESC_AUXPW));
	}
	return(CTBErr);
}

/************************************************************************
 * MyCMClose - call CMClose
 ************************************************************************/
short MyCMClose(ConnHandle cn)
{
	AsyncStuff stuff;
	short err;						/* real error */

	(*cn)->refCon = (long) &stuff;				/* store address */
	stuff.pending = 1;
	if (PrefIsSet(PREF_SYNCH_CTB))
		err=CMClose(cn,False,nil,-1,True);
	else
	{
		(void) CMIOKill(cn,cmDataIn);
		(void) CMIOKill(cn,cmDataOut);
		if (!(err=CMClose(cn,True,CTBComplete,-1,True)))
			err = SpinOn(&stuff.pending,0) || stuff.errCode;
			
	}
	CTBTrouble(err);
	return (err); 				/* error? */
}

/************************************************************************
 * MyCMOpen - call CMOpen
 ************************************************************************/
short MyCMOpen(ConnHandle cn,long timeout)
{
	AsyncStuff stuff; 										
	short err;						/* real error */
	
	(*cn)->refCon = (long) &stuff;				/* store address */
	stuff.pending = 1;
	if (PrefIsSet(PREF_SYNCH_CTB))
		err=CMOpen(cn,False,CTBComplete,timeout);
	else
	{
		if (!(err=CMOpen(cn,True,CTBComplete,timeout)))
		{
			err = SpinOn(&stuff.pending,0) || stuff.errCode;
			if (err==userCancelled)
				(void) CMAbort(CnH);
		}
	}

	CTBTrouble(err);
	return (err);
}

/************************************************************************
 * MyCMRead - call CMRead
 ************************************************************************/
short MyCMRead(ConnHandle cn,UPtr buffer,long *size,long timeout)
{
	AsyncStuff stuff;
	short err;						/* real error */
	CMFlags junk;
	
	(*cn)->refCon = (long)&stuff;
	stuff.pending = 1;
	if (!(err=CMRead(cn,buffer,size,cmData,True,CTBComplete,timeout,&junk)))
	{
		err = SpinOn(&stuff.pending,0) || stuff.errCode;
		if (err==userCancelled)
			CMIOKill(cn,cmDataIn);
	}

	if (!err || err==cmTimeOut)
	{
		*size = stuff.asyncCount[cmDataIn];
		if (*size) err = noErr;
		else err = cmTimeOut;
	}
	else
		*size = 0;
	if (*size && LogLevel&LOG_TRANS) CarefulLog(LOG_TRANS,LOG_GOT,buffer,*size);
	if (err!=cmTimeOut) CTBTrouble(err);
	return(err);
}

/************************************************************************
 * MyCMWrite - call CMWrite
 ************************************************************************/
short MyCMWrite(ConnHandle cn,UPtr buffer,long *size)
{
	AsyncStuff stuff;
	short err;						/* real error */
	(*cn)->refCon = (long)&stuff;
	stuff.pending = 1;
	if (!DontTranslate && TransOut)
		{Uptr cp; for(cp=buffer;cp<buffer+*size;cp++) *cp = TransOut[*cp];}
	if (LogLevel&LOG_TRANS) CarefulLog(LOG_TRANS,LOG_SENT,buffer,*size);
	if (!(err=CMWrite(cn,buffer,size,cmData,True,CTBComplete,-1,0)))
	{
		err = SpinOn(&stuff.pending,0) || stuff.errCode;
		if (err==userCancelled)
			CMIOKill(cn,cmDataOut);
	}

	if (!err)
		*size = stuff.asyncCount[cmDataOut];
	else
		*size = 0;

	CTBTrouble(err);
	return(err);
}

/************************************************************************
 * MyCMBreak - call CMBreak
 ************************************************************************/
short MyCMBreak(ConnHandle cn,long howLong)
{
	AsyncStuff stuff;
	short err;						/* real error */
	(*cn)->refCon = (long)&stuff;
	stuff.pending = 1;
	CMBreak(cn,howLong,True,CTBComplete);
	
	err = SpinOn(&stuff.pending,0) || stuff.errCode;
	if (err==userCancelled)
		CMIOKill(cn,cmDataOut);

	CTBTrouble(err);
	return(err);
}

/************************************************************************
 * CTBTR - report a CTB error
 ************************************************************************/
short CTBTR(short err,short file,short line)
{
	Uhandle strnH;
	
	if (!SilentCTB &&!CommandPeriod && err && err!=userCancelled)
	{
		strnH = GetResource('STR#',CTB_ERR_STRN);
		if (strnH)
		{
			Str127 errString,debugStr,message;
			if (err+2 > 0 && err+2 <= (*strnH)[0]*256 + (*strnH)[1])
				GetRString(errString,CTB_ERR_STRN+err+2);
			else
				NumToString(err,errString);
			ReleaseResource(strnH);
			GetRString(message,CTB_PROBLEM);
			ComposeRString(debugStr,FILE_LINE_FMT,file,line);
			MyParamText(message,errString,"",debugStr);
			(void) ReallyDoAnAlert(OK_ALRT,Caution);
			CTBSilenceTrans(True);
		}
	}
	return(err);
}

/************************************************************************
 * CTBWhoAmI - return the mac's ctb name
 ************************************************************************/
UPtr CTBWhoAmI(Uptr who)
{
	return(GetRString(who,CTB_ME));
}

/************************************************************************
 * MyCMIdle - do the things I should do at CMIdle time
 ************************************************************************/
void MyCMIdle(void)
{
	CMIdle(CnH);
	CTBHasChars = CTBDataCount() ? noErr : inProgress;
}

/************************************************************************
 * CTBDataCount - how much data do we have?
 ************************************************************************/
long CTBDataCount(void)
{
	CMStatFlags flags;
	CMBufferSizes sizes;
	
	CMStatus(CnH,sizes,&flags);
	return(flags&cmStatusDataAvail ? sizes[cmDataIn] : 0);
}

/************************************************************************
 * GetSecondPass - get the user's secondary password
 ************************************************************************/
short GetSecondPass(UPtr pass)
{
	Str31 save;
	short err;
	Str31 dialup;
	
	if (!*SecondPass)
	{
		PCopy(save,Password);
		*Password = 0;
		GetRString(dialup,DIALUP);
		if (!(err=GetPassword(dialup,pass,32,ENTER)))
		{
			PCopy(SecondPass,pass);
			if (PrefIsSet(PREF_SAVE_PASSWORD) && *SecondPass)
			{
				ChangeStrn(PREF_STRN,PREF_AUXPW,Password);
				UpdateResFile(SettingsRefN);
			}
		}
		PCopy(Password,save);
		return(err);
	}
	else
	{
		PCopy(pass,SecondPass);
		return(noErr);
	}
}

#pragma segment Transport
/************************************************************************
 * EscapeCopy - copy a string, expanding escapes
 ************************************************************************/
short EscapeCopy(PStr to,PStr from,PStr expect)
{
	UPtr end = from+*from;
	UPtr spot = to+1;
	Boolean esc;
	Str127 scratch1, scratch2;
	short returnVal=0;
	
	*expect = 0;
	from++;
	for (esc=false; from<=end; from++)
	{
		if (esc)
		{
			esc = False;
			switch (*from)
			{
				case 'U':
					GetPref(scratch1,PREF_AUXUSR);
					BlockMove(scratch1+1,spot,*scratch1);
					spot += *scratch1;
					returnVal |= ESC_AUXUSR;
					if (!*scratch1) returnVal |= ESC_AUXUSRERR;
					break;
				case 'P':
					if (GetSecondPass(SecondPass))
					{
						CommandPeriod = True;
						break;
					}
					BlockMove(SecondPass+1,spot,*SecondPass);
					spot += *SecondPass;
					returnVal |= ESC_AUXPW;
					break;
				case 'u':
					GetPOPInfo(scratch1,scratch2);
					BlockMove(scratch1+1,spot,*scratch1);
					spot += *scratch1;
					returnVal |= ESC_USR;
					break;
				case 'h':
					GetPOPInfo(scratch1,scratch2);
					BlockMove(scratch2+1,spot,*scratch2);
					spot += *scratch2;
					break;
				case 's':
					GetSMTPInfo(scratch1);
					BlockMove(scratch1+1,spot,*scratch1);
					spot += *scratch1;
					break;
				case 'e':						/* switch to 'expect' string */
					*to = spot-to-1;
					to[*to+1] = 0;
					to = expect;
					spot=to+1;
					break;
#ifndef KERBEROS
				case 'p':
					BlockMove(Password+1,spot,*Password);
					spot += *Password;
					returnVal |= ESC_PW;
					break;
#endif
				case 'n': *spot++ = '\r'; break;
				case 'r': *spot++ = '\n'; break;
				case 'b': *spot++ = bulletChar; break;
				case 'B': *spot++ = betaChar; break;
				case 'D': *spot++ = deltaChar; break;
				default: *spot++ = *from; break;
			}
		}
		else
		{
			esc = *from=='\\';
			if (!esc) *spot++ = *from;
		}
	}
	*to = spot-to-1;
	to[*to+1] = 0;	/* null terminate, just for fun */
	return(returnVal);
}

#pragma segment Main

/************************************************************************
 * CTBComplete - completion routine
 ************************************************************************/
pascal void CTBComplete(ConnHandle cn)
{
	AsyncStuffPtr stuffP = (AsyncStuffPtr)(*cn)->refCon;
	uLong *fp, *tp, *ep;
	
	stuffP->errCode = (*cn)->errCode;
	fp=(*cn)->asyncCount;
	tp=stuffP->asyncCount;
	ep=tp+sizeof(CMBufferSizes)/sizeof(uLong);
	while (tp<ep) *tp++ = *fp++;
	stuffP->pending = 0;
}
