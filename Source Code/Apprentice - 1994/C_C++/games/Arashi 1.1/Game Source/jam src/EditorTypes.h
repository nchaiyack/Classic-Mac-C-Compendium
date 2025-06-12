/*/
     Project Arashi: EditorTypes.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 22:13
     Created: Saturday, October 12, 1991, 14:29

     Copyright � 1991-1992, Juri Munkki
/*/

typedef struct
{
	int			points;
	int			freq;
	int			count;
	int			speed;
	int			color;
	long		extra[4];
}	SampoTankType;

typedef struct
{
	char		levelName[16];
	char		levelDesigner[64];
	int			fieldId;
	int			fieldColor;
	int			paletteId;
	int			levelBonus;
	int			levelFlags;
	int			levelFlyThruWait;
	int			emptyCount;
	int			extraProbability;
	long		levelExtra[4];
	
	int			playerSpeed;
	int			playerColor;
	int			playerMaxShots;
	int			playerShotSpeed;
	int			playerShotColors[5];
	int			playerSuperZapperCount;
	long		playerExtra[4];
	
	int			flipperPoints;
	int			flipperFreq;
	int			flipperCount;
	int			flipperColor;
	int			flipperSpeed;
	int			flipperRotSpeed;
	long		flipperExtra[4];
	
	int			spikerPoints;
	int			spikerFreq;
	int			spikerCount;
	int			spikerColor;
	int			spikerUpSpeed;
	int			spikerDownSpeed;
	
	int			spikePoints;
	int			spikeInitialLength;
	int			spikeMaxLength;
	int			spikeDurability;
	
	int			spikerPlasmaFlags;
	long		spikerPlasmaPoints;
	int			spikerPlasmaSpeed;
	int			spikerPlasmaColors[2];
	long		spikerExtra[4];
	
	int			pulsarPoints;
	int			pulsarFreq;
	int			pulsarCount;
	int			pulsarColor;
	int			pulsarSpeed;
	int			pulsarRotSpeed;
	int			pulsarPulseTime;
	long		pulsarExtra[4];
	
	int			fusePoints;
	long		fuseBullsEyePoints;
	int			fuseFreq;
	int			fuseCount;
	int			fuseColors[5];
	int			fuseWarpProbability;
	int			fuseAgressionProbability;
	long		fuseExtra[4];
	
	SampoTankType	tanks[3];

}	EditorLevelType;
