#include <ShutDown.h>

#include "Global.h"
#include "Utilities.h"			// To see if option key is down
#include "Pack3.h"				// To install Pack3 patch
#include "Munge Dialog.h"		// To install GetNewDialog, ShowWindow,
								// NewControl, DisposeControl patches
#include "GetItemList.h"		// To install SetDItem and Pack0 patches
#include "MenuSelect.h"			// To install MenuSelect patch
#include "Filing.h"				// To install ShutDown procedure and read old prefs
#include "InitGraf.h"			// To install InitGraf patch
#include "SpinningCursor.h"		// For call to InitCursorCtl
#include "MiniFinder.h"			// For InstallMiniFinder
#include "AEPatch.h"			// To install MyAppleEvent
#include "Remember.h"			// To install MyMountVol and MyUnmountVol

#include <Traps.h>

FSSpec*				gMe;	

//--------------------------------------------------------------------------------
/*
	Contains the entry point for the whole INIT. When the system
	loads us in at boot time, this procedure is called. Because this
	stuff is run just once at boot time, we want it to be as small as
	possible. Therefore, as much of the initialization is performed in
	external routines (stored in PROC resources). These routines are
	loaded in and executed by the code below.
	
	However, there�s a bunch of stuff that couldn�t be farmed out
	to the external routines, and all of that is performed below. The
	first thing we do is drop into assembly and do some register setup.
	THINK C allows us to access our so-called global variable off of
	the A4 register, where A4 shouldpoint to the beginning of the
	code resource. Normally, we could call the THINK C standard
	functions RememberA0() and SetUpA4() to do this for us. However,
	doing this requires us to include the standard code resource header,
	which we�ve turned off to save some space. This means that it�s up
	to us to setup A4 properly.
	
	In the assembly, we also take care of a little PC housekeeping, ala
	Technote #245. Our INIT patches some traps. In TN #254, it says that
	we should call SetTrapAddress with a 32-bit clean pointer. Normally,
	we�d generate this pointer by using some &SomeFunction statement, which
	will generate some PC-relative code. However, if we are running in 24-bit
	mode, the PC of code stored in a resource may have some dirty bits in the
	upper byte. This means that using &SomeFunction will generate a dirty
	pointer (since it uses the PC, which is itself dirty). To fix the whole
	problem, we clean up the PC first thing.
	
	Next, because we are going to allocate a lot of memory (especially
	when we read in the Preference file), we switch over to the System
	zone. We load in the code resource holding the external initialization
	code and execute it. If everything went OK, it returns noErr. Otherwise,
	it returns some error code, and we don�t install ourselves.
	
	Next, we allocate the empty lists that will hold all our files, folders,
	and volumes. If that fails, we stop right there.
	
	If we successfully allocated the lists, we�ve past the point where
	installation can fail, and we proceed to do the final installation.
	First, we detach the resource we are in so that when the system closes
	the INIT, we stay around. This INIT is built with the resSys and
	resLocked bits set, which means we get loaded low in the System
	heap. Next, we install our myriad of trap patches and our shutdown
	procedure. Finally, we read in the data in the preferences file, get
	everything we need for the spinning cursor, and install our MiniFinder
	stuff via the CEToolbox.
	
	After all that is done, we reach some code that is executed regardless
	of how things went. First, we set the zone back the way we found it.
	Next, we show an icon indicating whether we successfully installed
	or not. Finally, if we didn�t install, we set up a notification procedure
	that will tell the user _why_ we didn�t install.
	
	With a little more work, more of the stuff below could be put into
	the external initialization segment. Maybe I�ll look into that later.
	
*/

