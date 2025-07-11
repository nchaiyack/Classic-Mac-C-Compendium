/* CMyDocument.h */

#pragma once

#include "CDocument.h"
#include <Aliases.h>
#include <Processes.h>
#include <AppleEvents.h>

class CWindow;
class CArray;
class CSongList;
class CPicture;
class CStaticText;
class CCheckbox;
class CNumberEdit;

class CStopButton;
class CRewindButton;
class CFastForwardButton;
class CSkipToNextButton;
class CStaticText;
class CAutoNextSong;
class CRepeat;
class CRandomize;
class CAutoStartSongs;
class CDefaultStereoOn;
class CDefaultAntiAliasing;
class CDefaultSamplingRate;
class CDefaultStereoMix;
class CDefaultNumRepeats;
class CDefaultSpeed;
class CDefaultVolume;
class CSpecificStereoOn;
class CSpecificAntiAliasing;
class CSpecificSamplingRate;
class CSpecificStereoMix;
class CSpecificNumRepeats;
class CSpecificSpeed;
class CSpecificVolume;
class COverrideStereoOn;
class COverrideAntiAliasing;
class COverrideSamplingRate;
class COverrideStereoMix;
class COverrideNumRepeats;
class COverrideSpeed;
class COverrideVolume;
class CNumOutputBits;

#define SongNameLength (32)

typedef enum
	{
		eTrackerMarkEspie = 0,
		eTrackerFrankSeide = 1,
		eTrackerAntoineRosset = 2
	} TrackerTypes;

typedef struct
	{
		MyBoolean				SamplingRateOverrideDefault;
		ushort					SamplingRate;
		MyBoolean				StereoOnOverrideDefault;
		MyBoolean				StereoOn;
		MyBoolean				StereoMixOverrideDefault;
		short						StereoMix;
		MyBoolean				AntiAliasingOverrideDefault;
		MyBoolean				AntiAliasing;
		MyBoolean				SpeedOverrideDefault;
		short						Speed;
		MyBoolean				NumRepeatsOverrideDefault;
		short						NumRepeats;
		MyBoolean				VolumeOverrideDefault;
		short						Volume;
		TrackerTypes		Tracker;
		AliasRecord**		SongLocation;
		uchar						SongName[SongNameLength];

		MyBoolean				PlayedFlag;  /* for randomize */
	} SongRec;

/* possible player states */
enum	{PlayerLaunching, PlayerWaitingForInit, PlayerRunning, PlayerDying};

