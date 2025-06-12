typedef struct {
	Handle	buffer;
	short	headerlength;
	long	buffSize;
	Boolean	playable;
} BufInfo;

typedef enum play_record
	{
	iPlay,
	iRecord
	}
PlayOrRec;

typedef struct {
	Boolean			fullBuffer;
	Boolean 		loudEnough;
	short			bufferGettingFilled;
	short			numBuffers;
	BufInfo			*buffers;


	SPBPtr			RecordRec;
	long			SoundRefNum;
	SndChannelPtr	rightChan;
	SndChannelPtr	leftChan;
	Fixed			sampleRate;
	Boolean			firstTime;
	PlayOrRec		direction;
} SCGlobals;


void		BufPlay (Handle Buf, SndChannelPtr	channel);
Handle		SetUpSounds (Handle Buf, short *HeaderSize, Fixed sampRate);
short		RndRange(short min, short max);
short		PickPlayableBuffer(BufInfo *buffers, short max);
short		PickReadableBuffer(BufInfo *buffers, short max);
void		InitSoundConfusion(void);
void		ConfuseSound(void);
void		StopConfusion(void);
void		DoAbout(void);
pascal void	MyRecComp (SPBPtr inParamPtr);
long		TrimBuffer(Handle buffer, long headerlen);
void		ResetChannels(void);
unsigned char *Pstrcat(register Str255 s1, register Str255 s2);

/* Resources */
#define rMenuBar	128

#define mApple		128
#define iAbout		1

#define mFile		129
#define iQuit		1

#define mEdit		130

#define rAboutAlert	128
#define rErrAlert	129

#define rErrStrings	128
enum
	{
	iGeneral=1,
	iAEErr,
	iPlaying,
	iSetupBuffer,
	iNoInput,
	iGestaltFailed,
	iStarting,
	iMemory,
	iReadingLevel,
	iRecording,
	iClosing,
	iMakingGlobals,
	iOpeningDevice,
	iGettingRate,
	iMakingRecordRec,
	iAllocatingBuffers,
	iMakingLeft,
	iMakingRight,
	iMakingMono,
	iGettingSMStatus
	};

#define rSettingStrings	129
enum
	{
	iThreshold=1,
	iBufferSize
	};
