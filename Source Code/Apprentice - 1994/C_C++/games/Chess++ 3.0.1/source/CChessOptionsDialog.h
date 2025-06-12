/******************************************************************************
 CNewDocDialog.h

		Interface for CChessOptionsDialog class.
		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1993 Steven J. Bushell. All rights reserved.
	

 ******************************************************************************/

#define _H_CChessOptionsDialog

#include "CDLOGDirector.h"

class CChessOptionsDialog : public CDLOGDirector
{
	short	selectedSearchDepth;
	Boolean	selectedSoundOnBetterMoves;
	Boolean selectedShowThoughts;
	Boolean	selectedBackPropagation;
	long	selectedBackgroundTimeInterval;

public:

	void 	IChessOptionsDialog( CDirectorOwner *aSupervisor);
	
	virtual void DoChessOptionsDialog( void);
	
	void	DoCommand(long theCommand);
};

enum	// dialog item IDs
{
	kRadioGroupID = 2,
	kEasyGame,
	kHardGame,
	kWayHardGame,
	kSoundOnBetterMoves = 10,
	kShowThoughts,
	kDoReallyCoolThings,
	kBackgroundTimeInterval = 15
};