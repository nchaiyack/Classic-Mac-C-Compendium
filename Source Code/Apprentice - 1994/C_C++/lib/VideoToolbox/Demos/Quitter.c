/*
Quitter.c

Kill a program on a local or remote computer. Works by sending an AppleEvent.
Unfortunately, it appears that Apple doesn't consider active desk accessories
as "processes" to which one can send an AppleEvent. 

It is essential that the HighLevelEvent-Aware bit be set in the SIZE resource; 
use the menu item Project:SetProjectType:SIZE Flags.

At present this has nothing at all to do with the rest of the VideoToolbox, but
David Brainard and I have been discussing the idea of a stand-alone VideoToolbox
application that accepts AppleEvents commanding it to produce visual stimuli. 

HISTORY:
3/5/93 dgp wrote it
3/9/93	dgp got it to work, by setting the HighLevelEvent-Aware bit, as instructed
by Larry Harris, 76150,1027, a friendly fellow programmer on CompuServe.
4/17/93	dgp	#include VideoToolbox.h for PrintfExit().
12/15/93 dgp Go away quietly if user hits Cancel.
*/
#include <VideoToolbox.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <AppleEvents.h>

void main(void)
{
	OSErr error;
	unsigned char prompt[]="\pChoose a program to quit.";
	PortInfoRec portInfo;
	TargetID targetID;
	AEAddressDesc target;
	AppleEvent appleEvent,reply;
	AESendMode sendMode;
	long value;

	printf("Welcome to Quitter.\n");
	Gestalt(gestaltAppleEventsAttr,&value);
	if(!(value&(1<<gestaltAppleEventsPresent)))PrintfExit("Sorry, I need AppleEvents.\n");
	PPCInit();
	// Select target through dialog with user.
	error=PPCBrowser(prompt,NULL,0,&targetID.location,&portInfo,NULL,NULL);
	if(error==-128)abort();	// User pressed Cancel.
	if(error)PrintfExit("PPCBrowser error %d\n",error);
	targetID.name=portInfo.name;
	error=AECreateDesc(typeTargetID,(Ptr)&targetID,sizeof(targetID),&target);
	if(error)PrintfExit("AECreateDesc error %d\n",error);
	error=AECreateAppleEvent(kCoreEventClass,kAEQuitApplication,&target
		,kAutoGenerateReturnID,kAnyTransactionID,&appleEvent);
	if(error)PrintfExit("AECreateAppleEvent error %d\n",error);
	error=AEDisposeDesc(&target);
	sendMode=kAENoReply+kAENeverInteract+kAEDontReconnect;
	error=AESend(&appleEvent,&reply,sendMode,kAENormalPriority
		,kAEDefaultTimeout,NULL,NULL);
	if(error)PrintfExit("AESend error %d\n",error);
	error=AEDisposeDesc(&appleEvent);
	if(error)PrintfExit("AEDisposeDesc error %d\n",error);
	printf("Done. �%#s� has been asked to quit.\n",targetID.name.name);
}
