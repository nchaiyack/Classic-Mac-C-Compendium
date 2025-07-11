/************************************************************************************
 * CToast.h
 *
 * CheeseToast by Jim Bumgardner
 *
 ************************************************************************************/

#include <stdarg.h>
#include "QDOffscreen.h"
#if __option(profile)			// 6/15 Optional profiling support
#include <profile.h>
#endif

/*
 * Constants
 */

#define DEBUGGING		0		// Enable debugging key and debugging sprites
#define MaxSprites		512		// Maximum simultaneous sprites (incl. bullets/shards)
#define MaxSaveMaps		48		// Maximum Large Sprites
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
#define MaxAngles		32		// Number of vectors in vector table
#define NbrYummies		5		// Number of tasty objects
#define MaxShieldPower	100		// Maximum Shield Power
#define AsterPoints		100		// Points for zapping an asteroid
#define SaucerPoints	500		// Points for zapping a saucer
#define BarbellPoints	600		// Points for zapping a barbell
#define CubePoints		750		// Points for zapping a cube
#define LevelBonusScore	1000	// Points for advancing to next level

#define TopScoreResType	'tScr'
#define PrefsResType	'prfs'

/*
 * Function Macros
 */
 
#define AniFrameIndex(i)	(i << 10)	// Multiply by 1024 (ColorMapSize)
#define AniRowIndex(i)		(i << 5)	// Multiply by 32 (rowsize)

/*
 * Enumerated Types
 */

// Ship Flags - are we firing?  turning? thrusting? shields up?
//
enum ShipFlags	 {SF_Fire=0x01, SF_Left=0x02, SF_Right=0x04,
				  SF_Thrust=0x08, SF_Shield=0x10};

// Sound IDs
//
enum Sounds		 {S_Startup, S_Explosion, S_BadGuyDebut, S_ShipDebut, S_ShipExplodes,
			 	  S_EnemyFires, S_Thrust, S_DudBullet, 
			 	  S_LevelCompletion, S_CompoundFracture, S_Shield, S_YummyConsume,
			 	  S_HighScore1, S_LetterDisplay, S_ExtraShip, S_Loser, S_Firing,
			 	  S_HighScore2,
			 	  S_NbrSounds};


// Game Modes
//
enum GameStates	{ GS_Attract, GS_Play, GS_GameOver };

// Visual Sprites types
enum SpriteTypes {
				  ST_Wheel, ST_Aster, ST_Teacup, ST_Jim,
				  ST_WheelR, ST_WheelG, ST_WheelB, ST_Aster1, 
				  ST_Aster2, ST_Saucer, ST_Barbell, ST_Cube,
				  ST_Teapot,  ST_TeapotT, ST_TeapotS, ST_TeapotNew,
				  ST_Yummies, ST_Bullet, ST_Photon, ST_Spark,
// Logic Sprites
				  ST_StatusDisplay,
#if DEBUGGING
// Debugging Sprites
				  ST_SpriteCnt, ST_MaxSprite,
#endif
				  NbrSprites};

// Mask which identifies valid targets for "good bullets" (based on sprite type)
#define BF_GoodBullet	0x00010FFF

// Mask which identifies valid targets for "bad bullets" (based on sprite type)
#define	BF_BadBullet	0x0000F000

// Ship Modes (special guns, magic shield)
enum ShipModes	 {SM_Triple=1, SM_Uzi=2, SM_AutoShield=4};

/*
 * Data Structures
 */

// User Preferences - keyboard remapping, sound level, username
typedef struct {
	char	fireKeyByte, fireKeyBit, fireKeyCode, fireKeyAscii;
	char	thrustKeyByte, thrustKeyBit, thrustKeyCode, thrustKeyAscii;
	char	leftKeyByte, leftKeyBit, leftKeyCode, leftKeyAscii;
	char	rightKeyByte, rightKeyBit, rightKeyCode, rightKeyAscii;
	char	shieldKeyByte, shieldKeyBit, shieldKeyCode, shieldKeyAscii;
	short	soundLevel;
	Str31	userName;
	char	reserved[30];
} PrefsData;

