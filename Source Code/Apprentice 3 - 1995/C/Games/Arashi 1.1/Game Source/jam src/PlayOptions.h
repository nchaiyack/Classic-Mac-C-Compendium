/*/
     Project Arashi: PlayOptions.h
     Major release: Version 1.1, 7/22/92

     Last modification: Friday, March 12, 1993, 22:12
     Created: Sunday, March 17, 1991, 0:24

     Copyright � 1991-1993, Juri Munkki
/*/

#define	OPTIONSFILENAMESTRING	128
#define	OPTIONSDIALOG			128

typedef	struct
{
	int		absMoveFlag;
	int		mouseSensitivity;
	int		rotationType;
	int		blankUnused;
	int		monochrome;
	int		soundOff;
	int		noLoudSounds;
	int		verticalGame;
	int		sys607Sound;
	int		restart;					/* (mz) how to restart */
	int		showfscores;				/* (mz) */
	int		noBackgroundTasks;
}	PlayOptionsRecord;

#ifdef	MAINOPTIONS
PlayOptionsRecord	*PlayOptions;
#else
extern	PlayOptionsRecord	*PlayOptions;
#endif