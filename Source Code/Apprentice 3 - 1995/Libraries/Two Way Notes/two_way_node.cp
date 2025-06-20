/********************************************************/
/*                                                      */
/*                 Two_WayNode.cp                       */
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
#include <string.h>

#include "two_way.h"

#include "monster.h"


/*************************node::fillNode*********************************/
void node::fillNode(node *Prev,node *Next, void *Data,unsigned long DataLength, short DataKey)
{
data = malloc(DataLength);
dataLength = DataLength;
next = Next;
prev = Prev;
memcpy( data, Data, dataLength );
dataKey = DataKey;
}

/*************************node::getData*********************************/
void *node::getData(void)
{
return (data);
}

/*************************node::getDataKey*********************************/
short node::getDataKey(void)
{
return(dataKey);
}

/*************************node::getDataLength*********************************/
unsigned long node::getDataLength(void)
{
return(dataLength);
}

/*************************node::getNextNode*********************************/
node *node::getNextNode(void)
{
return(next);
}

/*************************node::getPrevNode*********************************/
node *node::getPrevNode(void)
{
return(prev);
}

/*************************node::setNextNode*********************************/
void node::setNextNode(node *theNode)
{
next = theNode;
}

/*************************node::setPrevNode*********************************/
void node::setPrevNode(node *theNode)
{
prev = theNode;
}

/*************************node::deleteNode*********************************/
void node::deleteNode(void)
{
free(data);

if(prev->next != next)
	{
	prev->next = next;
	}
if(next->prev != prev)
	{
	next->prev = prev;
	}
	
delete this;
}