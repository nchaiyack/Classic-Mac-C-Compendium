/*
	File:			VoicePicker.c

	Contains:		VoicePicker library routines.
				
	Written by:	Luigi Belverato
				P.O. Box 19,
				20080 Basiglio MI,
				Italy

	Copyright:	�1995 Luigi Belverato
	
	Change History (most recent first):
				06/26/95	0.0.2	Added DisposeRoutineDescriptor in function 'VoicePicker'
									for function 'DialProc'. Needed for PPC version.
				06/23/95	0.0.1

	Notes: 		This code uses Apple's Universal Interfaces for C version 2.01f.
	
				Send bug reports to lbelvera@micronet.it
				(if I don't reply within a few days use snail mail as I might have changed internet provider)
*/

//-----------------------------------------------------------------------
// this text document was formatted with Geneva 9, tab size 4

#include "VoicePicker.h"
#include "VoicePickerPrivate.h"

PicHandle		gDial[3];
VPpreferences	gPreferences;

extern Boolean VoicePicker(	ConstStr255Param	passedStr,
						VoiceData			*passedVoiceData)
{
	Boolean			returnedValue=false;

	if ((IsCompatible) && (passedVoiceData!=nil))
	{
		DialogPtr			theDialog=nil;
		
		GetDefault();

		theDialog=GetNewDialog(kDLOGVoicePicker,nil,(WindowRef)-1);
		
		if (theDialog!=nil)
		{
			UserItemUPP		theDialProcPtr=nil;
			OSErr			error=noErr;
			short			itemHit=0;
			short			iType=0;
			Handle			iHandle=nil;
			Rect				iRect={0,0,0,0};
			VoiceData			passedVoiceDataCopy;			
			Str255			speechString;
			VoiceListH			firstVoiceH=nil;
			SpeechChannel		channel=nil;
			short			lastPopupValue=1;
			
			theDialProcPtr=NewUserItemProc(DialProc);

			passedVoiceDataCopy=*passedVoiceData;

			error=SetDialogDefaultItem(theDialog,kVPcancelBtn);
			error=SetDialogCancelItem(theDialog,kVPcancelBtn);
			error=SetDialogTracksCursor(theDialog,true);
			
			if (Fix2Long((*passedVoiceData).pitch)<gPreferences.pitchMin)
				(*passedVoiceData).pitch=Long2Fix(gPreferences.pitchMin);
		
			if (Fix2Long((*passedVoiceData).pitch)>gPreferences.pitchMax)
				(*passedVoiceData).pitch=Long2Fix(gPreferences.pitchMin);
		
			if (Fix2Long((*passedVoiceData).rate)<gPreferences.rateMin)
				(*passedVoiceData).rate=Long2Fix(gPreferences.rateMin);
		
			if (Fix2Long((*passedVoiceData).rate)>gPreferences.rateMax)
				(*passedVoiceData).rate=Long2Fix(gPreferences.rateMax);
		
			BuildVoicesList(&firstVoiceH,&lastPopupValue,passedVoiceData);
			
			GetDialogItem(theDialog,kVPmsgStr,&iType,&iHandle,&iRect);
			if (iHandle!=nil)
			{
				if (passedStr==nil)
				{
					Str255	theString;

					GetIndString(theString,kSTRxResID,kSTRxMessage);
					SetDialogItemText(iHandle,theString);
				}
				else
					SetDialogItemText(iHandle,passedStr);
			}
			
			GetDialogItem(theDialog,kVPvoicePopup,&iType,&iHandle,&iRect);
			if (iHandle!=nil)
			{
				MenuHandle	menuH=nil;

				menuH=(**(PopupPrivateDataHandle)(**(ControlHandle)iHandle).contrlData).mHandle;
				if (menuH!=nil)
				{
					BuildVoicesMenu(menuH,&firstVoiceH);
					
					SetControlMaximum((ControlHandle)iHandle,CountMItems(menuH));
					SetControlValue((ControlHandle)iHandle,lastPopupValue);
				}
			}
			
			if (((*passedVoiceData).voice.creator==0) || ((*passedVoiceData).voice.id==0))
				AdjustItems(theDialog,firstVoiceH,passedVoiceData,&channel,false);
			else
				AdjustItems(theDialog,firstVoiceH,passedVoiceData,&channel,true);
			
			GetDialogItem(theDialog,kVPrateDial,&iType,&iHandle,&iRect);
			SetDialogItem(theDialog,kVPrateDial,iType,(Handle)theDialProcPtr,&iRect);
	
			GetDialogItem(theDialog,kVPpitchDial,&iType,&iHandle,&iRect);
			SetDialogItem(theDialog,kVPpitchDial,iType,(Handle)theDialProcPtr,&iRect);
	
			SetGrafPortOfDialog(theDialog);
			ShowWindow(GetDialogWindow(theDialog));
			
			do
			{
				ModalDialog(nil,&itemHit);
				switch (itemHit)
				{
					case kVPvoicePopup:
						GetDialogItem(theDialog,kVPvoicePopup,&iType,&iHandle,&iRect);
						if (iHandle!=nil)
						{
							if (lastPopupValue!=GetControlValue((ControlHandle)iHandle))
							{
								lastPopupValue=GetControlValue((ControlHandle)iHandle);
								AdjustItems(theDialog,firstVoiceH,passedVoiceData,&channel,false);
							}
						}
						break;
					case kVPrateDial:
					case kVPpitchDial:
						HandleDial(theDialog,itemHit,channel,passedVoiceData);
						break;
					case kVPtryBtn:
						if (channel!=nil)
						{
							GetDialogItem(theDialog,kVPsampleEdit,&iType,&iHandle,&iRect);
							if (iHandle!=nil)
							{
									GetDialogItemText(iHandle,speechString);
									error=SpeakText(channel,(Ptr)&speechString[1],speechString[0]);
							}
						}
						break;
					case kVPstopBtn:
						if (channel!=nil)
							error=SpeakText(channel,nil,0);
						break;
				}
			}
			while ((itemHit!=kVPokBtn) && (itemHit!=kVPcancelBtn));
			
			switch (itemHit)
			{
				case kVPokBtn:
					returnedValue=true;
					break;
				case kVPcancelBtn:
					(*passedVoiceData)=passedVoiceDataCopy;
					break;
			}
				
			DisposeDialog(theDialog);
			theDialog=nil;
			
			if (channel!=nil)
			{
				error=DisposeSpeechChannel(channel);
				channel=nil;
			}
			DestroyVoicesList(firstVoiceH);
			
			if (gDial[kNoArrow])
				ReleaseResource((Handle)gDial[kNoArrow]);
			if (gDial[kNoArrow])
				ReleaseResource((Handle)gDial[kUpArrow]);
			if (gDial[kNoArrow])
				ReleaseResource((Handle)gDial[kDownArrow]);
			
			if (theDialProcPtr!=nil)	/*� added in 0.0.2*/
				DisposeRoutineDescriptor(theDialProcPtr);
	
		}
	}
	
	return returnedValue;
}

