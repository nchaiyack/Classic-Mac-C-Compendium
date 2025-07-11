#include "brlr grade 1 meat.h"
#include "brlr conversion.h"
#include "brlr main window.h"

void DealWithGrade1Text(WindowPtr theWindow, unsigned char *a, long len)
{
	long			offset;
	
	gPostingEvents=TRUE;
	gTheWindow=theWindow;
	offset=0;
	while (offset<len)
	{
		AddCharacter(a[offset++]);
	}
	gPostingEvents=FALSE;
}

void AddCharacter(unsigned char theChar)
{
	unsigned char	brailleChar;
	short			time, oldTime;
	
	time=1;
	do
	{
		oldTime=time;
		brailleChar=DealWithLetter(theChar, &time);
		if (brailleChar!=0x00)
		{
			KeyPressedInMainWindow(gTheWindow, brailleChar);
		}
	}
	while (oldTime!=time);
}