void main(void)
{
	typedef short	(*ExternInitProc)(FSSpecPtr prefsFile, FSSpecPtr me);
	typedef void	(*ShowINITProc)(short iconID, short moveX);
	typedef void	(*NotificationProc)(Handle it, short errNumber);
	
	Handle			init;
	Handle			show;
	Handle			notification;
		
	Ptr				ourAddress;
	THz				currentZone;
	short			installationError;
	short			iconID;

	SetUpA4();
	asm {
		move.l	A4, ourAddress		; for the RecoverHandle, later

		lea		@continue,A0		; now, fix up the PC (TN #245)
		move.l	A0,D0
		dc.w		_StripAddress
		move.l	D0,A0
		jmp		(A0)
	@continue
	}
	
	iconID = kBadIcon;

	currentZone = GetZone();
	SetZone(SystemZone());
	
	gMe = (FSSpec*) NewPtr(sizeof(FSSpec));
	gPrefsFile = (FSSpec*) NewPtr(sizeof(FSSpec));	

	installationError = noErr;
	init = GetResource('PROC', kProcExternalInit);
	if (init != NIL)
		installationError = ((ExternInitProc) *init)(gPrefsFile, gMe);
	else
		installationError = kErrNoExternInit;

	if (!installationError) {
		gDirectoryList = CreateEmptyList();
		gFileList = CreateEmptyList();
		gVolumeList = CreateEmptyList();
		gReboundList = CreateEmptyList();
		if (!gDirectoryList || !gFileList || !gVolumeList || !gReboundList)
			installationError = kMemFullErr;
	}

	if (!installationError) {
	
		DetachResource(RecoverHandle(ourAddress));
		
		gOldStandardFile = GetToolTrapAddress(_Pack3);
		SetToolTrapAddress((UniversalProcPtr) MyPack3, _Pack3);

		gOldGetNewDialog = GetToolTrapAddress(_GetNewDialog);
		SetToolTrapAddress((UniversalProcPtr) MyGetNewDialog, _GetNewDialog);

		gOldShowWindow = GetToolTrapAddress(_ShowWindow);
		SetToolTrapAddress((UniversalProcPtr) MyShowWindow, _ShowWindow);

		gOldSetDItem = GetToolTrapAddress(_SetDItem);
		SetToolTrapAddress((UniversalProcPtr) MySetDItem, _SetDItem);

		gOldListManager = GetToolTrapAddress(_Pack0);
		SetToolTrapAddress((UniversalProcPtr) MyListManager, _Pack0);

		gOldMenuSelect = GetToolTrapAddress(_MenuSelect);
		SetToolTrapAddress((UniversalProcPtr) MyMenuSelect, _MenuSelect);

		gOldNewControl = GetToolTrapAddress(_NewControl);
		SetToolTrapAddress((UniversalProcPtr) MyNewControl, _NewControl);

		gOldDisposeControl = GetToolTrapAddress(_DisposControl);
		SetToolTrapAddress((UniversalProcPtr) MyDisposeControl, _DisposControl);
		
		gOldInitGraf = GetToolTrapAddress(_InitGraf);
		SetToolTrapAddress((UniversalProcPtr) MyInitGraf, _InitGraf);

		gOldAppleEvent = GetToolTrapAddress(_Pack8);
		SetToolTrapAddress((UniversalProcPtr) MyAppleEvent, _Pack8);

		gOldMountVol = GetOSTrapAddress(_MountVol);
		SetOSTrapAddress((UniversalProcPtr) MyMountVol, _MountVol);

		gOldUnmountVol = GetOSTrapAddress(_UnmountVol);
		SetOSTrapAddress((UniversalProcPtr) MyUnmountVol, _UnmountVol);

		ShutDwnInstall((UniversalProcPtr) SaveOnShutDown, sdOnDrivers);
		
		ReadPrefsFile();
		InitCursorCtl(kACURResource);
		InstallMiniFinder();

		iconID = kOKIcon;
	}

	SetZone(currentZone);

	show = GetResource('PROC', kProcShowINIT);
	if (show != NIL)
		((ShowINITProc) *show)(iconID, -1);

	if (installationError != noErr) {
		notification = GetResource('PROC', kProcNotification);
		if (notification != NIL)
			((NotificationProc) *notification)(notification, installationError);
	}

	RestoreA4();
}
