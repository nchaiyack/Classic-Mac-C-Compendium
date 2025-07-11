/* ----------------------------------------------------------------- */
/* ReadAudioFromCD.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <Sound.h>
#include <Sane.h>
#include <Script.h>
#include <Files.h>
#include <StandardFile.h>

enum csCDCode
{
    csEjectTheDisc = 7,
    csDiscStatus = 8,
    csGetDriveType = 96,
    csWhoIsThere = 97,
    csReadTOC = 100,
    csReadQ = 101,
    csReadHeader = 102,
    csReadMCN = 110,
    csReadISRC = 111,
    csReadAudio = 115
};

enum csReadTOCSubCodes
{
    csReadTOCTrackCount = 1,
    csReadTOCLeadOut = 2,
    csReadTOCStartingAddress = 3,
    csReadTOCTableOfContents = 4,
    csReadTOCNrOfSessions = 5,
    csReadTOCQSubCodes = 6
};

enum csReadAudioCodes
{
    csReadAudioDataOnly = 0, /* 2352 bytes per block */
    csReadAudioWithQSubCode = 1, /* 2368 bytes per block */
    csReadAudioWithAllSubCodes = 2, /* 2448 bytes per block */
    csReadAudioSubCodesOnly = 3 /* 96 bytes per block */
};

enum AddressTypes
{
    logicalBlockAddressType = 0,
    minSecFrameType = 1,
    bcdTrackNumberType = 2
};

enum DriveTypes
{
    AppleCDSC = 1,
    AppleCDSCPlus150 = 2,
    AppleCD300 = 3
};

typedef struct StartingAddress
{
    Byte controlField;
    Byte minutes; /* BCD */
    Byte seconds; /* BCD */
    Byte frames; /* BCD */
} *StartingAddress;

typedef struct
{
    ParamBlockHeader
    short int ioRefNum;
    short int csCode;
    union
    {
	struct
	{
	    short int track;
	    Byte writeProtect;
	    Byte discStatus;
	    Byte installed;
	    Byte side;
	    long int qLink;
	    short int qType;
	    short int dQDrive;
	    short int dQRefNum;
	    short int dQFSID;
	    Byte twoSideFormat;
	    Byte needsFlush;
	    Byte diskErrs;
	} discStatus;
	struct
	{
	    Byte fill;
	    Byte SCSIMask;
	} showIsThere;
	union
	{
	    short int subType;
	    struct /* csReadTOCTrackCount */
	    {
		Byte firstTrackNumber; /* BCD */
		Byte lastTrackNumber; /* BCD */
	    } trackCount;
	    struct /* csReadTOCLeadOut */
	    {
		Byte minutes; /* BCD */
		Byte seconds; /* BCD */
		Byte frames; /* BCD */
	    } leadOut;
	    struct /* csReadTOCStartingAddress */
	    {
		short int subType;
		StartingAddress buffer;
		short int bufferSize;
		Byte startingTrackNumber; /* BCD */
		Byte fill;
	    } startingAddress;
	    struct /* csReadTOCTableOfContents */
	    {
		short int subType;
		void *buffer;
	    } tableOfContents;
	    struct /* csReadTOCNrOfSessions */
	    {
		short int subType;
		short int firstSessionNumber; /* BCD */
		short int lastSessionNumber; /* BCD */
		struct
		{
		    Byte controlField;
		    Byte minutes; /* BCD */
		    Byte seconds; /* BCD */
		    Byte frames; /* BCD */
		} firstTrackOfLastSession;
	    } nrOfSessions;
	    struct /* csReadTOCQSubCodes */
	    {
		short int subType;
		void *buffer;
		short int bufferSize;
		Byte startingSessionNumber; /* BCD */
		Byte fill;
	    } qSubCode;
	} readTOC;
	struct
	{
	    Byte mcValFound; /* MCVal found iff (mcValFound & 0x80) == 0x80 */
	    Byte mcn[15]; /* Media Catalog Number from MSB to LSB */
	} readMCN;
	struct
	{
	    Byte tcValFound; /* ISRC data found iff (tcValFound & 0x80) == 0x80 */
	    Byte isrc[15]; /* Media Catalog Number from MSB to LSB */
	} readISRC;
	struct
	{
	    short int audioType;
	    short int *buffer;
	    short int addressType;
	    union
	    {
		long int logicalBlockAddress;
		struct
		{
		    Byte controlField;
		    Byte minutes; /* BCD */
		    Byte seconds; /* BCD */
		    Byte frames; /* BCD */
		} startTime;
		Byte trackNumber; /* BCD */
	    } address;
	    int numberOfFrames;
	    short int filler[4];
	} readAudio;
	short int driveType;
	short int csParam[11];
    } csParam;
} CDIORec;

typedef union IntelWord
{
    Byte lh[2];
    short int word;
} IWrd;

