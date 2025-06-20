#ifndef __KANT_EVENT_CLASS_H__
#define __KANT_EVENT_CLASS_H__

enum	/* KGP save enumeration IDs */
{
	kSaveYes					='yes ',
	kSaveNo						='no  ',
	kSaveAsk					='ask '
};

enum	/* KGP setting enumeration IDs */
{
	kSettingOn					='on  ',
	kSettingOff					='off '
};

enum	/* KGP option enumeration IDs */
{
	kOptionAlwaysResolve		='Arai',
	kOptionShowStages			='Sswr',
	kOptionListInteresting		='Liro',
	kOptionDynamicScrolling		='Ds  ',
	kOptionMenuBarIcons			='Mbi '
};

enum	/* KGP close/save result enumeration IDs */
{
	kResultSuccess				='Succ',
	kResultFailure				='Fail',
	kResultCancel				='Canc'
};

enum	/* KGP speed enumeration IDs */
{
	kSpeedNoDelay				='Zero',
	kSpeedOneTenthSecondDelay	='Tnth',
	kSpeedOneFifthSecondDelay	='Ffth',
	kSpeedOneHalfSecondDelay	='Half',
	kSpeedOneSecondDelay		='One '
};

enum	/* KGP music options enumeriation IDs */
{
	kMusicOptionAlways			='Alwa',
	kMusicOptionOnly			='Owg ',
	kMusicOptionNever			='Neve'
};

#ifdef __cplusplus
extern "C" {
#endif

extern	void KGPResolve(void);
extern	void KGPInsert(Str255 theStr);
extern	void KGPNew(void);
extern	DescType KGPUseModule(Str255 theStr, Boolean tryFullPath);
extern	void KGPGetText(Ptr *theTextPtr, unsigned short *theTextLength);
extern	DescType KGPCloseDocument(DescType saveType);
extern	Boolean IsOkayOptionTypeQQ(DescType optionType);
extern	void KGPSetOption(DescType optionType, DescType saveType);
extern	void KGPSpeed(DescType speedType);
extern	void KGPMusic(DescType musicType);
extern	DescType KGPSave(FSSpec *fsPtr, Boolean fsIsValid);
extern	void KGPRemember(void);
extern	DescType KGPRestore(void);
extern	DescType KGPSpeech(Boolean speakAll);
extern	DescType KGPSetVoice(Str255 theVoiceStr);

#ifdef __cplusplus
}
#endif

#endif
