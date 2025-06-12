#define FILE_NUM 41
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * various useful functions
 **********************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Util
#ifdef	KERBEROS
#include				<krb.h>
#endif

char BitTable[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
pascal Boolean PasswordFilter(DialogPtr dgPtr,EventRecord *event,short *item);
void CopyPassword(UPtr password);
int ResetPassword(void);
UHandle PwChars=nil;
void NukeMenuItem(MenuHandle mh,short item);

/**********************************************************************
 * Allocate a handle, and write zeroes all over it
 **********************************************************************/
Handle NewZHandle(long size)
{
	Handle theHandle;
	
	/*
	 * allocate it
	 */
	if ((theHandle=NuHandle(size))==nil)
		return(nil);
		
	/*
	 * zero it
	 */
	WriteZero(*theHandle,size);
	
	/*
	 * all done
	 */
	return(theHandle);
}

/**********************************************************************
 * write zeroes over an area of memory
 **********************************************************************/
void WriteZero(UPtr pointer,long size)
{
	while (size--) *pointer++ = 0;
}

/**********************************************************************
 * initialize all the mac managers
 **********************************************************************/
void MacInitialize(int masterCount, long ensureStack)
{
	EventRecord event;
	
	if (*(long *)CurStackBase - *(long *)ApplLimit <ensureStack)
		SetApplLimit((UPtr)(*(long *)CurStackBase - ensureStack));
	MaxApplZone();
	while (masterCount--) MoreMasters();
	FlushEvents (everyEvent - diskMask, 0 );
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);		/* no restart proc */
	InitCursor ();
	InitPack(stdFile);
	InitPack(bdConv);
	InitPack(intUtil);
	WNE(nullEvent,&event,0);
	WNE(nullEvent,&event,0);
	WNE(nullEvent,&event,0);
}

/**********************************************************************
 * turn a font name into a font id; if the name is not found, use ApplFont
 **********************************************************************/
GetFontID(UPtr theName)
{
	short theID;
	Str255 systemName;
	
	GetFNum(theName,&theID);
	if (!theID)
	{
		GetFontName(0,systemName);
		return (EqualString(theName,systemName,FALSE,FALSE) ? 0: applFont);
	}
	else
		return (theID);
}
/**********************************************************************
 * get an event, giving time to system tasks as necessary
 **********************************************************************/
Boolean GrabEvent(EventRecord *theEvent)
{
	Boolean result;
	
	/*
	 * get the event
	 */
	result = WNE(everyEvent,theEvent,InBG ? 300L : 10L);

	/*
	 * handle special keys
	 */
	if (theEvent->what==keyDown || theEvent->what==autoKey)
		SpecialKeys(theEvent);
	
	/*
	 * gd dialog manager
	 */
	if (!result && IsDialogEvent(theEvent)) DoModelessEvent(theEvent);

	return(result);
}

/**********************************************************************
 * Check or uncheck a font size in the font menu
 **********************************************************************/
void CheckFontSize(int menu,int size,Boolean check)
{
	Str255 aString;
	Str255 itemString;
	MenuHandle mHandle;
	short item;
	
	/*
	 * turn the font size into a string
	 */
	NumToString((long)size,aString);
		
	/*
	 * get a copy of the menu handle
	 */
	mHandle = GetMHandle(menu);
	
	/*
	 * look for the proper item
	 */
	for (item = 1; ;item++)
	{
		GetItem(mHandle,item,itemString);
		if (itemString[1]=='-') break;
		if (EqualString(aString,itemString,FALSE,FALSE))
		{
			CheckItem(mHandle,item,check);
			break;
		}
	}
}
/**********************************************************************
 * check (or uncheck) a font name in a menu.	If check, also outline sizes
 **********************************************************************/
void CheckFont(int menu,int fontID,Boolean check)
{
	Str255 aString;
	Str255 itemString;
	MenuHandle mHandle;
	short item;
	
	/*
	 * turn the font id into a font name
	 */
	GetFontName(fontID,aString);
	
	/*
	 * get a copy of the menu handle
	 */
	mHandle = GetMHandle(menu);
	
	/*
	 * look for the proper item
	 */
	for (item = 1; ;item++)
	{
		GetItem(mHandle,item,itemString);
		if (itemString[1]=='-') break;		/* skip the sizes */
	}
	for (item++; ;item++)
	{
		GetItem(mHandle,item,itemString);
		if (EqualString(aString,itemString,FALSE,FALSE))
		{
			CheckItem(mHandle,item,check);
			if (check) OutlineFontSizes(menu,fontID);
			break;
		}
	}
}

/**********************************************************************
 * outline sizes of a font in a menu
 **********************************************************************/
void OutlineFontSizes(int menu,int fontID)
{
	Str255 aString;
	MenuHandle mHandle;
	short item;
	long aSize;
	
	/*
	 * get a copy of the menu handle
	 */
	mHandle = GetMHandle(menu);

	/*
	 * outline'em
	 */
	for (item = 1; ;item++)
	{
		GetItem(mHandle,item,aString);
		if (aString[1]=='-') break; 	/* end of sizes? */
		/*
		 * turn text into size
		 */
		StringToNum(aString,&aSize);
		
		/*
		 * does it exist?
		 */
		if (RealFont(fontID,(short)aSize))
			SetItemStyle(mHandle,item,outline);
		else
			SetItemStyle(mHandle,item,nil);
	}
}

/**********************************************************************
 * figure out the appropriate leading for a font
 **********************************************************************/
int GetLeading(int fontID,int fontSize)
{
	FMInput fInInfo;
	FMOutput *fOutInfo;
	
	/*
	 * set up the font input struct
	 */
	fInInfo.family = fontID;
	fInInfo.size = fontSize;
	fInInfo.face = 0;
	fInInfo.needBits = FALSE;
	fInInfo.device = 0;
	fInInfo.numer.h = fInInfo.numer.v = 1;
	fInInfo.denom.h = fInInfo.denom.v = 1;
	
	
	/*
	 * get the actual info
	 */
	fOutInfo = FMSwapFont(&fInInfo);
	
	/*
	 * yokey-dokey
	 */
	return(((fOutInfo->leading + fOutInfo->ascent + fOutInfo->descent)*fOutInfo->numer.v)/fOutInfo->denom.v);
}

/**********************************************************************
 * find width of largest char in font
 **********************************************************************/