static int nrTracks;
static struct StartingAddress tracks[100];
static int scsiID;
static int driverRefNum;
static int ioRefNum;
static Handle sndHandle = NULL;
static long int bufSize = 0;
static int startTrack, startMinutes, startSeconds, startFrames,
	   endTrack,   endMinutes,   endSeconds,   endFrames;
static OSErr mcnErr, isrcErr;
static Boolean discPresent;
static struct StartingAddress totalTime;
static Byte mcn[15], isrc[15];
static enum DriveTypes driveType;
static unsigned int channels = 1, resolution = 8, sampleRate = 11025; 
struct SndStruct
{
    unsigned int resource; /* Should be 1 */
    struct SynthDef
    {
	short int nrOfSynths;
	struct
	{
	    short int resourceID;
	    long int initCmd;
	} synth[1];
    } synthDef;
    unsigned int nrOfSoundCommands; /* Should be 1 */
    struct SndCommand soundCommand;
    Ptr data;
    unsigned long int nrOfSamples; /* Or numChannels! */
    Fixed sampleRate;
    unsigned long int loopStart;
    unsigned long int loopEnd;
    unsigned char encode;
    unsigned char baseNote;
    struct ExtHeader
    {
	unsigned long numFrames;
	extended80 AIFFSampleRate;
	Ptr markerChunk;
	Ptr instrumentChunks;
	Ptr AESRecording;
	unsigned short sampleSize;
	unsigned short futureUse1;
	unsigned long futureUse2;
	unsigned long futureUse3;
	unsigned long futureUse4;
    } extHeader;
} genHeader = {1, {1, {{sampledSynth, initMono}}},
	       1, {dataOffsetFlag | bufferCmd, 0, 0x14},
	       NULL, 0, 0x56EE8BA3, 0, 0, stdSH, 60};
static int headerSize;
static long int numFrames;
static Boolean normalize = false;

static void aprintf(char *format, ...)
{
    va_list args;
    char pText[256];

    va_start(args, format);
    pText[0] = vsprintf(pText + 1, format, args);
    va_end(args);
    ParamText(pText, "\p", "\p", "\p");
    Alert(128, NULL);
}

static void dprintf(DialogPtr theDialog, int item, char *format, ...)
{
    va_list args;
    char pText[256];
    int itemType;
    Handle itemHandle;
    Rect itemRect;

    va_start(args, format);
    pText[0] = vsprintf(pText + 1, format, args);
    va_end(args);
    GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
    SetIText(itemHandle, pText);
}

static void dcontrol(DialogPtr theDialog, int item, int value)
{
    int itemType;
    Handle itemHandle;
    Rect itemRect;

    GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
    SetCtlValue(itemHandle, value);
}

static long int dgetint(DialogPtr theDialog, int item)
{
    char pText[256];
    int itemType;
    Handle itemHandle;
    Rect itemRect;
    long int num;

    GetDItem(theDialog, item, &itemType, &itemHandle, &itemRect);
    GetIText(itemHandle, pText);
    StringToNum(pText, &num);
    return (num);
}

static void Signal(OSErr err)
{
    if (err != noErr)
    {
	aprintf("error %i occurred",err);
	exit(1);
    }
}

static Byte Decimal2BCD(Byte n)
{
    return (((n / 10) << 4) + (n % 10));
}

static Byte BCD2Decimal(Byte n)
{
    return (((n >> 4) * 10) + (n & 0x0f));
}

static OSErr InitCD(void)
{
    return (OpenDriver("\p.AppleCD", &driverRefNum));
}

static OSErr CDIO(CDIORec *cdIo)
{
    cdIo->ioCompletion = NULL;
    cdIo->ioNamePtr = NULL;
    cdIo->ioRefNum = ioRefNum;
    return (PBControl((ParmBlkPtr) cdIo, false));
}

static OSErr OpenCD(void)
{
    CDIORec whoIsThereRec;
    OSErr err;

    whoIsThereRec.ioCompletion = NULL;
    whoIsThereRec.ioNamePtr = NULL;
    whoIsThereRec.ioRefNum = driverRefNum;
    whoIsThereRec.csCode = csWhoIsThere;
    if ((err = PBStatus((ParmBlkPtr) &whoIsThereRec, false)) != noErr)
    {
	return (err);
    }
    for (scsiID = 0; scsiID != 7; scsiID++)
    {
	/* Use lowest CD ID found */
	if (whoIsThereRec.csParam.showIsThere.SCSIMask & (1 << scsiID))
	{
	    ioRefNum = -(32 + scsiID) - 1;
	    return (noErr);
	}
    }
    return (badUnitErr);
}

/* Disc in Drive <=> discPresent */
static OSErr DiscStatus(Boolean *discPresent)
{
    OSErr err;
    CDIORec discStatusRec;

    discStatusRec.ioCompletion = NULL;
    discStatusRec.ioNamePtr = NULL;
    discStatusRec.ioRefNum = driverRefNum;
    discStatusRec.csCode = csDiscStatus;
    err = PBStatus((ParmBlkPtr) &discStatusRec, false);
    *discPresent = discStatusRec.csParam.discStatus.discStatus == 1;
    return (err);
}

