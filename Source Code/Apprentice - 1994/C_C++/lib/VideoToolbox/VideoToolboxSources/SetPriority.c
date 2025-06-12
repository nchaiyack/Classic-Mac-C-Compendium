/*
SetPriority.c

char priority=7;

SwapPriority(&priority);
// do high priority stuff here
SwapPriority(&priority);

Get and set the processor priority. (This is modeled after SwapMMUMode.) The
Macintosh operating system normally leaves the processor priority at zero,
allowing all interrupts. You can temporarily block interrupts by raising the
processor priority. The highest level is 7, blocking all interrupts. This is
useful for time critical tasks, such as running video animations, where your
primary concern is video timing, and you're willing to miss AppleTalk and
keyboard events. Note that while the processor priority is high the keyboard and
mouse will be dead, so that it is essential that your program lower it back down
to allow the user to regain control.

The inline functions GetStatusRegister() and SetStatusRegister() only work
because the processor is in supervisor state. As of this writing the Macintosh
operating system (up to 7.0) always leaves the 680x0 processor in supervisor
state. However, Apple has warned that this may change in future releases of the
operating system. In the future, therefore, it may be necessary to write more
elaborate versions of these two routines, begging the operating system's
permission to do this, since it cannot be done while the processor is in user
state.

HISTORY:
1989?	dgp	wrote it
8/1/91	dgp	introduced the inline functions as a way to isolate the machine code
			part, and to make it compatible with MPW C as well as THINK C.
8/6/91	dgp	fixed definition of GetStatusRegister() to make it a legal prototype.
2/15/93	dgp	added SwapPriority().
*/
#include "VideoToolbox.h"

/* These are inline functions. This format is supported by both THINK and MPW C */
pascal short GetStatusRegister(void)={0x40d7};			/* MOVE.W SR,(SP)	*/
pascal void SetStatusRegister(short status)={0x46df};	/* MOVE.W (SP)+,SR	*/

void SetPriority(int i)
{
	register short status;

	i &= 7;
	i <<= 8;
	status=GetStatusRegister();
	status &= ~0x700;	/* clear the priority bits */
	status |= i;		/* insert new priority bits */
	SetStatusRegister(status);
}

int GetPriority(void)
{
	register short status;

	status=GetStatusRegister();
	status >>= 8;
	status &= 7;
	return status;
}

void SwapPriority(char *priority)
{
	char oldPriority;
	
	oldPriority=GetPriority();
	SetPriority(*priority);
	*priority=oldPriority;
}