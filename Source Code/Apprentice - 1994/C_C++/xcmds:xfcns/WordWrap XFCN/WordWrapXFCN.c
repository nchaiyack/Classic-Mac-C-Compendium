#define	whoami	"\
\n    WordWrap(source string,line length) \
\n    Version 1.0.1 July 1993, by Eric Gundrum.\
\n    Inserts line breaks on spaces or tabs to fit the specified line length.\n"


//~~~~~	Mac includes
#include <Types.h>	
#include <Memory.h>	
#include <String.h>	
#include <Strings.h>	
#include <Packages.h>
#include <HyperXCmd.h>

//~~~~~	Function Declarations
void		doWordWrap(Handle SourceStr, const long lineLength);	
Handle	msgHandle(char* msg_string);

#define	strcpy(x,y)			BlockMove((y),(x),strlen(y))
#define	ReturnError(x)		{paramPtr->returnValue=msgHandle(x);return;}


pascal void Main(XCmdPtr paramPtr)
{
	long		lineLength;
	char		hstate;
	Str255	pString;
	
	if(paramPtr->paramCount != 2)
		ReturnError("Invalid parameters; expected a string and line length.");

	//lock second param, copy to Pascal string and convert to number
	hstate = HGetState(paramPtr->params[1]); HLock(paramPtr->params[1]);
	BlockMove(*(paramPtr->params[1]),&pString[1],strlen(*(paramPtr->params[1])));
	pString[0] = strlen(*(paramPtr->params[1]));
	StringToNum(pString,&lineLength);
	HSetState(paramPtr->params[1],hstate);

	//do WordWrap on first param
	hstate = HGetState(paramPtr->params[0]); HLock(paramPtr->params[0]);
	doWordWrap(paramPtr->params[0],lineLength);
	HSetState(paramPtr->params[0],hstate);
	
	paramPtr->returnValue = paramPtr->params[0];
}

Handle msgHandle(char* msg_string)
{
	Handle returnMsg = NewHandle(strlen(msg_string) + strlen(whoami) - 1);

	HLock(returnMsg);
	BlockMove(msg_string,*returnMsg,strlen(msg_string)); 
	BlockMove(whoami,*returnMsg+strlen(msg_string),strlen(whoami));
	HUnlock(returnMsg);
	return returnMsg;
}

void doWordWrap(Handle srcString, const long lineLength)
{
	Ptr			buffer = *srcString;
	long			srcLength = GetHandleSize(srcString);	
	long			curLinePos,curPos;
	
	for(curPos=0,curLinePos=0; curPos < srcLength; curPos++, curLinePos++)	
	{	//check each character
		if(buffer[curPos] == '\n')	curLinePos	=	0;		//mark at CR
		else if(curLinePos == lineLength)					//a full line
		{
			//backup to white space
			while((curLinePos > 0)&&(buffer[curPos]!=' ')&&(buffer[curPos]!='\t'))	
				{curPos--;curLinePos--;}
			if(curLinePos > 0)			//make line break
			{
				buffer[curPos] = '\n';	//write new CR
				curLinePos	=	0;			//reset line counter
			}
			else
				curPos  +=	lineLength;	//move to start of new line
		}
	}
}
