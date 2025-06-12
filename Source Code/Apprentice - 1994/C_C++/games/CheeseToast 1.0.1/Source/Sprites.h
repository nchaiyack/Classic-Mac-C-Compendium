// Sprites.h
#include <stdarg.h>
#include "QDOffscreen.h"


/*
 * Constants
 */

#define DEBUGGING		1		// Enable debugging key and debugging sprites
#define MaxSprites		200		// Maximum simultaneous sprites
#define MaxScoreRecords	10		// Number of top scores shown
#define ColorMapSize	1024L	// Size of 32x32 8-bit sprite map
#define MaskMapSize		128L	// Size of 32x32 1-bit MaskMap
#define BulletMapSize	4L		// Size of 2x2 8-bit bullet map
#define IconWidth		32		// Width of Icon
#define IconHeight		32		// Height of Icon
#define BulletWidth		2		// Width/Height of Bullet
#define StartupPICT		128		// Starup Picture Resource ID
#define BackgroundPICT	129		// Background Picture Resource ID
#define SmallLogoPICT	131		// Small Logo Picture ID (Attract Mode)
#define NbrAsteroids	3		// Number compound asteroids in table
#define MaxAngles		16		// Number of vectors in vector table
#define NbrYummies		3
#define AsterPoints		100
#define SaucerPoints	500
#define BarbellPoints	500
#define LevelBonusScore	1000

/*
 * Function Macros
 */
 
#define AniFrameIndex(i)	(i << 10)	// Multiply by 1024 (ColorMapSize)
#define AniRowIndex(i)		(i << 5)	// Multiply by 32 (rowsize)

/*
 * Enumerated Types
 */

enum ShipFlags	 {SF_Fire=0x01, SF_Left=0x02, SF_Right=0x04,
				  SF_Thrust=0x08, SF_Shield=0x10};

enum Sounds		 {S_Firing, S_Explosion, S_BadGuyDebut, S_ShipDebut, S_ShipExplodes,
			 	  S_EnemyFires, S_Thrust, S_DudBullet, 
			 	  S_LevelCompletion, S_CompoundFracture, S_Shield, S_YummyConsume,
			 	  S_HighScore1, S_LetterDisplay, S_ExtraShip, S_Loser, S_Startup,
			 	  S_HighScore2,
			 	  S_NbrSounds};


enum GameStates	{ GS_Attract, GS_Play, GS_GameOver };

enum SpriteTypes {
// Visual Sprites
				  ST_Wheel, ST_Aster, ST_Teacup, ST_WheelR, 
				  ST_WheelG, ST_WheelB, ST_Aster1, ST_Aster2, 
				  ST_Saucer, ST_Barbell, ST_Teapot, ST_TeapotT, 
				  ST_TeapotS, ST_TeapotNew, ST_Yummies, ST_Bullet,
				  ST_Photon, ST_Spark,
// Logic Sprites
				  ST_StatusDisplay,
#if DEBUGGING
				  ST_SpriteCnt, ST_MaxSprite,
#endif
				  NbrSprites};

enum BulletFlags {BF_GoodBullet=0x43FF, BF_BadBullet=0x1C00};

enum GameModes	 {GM_X2=1, GM_X3=2, GM_X5=4};

/*
 * Data Structures
 */

typedef struct {
	short	fireKey;
	short	thrustKey;
	short	leftKey;
	short	rightKey;
	short	shieldKey;
	Boolean	soundLevel;
	Str31	userName;
	char	reserved[30];
} PrefsData;

typedef struct	{
	long	lh,lv;
} LongPoint;

typedef struct {
	Point		pos,oldPos;		// Sprite Coords
	LongPoint	vector;			// Sprite Movement Velocity
	short		type;			// Sprite Type - Index into Sprite Def Table
	short		param1;			// Varies for each sprite type
	short		param2;			// Varies for each sprite type
	short		aniState;		// Animation Frame
	short		aniSpeed;		// Animation Frame Rate
	short		tickCtr;		// Time Counter
	short		lifeSpan;		// Life Span of Sprite
	short		angle;			// Index into Vector Table
	short		width;			// Width of Sprite
	Boolean		active,			// Sprite is Active
				update;			// Sprite needs updating onscreen
	char		saveMap[1024];	// Screen save data
} SpriteInstance;

typedef struct {
	short	type;
	short	firstIconID;
	short	nbrIcons;
	void	(*moveFunc)(register SpriteInstance *sp);
	void	(*eraseFunc)(register SpriteInstance *sp);
	void	(*drawFunc)(register SpriteInstance *sp);
	Ptr		colorMaps;
	Ptr		maskMaps;
} SpriteDef;

typedef struct {
	long	score;
	short	level;
	Str31	name;
} ScoreRecord;

/*
 * Extern Variables
 */

extern short		gResFile;
extern GWorldPtr	gOffScreen;
extern PixMapHandle	gPixMap;
extern short		gMaxSprite,gSpriteCnt,gSparkCnt,gAsteroidCnt,gRemainingShips;
extern short		gYummyCnt;
extern long			gScreenRowBytes,gVideoRowBytes;
extern Ptr			gScreenMem,gVideoMem;
extern Rect			gPlayRect,gOffscreenRect;
extern long			gPlayWidth,gPlayHeight;
extern Boolean		gSoundFlag;
extern long			gGameClockTicks;		// 20 Frames per Second
extern long			gLastDispTime;
extern short		gGameState,gGameLevel;
extern long			gGameScore;
extern short		gScoreMode;
extern SpriteInstance *sTable,*gShip;
extern SpriteDef	sDef[NbrSprites];
extern LongPoint	vecTable[MaxAngles];
extern ScoreRecord	scoreRecord[MaxScoreRecords];
extern PrefsData	gPrefs;