int GetWidth(int fontID,int fontSize)
{
#ifdef FONTMGR
	FMInput fInInfo;
	FMOutput *fOutInfo;
	
	/*
	 * set up the font input struct
	 */
	fInInfo.family = fontID;
	fInInfo.size = fontSize;
	fInInfo.face = 0;
	fInInfo.needBits = FALSE;
	fInInfo.device = 0;
	fInInfo.numer.h = fInInfo.numer.v = 1;
	fInInfo.denom.h = fInInfo.denom.v = 1;
	
	
	/*
	 * get the actual info
	 */
	fOutInfo = FMSwapFont(&fInInfo);
	
	/*
	 * yokey-dokey
	 */
	return((fOutInfo->widMax * fOutInfo->numer.h)/fOutInfo->denom.h);
#else

	GrafPtr oldPort;
	GrafPort aPort;
	int width;
	
	GetPort(&oldPort);
	
	OpenPort(&aPort);
	TextFont(fontID);
	TextSize(fontSize);
	width = CharWidth('0');
	
	ClosePort(&aPort);
	SetPort(oldPort);
	
	return(width);
#endif
}

/**********************************************************************
 * find descent font
 **********************************************************************/
int GetDescent(int fontID,int fontSize)
{
	FMInput fInInfo;
	FMOutput *fOutInfo;
	
	/*
	 * set up the font input struct
	 */
	fInInfo.family = fontID;
	fInInfo.size = fontSize;
	fInInfo.face = 0;
	fInInfo.needBits = FALSE;
	fInInfo.device = 0;
	fInInfo.numer.h = fInInfo.numer.v = 1;
	fInInfo.denom.h = fInInfo.denom.v = 1;
	
	
	/*
	 * get the actual info
	 */
	fOutInfo = FMSwapFont(&fInInfo);
	
	/*
	 * yokey-dokey
	 */
	return((fOutInfo->descent * fOutInfo->numer.v)/fOutInfo->denom.v);
}

/**********************************************************************
 * find ascent font
 **********************************************************************/
int GetAscent(int fontID,int fontSize)
{
	FMInput fInInfo;
	FMOutput *fOutInfo;
	
	/*
	 * set up the font input struct
	 */
	fInInfo.family = fontID;
	fInInfo.size = fontSize;
	fInInfo.face = 0;
	fInInfo.needBits = FALSE;
	fInInfo.device = 0;
	fInInfo.numer.h = fInInfo.numer.v = 1;
	fInInfo.denom.h = fInInfo.denom.v = 1;
	
	
	/*
	 * get the actual info
	 */
	fOutInfo = FMSwapFont(&fInInfo);
	
	/*
	 * yokey-dokey
	 */
	return((fOutInfo->ascent * fOutInfo->numer.v)/fOutInfo->denom.v);
}

/**********************************************************************
 * find fixed-width-ness of font
 **********************************************************************/
Boolean IsFixed(int fontID,int fontSize)
{
	FMInput fInInfo;
	FMOutput *fOutInfo;
	
	/*
	 * set up the font input struct
	 */
	fInInfo.family = fontID;
	fInInfo.size = fontSize;
	fInInfo.face = 0;
	fInInfo.needBits = FALSE;
	fInInfo.device = 0;
	fInInfo.numer.h = fInInfo.numer.v = 1;
	fInInfo.denom.h = fInInfo.denom.v = 1;
	
	
	/*
	 * get the actual info
	 */
	fOutInfo = FMSwapFont(&fInInfo);
	
	/*
	 * yokey-dokey
	 */
	return((((FontRec *)(*fOutInfo->fontHandle))->fontType & fixedFont)==fixedFont);
}

/**********************************************************************
 * wait for the user to strike a modifier key
 **********************************************************************/
void AwaitKey(void)
{
	KeyMap kMap;
	register long * k;
	register long * kEnd;
	
	while (1)
	{
		GetKeys(&kMap);
		for (k= &kMap,kEnd=k+sizeof(KeyMap)/sizeof(long); k<kEnd; k++)
			if (*k) return;
	}
}
/**********************************************************************
 * change or add some data to the current resource file
 **********************************************************************/
void ChangePResource(UPtr theData,int theLength,long theType,int theID,UPtr theName)
{
	Handle aHandle;
	
	/*
	 * does the resource exist and reside in the topmost res file?
	 */
	SetResLoad(FALSE);
	aHandle = Get1Resource(theType,(short)theID);
	SetResLoad(TRUE);
	
	if (aHandle)
	{
		/* must nuke the resource first */
		RmveResource(aHandle);
		DisposHandle(aHandle);
	}
		
	AddPResource(theData, theLength, theType, theID, theName);
}

/**********************************************************************
 * add some data to the current resource file
 **********************************************************************/
void AddPResource(theData, theLength, theType, theID, theName)
UPtr theData; 		 /* data to go into resource */
int theLength;		/* number of bytes of data */
long theType; 		/* resource type */
int theID;				/* resource id */
UPtr theName; 		 /* name of resource */
{
	Handle aHandle;
	
	/*
	 * allocate the handle
	 */
	aHandle = NuHandle((long)theLength);
	if (aHandle == nil)
		return;
	
	/*
	 * copy the data
	 */
	BlockMove(theData,*aHandle,(long)theLength);
	
	/*
	 * add it
	 */
	AddResource(aHandle,theType,theID,theName);
}

/**********************************************************************
 * ResourceCpy - copy a resource from one resource file to the other
 **********************************************************************/
int ResourceCpy(short toRef, short fromRef,long type,int id)
{
	int oldRef;
	Handle resource;
	Str255 name;
	short attrs;
	
	oldRef = CurResFile();
	UseResFile(toRef);
	SetResLoad(false);
	resource = GetResource(type,id); /* no sense loading it if it's wrong */
	if (resource!=nil)
	{
		if (HomeResFile(resource)==toRef)
		{
			/*
			 * delete the old one first
			 */
			RmveResource(resource);
			DisposHandle(resource);
			resource = nil;
		}
		else if (HomeResFile(resource)!=fromRef)
		{
			ReleaseResource(resource);		/* wrong one */
			resource = nil;
		}
	}
	
	if (resource==nil)
	{ 	
		/*
		 * fetch from the proper file
		 */
		UseResFile(fromRef);
		resource = Get1Resource(type,id);
		if (resource==nil) return(resNotFound);
	}
	
	SetResLoad(True); 									/* turn that goodie back on */
	attrs = GetResAttrs(resource);			/* save attributes */
	GetResInfo(resource,&id,&type,name);/* get the name */
	HNoPurge(resource); 								/* keep it right here */
	LoadResource(resource); 						/* load it in  */
	if (ResError()) return(ResError()); /* did that work? */
	DetachResource(resource); 					/* break cnxn with old file */
	UseResFile(toRef);									/* point at to resource file */
	AddResource(resource,type,id,name); /* stick into new file */
	if (ResError()) return(ResError()); /* did that work? */
	SetResAttrs(resource,attrs);				/* restore attributes */
	ChangedResource(resource);					/* except it's been changed */
	UseResFile(oldRef); 								/* restore old res file */
	
	return(noErr);
}