static OSErr DriveType(enum DriveTypes *driveType)
{
    CDIORec driveTypeRec;
    OSErr err;

    driveTypeRec.ioCompletion = NULL;
    driveTypeRec.ioNamePtr = NULL;
    driveTypeRec.ioRefNum = driverRefNum;
    driveTypeRec.csCode = csGetDriveType;
    err = PBStatus((ParmBlkPtr) &driveTypeRec, false);
    *driveType = driveTypeRec.csParam.driveType;
    return (err);
}

/* Gets number of tracks */
static OSErr TrackCount(int *trackCount)
{
    OSErr err;
    CDIORec readTOC;

    readTOC.csCode = csReadTOC;
    readTOC.csParam.readTOC.subType = csReadTOCTrackCount;
    if ((err = CDIO((CDIORec *) &readTOC)) == noErr)
    {
	*trackCount = BCD2Decimal(readTOC.csParam.readTOC.trackCount.lastTrackNumber) -
		      BCD2Decimal(readTOC.csParam.readTOC.trackCount.firstTrackNumber) + 1;
    }
    return (err);
}

/* Info about total time */
static OSErr TotalDiscTime(StartingAddress dTime)
{
    OSErr err;
    CDIORec readTOC;

    readTOC.csCode = csReadTOC;
    readTOC.csParam.readTOC.subType = csReadTOCLeadOut;
    if ((err = CDIO(&readTOC)) == noErr)
    {
	dTime->minutes = BCD2Decimal(readTOC.csParam.readTOC.leadOut.minutes);
	dTime->seconds = BCD2Decimal(readTOC.csParam.readTOC.leadOut.seconds);
	dTime->frames = BCD2Decimal(readTOC.csParam.readTOC.leadOut.frames);
    }
    return (err);
}

/* Info about track "track" */
static OSErr TrackInfo(Byte track, StartingAddress dTime, int nrTracks)
{
    OSErr err;
    CDIORec readTOC;
    int i;

    readTOC.csCode = csReadTOC;
    readTOC.csParam.readTOC.subType = csReadTOCStartingAddress;
    readTOC.csParam.readTOC.startingAddress.buffer = dTime;
    readTOC.csParam.readTOC.startingAddress.bufferSize = nrTracks * sizeof(struct StartingAddress);
    readTOC.csParam.readTOC.startingAddress.startingTrackNumber = Decimal2BCD(track);
    err = CDIO(&readTOC);
    for (i = 0; i != nrTracks; i++)
    {
	dTime[i].minutes = BCD2Decimal(dTime[i].minutes);
	dTime[i].seconds = BCD2Decimal(dTime[i].seconds);
	dTime[i].frames = BCD2Decimal(dTime[i].frames);
    }
    return (err);
}

static OSErr MediaCatalogNumber(Byte mcn[15])
{
    CDIORec mcnRec;
    OSErr err;

    mcnRec.csCode = csReadMCN;
    err = CDIO(&mcnRec);
    memcpy(mcn, mcnRec.csParam.readMCN.mcn, 15);
    return (err != noErr? err: (mcnRec.csParam.readMCN.mcValFound &0x80) == 0x80? noErr: -1);
}

static OSErr InternationalStandardRecordingCode(Byte isrc[15])
{
    CDIORec isrcRec;
    OSErr err;

    isrcRec.csCode = csReadISRC;
    err = CDIO(&isrcRec);
    memcpy(isrc, isrcRec.csParam.readISRC.isrc, 15);
    return (err != noErr? err: (isrcRec.csParam.readISRC.tcValFound & 0x80) == 0x80? noErr: -1);
}

static OSErr EjectCD(void)
{
    OSErr err;
    Str255 ioName;
    HVolumeParam pb;

    pb.ioCompletion = NULL;
    pb.ioNamePtr = ioName;
    pb.ioVolIndex = 0;
    do
    {
	pb.ioVolIndex++;
	if ((err = PBHGetVInfo((HParmBlkPtr) &pb, false)) != noErr)
	{
	    return (err);
	}
    }
    while (pb.ioVDRefNum != ioRefNum);
    if ((err = PBEject((ParmBlkPtr) &pb)) == noErr)
    {
	err = PBUnmountVol((ParmBlkPtr) &pb);
    }
    return (err);
}