// Sprite Vector Coordinate system uses fixed point arithmetic
//
typedef struct	{
	long	lh,lv;
} LongPoint;

// Data structure for saving the area of screen underneath sprite
//
typedef struct {
	struct SpriteInstance	*sp;
	short	active;
	short	reserved;
	char	saveMap[1024];
} SaveMapRecord,*SaveMapPtr;

// Data structure for each sprite instance
//
typedef struct SpriteInstance {
	Point		pos,oldPos;		// Sprite Coords
	LongPoint	vector;			// Sprite Movement Velocity
	short		type;			// Sprite Type - Index into Sprite Def Table
	long		param1;			// Varies for each sprite type
	short		param2;			// Varies for each sprite type
	short		aniState;		// Animation Frame
	short		aniSpeed;		// Animation Frame Rate
	short		tickCtr;		// Time Counter
	short		lifeSpan;		// Life Span of Sprite
	short		angle;			// Index into Vector Table
	short		width;			// Width of Sprite
	Boolean		active,			// Sprite is Active
				update;			// Sprite needs updating onscreen
	char		savePixel[4];
	SaveMapPtr	saveMapPtr;
} SpriteInstance;

// Sprite Definition Table - defines behavior for each variety of sprite
// function pointers are used for drawing, erasing and moving
//
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

// Entry in "top 10" scores table
//
typedef struct {
	long	score;
	short	level;
	Str31	name;
} ScoreRecord;

/*
 * Extern Variables - see below for comments
 */

extern short		gResFile;
extern GWorldPtr	gOffScreen;
extern PixMapHandle	gPixMap;
extern Boolean		g12InchMode;
extern short		gMaxSprite,gSpriteCnt,gSparkCnt,gAsteroidCnt,gRemainingShips;
extern short		gNbrSaveMaps;
extern short		gYummyCnt;
extern long			gScreenRowBytes,gVideoRowBytes;
extern Ptr			gScreenMem,gVideoMem;
extern Rect			gPlayRect,gOffscreenRect;
extern Point		gCenterP,gVidOffset;
extern long			gPlayWidth,gPlayHeight;
extern long			gGameClockTicks;		// 20 Frames per Second
extern long			gLastDispTime;
extern short		gGameState,gGameLevel,gShipMode,gShieldPower;
extern long			gGameScore;
extern short		gScoreMultiply;
extern SpriteDef	sDef[NbrSprites];
extern LongPoint	vecTable[MaxAngles];
extern ScoreRecord	scoreRecord[MaxScoreRecords];
extern PrefsData	gPrefs;
extern SaveMapRecord	*smTable;
extern SpriteInstance	*sTable,*gShip;

/*
 * Function Declarations
 */

// Low level Display
void MyCopyBits(void);
void MyCopyBits12Inch(void);
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
void PrintfXY(short x, short y, char *template, ...);
void CenterString(StringPtr str);
void DrawStatusBar(short curLevel, short maxLevel);
void AddScore(short amt);
void GetUserName(StringPtr name);
void CheckKeys(void);

// Sprite Maintenence
SpriteInstance *NewSprite(Boolean saveMapFlag);
void KillSprite(SpriteInstance *sp);
void NewShip(void);
void NewSaucer(void);
void NewBarbell(void);
void NewCube(void);
void NewAsteroid(short type);
void NewYummy(void);
void LaunchBullet(short type, short x, short y, long vx, long vy, short lifeSpan, long bTarget);
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
void CubeMove(register SpriteInstance *);
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


// The following code is only compiled if "SPRITEMAIN" is turned on
//
#if SPRITEMAIN

// Note: I'm using fixed point arithmetic here - the upper word is used for
// screen coords, this table was computed using Excel: Sin() * 65535

