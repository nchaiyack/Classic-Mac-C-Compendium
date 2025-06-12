#define FILE_NUM 32
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * functions for dealing with printing
 ************************************************************************/
#pragma load EUDORA_LOAD
#pragma segment Print

/************************************************************************
 * private function declarations
 ************************************************************************/
	int PrintMessage(MyWindowPtr win,Boolean isOut, Rect *uRect, Boolean select);
	void PrintMessageHeader(UPtr title, short pageNum, short height, short bottom, short left, short right);
	int PrintMessagePage(UPtr title, UHandle text,long size,long *offset,Rect *uRect,short hMar,short pageNum);
	int PrintCompMessage(UPtr title, MessType **messH,Rect *uRect,short hMar);
	void CollectReturnAddr(UPtr returnAddr);
	void SetupPrintFont(void);
	void GetURect(Rect *useRect);
/************************************************************************
 * PrintSelectedMessages - print out a given message.  Handles the print
 * manager stuff.
 ************************************************************************/
int PrintSelectedMessages(TOCType **tocH,Boolean select)
{
	int err, cumErr=0;
	TPPrPort port;
	GrafPtr oldPort;
	TPrStatus status;
	int sumNum;
	MessType **messH;
	Boolean isOut;
	Str31 outName;
	Rect uRect;
	
	select = select || 0!=(CurrentModifiers()&shiftKey);
	
	GetPort(&oldPort);
	PrOpen();
	if (PrError())
	{
		WarnUser(NO_PRINTER,PrError());
		return(PrError());
	}
	
	if (PageSetup==nil) PageSetup = GetResource(PRINT_RTYPE,PRINT_CSOp);
	if (PageSetup==nil)
	{
		PageSetup = NuHandle(sizeof(TPrint));
		if (PageSetup==nil) {WarnUser(COULDNT_SETUP,err=MemError()); goto done;}
		PrintDefault(PageSetup);
	}
	else
		PrValidate(PageSetup);
	
	PushCursor(arrowCursor);
	if (PrJobDialog(PageSetup))
	{
		PopCursor();
		port = PrOpenDoc(PageSetup,nil,nil);
		if (!port) {WarnUser(NO_PPORT,err=MemError()); goto done;}
		GetURect(&uRect);
		isOut = EqualString(GetRString(outName,OUT),LDRef(tocH)->name,False,True);
		UL(tocH);
		for (sumNum=0;sumNum<(*tocH)->count;sumNum++)
			if ((*tocH)->sums[sumNum].selected)
			{
				messH = (*tocH)->sums[sumNum].messH;
				if (messH || !(err=!GetAMessage(tocH,sumNum,nil,False)))
				{
					SetPort(port);
					err = PrintMessage((*(MessHandle)(*tocH)->sums[sumNum].messH)->win,isOut,&uRect,select);
					if (!messH)
						CloseMyWindow((*(MessType **)(*tocH)->sums[sumNum].messH)->win);
					if (err) cumErr = err;
				}
			}
		PrCloseDoc(port);
		if ((*PageSetup)->prJob.bJDocLoop==bSpoolLoop && !err && !PrError())
			PrPicFile(PageSetup,nil,nil,nil,&status);
		if (cumErr) WarnUser(PART_PRINT_FAIL,err);
	}
	else
		PopCursor();
done:
	PrClose();
	FigureOutFont();
	SetPort(oldPort);
	return(err);
}

/************************************************************************
 * PrintOneMessage - print out a given message.  Handles the print manager
 * stuff.
 ************************************************************************/