static OSErr ReadAudio(void *data, long int bufSize, int minutes, int seconds, int frames)
{
    CDIORec audioRec;
    OSErr err;

    audioRec.csCode = csReadAudio;
    audioRec.csParam.readAudio.audioType = csReadAudioDataOnly;
    audioRec.csParam.readAudio.buffer = data;
    audioRec.csParam.readAudio.addressType = minSecFrameType;
    audioRec.csParam.readAudio.address.startTime.controlField = 0;
    audioRec.csParam.readAudio.address.startTime.minutes = Decimal2BCD(minutes);
    audioRec.csParam.readAudio.address.startTime.seconds = Decimal2BCD(seconds);
    audioRec.csParam.readAudio.address.startTime.frames = Decimal2BCD(frames);
    audioRec.csParam.readAudio.numberOfFrames = bufSize / 2352;
    memset(audioRec.csParam.readAudio.filler, 0, sizeof(audioRec.csParam.readAudio.filler));
    return (CDIO(&audioRec));
}

static void ReadTime(void)
{
    long int t0;
    OSErr err;

    startFrames += tracks[startTrack - 1].frames;
    startSeconds += startFrames / 75 + tracks[startTrack - 1].seconds;
    startFrames %= 75;
    startMinutes += startSeconds / 60 + tracks[startTrack - 1].minutes;
    startSeconds %= 60;
    endFrames += tracks[endTrack - 1].frames;
    endSeconds += endFrames / 75 + tracks[endTrack - 1].seconds;
    endFrames %= 75;
    endMinutes += endSeconds / 60 + tracks[endTrack - 1].minutes;
    endSeconds %= 60;

    bufSize = (((long int) (endMinutes - startMinutes) * 60 +
		endSeconds - startSeconds) * 75
	       + endFrames - startFrames)
	      * 2352L;
    numFrames = bufSize / 4;
    if ((sndHandle = NewHandle(bufSize + headerSize)) == NULL)
    {
	aprintf("Not enough memory");
	exit(1);
    }
    HLock(sndHandle);

    t0 = TickCount();
    err = ReadAudio(*sndHandle + headerSize, bufSize, startMinutes, startSeconds, startFrames);
    t0 = TickCount() - t0;
    if (err != noErr)
    {
	Signal(err);
    }
    aprintf("%ld bytes read in %g sec.", bufSize, t0 / 60.0);
}

static Boolean GetTimes(void)
{
    DialogPtr timeDialog = GetNewDialog(128, NULL, (char *) -1);
    int item;
    int itemType;
    Handle itemHandle;
    Rect itemRect;

    if (timeDialog == NULL)
    {
	aprintf("???");
	ExitToShell();
    }
    SetPort(timeDialog);
    dprintf(timeDialog, 3, "Number of tracks: %d", nrTracks);
    dprintf(timeDialog, 4, "Total playing time: %d:%02d:%02d",
			    totalTime.minutes, totalTime.seconds, totalTime.frames);
    if (mcnErr == noErr) dprintf(timeDialog, 5, "MCN: %15s", mcn);
    else if (mcnErr == -1) dprintf(timeDialog, 5, "MCN: unknown");
    else dprintf(timeDialog, 5, "MCN: error %d", mcnErr);
    if (isrcErr == noErr) dprintf(timeDialog, 6, "ISRC: %15s", isrc);
    else if (isrcErr == -1) dprintf(timeDialog, 6, "ISRC: unknown");
    else dprintf(timeDialog, 6, "ISRC: error %d", isrcErr);
    dcontrol(timeDialog, 21, 1);
    dcontrol(timeDialog, 23, 1);
    dcontrol(timeDialog, 25, 1);
    dcontrol(timeDialog, 28, 0);
    GetDItem(timeDialog, 1, &itemType, &itemHandle, &itemRect);
    PenSize(3, 3); InsetRect(&itemRect, -4, -4);
    FrameRoundRect(&itemRect, 16, 16);
    do
    {
	ModalDialog(NULL, &item);
	if (item == 21 || item == 22)
	{
	    channels = (item == 21? 1: 2);
	    dcontrol(timeDialog, 21, channels == 1);
	    dcontrol(timeDialog, 22, channels == 2);
	}
	else if (item == 23 || item == 24)
	{
	    resolution = (item == 23? 8: 16);
	    dcontrol(timeDialog, 23, resolution == 8);
	    dcontrol(timeDialog, 24, resolution == 16);
	}
	else if (25 <= item && item <= 27)
	{
	    sampleRate = (item == 25? 11025: (item == 26? 22050: 44100));
	    dcontrol(timeDialog, 25, sampleRate == 11025);
	    dcontrol(timeDialog, 26, sampleRate == 22050);
	    dcontrol(timeDialog, 27, sampleRate == 44100);
	}
	else if (item == 28)
	{
	    normalize = !normalize;
	    dcontrol(timeDialog, 28, normalize);
	}
    }
    while (item != 1 && item != 2);
    startTrack = dgetint(timeDialog, 13);
    startMinutes = dgetint(timeDialog, 14);
    startSeconds = dgetint(timeDialog, 15);
    startFrames = dgetint(timeDialog, 16);
    endTrack = dgetint(timeDialog, 17);
    endMinutes = dgetint(timeDialog, 18);
    endSeconds = dgetint(timeDialog, 19);
    endFrames = dgetint(timeDialog, 20);
    if (channels == 1 && resolution == 8)
    {
	headerSize = sizeof(struct SndStruct) - sizeof(struct ExtHeader);
    }
    else
    {
	headerSize = sizeof(struct SndStruct);
    }
    DisposDialog(timeDialog);
    return (item == 1);
}

