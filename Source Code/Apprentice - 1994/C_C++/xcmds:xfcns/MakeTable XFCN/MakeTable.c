/*
	MakeTable	- XFCN to compress runs of spaces and tabs to a space
*/

#define	whoami	"\n\
--- MakeTable(string), Version 1.0, February 1993,\n\
    by Eric Gundrum, based on Reduce by Dan Allen.\n\
    Converts runs of spaces & tabs to a single tab in the passed string.\n"

#include <Types.h>
#include <Memory.h>
#include <String.h>
#include <HyperXCmd.h>

#define ReturnError(x)	{paramPtr->returnValue=msgHandle(x);return;}

Handle msgHandle(char* msg_string);

pascal void EntryPoint(XCmdPtr paramPtr)
{
	char		*sourceChar,*resultChar;
	Handle		Result_hdl;

	if(paramPtr->paramCount != 1)
		ReturnError("List of parameters is not usable.");
	MoveHHi(paramPtr->params[0]);
	Result_hdl = NewHandle(GetHandleSize(paramPtr->params[0]));
	if (!Result_hdl)
		ReturnError("Could not allocate space for table.")
	HLock(Result_hdl);
	sourceChar = *(paramPtr->params[0]);
	resultChar = *Result_hdl;
	while(*sourceChar)
	{
		if (*sourceChar == '\t' || *sourceChar == ' ')
		{
			do
				sourceChar++;
			while (*sourceChar == '\t' || *sourceChar == ' ');
			*resultChar++ = '\t';
		} else
			*resultChar++ = *sourceChar++;
	}
	*resultChar = '\0';
	SetHandleSize(Result_hdl,strlen(*Result_hdl));
	paramPtr->returnValue = Result_hdl;
	HUnlock(Result_hdl);
}

Handle msgHandle(char* msg_string)
{
	Handle returnMsg = NewHandle(strlen(msg_string) + strlen(whoami) + 1);

	HLock(returnMsg);
	BlockMove(msg_string,*returnMsg,strlen(msg_string)); 
	BlockMove(whoami,*returnMsg+strlen(msg_string),strlen(whoami));
	HUnlock(returnMsg);
	return returnMsg;
}
