/*/
     Project Arashi: PlayOptions.c
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, March 15, 1993, 20:00
     Created: Sunday, March 17, 1991, 0:29

     Copyright � 1991-1993, Juri Munkki
/*/

#define		MAINOPTIONS
#include	"VA.h"
#include	"PlayOptions.h"
#include	"ScreenSelect.h"
#include	<GestaltEqu.h>

DialogPtr			StartDialog;
PlayOptionsRecord	**OptionsHandle;
int					OptionsResourceRef;

enum	/*	Dialog item numbers:	*/
{	FunctionButtons,		StartButton,
							QuitButton,
	StaticMoveOptions,		RelativeMove,
							AbsoluteMove,
	StaticSensitivity,		HighSensitivity,
							MediumSensitivity,
							LowSensitivity,
							VeryLowSensitivity,
	StaticDisplayOptions, 	BlankUnusedOption,
							MonochromeOption,
	StaticRotationDirect,	RightClockwise,
							LeftClockwise,
							UpClockwise,
							DownClockwise,
	StaticSoundOptions,		DisableSound,
							NoLoudSounds,
	StaticHelpScreen,
							VerticalScreen,
							Sys607Sound,
	StaticRestartMode,		ArcadeRestart,    	/* ( mz) */
							EasyRestart,
							ShowFuseScores,		/* (mz) */
							
							MoreInfoButton,		/* mz */
							
							NoBackgroundTasks
	
						
		/*	ResEdit 2.1 is incredible!	*/
};


/*
>>	SetItemEnable is a useful little subroutine for setting
>>	the hiliting dialog items.
*/
void	SetItemEnable(dialog,num,value)
DialogPtr	dialog;
int			num;
int			value;
{
	Rect			Garbage;
	ControlHandle	ItemH;
	char			str[256];

	GetDItem(dialog,num,&Garbage,&ItemH,&Garbage);
	HiliteControl(ItemH,value);
}

/*
>>	SetItemValue is a useful little subroutine for setting
>>	and resetting dialog button values.
*/
void	SetItemValue(dialog,num,value)
DialogPtr	dialog;
int			num,value;
{
	Rect			Garbage;
	ControlHandle	ItemH;

	GetDItem(dialog,num,&Garbage,&ItemH,&Garbage);
	if(GetCtlValue(ItemH) != value)
		SetCtlValue(ItemH,value);
}
/*
>>	OpenDefaults tries to open the defaults
>>	file from the system folder or the folder
>>	that the program is located in. It returns
>>	the reference number.
*/
int		OpenDefaults()
{
	register	Handle	temp;
	register	int		theref;
	
	temp=(void *)GetString(OPTIONSFILENAMESTRING);
	HLock(temp);
	theref=OpenResFile(*temp);

	if(theref==-1) theref=0;

	HUnlock(temp);
	ReleaseResource(temp);

	return theref;
}

void	SetStartItem(num,value)
int		num,value;
{
	SetItemValue(StartDialog,num,value);
}

void	DataToRadioButtons()
{
	int		i;
	long			myFeature;
	
	
	/* Now do check for System Version 6.0.7 for sound */
	if(Gestalt(gestaltSystemVersion, &myFeature))
	{	
		ParamText("\pARASHI encountered an unrecoverable _GESTALT error.",nil,nil,nil);
		Alert(131,0);
		ExitToShell();
	}
	if (myFeature < 0x0607)
	{
		PlayOptions->sys607Sound = 0;
		SetItemEnable(StartDialog,Sys607Sound,255);
	}

	SetStartItem(RelativeMove,		!PlayOptions->absMoveFlag);
	SetStartItem(AbsoluteMove,		PlayOptions->absMoveFlag);

	SetStartItem(HighSensitivity,	PlayOptions->mouseSensitivity == 7);
	SetStartItem(MediumSensitivity,	PlayOptions->mouseSensitivity == 4);
	SetStartItem(LowSensitivity,	PlayOptions->mouseSensitivity == 3);
	SetStartItem(VeryLowSensitivity,PlayOptions->mouseSensitivity == 2);
	
	SetStartItem(RightClockwise,	PlayOptions->rotationType == 0);
	SetStartItem(LeftClockwise,		PlayOptions->rotationType == 1);
	SetStartItem(UpClockwise,		PlayOptions->rotationType == 2);
	SetStartItem(DownClockwise,		PlayOptions->rotationType == 3);

	SetStartItem(BlankUnusedOption,	PlayOptions->blankUnused);
	SetStartItem(MonochromeOption,	PlayOptions->monochrome);
	SetStartItem(VerticalScreen,	PlayOptions->verticalGame);
	SetStartItem(DisableSound,		PlayOptions->soundOff);
	SetStartItem(NoLoudSounds,		PlayOptions->noLoudSounds);
	SetStartItem(Sys607Sound,		PlayOptions->sys607Sound);
	
	SetStartItem(ArcadeRestart,		PlayOptions->restart == 0);   /* (mz) */
	SetStartItem(EasyRestart,		PlayOptions->restart == 1);
	SetStartItem(ShowFuseScores,	PlayOptions->showfscores);
	SetStartItem(NoBackgroundTasks,	PlayOptions->noBackgroundTasks);
		
	
	for(i=RightClockwise;i<=DownClockwise;i++)
	{	SetItemEnable(StartDialog,i,PlayOptions->absMoveFlag?0:255);
	}
}

