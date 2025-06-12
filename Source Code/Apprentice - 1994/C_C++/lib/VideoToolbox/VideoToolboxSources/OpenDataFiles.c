/*
OpenDataFile.c
This is something I found myself writing repeatedly. It asks the user for a
parameter file with extension ".in" to be used for input (to control an experiment),
and opens it as the input stream *inPtr (with default stdin). Then it opens a new output
file with a unique name formed by appending a time stamp onto the input file name.
If there's no input file then the user is asked to supply a base file name for the
output file, to which the extension is added. If the total file name would be more than
31 characters, the maximum allowed on the Mac, then the base name is truncated. This
guarantees that the extension will still be unique. 

Finally the output file type is set to Excel.

Since the time stamp advances only once per second, I added a check to make sure
that the proposed new output file doesn't already exist. The time stamp is advanced
as many seconds as necessary to create a unique file name.

The inName string allocation should be at least 32 bytes. I suggest the outName 
string allocation be at least 40 bytes.

HISTORY:
9/14/90	dgp	extracted it from UserOpen.c, and cleaned it up for general use.
9/15/90	dgp	added handling of special cases: filename too long, file 
			already exists.
10/11/90 dgp Now you can supply NULL instead of any of the arguments, causing the
			corresponding questions (input & output file names) to be skipped.
8/24/91	dgp	Made compatible with THINK C 5.0.
10/23/92 dgp Treat single-character filename same as longer names.
1/25/93 dgp removed obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"

unsigned long OpenDataFiles(FILE **inPtr,FILE **outPtr,char *inName,char *outName)
{
	unsigned long seconds;
	char string[64]="";
	FILE *in;
	
	GetDateTime(&seconds);
	in=NULL;
	if(inPtr!=NULL && inName!=NULL){
		printf("Please enter input filename. \n"
			"Extension .in will be added. Just cr for none:");
		gets(inName);
		strcpy(string,inName);
		if(strlen(inName)>0){
			strncat(inName,".in",5);
			in=fopen(inName,"r");
			if(in==NULL)PrintfExit("Sorry, can't find \"%s\".\007\n",inName);
		}
	}
	if(in==NULL)in=stdin;
	if(inPtr!=NULL)*inPtr=in;
	if(outPtr!=NULL && outName!=NULL){
		*outPtr=NULL;
		if(strlen(string)==0){
			printf("Please enter output filename. \n"
				"Extension .%s.data will be added.\n"
				"Just cr for none:",DateString(seconds));
			fgets(string,sizeof(string)-1,in);
			if(strlen(string)>0)string[strlen(string)-1]=0;		/* Remove trailing cr */
		}
		if(strlen(string)>0){
			goto A;
			do{
				if(strlen(string)>0)string[strlen(string)-1]=0;		/* Remove last char */
				A: sprintf(outName,"%s.%s.data",string,DateString(seconds));
			} while(strlen(outName)>31);		/* max length for Mac filename */
			*outPtr=fopen(outName,"r");
			if(*outPtr!=NULL){
				fclose(*outPtr);
				seconds++;
				goto A;
			}
			*outPtr=fopen(outName,"w");
			if(*outPtr==NULL)PrintfExit("Sorry, can't create \"%s\".\007\n",outName);
		}
		if(*outPtr!=NULL) SetFileInfo(outName,'TEXT','KAHL');
	}
	return seconds;
}