static void BuildVoicesList(	VoiceListH			*firstVoiceH,
						short			*lastPopupValue,
						VoiceData			*passedVoiceData)
{
	OSErr		error=noErr;
	VoiceListH		tempH=nil;
	VoiceListH 	lastVoiceH=nil;
	VoiceSpec		voice;
	short		voiceCount=0;
	short		maxVoices=0;

	error=CountVoices(&maxVoices);
	if ((error==noErr) && (maxVoices>0))
	{
		for (voiceCount=1; (error==noErr) && (voiceCount<=maxVoices); voiceCount++)
		{
			error=GetIndVoice(voiceCount,&voice);
			
			if (error==noErr)
			{
				tempH=(VoiceListH)NewHandleClear(sizeof(VoiceList));
				error=MemError();
				if ((error==noErr) && (tempH!=nil))
				{
					if ((voice.creator==(*passedVoiceData).voice.creator) && (voice.id==(*passedVoiceData).voice.id))
						*lastPopupValue=voiceCount;
					if (lastVoiceH==nil)
						*firstVoiceH=tempH;
					else
						(**lastVoiceH).nextH=tempH;

					(**tempH).voice=voice;
					(**tempH).nextH=nil;
					(**tempH).previousH=lastVoiceH;
					lastVoiceH=tempH;
				}
			}
		}
	}
}