int PrintOneMessage(MyWindowPtr win,Boolean select)
{
	int err;
	TPPrPort port;
	GrafPtr oldPort;
	TPrStatus status;
	Rect uRect;
	Boolean isOut;
	
	select = select || 0!=(CurrentModifiers()&shiftKey);
	
	GetPort(&oldPort);
	PrOpen();
	if (PrError())
	{
		WarnUser(NO_PRINTER,PrError());
		return(PrError());
	}
	
	if (PageSetup==nil) PageSetup = GetResource(PRINT_RTYPE,PRINT_CSOp);
	if (PageSetup==nil)
	{
		PageSetup = NuHandle(sizeof(TPrint));
		if (PageSetup==nil) {WarnUser(COULDNT_SETUP,err=MemError()); goto done;}
		PrintDefault(PageSetup);
	}
	else
		PrValidate(PageSetup);
	
	PushCursor(arrowCursor);	
	if (PrJobDialog(PageSetup))
	{
		PopCursor();
		port = PrOpenDoc(PageSetup,nil,nil);
		GetURect(&uRect);
		if (!port) {WarnUser(NO_PPORT,err=MemError()); goto done;}
		isOut = win->qWindow.windowKind==COMP_WIN;
		SetPort(port);
		err = PrintMessage(win,isOut,&uRect,select);
		PrCloseDoc(port);
		if ((*PageSetup)->prJob.bJDocLoop==bSpoolLoop && !err && !PrError())
			PrPicFile(PageSetup,nil,nil,nil,&status);
		if (err) WarnUser(PRINT_FAILED,err);
	}
	else
		PopCursor();
done:
	PrClose();
	FigureOutFont();
	SetPort(oldPort);
	return(err);
}

/************************************************************************
 * PrintMessage - print a given message, assuming everything is already
 * set up with the print manager.
 ************************************************************************/
int PrintMessage(MyWindowPtr win,Boolean isOut, Rect *uRect,Boolean select)
{
	long offset = 0;
	short hMar = GetRLong(PRINT_H_MAR);
	int err;
	short pageNum=0;
	Str255 title;
	GrafPtr port;
	short device = ((*PageSetup)->prStl.wDev>>8)&0xff;
	Boolean isMessage;
	TEHandle teh;
	
	isMessage = win->qWindow.windowKind==COMP_WIN ||
							win->qWindow.windowKind==MESS_WIN;
	teh = isMessage ? Win2Body(win) : WinTEH(win);
	
#define bDevLaser 3
	
	GetPort(&port);
	
	SetupPrintFont();
	TextFont(FontID);
	TextSize(FontSize);
	PCopy(title,*win->qWindow.titleHandle);
	if (!isOut)
	{
		long size = (*teh)->teLength;
		if (select && (*teh)->selStart<(*teh)->selEnd)	/* print selection */
		{
			offset = (*teh)->selStart;
			size = (*teh)->selEnd;
		}
		do
		{
			GiveTime();
			err=PrintMessagePage(title,(*teh)->hText,size,
																							 &offset,uRect,hMar,++pageNum);
		}
		while (!err && offset < size);
	}
	else
		err = PrintCompMessage(title,(MessHandle)win->qWindow.refCon,uRect,hMar);
	if (!err && isMessage && SumOf(Win2MessH(win))->state == UNREAD)
		SetState((*Win2MessH(win))->tocH,(*Win2MessH(win))->sumNum,READ);
	return(err);
}

/************************************************************************
 * PrintMessagePage - print a pageful of the current message
 ************************************************************************/
