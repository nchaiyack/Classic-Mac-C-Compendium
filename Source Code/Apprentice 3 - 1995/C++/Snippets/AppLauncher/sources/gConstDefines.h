#pragma once
/*		
*	gConstDefines.h
*
*	AppLauncher
*	^^^^^^^^^^^
*
*	Defined items with global scope
*	� Andrew Nemeth (where applicable), Warrimoo Australia 1995
*
*	File created:		8 Mar 95.
*	Modified:			8, 9, 14 Mar 95.
*/


//	#DEFINES
//

#define		kMoveToFront			(WindowPtr)-1L
#define		kEmptyString			"\p"
#define		kNilFilterProc			nil


//	APPLICATION SIGNATURE
//

const  OSType	kAppCreator = '????';




//	MENUs
//

//														Application Menu bar
const  short	kMenuBar_ID = 2000;
//														Indexes & Number of menus in bar
enum		{
					kAppleNdx,
					kFileNdx,
					kOptionsNdx,
					kMenusInBar  };

enum		{  kMyAppleMenu = 0,

					kM_About,
					
					kMApple_ID = 2000  };

enum		{  kMyFileMenu = 0,

					kM_Quit,
					
					kMFile_ID = 2001  };

enum		{  kMyOptionsMenu = 0,

				kBackground_Heading,
					kM_LaunchApp_Back,
					kM_LaunchDoc_Back,
					kM_OpenDocInRemoteApp_Back,
					k______3a,
				kForeground_Heading,
					kM_LaunchApp_Fore,
					kM_LaunchDoc_Fore,
					kM_OpenDocInRemoteApp_Fore,
					k______3b,
					kM_HideOtherApp,
					kM_QuitOtherApp,
					k______3c,
					kM_OpenControlPnl,
					
					kMOptions_ID = 2002  };

//	DIALOGS
//

//														'About application' ALRT
const  short	kresidALRT_About = 2000,
			kresidALRT_Death = 2001;
//														useful constants for DITLs
const  short	iOKButton = 1,
			iCancelButton = 2,
			kDisableControl = 255,
			kEnableControl = 0;
