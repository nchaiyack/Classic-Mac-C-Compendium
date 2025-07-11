#ifndef __NIM_GLOBALS_H__
#define __NIM_GLOBALS_H__

enum GameError
{
	eNoErr=0,
	eSpotTaken,
	eOffBoard,
	eCantMoveBackwards,
	eCantJumpOverOpponent,
	eMustBePrime,
	eOnlyMoveLeftDown,
	eMustMoveLikeQueen,
	eMustMoveLikeKing,
	eMustMoveLikeQueenOrKnight,
	eMustMoveLikeKingOrKnight,
	eMustSelectFirst,
	eMustMoveLeft,
	eCantJumpOtherCoins,
	eBottomRightIsntBlack
};

typedef short	GameError;

enum	/* game status */
{
	kNoGameStarted=0,
	kGameInProgress,
	kGameDone,
	kShowingEndGame
};

enum	/* game type */
{
	kNim=0,
	kPrimeNim,
	kColumnsNim,
	gametype_unused1,
	kTurnablock,
	kSilver,
	gametype_unused2,
	kCornerTheQueen,
	kCornerTheKing,
	kCornerTheSuperqueen,
	kCornerTheSuperking
};

#define	kNumGames			12

enum	/* player modes */
{
	kHumanHumanMode=0,
	kMacHumanMode,
	kHumanMacMode,
	kMacMacMode
};

#define	kFirstPlayer		FALSE
#define	kSecondPlayer		TRUE

enum	/* nim distribution */
{
	kNimDistRandom=1,
	kNimDistUniform,
	kNimDistLinear
};

enum
{
	kTurnDistRandom=1,
	kTurnDistCheckerboard,
	kTurnDistParallelLines
};

enum
{
	kCornerStartTopRight=1,
	kCornerStartRandom,
	kCornerStartAsk
};

enum	/* icon IDs */
{
	kNimBitIconID=1000,
	kNimBitFirstSelectedID,
	kNimBitSecondSelectedID
};

#define kNimNumRowsMin		3
#define kNimNumRowsMax		7
#define kNimMaxPerRowMin	5
#define kNimMaxPerRowMax	11

#define kColumnsNumRowsMin		kCornerNumRowsMin
#define kColumnsNumRowsMax		kCornerNumRowsMax
#define kColumnsNumColumnsMin	kCornerNumColumnsMin
#define kColumnsNumColumnsMax	kCornerNumColumnsMax

#define kTurnNumRowsMin			3
#define kTurnNumRowsMax			8
#define kTurnNumColumnsMin		3
#define kTurnNumColumnsMax		8

#define kSilverNumPlacesMin		14
#define kSilverNumPlacesMax		24
#define kSilverNumTilesMin		4
#define kSilverNumTilesMax		10

#define kCornerNumRowsMin		5
#define kCornerNumRowsMax		13
#define kCornerNumColumnsMin		5
#define kCornerNumColumnsMax		24

#define TheCornerIndex		(gGameType-kCornerTheQueen)

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct GameStuffRec
{
	short			numRows, numColumns, start;
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct GameStuffRec		GameStuffRec;

extern	Boolean			gDisplayMessages;
extern	Boolean			gUseAnimation;
extern	Boolean			gMisere;

extern	short			gGameType;
extern	short			gGameStatus;

extern	Boolean			gPlayer;
extern	short			gPlayerMode;

extern	Handle			gNimBitIconHandle, gNimBitFirstSelectedHandle, gNimBitSecondSelectedHandle;

extern	short			gNimNumRows, gNimMaxPerRow, gNimDistribution;
extern	short			gPrimeNumRows, gPrimeMaxPerRow, gPrimeDistribution;
extern	short			gColumnsNumRows, gColumnsNumColumns, gColumnsDistribution;
extern	short			gTurnNumRows, gTurnNumColumns, gTurnDistribution;
extern	short			gSilverNumPlaces, gSilverNumTiles, gSilverDistribution;
extern	GameStuffRec	gCornerInfo[4];

#endif
