#pragma once

	/*
		This structure matches ResEdit's editor rather than the 
		'vers' type definition in Apple's SysTypes.r file.
	*/

typedef struct 
{
	short Major;  /* ie. 1.2.3 */
	short Minor;
	short Minuscule;

	enum
	{
		kDevelopmentVersion = 0x20, 
		kAlphaVersion       = 0x40, 
		kBetaVersion        = 0x60, 
		kFinalVersion       = 0x80
	} Stage;
	short Release;
	
	short Region; /* Country code */
	
	Str255 ShortDescription;
	Str255 LongDescription;

} VERSION;


void GetVersion( short id, VERSION* ver );
