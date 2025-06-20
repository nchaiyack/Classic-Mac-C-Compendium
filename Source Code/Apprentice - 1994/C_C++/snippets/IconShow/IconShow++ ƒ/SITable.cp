/*  
    Copyright � General Consulting & Research Inc. 1993-1994 All rights reserved.
  	Author: 	Peter H. Teeson 
    CIS:		72647,3674
    AppleLink:	CDA0197
  	Date:		1 June 1993 
	Path:IconShow++ �:SITable.cp
   	
   	This is the C++ Class implementation for the ShowINIT table.	

 */

#include <SITable.h>
extern unsigned short myScreenWidth,myScreenDepth;
/* Public members */
SITable::SITable(){
	sTable=(unsigned short*)(0x910+32-8);			// CurrAppName+32-8 
	sz=4;											// there are only 4 entries	
	if (CheckSum(sTable[hloc]) != sTable[hcheck])	// first INIT means Reset
		{ sTable[vloc] = myScreenDepth;				// will be decremented by Reset
		  Reset();
		}
}					

SITable::~SITable(){}

unsigned short& SITable::operator[](int index){
	if (index<0 || sz<=index)
		{ return sTable[0]; }			// should signal a range error *******
	else
		return sTable[index];
}	

void SITable::CheckRHEdge(SITable& x){
	/*
	 * if we will be off right hand edge of the screen move up
	 * one row and reset horizontal location to first position.
	 */	 
	if ( x.sTable[hloc]>myScreenWidth-IconWidth)
		Reset();
}

void SITable::NextLoc(SITable& x) {
	/* 
	 * compute location of next icon
	 */
	 x.sTable[hloc] += XOffset;
	 x.sTable[hcheck] = CheckSum(x.sTable[hloc]);
	 CheckRHEdge(x);
}

/* Private members */
void SITable::Reset(){
	/* 
	 * Reset horizontal & vertical locations
	 * and re-compute the check sums.
	 */
	sTable[vloc] -= YOffset;						// move up one row
	sTable[hloc] = InitialX;						// init horizontal loc
	sTable[hcheck] = CheckSum(sTable[hloc]);		// recompute
	sTable[vcheck] = CheckSum(sTable[vloc]);
}