/* CMyDocument.c */

#define compilingmydocument_c

#include "CMyDocument.h"
#include "MenuController.h"
#include "CSaveBeforeClosing.h"
#include "Memory.h"
#include "CApplication.h"
#include "File.h"
#include "CSack.h"
#include "Alert.h"
#include "CArray.h"
#include "PortableFile.h"
#include "Error.h"
#include "StandardFile.h"
#include <Sound.h>
#include "CSongList.h"
#include "CRewindButton.h"
#include "CFastForwardButton.h"
#include "CStopButton.h"
#include "CPlayButton.h"
#include "CPauseButton.h"
#include "CSkipToNextButton.h"
#include "LocationConstants.h"
#include "CStaticText.h"
#include "StringUtils.h"
#include "CCheckbox.h"
#include "CNumberEdit.h"
#include "CDefaultStereoOn.h"
#include "CDefaultAntiAlias.h"
#include "CDefaultSamplingRate.h"
#include "CDefaultStereoMix.h"
#include "CDefaultNumRepeats.h"
#include "CDefaultSpeed.h"
#include "CSpecificStereoOn.h"
#include "CSpecificAntiAlias.h"
#include "CSpecificSamplingRate.h"
#include "CSpecificStereoMix.h"
#include "CSpecificNumRepeats.h"
#include "CSpecificSpeed.h"
#include "COverrideStereoOn.h"
#include "COverrideAntiAlias.h"
#include "COverrideSamplingRate.h"
#include "COverrideStereoMix.h"
#include "COverrideNumRepeats.h"
#include "COverrideSpeed.h"
#include "CDividerLine.h"
#include "CMyApplication.h"
#include "CSpecificVolume.h"
#include "CDefaultVolume.h"
#include "COverrideVolume.h"
#include "CAutoNextSong.h"
#include "CRepeat.h"
#include "CAutoStartSongs.h"
#include "CRandomize.h"
#include "CNumOutputBits.h"
#include "Compatibility.h"


#define UntitledStringID (6592L*65536L + 1)
#define SeparatorID (6592L*65536L + 2)
#define EVENTDELAY (15)

#define CouldntSaveSongID (5120L*65536L + 1)
#define ErrorOccurredWhileLoadingID (5120L*65536L + 2)

#define MAXRANDOMTRIESBEFOREGIVINGUP (50)


/* */				CMyDocument::CMyDocument()
	{
		CArray*				SongTemp;
		Handle				Temp;

		SongTemp = new CArray;
		SongTemp->IArray(sizeof(SongRec),16);
		ListOfSongs = SongTemp;

		RootWindow = NIL;
		EverSaved = False;
		UpToDate = True;
		FileOpenFlag = False;

		SamplingRate = 22254;
		StereoOn = False;
		StereoMix = 0;
		AntiAliasing = True;
		Speed = 50;
		NumRepeats = 1;
		Volume = 48;
		AutoNextSong = True;
		Repeat = False;
		AutoStartSongs = True;
		Randomize = False;
		UseSixteenBitMode = False;

		Selection = -1;

		SongToStart = -1;
		PlayerExists = False;
		PlayNextWhenThisOneStops = False;
		Playing = -1;

		GetDateTime((void*)&randSeed);

		Temp = GetCString(UntitledStringID);
		Handle2PString(Temp,TheFile.name);
		ReleaseHandle(Temp);
	}


/* */				CMyDocument::~CMyDocument()
	{
		long				Scan;
		SongRec			Thang;

		DeregisterIdler(this);

		if (RootWindow != NIL)
			{
				delete RootWindow;
			}

		for (Scan = 0; Scan < ListOfSongs->GetNumElements(); Scan += 1)
			{
				ListOfSongs->GetElement(Scan,&Thang);
				ReleaseHandle((Handle)Thang.SongLocation);
			}
		delete ListOfSongs;

		if (FileOpenFlag)
			{
				FCloseFile(FileRef);
			}
	}


void				CMyDocument::DoNewFile(void)
	{
		MakeNewWindow();
	}


MyBoolean		CMyDocument::GoAway(void)
	{
		if (!UpToDate && EverSaved)
			{
				CSaveBeforeClosingWindow*		Saver;
				MyBoolean										Result;

				Saver = new CSaveBeforeClosingWindow;
				HLock((Handle)this);
				Result = Saver->SaveBeforeClosing(TheFile.name);
				HUnlock((Handle)this);
				switch (Result)
					{
						case Yes_Save:
							if (!SaveFile())
								{
									return False;
								}
							break;
						case No_Save:
							break;
						case Cancel_Close:
							return False;
					}
			}

		if (PlayerExists)
			{
				Application->KillPlayer(OurPlayer);
			}

		delete this;
		return True;
	}


void				CMyDocument::DoPrint(void)
	{
	}


void				CMyDocument::DoOpenFile(FSSpec* TheSpec)
	{
		short					LocalRef;
		ulong					SongCount;

		/* reading in all them alias records */
		TheFile = *TheSpec;
		if (FOpenFile(TheSpec,&LocalRef) != noErr)
			{
				/* error */
			}
		FileOpenFlag = True;
		FileRef = LocalRef;

		ResetErrorCheck();
		SamplingRate = (ushort)RShort(LocalRef);
		StereoOn = RChar(LocalRef);
		StereoMix = RShort(LocalRef);
		AntiAliasing = RChar(LocalRef);
		Speed = RShort(LocalRef);
		NumRepeats = RShort(LocalRef);
		Volume = RShort(LocalRef);
		AutoNextSong = RChar(LocalRef);
		Repeat = RChar(LocalRef);
		AutoStartSongs = RChar(LocalRef);
		Randomize = RChar(LocalRef);

		SongCount = RShort(LocalRef);
		while (SongCount > 0)
			{
				ulong						AliasLength;
				SongRec					DaAlias;
				long						NewEntry;

				DaAlias.SamplingRateOverrideDefault = RChar(LocalRef);
				DaAlias.SamplingRate = (ushort)RShort(LocalRef);
				DaAlias.StereoOnOverrideDefault = RChar(LocalRef);
				DaAlias.StereoOn = RShort(LocalRef);
				DaAlias.StereoMixOverrideDefault = RChar(LocalRef);
				DaAlias.StereoMix = RShort(LocalRef);
				DaAlias.AntiAliasingOverrideDefault = RChar(LocalRef);
				DaAlias.AntiAliasing = RShort(LocalRef);
				DaAlias.SpeedOverrideDefault = RChar(LocalRef);
				DaAlias.Speed = RShort(LocalRef);
				DaAlias.NumRepeatsOverrideDefault = RChar(LocalRef);
				DaAlias.NumRepeats = RShort(LocalRef);
				DaAlias.VolumeOverrideDefault = RChar(LocalRef);
				DaAlias.Volume = RShort(LocalRef);
				DaAlias.Tracker = RChar(LocalRef);
				FReadBlock(LocalRef,(char*)DaAlias.SongName,SongNameLength);
				AliasLength = RLong(LocalRef);
				DaAlias.SongLocation = (AliasRecord**)AllocHandle(AliasLength);
				HLock((Handle)DaAlias.SongLocation);
				FReadBlock(LocalRef,(char*)*DaAlias.SongLocation,AliasLength);
				HUnlock((Handle)DaAlias.SongLocation);
				DaAlias.PlayedFlag = False;
				NewEntry = ListOfSongs->AppendElement();
				ListOfSongs->PutElement(NewEntry,&DaAlias);
				SongCount -= 1;
			}
		if (ErrorOccurred())
			{
				AlertError(ErrorOccurredWhileLoadingID,NIL);
			}
		MakeNewWindow();
		SongList->RecalculateScrollBars();
		if ((LastModifiers & controlKey) == 0)
			{
				/* if they were NOT holding down the control key, then we want */
				/* to see if they are now.  Otherwise, we don't want to clear it. */
				RelinquishCPU();
			}
		if (AutoStartSongs && ((LastModifiers & controlKey) == 0))
			{
				/* if they want to autostart songs upon opening the document AND */
				/* they didn't hold down the command key while selecting "open" */
				/* (or starting the program) then we can do it. */
				if (Randomize)
					{
						StartRandomSong();
					}
				 else
					{
						StartThisSong(0);
					}
			}
		EverSaved = True;
	}


