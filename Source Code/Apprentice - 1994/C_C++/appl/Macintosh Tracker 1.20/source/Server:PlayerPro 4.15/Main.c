/* Main.c */

/* this file is based on "Simple Trecker.c" by Frank Seide */


#include <sound.h>
#include <GestaltEqu.h>
#include <Power.h>
#include "mac_event.h"
#include "MAD.h"
#include "RDriver.h"

#define CREATORCODE ('ºTss')
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


long													PowerManagerInfo = 0;

Boolean												Pausing = false;


void				SetParameters(void);


void				discard_buffer(void)
	{
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
							break;
						case '-':
	            Loudness -= 8;
	            if (Loudness < 0)
	              {
	                Loudness = 0;
	              }
							SetParameters();
							break;
						case ' ':
							TogglePause();
							break;
						case '>':
							break;
						case '|':
							break;
						case '<':
							break;
					}
				goto LoopPoint;
			}
	}


void			SetParameters(void)
	{
	}


void			main(void)
	{
		short					i;
		short					WorkingDirectoryRefNum;
		OSErr					Error;
		long					ProcTypeInfo;

		Boolean				Stereo;
		Boolean				StereoMixing;
		Boolean				NewSoundManager;
		Boolean				hasASC;
		long					myBit;
		long					gestaltAnswer;
		NumVersion		nVers;


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
		Error = Gestalt(gestaltProcessorType,&ProcTypeInfo);
		if ((Error != noErr) || (ProcTypeInfo == gestalt68000))
			{
				FatalError(FatalError68020NeededID);
				return;
			}

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

		/* now loading the song.  First, coerce the FSSpec into a working directory */
		OpenWD(GlobalFileSpec.vRefNum,GlobalFileSpec.parID,
			CREATORCODE,&WorkingDirectoryRefNum);
		SetVol((void*)0L/*novolname*/,WorkingDirectoryRefNum);
		if (noErr != RLoadMOD(GlobalFileSpec.name))
			{
				CloseWD(WorkingDirectoryRefNum);
				FatalError(FatalErrorInternalError);
				goto ExitPoint;
			}
		CloseWD(WorkingDirectoryRefNum);

		/* find out what driver we should use */
		Gestalt(gestaltHardwareAttr,&gestaltAnswer);
		myBit = gestaltHasASC;
		if (!BitTst(&gestaltAnswer,31-myBit))
			{
				hasASC = false;
			}
		 else
			{
				hasASC = true;
			}
		Gestalt(gestaltSoundAttr,&gestaltAnswer);
		myBit = gestaltStereoCapability;
		Stereo = BitTst(&gestaltAnswer,31-myBit);
		myBit = gestaltStereoMixing;
		StereoMixing = BitTst(&gestaltAnswer,31-myBit);
		nVers = SndSoundManagerVersion();
		if (nVers.majorRev >= 3)
			{
				NewSoundManager = true;
			}
		 else
			{
				NewSoundManager = false;
			}
		/* initialize the library */
		if (StereoOn && StereoMixing)
			{
				if (NewSoundManager)
					{
						if (RInitMOD(SMStereo,thePartition.header->Tracks) != noErr)
							{
								FatalError(FatalErrorInternalError);
								goto ExitPoint;
							}
					}
				else if (hasASC)
					{
						if (RInitMOD(ASCStereo,thePartition.header->Tracks) != noErr)
							{
								FatalError(FatalErrorInternalError);
								goto ExitPoint;
							}
					}
				else
					{
						if (RInitMOD(SMDSP,thePartition.header->Tracks) != noErr)
							{
								FatalError(FatalErrorInternalError);
								goto ExitPoint;
							}
					}
			}
		else
			{
				if (NewSoundManager)
					{
						if (RInitMOD(SMMono,thePartition.header->Tracks) != noErr)
							{
								FatalError(FatalErrorInternalError);
								goto ExitPoint;
							}
					}
				else if (hasASC)
					{
						if (RInitMOD(ASCMono,thePartition.header->Tracks) != noErr)
							{
								FatalError(FatalErrorInternalError);
								goto ExitPoint;
							}
					}
				else
					{
						FatalError(FatalErrorInternalError);
						goto ExitPoint;
					}
			}

		/* begin playing the thing */
		RPlayMOD();
		Reading = true;

		/* now we do event loop waiting for codes & finish... */
		while (!QuitPending)
			{
				if (RecalibratePlayer)
					{
						SetParameters();
					}
				HandleKeyBuffer(); /* calls WaitForEvent */
				/* is the soundtrack done? */
				if (!Reading)
					{
						QuitPending = true;
					}
			}

	 PreExitPoint:
		RStopMOD();  /* stop the music */
		RClearMOD();  /* Clear MADF Music */
		RQuitMOD();  /* Clear MADF Driver */

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
				/* StartPChannel(pc); */
			}
		 else
			{
				Pausing = true;
				/* StopPChannel(pc,false); */
				while (Pausing && !QuitPending)
					{
						HandleKeyBuffer();
					}
			}
	}
