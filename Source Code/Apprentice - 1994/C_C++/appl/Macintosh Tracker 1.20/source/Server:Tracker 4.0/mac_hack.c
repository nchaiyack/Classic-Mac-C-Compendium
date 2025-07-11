/* mac_hack.c */

/* All the stuff in this file was written by Thomas R. Lawrence. */
/* See the "mac_readme" or "mac_programmer_info" files for more information */
/* about the Macintosh port */

#include "mac_hack.h"
#include "mac_event.h"
#include <console.h>
#include <unix.h>

#include <Aliases.h>
#include <SANE.h>
#include <GestaltEqu.h>
#include <Power.h>


/* stuff from mac_event.c */
extern FSSpec						GlobalFileSpec;
extern char							FakeKeyBuffer[MAXKEYS];
extern int							KeyBufPtr;
extern Boolean					QuitPending;
extern short						Pausing;
extern Boolean					ReceivedOpenEventFlag;

extern short						AntiAliasing;
extern short						StereoOn;
extern unsigned short		SamplingRate;
extern short						NumRepeats;
extern short						Speed;
extern short						StereoMix;
extern short						Loudness;


#define VOLUMEINCREMENT (8)

#define SMALL_MAX_TIME_SLICE (10)
#define LARGE_MAX_TIME_SLICE (120)
#define WHATISBREATHINGSPACE (48) /* about 16 seconds at 22254 Hz */

long										PowerManagerInfo = 0;


void			ReenableIdleState(void)
	{
		if ((PowerManagerInfo & (1 << gestaltPMgrExists)) != 0)
			{
				EnableIdle();
			}
	}


#define MAXCOUNT (3) /* every MAXCOUNTth call to may_getchar actually does something */

static unsigned long		LastEventTime = 0;
static short						Count = MAXCOUNT;

/* this is called by the tracker code to get keypresses from the user. */
/* we use it to manage use of processor. */
int        may_getchar(void)
  {
	 LoopPoint:
		if (QuitPending)
			{
				end_all("");
			}

		if (Pausing)
			{
				goto DoThingPoint;
			}

		if (Count < 0)
			{
				Count = MAXCOUNT;
			}
		 else
			{
				Count -= 1;
				return EOF;
			}

 		if (NumberPendingBlocks() < WHATISBREATHINGSPACE)
 			{
 				/* if we don't have very many blocks prepared, then we don't call */
 				/* the event routine for a long time.  In other words, we give */
 				/* ourselves a LARGE_MAX_TIME_SLICE */
		 		if (TickCount() - LastEventTime > LARGE_MAX_TIME_SLICE)
		 			{
		 			 DoThingPoint:
		 				WaitForEvent(0);
		 				LastEventTime = TickCount();
		 			}
		 	}
		 else
			{
				/* otherwise, we grant ourselves a SMALL_MAX_TIME_SLICE during */
				/* which to construct more blocks */
		 		if (TickCount() - LastEventTime > SMALL_MAX_TIME_SLICE)
		 			{
		 				goto DoThingPoint;
		 			}
			}

		/* extract 'keypresses' from our buffer.  First we handle our own */
		/* special volume controls and our special pause function, and if the */
		/* key wasn't one of those, we pass it back to the caller to be handled. */
		if (KeyBufPtr > 0)
			{
				int					Scan;
				int					KeyTemp;

				KeyTemp = (unsigned char)FakeKeyBuffer[0];
				for (Scan = 1; Scan < KeyBufPtr; Scan += 1)
					{
						FakeKeyBuffer[Scan - 1] = FakeKeyBuffer[Scan];
					}
				KeyBufPtr -= 1;
				if (KeyTemp == '+')
					{
            Loudness += VOLUMEINCREMENT;
            if (Loudness > 255)
              {
                Loudness = 255;
              }
            ResetVolumeTable();
            goto LoopPoint;
          }
        if (KeyTemp == '-')
        	{
            Loudness -= VOLUMEINCREMENT;
            if (Loudness < 0)
              {
                Loudness = 0;
              }
            ResetVolumeTable();
            goto LoopPoint;
          }
        if (KeyTemp == ' ')
        	{
        		TogglePause();
        		goto LoopPoint;
        	}
        if (KeyTemp == '<')
        	{
        		/* fudge rewind into restart, for consistency */
        		KeyTemp = 'r';
        	}
        if ((KeyTemp == '>') || (KeyTemp == '|'))
        	{
        		/* these don't work well in this version because it doesn't run */
        		/* at interrupt level, but instead precomputes the song as far */
        		/* ahead as possible.  This usually means, on a fast computer, that */
        		/* you don't hear the result of clicking the button until 5 or */
        		/* 10 seconds later.  So I just block them for the time being.  If */
        		/* I or someone else hacks this to run at interrupt time, then */
        		/* we can put these back in. */
        		goto LoopPoint;
        	}
				return KeyTemp;
			}
		 else
		 	{
		 		return EOF;
		 	}
  }


