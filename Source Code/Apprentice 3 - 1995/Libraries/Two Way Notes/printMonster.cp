/********************************************************/
/*                                                      */
/*                 PrintMonster.cp                      */
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

#include <stdio.h>

#include "monster.h"

void printMonster(monster *theMonster)
{
printf("%s: ",theMonster->name);
printf("st = %d, ",theMonster->st);
printf("in = %d, ",theMonster->in);
printf("dx = %d\n",theMonster->dx);
}