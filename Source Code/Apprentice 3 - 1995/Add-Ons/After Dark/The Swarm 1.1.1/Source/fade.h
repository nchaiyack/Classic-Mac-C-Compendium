/********************************************************************************
         Copyright  �  N. Jonas Englund, 1993.    All Rights Reserved.
 ********************************************************************************

 	PROJECT:	clut_fade.�
 	
 	FILE:		fade.h
 	
 	PURPOSE:	constants, type definitions and prototypes

 ********************************************************************************/

//================================= DEFINES =====================================

#define 	MAXBYTE 	0xFF 	// 256 colors

//================================= TYPEDEFS ====================================

typedef struct FadeValues
{
	short	reds[MAXBYTE];
	short	greens[MAXBYTE];
	short	blues[MAXBYTE];
}
FadeValues;

//================================= FUNCTIONS ===================================

void fade_screen(short, Boolean);
void restore_clut(void);
void install_clut(short clutID);
void setup_cluts();

//=================================== EOF =======================================