/**********************************************************************
 * DrawTruncString - truncate and draw a string; restores string when done
 **********************************************************************/
void DrawTruncString(UPtr string,int len)
{
	int sLen = *string;
	
	if (sLen > len)
	{
		Byte save = string[len];
		string[len] = 'É';
		*string = len;
		DrawString(string);
		*string = sLen;
		string[len] = save;
	}
	else
		DrawString(string);
}

/**********************************************************************
 * CalcTrunc - figure out how much of a string we can print to fit
 * in a given width
 **********************************************************************/
int CalcTextTrunc(UPtr string,short length, short width,GrafPtr port)
{
	short tLen;
	int cWidth;
	GrafPtr oldPort;
	
	if (width<=0) return(0);
	
	GetPort(&oldPort);
	SetPort(port);
	
	/*
	 * make an initial estimate
	 */
	tLen = MIN(width/CharWidth(' '),length);
	
	/*
	 * if the string fits, ...
	 */
	cWidth = TextWidth(string,0,tLen);
	if (cWidth<width && tLen==length) {SetPort(oldPort);return(length);}
	
	/*
	 * if it's too short
	 */
	if (cWidth<=width)
	{
		if (tLen<length)
		{
			tLen++;
			while (TextWidth(string,0,tLen)<width && tLen<length) tLen++;
			tLen--;
		}
	}
	/*
	 * too long...
	 * (we do pretend there is always room for 1 character)
	 */
	else
		do
		{
			tLen--;
			cWidth = TextWidth(string,0,tLen);
		}
		while (tLen && cWidth>width);
	
	SetPort(oldPort);
	return (tLen ? tLen : 1);
}

/**********************************************************************
 * WhiteRect - draw white rectangle with a black border
 **********************************************************************/
void WhiteRect(Rect *r)
{
	Rect myR = *r;
	
	FrameRect(&myR);
	InsetRect(&myR,1,1);
	EraseRect(&myR);
}

/************************************************************************
 * WannaSave - find out of the user wants to save the contents of a window
 ************************************************************************/
int WannaSave(MyWindowPtr win)
{
	Str255 title;
	
	PCopy(title,*win->qWindow.titleHandle);
	return(AlertStr(WANNA_SAVE_ALRT,Stop,title));
}

/************************************************************************
 * GetPassword - read a user's password
 ************************************************************************/
#ifdef	KERBEROS
int GetPassword(void)
#else
int GetPassword(UPtr forString,UPtr word, int size, short prompt)
#endif
{
	Str255 string;
	DialogPtr dgPtr;
	short item;
#ifdef	KERBEROS
	Str63 userName, args;
	char	name[ANAME_SZ];
	char	inst[INST_SZ];
	char	realm[REALM_SZ];
	CREDENTIALS 	cr;
	int 	status;
#endif

#ifdef	KERBEROS
				/* perhaps we're logged in already? */
				GetPOPInfo(userName,args);
				PtoCcpy(name, userName);

				inst[0] = 0;
				if(0 != (status = krb_get_lrealm(realm, 1))) {
								char		message[256];
								
								sprintf(message, "Couldn't log in: %s", krb_err_txt[status]);
								c2pstr(message);
								AlertStr(OK_ALRT,Caution,message);
								return 1;
								}
				status = krb_get_cred(name, inst, realm, "krbtgt", realm, realm, &cr);
				if(!status) { 	/* credentials exist */
								return 0;
								}
				/* otherwise we need to get the password */
#endif
	
	if (!MommyMommy(ATTENTION,nil)) return(PASSWORD_CANCEL);
	GetRString(string,prompt);
	MyParamText(forString,string,"","");
	ThirdCenterDialog(PASSWORD_DLOG);
	if ((dgPtr = GetNewMyDialog(PASSWORD_DLOG,nil,InFront,ThirdCenterDialog))==nil)
	{
		WarnUser(GENERAL,MemError());
		return(PASSWORD_CANCEL);
	}
	if (!(CurrentModifiers()&alphaLock)) HideDItem(dgPtr,PASSWORD_WARNING);
	
	ShowWindow(dgPtr);
	HiliteButtonOne(dgPtr);
	do
	{
		SetDIText(dgPtr,PASSWORD_WORD,"");
		SelIText(dgPtr,PASSWORD_WORD,0,INFINITY);
		if (ResetPassword()) {item=PASSWORD_CANCEL; break;}
		PushCursor(arrowCursor);
		ModalDialog(PasswordFilter,&item);
		PopCursor();
		CopyPassword(string);
	}
	while (item==PASSWORD_OK && !*string);
	DisposDialog(dgPtr);
	InBG = False; 
	if (item==PASSWORD_OK)
	{
#ifndef KERBEROS
		if (*string > size-1) *string = size - 1;
		PCopy(word,string);
#else
				/* login */
				p2cstr(string);
				status = krb_get_pw_in_tkt(name, inst, realm, "krbtgt", realm, 96,
																																(char *)string);
				if(status != KSUCCESS) {
								char		message[256];
								
								sprintf(message, "Login failed: %s", krb_err_txt[status]);
								c2pstr(message);
								AlertStr(OK_ALRT,Caution,message);
								return 1;
								}
#endif
	}
	return(item!=PASSWORD_OK);
}

/************************************************************************
 * ResetPassword - get the password routines ready
 ************************************************************************/
int ResetPassword(void)
{
	if (!PwChars)
		PwChars = NuHandle(256L);
	else if (!*PwChars)
		ReallocHandle(PwChars,256L);
	if (!PwChars || !*PwChars) return(MemError());
	HNoPurge(PwChars);
	**PwChars = 0;
	return(0);
}

