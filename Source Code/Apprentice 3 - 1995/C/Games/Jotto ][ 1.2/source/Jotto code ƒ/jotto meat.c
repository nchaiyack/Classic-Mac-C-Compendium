#include "program globals.h"
#include "jotto meat.h"
#include "jotto dictionary.h"
#include "jotto environment.h"
#include "util.h"

enum
{
	kGoDown=-1,
	kGotIt,
	kGoUp
};

void NewWord(void)
{
	short			i;
	
	for (i=0; i<gNumLetters; i++)
		gHumanWord[gNumTries][i]=' ';
	gWhichChar=0x00;
}

Boolean ValidWord(char *thisWord)
{
	if (InRealDictionary(thisWord))
		return TRUE;
	else
		return (InCustomDictionary(thisWord));
}

Boolean InCustomDictionary(char *thisWord)
{
	unsigned long	index;
	char			temp[6];
	short			found;
	
	if ((gNumLetters==5) ? (FiveLetterCustomOKQQ()) : (SixLetterCustomOKQQ()))
	{
		index=0L;
		found=666;
		while ((GetCustomWord(temp, index++)) && (found!=kGotIt))
			found=DirectionalCompare(thisWord, temp);
		
		return (found==kGotIt);
	}
	else return FALSE;
}

Boolean InRealDictionary(char *thisWord)
{
	unsigned long	index;
	char			temp[6];
	short			direction;
	unsigned long	gap;
	short			iter;
	
	index=gNumHumanWords[gNumLetters-5]/2;
	gap=index;
	iter=(gNumLetters==6) ? 14 : 13;
	while (iter>0)
	{
		iter--;
		if (gap%2)
			gap=(gap>>1)+1;
		else
			gap=gap>>1;
		GetHumanWord(temp, index);
		direction=DirectionalCompare(thisWord, temp);
		switch (direction)
		{
			case kGotIt:	return TRUE; break;
			case kGoDown:	index-=gap; break;
			case kGoUp:		index+=gap; break;
		}
		if (index<0)
			index=0;
		else if (index>=gNumHumanWords[gNumLetters-5])
			index=gNumHumanWords[gNumLetters-5]-1;
	}
	
	return FALSE;
}

short DirectionalCompare(char *thisOne, char *thatOne)
{
	short			i;
	
	for (i=0; i<gNumLetters; i++)
		if (thisOne[i]<thatOne[i])
			return kGoDown;
		else if (thisOne[i]>thatOne[i])
			return kGoUp;

	return kGotIt;
}

void CalculateNumRight(void)
{
	Boolean			used[6];
	short			i,j;
	
	gNumRight[gNumTries]=0;
	for (i=0; i<gNumLetters; i++)
		used[i]=FALSE;
	for (i=0; i<gNumLetters; i++)
		for (j=0; j<gNumLetters; j++)
		{
			if ((gHumanWord[gNumTries][i]==gComputerWord[j]) && (!used[j]))
			{
				gNumRight[gNumTries]++;
				used[j]=TRUE;
				j=gNumLetters;
			}
		}	
}