/* this is the real main function.  We basically wait for the open event, */
/* then using the parameters sent to use, we construct a fake little command line */
/* string which is passed to the tracker's main function.  It never knows */
/* what happened. */
#undef main
void        main(void)
  {
    int							argc;
    char*						argv[14];
    OSErr						Error;
    long						ProcTypeInfo;
    char						SamplingRateString[20];
    char						RepeatsString[20];
    char						SpeedString[20];
    char						MixString[20];

		MaxApplZone();
		InitGraf(&thePort);
		InitFonts();
		FlushEvents(everyEvent,0);
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs(NULL);
		InitCursor();

		if (!RegisterEventHandlers())
			{
				return;
			}

		Error = Gestalt(gestaltPowerMgrAttr,&PowerManagerInfo);
		if (Error != noErr)
			{
				return;
			}
		if ((PowerManagerInfo & (1 << gestaltPMgrExists)) != 0)
			{
				DisableIdle();
				atexit(&ReenableIdleState);
			}
#if __option(mc68020)
		Error = Gestalt(gestaltProcessorType,&ProcTypeInfo);
		if ((Error != noErr) || (ProcTypeInfo == gestalt68000))
			{
				FatalError(FatalError68020NeededID);
				return;
			}
#endif

#ifdef AskForFile
		{
			Point								Thing = {50,50};
			StandardFileReply		R;

			/* this is only used for debugging when we don't have access to the interface */
			/* program (since the interface program can't link to this program when it */
			/* is being run under the debugger.) */
			StandardGetFile(NULL,-1,NULL,&R);
			GlobalFileSpec = R.sfFile;
		}
#else
		/* waiting for open document command to come */
		while (!ReceivedOpenEventFlag && !QuitPending)
			{
				WaitForEvent(60);
			}
		if (QuitPending)
			{
				return;
			}
#endif

		/* constructing parameter list */
		argc = 14;
		argv[0] = "tracker";
		argv[1] = "-oversample";
		if (AntiAliasing)
			{
				argv[2] = "2";
			}
		 else
			{
				argv[2] = "1";
			}
		if (StereoOn)
			{
				argv[3] = "-stereo";
			}
		 else
			{
				argv[3] = "-mono";
			}
		argv[4] = "-frequency";
		argv[5] = SamplingRateString; sprintf(SamplingRateString,"%d",(int)SamplingRate);
		argv[6] = "-repeats";
		argv[7] = RepeatsString; sprintf(RepeatsString,"%d",(int)NumRepeats);
		argv[8] = "-speed";
		argv[9] = SpeedString; sprintf(SpeedString,"%d",(int)Speed);
		argv[10] = "-mix";
		argv[11] = MixString; sprintf(MixString,"%d",(int)StereoMix);
		argv[12] = "-tolerant";
		argv[13] = "FileName";

    main2(argc,argv);
  }


void* popen(char* pipe, char* Mode)
  {
    perror("Macintosh doesn't have pipes!");
    FatalError(FatalErrorCantOpenCompressedFiles);
    end_all("");
  }


void pclose(FILE* file)
  {
  }




#include "defs.h"
#include "extern.h"
#include "song.h"
#include "channel.h"

#include "tags.h"


int show;  /* prototype in extern.h */




int MYprintf(...)
	{
	}

int MYfprintf(...)
	{
	}

int MYfgetc(FILE* FileToGetFrom)
	{
		unsigned char			Temp;
		long							Length;
		OSErr							Error;

		if (FileToGetFrom->BufPtr == FILEBUFFERSIZE)
			{
				FileToGetFrom->BufPtr = 0;
				Length = FILEBUFFERSIZE;
				SetFPos(FileToGetFrom->MacFileHandle,fsFromStart,FileToGetFrom->Index);
				Error = FSRead(FileToGetFrom->MacFileHandle,&Length,
					&(FileToGetFrom->Buffer[0]));
			}
		Temp = FileToGetFrom->Buffer[FileToGetFrom->BufPtr];
		FileToGetFrom->Index += 1;
		FileToGetFrom->BufPtr += 1;
		if (FileToGetFrom->Index >= FileToGetFrom->EndOfFile)
			{
				return EOF;
			}
		 else
			{
				return Temp;
			}
	}

int MYfputc(int CharToPut, FILE *FileToPutTo)
	{
	}