/************************************************************************
 * InvalidatePasswords - wipe out our memory of passwords
 ************************************************************************/
void InvalidatePasswords(Boolean pwGood,Boolean auxpwGood)
{
	if (!pwGood)
	{
		*Password = 0;
		POPSecure = 0;
		if (PrefIsSet(PREF_SAVE_PASSWORD))
			ChangeStrn(PREF_STRN,PREF_PASS_TEXT,Password);
	}
	if (!auxpwGood)
	{
		*SecondPass = 0;
		if (PrefIsSet(PREF_SAVE_PASSWORD))
			ChangeStrn(PREF_STRN,PREF_AUXPW,Password);
	}
}

/************************************************************************
 * PasswordFilter - a ModalDialog filter for getting passwords
 ************************************************************************/
pascal Boolean PasswordFilter(DialogPtr dgPtr,EventRecord *event,short *item)
{
	if (!MiniMainLoop(event))
		if (!CommandPeriod)
			return(False);
		else
		{
			*item = CANCEL_ITEM;
			return(True);
		}
	if (event->what==keyDown || event->what==autoKey)
	{
		char key = event->message & charCodeMask;
		switch (key)
		{
			case enterChar:
			case returnChar:
				*item = 1;
				return(True);
				break;
			case backSpace:
				if (**PwChars) --**PwChars;
				return(False);
				break;
			case '.':
				if (event->modifiers & cmdKey)
				{
					*item=2;
					return(True);
					break;
				}
				/* fall through */
			default:
				if (**PwChars < 255 && key != tabChar)
				{
					PCatC(*PwChars,key);
					event->message = ((event->message >> 8)<<8) | bulletChar;
				}
				else
				{
					SysBeep(20);
					event->what = nullEvent;
				}
				return(False);
				break;
		}
	}
	else if (event->what==updateEvt)
	{
		if ((DialogPtr)event->message == dgPtr)
			HiliteButtonOne(dgPtr);
		else
			UpdateMyWindow((MyWindowPtr)event->message);
	}
	else
	{
		if (TickCount()%120<100 && CurrentModifiers()&alphaLock)
			ShowDItem(dgPtr,PASSWORD_WARNING);
		else
			HideDItem(dgPtr,PASSWORD_WARNING);
	}

	return(False);
}

/************************************************************************
 * CopyPassword - retrieve the password
 ************************************************************************/
void CopyPassword(UPtr password)
{
	BlockMove(*PwChars,password,**PwChars + 1);
	HPurge(PwChars);
}

/************************************************************************
 * MyAppendMenu - see that a menu item gets appended to a menu.  Avoids
 * menu manager meta-characters.
 ************************************************************************/
void MyAppendMenu(MenuHandle menu, UPtr name)
{
	MyInsMenuItem(menu,name,CountMItems(menu));
}

/************************************************************************
 * MyInsMenuItem - see that a menu item gets appended to a menu.	Avoids
 * menu manager meta-characters.
 ************************************************************************/
void MyInsMenuItem(MenuHandle menu, UPtr name, short afterItem)
{
	Str255 fixName;
	
	PCopy(fixName,"\pn");
	InsMenuItem(menu,fixName,afterItem);
	BlockMove(name+1,fixName+2,name[0]);
	fixName[0] = name[0] + 1;
	fixName[1] = '\0';
	SetItem(menu,afterItem+1,fixName);
}

/************************************************************************
 * MyGetItem - get the text of a menu item.  Strip leading NULL, if any
 ************************************************************************/
void MyGetItem(MenuHandle menu, short item, UPtr name)
{
	GetItem(menu,item,name);
	if (*name && !name[1])
	{
		BlockMove(name+2,name+1,name[0]-1);
		name[0]--;
	}
}

/************************************************************************
 * FindItemByName - find a named menu item
 ************************************************************************/
short FindItemByName(MenuHandle menu, UPtr name)
{
	short item;
	Str255 itemTitle;
	
	for (item=CountMItems(menu);item;item--)
	{
		MyGetItem(menu,item,itemTitle);
		if (EqualString(name,itemTitle,False,True)) break;
	}
	
	return(item);
}

/************************************************************************
 * BinFindItemByName - find a named menu item, using binary search
 ************************************************************************/
short BinFindItemByName(MenuHandle menu, UPtr name)
{
	short item;
	Str255 itemTitle;
	short first,last;
	short cmp;
	
	first = 1;
	last = CountMItems(menu);
	for (item=(first+last)/2;first<=last;item=(first+last)/2)
	{
		MyGetItem(menu,item,itemTitle);
		cmp = RelString(name,itemTitle,False,True);
		switch (cmp)
		{
			case -1:
				last = item-1;
				break;
			case 1:
				first = item+1;
				break;
			case 0:
				return(item);
		}
	}
	
	return(0);
}

/************************************************************************
 * SpecialKeys - mutate events involving particular keys
 ************************************************************************/
void SpecialKeys(EventRecord *event)
{
	long flags;
	uShort menu,item;
	
	switch(event->message & charCodeMask)
	{
		case homeChar:
		case endChar:
		case pageUpChar:
		case pageDownChar:
			event->what = app1Evt;
			break;
		default:
			switch((event->message & keyCodeMask)>>8)
			{
				case undoKey:
					menu = EDIT_MENU;
					item = EDIT_UNDO_ITEM;
					break;
				case cutKey:
					menu = EDIT_MENU;
					item = EDIT_CUT_ITEM;
					break;
				case copyKey:
					menu = EDIT_MENU;
					item = EDIT_COPY_ITEM;
					break;
				case pasteKey:
					menu = EDIT_MENU;
					item = EDIT_PASTE_ITEM;
					break;
				case clearKey:
					menu = EDIT_MENU;
					item = EDIT_CLEAR_ITEM;
					break;
				default:
					return;
			}
			EnableMenuItems();
			flags = (*GetMHandle(menu))->enableFlags;
			if ((flags & 1) && (flags & (1<<item)))
			{
				short oldMenu = TheMenu;
				HiliteMenu(menu);
				DoMenu(FrontWindow(),(menu<<16)|item,event->modifiers);
				if (oldMenu) HiliteMenu(oldMenu);
			}
			event->what = nullEvent;
			break;
	}
}

