/*
MultipleChoice.c
Accepts (and echoes on the console) a partial typed response to a
multiple-choice question, expanding the response to print the full answer that
it uniquely specifies. If alternative answers begin with the same letters (e.g.
"Arabic" and "Armenian") then MultipleChoice keeps accepting characters until an
answer is uniquely determined. Typing any non-printing character (e.g. Return or
Enter) terminates the typed string, e.g. to distinguish "ant" from "anteater".
If the typed string doesn't match any of the supplied answer strings then
MultipleChoice makes the defaultChoice. Thus the user can select the default
answer by just hitting return. Here's an example:
	static char bread[]="Bread",fish[]="Fish",*breadFish[]={bread,fish};
	short choice=1;
	
	printf("Would you like to eat Bread or Fish?");
	choice=MultipleChoice(choice,2,breadFish);
	printf("\n");
	
YesOrNo() restricts the answers to "yes" and "no", with a Boolean argument
specifying the default.
	printf("Do you want to go to heaven?");
	choice=YesOrNo(0);
	printf("\n");

NOTE:
The declarations above explicitly allocate space for the strings "Bread" and "Fish". 
In a stand-alone application you could implicitly allocate that space,
	char *breadFish[]={"Bread","Fish"};
but the THINK C compiler doesn't allow this in code resources (e.g. a MATLAB MEX file).

HISTORY:
2/16/93	dgp	wrote YesOrNo().
5/24/93	dgp	added MultipleChoice().
9/24/93	dgp	cosmetic.
1/25/94 dgp	enhanced MultipleChoice() to keep accepting characters until they uniquely
			specify an answer, as suggested by Bart Farell.
*/
#include "VideoToolbox.h"
#if !defined(__TYPES__)
	typedef unsigned char Boolean;
#endif

Boolean YesOrNo(Boolean defaultChoice)
/* Accept "y" or "n" and spell out "Yes" or "No". Anything else gets defaultChoice. */
{
	static char no[]="No",yes[]="Yes",*noYes[]={no,yes};
	return MultipleChoice(defaultChoice,2,noYes);
}

int MultipleChoice(short defaultChoice,short n,char *answer[])
{
	char c,s[64];
	short i,j,imatch,match,matches,is;
	
	printf(" (%s):",answer[defaultChoice]);
	fflush(stdout);
	for(is=0;is<sizeof(s)-1;is++){
		matches=0;
		do{
			c=getcharUnbuffered();
		}while(c==-1);
		if(c==14)abort();			/* Crude test for command-period. */
		if(!isprint(c))c=0;
		s[is]=c;
		s[is+1]=0;
		for(i=0;i<n;i++){
			match=1;
			for(j=0;j<=is;j++)match&=(tolower(s[j])==tolower(answer[i][j]));
			if(match){
				imatch=i;
				matches++;
			}
		}
		if(matches<=1 || s[is]==0)break;
		printf("%c",c);
		fflush(stdout);
	}
	
	/* Erase partial answer, which may be longer than the default answer. */
	is=strlen(s);
	for(j=0;j<is;j++)printf("\b \b");
	
	/* Print answer */
	if(matches==0)imatch=defaultChoice;
	printf("%s.",answer[imatch]);
	fflush(stdout);
	return imatch;
}
