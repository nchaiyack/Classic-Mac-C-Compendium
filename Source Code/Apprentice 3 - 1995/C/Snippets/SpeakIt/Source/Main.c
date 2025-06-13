#include "SpeakIt.h"
#include <stdio.h>

short	main(void);
static 	void DoStuff(void);

short	main ()
{
	CheckSpeechMgr();
	SpeakIt( "Starting up.", true );
	
//	code...

	DoStuff();


	SpeakIt( "Exiting.", true );
} /*main*/


static void DoStuff()
{
	OSErr	iErr = 1;
	char	errorStr[256];

//	code...
	
	if( iErr != noErr ) {
		sprintf( errorStr, "Bummer. DoStuff didn't work. Error number %d.", iErr );
		SpeakIt( errorStr, true );
	}
}


/*

<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
William Modesitt ¥ Maui Software ¥ 189 Auoli Dr. ¥ Makawao, Maui, HI 96768
808-573-0011             fax: 808-572-2406           email: billm@maui.com
ftp://ftp.maui.com/pub/mauisw                   http://www.maui.com/~billm
                            (Life's a Beach!)
<><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>

*/