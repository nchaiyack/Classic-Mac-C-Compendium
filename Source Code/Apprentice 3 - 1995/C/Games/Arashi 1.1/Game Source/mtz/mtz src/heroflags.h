/* use by ORing and ANDing with Flags (mz)				*/
/* 1-  Used to store CLOT # for flashing screen 		*/
/* 2- 	"												*/
/* 4- 	"												*/
/* 8-   "												*/
/* 16- flag for if first level done (mz)				*/
/* 32- flag for show ZapMsg	(mz)						*/
/* 64- level select of saved game						*/
/* 128-  first level of a saved game complete			*/
						
#define STBonusGivenMask 				0x0010
#define ShowSuperZapMsgMask 			0x0020
#define SavedGameStartMask				0x0040
#define	SavedgameFirstLVLCompleteMask	0x0080
#define	NOFLAGSMASK 0x0000
#define FLAGSMASK 0x7FFF		/* use for Player.Flags (mz)			*/