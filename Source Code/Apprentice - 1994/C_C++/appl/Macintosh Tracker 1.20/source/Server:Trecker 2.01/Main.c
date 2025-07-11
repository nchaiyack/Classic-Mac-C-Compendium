/* Main.c */

/* this file is based on "Simple Trecker.c" by Frank Seide */


#include <sound.h>
#include <GestaltEqu.h>
#include <Power.h>
#include "PSyn.h"
#include "STrI.h"
#include "mac_event.h"

#define CREATORCODE ('�Tsq')
#define WAITNEXTEVENTDELAY (30)


extern Boolean								ReceivedOpenEventFlag;
extern char										FakeKeyBuffer[MAXKEYS];
extern int										KeyBufPtr;

extern Boolean								QuitPending;

/* parameters controlling the synthesis, with handy default values. */
extern short									AntiAliasing;
extern short									StereoOn;
extern unsigned short					SamplingRate;
extern short									NumRepeats;
extern short									Speed;
extern short									StereoMix;
extern short									Loudness;

extern int										RecalibratePlayer; /* set when settings change */

extern FSSpec									GlobalFileSpec;


static long										PowerManagerInfo = 0;

static PChannelPtr						pc = NULL;
static SoundTrackHandle				strk = NULL;

static Boolean								Pausing = false;



void				SetParameters(void);


void				discard_buffer(void)
	{
		StopPChannel(pc, true);		/* stop playing with fadeout */
		UnlinkSoundTrack(strk);		/* Unlink STrk from channel */
	}


void				HandleKeyBuffer(void)
	{
		WaitForEvent(15);
	 LoopPoint:
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
				switch (KeyTemp)
					{
						case '+':
	            Loudness += 8;
	            if (Loudness > 64)
	              {
	                Loudness = 64;
	              }
							SetParameters();
							UpdateSoundTrack(strk);
							break;
						case '-':
	            Loudness -= 8;
	            if (Loudness < 0)
	              {
	                Loudness = 0;
	              }
							SetParameters();
							UpdateSoundTrack(strk);
							break;
						case ' ':
							TogglePause();
							break;
						case '>':
							/* (**strk).musicRecord.fastForward = 3; */
							UpdateSoundTrack(strk);
							break;
						case '|':
							/* (**strk).musicRecord.fastForward = 1; */
							UpdateSoundTrack(strk);
							break;
						case '<':
							StopPChannel(pc,false);
							ResetPChannel(pc);
							StartPChannel(pc);
							break;
					}
				goto LoopPoint;
			}
	}


void			SetParameters(void)
	{
		int			i;

		pc->antiAlias = AntiAliasing;
		pc->userMixing = false;
		pc->noSurround = StereoOn;
		StereoPChannel(pc,(Boolean)StereoOn);

		GetSoundTrackWorkspace(strk)->loopDetect = (NumRepeats != 0);

#if 0
		for (i = 0; i < 32; i++)
			{
				PChannelVolume (pc, i, 0x10000);	/* Fixed 0x10000 = 100% */
			}
#endif
		PChannelVolume (pc, -1, (0x10000 * (long)Loudness) / 64);	/* -1 = Gesamtlautst. / -1 denotes total volume */
	}


void			main(void)
	{
		short					i;
		short					WorkingDirectoryRefNum;
		OSErr					Error;
		long					ProcTypeInfo;

		/* this may not be necessary, but I'll do it anyway to avoid problems */
		InitGraf (&thePort);
		InitFonts();
		FlushEvents (everyEvent, 0);
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs (NULL);
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
#if 0
		/* 68020 is no longer required */
		Error = Gestalt(gestaltProcessorType,&ProcTypeInfo);
		if ((Error != noErr) || (ProcTypeInfo == gestalt68000))
			{
				FatalError(FatalError68020NeededID);
				return;
			}
#endif

		if ((PowerManagerInfo & (1 << gestaltPMgrExists)) != 0)
			{
				DisableIdle();
			}

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
				goto ExitPoint;
			}
#endif

		if (OpenPChannel (32,(StereoOn ? opc_stereo : 0) + opc_16Bit,820*8,&pc))
			{
				FatalError(FatalErrorInternalError);
				goto ExitPoint;
			}

		/* change to desired frequency */
		pc->hardFreq = (unsigned long)SamplingRate << 16;

		/* set maximum volume for 4 voices. */
		for (i = 0; i < 4; i += 1)
			{
				PChannelVolume(pc,i,0x10000);
			}

		/* now loading the song.  First, coerce the FSSpec into a working directory */
		OpenWD(GlobalFileSpec.vRefNum,GlobalFileSpec.parID,
			CREATORCODE,&WorkingDirectoryRefNum);
		if (GetSoundTrack(WorkingDirectoryRefNum,GlobalFileSpec.name,0,&strk,FALSE))
			{
				CloseWD(WorkingDirectoryRefNum);
				FatalError(FatalErrorNotASong);
				goto ExitPoint;
			}
		CloseWD(WorkingDirectoryRefNum);

		LinkSoundTrack (strk, pc);

		SetParameters();

		ResetPChannel (pc);
		if (StartPChannel(pc))
			{
				FatalError(FatalErrorInternalError);
				goto PreExitPoint;
			}

		/* now we do event loop waiting for codes & finish... */
		while (!QuitPending)
			{
				if (RecalibratePlayer)
					{
						SetParameters();
						UpdateSoundTrack(strk);
					}
				HandleKeyBuffer(); /* calls WaitForEvent */
				/* is the soundtrack done? */
				if (GetSoundTrackWorkspace(strk)->nextOne)
					{
						QuitPending = true;
					}
			}

	 PreExitPoint:
		ClosePChannel(pc);

	 ExitPoint:
		if ((PowerManagerInfo & (1 << gestaltPMgrExists)) != 0)
			{
				EnableIdle();
			}
	}


void				TogglePause(void)
	{
		if (Pausing)
			{
				Pausing = false;
				StartPChannel(pc);
			}
		 else
			{
				Pausing = true;
				StopPChannel(pc,false);
				while (Pausing && !QuitPending)
					{
						HandleKeyBuffer();
					}
			}
	}
