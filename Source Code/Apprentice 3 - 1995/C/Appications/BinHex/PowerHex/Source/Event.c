#include "Menu.h"
#include "Event.h"





/*	Local globals	*/
short				gQuitFlag;
WorkerInfoPtr	gWorkers;





void InitEvents(void)
{
	gQuitFlag = 0;
	gWorkers = NULL;
}





short doEventLoop(long sleepTime)
{
	WindowProcsPtr	procs;
	DialogPtr		dialog;
	EventRecord		event;
	GrafPtr			savedPort;
	short				item;
	short				gotEvent;
	
	
	if (gWorkers)
		sleepTime = 0L;
	
	gotEvent = WaitNextEvent(everyEvent,&event,sleepTime,NULL);
	if (gotEvent)
	{
		if (IsDialogEvent(&event) && DialogSelect(&event,&dialog,&item))
		{
			procs = (WindowProcsPtr)GetWRefCon((WindowPtr)dialog);
			if (procs && procs->hit)
				procs->hit((WindowPtr)dialog,procs->param,item);
		}
		else
			switch(event.what)
			{
				case mouseDown:
					doMouseDown(event.where);
					break;
				case mouseUp:
					break;
				case keyDown:
					doKeyDown(event.message,event.modifiers);
					break;
				case keyUp:
					break;
				case autoKey:
					break;
				case updateEvt:
					GetPort(&savedPort);
					SetPort((WindowPtr)event.message);
					BeginUpdate((WindowPtr)event.message);
					
					procs = (WindowProcsPtr)GetWRefCon((WindowPtr)event.message);
					if (procs && procs->update)
						procs->update((WindowPtr)event.message,procs->param);
						
					EndUpdate((WindowPtr)event.message);
					SetPort(savedPort);
					break;
				case activateEvt:
					break;
			}
	}
	else
	{
		if (IsDialogEvent(&event))
			DialogSelect(&event,&dialog,&item);
		
		doWork();
	}
	
	if (gQuitFlag)
		KillWorkers();
	
	return gQuitFlag;
}





void doMouseDown(Point where)
{
	WindowProcsPtr	procs;
	WindowPtr		window;
	long				menuSelection;
	short				partCode;
	
	
	partCode = FindWindow(where,&window);
	if (window && (window != FrontWindow()))
		SelectWindow(window);
	else
		switch(partCode)
		{
			case inDesk:
				break;
			case inMenuBar:
				menuSelection = MenuSelect(where);
				if (menuSelection & 0xFFFF0000)
				{
					doMenuSelect(menuSelection>>16,menuSelection);
					HiliteMenu(0);
				}
				break;
			case inSysWindow:
				break;
			case inContent:
				procs = (WindowProcsPtr)GetWRefCon(window);
				GlobalToLocal(&where);
				if (procs && procs->click)
					procs->click(window,procs->param,where);
				break;
			case inDrag:
				DragWindow(window,where,&qd.screenBits.bounds);
				break;
			case inGrow:
				break;
			case inGoAway:
				break;
			case inZoomIn:
				break;
			case inZoomOut:
				break;
		}
}





void doKeyDown(long message,short modifiers)
{
	WindowProcsPtr	procs;
	WindowPtr		window;
	long				menuSelection;
	char				theChar;
	
	
	theChar = charCodeMask & message;
	if (modifiers & cmdKey)
	{
		menuSelection = MenuKey(theChar);
		if (menuSelection & 0xFFFF0000)
		{
			doMenuSelect(menuSelection>>16,menuSelection);
			HiliteMenu(0);
		}
	}
	
	if ((!(menuSelection&0xFFFF)) && (window = FrontWindow()) != NULL)
	{
		procs = (WindowProcsPtr)GetWRefCon(window);
		if (procs && procs->key)
			procs->key(window,procs->param,theChar,modifiers);
	}
}





void doWork(void)
{
	WorkerInfoPtr	worker;
	
	
	worker = gWorkers;
	while(worker)
	{
		if (worker->proc)
		{
			if (worker->proc(worker->param))
			{
				WorkerInfoPtr	temp,waxed;
				
				if (gWorkers != worker)
				{
					temp = gWorkers;
					while(temp)
					{
						if (temp->next == worker)
						{
							temp->next = temp->next->next;
							waxed = worker;
							worker= worker->next;
							if (waxed->kill)
								waxed->kill(waxed->param);
							DisposePtr((Ptr)waxed);
							break;
						}
						temp = temp->next;
					}
				}
				else
				{
					waxed = gWorkers;
					gWorkers = gWorkers->next;
					if (waxed->kill)
						waxed->kill(waxed->param);
					DisposePtr((Ptr)waxed);
					worker = gWorkers;
				}
			}
			else
				worker = worker->next;
		}
		else
			worker = worker->next;
	}
}





void NewWorker(WorkProc worker,KillProc kill,void *param)
{
	WorkerInfoPtr	nu;
	
	
	nu = (WorkerInfoPtr)NewPtrClear(sizeof(WorkerInfo));
	nu->next = gWorkers;
	nu->param = param;
	nu->proc = worker;
	nu->kill = kill;
	gWorkers = nu;
}





void KillWorkers(void)
{
	WorkerInfoPtr	worker,waxed;
	
	
	worker = gWorkers;
	while(worker)
	{
		waxed = worker;
		worker = worker->next;
		
		if (waxed->kill)
			waxed->kill(waxed->param);
		DisposePtr((Ptr)waxed);
	}
}