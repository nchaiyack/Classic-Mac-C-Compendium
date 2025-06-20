/**********************************************************************\

File:		main.h

Purpose:	This is the header file for main.c

\**********************************************************************/

void main(void);
void EventLoop(void);
void GetTheEvent(EventRecord *theEvent, int waitTime);
void DispatchEvents(EventRecord theEvent);
void HandleMouseDown(EventRecord theEvent);
void ShutDownEnvironment(void);