LongPoint	vecTable[MaxAngles] = {
				0L,			-65535L,
				12785L,		-64276L,
				25079L,		-60546L,
				36409L,		-54490L,
				46340L,		-46340L,
				54490L,		-36409L,
				60546L,		-25079L,
				64276L,		-12785L,
				65535L,		0L,
				64276L,		12785L,
				60546L,		25079L,
				54490L,		36409L,
				46340L,		46340L,
				36409L,		54490L,
				25079L,		60546L,
				12785L,		64276L,
				0L,			65535L,
				-12785L,	64276L,
				-25079L,	60546L,
				-36409L,	54490L,
				-46340L,	46340L,
				-54490L,	36409L,
				-60546L,	25079L,
				-64276L,	12785L,
				-65535L,	0L,
				-64276L,	-12785L,
				-60546L,	-25079L,
				-54490L,	-36409L,
				-46340L,	-46340L,
				-36409L,	-54490L,
				-25079L,	-60546L,
				-12785L,	-64276L};

GWorldPtr		gOffScreen;		// Offscreen Drawing Area
short			gResFile;		// "CT Resources" file - contains sprites & snds
PixMapHandle	gPixMap;		// Offscreen Pixel Map
Boolean			g12InchMode;	// True if 512 x 384 mode, otherwise 640 x 480
short			gMaxSprite,		// Maximum sprites onscreen
				gSpriteCnt,		// Number of sprites onscreen
				gSparkCnt,		// Number of sparks onscreen
				gAsteroidCnt,	// Number of asteroids onscreen
				gRemainingShips;	// Number of remaining free ships
short			gNbrSaveMaps;	// Number of 32 x 32 savemaps in use
short			gYummyCnt;		// Number of "Yummies" onscreen
long			gScreenRowBytes,	// RowBytes for offscreen area
				gVideoRowBytes;	// RowBytes for onscreen video
Ptr				gScreenMem,		// BaseAddr for offscreen area
				gVideoMem;		// BaseAddr for onscreen video
Rect			gPlayRect,		// Bounds of legal sprite coordinates
				gOffscreenRect;	// Bounds of offscreen drawing area
Point			gCenterP,		// Center of offscreen drawing area
				gVidOffset;		// Top,Left of onscreen Play field 
								// in global coordinates
long			gPlayWidth,		// Width of playing field
				gPlayHeight;	// Height of playing field
long			gGameClockTicks = 3L;		
								// 20 Frames per Second
long			gGameClock;		// Incremented every 3 ticks
long			gLastDispTime = 0L;
								// Ticks of last frame
short			gGameState,		// Attract=0,Play=1,GameOver=2
				gGameLevel,		// Game Level - effects difficulty
				gLastLevel,		// Last Level - used to update display
				gLastRemainingShips;  // Last Remaining Ships - used to update display
short			gShipMode,		// Ship Modes - Magic Shield, Special Bullets
				gShieldPower,	// Power remaining in shields
				gLastShieldPower;	// Used to update display
short			gScoreMultiply,	// Score Multiplier (times two, times three etc)
				gLastScoreMultiply;	// Used to update display
long			gGameScore,		// Current Score
				gLastScore;		// Last Score - used to update display
short			gBadGuyChance,	// Chance of bad guy appearing
				gYummyChance;	// Chance of Yummy appearing
short			oldMBarHeight;	// Old Menu Bar Height

SpriteInstance	*sTable,		// Sprite Table
				*gShip;			// Pointer to Ship Sprite in sprite table, 
								// for quick access
SaveMapRecord	*smTable;		// Save Map Table

// Sprite Definition Table - defines behavior for each variety of sprite
// function pointers are used for drawing, erasing and moving
//
SpriteDef sDef[NbrSprites] = {
//  type      iconID nbrIcons  MoveFunction       EraseFunction        DrawFunction
//  ----      -----  -------   ------------      ---------------     --------------
	{ST_Wheel, 	500, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster,  700, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Teacup, 600, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Jim, 	6000,12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelR, 182, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelG, 194, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_WheelB, 206, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster1, 218, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Aster2, 230, 12,	StandardSpriteMove, StandardSpriteErase, StandardSpriteDraw},
	{ST_Saucer, 164, 18,	SaucerMove,			StandardSpriteErase, StandardSpriteDraw},
	{ST_Barbell,4000,12,	BarbellMove,		StandardSpriteErase, StandardSpriteDraw},
	{ST_Cube,	5000, 18,	CubeMove,			 StandardSpriteErase, StandardSpriteDraw},
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

/* End of CToast.h */