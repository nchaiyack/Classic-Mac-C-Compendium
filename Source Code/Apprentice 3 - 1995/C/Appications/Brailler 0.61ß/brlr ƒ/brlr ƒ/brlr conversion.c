#include "brlr conversion.h"
#include "brlr main window.h"
#include "key layer.h"

WindowPtr		gTheWindow;
Boolean			gPostingEvents;
short			gGrade;

unsigned char DealWithLetter(unsigned char theChar, short *time)
{
	if ((*time)==1)
	{
		if ((theChar>='0') && (theChar<='9'))
		{
			(*time)++;
			return Dots("\p3456");
		}
		if ((theChar=='[') || (theChar=='{') || (theChar=='<') || ((theChar>='A') && (theChar<='Z')))
		{
			(*time)++;
			return Dots("\p6");
		}
		if ((theChar==']') || (theChar=='}') || (theChar=='>'))
		{
			(*time)++;
			return Dots("\p356");
		}
		if ((theChar=='*') || (theChar=='�') || (theChar=='�') || (theChar=='�'))
			(*time)++;
	}
	
	if ((*time)==2)
	{
		if ((theChar=='�') || (theChar=='�'))
			(*time)++;
	}
	
	if ((*time)==3)
	{
		if (theChar=='�')
			(*time)++;
	}
	
	if ((theChar>='A') && (theChar<='Z'))
		theChar|=0x20;

	switch (theChar)
	{
		case '1':
		case 'a':	return Dots("\p1");
		case '2':
		case 'b':	return Dots("\p12");
		case '3':
		case 'c':	return Dots("\p14");
		case '4':
		case 'd':	return Dots("\p145");
		case '5':
		case 'e':	return Dots("\p15");
		case '6':
		case 'f':	return Dots("\p124");
		case '7':
		case 'g':	return Dots("\p1245");
		case '8':
		case 'h':	return Dots("\p125");
		case '9':
		case 'i':	return Dots("\p24");
		case '0':
		case 'j':	return Dots("\p245");
		case 'k':	return Dots("\p13");
		case 'l':	return Dots("\p123");
		case 'm':	return Dots("\p134");
		case 'n':	return Dots("\p1345");
		case 'o':	return Dots("\p135");
		case 'p':	return Dots("\p1234");
		case 'q':	return Dots("\p12345");
		case 'r':	return Dots("\p1235");
		case 's':	return Dots("\p234");
		case 't':	return Dots("\p2345");
		case 'u':	return Dots("\p136");
		case 'v':	return Dots("\p1236");
		case 'w':	return Dots("\p2456");
		case 'x':	return Dots("\p1346");
		case 'y':	return Dots("\p13456");
		case 'z':	return Dots("\p1356");
		case ',':	return Dots("\p2");
		case ':':	return Dots("\p25");
		case '-':	return Dots("\p36");
		case ';':	return Dots("\p23");
		case '.':	return Dots("\p256");
		case '!':	return Dots("\p235");
		case '(':
		case ')':	return Dots("\p2356");
		case '?':
		case '"':
		case '�':
		case '[':
		case '{':
		case '<':	return Dots("\p236");
		case '�':
		case '\'':
		case '�':
		case ']':
		case '}':
		case '>':	return Dots("\p3");
		case '*':	return Dots("\p35");
		case '/':	return Dots("\p34");
		case '�':
		case '�':	return Dots("\p36");
		case ' ':
		case key_Return:	return theChar;
		case key_Enter:		return key_Return;
		case key_Tab:		return theChar;
	}
	
	return 0x00;
}

unsigned char Dots(Str15 dotStr)
{
	unsigned char	result;
	short			i;
	
	result=0x00;
	if (dotStr[0]==0x00)
		return result;
	
	for (i=1; i<=dotStr[0]; i++)
	{
		switch (dotStr[i])
		{
			case '1':	result|=0x01;	break;
			case '2':	result|=0x02;	break;
			case '3':	result|=0x04;	break;
			case '4':	result|=0x08;	break;
			case '5':	result|=0x10;	break;
			case '6':	result|=0x20;	break;
		}
	}
	
	return result+' ';
}
