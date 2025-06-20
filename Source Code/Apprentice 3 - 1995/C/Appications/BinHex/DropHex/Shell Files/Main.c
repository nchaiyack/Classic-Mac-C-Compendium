#include <ctype.h>
#include	<AppleEvents.h>
#include "DecodeBinHex.h"
#include "EncodeBinHex.h"
#include "Main.h"





/*	local defines	*/

/*	external globals	*/

/*	local globals	*/
AEEventHandlerUPP				gAECoreProc;
short								gQuitFlag;





void main(void)
{
	EventRecord		event;
	unsigned long	ticks;
	
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	
	gAECoreProc = NewAEEventHandlerProc(AECoreHandler);
	AEInstallEventHandler(kCoreEventClass,kAEOpenApplication,gAECoreProc,kAEOpenApplication,false);
	AEInstallEventHandler(kCoreEventClass,kAEOpenDocuments,gAECoreProc,kAEOpenDocuments,false);
	AEInstallEventHandler(kCoreEventClass,kAEPrintDocuments,gAECoreProc,kAEPrintDocuments,false);
	AEInstallEventHandler(kCoreEventClass,kAEQuitApplication,gAECoreProc,kAEQuitApplication,false);
	
	gQuitFlag = false;
	ticks = TickCount();
	
	while(!gQuitFlag)
	{
		if (WaitNextEvent(everyEvent,&event,60L,NULL))
		{
			switch (event.what)
			{
				case kHighLevelEvent:
					AEProcessAppleEvent(&event);
					break;
			}
			
			ticks = TickCount();
		}
		
		if ((TickCount() - ticks) > (60*3))
			gQuitFlag = true;
	}
}





pascal short AECoreHandler(AppleEvent *apple,AppleEvent *reply,long refCon)
{
	DescType 	type;
	AEKeyword	keyword;
	AEDescList	list;
	FSSpec		file;
	long 			count;
	Size 			size;
	short 		err,index;
	
	
	switch(refCon)
	{
		case kAEOpenApplication:
			gQuitFlag = true;
			break;
		
		case kAEOpenDocuments:
			err = AEGetParamDesc(apple,keyDirectObject,typeAEList,&list);
			if (err)
				return(err);
			
			err = AECountItems(&list,&count);
			for(index = 1;index <= count;index++)
			{
				err = AEGetNthPtr(&list,index,typeFSS,&keyword,&type,(Ptr)&file,sizeof(file),&size);
				ProcessFile(&file);
			}
			
			gQuitFlag = true;
			break;
		
		case kAEPrintDocuments:
			break;
		
		case kAEQuitApplication:
			gQuitFlag = true;
			break;
	}
	
	return (AEGotRequiredParams(apple));
}





short AEGotRequiredParams(AppleEvent *aevent)
{
	DescType	returnedType;
	Size		actualSize;
	short		err;
	
	
	err = AEGetAttributePtr(aevent,keyMissedKeywordAttr,typeWildCard,&returnedType,nil,0,&actualSize);
	if (err == errAEDescNotFound)		// we got all the required parameters
		return noErr;
	else if (!err)						// we missed a required parameter
		return errAEEventNotHandled ;
	else								// the call to AEGetAttributePtr failed
		return err;
}





void ProcessFile(FSSpec *file)
{
	Str32			name;
	
	
	//	does the filename end in .hqx?
	if (file->name[0] >= 4)
	{
		BlockMove(&file->name[file->name[0]-3],&name[1],4);
		name[0] = 4;
		if (EqualString(name,"\p.hqx",false,true))
		{
			DecodeBinHex(file);
			return;
		}
	}
	
	EncodeBinHex(file);
}