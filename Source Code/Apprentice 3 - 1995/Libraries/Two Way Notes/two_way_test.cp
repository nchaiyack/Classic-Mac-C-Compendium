/********************************************************/
/*                                                      */
/*                 two_way_test.cp                      */
/*                 A test Routine for: Two_Way.cp       */
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "two_way.h"

#include "monster.h"

void main(void)
{
list	*myList, *sortedList;
void	*myData;
monster	*myMonster, *testData;
short	myDataKey, testDataKey;
unsigned long myDataLength;
short	error, i;

myMonster = (monster *)malloc(sizeof(monster));
testData = (monster *)malloc(sizeof(monster));
strcpy( myMonster->name, "One" );
myMonster->st = 10; myMonster->in = 10; myMonster->dx = 10; myDataKey = 1;
myList = myList->initList((void *)myMonster,sizeof(monster), myDataKey);

strcpy( myMonster->name, "Two" );
myMonster->st = 20; myMonster->in = 10; myMonster->dx = 10; myDataKey = 2;
error = myList->addLastNode((void *)myMonster, sizeof(monster), myDataKey);

strcpy( myMonster->name, "Four" );
myMonster->st = 5; myMonster->in = 12; myMonster->dx = 2; myDataKey = 4;
error = myList->addLastNode((void *)myMonster, sizeof(monster), myDataKey);

strcpy( myMonster->name, "Five" );
myMonster->st = 50; myMonster->in = 120; myMonster->dx = 20; myDataKey = 5;
error = myList->addLastNode((void *)myMonster, sizeof(monster), myDataKey);

strcpy( myMonster->name, "Three" );
myMonster->st = 50; myMonster->in = 120; myMonster->dx = 20; myDataKey = 3;
error = myList->addLastNode((void *)myMonster, sizeof(monster), myDataKey);

sortedList = myList->insSortList();

if(error)
	{
	myList->moveCurrNodeToStart();
	printf("----------From Start To End---------------\n");
	for(i=1; i<=myList->getListLength();i++)
		{
		testData = (monster *)(myList->extractCurrNode(&testDataKey, &myDataLength));
		printMonster(testData);
		myList->moveCurrNodeForward();
		}
	printf("-------------------------\n");
	
	myList->moveCurrNodeToEnd();
	printf("----------From End To Start---------------\n");
	for(i=myList->getListLength(); i>=1;i--)
		{
		testData = (monster *)(myList->extractCurrNode(&testDataKey, &myDataLength));
		printMonster(testData);
		myList->moveCurrNodeBackward();
		}
	printf("-------------------------\n");
	printf("----------From Start To End, Sorted List---------------\n");
	sortedList->moveCurrNodeToStart();
	for(i=1; i<=sortedList->getListLength();i++)
		{
		testData = (monster *)(sortedList->extractCurrNode(&testDataKey, &myDataLength));
		printMonster(testData);
		sortedList->moveCurrNodeForward();
		}
	printf("-------------------------\n");
	}

testData = (monster *)(myList->extractNthNode(&testDataKey, &myDataLength,3));
printMonster(testData);

myList->deleteFirstNode();
testData = (monster *)(myList->extractFirstNode(&testDataKey, &myDataLength));
printMonster(testData);

myList->deleteFirstNode();
testData = (monster *)(myList->extractFirstNode(&testDataKey, &myDataLength));
printMonster(testData);

myList->deleteFirstNode();
testData = (monster *)(myList->extractFirstNode(&testDataKey, &myDataLength));
printMonster(testData);

free(myMonster);
myList->deleteList();
}