/************************************************************************
 * CurrentModifiers - return the current state of the modifers
 ************************************************************************/
short CurrentModifiers(void)
{
	EventRecord theEvent;
	
	OSEventAvail(nil,&theEvent);
	return(theEvent.modifiers);
}

/************************************************************************
 * AttachHierMenu - attach a hierarchical menu to a menu item
 ************************************************************************/
void AttachHierMenu(short menu,short item,short hierId)
{
	MenuHandle mh = GetMHandle(menu);
	SetItemCmd(mh,item,0x1b);
	SetItemMark(mh,item,hierId);
}

/************************************************************************
 * DirtyKey - does a keystroke cause a window to become dirty?
 ************************************************************************/
Boolean DirtyKey(long keyAndChar)
{
	short charCode = keyAndChar&charCodeMask;
	short keyCode = (keyAndChar&keyCodeMask)>>8;
	static short safeChars[]={
		homeChar,endChar,pageUpChar,pageDownChar,
		leftArrowChar,rightArrowChar,upArrowChar,downArrowChar};
	short *which;
	
	for (which=safeChars;which<safeChars+sizeof(safeChars)/sizeof(short);which++)
		if (charCode==*which) return(False);
	
	return(True);
}

/************************************************************************
 * LocalDateTimeStr - return a ctime format date and time,
 * but as a pascal string
 * <length>Sun Sep 16 01:03:52 1973\n\0
 * This is quite purposefully not internationally blessed
 ************************************************************************/
UPtr LocalDateTimeStr(UPtr string)
{
	DateTimeRec dtr;
	
	GetTime(&dtr);
	return(ComposeRString(string,DATE_STRING_FMT,
															 WEEKDAY_STRN+dtr.dayOfWeek,
															 MONTH_STRN+dtr.month,
															 dtr.day/10, dtr.day%10,
															 dtr.hour/10, dtr.hour%10,
															 dtr.minute/10, dtr.minute%10,
															 dtr.second/10, dtr.second%10,
															 dtr.year));
}

/************************************************************************
 * GMTDateTime - return the current seconds
 ************************************************************************/
long GMTDateTime(void)
{
	long secs;
	GetDateTime(&secs);
	secs -= ZoneSecs();
	return(secs);
}

/************************************************************************
 * LocalDateTime - return the current seconds
 ************************************************************************/
long LocalDateTime(void)
{
	long secs;
	GetDateTime(&secs);
	return(secs);
}

/************************************************************************
 * FixNewlines - remove cr, and turn nl into cr
 ************************************************************************/
void FixNewlines(UPtr string,long *count)
{
	char *from, *to;
	long n;
	
	for (to=from=string,n= *count;n;n--,from++)
		if (*from=='\012') *to++ = '\015';
		else if (*from!='\015') *to++ = *from;
	*count = to-string;
}

/************************************************************************
 * MiniEvents - call WNE, handle a few events, allow cmd-.
 ************************************************************************/
Boolean MiniEvents(void)
{
	EventRecord event;
	static long oldTicks;
	Boolean oldCommandPeriod = CommandPeriod;
	CommandPeriod = False;
	
	if (TickCount()-oldTicks > InBG ? 2 : 30)
	{
		if (WNE(everyEvent,&event,0))
		{
			(void) MiniMainLoop(&event);
			if (CommandPeriod) return(True);
		}
	}
	CommandPeriod = oldCommandPeriod;
	return(False);
}

/************************************************************************
 * GiveTime - allow for bg processing
 ************************************************************************/
void GiveTime(void)
{
	EventRecord tossMe;
	(void) WNE(0,&tossMe,0);
}

/************************************************************************
 * UnadornMessage - unadorn an event message
 ************************************************************************/
long UnadornMessage(EventRecord *event)
{
	long message;
	Handle curKCHR;
	static long state=0;
	short key;
	
	curKCHR = GetResource('KCHR',
											GetScript(GetEnvirons(smKeyScript),smScriptKeys));
	if (!curKCHR || !(event->modifiers&optionKey))
		message = event->message;
	else
	{
		/*
		 * get what it would have been without the option key
		 */
		key = ((event->message>>8)&0xff) | (event->modifiers&(~optionKey)&0xff00);
		message = KeyTrans(LDRef(curKCHR),key,&state);
		UL(curKCHR);
		
		/*
		 * now massage this into an event "message"
		 */
		message = (message & 0xff) | (event->message & 0xff00);
	}
	
	return(message);
}

/************************************************************************
 * MyMenuKey - fix MenuKey to ignore option key
 ************************************************************************/
long MyMenuKey(EventRecord *event)
{
	if (event->modifiers&cmdKey) EnableMenuItems();
	return(MenuKey(UnadornMessage(event)));
}

/************************************************************************
 * TrimPrefix - strip a prefix from a string
 ************************************************************************/
Boolean TrimPrefix(UPtr string, UPtr prefix)
{
	short oldLen = *string;

	if (oldLen < *prefix) return(False);
	
	*string = *prefix;
	if (EqualString(string,prefix,False,True))
	{
		BlockMove(string+1+*prefix,string+1,oldLen-*prefix);
		*string = oldLen - *prefix;
		return(True);
	}
	else
	{
		*string = oldLen;
		return(False);
	}
}

/************************************************************************
 * ChangeStrn - change a string in an Str# resource
 ************************************************************************/
UPtr ChangeStrn(short resId,short num,UPtr string)
{
	Handle resH = GetResource('STR#',resId);
	UPtr spot;
	short count;
	short i;
	long hSize;
	short diff;
	
	if (!resH) return(nil);
	spot = LDRef(resH);
	hSize = GetHandleSize(resH);
	count = 256*spot[0] + spot[1];
	if (num<=count)
	{
		spot += 2;
		for (i=1;i<num;i++) spot += *spot + 1;
		diff = *string-*spot;
		if (diff<0)
		{
			diff *= -1;
			if (num<count)
				BlockMove(spot+diff,spot,hSize-(spot-*resH)-diff);
			SetHandleBig(resH,hSize-diff);
		}
		else if (diff>0)
		{
			SetHandleBig(resH,hSize+diff);
			if (i=MemError()) {WarnUser(MEM_ERR,i);UL(resH);return(nil);}
			if (num<count)
				BlockMove(spot,spot+diff,hSize-(spot-*resH));
		}
	}
	else
	{
		UL(resH);
		SetHandleBig(resH,hSize+*string+num-count);
		if (i=MemError()) {WarnUser(MEM_ERR,i);return(nil);}
		spot = LDRef(resH);
		if (num>count+1)
			WriteZero(spot+hSize,num-count-1);
		spot[0] = num/256;
		spot[1] = num%256;
		spot += hSize + num-count-1;
	}
	hSize = GetHandleSize(resH);
	BlockMove(string,spot,*string+1);
	UL(resH);
	HNoPurge(resH);
	ChangedResource(resH);
	return(string);
}

