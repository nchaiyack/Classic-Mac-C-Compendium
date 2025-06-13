/********************************************************/
/*                                                      */
/*                 Two_Way_List.cp                      */
/*                 Routines for: Two_Way_List           */
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

#include "two_way.h"

#include "monster.h"

/*************************list::initList*********************************/
list *list::initList(void *data, unsigned long dataLength, short dataKey)
{
node *newNode;
list *newList;

newList = new list;
newNode = new node;

if(newList && newNode)
	{
	newNode->fillNode(newNode, newNode,data,dataLength, dataKey);
	newList->first = newNode;
	newList->last = newNode;
	newList->curr = newNode;
	newList->nodeCount = 1;
	return(newList);
	}
else
	{
	return(NULL);
	}
}

/*************************list::addFirstNode*********************************/
short list::addFirstNode(void *data, unsigned long dataLength, short dataKey)
{
node *newNode;

newNode = new node;
if(newNode)
	{
	newNode->fillNode(newNode, first,data,dataLength,dataKey);
	first->setPrevNode(newNode);
	first = newNode;
	curr = newNode;
	nodeCount++;
	return(1);
	}
else
	{
	return(0);
	}
}

/*************************list::addNodeBeforeCurrent*********************************/
short list::addNodeBeforeCurrent(void *data, unsigned long dataLength, short dataKey)
{
node	*newNode;
short	error;
if(first != curr)
	{
	newNode = new node;
	if(newNode)
		{
		newNode->fillNode(curr->getPrevNode(), curr,data,dataLength,dataKey);
		( (curr->getPrevNode()) ->setNextNode(newNode));
		curr->setPrevNode(newNode);
		curr = newNode;
		nodeCount++;
		return(1);
		}
	else
		{
		return(0);
		}
	}
else
	{
	error = this->addFirstNode(data,dataLength,dataKey);
	if(error)
		{
		return(1);
		}
	else
		{
		return(0);
		}
	}
}

/*************************list::addNodeAfterCurrent*********************************/
short list::addNodeAfterCurrent(void *data, unsigned long dataLength, short dataKey)
{
node	*newNode;
short	error;
if(last != curr)
	{
	newNode = new node;
	if(newNode)
		{
		newNode->fillNode(curr, curr->getNextNode(),data,dataLength,dataKey);
		( (curr->getNextNode()) ->setPrevNode(newNode));
		curr->setNextNode(newNode);
		curr = newNode;
		nodeCount++;
		return(1);
		}
	else
		{
		return(0);
		}
	}
else
	{
	error = this->addLastNode(data,dataLength,dataKey);
	if(error)
		{
		return(1);
		}
	else
		{
		return(0);
		}
	}
}

/*************************list::addLastNode*********************************/
short list::addLastNode(void *data, unsigned long dataLength, short dataKey)
{
node *newNode;

newNode = new node;

if(newNode)
	{
	newNode->fillNode(last, newNode,data,dataLength,dataKey);
	last->setNextNode(newNode);
	last = newNode;
	curr = newNode;
	nodeCount++;
	return(1);
	}
else
	{
	return(0);
	}
}

/*************************list::extractFirstNode*********************************/
void *list::extractFirstNode(short *dataKey, unsigned long *dataLength)
{
*dataKey = first->getDataKey();
*dataLength = first->getDataLength();
return(first->getData());
}

/*************************list::extractCurrNode*********************************/
void *list::extractCurrNode(short *dataKey, unsigned long *dataLength)
{
*dataKey = curr->getDataKey();
*dataLength = curr->getDataLength();
return(curr->getData());
}

/*************************list::extractNthNode*********************************/
void *list::extractNthNode(short *dataKey, unsigned long *dataLength, short n)
{
short length, halfLength;
short i;

length = this->getListLength();
halfLength = length/2;
if(n<=length)
	{
	if(n <= halfLength)
		{
		this->moveCurrNodeToStart();
		for(i=1; i<n; i++)
			{
			this->moveCurrNodeForward();
			}
		}
	else
		{
		this->moveCurrNodeToEnd();
		for(i=length; i>n; i--)
			{
			this->moveCurrNodeBackward();
			}
		}
	}
*dataKey = curr->getDataKey();
*dataLength = curr->getDataLength();
return(curr->getData());
}

/*************************list::extractLastNode*********************************/
void *list::extractLastNode(short *dataKey, unsigned long *dataLength)
{
*dataKey = last->getDataKey();
*dataLength = last->getDataLength();
return(last->getData());
}

/*************************list::moveCurrNodeForward*********************************/
void	list::moveCurrNodeForward(void)
{
if(curr != last)
	{
	curr = curr->getNextNode();
	}
}

/*************************list::moveCurrNodeToStart*********************************/
void	list::moveCurrNodeToStart(void)
{
curr = first;
}

/*************************list::moveCurrNodeToNth*********************************/
void	list::moveCurrNodeToNth(short n)
{
short length, halfLength;
short i;

length = this->getListLength();
halfLength = length/2;

if(n<=length)
	{
	if(n<=halfLength)
		{
		this->moveCurrNodeToStart();
		for(i=1;i<n;i++)
			{
			this->moveCurrNodeForward();
			}
		}
	else
		{
		this->moveCurrNodeToEnd();
		for(i=length;i>n;i--)
			{
			this->moveCurrNodeBackward();
			}
		}
	}
curr = last;
}

/*************************list::moveCurrNodeToEnd*********************************/
void	list::moveCurrNodeToEnd(void)
{
curr = last;
}

/*************************list::moveCurrNodeBackward*********************************/
void	list::moveCurrNodeBackward(void)
{
if(curr != first)
	{
	curr = curr->getPrevNode();
	}
}

/*************************list::deleteFirstNode*********************************/
void list::deleteFirstNode(void)
{
node  *temp;

if(nodeCount > 1)
	{
	temp = first;
	if(curr == first)
		{
		curr = first->getNextNode();
		}
	first = first->getNextNode();
	nodeCount--;
	temp->deleteNode();
	}
else
	{
	this->deleteList();
	}
}

/*************************list::deleteList*********************************/
void list::deleteList(void)
{
short i;

for(i=1; i<nodeCount; i++)
	{
	first->deleteNode();
	}
delete this;
}

/*************************list::getListLength*********************************/
short	list::getListLength(void)
{
return(nodeCount);
}