MyBoolean		CMyDocument::SaveFile(void)
	{
		if (EverSaved)
			{
				WriteData();
				return True;
			}
		 else
			{
				return SaveFileAs();
			}
	}


MyBoolean		CMyDocument::SaveFileAs(void)
	{
		FSSpec			NewInfo;
		MyBoolean		Result;
		MyBoolean		Replacing;

		HLock((Handle)this);
		Result = FPutFile(TheFile.name,&NewInfo,&Replacing);
		if (Result)
			{
				if (Replacing)
					{
						FDeleteFile(&NewInfo);
					}
				TheFile = NewInfo;
				FCreate(&NewInfo,CREATORCODE,FILETYPE1);
				FOpenFile(&NewInfo,&FileRef);
				FileOpenFlag = True;
				WriteData();
				EverSaved = True;
				SetWTitle(RootWindow->MyGrafPtr,TheFile.name);
			}
		HUnlock((Handle)this);
		return Result;
	}


void				CMyDocument::WriteData(void)
	{
		ulong					SongCount;
		ulong					SongScan;
		long					FilePosTemp;

		ERROR(!FileOpenFlag,PRERR(ForceAbort,"CMyDocument::WriteData called when "
			"no file was open for writing."));

		SpecificSamplingRateBox->StoreValue();
		SpecificStereoMixBox->StoreValue();
		SpecificNumRepeatsBox->StoreValue();
		SpecificSpeedBox->StoreValue();
		SpecificVolumeBox->StoreValue();
		DefaultSamplingRateBox->StoreValue();
		DefaultStereoMixBox->StoreValue();
		DefaultNumRepeatsBox->StoreValue();
		DefaultSpeedBox->StoreValue();
		DefaultVolumeBox->StoreValue();

		ResetErrorCheck();
		FSetFilePos(FileRef,0);

		WShort(FileRef,SamplingRate);
		WChar(FileRef,StereoOn);
		WShort(FileRef,StereoMix);
		WChar(FileRef,AntiAliasing);
		WShort(FileRef,Speed);
		WShort(FileRef,NumRepeats);
		WShort(FileRef,Volume);
		WChar(FileRef,AutoNextSong);
		WChar(FileRef,Repeat);
		WChar(FileRef,AutoStartSongs);
		WChar(FileRef,Randomize);

		SongCount = ListOfSongs->GetNumElements();
		WShort(FileRef,SongCount);
		for (SongScan = 0; SongScan < SongCount; SongScan += 1)
			{
				SongRec			Song;
				ulong				AliasSize;

				ListOfSongs->GetElement(SongScan,&Song);
				WChar(FileRef,Song.SamplingRateOverrideDefault);
				WShort(FileRef,Song.SamplingRate);
				WChar(FileRef,Song.StereoOnOverrideDefault);
				WShort(FileRef,Song.StereoOn);
				WChar(FileRef,Song.StereoMixOverrideDefault);
				WShort(FileRef,Song.StereoMix);
				WChar(FileRef,Song.AntiAliasingOverrideDefault);
				WShort(FileRef,Song.AntiAliasing);
				WChar(FileRef,Song.SpeedOverrideDefault);
				WShort(FileRef,Song.Speed);
				WChar(FileRef,Song.NumRepeatsOverrideDefault);
				WShort(FileRef,Song.NumRepeats);
				WChar(FileRef,Song.VolumeOverrideDefault);
				WShort(FileRef,Song.Volume);
				WChar(FileRef,Song.Tracker);
				FWriteBlock(FileRef,(char*)Song.SongName,SongNameLength);
				AliasSize = HandleSize((Handle)Song.SongLocation);
				WLong(FileRef,AliasSize);
				HLock((Handle)Song.SongLocation);
				FWriteBlock(FileRef,(char*)*Song.SongLocation,AliasSize);
				HUnlock((Handle)Song.SongLocation);
			}
		FGetFilePos(FileRef,&FilePosTemp);
		FSetEOF(FileRef,FilePosTemp);
		if (ErrorOccurred())
			{
				AlertError(CouldntSaveSongID,NIL);
			}
		 else
			{
				UpToDate = True;
			}
	}


void				CMyDocument::AddSong(FSSpec* TheSpec)
	{
		AliasRecord**			SystemAlias;
		SongRec						MyAlias;
		long							IndexToStoreAt;
		OSErr							Error;
		short							Scan;

		for (Scan = 0; Scan < sizeof(MyAlias); Scan += 1)
			{
				((char*)&MyAlias)[Scan] = 0;
			}
		MyAlias.SamplingRateOverrideDefault = False;
		MyAlias.StereoOnOverrideDefault = False;
		MyAlias.StereoMixOverrideDefault = False;
		MyAlias.AntiAliasingOverrideDefault = False;
		MyAlias.SpeedOverrideDefault = False;
		MyAlias.NumRepeatsOverrideDefault = False;
		MyAlias.VolumeOverrideDefault = False;
		MyAlias.SamplingRate = SamplingRate;
		MyAlias.StereoOn = StereoOn;
		MyAlias.StereoMix = StereoMix;
		MyAlias.AntiAliasing = AntiAliasing;
		MyAlias.Speed = Speed;
		MyAlias.NumRepeats = NumRepeats;
		MyAlias.Volume = Volume;
		MyAlias.Tracker = False;
		MemCpy((char*)MyAlias.SongName,(char*)TheSpec->name,TheSpec->name[0] + 1);
		if (MyAlias.SongName[0] > SongNameLength - 1)
			{
				MyAlias.SongName[0] = SongNameLength - 1;
			}
		Error = NewAlias(NIL,TheSpec,&SystemAlias);
		MyAlias.SongLocation = (AliasRecord**)DupSysHandle((Handle)SystemAlias);
		DisposHandle((Handle)SystemAlias);
		MyAlias.PlayedFlag = False;
		IndexToStoreAt = ListOfSongs->AppendElement();
		ListOfSongs->PutElement(IndexToStoreAt,&MyAlias);
		EXECUTE(ListOfSongs->GetElement(IndexToStoreAt,&MyAlias);)
		SongList->Redraw(IndexToStoreAt,IndexToStoreAt);
		SongList->RecalculateScrollBars();
		if ((LastModifiers & controlKey) == 0)
			{
				/* if they were NOT holding down the control key, then we want */
				/* to see if they are now.  Otherwise, we don't want to clear it. */
				RelinquishCPU();
			}
		if ((Playing == -1) && (SongToStart == -1) && AutoStartSongs
			&& ((LastModifiers & controlKey) == 0))
			{
				StartThisSong(IndexToStoreAt);
			}
		UpToDate = False;
	}


void				CMyDocument::CancelCurrentSong(void)
	{
		if (PlayerExists)
			{
				Application->KillPlayer(OurPlayer);
				PlayerState = PlayerDying;
			}
		PlayNextWhenThisOneStops = False;
	}