/************************************************************************
 * RecountStrn - make sure an STR# resource really has the right number
 * of strings
 ************************************************************************/
void RecountStrn(short resId)
{
	Handle resH = GetResource('STR#',resId);
	UPtr spot,end;
	short count;
	short realCount=0;
	
	if (!resH) return;
	spot = *resH;
	end = spot+GetHandleSize(resH);
	count = spot[0]*256 + spot[1];
	for (spot+=2;spot<end;spot += *spot+1) realCount++;
	
	if (realCount!=count)
	{
		spot=*resH;
		spot[0] = realCount/256;
		spot[1] = realCount%256;
		ChangedResource(resH);
		HNoPurge(resH);
	}
}

/************************************************************************
 *
 ************************************************************************/
void NukeMenuItemByName(short menuId,UPtr itemName)
{
	MenuHandle mh=GetMHandle(menuId);
	short itemNum=FindItemByName(mh,itemName);
	
	if (itemNum) NukeMenuItem(mh,itemNum);
}

void NukeMenuItem(MenuHandle mh,short item)
{
	if (HasSubmenu(mh,item))
	{
		short subId,subItem;
		MenuHandle subMh;
		GetItemMark(mh,item,&subId);
		subMh=GetMHandle(subId);
		for (subItem=CountMItems(subMh);subItem;subItem--)
			NukeMenuItem(mh,subItem);
	}
	DelMenuItem(mh,item);
}

/************************************************************************
 *
 ************************************************************************/
void RenameItem(short menuId,UPtr oldName,UPtr newName)
{
	MenuHandle mh=GetMHandle(menuId);
	
	SetItem(mh,FindItemByName(mh,oldName),newName);
}

/************************************************************************
 *
 ************************************************************************/
Boolean HasSubmenu(MenuHandle mh, short item)
{
	short cmd;
	GetItemCmd(mh,item,&cmd);
	return(cmd==0x1b);
}

/************************************************************************
 * GreyControl - grey out a control
 ************************************************************************/
void GreyControl(ControlHandle cntl)
{
	Rect r = (*cntl)->contrlRect;
	InsetRect(&r,2,2);
	GreyOutRoundRect(&r,3,3);
}

/************************************************************************
 * SetGreyControl - grey a control, if it isn't already
 ************************************************************************/
Boolean SetGreyControl(ControlHandle cntl, Boolean shdBeGrey)
{
	if (((*cntl)->contrlRfCon=='GREY')!=shdBeGrey)
	{
		Rect r;
		(*cntl)->contrlRfCon = shdBeGrey ? 'GREY' : 0;
		r = (*cntl)->contrlRect;
		InsetRect(&r,2,2);
		InvalRect(&r);
	}
	return(shdBeGrey);
}

/************************************************************************
 * IsAUX - is A/UX running?
 ************************************************************************/
Boolean IsAUX(void)
{
	return(((*(short *)0xb22)&(1<<9))!=0);
}

/************************************************************************
 * ZoneSecs - get the timezone offset, in seconds
 ************************************************************************/
long ZoneSecs(void)
{
	MachineLocation ml;
	long delta;
#define GMTDELTA gmtFlags.gmtDelta
	ReadLocation(&ml);
	if (ml.latitude==ml.longitude && ml.GMTDELTA==ml.longitude && ml.GMTDELTA==0)
		return(-1);

	delta = ml.GMTDELTA&0xffffff;
	if ((delta>>23)&1) delta |= 0xff000000;
	
	return(delta);
}

/************************************************************************
 * WNE - call WaitNextEvent.	Set InBG properly.
 ************************************************************************/
Boolean WNE(short eventMask,EventRecord *event,long sleep)
{
	EventRecord localEvt;
	Boolean result;
	short mods = CurrentModifiers();
#ifdef NEVER
	static RgnHandle oldMouseRgn;
	if (!oldMouseRgn) oldMouseRgn = NewRgn();
	if (BUG1 && !EqualRgn(MouseRgn,oldMouseRgn))
	{
		ShowGlobalRgn(MouseRgn,"\pWNE MouseRgn");
		CopyRgn(MouseRgn,oldMouseRgn);
	}
#endif
	
	result = WaitNextEvent(eventMask,&localEvt,sleep,MouseRgn);
#ifdef DEBUG
	if (LogLevel&LOG_EVENT)
	{
		if (localEvt.what==keyDown)
			ComposeLogS(LOG_EVENT,nil,"\p%d %d %d ",
				localEvt.what,localEvt.message&0xff,localEvt.modifiers);
		else if (localEvt.what==mouseDown)
			ComposeLogS(LOG_EVENT,nil,"\p%d %d %d %d",
				localEvt.what,localEvt.modifiers,localEvt.where.h,localEvt.where.v);
	}
#endif
	
	if (mods!=localEvt.modifiers) SFWTC = True;

	if (localEvt.what==app4Evt && ((localEvt.message)>>24)&1 == 1)
		InBG = ((localEvt.message&1) != 1); /* suspend/resume */
	else if (HasPM)
	{
		ProcessSerialNumber me, him;								/* System 7 */
		GetCurrentProcess(&me);
		GetFrontProcess(&him);
		InBG = me!=him;
	}
	if (event) *event=localEvt;
	return(result);
}

/************************************************************************
 * Provide the same benefit as a politician
 ************************************************************************/
void NOOP(void) {}
/************************************************************************
 * DlgUpdate - update a dialog window
 ************************************************************************/
void DlgUpdate(MyWindowPtr win)
{
	UpdtDialog(win,((GrafPtr)win)->visRgn);
}

/************************************************************************
 * RoundDiv - Divide with rounding away from the origin
 ************************************************************************/