FILE* MYfopen(char* FileName, char* Mode)
	{
		OSErr				Error;
		FILE*				Temp;
		short				FileHandleTemp;

		Error = FSpOpenDF(&GlobalFileSpec,fsCurPerm,&FileHandleTemp);
		if (Error != noErr)
			{
				BUG("\pError opening file in MYfopen");
				FatalError(FatalErrorCouldntOpenFile);
				end_all("");
				/* return NULL; */
			}
		 else
			{
				Temp = (FILE*)NewPtr(sizeof(FILE));
				Temp->MacFileHandle = FileHandleTemp;
				Temp->Index = 0;
				GetEOF(FileHandleTemp,&(Temp->EndOfFile));
				Temp->BufPtr = FILEBUFFERSIZE; /* force refill upon first read */
				return Temp;
			}
	}

int MYfclose(FILE* FileToClose)
	{
		OSErr				Error;

		if (FileToClose != NULL)
			{
				Error = FSClose(FileToClose->MacFileHandle);
				if (Error != noErr)
					{
						FatalError(FatalErrorCouldntCloseFile);
						BUG("\pError closing file in MYfclose");
					}
				DisposPtr((Ptr)FileToClose);
			}
	}

int MYfread(char* PlaceToPut, int SizeOfElement, int NumElements, FILE* TheFile)
	{
		long		NumBytesDone;
		OSErr		LastError;
		long		NumberOfBytes;
		long		LengthTemp;

		SetFPos(TheFile->MacFileHandle,fsFromStart,TheFile->Index);
		NumberOfBytes = SizeOfElement * NumElements;
		TheFile->Index += NumberOfBytes;
	 LoopPoint:
		NumBytesDone = NumberOfBytes;
		LastError = FSRead(TheFile->MacFileHandle,&NumBytesDone,PlaceToPut);
		PlaceToPut += NumBytesDone;
		NumberOfBytes -= NumBytesDone;
		if ((LastError == noErr) && (NumberOfBytes != 0)) goto LoopPoint;
		if (LastError != noErr)
			{
				BUG("\pError reading from file in MYfread");
			}
		TheFile->BufPtr = TheFile->Index & (FILEBUFFERSIZE - 1);
		SetFPos(TheFile->MacFileHandle,fsFromStart,TheFile->Index - TheFile->BufPtr);
		LengthTemp = FILEBUFFERSIZE;
		FSRead(TheFile->MacFileHandle,&LengthTemp,&(TheFile->Buffer[0]));
		return LastError;
	}

void* MYmalloc(long SizeOfBlock)
	{
		void*			Block;

		Block = NewPtrClear(SizeOfBlock);
		if (Block == NULL)
			{
				FatalError(FatalErrorOutOfMemory);
				end_all("");
			}
		return Block;
	}

void MYfree(void* Block)
	{
		DisposPtr(Block);
	}

void *MYcalloc(size_t NumThings, size_t SizeOfThing)
  {
  	return malloc((NumThings + 1) * SizeOfThing); /* extra for antialiasing byte */
  }

#undef exit
void MYexit(int Value)
	{
		/* ExitToShell(); */
		exit(Value); /* we DO want to call "atexit"ed things, such as powerbook idle fixer */
	}

char* MYgetenv(char* MeaninglessParameter)
	{
		return NULL;
	}

void MYperror(char* ErrorMessage)
	{
	}

void MYputs(char* Message)
	{
	}

long double MYfloor(long double Base)
	{
		return (long int)Base;
	}

long double MYpow(long double Base, long double Exponent)
	{
		extended			ExtBase,ExtExponent,ExtResult;
		long double		DblResult;

		x96tox80(&Base,&ExtBase);
		x96tox80(&Exponent,&ExtExponent);
		ExtResult = power(ExtBase,ExtExponent);
		x80tox96(&ExtResult,&DblResult);
		return DblResult;
	}





GENERIC		begin_info(char *title) /* declared as GENERIC begin_info() */
	{
		return 0;
	}


void			display_pattern(int current, int total)
	{
	}


void			end_info(GENERIC handle)
	{
	}


int       run_in_fg(void)  /* prototype in mac_hack.h is BOOL run_in_fg(void) */
  {
    return TRUE;
  }


void      sane_tty(void)
  {
  }


void      nonblocking_io(void)
  {
  }


LOCAL struct tag end_marker;

struct tag *get_ui()
	{
		may_getchar();
		end_marker.type = TAG_END;
		return &end_marker;
	}


void info(GENERIC handle, char *line)
	{
	}


void infos(GENERIC handle, char *s)
	{
	}


void notice(char *s)
	{
	}


char *new_scroll(void)
	{
	}


void song_title(char *s)
	{
	}


void scroll()
	{
	}


void status(char *s)
	{
	}