static long int MaxVal(void)
{
    long int length = bufSize;
    IWrd *lChan = (IWrd *) (*sndHandle + headerSize);
    IWrd *rChan = lChan + 1;
    long int lDatum, rDatum;
    int nrSamples = (sampleRate == 11025? 4: sampleRate == 22050? 2: 1);
    int i;
    Byte swap;
    long int max = 0;

    while (length > 0)
    {
	lDatum = rDatum = 0;
	for (i = 0; i != nrSamples; i++)
	{
	    swap = lChan->lh[0]; lChan->lh[0] = lChan->lh[1]; lChan->lh[1] = swap;
	    swap = rChan->lh[0]; rChan->lh[0] = rChan->lh[1]; rChan->lh[1] = swap;
	    lDatum += lChan->word; rDatum += rChan->word;
	    lChan += 2; rChan += 2; length -= 4;
	}
	if (channels == 1)
	{
	    lDatum = (lDatum + rDatum) / 2;
	    if (lDatum < 0) lDatum = -lDatum;
	    if (lDatum > max) max = lDatum;
	}
	else
	{
	    if (lDatum < 0) lDatum = -lDatum;
	    if (lDatum > max) max = lDatum;
	    if (rDatum < 0) rDatum = -rDatum;
	    if (rDatum > max) max = rDatum;
	}
    }
    return (max);
}

static void ConvertSound(void)
{
    char *bPtr = *sndHandle + headerSize;
    short int *wPtr = (short int *) bPtr;
    long int length = bufSize;
    IWrd *lChan = (IWrd *) bPtr;
    IWrd *rChan = lChan + 1;
    long int lDatum, rDatum;
    int nrSamples = (sampleRate == 11025? 4: sampleRate == 22050? 2: 1);
    int i;
    Byte swap;
    long int max = (normalize? MaxVal(): 32768);

    while (length > 0)
    {
	lDatum = rDatum = 0;
	for (i = 0; i != nrSamples; i++)
	{
	    if (!normalize)
	    {
		swap = lChan->lh[0]; lChan->lh[0] = lChan->lh[1]; lChan->lh[1] = swap;
		swap = rChan->lh[0]; rChan->lh[0] = rChan->lh[1]; rChan->lh[1] = swap;
	    }
	    lDatum += lChan->word; rDatum += rChan->word;
	    lChan += 2; rChan += 2; length -= 4;
	}
	if (normalize)
	{
	    lDatum = lDatum * 32767 / max;
	    rDatum = rDatum * 32767 / max;
	}
	else
	{
	    lDatum /= nrSamples;
	    rDatum /= nrSamples;
	}
	if (channels == 1)
	{
	    lDatum = (lDatum + rDatum) / 2;
	}
	if (resolution == 8)
	{
	    *bPtr++ = (Byte) (lDatum >> 8) ^ 0x80;
	}
	else
	{
	    *wPtr++ = lDatum;
	}
	if (channels == 2)
	{
	    if (resolution == 8)
	    {
		*bPtr++ = (rDatum >> 8) ^ 0x80;
	    }
	    else
	    {
		*wPtr++ = rDatum;
	    }
	}
    }
    HUnlock(sndHandle);
    SetHandleSize(sndHandle, headerSize + resolution / 8 * channels * numFrames);
    Signal(ResError());
}

static void InitHeader(void)
{
    long double dSampleRate = (double) sampleRate;

    numFrames = numFrames / (44100 / (long unsigned int) sampleRate);
    genHeader.sampleRate = (long unsigned int) sampleRate << 16;
    if (channels == 1 && resolution == 8)
    {
	genHeader.nrOfSamples = numFrames;
    }
    else
    {
	if (channels == 2)
	{
	    genHeader.synthDef.synth[0].initCmd = initStereo;
	}
	genHeader.nrOfSamples = channels;
	genHeader.encode = extSH;
	genHeader.extHeader.numFrames = numFrames;
	x96tox80(&dSampleRate, &genHeader.extHeader.AIFFSampleRate);
	genHeader.extHeader.markerChunk = NULL;
	genHeader.extHeader.instrumentChunks = NULL;
	genHeader.extHeader.AESRecording = NULL;
	genHeader.extHeader.sampleSize = resolution;
	genHeader.extHeader.futureUse1 = 0;
	genHeader.extHeader.futureUse2 = 0;
	genHeader.extHeader.futureUse3 = 0;
	genHeader.extHeader.futureUse4 = 0;
    }
    memcpy((void *) *sndHandle, &genHeader, headerSize);
}