int PrintMessagePage(UPtr title, UHandle text,long size,long *offset,Rect *uRect,short hMar,short pageNum)
{
	short baseV=uRect->top+hMar+FontLead;
	short lastBase = uRect->bottom-hMar-FontDescent;
	Str255 returnAddr;
	UPtr begin,spot,end,wrap,space;
	short width=uRect->right-uRect->left;
	
	CollectReturnAddr(returnAddr);
	PrOpenPage(qd.thePort,nil);
	PrintMessageHeader(title,pageNum,hMar,
																uRect->top+hMar,uRect->left,uRect->right);
	begin = LDRef(text) + *offset;
	end = *text + size;
	for (; begin<end && baseV<lastBase; baseV+=FontLead,begin=spot)
	{
		/*
		 * point end at the end of the current line
		 */
		for (spot=begin;spot<end;spot++) if (*spot=='\n') break;
		wrap = begin + CalcTextTrunc(begin,spot-begin,width,qd.thePort);
		if (wrap>end) wrap = end;
		space = nil;
		for (spot=begin; spot<wrap; spot++) if (*spot==' ') space=spot;
		if (*spot!='\n')
		{
			if (spot!=end)
				spot = space ? space : spot+1;
		}
		
		if (spot>begin)
		{
			MoveTo(uRect->left,baseV);
			DrawText(begin,0,spot-begin);
		} 

		if (*spot=='\n')
			spot++; /* skip newline */
		else
			while (spot<end && *spot==' ') spot++; /* skip trailing blanks */
	}
	*offset = spot-*text;
	HUnlock(text);
	PrintMessageHeader(returnAddr,pageNum,hMar,
																uRect->bottom,uRect->left,uRect->right);
	PrClosePage(qd.thePort);
	return(PrError());
}

/************************************************************************
 * PrintCompMessage - print a composition message
 * this is an ugly hack--so shoot me.
 ************************************************************************/
int PrintCompMessage(UPtr title, MessType **messH,Rect *uRect,short hMar)
{
	short baseV;
	short lastBase = uRect->bottom-hMar-FontDescent;
	int err;
	int pageNum=1;
	Str31 headerName;
	Str255 returnAddr;
	int hOffset;
	UPtr begin=nil,spot,end,wrap,space;
	int tx=0;
	
	CollectReturnAddr(returnAddr);
	GetRString(headerName,HEADER_STRN+ATTACH_HEAD);
	hOffset = StringWidth(headerName) + FontWidth;
	baseV=uRect->top+hMar+FontLead;
	PrOpenPage(qd.thePort,nil);
	PrintMessageHeader(title,pageNum,hMar,
																uRect->top+hMar,uRect->left,uRect->right);
	do
	{
		if (!begin)
		{
			begin = LDRef((*(*messH)->txes[tx])->hText);
			end = begin + (*(*messH)->txes[tx])->teLength;
			if (tx==HEAD_LIMIT-1)
			{
				baseV += FontLead;
				hOffset = 0;
				*headerName = 0;
			}
			else
				GetRString(headerName,HEADER_STRN+tx+1);
		}
		else
		{
			baseV=uRect->top+hMar+FontLead;
			PrOpenPage(qd.thePort,nil);
			PrintMessageHeader(title,pageNum,hMar,
																		uRect->top+hMar,uRect->left,uRect->right);
		}
		for (; begin<end && baseV<lastBase; baseV+=FontLead,begin=spot)
		{
			/*
				* point end at the end of the current line
				*/
			for (spot=begin;spot<end;spot++) if (*spot=='\n') break;
			wrap = begin + CalcTextTrunc(begin,spot-begin,
											 uRect->right-uRect->left-hOffset,qd.thePort);
			if (wrap>end) wrap = end;
			space = nil;
			for (spot=begin; spot<wrap; spot++) if (*spot==' ') space=spot;
			if (*spot!='\n')
			{
				if (spot!=end)
					spot = space ? space : spot+1;
			}
			
			if (spot>begin)
			{
				if (*headerName)
				{
					MoveTo(uRect->left+hOffset-StringWidth(headerName)-FontWidth,baseV);
					DrawString(headerName);
					DrawChar(' ');
					*headerName=0;
				}
				MoveTo(uRect->left+hOffset,baseV);
				DrawText(begin,0,spot-begin);
			} 
	
			if (*spot=='\n')
				spot++; /* skip newline */
			else
				while (spot<end && *spot==' ') spot++; /* skip trailing blanks */
		}
		if (begin>=end)
		{
			HUnlock((*(*messH)->txes[tx])->hText);
			tx++;
			begin=nil;
		}
		else
		{
			PrintMessageHeader(returnAddr,pageNum++,hMar,
																		uRect->bottom,uRect->left,uRect->right);
			PrClosePage(qd.thePort);
		}
	}
	while (!(err=PrError()) && tx<HEAD_LIMIT);
	PrintMessageHeader(returnAddr,pageNum,hMar,
																uRect->bottom,uRect->left,uRect->right);
	PrClosePage(qd.thePort);
	return(PrError());
}

