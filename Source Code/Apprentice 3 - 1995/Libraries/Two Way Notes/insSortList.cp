/********************************************************/
/*                                                      */
/*                 InsSortList.cp                       */
/*                 Routines for: Two_Way.cp             */
/*                 Copyright 4/24/94 Hasan Edain        */
/*                 All Reights Reserved Worldwide       */
/*                                                      */
/********************************************************/

//	Please feel free to modify and use this code for any purpose, as long as you
// send me
// a) source of the modified code.
// b) licenced version of product the code is used in.
// c) notes as to how the code could be made more usefull if you tried to use it, but
// ended up not using it for any reason.
// Hasan Edain
// HasanEdain@AOL.com
// box 667 Langley Wa 98260
// fax: 206-579-6456

#include "two_Way.h"

list *list::insSortList(void)
{
list	*newList;
void	*temp, *nTemp;
short	dataKey, length, nDataKey, nLength;
unsigned long dataLength, nDataLength;
short i, placed, error;

this->moveCurrNodeToStart();
length = this->getListLength();

temp = this->extractCurrNode(&dataKey, &dataLength);
newList = newList->initList(temp, dataLength, dataKey);
if(newList)
	{
	for(i=1;i<length;i++)
		{
		this->moveCurrNodeForward();
		temp = this->extractCurrNode(&dataKey, &dataLength);
		newList->moveCurrNodeToStart();
		placed = 0;
		while(!placed )
			{
			nTemp = newList->extractCurrNode(&nDataKey, &nDataLength);
			if(nDataKey >= dataKey)
				{
				error = newList->addNodeBeforeCurrent(temp, dataLength, dataKey);
				placed = 1;
				}
			else if(newList->curr == newList->last)
				{
				error = newList->addNodeAfterCurrent(temp, dataLength, dataKey);
				placed = 1;
				}
			newList->moveCurrNodeForward();
			}
		}
	}
return(newList);
}