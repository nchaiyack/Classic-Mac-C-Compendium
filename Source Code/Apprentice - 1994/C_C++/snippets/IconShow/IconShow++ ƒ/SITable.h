/* 
    Copyright © General Consulting & Research Inc. 1993-1994 All rights reserved.
  	Author: 	Peter H. Teeson 
    CIS:		72647,3674
    AppleLink:	CDA0197
  	Date:		1 June 1993 
	Path:IconShow++ Ä:SITable.h
   	
   	This is the C++ Class declaration for the ShowINIT table.	

 */
#pragma once 
/* Constants & Enumerations */
enum	 SIitem{ vcheck, vloc, hloc, hcheck };		// indices of ShowInit table
const	 short	XOffset=40, YOffset=40;				// Icon bucket size
const	 short	IconWidth=32, InitialX=8;			// Icon width & initial location

/* Declarations */
//extern	unsigned short screenWidth,screenDepth;
class SITable {			// for our one and only ShowInit Table object	
public:
	SITable();										// constructor
	~SITable();										// destructor	
 	unsigned short& operator=(unsigned short& x);	// assignment operator
	unsigned short& operator[](int index);			// subscripting operator
	
 	unsigned short CheckSum(unsigned short x);		// computes checksum
 	void CheckRHEdge(SITable& x);					// check if will be off RH edge
 	void NextLoc(SITable& x);						// compute location of next icon	
private:
	unsigned short *sTable;
	int	sz;											// for range checking
	void Reset();									// reset table

};

/* Inlines */		
#pragma parameter __D0 CheckSum( __D0 )
unsigned short SITable::CheckSum( unsigned short x )={
	0xE358, 	// ROL.W	#1,D0
	0x0A40, 	// EORI.W	#$1021,D0
	0x1021
};