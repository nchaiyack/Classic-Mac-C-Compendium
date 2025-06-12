/* CMyApplication.c */

#include "CMyApplication.h"
#include "MenuController.h"
#include "CMyDocument.h"
#include "CSack.h"
#include "Memory.h"
#include "Alert.h"
#include "StandardFile.h"
#include "File.h"


#define AppleMenuID (256)
#define FileMenuID (257)
#define EditMenuID (258)
#define StuffMenuID (259)
#define TrackerMenuID (260)

#define NotEnoughMemoryID (130L*65536L + 2)

#define CantOpenMultipleSongListsID (5120L*65536L + 3)

#define CantFindServer (4096L*65536L + 1)
#define CantSearchForServer (4096L*65536L + 2)
#define CantLaunchServer (4096L*65536L + 4)

#define RemoteFatalErrorTemplate (4097L*65536L)


/* */			CMyApplication::CMyApplication()
	{
		FSSpec			PrefsFile;
		short				VRefNum;
		long				DirID;
		short				Error;
		short				LocalRefNum;
		long				SizeOfAlias;
		Handle			Temp;

		Error = AEInstallEventHandler(kCoreEventClass,
			kAEApplicationDied,&MyHandleOBIT,0,False);

		Error = AEInstallEventHandler(ControlEventClass,
			ErrorEvent,&MyHandleERROR,0,False);

	}


void			CMyApplication::InitMenuBar(void)
	{
		APRINT(("+CMyApplication::InitMenuBar"));
		PostMenuToBar(AppleMenuID);
		PostMenuToBar(FileMenuID);
		PostMenuToBar(EditMenuID);
		PostMenuToBar(StuffMenuID);
		PostMenuToBar(TrackerMenuID);
		APRINT(("-CMyApplication::InitMenuBar"));
	}


void			CMyApplication::DispatchOpenDocument(FSSpec* TheFSSpec)
	{
		CMyDocument*	ADoc;
		FInfo					FileInfo;

		/* identifying document */
		FSpGetFInfo(TheFSSpec,&FileInfo);
		if (FileInfo.fdType == FILETYPE1)
			{
				/* it's one of our preferences files */
				if (ListOfDocuments->NumElements() == 0)
					{
						/* make a new one */
						if (!FirstMemCacheValid())
							{
								AlertError(NotEnoughMemoryID,NIL);
								return;
							}
						ADoc = new CMyDocument;  /* create the document */
						ADoc->DoOpenFile(TheFSSpec);  /* make it link to a file */
					}
				 else
					{
						AlertError(CantOpenMultipleSongListsID,NIL);
					}
			}
		else
			{
				/* assume it is a tracker file */
				if ((FileInfo.fdType == '\?\?\?\?') || (FileInfo.fdType == 'TEXT')
					|| (FileInfo.fdType == 'BINA') || (FileInfo.fdCreator == '\?\?\?\?'))
					{
						/* change file type */
						FileInfo.fdType = 'STrk';
						FileInfo.fdCreator = CREATORCODE;
						FSpSetFInfo(TheFSSpec,&FileInfo); /* no big loss if it fails... */
					}
				if (ListOfDocuments->NumElements() == 0)
					{
						/* no preference file, create a new one */
						DoMenuNew();
					}
				/* dispatch it to the document */
				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&ADoc);
				ADoc->AddSong(TheFSSpec);
			}
	}


/* open a file using the standard file things */
void				CMyApplication::DoMenuOpen(void)
	{
		FSSpec			FileInfo;

		if (ListOfDocuments->NumElements() == 0)
			{
				inherited::DoMenuOpen();
			}
		 else
			{
				if (!FirstMemCacheValid())
					{
						AlertError(NotEnoughMemoryID,NIL);
						return;
					}
				if (FGetFile(&FileInfo,NIL,NIL,-1))
					{
						DispatchOpenDocument(&FileInfo);
					}
			}
	}


void				CMyApplication::DoMenuNew(void)
	{
		if (ListOfDocuments->NumElements() == 0)
			{
				inherited::DoMenuNew();
			}
	}


void				CMyApplication::EnableMenuItems(void)
	{
		if (ListOfDocuments->NumElements() == 0)
			{
				MyEnableItem(mFileNew);
			}
		MyEnableItem(mFileOpen);
		MyEnableItem(mAppleAbout);
		MyEnableItem(mFileQuit);
	}