long RoundDiv(long quantity,long unit)
{
  if (quantity<0) quantity -= unit-1;
	else quantity += unit-1;
	return(quantity/unit);
}

/************************************************************************
 * TZName2Offset - interpret the time zone with a resource
 ************************************************************************/
long TZName2Offset(CStr zoneName)
{
	UPtr this,end;
	Handle tznH = GetResource('zon#',TZ_NAMES);
	long offset = 0;
	Str15 pName;
	
	CtoPCpy(pName,zoneName);
	
	if (tznH)
	{
		this = LDRef(tznH);
		end = this + GetHandleSize(tznH);
		for (;this<end;this+=*this+1+2*sizeof(short))
			if (EqualString(this,pName,False,True))
			{
				short hrs,mins;
				this += *this +1;
				hrs = this[0]*256+this[1];
				mins = this[2]*256+this[3];
				offset = hrs*3600 + 60*mins;
				break;
			}
		UL(tznH);
	}
	return(offset);
}

/************************************************************************
 * CenterRectIn - center one rect in another
 ************************************************************************/
void CenterRectIn(Rect *inner,Rect *outer)
{
	OffsetRect(inner,(outer->left+outer->right-inner->left-inner->right)/2, 
									 (outer->top+outer->bottom-inner->top-inner->bottom)/2);																	
}

/************************************************************************
 * TopCenterRectIn - center one rect in (the bottom of) another
 ************************************************************************/
void TopCenterRectIn(Rect *inner,Rect *outer)
{
	OffsetRect(inner,(outer->left+outer->right-inner->left-inner->right)/2, 
									 outer->top-inner->top);																	
}

/************************************************************************
 * BottomCenterRectIn - center one rect in (the bottom of) another
 ************************************************************************/
void BottomCenterRectIn(Rect *inner,Rect *outer)
{
	OffsetRect(inner,(outer->left+outer->right-inner->left-inner->right)/2, 
									 outer->bottom-inner->bottom);																	
}

/************************************************************************
 * ThirdCenterRectIn - center one rect in (the top 1/3 of) another
 ************************************************************************/
void ThirdCenterRectIn(Rect *inner,Rect *outer)
{
	OffsetRect(inner,(outer->left+outer->right-inner->left-inner->right)/2, 
									 outer->top-inner->top +
									 (outer->bottom-outer->top-inner->bottom+inner->top)/3);																	
}

/************************************************************************
 * MyLClick - call LClick, but disallow cmd-double-click
 ************************************************************************/
Boolean MyLClick(Point pt,short modifiers,ListHandle lHandle)
{
	return(LClick(pt,modifiers,lHandle)&!(modifiers&cmdKey));
}

#pragma segment StringUtils
/**********************************************************************
 * return a string from an STR# resource
 **********************************************************************/
UPtr GetRString(UPtr theString,int theIndex)
{
	StringHandle s;
	short oldRes = CurResFile();
	if (SettingsRefN) UseResFile(SettingsRefN);
	if (theIndex != 1001 && (s=GetString(theIndex)) && HomeResFile(s))
	{
		PCopy(theString,*s);
		ReleaseResource(s);
	}
	else
	{
		theString[0] = 0;
		GetIndString(theString,200*(theIndex/200),theIndex%200);
	}
	theString[*theString+1] = 0;
	UseResFile(oldRes);
	return(theString);
}

/**********************************************************************
 * copy a pascal string into a c string
 **********************************************************************/
UPtr PtoCcpy(UPtr cStr, UPtr pStr)
{
	BlockMove(pStr+1,cStr,*pStr);
	cStr[*pStr] = 0;
	return(cStr);
}

/**********************************************************************
 * concatenate a pascal string on the end of another
 **********************************************************************/
UPtr PCat(UPtr string,UPtr suffix)
{
	short sufLen;
	
	sufLen = *suffix;
	
	BlockMove(suffix+1,string+*string+1,sufLen);
	*string += sufLen;
	
	return(string);
}

/**********************************************************************
 * concatenate a pascal string on the end of another
 * escape certain chars in the string
 **********************************************************************/
UPtr PEscCat(UPtr string, UPtr suffix, short escape, char *escapeWhat)
{
	short sufLen;
	char *suffSpot,*stringSpot;
	
	sufLen = *suffix;
	stringSpot = string+*string+1;
	
	for (suffSpot=suffix+1;sufLen--;suffSpot++)
	{
		if (*suffSpot==escape || strchr(escapeWhat,*suffSpot))
			*stringSpot++ = escape;
		*stringSpot++ = *suffSpot;
	}
	*string = stringSpot-string-1;
	
	return(string);
}

/**********************************************************************
 * Get a long out of a resource file
 **********************************************************************/
long GetRLong(int index)
{
	Str255 scratch;
	long aLong;
	
	if (GetRString(scratch,index)==nil)
		return(0L);
	else
	{
		StringToNum(scratch,&aLong);
		return(aLong);
	}
}

/**********************************************************************
 * striscmp - compare two strings, up to the length of the shorter string,
 * and ignoring case
 **********************************************************************/
#define isupper(c)	('A'<=c && c<='Z')
#define tolower(c)	(c + ('a'-'A'))
int striscmp(UPtr s1,UPtr s2)
{
	register c1, c2;
	for (c1= *s1, c2= *s2; c1 && c2; c1 = *++s1, c2= *++s2)
	{
		if (isupper(c1)) c1=tolower(c1);
		if (isupper(c2)) c2=tolower(c2);
		if (c1-c2) return (c1-c2);
	}
	return(0);
}

/**********************************************************************
 * TrimWhite - remove whitespace characters from the end of a string
 **********************************************************************/
void TrimWhite(PStr s)
{
	register int len=*s;
	register UPtr cp=s+len;
	
	while (len && isspace(*cp)) cp--,len--;
	
	*s = len;
}

/**********************************************************************
 * TrimInitialWhite - remove whitespace characters from the beginning of a string
 **********************************************************************/
void TrimInitialWhite(PStr s)
{
	UPtr cp=s+1;
	short len;
	
	for (cp=s+1;cp<=s+*s && isspace(*cp);cp++);
	if (cp>s+1 && cp<=s+*s)
	{
		len = *s - (cp-(s+1));
		BlockMove(cp,s+1,len);
		*s = len;
	}
}

/************************************************************************
 * Tokenize - set pointers to the beginning and end of a delimited token
 ************************************************************************/