void				CMyDocument::StartThisSong(long SongIndex)
	{
		SongRec					Temp;

		if (Playing != -1)
			{
				CancelCurrentSong();
			}
		if ((SongIndex >= 0) && (SongIndex < ListOfSongs->GetNumElements()))
			{
				RegisterIdler(this,EVENTDELAY);
				SongToStart = SongIndex;
				ListOfSongs->GetElement(SongIndex,&Temp);
				Temp.PlayedFlag = True;
				ListOfSongs->PutElement(SongIndex,&Temp);
			}
		PlayNextWhenThisOneStops = AutoNextSong;
	}


void				CMyDocument::RemoveSongFromList(long SongIndex)
	{
		SongRec				Temp;

		if ((SongIndex >= 0) && (SongIndex < ListOfSongs->GetNumElements()))
			{
				if (SongIndex == Playing)
					{
						CancelCurrentSong();
					}
				if (SongIndex < Playing)
					{
						Playing -= 1;
					}
				UpToDate = False;
				ListOfSongs->GetElement(SongIndex,&Temp);
				ReleaseHandle((Handle)Temp.SongLocation);
				ListOfSongs->DeleteElement(SongIndex);
				SongList->Redraw(SongIndex,ListOfSongs->GetNumElements());
			}
	}


void				CMyDocument::MakeNewWindow(void)
	{
		CWindow*						Window;
		LongPoint						Start;
		LongPoint						Extent;
		CStaticText*				StaticText;
		CPlayButton*				PlayButton;
		CDividerLine*				DividerLine;


		MoveHHi((Handle)this);
		HLock((Handle)this);


		Window = new CWindow;
		GetRect(WindowLocsID,&Start,&Extent);
		Start = CenterRect(Extent,MainScreenSize());
		Window->IWindow(Start,Extent,ModelessWindow,NoGrowable,NoZoomable);
		RootWindow = Window;
		SetWTitle(Window->MyGrafPtr,TheFile.name);


		SongList = new CSongList;
		SongList->ISongList(this,Window);


		RewindButton = new CRewindButton;
		RewindButton->IRewindButton(this,Window);

		StopButton = new CStopButton;
		StopButton->IStopButton(this,Window);

		PlayButton = new CPlayButton;
		PlayButton->IPlayButton(this,Window);
		PausePlayButton = PlayButton;

		FastForwardButton = new CFastForwardButton;
		FastForwardButton->IFastForwardButton(this,Window);

		SkipToNextButton = new CSkipToNextButton;
		SkipToNextButton->ISkipToNextButton(this,Window);


		StaticText = new CStaticText;
		GetRect(DefaultSettingsID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultSettingsID),
			applFont,9,Window,Window,JustifyLeft);

		StaticText = new CStaticText;
		GetRect(DefaultFrequencyID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultFrequencyID),
			applFont,9,Window,Window,JustifyLeft);

		StaticText = new CStaticText;
		GetRect(DefaultSpeedID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultSpeedID),
			applFont,9,Window,Window,JustifyLeft);

		StaticText = new CStaticText;
		GetRect(DefaultRepeatsID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultRepeatsID),
			applFont,9,Window,Window,JustifyLeft);

		StaticText = new CStaticText;
		GetRect(DefaultMixID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultMixID),
			applFont,9,Window,Window,JustifyLeft);

		StaticText = new CStaticText;
		GetRect(DefaultVolumeID,&Start,&Extent);
		StaticText->IStaticText(Start,Extent,GetCString(DefaultVolumeID),
			applFont,9,Window,Window,JustifyLeft);

		SpecificSamplingRateText = new CStaticText;
		GetRect(SpecificFrequencyID,&Start,&Extent);
		SpecificSamplingRateText->IStaticText(Start,Extent,GetCString(SpecificFrequencyID),
			applFont,9,Window,Window,JustifyLeft);

		SpecificSpeedText = new CStaticText;
		GetRect(SpecificSpeedID,&Start,&Extent);
		SpecificSpeedText->IStaticText(Start,Extent,GetCString(SpecificSpeedID),
			applFont,9,Window,Window,JustifyLeft);

		SpecificNumRepeatsText = new CStaticText;
		GetRect(SpecificRepeatsID,&Start,&Extent);
		SpecificNumRepeatsText->IStaticText(Start,Extent,GetCString(SpecificRepeatsID),
			applFont,9,Window,Window,JustifyLeft);

		SpecificStereoMixText = new CStaticText;
		GetRect(SpecificMixID,&Start,&Extent);
		SpecificStereoMixText->IStaticText(Start,Extent,GetCString(SpecificMixID),
			applFont,9,Window,Window,JustifyLeft);

		SpecificVolumeText = new CStaticText;
		GetRect(SpecificVolumeID,&Start,&Extent);
		SpecificVolumeText->IStaticText(Start,Extent,GetCString(SpecificVolumeID),
			applFont,9,Window,Window,JustifyLeft);

		AutoNextSongBox = new CAutoNextSong;
		AutoNextSongBox->IAutoNextSong(this,Window);

		RepeatBox = new CRepeat;
		RepeatBox->IRepeat(this,Window);

		AutoStartSongsBox = new CAutoStartSongs;
		AutoStartSongsBox->IAutoStartSongs(this,Window);

		RandomizeBox = new CRandomize;
		RandomizeBox->IRandomize(this,Window);

		DividerLine = new CDividerLine;
		DividerLine->IDividerLine(Window);


		DefaultStereoOnBox = new CDefaultStereoOn;
		DefaultStereoOnBox->IDefaultStereoOn(this,Window);

		DefaultAntiAliasingBox = new CDefaultAntiAliasing;
		DefaultAntiAliasingBox->IDefaultAntiAliasing(this,Window);

		DefaultVolumeBox = new CDefaultVolume;
		DefaultVolumeBox->IDefaultVolume(this,Window);
		DefaultVolumeBox->SetValue(Volume);

		DefaultSamplingRateBox = new CDefaultSamplingRate;
		DefaultSamplingRateBox->IDefaultSamplingRate(this,Window);
		DefaultSamplingRateBox->SetValue(SamplingRate);

		DefaultSpeedBox = new CDefaultSpeed;
		DefaultSpeedBox->IDefaultSpeed(this,Window);
		DefaultSpeedBox->SetValue(Speed);

		DefaultNumRepeatsBox = new CDefaultNumRepeats;
		DefaultNumRepeatsBox->IDefaultNumRepeats(this,Window);
		DefaultNumRepeatsBox->SetValue(NumRepeats);

		DefaultStereoMixBox = new CDefaultStereoMix;
		DefaultStereoMixBox->IDefaultStereoMix(this,Window);
		DefaultStereoMixBox->SetValue(StereoMix);

		NumOutputBitsBox = new CNumOutputBits;
		NumOutputBitsBox->INumOutputBits(this,Window);


		SpecificStereoOnBox = new CSpecificStereoOn;
		SpecificStereoOnBox->ISpecificStereoOn(this,Window);

		SpecificAntiAliasingBox = new CSpecificAntiAliasing;
		SpecificAntiAliasingBox->ISpecificAntiAliasing(this,Window);

		SpecificVolumeBox = new CSpecificVolume;
		SpecificVolumeBox->ISpecificVolume(this,Window);

		SpecificSamplingRateBox = new CSpecificSamplingRate;
		SpecificSamplingRateBox->ISpecificSamplingRate(this,Window);

		SpecificSpeedBox = new CSpecificSpeed;
		SpecificSpeedBox->ISpecificSpeed(this,Window);

		SpecificNumRepeatsBox = new CSpecificNumRepeats;
		SpecificNumRepeatsBox->ISpecificNumRepeats(this,Window);

		SpecificStereoMixBox = new CSpecificStereoMix;
		SpecificStereoMixBox->ISpecificStereoMix(this,Window);


		OverrideStereoOnBox = new COverrideStereoOn;
		OverrideStereoOnBox->IOverrideStereoOn(this,Window);

		OverrideAntiAliasingBox = new COverrideAntiAliasing;
		OverrideAntiAliasingBox->IOverrideAntiAliasing(this,Window);

		OverrideSamplingRateBox = new COverrideSamplingRate;
		OverrideSamplingRateBox->IOverrideSamplingRate(this,Window);

		OverrideStereoMixBox = new COverrideStereoMix;
		OverrideStereoMixBox->IOverrideStereoMix(this,Window);

		OverrideNumRepeatsBox = new COverrideNumRepeats;
		OverrideNumRepeatsBox->IOverrideNumRepeats(this,Window);

		OverrideSpeedBox = new COverrideSpeed;
		OverrideSpeedBox->IOverrideSpeed(this,Window);

		OverrideVolumeBox = new COverrideVolume;
		OverrideVolumeBox->IOverrideVolume(this,Window);


		SpecificStereoOnBox->DoDisable();
		SpecificAntiAliasingBox->DoDisable();
		SpecificSamplingRateBox->DoDisable();
		SpecificStereoMixBox->DoDisable();
		SpecificNumRepeatsBox->DoDisable();
		SpecificSpeedBox->DoDisable();
		SpecificVolumeBox->DoDisable();
		SpecificSamplingRateText->DoDisable();
		SpecificStereoMixText->DoDisable();
		SpecificNumRepeatsText->DoDisable();
		SpecificSpeedText->DoDisable();
		SpecificVolumeText->DoDisable();

		OverrideStereoOnBox->DoDisable();
		OverrideAntiAliasingBox->DoDisable();
		OverrideSamplingRateBox->DoDisable();
		OverrideStereoMixBox->DoDisable();
		OverrideNumRepeatsBox->DoDisable();
		OverrideSpeedBox->DoDisable();
		OverrideVolumeBox->DoDisable();

		if (StereoOn)
			{
				DefaultStereoOnBox->DoThang();
			}

		if (AntiAliasing)
			{
				DefaultAntiAliasingBox->DoThang();
			}

		if (AutoNextSong)
			{
				AutoNextSongBox->DoThang();
			}

		if (Repeat)
			{
				RepeatBox->DoThang();
			}

		if (AutoStartSongs)
			{
				AutoStartSongsBox->DoThang();
			}

		if (Randomize)
			{
				RandomizeBox->DoThang();
			}

		RewindButton->DoDisable();
		StopButton->DoDisable();
		PausePlayButton->DoEnable();
		FastForwardButton->DoDisable();
		SkipToNextButton->DoDisable();


		HUnlock((Handle)this);
	}


