#define MAX_ARP 3
#define NUMBER_FINETUNES 17
#define AMIGA_CLOCKFREQ 33659820		//35758720
#define MIN_PITCH 142
#define MAX_PITCH 17161L
#define MIN_VOLUME 0
#define MAX_VOLUME 64
#define ACCURACY 12
#define fix_to_int(x) ((x) >> ACCURACY)
#define int_to_fix(x) ((x) << ACCURACY)
#define	FREQBASE	161L	// -> Utilisé avec le pitch qui est divisé par 10 !!!!!
#define MAXTRACK	32
#define MAXINSTRU	64

struct VoiceActive	{
		Ptr			MaxPtr;
		Ptr			InstruVoice;
		long		loopWord;
		long		loopWords;
		int			whichInstru;
		int			whichInstruOld;
		int			lastpitch;
		int			pitch;              /* current pitch of the sample */
		int 		realpitch;
		int			volume;
		int			arg;
    	int 		note;               /* we have to save the note cause */
                            			/* we can do an arpeggio without a new note */
    
		int 		arp[MAX_ARP];       /* the three pitch values for an arpeggio */
		int 		arpindex;           /* an index to know which note the arpeggio is doing	*/

		int			viboffset;          /* current offset for vibrato (if any) */
		int 		vibdepth;           /* depth of vibrato (if any) */

		int 		slide;              /* step size of pitch slide */

		int 		pitchgoal;          /* pitch to slide to */
		int 		pitchrate;          /* step rate for portamento */
		int			oldpitchrate;

		int 		volumerate;         /* step rate for volume slide */

		int 		vibrate;            /* step rate for vibrato */
		int			oldvibrate;

		int 		retrig;             /* delay for extended retrig command */
		int 		effect;
		int			ID;
};
typedef		struct VoiceActive	VoiceActive;

struct	MODPartition	{
		SoundTrackSpec	*header;
		struct	Pattern	*partition;
		Ptr				instrument[ 64];
};
typedef		struct MODPartition	MODPartition;

#ifdef MAINSYSTEM
#define EXT
#else
#define EXT extern
#endif

EXT	MODPartition			thePartition;
EXT	VoiceActive				theVoiceActive[ MAXTRACK];
EXT	int						DriverTypeInt, smallcounter;
EXT	unsigned char			InstruEffect;
EXT	int						Tube[ MAXTRACK], PartitionReader, Pat, PL, VolExt[ MAXTRACK];
EXT	struct Pattern			*PatternInt;
EXT	long					DBTick, ASCBUFFER;
EXT	int						TotInstru, speed, finespeed, **FreqHandle, InstruActif[ MAXINSTRU], InstruTube[ MAXINSTRU];
EXT	Ptr						SysHeapPtr, Vol, IntPtr[ 2], IntPtr2[ 2], PseudoBuf[ 2];
EXT	Boolean					MODFileType, InternalRestart, Reading, MusiqueFertig, SMTicker;
EXT	SoundTrackSpec			*theFileInt;
EXT	long					oldNotePosAA, SInc, VMOD, VExt, FreqExt;

EXT	Boolean					DriveStereo, PtrSystem;
EXT	int						DriveOutBit, DriveTrackNo;

extern	int	Tuning[ ], vibrato_table[ ];


enum {
	ASCMono = 1,
	ASCStereo = 2,
	SMMono = 3,
	SMStereo = 4,
	SMDSP = 5
};

enum {
		set_arpeggio = 0,
		set_speed = 15,
		set_skip = 13,
		set_fastskip = 11,
		set_volume = 12,
		set_slidevol = 10,
		set_offset = 9,
		set_portamento = 3,
		set_portaslide = 5,
		set_upslide = 2,
		set_downslide = 1,
		set_vibrato = 4,
		set_vibratoslide = 6,
		set_extended = 14,
		set_nothing = 7,
		do_retrig = 50,
		do_cut = 51
	};

void	Sampler8in8( VoiceActive *curVoice, Ptr	ASCBuffer);
void	Sampler8in8Add( VoiceActive *curVoice, Ptr	ASCBuffer);
void	NoteAnalyse( void);
OSErr	RInitMOD( int, int);
void	ChangeTrackNo( int);
OSErr	RLoadMOD( Str255 fName);
OSErr	RPlayMOD(void);
OSErr	RStopMOD(void);
OSErr	RQuitMOD(void);
OSErr	RResetMOD(void);
OSErr	RClearMOD(void);
OSErr	SetUpFrequence( long, long);
OSErr	RLoadMADFRsrc( OSType IDName, short	IDNo);
OSErr	ReadInstrument( short srcFile, long	MODSize, Boolean aRsrc, Handle);
OSErr	AmigatoSnd22(Handle sound, long SizeH);
OSErr	SetMODVol(void);
OSErr 	SetMODVol2(void);
void	DoEffect( VoiceActive *);
OSErr	DBSndClose();
void	StopChannel();
OSErr 	SetMODVol4( long , long , Boolean );
void	PlayChannel();
void 	checkpitch( VoiceActive *, Boolean);
void 	BufferCopyM();
OSErr	RInstallMADF( MODPartition aPartition);
void 	BufferCopyS();
void	ConvertMod2Mad( Ptr		aMOD, long MODSize, MODPartition		*theMAD);
Ptr		ConvertMad2Mod( MODPartition *theMAD, long MODSize);
void 	TestBufferCopyS();
void	RemplaceMODFile( Ptr theNewPtr);
void	MODPlay(void);
void	InstallMODVBL(void);
struct 	Command* GetCommand( short Pos, short	TrackId, short Pat);
void 	ReadNote( VoiceActive *curVoice, struct Command		*theCommand);
void	RemoveMODVBL(void);
void	MODRelance(void);
void	AmigatoSnd(Handle sound, long SizeH);
void	Play(void);
void	BufferCopyLeft(void);
void	BufferCopyRight(void);
void	VIAOn(void);
void 	SndOff(void);
void 	SndOn(void);
void 	BufferTestM(void);
void	VIAOn2(void);
void 	VIAOn3(void);
OSErr	InitDBSoundManager( long);
void	SetUpEffect( int Arg, VoiceActive *ch);
void	DoEffect( VoiceActive *ch);