static void BuildVoicesMenu(	MenuHandle		theMenu,
						VoiceListH			*firstVoiceH)
{
	VoiceDescription	info;
	VoiceListH			tempH;
	VoiceListH			temp1H;
	long				infoLength=sizeof(VoiceDescription);
	OSErr			error;
	//Str255			theString;

	if (theMenu!=nil)
	{
		tempH=*firstVoiceH;
		
		while (tempH!=nil)
		{
			error=GetVoiceDescription(&(**tempH).voice,&info,infoLength);
			
			if (error==noErr)
			{
				AppendMenu(theMenu,info.name);
				tempH=(**tempH).nextH;
			}
			else
			{
				//GetIndString(theString,kSTRxResID,kSTRxVoiceUnav);
				//AppendMenu(theMenu,theString);
				if ((**tempH).previousH!=nil)
					(**(**tempH).previousH).nextH=(**tempH).nextH;
				if ((**tempH).nextH!=nil)
					(**(**tempH).nextH).previousH=(**tempH).previousH;
				temp1H=tempH;
				tempH=(**tempH).nextH;
				if (*firstVoiceH==temp1H)
					*firstVoiceH=tempH;
				DisposeHandle((Handle)temp1H);
				temp1H=nil;
			}
		}
	}
}

static void GetIndVoiceInfo(	short			item,
						VoiceDescription	*info,
						VoiceListH			firstVoiceH)
{
	short		counter=1;
	OSErr		error=noErr;
	long			infoLength=sizeof(VoiceDescription);
	VoiceListH	 	tempH;
	
	tempH=firstVoiceH;
	while ((counter<=item) && (tempH!=nil))
	{
		if (counter==item)
			error=GetVoiceDescription(&(**tempH).voice,info,infoLength);

		tempH=(**tempH).nextH;
		counter++;
	}
}