void				CMyDocument::SetStereoOn(MyBoolean NewStereoOn)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.StereoOn = NewStereoOn;
		ListOfSongs->PutElement(Selection,&Temp);
		if (SpecificStereoOnBox->State != NewStereoOn)
			{
				SpecificStereoOnBox->DoThang();
			}
		if (Selection == Playing)
			{
				SendMessage((short)NewStereoOn,keyStereoOn);
			}
	}


void				CMyDocument::SetNumOutputBits(MyBoolean SixteenBitMode)
	{
		UseSixteenBitMode = SixteenBitMode;
		if (NumOutputBitsBox->State != SixteenBitMode)
			{
				NumOutputBitsBox->DoThang();
			}
	}


void				CMyDocument::SetAntiAliasing(MyBoolean NewAntiAliasing)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.AntiAliasing = NewAntiAliasing;
		ListOfSongs->PutElement(Selection,&Temp);
		if (SpecificAntiAliasingBox->State != NewAntiAliasing)
			{
				SpecificAntiAliasingBox->DoThang();
			}
		if (Selection == Playing)
			{
				SendMessage((short)NewAntiAliasing,keyAntiAliasing);
			}
	}


void				CMyDocument::SetSamplingRate(ulong NewSamplingRate)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.SamplingRate = NewSamplingRate;
		ListOfSongs->PutElement(Selection,&Temp);
		SpecificSamplingRateBox->SetValue(NewSamplingRate);
		if (Selection == Playing)
			{
				SendMessage((short)NewSamplingRate,keySamplingRate);
			}
	}


void				CMyDocument::SetStereoMix(short NewStereoMix)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.StereoMix = NewStereoMix;
		ListOfSongs->PutElement(Selection,&Temp);
		SpecificStereoMixBox->SetValue(NewStereoMix);
		if (Selection == Playing)
			{
				SendMessage((short)NewStereoMix,keyStereoMix);
			}
	}


void				CMyDocument::SetNumRepeats(short NewNumRepeats)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.NumRepeats = NewNumRepeats;
		ListOfSongs->PutElement(Selection,&Temp);
		SpecificNumRepeatsBox->SetValue(NewNumRepeats);
		if (Selection == Playing)
			{
				SendMessage((short)NewNumRepeats,keyNumRepeats);
			}
	}


void				CMyDocument::SetSpeed(short NewSpeed)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.Speed = NewSpeed;
		ListOfSongs->PutElement(Selection,&Temp);
		SpecificSpeedBox->SetValue(NewSpeed);
		if (Selection == Playing)
			{
				SendMessage((short)NewSpeed,keySpeed);
			}
	}


void				CMyDocument::SetVolume(short NewVolume)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.Volume = NewVolume;
		ListOfSongs->PutElement(Selection,&Temp);
		SpecificVolumeBox->SetValue(NewVolume);
		if (Selection == Playing)
			{
				SendMessage((short)NewVolume,keyLoudness);
			}
	}