struct	CMyDocument	:	CDocument
	{
		CWindow*								RootWindow;
		FSSpec									TheFile;
		MyBoolean								EverSaved;
		MyBoolean								UpToDate;
		CArray*									ListOfSongs;
		MyBoolean								FileOpenFlag;
		short										FileRef;

		ushort									SamplingRate;
		MyBoolean								StereoOn;
		short										StereoMix;
		MyBoolean								AntiAliasing;
		short										Speed;
		short										NumRepeats;
		short										Volume;
		MyBoolean								AutoNextSong;
		MyBoolean								Repeat;
		MyBoolean								AutoStartSongs;
		MyBoolean								Randomize;
		MyBoolean								UseSixteenBitMode;

		long										Selection;

		CSongList*							SongList;
		CPicture*								PausePlayButton;
		CStopButton*						StopButton;
		CRewindButton*					RewindButton;
		CFastForwardButton*			FastForwardButton;
		CSkipToNextButton*			SkipToNextButton;
		CAutoNextSong*					AutoNextSongBox;
		CRepeat*								RepeatBox;
		CAutoStartSongs*				AutoStartSongsBox;
		CRandomize*							RandomizeBox;
		CDefaultStereoOn*				DefaultStereoOnBox;
		CDefaultAntiAliasing*		DefaultAntiAliasingBox;
		CDefaultSamplingRate*		DefaultSamplingRateBox;
		CDefaultStereoMix*			DefaultStereoMixBox;
		CDefaultNumRepeats*			DefaultNumRepeatsBox;
		CDefaultSpeed*					DefaultSpeedBox;
		CDefaultVolume*					DefaultVolumeBox;
		CSpecificStereoOn*			SpecificStereoOnBox;
		CSpecificAntiAliasing*	SpecificAntiAliasingBox;
		CSpecificSamplingRate*	SpecificSamplingRateBox;
		CSpecificStereoMix*			SpecificStereoMixBox;
		CSpecificNumRepeats*		SpecificNumRepeatsBox;
		CSpecificSpeed*					SpecificSpeedBox;
		CSpecificVolume*				SpecificVolumeBox;
		CStaticText*						SpecificSamplingRateText;
		CStaticText*						SpecificStereoMixText;
		CStaticText*						SpecificNumRepeatsText;
		CStaticText*						SpecificSpeedText;
		CStaticText*						SpecificVolumeText;
		COverrideStereoOn*			OverrideStereoOnBox;
		COverrideAntiAliasing*	OverrideAntiAliasingBox;
		COverrideSamplingRate*	OverrideSamplingRateBox;
		COverrideStereoMix*			OverrideStereoMixBox;
		COverrideNumRepeats*		OverrideNumRepeatsBox;
		COverrideSpeed*					OverrideSpeedBox;
		COverrideVolume*				OverrideVolumeBox;
		CNumOutputBits*					NumOutputBitsBox;

		MyBoolean								PlayNextWhenThisOneStops;
		long										SongToStart;
		MyBoolean								PlayerExists;
		short										PlayerState;
		long										Playing; /* index of song being played */
		ProcessSerialNumber			OurPlayer;


		/* */				CMyDocument();
		/* */				~CMyDocument();
		void				DoNewFile(void);
		MyBoolean		GoAway(void);
		void				DoPrint(void);
		void				DoOpenFile(FSSpec* TheSpec);
		MyBoolean		SaveFile(void);
		MyBoolean		SaveFileAs(void);
		void				WriteData(void);
		void				AddSong(FSSpec* TheSpec);
		void				CancelCurrentSong(void);
		void				StartThisSong(long SongIndex);
		void				RemoveSongFromList(long SongIndex);
		void				MakeNewWindow(void);
		void				DoIdle(long Stupid);
		void				SetNewSelection(long NewSelection);
		void				MoveSong(long SongIndex, long NewIndex);

		void				SetStereoOn(MyBoolean NewStereoOn);
		void				SetAntiAliasing(MyBoolean NewAntiAliasing);
		void				SetSamplingRate(ulong NewSamplingRate);
		void				SetStereoMix(short NewStereoMix);
		void				SetNumRepeats(short NewNumRepeats);
		void				SetSpeed(short NewSpeed);
		void				SetVolume(short NewVolume);
		void				SetOverrideStereoOn(MyBoolean Flag);
		void				SetOverrideAntiAliasing(MyBoolean Flag);
		void				SetOverrideSamplingRate(MyBoolean Flag);
		void				SetOverrideStereoMix(MyBoolean Flag);
		void				SetOverrideNumRepeats(MyBoolean Flag);
		void				SetOverrideSpeed(MyBoolean Flag);
		void				SetOverrideVolume(MyBoolean Flag);
		void				SetDefaultStereoOn(MyBoolean NewStereoOn);
		void				SetDefaultAntiAliasing(MyBoolean NewAntiAliasing);
		void				SetDefaultSamplingRate(ulong NewSamplingRate);
		void				SetDefaultStereoMix(short NewStereoMix);
		void				SetDefaultNumRepeats(short NewNumRepeats);
		void				SetDefaultSpeed(short NewSpeed);
		void				SetDefaultVolume(short NewVolume);
		void				SetNumOutputBits(MyBoolean SixteenBitMode);
		void				SetTrackerServerToUse(TrackerTypes WhatKind);
		TrackerTypes	FindOutTrackerServerToUse(void);

		void				SetAutoNextSong(MyBoolean NewAutoNextSong);
		void				SetRepeat(MyBoolean NewRepeat);
		void				SetAutoStartSongs(MyBoolean NewAutoStartSongs);
		void				SetRandomize(MyBoolean NewRandomize);
		void				ResetRandomPlayList(void);
		void				StartRandomSong(void);

		void				SendMessage(short TheMessage, DescType TheKeyWord);

		void				DoRewind(MyBoolean Flag);
		void				DoStop(void);
		void				DoPlay(void);
		void				DoPause(void);
		void				DoFastForward(MyBoolean Flag);
		void				DoSkipToNext(void);
		void				DoVolumeUp(void);
		void				DoVolumeDown(void);

		void				PlayerDiedNotification(void);
		void				PlayerLaunchedNotification(ProcessSerialNumber TheProcNum);
	};


/* menu options */
#define mDeleteSelection (0x1000)
#define mPlaySelection (0x1001)
#define mStopPlaying (0x1002)
#define mIncreaseVolume (0x1003)
#define mDecreaseVolume (0x1004)
#define mShowSelection (0x1005)
#define mShowPlaying (0x1006)
#define mUseEspieTracker (0x1007)
#define mUseSeideTracker (0x1008)
#define mUseRossetTracker (0x1009)