/************************************************************************
 * PrintMessageHeader - print a header with the message info in it.
 ************************************************************************/
void PrintMessageHeader(UPtr title, short pageNum, short height, short bottom, short left, short right)
{
	Rect border;
	short h,v;
	Str31 pageStr;
	short pageStrWidth;
	GrafPtr port;
	short oldFontId;
	
	SetRect(&border,left,bottom-height,right,bottom);
	FillRect(&border,&qd.gray);
	InsetRect(&border,1,1);
	EraseRect(&border);
	GetPort(&port);
	oldFontId = port->txFont;
	
	v = bottom - height/4;
	TextFace(bold);
	TextSize(GetRLong(PRINT_H_SIZE));
	TextFont(GetFontID(GetRString(pageStr,PRINT_H_FONT)));
	NumToString(pageNum,pageStr);
	pageStrWidth = StringWidth(pageStr);
	h = right - height - pageStrWidth;
	MoveTo(h,v);
	DrawString(pageStr);
	h = left + height;
	MoveTo(h,v);
	DrawText(title,1,CalcTrunc(title,right-3*height-pageStrWidth-left,port));
	
	TextFont(oldFontId);
	TextFace(0);
	TextSize(FontSize);
}

/************************************************************************
 * CollectReturnAddr - collect the return addr for the bottoms of printouts
 ************************************************************************/
void CollectReturnAddr(UPtr returnAddr)
{
	short len;
	char saveChar;
	
	GetRString(returnAddr,RETURN_PRINT_INTRO);
	len = *returnAddr;
	saveChar = returnAddr[len];
	GetReturnAddr(returnAddr+len,True);
	*returnAddr += returnAddr[len];
	returnAddr[len] = saveChar;
}

/************************************************************************
 * SetupPrintFont - set up the font to use for printing
 ************************************************************************/
void SetupPrintFont(void)
{
	Str31 scratch;
	short fID,fSize;
	short device = ((*PageSetup)->prStl.wDev>>8)&0xff;
	
	GetPref(scratch,PREF_PRINT_FONT);
	fID = *scratch ? GetFontID(GetPref(scratch,PREF_PRINT_FONT))
								 : FontIsFixed ?
									 GetFontID(GetRString(scratch,PRINT_FONT)) : FontID;
	GetPref(scratch,PREF_PRINT_FONT_SIZE);
	fSize = *scratch ? GetRLong(PREF_STRN+PREF_PRINT_FONT_SIZE) : FontSize;
	if (fSize!=FontSize || fID!=FontID)
	{
		FontID = fID;
		FontSize = fSize;
		FontWidth = GetWidth(FontID,FontSize);
		FontLead = GetLeading(FontID,FontSize);
		FontDescent = GetDescent(FontID,FontSize);
		FontAscent = GetAscent(FontID,FontSize);
		FontIsFixed = IsFixed(FontID,FontSize);
	}
}

/************************************************************************
 *
 ************************************************************************/
void GetURect(Rect *useRect)
{
	short t;
	Rect uRect;
	uRect = (*PageSetup)->prInfo.rPage;
	InsetRect(&uRect,3,3);	/* Apple lies about page size sometimes */
	t = (*PageSetup)->rPaper.left+GetRLong(PRINT_LEFT_MAR);
	uRect.left = MAX(uRect.left,t);
	t = (*PageSetup)->rPaper.right-GetRLong(PRINT_RIGHT_MAR);
	uRect.right = MIN(uRect.right,t);
	*useRect = uRect;
}