static void SaveSound(void)
{
    StandardFileReply reply;
    int refNum;

    StandardPutFile("\pWhere to put file", "\pSound", &reply);
    if (reply.sfGood)
    {
	if (reply.sfReplacing)
	{
	    FSpDelete(&reply.sfFile);
	}
	FSpCreateResFile(&reply.sfFile, 'movr', 'sfil', smSystemScript);
	Signal(ResError());
	refNum = FSpOpenResFile(&reply.sfFile, fsWrPerm);
	if (refNum == -1)
	{
	    Signal(ResError());
	}
	AddResource(sndHandle, 'snd ', 128, reply.sfFile.name);
	CloseResFile(refNum);
    }
}

static void SetupCD(void)
{
    /* Initialize CD */
    Signal(InitCD());
    Signal(OpenCD());

    /* Check for drive */
    Signal(DriveType(&driveType));
    if (driveType < AppleCD300)
    {
	aprintf("Improper drive");
    }

    /* Check if present and of proper type */
    Signal(DiscStatus(&discPresent));
    if (!discPresent)
    {
	aprintf("No disc present");
	ExitToShell();
    }

    /* Get CD info */
    Signal(TrackCount(&nrTracks));
    Signal(TotalDiscTime(&totalTime));

    /* Get info of all tracks */
    Signal(TrackInfo(1, tracks, nrTracks));

    /* Check for Media Catalog Number */
    mcnErr = MediaCatalogNumber(mcn);

    /* Check for International Standard Recording Code */
    isrcErr = InternationalStandardRecordingCode(isrc);
}

void main(void)
{
    InitGraf(&thePort);
    InitFonts();
    FlushEvents(everyEvent, 0);
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);
    InitCursor();

    SetupCD();
    if (GetTimes())
    {
	ReadTime();
	InitHeader();
	ConvertSound();
	SaveSound();
    }
}

/* ----------------------------------------------------------------- */
/* ReadAudioFromCD.r: compile with SARez */

data 'ALRT' (128) {
    $"0048 0034 00A7 0123 0080 5555 300A"
};

data 'DITL' (128) {
    $"0002 0000 0000 003C 005A 0050 0094 0404"
    $"416C 6173 0000 0000 000A 000A 002A 002A"
    $"A002 0000 0000 0000 000A 0032 002A 00DC"
    $"8802 5E30"
};

data 'DITL' (129) {
    $"001B 0000 0000 0057 0118 006B 0152 0402"
    $"4F6B 0000 0000 0076 0118 008A 0152 0406"
    $"4361 6E63 656C 0000 0000 000A 000A 001A"
    $"0098 8800 0000 0000 000A 00A0 001A 015B"
    $"8800 0000 0000 001E 000A 002D 0131 8800"
    $"0000 0000 0032 000A 0041 0131 8800 0000"
    $"0000 005A 000A 006A 0031 8804 4672 6F6D"
    $"0000 0000 0046 0032 0056 0058 8805 5472"
    $"6163 6B00 0000 0000 0077 000A 0087 0022"
    $"8802 546F 0000 0000 0046 0064 0056 0096"
    $"8806 4D69 6E75 7465 0000 0000 0046 009B"
    $"0056 00CF 8806 5365 636F 6E64 0000 0000"
    $"0046 00D2 0056 0102 8805 4672 616D 6500"
    $"0000 0000 005A 0033 006B 005A 1000 0000"
    $"0000 005A 0069 006B 0090 1000 0000 0000"
    $"005A 00A0 006B 00C7 1000 0000 0000 005A"
    $"00D5 006B 00FC 1000 0000 0000 0076 0033"
    $"0087 005A 1000 0000 0000 0076 0069 0087"
    $"0090 1000 0000 0000 0076 00A0 0087 00C7"
    $"1000 0000 0000 0076 00D5 0087 00FC 1000"
    $"0000 0000 0096 000A 00A8 0048 0604 4D6F"
    $"6E6F 0000 0000 00AA 000A 00BC 004C 0606"
    $"5374 6572 656F 0000 0000 0096 0064 00A8"
    $"0098 0605 3820 6269 7400 0000 0000 00AA"
    $"0064 00BD 00A0 0606 3136 2062 6974 0000"
    $"0000 0096 00B4 00A8 00FC 0606 3131 206B"
    $"487A 0000 0000 00AA 00B4 00BC 00FC 0606"
    $"3232 206B 487A 0000 0000 00BE 00B4 00D0"
    $"00FC 0606 3434 206B 487A 0000 0000 00BE"
    $"000A 00D0 0074 0509 4E6F 726D 616C 697A"
    $"6500"
};

data 'DLOG' (128) {
    $"003C 0020 0116 017D 0005 0100 0000 0000"
    $"0000 0081 00"
};

