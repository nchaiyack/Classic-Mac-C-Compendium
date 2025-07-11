
enum
{
	iShowPreferences = 1,
	iHideOnLaunch,
	iNeither,
	iRequeue,
	iShuffle,
	iVolume,
	iCancelCurrent,
	iCancelQueue,
	iNameOfFile,
	iQueueLength
};

typedef enum soundVolume
{
	VolumeSoftest = 1,
	VolumeSofter,
	VolumeSoft,
	VolumeNormal,
	VolumeLoud,
	VolumeLouder,
	VolumeLoudest,
	numVolumes
} soundVolume;

const short *theVolumes = (short *)*Get1Resource( 'VOLs', 130);

class muZaksettings
{
	public:
		short wie_van_de_drie;
		Boolean requeue_after_playing;
		Boolean shuffle;
		soundVolume volume;
};

typedef preferences<muZaksettings> muZakPrefs;

#pragma template preferences<muZaksettings>;

typedef queue<AliasHandle> alias_queue;

#pragma template queue<AliasHandle>;

class muZakshell : public boxmaker, public muZakPrefs, public alias_queue
{
	public:
		muZakshell( Str255 prefsFileName,
			muZaksettings &defaultsettings, unsigned long queueLength);
		~muZakshell();

	protected:
		virtual void OpenDoc( Boolean opening);
		virtual void OpenApp() {};	// do not show preferences on OAPP
		virtual void HandleDialogEvent( short itemHit, DialogPtr theDialog);
		//
		// for added speed we return false from 'mayEnterFolder' when the
		// queue is full.
		//
		virtual Boolean mayEnterFolder( Boolean opening);

		virtual Boolean EventloopHook();

		void StartAMovie();
		void StopCurrentMovie( Boolean dispose_of_alias = true);
		void SetButtons();
		void SetRequeueButton();
		void SetShuffleButton();
		void SetVolumePopup();

		void updateName( Str63 theName);
		void updateNumber();
		
	private:
		Movie theMovie;
		short theMovieFile;
		AliasHandle theAlias;
};
