/**********************************************************************\

File:		help.h

Purpose:	This is the header file for help.c

\**********************************************************************/

extern	int				gWhichHelp;
extern	int				gLastHelp;

int HelpWindowDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc);
