/*
BreakLines.c

	char *BreakLines(char *string,long lineLength);
Takes a long C string and judiciously changes spaces to '\n' to yield lines
shorter than lineLength, but words are never broken, even if they are longer
than lineLength. All characters other than space and '\n' are treated as letters.
E.g.	printf("%s\n",BreakLines(string,80));

	void PrintWrappedText(FILE *stream,const char *s);
Prints out text, with word wrapping. Leaves the supplied string unmodified.

	void PrintWrappedTextToFile(const char *filename,const char *s);
Appends wrapped text to a file.

	void PrintWrappedComment(FILE *stream,const char *s);
Encloses the string s inside comment delimiters and prints it with word wrapping.

	void PrintWrappedCommentToFile(const char *filename,const char *s);
Encloses the string s inside comment delimiters and appends it to a file,
with word wrapping.

	The preprocessor symbol NEWLINE is normally defined as '\n' in VideoToolbox.h. 
In the MATLAB environment it is instead defined as '\r' for reasons that are explained
in VideoToolbox.h.

HISTORY:
1993	dgp	Wrote BreakLines.
9/7/93	dgp	Wrote PrintWrappedText and PrintWrappedTextToFile based on similar routines
		called PrintComment, etc., written by dhb and jms.
9/11/93	dgp Added PrintWrappedComment
*/
#include "VideoToolbox.h"

char *BreakLines(char *string,long lineLength)
{
	long i,leftMargin,rightMargin,length;
	int here;
	
	leftMargin=0;
	length=strlen(string);
	while(1){
		rightMargin=leftMargin+lineLength;
		if(rightMargin>=length)return string;		/* successful completion */
		here=0;
		if(!here)for(i=leftMargin;i<rightMargin;i++)if(string[i]==NEWLINE){
			here=1;
			break;
		}
		if(!here)for(;i>=leftMargin;i--)if(string[i]==' ' || string[i]==NEWLINE){
			here=1;
			break;
		}
		if(!here)for(i=leftMargin;i<length;i++)if(string[i]==' ' || string[i]==NEWLINE){
			here=1;
			break;
		}
		if(!here)return string;
		string[i]=NEWLINE;
		leftMargin=i+1;
	}
}

/*
ROUTINE: PrintWrappedText
PURPOSE:
  Prints out text, with word wrapping. Leaves the supplied string unmodified.
*/

void PrintWrappedText(FILE *stream,const char *s)
{
	char *sTemp;
	
	sTemp=malloc(strlen(s)+1L);
	if(sTemp==NULL)PrintfExit("PrintWrappedText: malloc(%ld) failed." NL
		,strlen(s)+1L);
	strcpy(sTemp,s);
	BreakLines(sTemp,80);
	fprintf(stream,"%s",sTemp);
	free(sTemp);
}

/*
ROUTINE: PrintWrappedTextToFile
PURPOSE:
  Append wrapped text to a file.
*/

void PrintWrappedTextToFile(const char *filename,const char *s)
{
	FILE *stream;
	
	stream = fopen(filename,"a");
	if (stream == NULL)
		PrintfExit("PrintWrappedTextToFile: fopen(\"%s\",...) failed.",filename);
	PrintWrappedText(stream,s);
	fclose(stream);
}

/*
ROUTINE: PrintWrappedComment
PURPOSE:
  Prints out a comment, with word wrapping.
*/

void PrintWrappedComment(FILE *stream,const char *s)
{
	char *sTemp;
	long i;
	
	sTemp=malloc(strlen(s)+6L);
	if(sTemp==NULL)PrintfExit("PrintCommentString: malloc(%ld) failed." NL
		,strlen(s)+6L);
	strcpy(sTemp,"/* ");
	strcat(sTemp,s);
	strcat(sTemp," */" NL);
	PrintWrappedText(stream,sTemp);
	free(sTemp);
}

/*
ROUTINE: PrintWrappedCommentToFile
PURPOSE:
  Append wrapped comment to a file.
*/

void PrintWrappedCommentToFile(const char *filename,const char *s)
{
	FILE *stream;
	
	stream = fopen(filename,"a");
	if (stream == NULL)
		PrintfExit("PrintWrappedCommentToFile: fopen(\"%s\",...) failed.",filename);
	PrintWrappedComment(stream,s);
	fclose(stream);
}