void				CMyDocument::SetOverrideStereoOn(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.StereoOnOverrideDefault = Flag;
		if (Flag)
			{
				Temp.StereoOn = StereoOn;
				SpecificStereoOnBox->DoEnable();
				if (SpecificStereoOnBox->State != Temp.StereoOn)
					{
						SpecificStereoOnBox->DoThang();
					}
			}
		 else
			{
				SpecificStereoOnBox->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)StereoOn,keyStereoOn);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideAntiAliasing(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.AntiAliasingOverrideDefault = Flag;
		if (Flag)
			{
				Temp.AntiAliasing = AntiAliasing;
				SpecificAntiAliasingBox->DoEnable();
				if (SpecificAntiAliasingBox->State != Temp.AntiAliasing)
					{
						SpecificAntiAliasingBox->DoThang();
					}
			}
		 else
			{
				SpecificAntiAliasingBox->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)AntiAliasing,keyAntiAliasing);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideSamplingRate(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.SamplingRateOverrideDefault = Flag;
		if (Flag)
			{
				Temp.SamplingRate = SamplingRate;
				SpecificSamplingRateBox->DoEnable();
				SpecificSamplingRateText->DoEnable();
				if (SpecificSamplingRateBox->GetValue() != Temp.SamplingRate)
					{
						SpecificSamplingRateBox->SetValue(Temp.SamplingRate);
					}
			}
		 else
			{
				SpecificSamplingRateBox->DoDisable();
				SpecificSamplingRateText->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)SamplingRate,keySamplingRate);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideStereoMix(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.StereoMixOverrideDefault = Flag;
		if (Flag)
			{
				Temp.StereoMix = StereoMix;
				SpecificStereoMixBox->DoEnable();
				SpecificStereoMixText->DoEnable();
				if (SpecificStereoMixBox->GetValue() != Temp.StereoMix)
					{
						SpecificStereoMixBox->SetValue(Temp.StereoMix);
					}
			}
		 else
			{
				SpecificStereoMixBox->DoDisable();
				SpecificStereoMixText->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)StereoMix,keyStereoMix);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideNumRepeats(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.NumRepeatsOverrideDefault = Flag;
		if (Flag)
			{
				Temp.NumRepeats = NumRepeats;
				SpecificNumRepeatsBox->DoEnable();
				SpecificNumRepeatsText->DoEnable();
				if (SpecificNumRepeatsBox->GetValue() != Temp.NumRepeats)
					{
						SpecificNumRepeatsBox->SetValue(Temp.NumRepeats);
					}
			}
		 else
			{
				SpecificNumRepeatsBox->DoDisable();
				SpecificNumRepeatsText->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)NumRepeats,keyNumRepeats);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideSpeed(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.SpeedOverrideDefault = Flag;
		if (Flag)
			{
				Temp.Speed = Speed;
				SpecificSpeedBox->DoEnable();
				SpecificSpeedText->DoEnable();
				if (SpecificSpeedBox->GetValue() != Temp.Speed)
					{
						SpecificSpeedBox->SetValue(Temp.Speed);
					}
			}
		 else
			{
				SpecificSpeedBox->DoDisable();
				SpecificSpeedText->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)Speed,keySpeed);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetOverrideVolume(MyBoolean Flag)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.VolumeOverrideDefault = Flag;
		if (Flag)
			{
				Temp.Volume = Volume;
				SpecificVolumeBox->DoEnable();
				SpecificVolumeText->DoEnable();
				if (SpecificVolumeBox->GetValue() != Temp.Volume)
					{
						SpecificVolumeBox->SetValue(Temp.Volume);
					}
			}
		 else
			{
				SpecificVolumeBox->DoDisable();
				SpecificVolumeText->DoDisable();
				if (Selection == Playing)
					{
						SendMessage((short)Volume,keyLoudness);
					}
			}
		ListOfSongs->PutElement(Selection,&Temp);
	}


