/* #include file for the C interface to InvadeSAT */

//¥ Resource numbers.
enum {
	fileMenuRes = 1004,	//¥ Race menu.
	aboutAlrt = 1000,		//¥ About box.
	gameWindRes = 262,		//¥ game window.

//¥ File menu.
	run = 1,
	sound = 3,
	fastAnimation = 4,
	quit = 6
};

extern MenuHandle fileMenu;
extern long level;
extern Boolean stillRunning;
extern Point globalSpeed;
extern Boolean turnFlag;
extern Handle toffH, dunkH, piuH, kraschH; /* Sounds preloaded in soundConst.c */
extern short missileCount;
extern short downCount, last_H;

extern void		SetUpLevel(short level);
extern void		StartGame(void);
extern void		DoFileMenu(short item);
extern int		MoveIt(void);
extern int		GameWindUpdate(void);
extern void		GameWindIdle(void);
extern int		GameWindInit(void);
extern void		DoAbout(void);
extern void		SetUpMenus(void);
extern void		DoSuspendResume(Boolean b);
extern Boolean	DoEvt(EventRecord e);
extern int		main(void);
extern void		InitEnemy(void);
extern pascal void		SetupEnemy(SpritePtr sp);
extern pascal void		HandleEnemy(SpritePtr me);
extern void		InitMissile(void);
extern pascal void		SetupMissile(SpritePtr sp);
extern pascal void		HandleMissile(SpritePtr me);
extern void		LoadSounds(void);
extern void		InitPlayer(void);
extern pascal void		SetupPlayer(SpritePtr player);
extern pascal void		HandlePlayer(SpritePtr me);
extern void		InitShot(void);
extern pascal void		SetupShot(SpritePtr sp);
extern pascal void		HandleShot(SpritePtr me);