UPtr Tokenize(UPtr string, int size, UPtr *start, UPtr *end, UPtr delims)
{
	UPtr stop = string+size;
	char safe = *stop;
	UPtr last;
	
	*stop = 0;
	while (strchr(delims,*string)) string++;
	*stop = *delims;
	for (last=string; !strchr(delims,*last); last++);
	*stop = safe;
	if (string==stop) return(nil);
	*start = string;
	*end = stop;
	return(string);
}


/************************************************************************
 * RemoveSpaces - remove the spaces from some text
 ************************************************************************/
long RemoveSpaces(UPtr text,long size)
{
	UPtr from, to, limit;
	
	for (to=text,limit=text+size;*to!=' ' && to<limit;to++);
	if (to<limit)
		for (from=to;from<limit;from++) if (*from!=' ') *to++ = *from;
	return(to-text);
}

/************************************************************************
 * GetRStr - get a string from an 'STR ' resource
 ************************************************************************/
UPtr GetRStr(UPtr string,short id)
{
	Handle strH = GetString(id);
	if (strH)
	{
		PCopy(string,*strH);
		ReleaseResource(strH);
	}
	else
		*string = 0;
	return(string);
}

/************************************************************************
 * atoi - turn a string into an integer
 ************************************************************************/
long atoi(UPtr p)
{
	long sign=1;
	long value=0;
	
	while (isspace(*p)) p++;
	if (*p=='-') {sign = -1; p++;}
	else if (*p=='+') p++;
	
	while (isdigit(*p))
	{
		value *= 10;
		value += *p - '0';
		p++;
	}
	
	return(value*sign);
}

/************************************************************************
 * PLCat - concat a long onto a string (preceed it with a space)
 ************************************************************************/
UPtr PLCat(UPtr string,long num)
{
	short n;			/* length of old string + 1 */
	n = *string+1;
	NumToString(num,string+n);
	string[0] += string[n]+1;
	string[n] = ' ';
	return(string);
}

/************************************************************************
 * EndsWith - does one string end with another?
 ************************************************************************/
Boolean EndsWith(PStr name,PStr suffix)
{
	Boolean res;
	Byte c;
	UPtr spot;
	if (*name<*suffix) return(False);			/* too short */
	
	spot = name + *name - *suffix ;				/* before start of putative suffix */
	c = *spot;														/* save byte */
	*spot = *suffix;											/* pretend equal length */
	res = EqualString(suffix,spot,False,True);
	*spot = c;														/* restore byte */
	return(res);
}

/************************************************************************
 * RemoveParens - remove parenthesized information
 ************************************************************************/
void RemoveParens(UPtr string)
{
	UPtr to, from, end;
	short pLevel=0;
	
	for (to=from=string+1,end=string+*string; from<=end; from++)
		switch(*from)
		{
			case '(':
				pLevel++;
				break;
			case ')':
				if (pLevel) pLevel--;
				else *to++ = *from;
				break;
			case ' ':
				if (!pLevel) break;
				/* fall through is deliberate */
			default:
				*to++ = *from;
				break;
		}
	*string = to-string-1;
}

/************************************************************************
 * ComposeString - sprintf, only smaller
 * %s - c string
 * %d - int
 * %c - char (int)
 * %p - pascal string
 * %i - internet address
 * %I - internet address, turned into hostname
 * %r - string from a resource
 ************************************************************************/
UPtr ComposeString(UPtr into,UPtr format,...)
{
	va_list args;
	va_start(args,format);
	(void) VaComposeString(into,format,args);
	va_end(args);
	return(into);
}
UPtr ComposeRString(UPtr into,short format,...)
{
	va_list args;
	va_start(args,format);
	(void) VaComposeRString(into,format,args);
	va_end(args);
	return(into);
}
UPtr VaComposeRString(UPtr into,short format,va_list args)
{
	Str255 stringFormat;
	
	GetRString(stringFormat,format);
	return (VaComposeString(into,stringFormat,args));
}
UPtr VaComposeString(UPtr into,UPtr format,va_list args)
{
	UPtr intoP;
	UPtr formatP;
	Str255 scratch;
	Uptr argString;
	struct hostInfo hi;
	short n;
	
	intoP = into+1;
	for (formatP=format+1;formatP<format+*format+1;formatP++)
		if (*formatP!='%')
			*intoP++ = *formatP;
		else
		{
			formatP++;
			switch (*formatP)
			{
				case 'c':
					*intoP++ = va_arg(args,unsigned int);
					break;
				case '%':
					*intoP++ = '%';
					break;
				case 's':
					argString = va_arg(args,UPtr);
					while (*argString) *intoP++ = *argString++;
					break;
				default:
					switch (*formatP)
					{
						case 'p':
							argString = va_arg(args,UPtr);
							break;
						case 'i':
							argString = NumToDot(va_arg(args,unsigned int),scratch);
							break;
						case 'I':
							argString = scratch;
							if (!GetHostByAddr(&hi,va_arg(args,unsigned int)))
							{
								*scratch = strlen(hi.cname);
								BlockMove(hi.cname,scratch+1,*scratch+1);
							}
							else
								GetRString(scratch,UNTITLED);
							break;
						case 'd':
							NumToString(va_arg(args,int),scratch);
							argString=scratch;
							break;
						case 'r':
							GetRString(scratch,va_arg(args,int));
							argString=scratch;
							break;
#ifdef DEBUG
						case 'b':
							n = va_arg(args,int);
							*scratch = 32;
							for (argString=scratch+32;argString>scratch;argString--)
							{
								*argString = n&1 ? '1' : '0';
								n >>= 1;
							}
							break;
#endif
					}
					n = *argString;
					for (argString++;n--;argString++) *intoP++ = *argString;
					break;
			}
		}
	*into = intoP-into-1;
	*intoP = 0;
	return(into);
}

/************************************************************************
 * TransLitString - transliaterate with the default viewing table
 ************************************************************************/
void TransLitString(UPtr string)
{
	short id = GetRLong(PREF_IN_XLATE+PREF_STRN);
	Handle xlh;
	UPtr end;
	UPtr table;
	
	if (id!=NO_TABLE && (xlh=GetResource('taBL',id)))
	{
		table = LDRef(xlh);
		for (end = string+*string;end>string;end--)
			*end = table[*end];
		UL(xlh);
	}
}