void				CMyDocument::SetDefaultStereoOn(MyBoolean NewStereoOn)
	{
		StereoOn = NewStereoOn;
		if (DefaultStereoOnBox->State != NewStereoOn)
			{
				DefaultStereoOnBox->DoThang();
			}
		if (!OverrideStereoOnBox->State)
			{
				if (SpecificStereoOnBox->State != NewStereoOn)
					{
						SpecificStereoOnBox->DoThang();
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewStereoOn,keyStereoOn);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultAntiAliasing(MyBoolean NewAntiAliasing)
	{
		AntiAliasing = NewAntiAliasing;
		if (DefaultAntiAliasingBox->State != NewAntiAliasing)
			{
				DefaultAntiAliasingBox->DoThang();
			}
		if (!OverrideAntiAliasingBox->State)
			{
				if (SpecificAntiAliasingBox->State != NewAntiAliasing)
					{
						SpecificAntiAliasingBox->DoThang();
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewAntiAliasing,keyAntiAliasing);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultSamplingRate(ulong NewSamplingRate)
	{
		SamplingRate = NewSamplingRate;
		if (DefaultSamplingRateBox->GetValue() != NewSamplingRate)
			{
				DefaultSamplingRateBox->SetValue(NewSamplingRate);
			}
		if (!OverrideSamplingRateBox->State)
			{
				if (SpecificSamplingRateBox->GetValue() != NewSamplingRate)
					{
						SpecificSamplingRateBox->SetValue(NewSamplingRate);
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewSamplingRate,keySamplingRate);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultStereoMix(short NewStereoMix)
	{
		StereoMix = NewStereoMix;
		if (DefaultStereoMixBox->GetValue() != NewStereoMix)
			{
				DefaultStereoMixBox->SetValue(NewStereoMix);
			}
		if (!OverrideStereoMixBox->State)
			{
				if (SpecificStereoMixBox->GetValue() != NewStereoMix)
					{
						SpecificStereoMixBox->SetValue(NewStereoMix);
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewStereoMix,keyStereoMix);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultNumRepeats(short NewNumRepeats)
	{
		NumRepeats = NewNumRepeats;
		if (DefaultNumRepeatsBox->GetValue() != NewNumRepeats)
			{
				DefaultNumRepeatsBox->SetValue(NewNumRepeats);
			}
		if (!OverrideNumRepeatsBox->State)
			{
				if (SpecificNumRepeatsBox->GetValue() != NewNumRepeats)
					{
						SpecificNumRepeatsBox->SetValue(NewNumRepeats);
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewNumRepeats,keyNumRepeats);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultSpeed(short NewSpeed)
	{
		Speed = NewSpeed;
		if (DefaultSpeedBox->GetValue() != NewSpeed)
			{
				DefaultSpeedBox->SetValue(NewSpeed);
			}
		if (!OverrideSpeedBox->State)
			{
				if (SpecificSpeedBox->GetValue() != NewSpeed)
					{
						SpecificSpeedBox->SetValue(NewSpeed);
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewSpeed,keySpeed);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetDefaultVolume(short NewVolume)
	{
		Volume = NewVolume;
		if (DefaultVolumeBox->GetValue() != NewVolume)
			{
				DefaultVolumeBox->SetValue(NewVolume);
			}
		if (!OverrideVolumeBox->State)
			{
				if (SpecificVolumeBox->GetValue() != NewVolume)
					{
						SpecificVolumeBox->SetValue(NewVolume);
					}
				if (Selection == Playing)
					{
						SendMessage((short)NewVolume,keyLoudness);
					}
			}
		UpToDate = False;
	}


void				CMyDocument::SetTrackerServerToUse(TrackerTypes WhatKind)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return;
			}

		UpToDate = False;
		ListOfSongs->GetElement(Selection,&Temp);
		Temp.Tracker = WhatKind;
		ListOfSongs->PutElement(Selection,&Temp);
	}


TrackerTypes	CMyDocument::FindOutTrackerServerToUse(void)
	{
		SongRec				Temp;

		if (Selection == -1)
			{
				return eTrackerMarkEspie; /* default.  this should be a fatal error */
			}
		ListOfSongs->GetElement(Selection,&Temp);
		return Temp.Tracker;
	}


void				CMyDocument::SetAutoNextSong(MyBoolean NewAutoNextSong)
	{
		AutoNextSong = NewAutoNextSong;
		if (Playing != -1)
			{
				PlayNextWhenThisOneStops = NewAutoNextSong;
			}
		if (AutoNextSongBox->State != NewAutoNextSong)
			{
				AutoNextSongBox->DoThang();
			}
		UpToDate = False;
	}


void				CMyDocument::SetAutoStartSongs(MyBoolean NewAutoStartSongs)
	{
		AutoStartSongs = NewAutoStartSongs;
		if (AutoStartSongsBox->State != NewAutoStartSongs)
			{
				AutoStartSongsBox->DoThang();
			}
		UpToDate = False;
	}


void				CMyDocument::SetRandomize(MyBoolean NewRandomize)
	{
		Randomize = NewRandomize;
		if (RandomizeBox->State != NewRandomize)
			{
				RandomizeBox->DoThang();
			}
		UpToDate = False;
	}


void				CMyDocument::SetRepeat(MyBoolean NewRepeat)
	{
		Repeat = NewRepeat;
		if (RepeatBox->State != NewRepeat)
			{
				RepeatBox->DoThang();
			}
		UpToDate = False;
	}


void				CMyDocument::DoRewind(MyBoolean Flag)
	{
		short				SpiffParam;

		if ((Playing != -1) && PlayerExists)
			{
				if (Flag)
					{
						SpiffParam = '<';
					}
				 else
					{
						SpiffParam = '|';
					}
				SendMessage(SpiffParam,keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoStop(void)
	{
		if (Playing != -1)
			{
				CancelCurrentSong();
			}
		ResetRandomPlayList();
		DeregisterIdler(this);
	}


void				CMyDocument::DoPlay(void)
	{
		SpecificSamplingRateBox->StoreValue();
		SpecificStereoMixBox->StoreValue();
		SpecificNumRepeatsBox->StoreValue();
		SpecificSpeedBox->StoreValue();
		SpecificVolumeBox->StoreValue();
		DefaultSamplingRateBox->StoreValue();
		DefaultStereoMixBox->StoreValue();
		DefaultNumRepeatsBox->StoreValue();
		DefaultSpeedBox->StoreValue();
		DefaultVolumeBox->StoreValue();
		/* this could either be invoked while playing but paused, or when nothing */
		/* is playing at all. */
		if (Playing == -1)
			{
				/* nothing playing; start selected song */
				if (Selection != -1)
					{
						StartThisSong(Selection);
					}
				 else
					{
						if (Randomize)
							{
								StartRandomSong();
							}
						 else
							{
								StartThisSong(0);
							}
					}
			}
		 else
			{
				CPauseButton*			PauseButton;

				/* paused, resume playing */
				if (PausePlayButton != NIL)
					{
						delete PausePlayButton;
					}
				PauseButton = new CPauseButton;
				PausePlayButton = PauseButton;
				PauseButton->IPauseButton(this,RootWindow);
				if (!RootWindow->Suspended)
					{
						PauseButton->DoResume();
					}
				PauseButton->DoUpdate();

				/* sending unpause event */
				SendMessage(' ',keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoPause(void)
	{
		if ((Playing != -1) && PlayerExists)
			{
				CPlayButton*			PlayButton;

				/* playing, pause song */
				if (PausePlayButton != NIL)
					{
						delete PausePlayButton;
					}
				PlayButton = new CPlayButton;
				PausePlayButton = PlayButton;
				PlayButton->IPlayButton(this,RootWindow);
				if (!RootWindow->Suspended)
					{
						PlayButton->DoResume();
					}
				PlayButton->DoUpdate();

				/* sending pause event */
				SendMessage(' ',keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoFastForward(MyBoolean Flag)
	{
		short				SpiffParam;

		if ((Playing != -1) && PlayerExists)
			{
				if (Flag)
					{
						SpiffParam = '>';
					}
				 else
					{
						SpiffParam = '|';
					}
				SendMessage(SpiffParam,keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoSkipToNext(void)
	{
		if (Playing != -1)
			{
				if (Randomize)
					{
						StartRandomSong();
					}
				 else
					{
						if (Playing < ListOfSongs->GetNumElements() - 1)
							{
								StartThisSong(Playing + 1);
							}
						 else
							{
								StartThisSong(0);
							}
					}
			}
	}


void				CMyDocument::DoVolumeUp(void)
	{
		if ((Playing != -1) && PlayerExists)
			{
				SendMessage('+',keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoVolumeDown(void)
	{
		if ((Playing != -1) && PlayerExists)
			{
				SendMessage('-',keyKeyPressCharacter);
			}
	}


void				CMyDocument::DoIdle(long Stupid)
	{
		CPauseButton*					PauseButton;
		short									Error;
		AppleEvent						Event;
		AEAddressDesc					AddressDescriptor;
		AEDesc								FileList;
		SongRec								TheSong;
		AEDesc								AEFileDescriptor;
		short									ShortInt;
		long									LongInt;
		Boolean								BooleanValue;
		AEDesc								TempDesc;
		AEDesc								ListElement;
		long									OldPlayer;
		ProcessSerialNumber		OurPSN;
		PString								NewWindowName;
		Handle								String1,String2,String3;

		StackSizeTest();
		if ((!PlayerExists) && (SongToStart != -1))
			{
				PlayerState = PlayerLaunching;
				ListOfSongs->GetElement(SongToStart,&TheSong);
				switch (TheSong.Tracker)
					{
						case eTrackerMarkEspie:
							if (Has020orBetter)
								{
									Application->LaunchTracker(TRACKERSERVER68020CREATOR);
								}
							 else
								{
									Application->LaunchTracker(TRACKERSERVER68000CREATOR);
								}
							break;
						case eTrackerFrankSeide:
							Application->LaunchTracker(FRANKSEIDECREATOR);
							break;
						case eTrackerAntoineRosset:
							Application->LaunchTracker(ROSSETCREATOR);
							break;
					}
			}
		if (PlayerExists && (PlayerState == PlayerWaitingForInit))
			{
				short					BitTemp;


				DeregisterIdler(this);

				PlayerState = PlayerRunning;
				OldPlayer = Playing;
				Playing = SongToStart;
				SongToStart = -1;
				SongList->Redraw(OldPlayer,OldPlayer);
				SongList->Redraw(Playing,Playing);

				ListOfSongs->GetElement(Playing,&TheSong);
				CheckHeap();

				BeginStringOperation();
				String1 = PString2Handle(TheFile.name);
				RegisterString(String1);
				String2 = GetCString(SeparatorID);
				RegisterString(String2);
				String3 = PString2Handle(TheSong.SongName);
				RegisterString(String3);
				String1 = ConStr(String1,String2);
				String1 = ConStr(String1,String3);
				EndStringOperation(String1);
				Handle2PString(String1,NewWindowName);
				ReleaseHandle(String1);
				SetWTitle(RootWindow->MyGrafPtr,NewWindowName);

				/* constructing nasty parameter block & sending open event to tracker */
				HLock((Handle)this);

				Error = AECreateDesc(typeProcessSerialNumber,(void*)&OurPlayer,
					sizeof(ProcessSerialNumber),&AddressDescriptor);

				Error = AECreateAppleEvent(kCoreEventClass,kAEOpenDocuments,&AddressDescriptor,
					kAutoGenerateReturnID,kAnyTransactionID,&Event);
				CheckHeap();

				Error = AECreateList(NIL,0,False,&FileList);
				HLock((Handle)TheSong.SongLocation);
				AECreateDesc(typeAlias,(void*)*TheSong.SongLocation,
					HandleSize((Handle)TheSong.SongLocation),&ListElement);
				HUnlock((Handle)TheSong.SongLocation);
				Error = AEPutDesc(&FileList,0,&ListElement);
				Error = AEDisposeDesc(&ListElement);
				Error = AEPutParamDesc(&Event,keyDirectObject,&FileList);
				Error = AEDisposeDesc(&FileList);

				if (TheSong.AntiAliasingOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.AntiAliasing,
							sizeof(MyBoolean),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&AntiAliasing,
							sizeof(MyBoolean),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keyAntiAliasing,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.StereoOnOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.StereoOn,
							sizeof(MyBoolean),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&StereoOn,
							sizeof(MyBoolean),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keyStereoOn,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.SamplingRateOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.SamplingRate,
							sizeof(ushort),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&SamplingRate,
							sizeof(ushort),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keySamplingRate,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.NumRepeatsOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.NumRepeats,
							sizeof(short),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&NumRepeats,
							sizeof(short),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keyNumRepeats,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.SpeedOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.Speed,
							sizeof(short),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&Speed,
							sizeof(short),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keySpeed,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.StereoMixOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.StereoMix,
							sizeof(short),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&StereoMix,
							sizeof(short),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keyStereoMix,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (TheSong.VolumeOverrideDefault)
					{
						Error = AECreateDesc(typeShortInteger,(void*)&TheSong.Volume,
							sizeof(short),&TempDesc);
					}
				 else
					{
						Error = AECreateDesc(typeShortInteger,(void*)&Volume,
							sizeof(short),&TempDesc);
					}
				Error = AEPutParamDesc(&Event,keyLoudness,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				if (UseSixteenBitMode)
					{
						BitTemp = 16;
					}
				 else
					{
						BitTemp = 8;
					}
				Error = AECreateDesc(typeShortInteger,(void*)&BitTemp,
					sizeof(short),&TempDesc);
				Error = AEPutParamDesc(&Event,keyNumBits,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				Error = GetCurrentProcess(&OurPSN);
				Error = AECreateDesc(typeProcessSerialNumber,(void*)&OurPSN,
					sizeof(ProcessSerialNumber),&TempDesc);
				Error = AEPutParamDesc(&Event,keySenderPSN,&TempDesc);
				Error = AEDisposeDesc(&TempDesc);

				Application->SendMessage(OurPlayer,&Event);
				Error = AEDisposeDesc(&AddressDescriptor);
				Error = AEDisposeDesc(&Event);
				HUnlock((Handle)this);

				if (PausePlayButton != NIL)
					{
						delete PausePlayButton;
					}
				PauseButton = new CPauseButton;
				PausePlayButton = PauseButton;
				PauseButton->IPauseButton(this,RootWindow);
				if (!RootWindow->Suspended)
					{
						PauseButton->DoResume();
					}
				PauseButton->DoUpdate();

				StopButton->DoEnable();
				RewindButton->DoEnable();
				FastForwardButton->DoEnable();
				SkipToNextButton->DoEnable();
			}
	}


void				CMyDocument::PlayerDiedNotification(void)
	{
		CPlayButton*		PlayButton;
		long						OldPlayer;

		PlayerExists = False;
		OldPlayer = Playing;
		Playing = -1;
		HLock((Handle)this);
		SetWTitle(RootWindow->MyGrafPtr,TheFile.name);
		HUnlock((Handle)this);
		SongList->Redraw(OldPlayer,OldPlayer);
		if (PlayNextWhenThisOneStops)
			{
				if (SongToStart == -1)
					{
						/* we want to hear the "next" song, but one was not specified. */
						if (Randomize)
							{
								StartRandomSong();
							}
						 else
							{
								if (OldPlayer < ListOfSongs->GetNumElements() - 1)
									{
										StartThisSong(OldPlayer + 1);
									}
								 else
									{
										if (Repeat)
											{
												StartThisSong(0);
											}
										 else
											{
												PlayNextWhenThisOneStops = False;
											}
									}
							}
					}
				 else
					{
						/* they DID specify which song to listen to next */
						StartThisSong(SongToStart);
					}
			}

		delete PausePlayButton;
		PlayButton = new CPlayButton;
		PlayButton->IPlayButton(this,RootWindow);
		PausePlayButton = PlayButton;
		if (!RootWindow->Suspended)
			{
				PlayButton->DoResume();
			}
		PlayButton->DoUpdate();

		StopButton->DoDisable();
		RewindButton->DoDisable();
		FastForwardButton->DoDisable();
		SkipToNextButton->DoDisable();
	}


void				CMyDocument::PlayerLaunchedNotification(ProcessSerialNumber TheProcNum)
	{
		StackSizeTest();
		PlayerExists = True;
		PlayerState = PlayerWaitingForInit;
		OurPlayer = TheProcNum;
	}


void				CMyDocument::SetNewSelection(long NewSelection)
	{
		SpecificSamplingRateBox->StoreValue();
		SpecificStereoMixBox->StoreValue();
		SpecificNumRepeatsBox->StoreValue();
		SpecificSpeedBox->StoreValue();
		SpecificVolumeBox->StoreValue();
		DefaultSamplingRateBox->StoreValue();
		DefaultStereoMixBox->StoreValue();
		DefaultNumRepeatsBox->StoreValue();
		DefaultSpeedBox->StoreValue();
		DefaultVolumeBox->StoreValue();
		if (Selection != -1)
			{
				long			SelectionTemp;

				SelectionTemp = Selection;
				Selection = -1;
				SongList->Redraw(SelectionTemp,SelectionTemp);
			}
		Selection = NewSelection;
		if (NewSelection == -1)
			{
				CStaticText*		StaticText;
				LongPoint				Start,Extent;

				SpecificStereoOnBox->DoDisable();
				SpecificAntiAliasingBox->DoDisable();
				SpecificSamplingRateBox->DoDisable();
				SpecificStereoMixBox->DoDisable();
				SpecificNumRepeatsBox->DoDisable();
				SpecificSpeedBox->DoDisable();
				SpecificVolumeBox->DoDisable();
				SpecificSamplingRateText->DoDisable();
				SpecificStereoMixText->DoDisable();
				SpecificNumRepeatsText->DoDisable();
				SpecificSpeedText->DoDisable();
				SpecificVolumeText->DoDisable();
				OverrideStereoOnBox->DoDisable();
				OverrideAntiAliasingBox->DoDisable();
				OverrideSamplingRateBox->DoDisable();
				OverrideStereoMixBox->DoDisable();
				OverrideNumRepeatsBox->DoDisable();
				OverrideSpeedBox->DoDisable();
				OverrideVolumeBox->DoDisable();
			}
		 else
			{
				LongPoint			Start,Extent;
				Handle				String1;
				Handle				String2;
				Handle				String3;
				Handle				String4;
				SongRec				TheSong;
				CPauseButton*	PauseButton;
				CStaticText*	StaticText;

				StackSizeTest();
				SongList->Redraw(NewSelection,NewSelection);
				ListOfSongs->GetElement(NewSelection,&TheSong);

				OverrideStereoOnBox->DoEnable();
				if (OverrideStereoOnBox->State != TheSong.StereoOnOverrideDefault)
					{
						OverrideStereoOnBox->DoThang();
					}
				if (TheSong.StereoOnOverrideDefault)
					{
						SpecificStereoOnBox->DoEnable();
						if (SpecificStereoOnBox->State != TheSong.StereoOn)
							{
								SpecificStereoOnBox->DoThang();
							}
					}
				 else
					{
						SpecificStereoOnBox->DoDisable();
					}

				OverrideAntiAliasingBox->DoEnable();
				if (OverrideAntiAliasingBox->State != TheSong.AntiAliasingOverrideDefault)
					{
						OverrideAntiAliasingBox->DoThang();
					}
				if (TheSong.AntiAliasingOverrideDefault)
					{
						SpecificAntiAliasingBox->DoEnable();
						if (SpecificAntiAliasingBox->State != TheSong.AntiAliasing)
							{
								SpecificAntiAliasingBox->DoThang();
							}
					}
				 else
					{
						SpecificAntiAliasingBox->DoDisable();
					}

				OverrideSamplingRateBox->DoEnable();
				if (OverrideSamplingRateBox->State != TheSong.SamplingRateOverrideDefault)
					{
						OverrideSamplingRateBox->DoThang();
					}
				if (TheSong.SamplingRateOverrideDefault)
					{
						SpecificSamplingRateBox->DoEnable();
						SpecificSamplingRateText->DoEnable();
						SpecificSamplingRateBox->SetValue(TheSong.SamplingRate);
					}
				 else
					{
						SpecificSamplingRateBox->DoDisable();
						SpecificSamplingRateText->DoDisable();
					}

				OverrideStereoMixBox->DoEnable();
				if (OverrideStereoMixBox->State != TheSong.StereoMixOverrideDefault)
					{
						OverrideStereoMixBox->DoThang();
					}
				if (TheSong.StereoMixOverrideDefault)
					{
						SpecificStereoMixBox->DoEnable();
						SpecificStereoMixText->DoEnable();
						SpecificStereoMixBox->SetValue(TheSong.StereoMix);
					}
				 else
					{
						SpecificStereoMixBox->DoDisable();
						SpecificStereoMixText->DoDisable();
					}

				OverrideNumRepeatsBox->DoEnable();
				if (OverrideNumRepeatsBox->State != TheSong.NumRepeatsOverrideDefault)
					{
						OverrideNumRepeatsBox->DoThang();
					}
				if (TheSong.NumRepeatsOverrideDefault)
					{
						SpecificNumRepeatsBox->DoEnable();
						SpecificNumRepeatsText->DoEnable();
						SpecificNumRepeatsBox->SetValue(TheSong.NumRepeats);
					}
				 else
					{
						SpecificNumRepeatsBox->DoDisable();
						SpecificNumRepeatsText->DoDisable();
					}

				OverrideSpeedBox->DoEnable();
				if (OverrideSpeedBox->State != TheSong.SpeedOverrideDefault)
					{
						OverrideSpeedBox->DoThang();
					}
				if (TheSong.SpeedOverrideDefault)
					{
						SpecificSpeedBox->DoEnable();
						SpecificSpeedText->DoEnable();
						SpecificSpeedBox->SetValue(TheSong.Speed);
					}
				 else
					{
						SpecificSpeedBox->DoDisable();
						SpecificSpeedText->DoDisable();
					}

				OverrideVolumeBox->DoEnable();
				if (OverrideVolumeBox->State != TheSong.VolumeOverrideDefault)
					{
						OverrideVolumeBox->DoThang();
					}
				if (TheSong.VolumeOverrideDefault)
					{
						SpecificVolumeBox->DoEnable();
						SpecificVolumeText->DoEnable();
						SpecificVolumeBox->SetValue(TheSong.Volume);
					}
				 else
					{
						SpecificVolumeBox->DoDisable();
						SpecificVolumeText->DoDisable();
					}
			}
	}


void				CMyDocument::MoveSong(long SongIndex, long NewIndex)
	{
		SongRec			MySong;
		long				OldSelection;
		long				Temp;

		UpToDate = False;
		SpecificSamplingRateBox->StoreValue();
		SpecificStereoMixBox->StoreValue();
		SpecificNumRepeatsBox->StoreValue();
		SpecificSpeedBox->StoreValue();
		SpecificVolumeBox->StoreValue();
		DefaultSamplingRateBox->StoreValue();
		DefaultStereoMixBox->StoreValue();
		DefaultNumRepeatsBox->StoreValue();
		DefaultSpeedBox->StoreValue();
		DefaultVolumeBox->StoreValue();

		OldSelection = Playing;
		Playing = -1;
		SongList->Redraw(OldSelection,OldSelection);
		Playing = OldSelection;
		if (Playing == SongIndex)
			{
				Playing = NewIndex;
				if (NewIndex > SongIndex)
					{
						Playing -= 1;
					}
			}
		 else
			{
				if (SongIndex < NewIndex)
					{
						if ((Playing > SongIndex) && (Playing < NewIndex))
							{
								Playing -= 1;
							}
					}
				 else
					{
						if ((Playing >= NewIndex) && (Playing < SongIndex))
							{
								Playing += 1;
							}
					}
			}
		SongList->Redraw(Playing,Playing);

		OldSelection = Selection;
		Selection = -1;
		SongList->Redraw(OldSelection,OldSelection);
		if ((SongIndex < 0) || (SongIndex >= ListOfSongs->GetNumElements()))
			{
				return;
			}
		if (NewIndex < 0)
			{
				NewIndex = 0;
			}
		ListOfSongs->GetElement(SongIndex,&MySong);
		ListOfSongs->DeleteElement(SongIndex);
		if (NewIndex > SongIndex)
			{
				NewIndex -= 1;
			}
		NewIndex = ListOfSongs->InsertElement(NewIndex);
		ListOfSongs->PutElement(NewIndex,&MySong);
		Selection = NewIndex;
		if (SongIndex > NewIndex)
			{
				Temp = SongIndex;
				SongIndex = NewIndex;
				NewIndex = Temp;
			}
		SongList->Redraw(SongIndex,NewIndex);
	}


void				CMyDocument::ResetRandomPlayList(void)
	{
		long				Scan;
		long				End;
		SongRec			Temp;

		End = ListOfSongs->GetNumElements();
		for (Scan = 0; Scan < End; Scan += 1)
			{
				ListOfSongs->GetElement(Scan,&Temp);
				Temp.PlayedFlag = False;
				ListOfSongs->PutElement(Scan,&Temp);
			}
	}


void				CMyDocument::StartRandomSong(void)
	{
		short				RunoutCount;
		long				Possibility;
		long				NumSongs;
		SongRec			Temp;
		long				StartingPoint;

		/* randomly look for a song that hasn't been played.  If we */
		/* can't find one then do a linear search for one.  If we fail that */
		/* then there are no more: reset and play if <repeat> is on, otherwise */
		/* just stop. */
		CancelCurrentSong();
	 TryAgainPoint:
		NumSongs = ListOfSongs->GetNumElements();
		if (NumSongs == 0)
			{
				return; /* oops, can't play anything */
			}
		RunoutCount = MAXRANDOMTRIESBEFOREGIVINGUP;
		while (RunoutCount > 0)
			{
				Possibility = ((unsigned short)Random()) % NumSongs;
				ListOfSongs->GetElement(Possibility,&Temp);
				if (!Temp.PlayedFlag)
					{
						/* found one to play */
					 FoundOnePoint:
						StartThisSong(Possibility);
						return;
					}
				RunoutCount -= 1;
			}
		/* failed random search.  Do linear search */
		StartingPoint = Possibility;
		do
			{
				Possibility += 1;
				if (Possibility >= NumSongs)
					{
						Possibility = 0;
					}
				ListOfSongs->GetElement(Possibility,&Temp);
				if (!Temp.PlayedFlag)
					{
						goto FoundOnePoint;
					}
			} while (Possibility != StartingPoint);
		/* failed that!  Reset and try again. */
		if (Repeat)
			{
				/* there is no possible way to get into an infinite loop. */
				ResetRandomPlayList();
				goto TryAgainPoint;
			}
	}


/* this sends control events to the remote player.  Note that it only */
/* supports short integer messages, but that's ok because they're all shorts */
void				CMyDocument::SendMessage(short TheMessage, DescType TheKeyWord)
	{
		short						Error;
		AppleEvent			Event;
		AEAddressDesc		AddressDescriptor;
		short						KeyPress;

		HLock((Handle)this); /* make sure the OurPlayer variable doesn't move */
		Error = AECreateDesc(typeProcessSerialNumber,(void*)&OurPlayer,
			sizeof(ProcessSerialNumber),&AddressDescriptor);
		HUnlock((Handle)this);
		Error = AECreateAppleEvent(ControlEventClass,ControlEvent,&AddressDescriptor,
			kAutoGenerateReturnID,kAnyTransactionID,&Event);
		Error = AEPutParamPtr(&Event,TheKeyWord,typeShortInteger,
			(void*)&TheMessage,sizeof(short));
		Application->SendMessage(OurPlayer,&Event);
		Error = AEDisposeDesc(&AddressDescriptor);
		Error = AEDisposeDesc(&Event);
		HUnlock((Handle)this);
	}
