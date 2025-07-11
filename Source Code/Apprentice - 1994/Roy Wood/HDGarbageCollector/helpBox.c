static pascal void HelpScrollProc(ControlHandle theControl,int theCode)
{
int			scrollAmt,height;
int			controlMax,controlMin,controlVal;
TEHandle	whichTEH;

	whichTEH = (TEHandle) GetCRefCon(theControl);
	
	controlMax = GetCtlMax(theControl);
	controlMin = GetCtlMin(theControl);
	controlVal = GetCtlValue(theControl);
	
	switch (theCode) 
	{
		case inUpButton:
			if (controlVal > controlMin)
			{
				SetCtlValue(theControl,controlVal-1);
				
				TEScroll(0,TEGetHeight(controlVal-1,controlVal-1,whichTEH),whichTEH);
			}
			
			break;
			
		case inDownButton: 
			if (controlVal < controlMax)
			{
				SetCtlValue(theControl,controlVal+1);
				
				TEScroll(0,-TEGetHeight(controlVal,controlVal,whichTEH),whichTEH);
			}
			
			break;

		case inPageUp: 
			if (controlVal > controlMin)
			{
				scrollAmt = 1;
				height = (**whichTEH).viewRect.bottom - (**whichTEH).viewRect.top;
				
				while (controlVal-scrollAmt>controlMin && TEGetHeight(controlVal-scrollAmt,controlVal-1,whichTEH)<height)
					scrollAmt++;
				
				if (scrollAmt>1  && TEGetHeight(controlVal-scrollAmt,controlVal-1,whichTEH)>height)
					scrollAmt--;
				
				SetCtlValue(theControl,controlVal-scrollAmt);
				
				TEScroll(0,TEGetHeight(controlVal-scrollAmt,controlVal-1,whichTEH),whichTEH);
			}
			
			break;

		case inPageDown: 
			if (controlVal < controlMax)
			{
				scrollAmt = 1;
				height = (**whichTEH).viewRect.bottom - (**whichTEH).viewRect.top;
				
				while (controlVal+scrollAmt<controlMax && TEGetHeight(controlVal,controlVal+scrollAmt-1,whichTEH)<height)
					scrollAmt++;
				
				if (scrollAmt>1  && TEGetHeight(controlVal,controlVal+scrollAmt-1,whichTEH)>height)
					scrollAmt--;
				
				SetCtlValue(theControl,controlVal+scrollAmt);
				
				TEScroll(0,-TEGetHeight(controlVal,controlVal+scrollAmt-1,whichTEH),whichTEH);
			}
			
			break;
	}
}