/* ----------------------------------------------------------------- */
/* older SCSI parts */

/*From the CDROM FAQ:
  There are several ways to read digital audio from Sony CDU 561
  and Sony CDU 8003 mechanisms. Note that the technique of merely
  setting the density (0x82) using MODE SELECT SCSI command as on
  Toshiba 3401s will not work.
 
  Here are three ways to read digital audio Red Book standard
  audio track data across the SCSI bus into your computer complete
  with all sound processing already performed (For example the
  CIRC routine already run and the output is LRLRLR pairs of 16
  bit digital audio samples 2352 bytes per CD-ROM block.
 
  Method 1 : READ CD-DA scsi command 0xD8

  Byte 0: D8
  1: <LUN stuff> 0
  2: <4th most significant byte of logical block address>
  3: <3rd byte>
  4: <2nd>
  5: <1st, lowest of the address>
  6: <4th most significant byte of transfer length
  7: <3rd byte>
  8: <2nd>
  9: <1st, lowest of the number of contiguos blocks to transfer>
  10: <special sub code selector> (0 == normal 2352, other values
      are 01, 02, 03)
  11: <control>
   
  Method 2 : READ CD-DA MSF scsi command 0xD9
   
  byte 0: D9
  1: <LUN stuff> 0
  2: 0
  3: <starting minute in binary not BCD>
  4: <starting second in binary not BCD>
  5: <starting frame (75th of a second) in binary not BCD>
  6: 0
  7: <ending minute in binary not BCD>
  8: <ending second in binary not BCD>
  9: <ending frame (75th of a second) in binary not BCD>
  10: <special sub code selector> (0 == normal 2352 each, other
      values are 01, 02, 03)
  11: <control>
   
  For this one you will need to remember how to convert MSF to
  logical (LBA) address to set the SCSI transfer length correctly
  to avoid the Mac SCSI manager reporting a phase error. to
  calculate the number of bytes total you will get use the formula:

  ((Me-Ms)*60*75 + (Se-Ss) * 75 + (Fe-Fs)) * (2352) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SCSI.h>

#define csDiscStatus 8
#define csWhoIsThere 97
#define csReadTOC 100
#define csReadTOCTrackCount 1
#define csReadTOCEndOfDisk 2
#define csReadTOCStartTime 2
#define csReadQ 101

#define scsiReadXfer 1
#define scsiWriteXfer -1
#define scsiNoXfer 0

#define kSCSIStatusMask 0x1f

#define kGood 0x00
#define kCheckCondition 0x02
#define kBusy 0x08
#define kReservationConflict 0x18

#define kSCSIRetryValue 5

#define bLogicalUnitLSB 5

#define kCommandComplete 0x00
#define kExtendedMessage 0x01
#define kSaveDataPointer 0x02
#define kRestorePointers 0x03
#define kDisconnect 0x04
#define kMessageReject 0x07
#define kLinkedCommandComplete 0x0a
#define kLinkedCommandCompleteWithFlags 0x0b

#define DoSCSISenseUnit(TargetSCSIID, LogicalUnit) -1 /*
FindError(TargetSCSIID) */

enum
{
    eSCSIReservationConflict = 0xf000,
    eSCSIBusy,
    eDisconnect,
    eMessageReject,
    eReplyTOErr
};

static struct
{
    Byte command; /* D9 */
    Byte lun; /* 0 */
    Byte zero1; /* 0 */
    Byte startMinute; /* Not in BCD */
    Byte startSecond;
    Byte startFrame;
    Byte zero2; /* 0 */
    Byte endMinute; /* Not in BCD */
    Byte endSecond;
    Byte endFrame;
    Byte subCodeSelector; /* 0 == normal 2352 each, other values are 01, 02, 03 */
    Byte control; /* ??? */
} readCommand1 =
{
    0xD9, 0x00, 0x00,
    0x01, 0x02, 0x03,
    0x00, 0x01, 0x02,
    0x04, 0x00, 0x00
};

