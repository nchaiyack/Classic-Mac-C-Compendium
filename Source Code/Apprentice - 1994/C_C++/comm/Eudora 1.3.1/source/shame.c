#define FILE_NUM 35
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * functions of which I am not proud
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Util
static Boolean AlertBeep=True;
void DeepTrouble(UPtr);
void SilenceAlert(int template);
void TrashAlert(int template);
short AlertDefault(short template);
#define GetANumber() (*(short *)0xa98)
/************************************************************************
 *
 ************************************************************************/
int Aprintf(short template,short which,short rFormat,...)
{
	Str255 message;
	va_list args;
	va_start(args,rFormat);
	VaComposeRString(message,rFormat,args);
	va_end(args);
	return(AlertStr(template,which,message));
}
	
/************************************************************************
 *
 ************************************************************************/
int AlertStr(int template, int which, UPtr str)
{
	MyParamText(str,"","","");
	return(ReallyDoAnAlert(template,which));
}

/**********************************************************************
 * print an alert
 **********************************************************************/
int ReallyDoAnAlert(int template,int which)
{
	int item;
	
	LogAlert(template);
	if (!MommyMommy(ATTENTION,nil))
	  item = AlertDefault(template);
	else
	{
		WNE(nil,nil,0); 							/* make sure InBG is right */
		PushCursor(arrowCursor);
		ThirdCenterDialog(template);
		if (!AlertBeep) SilenceAlert(template);
		if (AlertsTimeout && AlertTicks < TickCount())
			AlertTicks=GetRLong(ALERT_TIMEOUT)*60+TickCount();
		switch (which)
		{
			case Normal:	item = Alert(template,DlgFilter); break;
			case Stop:		item = StopAlert(template,DlgFilter); break;
			case Note:		item = NoteAlert(template,DlgFilter); break;
			case Caution: item = CautionAlert(template,DlgFilter); break; 
		}
		if (!AlertBeep) TrashAlert(template);
		AlertTicks = 0;
	}
	ComposeLogR(LOG_ALRT,nil,ALERT_DISMISSED_ITEM,item);
	PopCursor();
	return(item);
}

/************************************************************************
 * AlertDefault - find the default action for an alert
 ************************************************************************/
short AlertDefault(short template)
{
	AlertTHndl alert=GetResource('ALRT',template);
	short item=1;
	short stage;
	if (alert)
	{
		stage = (template==GetANumber()) ? GetAlrtStage() : 0;
		stage = MIN(3,stage);
		GetAlrtStage() = MIN(stage+2,4);
		GetANumber() = template;
		item = ((*alert)->stages&(8<<(stage*4))) ? 2 : 1;
	}
	return(item);
}

/**********************************************************************
 * DeepTrouble - tell the user that BAD things are happening
 **********************************************************************/
void DeepTrouble(str)
UPtr str;
{
	MyParamText(str,"","","");
	InitCursor();
	CenterDialog(OK_ALRT);
	(void) StopAlert(OK_ALRT,nil);
	ExitToShell();
}

/************************************************************************
 * IsSwitch - is a mouseDown in the MF switch area?
 ************************************************************************/
Boolean IsSwitch(EventRecord *event)
{
	Rect bnds = qd.screenBits.bounds;
	bnds.bottom = bnds.top+GetMBarHeight();
	bnds.left = bnds.right-36;
	return (PtInRect(event->where,&bnds));
}

/************************************************************************
 * Switch - go to the next MF app
 ************************************************************************/
