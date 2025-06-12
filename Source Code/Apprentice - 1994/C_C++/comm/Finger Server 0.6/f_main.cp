/*
 * mac finger sever
 * by Aaron Wohl n3liw+@cmu.edu
 * 412-268-5032 / 412-731-3691
 */

#include <stdlib.h>
#include "sip_glue.h"
#include "sip_interface.h"
#include "limits.h"

#define xDEBUG

/*
 * set to false to exit
 */
int sip_keep_running=TRUE;

#define SLEEP_TIME			(ULONG_MAX)

#ifdef DEBUG
#define EXTRA_STACK			60000
#include <stdio.h>
#else
#define EXTRA_STACK			12000
#endif

/*
 * run a background process to handle a finger server
 */
static int run_finger_server(void)
{
	EventRecord theEvent;
	if(tcp_allocate_memory())
		return TRUE;
	//keep retrying MacTCP
	while(tcp_open_driver())
		WaitNextEvent(everyEvent,&theEvent,10*60,0);
	if(tcp_init_streams())
		return TRUE;
	while(sip_keep_running) {
		if(tcp_ok_to_sleep_now())
			WaitNextEvent(everyEvent,&theEvent,SLEEP_TIME,0);
		tcp_just_awoke();
		tcp_service_event();
	}
	tcp_release_streams();
	return FALSE;
}

/*
 * main program for finger server
 */
int main()
{
	SetApplLimit(GetApplLimit()-EXTRA_STACK);
	MaxApplZone();
#ifdef DEBUG
	printf("yo!\n");
	Debugger();
#else
	InitGraf(NewPtr(206) + 202);
#endif
	if(run_finger_server())
		SysBeep(20);
	ExitToShell();
}