void DoHelpBox(int helpResWIND,int helpResTEXT,int helpResSTYL)
{
GrafPtr			oldPort,helpWPtr;
TEHandle		helpTEH;
Rect			destRect,viewRect,vScrollRect,doneRect;
Handle			helpText,helpStyle;
ControlHandle	vScrollControl,doneControl,whichControl;
EventRecord		theEvent;
int				cntlCode,controlVal,oldVal,height;
char			theChar;


	GetPort(&oldPort);
	
	helpWPtr = GetNewWindow(helpResWIND,0L,(WindowPtr) -1L);
	SetPort(helpWPtr);
	
	CenterWindow(helpWPtr);
	ShowWindow(helpWPtr);
	
	TextFont(geneva);
	TextSize(10);
	TextFace(0);
	
	SetRect(&viewRect,helpWPtr->portRect.left,helpWPtr->portRect.top,helpWPtr->portRect.right-15,helpWPtr->portRect.bottom-35);
	InsetRect(&viewRect,5,5);
	destRect = viewRect;
	
	
	helpTEH = TEStylNew(&destRect,&viewRect);
	
	(**helpTEH).lineHeight = -1;
	(**helpTEH).fontAscent = -1;
	
	InsetRect(&viewRect,-5,-5);
	FrameRect(&viewRect);
	
	SetRect(&vScrollRect,viewRect.right-1,viewRect.top,helpWPtr->portRect.right,viewRect.bottom);
	vScrollControl = NewControl(helpWPtr,&vScrollRect,"\p",TRUE,1,1,1,scrollBarProc,(long) helpTEH);
	SetRect(&vScrollRect,viewRect.right-1,viewRect.top+16,helpWPtr->portRect.right,viewRect.bottom-16);
	
	SetRect(&doneRect,helpWPtr->portRect.right-55,helpWPtr->portRect.bottom-25,helpWPtr->portRect.right-5,helpWPtr->portRect.bottom-5);
	doneControl = NewControl(helpWPtr,&doneRect,"\pDone",TRUE,0,0,1,pushButProc,0L);
	
	PenNormal();
	PenSize(3,3);
	InsetRect(&doneRect,-4,-4);
	FrameRoundRect(&doneRect,16,16);
	InsetRect(&doneRect,4,4);
	PenSize(1,1);
	
	helpText = GetResource('TEXT',helpResTEXT);
	helpStyle = GetResource('styl',helpResSTYL);
	
	if (helpText!=0L && helpStyle!=0L)
	{
		HLock(helpText);
		
		TEStylInsert(*helpText,SizeResource(helpText),helpStyle,helpTEH);
		
		HUnlock(helpText);
		ReleaseResource(helpText);
		ReleaseResource(helpStyle);
		
		
		oldVal = (**helpTEH).nLines;
		controlVal = 1;
		height = (**helpTEH).viewRect.bottom - (**helpTEH).viewRect.top;
		
		while (oldVal-controlVal>1 && TEGetHeight(oldVal-controlVal,oldVal-1,helpTEH)<height)
			controlVal++;
		
		if (controlVal>1  && TEGetHeight(oldVal-controlVal,oldVal-1,helpTEH)>height)
			controlVal--;
		
		SetCtlMax(vScrollControl,oldVal-controlVal);
		
		BeginUpdate(helpWPtr);
		EndUpdate(helpWPtr);
		
		do
		{
			SystemTask();
			
			cntlCode = 0;
			
			if (GetNextEvent(everyEvent,&theEvent))
			{
				if (theEvent.what == mouseDown)
				{
					GlobalToLocal(&theEvent.where);
					
					cntlCode = FindControl(theEvent.where,helpWPtr,&whichControl);
					
					if (cntlCode != 0)
					{
						if (whichControl == vScrollControl)
						{
							if (cntlCode == inThumb)
							{
								oldVal = GetCtlValue(vScrollControl);
								
								TrackControl(vScrollControl,theEvent.where,0L);
								
								controlVal = GetCtlValue(vScrollControl);
								
								if (controlVal > oldVal)
									TEScroll(0,-TEGetHeight(oldVal,controlVal-1,helpTEH),helpTEH);
									
								else if (controlVal < oldVal)
									TEScroll(0,TEGetHeight(controlVal,oldVal-1,helpTEH),helpTEH);
							}
							
							else
								TrackControl(vScrollControl,theEvent.where,HelpScrollProc);
						}
						
						else if (whichControl == doneControl)
							cntlCode = TrackControl(doneControl,theEvent.where,0L);
						
						else
							SysBeep(1);
					}
					else
						SysBeep(1);
				}
				
				else if (theEvent.what==keyDown || theEvent.what==autoKey)
				{
					theChar = theEvent.message & charCodeMask;
					
					if (theChar == (char) 0x0d || theChar == (char) 0x03 || theChar == 'd' || theChar == 'D')
						cntlCode = inButton;
				}
				
				else if (theEvent.what==updateEvt && (WindowPtr) theEvent.message==helpWPtr)
				{
					BeginUpdate(helpWPtr);
					
					DrawControls(helpWPtr);
					
					if (FrontWindow() != helpWPtr)
						EraseRect(&vScrollRect);
					else
					{
						PenNormal();
						PenSize(3,3);
						InsetRect(&doneRect,-4,-4);
						FrameRoundRect(&doneRect,16,16);
						InsetRect(&doneRect,4,4);
						PenSize(1,1);
					}
					
					TEUpdate(&viewRect,helpTEH);
					FrameRect(&viewRect);
					
					EndUpdate(helpWPtr);
				}
				
				else if (theEvent.message==activateEvt && (WindowPtr) theEvent.message==helpWPtr) 
				{
					if (theEvent.modifiers & activeFlag) 
					{
						TEActivate(helpTEH);
						HiliteControl(doneControl,0);
						
						PenNormal();
						PenSize(3,3);
						InsetRect(&doneRect,-4,-4);
						FrameRoundRect(&doneRect,16,16);
						InsetRect(&doneRect,4,4);
					}
					else 
					{
						TEDeactivate(helpTEH);
						EraseRect(&vScrollRect);
						
						HiliteControl(doneControl,255);
						PenNormal();
						PenSize(3,3);
						PenPat(white);
						InsetRect(&doneRect,-4,-4);
						FrameRoundRect(&doneRect,16,16);
						InsetRect(&doneRect,4,4);
					}
				}
			}
		
		} while (cntlCode != inButton);
	}
	else
	{
		SysBeep(1);
		SysBeep(1);
		SysBeep(1);
	}
	
	
	KillControls(helpWPtr);
	DisposeWindow(helpWPtr);
}