static OSErr DoSCSIOutput(short TargetSCSIID, short scsiCmdSize, void *scsiCmd,
			  SCSIInstr *scsiInst, short RdWrNone, long TimeOut)
{
    short ReturnedSCSIStat;
    short ReturnedSCSIMessage;
    short NumTries = 0;
    short stat;
    OSErr err;
    OSErr err1;
    short i;
    short message;
    
    while (NumTries++ < kSCSIRetryValue) 
    {
	err = SCSIGet();
	if (err == noErr) 
	{
	    err = SCSISelect(TargetSCSIID);
	    if (err == noErr) 
	    {
		err = SCSICmd(scsiCmd, scsiCmdSize);
		if (err == noErr) 
		{
		    stat = SCSIStat();
		    switch (RdWrNone) 
		    {
			case scsiReadXfer:
			    err = SCSIRead((Ptr) scsiInst);
			    break;
			case scsiWriteXfer:
			    err = SCSIWrite((Ptr) scsiInst);
			    break;
		    }
		}
		err1 = SCSIComplete(&ReturnedSCSIStat, &ReturnedSCSIMessage, TimeOut);
		if (err == noErr) 
		{
		    if (err1 == noErr) 
		    {
			switch (ReturnedSCSIStat & kSCSIStatusMask) 
			{
			    case kGood:
				err = noErr;
				break;
			 /* case kConditionMetGood:
				err = noErr;
				break;
			    case kIntermediateGood:
				err = noErr;
				break;
			    case kIntermediateMetGood:
				err = noErr;
				break; */
			    case kReservationConflict:
				err = eSCSIReservationConflict;
				break;
			    case kBusy:
				err = eSCSIBusy;
				break;
			    case kCheckCondition:
				switch (ReturnedSCSIMessage) 
				{
				    case kCommandComplete:
				    case kLinkedCommandCompleteWithFlags:
				    case kLinkedCommandComplete:
					err = DoSCSISenseUnit(TargetSCSIID,
							      scsiCmd->LUN_Res >> bLogicalUnitLSB);
					break;
				    case kExtendedMessage:
					if (SCSIMsgIn(&message) == noErr) 
					{
					    for (i = 0; i < message; i++) 
					    {
						if (SCSIMsgIn(&message) != noErr)
						{
						    break;
						}
					    }
					}
					break;
				    case kDisconnect:
					err = eDisconnect;
					break;
				    case kMessageReject:
					err = eMessageReject;
					break;
				    default:
					if (ReturnedSCSIMessage >= 0x80) 
					{
					    err = noErr;
					} 
					else 
					{
					    err = DoSCSISenseUnit(TargetSCSIID,
								  scsiCmd->LUN_Res >> bLogicalUnitLSB);
					}
					break;
				}
				break;
			}
		    } 
		    else 
		    {
			err = err1;
		    }
		}
		break;
	    }
	}
    }
    if (NumTries >= kSCSIRetryValue) 
    {
	err = eReplyTOErr;
    }
    return (err);
}

static void ReadTime(void)
{
    long int t0;
    int track, startTrack, startMinutes, startSeconds, startFrames,
	       endTrack,   endMinutes,   endSeconds,   endFrames;
    OSErr err;

    /* Enter format as track+min:sec:frame */
    printf("from? ");
    switch (scanf("%d+%d:%d:%d", &startTrack, &startMinutes, &startSeconds, &startFrames))
    {
	case 0:
	    startTrack = 1;
	case 1:
	    startMinutes = 0;
	case 2:
	    startSeconds = 0;
	case 3:
	    startFrames = 0;
    }
    printf("to? ");
    switch (scanf("%d+%d:%d:%d", &endTrack, &endMinutes, &endSeconds, &endFrames))
    {
	case 0:
	    endTrack = 1;
	case 1:
	    endMinutes = 0;
	case 2:
	    endSeconds = 0;
	case 3:
	    endFrames = 0;
    }

    startFrames += tracks[startTrack].frame;
    if (startFrames >= 75)
    {
	startSeconds += startFrames / 75;
	startFrames %= 75;
    }
    startSeconds += tracks[startTrack].second;
    if (startSeconds >= 60)
    {
	startMinutes += startSeconds / 60;
	startSeconds %= 60;
    }
    startMinutes += tracks[startTrack].minute;
    endFrames += tracks[endTrack].frame;
    if (endFrames >= 75)
    {
	endSeconds += endFrames / 75;
	endFrames %= 75;
    }
    endSeconds += tracks[endTrack].second;
    if (endSeconds >= 60)
    {
	endMinutes += endSeconds / 60;
	endSeconds %= 60;
    }
    endMinutes += tracks[endTrack].minute;

    bufSize = 2352L * (long int) ((long int) ((long int)
	      (endMinutes - startMinutes) * 60 + endSeconds - startSeconds)
	      * 75 + endFrames - startFrames);
    if ((data = malloc(bufSize)) == NULL)
    {
	perror("data");
	exit(1);
    }

    tib[0].scParam1 = (long int) data;
    tib[0].scParam2 = bufSize;
    readCommand1.startMinute = startMinutes;
    readCommand1.startSecond = startSeconds;
    readCommand1.startFrame = startFrames;
    readCommand1.endMinute = endMinutes;
    readCommand1.endSecond = endSeconds;
    readCommand1.endFrame = endFrames;
    t0 = TickCount();
    err = DoSCSIOutput(scsiID, sizeof(readCommand1), &readCommand1,
		       tib, scsiReadXfer, 60 + bufSize / 2940);
    t0 = TickCount() - t0;
    if (err != noErr)
    {
	Signal(err);
    }
    printf("%ld bytes read in %g sec.\n", bufSize, t0 / 60.0);
    DumpBytes1(data, bufSize); /* Do something with it... */
}
