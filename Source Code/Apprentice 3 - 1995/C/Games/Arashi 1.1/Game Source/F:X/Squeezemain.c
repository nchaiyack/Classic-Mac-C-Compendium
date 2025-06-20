/*/
     Project Arashi/STORM: Squeezemain.c
     Major release: 9/9/92

     Last modification: Wednesday, September 9, 1992, 21:42
     Created: Saturday, October 6, 1990, 1:16

     Copyright � 1990-1992, Juri Munkki
/*/

#include "Shuddup.h"

#define	COMPRESS
void	main()
{
	int		i;
	long	ticker;
	
	DoInits();
	SetPort(GetNewWindow(1000,0,-1));
#ifdef	COMPRESS
	OpenResFile("\P STORM.rsrc");
	DoCompress();
#else
	ticker=TickCount();
	InitSoundKit();
	ticker=TickCount()-ticker;
	SKVolume(8);
	
	for(i=0;i<NumSounds;i++)
	{	PlayA(i,999);
		while(!Button());
		while(Button());
	}
	CloseSoundKit();
#endif
}