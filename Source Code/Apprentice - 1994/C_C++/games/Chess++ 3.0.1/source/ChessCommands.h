/******************************************************************************
 ChessCommands.h

		Commands and Menu numbers for Chess

		Copyright � 1993 Steven J. Bushell. All rights reserved.
		
 ******************************************************************************/
 
#define _H_ChessCommands

	/**** M E N U S ****/

#define		MENUchess		1000			// Menu of chess app commands

	/**** C O M M A N D S ****/

#define	cmdSwapPlayers			1024L			// Swap players
#define	cmdStopThinking			1025L			// Stop Brain from thinking
#define cmdOptions				1030L			// Bring up options dialog
#define	cmdTellTime				5800L			// Tell the time

#define ResignNoteAlert			1024			// 'I resign' note alert res ID
#define CheckmateNoteAlert		1025			// 'Checkmate' note alert res ID
#define CheckNoteAlert			1026			// 'Moved into Check' note alert res ID
#define StalemateNoteAlert		1027			// 'Stalemate' note alert res ID