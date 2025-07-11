/*/
     Project Arashi: GameEvent.c
     Major release: Version 1.1, 7/22/92

     Last modification: Thursday, March 18, 1993, 14:52
     Created: Friday, March 1, 1991, 4:46

     Copyright � 1991-1993, Juri Munkki
/*/

#include "VA.h"
#include "STORM.h"
#include "Shuddup.h"
#include "PlayOptions.h"

extern	EventRecord	Event;

int		HandleGameEvent(flag)
int		flag;
{
	int			handled=1;
	WindowPtr	thewind;
	int			makesound=0;
	
	if(flag)
	{	switch(Event.what)
		{	case keyDown:
			case autoKey:
				switch(Event.message & 0xFF)
				{	case '-':
					case '_':		SKVolume(Vv.volume-1);
									break;
					case '+':
					case '=':		SKVolume(Vv.volume+1);
									break;

					case 9:			PlayOptions->absMoveFlag = 0;
									makesound=1;
									break;

					case 0x1D:		PlayOptions->rotationType=0;
									PlayOptions->absMoveFlag=1;
									makesound=1;
									break;

					case 0x1C:		PlayOptions->rotationType=1;
									PlayOptions->absMoveFlag=1;
									makesound=1;
									break;

					case 0x1E:		PlayOptions->rotationType=2;
									PlayOptions->absMoveFlag=1;
									makesound=1;
									break;

					case 0x1F:		PlayOptions->rotationType=3;
									PlayOptions->absMoveFlag=1;
									makesound=1;
									break;
		
					default:		handled=0;
									break;
				}
				break;
			case updateEvt:
				thewind=(WindowPtr)Event.message;
				BeginUpdate(thewind);
				SetPort(thewind);
				ForeColor(blackColor);
				PaintRect(&thewind->portRect);
				EndUpdate(thewind);
				SetPort(VA.window);

			default:
				handled=0;
				break;
		}
		if(handled)
			Event.what=0;
	}
	else
	{	Event.what=0;
	}
	
	if(makesound)
	{	PlayA(PhazerOut,999);
		PlayB(Bonk,999);
	}
	
	return handled;
}

void	GameEvent()
{
	if(PlayOptions->noBackgroundTasks)	HandleGameEvent(GetOSEvent(everyEvent, &Event));
	else								HandleGameEvent(GetNextEvent(everyEvent,&Event));
}

int		GamePeekEvent()
{
	int		flag;
	int		stop;
	
	stop = 0;
	if(PlayOptions->noBackgroundTasks)	flag = OSEventAvail(everyEvent, &Event);
	else								flag = EventAvail(everyEvent, &Event);
		
	if(flag)
	{	flag=HandleGameEvent(flag);
		if(flag)
		{	if(PlayOptions->noBackgroundTasks)
					GetOSEvent(everyEvent, &Event);
			else	GetNextEvent(everyEvent,&Event);
		}
		else
		{	stop = 1;
		}
	}
	
	return stop;
}