void				CMyApplication::LaunchTracker(OSType TrackerCreator)
	{
		FSSpec								Tracker;
		LaunchParamBlockRec		Launcher;
		DTPBRec								DesktopParamBlock;
		PString								NameOut;
		short									Error;
		short									DesktopRefNum;
		short									CurResFileValue;

		StackSizeTest();
		/* opening database */
		NameOut[0] = 0;
		DesktopParamBlock.ioNamePtr = NameOut;
		CurResFileValue = CurResFile();
		Error = GetVRefNum(CurResFileValue,&DesktopParamBlock.ioVRefNum);
		PBDTGetPath(&DesktopParamBlock);
		Error = DesktopParamBlock.ioResult;
		if (Error != noErr)
			{
				CMyDocument*		Document;

				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&Document);
				/* must call stop first otherwise a nasty race condition develops */
				/* because the document would still get idle events & try to call */
				/* this procedure again */
				Document->DoStop();
				AlertError(CantSearchForServer,NIL);
				return;
			}
		DesktopRefNum = DesktopParamBlock.ioDTRefNum;

		/* finding tracker program */
		DesktopParamBlock.ioCompletion = NIL;
		DesktopParamBlock.ioNamePtr = NameOut;
		DesktopParamBlock.ioDTRefNum = DesktopRefNum;
		DesktopParamBlock.ioIndex = 0;
		DesktopParamBlock.ioFileCreator = TrackerCreator;
		PBDTGetAPPLSync(&DesktopParamBlock);
		Error = DesktopParamBlock.ioResult;
		if (Error != noErr)
			{
				CMyDocument*		Document;

				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&Document);
				Document->DoStop();
				AlertError(CantFindServer,NIL);
				return;
			}
		FMakeFSSpec(0,DesktopParamBlock.ioAPPLParID,NameOut,&Tracker);

		Launcher.launchBlockID = extendedBlock;
		Launcher.launchEPBLength = extendedBlockLen;
		Launcher.launchFileFlags = 0;
		Launcher.launchControlFlags = launchNoFileFlags | launchContinue
			| launchUseMinimum | launchDontSwitch;
		Launcher.launchAppSpec = &Tracker;
		Launcher.launchAppParameters = NIL;
		Error = LaunchApplication(&Launcher);
		if (Error != noErr)
			{
				CMyDocument*		Document;

				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&Document);
				Document->DoStop();
				AlertError(CantLaunchServer,NIL);
				return;
			}
		 else
			{
				CMyDocument*		Document;

				ListOfDocuments->ResetScan();
				ListOfDocuments->GetNext(&Document);
				Document->PlayerLaunchedNotification(Launcher.launchProcessSN);
			}
	}


void			CMyApplication::SendMessage(ProcessSerialNumber PlayerSN, AppleEvent* Message)
	{
		AppleEvent			Reply;
		OSErr						Error;

		StackSizeTest();
		Error = AESend(Message,&Reply,kAENoReply,kAENormalPriority,kNoTimeOut,NIL,NIL);
	}


void			CMyApplication::KillPlayer(ProcessSerialNumber PlayerSN)
	{
		AEAddressDesc			AddressDescriptor;
		OSErr							Error;
		AppleEvent				Event;

		Error = AECreateDesc(typeProcessSerialNumber,(void*)&PlayerSN,
			sizeof(ProcessSerialNumber),&AddressDescriptor);
		Error = AECreateAppleEvent(kCoreEventClass,kAEQuitApplication,&AddressDescriptor,
			kAutoGenerateReturnID,kAnyTransactionID,&Event);
		SendMessage(PlayerSN,&Event);
		Error = AEDisposeDesc(&AddressDescriptor);
		Error = AEDisposeDesc(&Event);
	}


static	pascal	OSErr	CMyApplication::MyHandleOBIT(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		long									RemoteError;
		DescType							Stupid;
		OSErr									Error;
		long									ActualStupidSize;
		ProcessSerialNumber		PSN;

		Error = AEGetParamPtr(theAppleEvent,keyErrorNumber,typeLongInteger,
			&Stupid,(void*)&RemoteError,sizeof(long),&ActualStupidSize);

		Error = AEGetParamPtr(theAppleEvent,keyProcessSerialNumber,typeProcessSerialNumber,
			&Stupid,(void*)&PSN,sizeof(ProcessSerialNumber),&ActualStupidSize);

		Error = MyGotRequiredParams(theAppleEvent);

		if (Error != noErr)
			{
				return Error;
			}
		 else
			{
				if (Application->ListOfDocuments->NumElements() != 0)
					{
						CMyDocument*			Document;

						Application->ListOfDocuments->ResetScan();
						Application->ListOfDocuments->GetNext(&Document);
						Document->PlayerDiedNotification();
					}
				return noErr;
			}
	}


static	pascal	OSErr	CMyApplication::MyHandleERROR(AppleEvent* theAppleEvent,
												AppleEvent* reply, long handlerRefcon)
	{
		short									RemoteError;
		DescType							Stupid;
		OSErr									Error;
		long									ActualStupidSize;

		Error = AEGetParamPtr(theAppleEvent,keyErrorIDNum,typeShortInteger,
			&Stupid,(void*)&RemoteError,sizeof(short),&ActualStupidSize);

		Error = MyGotRequiredParams(theAppleEvent);

		if (Error != noErr)
			{
				return Error;
			}
		 else
			{
				CMyDocument*		Document;

				Application->ListOfDocuments->ResetScan();
				if (Application->ListOfDocuments->GetNext(&Document))
					{
						Document->DoStop();
					}
				switch (RemoteError)
					{
						case FatalErrorOutOfMemory:
						case FatalErrorInternalError:
						case FatalErrorCantOpenCompressedFiles:
						case FatalErrorCouldntOpenFile:
						case FatalErrorCouldntCloseFile:
						case FatalErrorNotASong:
						case FatalError68020NeededID:
							AlertError(RemoteFatalErrorTemplate + RemoteError,NIL);
							break;
						default:
							AlertError(RemoteFatalErrorTemplate + FatalErrorUnknown,NIL);
							break;
					}
				return noErr;
			}
	}
