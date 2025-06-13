/*/
     Project Arashi: ScreenSelect.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 22:22
     Created: Sunday, March 17, 1991, 0:35

     Copyright � 1991-1992, Juri Munkki
/*/

enum	{	NotDone, QuitDone, DefaultDone, SelectDone	};

#define		YESPICTID	129
#define		NOPICTID	128
#define		MAXSCREENS	16

typedef	struct
{
	WindowPtr	window;
	GDHandle	device;
	int			status;
}	ScreenInfo;