Boolean Switch(void)
{
	Str63 name, appName;
	short appRefNum;
	Handle apParam;
	short barItem, item, myItem, n, cmd;
	MenuHandle appM = GetMHandle(APPLE_MENU);
	long resp;
	
	if (!Gestalt(gestaltOSAttr,&resp) && resp&(1L<<gestaltLaunchControl))
	{
		Point pt;
		pt.h = qd.screenBits.bounds.right-30; pt.v = 10;
	
		return(MenuSelect(pt));
	}

	GetAppParms(appName,&appRefNum,&apParam);
	
	n = CountMItems(appM);
	
	for (item=2;item<n;item++)
	{
		GetItem(appM,item,name);
		if (name[1]=='-') barItem=item;
		else if (EqualString(name,appName,False,True))
		{
			myItem = item;
			break;
		}
	}
	
	if (item>n) return(False);										/* not found */
	if (item==n)
		if (barItem==item-1) return(False); 				/* nobody else */
		else item=barItem;													/* bottom of menu */
	else
	{
		GetItem(appM,item+1,name);
		if (name[1]=='-')
			if (barItem==item-1)
				return(False);													/* nobody else */
			else
				item=barItem; 													/* bottom of app list */
	}
	
	for (item++;item!=myItem;item++)
		if (item>n) item=barItem;
		else
		{
			GetItem(appM,item,name);
			if (name[1]=='-') item=barItem;
			else
			{
				GetItemCmd(appM,item,&cmd); 		/* undocumented magic here */
				if (cmd!=0x1f)									/* is the app not hidden? */
				{
					OpenDeskAcc(name);
					return(True);
				}
			}
		}

	return(False);
} 

/************************************************************************
 * SetAlertBeep - set whether or not an alert should beep
 ************************************************************************/
void SetAlertBeep(Boolean onOrOff)
{
	AlertBeep = onOrOff;
}

/************************************************************************
 * SilenceAlert - turn the beeps off in an alert template
 ************************************************************************/
void SilenceAlert(int template)
{
	AlertTHndl aTempl;

	if (aTempl=(AlertTHndl)GetResource('ALRT',template))
		(*aTempl)->stages &= ~0x3333;
}

/************************************************************************
 * TrashAlert - get rid of a mucked alert template
 ************************************************************************/
void TrashAlert(int template)
{
	AlertTHndl aTempl;

	if (aTempl=(AlertTHndl)GetResource('ALRT',template))
		ReleaseResource(aTempl);
}

pascal void OkWhatIsIt(QElemPtr nmReqPtr);

/************************************************************************
 * MommyMommy - get the user's attention
 ************************************************************************/
Boolean MommyMommy(short sId,UPtr string)
{
	NMRec nm;
	Str255 scratch;
	short pend;
	short nmResult;
	
	if (!InBG) return(True);
	WriteZero(&nm,sizeof(nm));
	
	nm.nmSound = PrefIsSet(PREF_NEW_SOUND) ? GetResource('snd ',ATTENTION_SND):nil;
	nm.nmResp = OkWhatIsIt;
	nm.nmStr = PrefIsSet(PREF_NEW_ALERT) ? (string ? string : (sId ? GetRString(scratch,sId) : nil)) : nil;
	nm.nmRefCon = (long) &pend;
	nm.nmMark = 1;
	nm.nmIcon = PrefIsSet(PREF_NO_APPLE_FLASH) ? nil : GetResource('SICN',EUDORA_SICN);
	nm.qType = nmType;
	if (nm.nmSound) HNoPurge(nm.nmSound);
	if (nm.nmIcon) HNoPurge(nm.nmIcon);
	nmResult = NMInstall(&nm);
	if (pend!=inProgress)
		pend=SpinOn(&InBG,AlertsTimeout?GetRLong(ALERT_TIMEOUT)*60:0);
	if (!nmResult) NMRemove(&nm);
	return(pend==0);
}

#pragma segment Main
/************************************************************************
 *
 ************************************************************************/
pascal void OkWhatIsIt(QElemPtr nmReqPtr)
{
	*(short *)((NMRec *)nmReqPtr)->nmRefCon = 0;
}
#pragma segment Util

#ifdef DEBUG
#define TRIGGER (28<size && size < 54)
/************************************************************************
 *
 ************************************************************************/
UPtr NuPtr(long size)
{
	if (BUG8 && TRIGGER) SysBreak();
	return(NewPtr(size));
}

/************************************************************************
 *
 ************************************************************************/
Handle NuHandle(long size)
{
	if (BUG8 && TRIGGER) SysBreak();
	return(NewHandle(size));
}

/************************************************************************
 *
 ************************************************************************/
void SetHandleBig(Handle h,long size)
{
	if (BUG8 && TRIGGER) SysBreak();
	SetHandleSize(h,size);
}
#endif