static void AdjustItems(		DialogPtr			theDialog,
						VoiceListH			firstVoiceH,
						VoiceData			*passedVoiceData,
						SpeechChannel		*channel,
						Boolean			usePassedValues)
{
	short			iType;
	Handle			iHandle;
	Rect				iRect;
	VoiceDescription	info;
	Str255			theString;
	short			whichVoice;
	OSErr			error;
	
	GetDialogItem(theDialog,kVPvoicePopup,&iType,&iHandle,&iRect);
	if (iHandle!=nil)
	{
		whichVoice=GetControlValue((ControlHandle)iHandle);
		
		GetIndVoiceInfo(whichVoice,&info,firstVoiceH);
		
		if (*channel!=nil)
		{
			error=DisposeSpeechChannel(*channel);
			*channel=nil;
		}
		
		(*passedVoiceData).voice=info.voice;
		
		error=NewSpeechChannel(&(*passedVoiceData).voice,channel);
		
		if ((error==noErr) && (channel))
		{
			if (usePassedValues)
			{
				error=SetSpeechRate(*channel,(*passedVoiceData).rate);
				error=SetSpeechPitch(*channel,(*passedVoiceData).pitch);
			}
			else
			{
				error=GetSpeechRate(*channel,&(*passedVoiceData).rate);
				error=GetSpeechPitch(*channel,&(*passedVoiceData).pitch);
			}
		}

		GetDialogItem(theDialog,kVPgenderStr,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			switch (info.gender)
			{
				case kMale:
					GetIndString(theString,kSTRxResID,kSTRxMale);
					break;
				case kNeuter:
					GetIndString(theString,kSTRxResID,kSTRxNeuter);
					break;
				case kFemale:
					GetIndString(theString,kSTRxResID,kSTRxFemale);
					break;
				default:
					theString[0]=0;
					break;
			}
			SetDialogItemText(iHandle,theString);
		}
		
		GetDialogItem(theDialog,kVPageStr,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			NumToString(info.age,theString);
			SetDialogItemText(iHandle,theString);
		}
	
		GetDialogItem(theDialog,kVPlanguageStr,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			NumToString(info.language,theString);
			SetDialogItemText(iHandle,theString);
		}
	
		GetDialogItem(theDialog,kVPregionStr,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			NumToString(info.region,theString);
			SetDialogItemText(iHandle,theString);
		}
	
		GetDialogItem(theDialog,kVPrateStat,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			NumToString(Fix2Long((*passedVoiceData).rate),theString);
			SetDialogItemText(iHandle,theString);
		}
	
		GetDialogItem(theDialog,kVPpitchStat,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			NumToString(Fix2Long((*passedVoiceData).pitch),theString);
			SetDialogItemText(iHandle,theString);
		}
	
		GetDialogItem(theDialog,kVPtryBtn,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			if (*channel!=nil)
				SetControlValue((ControlHandle)iHandle,0);
			else
				SetControlValue((ControlHandle)iHandle,255);
		}
	
		GetDialogItem(theDialog,kVPstopBtn,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
		{
			if (*channel!=nil)
				SetControlValue((ControlHandle)iHandle,0);
			else
				SetControlValue((ControlHandle)iHandle,255);
		}

		GetDialogItem(theDialog,kVPsampleEdit,&iType,&iHandle,&iRect);
		if (iHandle!=nil)
			SetDialogItemText(iHandle,info.comment);
		
	}
}

static void DestroyVoicesList(	VoiceListH			firstVoiceH)
{
	VoiceListH		tempH=nil;
	VoiceListH		nextH=nil;
	
	tempH=firstVoiceH;
	while (tempH!=nil)
	{
		nextH=(**tempH).nextH;
		DisposeHandle((Handle)tempH);
		tempH=nextH;
	}
}


static Boolean PointerInside(	Rect				*theRect)
{
	Point		mouseLoc;
	
	GetMouse(&mouseLoc);
	return PtInRect(mouseLoc,theRect);
}

static void ChangeButton(		Rect				theRect,
						short			thePictID)
{
	if (gDial[thePictID]!=nil)
		DrawPicture(gDial[thePictID],&theRect);
}

static void AdjustScrollerText(	DialogPtr			theDialog,
						short			which,
						long				value,
						SpeechChannel		channel)
{
	short		iType;
	Handle		iHandle;
	Rect			iRect;
	Str255		theString;
	OSErr		error;

	NumToString(value,theString);
	switch (which)
	{
		case kVPrateDial:
			GetDialogItem(theDialog,kVPrateStat,&iType,&iHandle,&iRect);
			if (iHandle!=nil)
				SetDialogItemText(iHandle,theString);
			if (channel)
				error=SetSpeechRate(channel,Long2Fix(value));

			break;
		case kVPpitchDial:
			GetDialogItem(theDialog,kVPpitchStat,&iType,&iHandle,&iRect);
			if (iHandle!=nil)
				SetDialogItemText(iHandle,theString);
			if (channel)
				error=SetSpeechPitch(channel,Long2Fix(value));
			break;
	}
}

static pascal void DialProc(	DialogPtr			theDialog,
						short			theItem)
{
	Rect		iRect;
	Handle	iHandle;
	short	iType;
	
	GetDialogItem(theDialog,theItem,&iType,&iHandle,&iRect);

	ChangeButton(iRect,kNoArrow);
}

static void HandleDial(		DialogPtr			theDialog,
						short			whichDial,
						SpeechChannel		channel,
						VoiceData			*passedVoiceData)
{
	Rect		iRect,topRect,bottomRect;
	Handle	iHandle;
	short	iType;
	Boolean	alreadyDown;
	long		finalTicks;

	GetDialogItem(theDialog,whichDial,&iType,&iHandle,&iRect);

	topRect=iRect;
	topRect.bottom=topRect.top+(iRect.bottom-iRect.top)/2;
	
	bottomRect=iRect;
	bottomRect.top=bottomRect.top+(iRect.bottom-iRect.top)/2;
	
	alreadyDown=false;
	
	if (StillDown())
	{
		if (PointerInside(&topRect))
		{
			ChangeButton(iRect,kUpArrow);
			alreadyDown=true;
			while (StillDown())
			{
				if (PointerInside(&topRect))
				{
					switch (whichDial)
					{
						case kVPrateDial:
							if (Fix2Long((*passedVoiceData).rate)<gPreferences.rateMax)
							{
								(*passedVoiceData).rate=(*passedVoiceData).rate+Long2Fix(1);
								AdjustScrollerText(theDialog,kVPrateDial,Fix2Long((*passedVoiceData).rate),channel);
							}
							break;
						case kVPpitchDial:
							if (Fix2Long((*passedVoiceData).pitch)<gPreferences.pitchMax)
							{
								(*passedVoiceData).pitch=(*passedVoiceData).pitch+Long2Fix(1);
								AdjustScrollerText(theDialog,kVPpitchDial,Fix2Long((*passedVoiceData).pitch),channel);
							}
							break;
					}
					
					 if (!alreadyDown)
					 {
					 	alreadyDown=true;
						ChangeButton(iRect,kUpArrow);
					}
				}
				else
				{
					 if (alreadyDown)
					 {
					 	alreadyDown=false;
						ChangeButton(iRect,kNoArrow);
					}
				}
				Delay(gPreferences.dialWait,&finalTicks);
			}
			ChangeButton(iRect,kNoArrow);
		}
		else if (PointerInside(&bottomRect))
		{
			ChangeButton(iRect,kDownArrow);
			alreadyDown=true;
			while (StillDown())
			{
				if (PointerInside(&bottomRect))
				{
					switch (whichDial)
					{
						case kVPrateDial:
							if (Fix2Long((*passedVoiceData).rate)>gPreferences.rateMin)
							{
								(*passedVoiceData).rate=(*passedVoiceData).rate-Long2Fix(1);
								AdjustScrollerText(theDialog,kVPrateDial,Fix2Long((*passedVoiceData).rate),channel);
							}
							break;
						case kVPpitchDial:
							if (Fix2Long((*passedVoiceData).pitch)>gPreferences.pitchMin)
							{
								(*passedVoiceData).pitch=(*passedVoiceData).pitch-Long2Fix(1);
								AdjustScrollerText(theDialog,kVPpitchDial,Fix2Long((*passedVoiceData).pitch),channel);
							}
							break;
					}

					 if (!alreadyDown)
					 {
					 	alreadyDown=true;
						ChangeButton(iRect,kDownArrow);
					}
				}
				else
				{
					 if (alreadyDown)
					 {
					 	alreadyDown=false;
						ChangeButton(iRect,kNoArrow);
					}
				}
				Delay(gPreferences.dialWait,&finalTicks);
			}
			ChangeButton(iRect,kNoArrow);
		}
	}
}

static Boolean IsCompatible(	void)
{
	OSErr	error;
	long		result;
	Boolean	returnedValue=true;
	
	error=Gestalt(gestaltSpeechAttr,&result);
	if ((error!=noErr) || !(result & (1<<gestaltSpeechMgrPresent)))
		returnedValue=false;
		
	error=Gestalt(gestaltPopupAttr,&result);
	if ((error!=noErr) || !(result & (1<<gestaltPopupPresent)))
		returnedValue=false;
	
	return returnedValue;
}

static void GetDefault(		void)
{
	VPpreferencesH	preferencesH=nil;
	OSErr			error=noErr;
	
	gDial[kNoArrow]=(PicHandle)Get1Resource('PICT',kNoArrowPict);
	gDial[kUpArrow]=(PicHandle)Get1Resource('PICT',kUpArrowPict);
	gDial[kDownArrow]=(PicHandle)Get1Resource('PICT',kDownArrowPict);
	
	preferencesH=(VPpreferencesH)Get1Resource(kVPprResType,kVPprResID);
	error=ResError();
	if ((error==noErr) && (preferencesH!=nil) && (GetHandleSize((Handle)preferencesH)==sizeof(VPpreferences)))
	{
		gPreferences=**preferencesH;
		ReleaseResource((Handle)preferencesH);
	}
	else
	{
		gPreferences.rateMin=krateMin;
		gPreferences.rateMax=krateMax;
		gPreferences.pitchMin=kpitchMin;
		gPreferences.pitchMax=kpitchMax;
		gPreferences.dialWait=kDialWait;
	}
}

/* end of file: VoicePicker.c */