int		DoStartupDialog(item)
int		item;
{
	register	int		done=NotDone;
	
	switch(item)
	{
		case RelativeMove:		PlayOptions->absMoveFlag=0;							break;
		case AbsoluteMove:		PlayOptions->absMoveFlag=1;							break;

		case HighSensitivity:	PlayOptions->mouseSensitivity=7;					break;
		case MediumSensitivity:	PlayOptions->mouseSensitivity=4;					break;
		case LowSensitivity:	PlayOptions->mouseSensitivity=3;					break;
		case VeryLowSensitivity:PlayOptions->mouseSensitivity=2;					break;

		case BlankUnusedOption:	PlayOptions->blankUnused= !PlayOptions->blankUnused;break;
		case MonochromeOption:	PlayOptions->monochrome= !PlayOptions->monochrome;	break;
		case VerticalScreen:	PlayOptions->verticalGame=!PlayOptions->verticalGame;break;
		case DisableSound:		PlayOptions->soundOff= !PlayOptions->soundOff;		break;
		case NoLoudSounds:		PlayOptions->noLoudSounds= !PlayOptions->noLoudSounds;break;
		case Sys607Sound:		PlayOptions->sys607Sound= !PlayOptions->sys607Sound;break;
		
		case RightClockwise:	PlayOptions->rotationType=0;						break;
		case LeftClockwise:		PlayOptions->rotationType=1;						break;
		case UpClockwise:		PlayOptions->rotationType=2;						break;
		case DownClockwise:		PlayOptions->rotationType=3;						break;
		
		case ArcadeRestart:		PlayOptions->restart=0;  							break;
		case EasyRestart:		PlayOptions->restart=1;								break;
		
		case ShowFuseScores:	PlayOptions->showfscores= !PlayOptions->showfscores;break;			
		case StartButton:		done=DefaultDone;									break;
		case QuitButton:		done=QuitDone;										break;
		case MoreInfoButton:	MoreInfo();											break;
		case NoBackgroundTasks:	PlayOptions->noBackgroundTasks= !PlayOptions->noBackgroundTasks;break;
	}
	
	DataToRadioButtons();
	return	done;
}

void	OptionsUpdate(ScreenAvailable)
int		ScreenAvailable;
{
	SetItemEnable(StartDialog, StartButton, ScreenAvailable ? 0 : 255);
}

void	CenterWindow(wind)
WindowPtr	wind;
{
	GDHandle	GDev;
	Rect		MainScreenRect;
	Point		TopLeft;
	GrafPtr		Saved;
	
	GDev=GetMainDevice();
	MainScreenRect=(*GDev)->gdRect;
	
	GetPort(&Saved);
	SetPort(wind);
	TopLeft.h=0;
	TopLeft.v=0;
	LocalToGlobal(&TopLeft);
	
	if(MainScreenRect.bottom > 400)
	{	TopLeft.v += 40;
	}
		
	MoveWindow(wind,(MainScreenRect.right-wind->portRect.right)/2,TopLeft.v,0);
	
	SetPort(&Saved);
}

void	CreateOptionsDialog()
{
	OptionsResourceRef=OpenDefaults();

	OptionsHandle = (PlayOptionsRecord **) GetResource('DFLT',128);
	
	if(GetHandleSize(OptionsHandle)<sizeof(PlayOptionsRecord))
		SetHandleSize(OptionsHandle,sizeof(PlayOptionsRecord));

	HLock(OptionsHandle);
	PlayOptions= *OptionsHandle;

	StartDialog=GetNewDialog(OPTIONSDIALOG,0,(void *)-1);
	CenterWindow(StartDialog);
	DataToRadioButtons();
	ShowWindow(StartDialog);
}

void	CloseOptionsDialog()
{
	DisposDialog(StartDialog);
	ChangedResource(OptionsHandle);
	WriteResource(OptionsHandle);
	if(OptionsResourceRef)
	{	DetachResource(OptionsHandle);
		CloseResFile(OptionsResourceRef);
	}
	
	VA.monochrome = PlayOptions->monochrome;
}