/*
 * Function Declarations
 */

// Low level Display
void MyCopyBits(void);
void MyCopyRect(register Rect *r);

// Picture Display
void DisplayPicture(short picID, short hOffset, short vOffset);

// Game Control
void InitializeGame(WindowPtr theWin);		// 1 Time only inits
void CleanUp(void);							// 1 Time only cleanup
void MainGameLoop(WindowPtr theWin);
void StartupScreen(void);
void BeginAttract(void);
void BeginGame(void);
void EndGame(void);
void InitLevel(void);
void InitStatusDisplay(void);
void StatusPrintf(short x, short y, char *tmp, ...);
void DrawStatusBar(short curLevel, short maxLevel);
void AddScore(short amt);

// Sprite Maintenence
SpriteInstance *NewSprite(void);
void KillSprite(SpriteInstance *sp);
void NewShip(void);
void NewSaucer(void);
void NewBarbell(void);
void NewAsteroid(short type);
void NewYummy(void);
void LaunchBullet(short type, short x, short y, long vx, long vy, short lifeSpan, short bType);
void LaunchSpark(short x, short y, long vx, long vy, short lifeSpan, short color);
void ExplodeSprite(register SpriteInstance *sp, register short lifeSpan);

// Util Funcs
short min(register short x, register short y);
short max(register short x, register short y);

#if DEBUGGING
void NewDebugDisplay(void);
#endif

// State Machine Functions
void StandardSpriteErase(register SpriteInstance *);
void StandardSpriteMove(register SpriteInstance *);
void StandardSpriteDraw(register SpriteInstance *);
void BulletErase(register SpriteInstance *);
void BulletMove(register SpriteInstance *);
void BulletDraw(register SpriteInstance *);
void PhotonDraw(register SpriteInstance *);
void SaucerMove(register SpriteInstance *);
void BarbellMove(register SpriteInstance *);
void YummyMove(register SpriteInstance *sp);
void ShipMove(register SpriteInstance *);
void SparkErase(register SpriteInstance *);
void SparkMove(register SpriteInstance *);
void SparkDraw(register SpriteInstance *);
void NewShipTimer(register SpriteInstance *);
void NullFunc(register SpriteInstance *);
void StatusDraw(register SpriteInstance *);
void QuakeMove(register SpriteInstance *);

#if DEBUGGING
void SpriteCntMove(register SpriteInstance *);
void MaxSpriteMove(register SpriteInstance *);
#endif


/*
 * Game Tables
 */
 
#if SPRITEMAIN

// Note: I'm using fixed point arithmetic here - the upper word is used for
// screen coords

LongPoint	vecTable[MaxAngles] = {
	0L,			-65536L,
	25080L,		-60547L,
	46341L,		-46341L,
	60547L,		-25080L,
	65536L,		0L,
	60547L,		25080L,
	46341L,		46341L,
	25080L,		60547L,
	0L,			65536L,
	-25080L,	60547L,
	-46341L,	46341L,
	-60547L,	25080L,
	-65536L,	0L,
	-60547L,	-25080L,
	-46341L,	-46341L,
	-25080L,	-60547L};

// State Machines
SpriteDef sDef[NbrSprites] = {
	{ST_Wheel, 	500, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster,  700, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Teacup, 600, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelR, 182, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelG, 194, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelB, 206, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster1, 218, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster2, 230, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Saucer, 164, 18,	SaucerMove,			StandardSpriteErase, StandardSpriteDraw},
	{ST_Barbell,4000,12,	BarbellMove,		StandardSpriteErase, StandardSpriteDraw},
	{ST_Teapot, 2000,16,    ShipMove, 			StandardSpriteErase, StandardSpriteDraw},
	{ST_TeapotT,2100,16,    ShipMove, 			StandardSpriteErase, StandardSpriteDraw},
	{ST_TeapotS,2200,16,    ShipMove, 			StandardSpriteErase, StandardSpriteDraw},
	{ST_TeapotNew, 0, 0,	NewShipTimer,		NullFunc,			 NullFunc},
	{ST_Yummies,3000, NbrYummies,	YummyMove,	NullFunc, 			 NullFunc},
	{ST_Bullet,	0,	  0,	BulletMove,			BulletErase,		 BulletDraw},
	{ST_Photon,	0,	  0,	BulletMove,			BulletErase,		 PhotonDraw},
	{ST_Spark,	0,	  0,	SparkMove,			SparkErase,			 SparkDraw},
	{ST_StatusDisplay, 0,0, NullFunc,	    	NullFunc,			 StatusDraw},
#if DEBUGGING
	{ST_SpriteCnt, 0, 0,	SpriteCntMove,		BulletErase,		 PhotonDraw},
	{ST_MaxSprite, 0, 0,	MaxSpriteMove,		BulletErase,		 PhotonDraw},
#endif
	};

#endif

/* End of Sprites.h */