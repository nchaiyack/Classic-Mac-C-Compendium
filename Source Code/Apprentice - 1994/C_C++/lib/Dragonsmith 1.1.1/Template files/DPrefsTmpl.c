/*
	DPrefsTmpl.c
	
	Template for a simple Dragon sub-species with preferences settings and an added menu
	
	To base a simple dragon (let's call it "Tiamat") on the code in this file Ñ
	
	1.	Make a copy of this file and rename it "DTiamat.c"
	2.	Make a copy of the file "Template.¹" and name it "Tiamat.¹"
	3.	Add DTiamat.c to the Tiamat.¹ project
	4.	Change your project "creator" value to your dragon's signature
	5.	Make a copy of the file "PrefsTmpl.¹.rsrc" and name it "Tiamat.¹.rsrc"
	6.	Change all instances in this file of the string "PrefsTmpl" to "Tiamat"
	7.	Move Tiamat.¹.rsrc up so it's in the same folder as Tiamat.¹
	8.	Make the following changes in Tiamat.¹.rsrc Ñ
		¦ Use ResEdit 2.1's 'BNDL' editor to change your dragon's signature and add an 'FREF' resource for each document
			type the user can drag-and-drop on your dragon ('****' is all files, 'fold' is folders, and 'disk' is volumes)
		¦ Use ResEdit's icon family editor to change your dragon's icon Ñ a boring icon is provided for you to fiddle with
		¦ Change the values in 'DrPr' 128 (resolveAliases, dirDepthLimit, etc.), if you wish Ñ do NOT delete this resource
		¦ Edit the 'TEXT'/'styl' 128 resources Ñ they provide the Finder's help text for your dragon's icon (or remove these
			two resources, as well as 'hfdr' Ð5696, if you'd rather not have this feature)
		¦ Change the value in 'MoMa' 128 if your dragon needs to call MoreMasters when it starts up
		¦ Edit the two 'vers' resources
		¦ Change the string in 'STR ' 128 to the name of your dragon's preferences file
		¦ Change the second entry in the 'PrRo' 128 resource so it designates your preferences resource
	9.	Rewrite the method DTiamat::ProcessFile (or ProcessDirectory, or both) so your dragon actually does something
	10.	Add other code as necessary
	
	Preferences that have been changed can be saved by calling preferences->SavePrefResource (prefPrefsTmplPrefs)
		Ñ it's done for you at the end of DoOptionsMenu, but you can move the call if there's a more appropriate place for it
	Adding additional preferences resources is easy Ñ just follow the example of prefPrefsTmplPrefs

	WARNING:	Do NOT call ReleaseResource or DetachResource on a preferences resource.  Instead, call
				preferences->ReleasePrefResource and preferences->DetachResource.

	Created	01 Oct 1992	Framework with Options menu and ReadPrefs
	Modified	
	
*/
									
#include	"Dragon.h"

enum {
	mOptions = mEdit + 1
};

enum {
	// Put your Options menu item constants here
	iNothingAtAll = 1
};

enum {
	// Put your preferences resource index constants here Ñ the first one should == prefDragonPrefs + 1
	prefPrefsTmplPrefs = prefDragonPrefs + 1
};

class DPrefsTmpl: public Dragon {
	
	protected:
		MenuHandle		optionsMenu;			// Delete this instance variable if your dragon doesn't add any menus
		Handle			templatePrefs;		// Change this to a custom struct or array or whatever you want
		// Put instance variables for the preferences themselves here

	public:
						DPrefsTmpl (void);
		virtual void		ProcessFile (void);
		virtual void		ProcessDirectory (void);
	
	protected:
		virtual void		SetUpMenus (void);
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoOptionsMenu (short itemNum);
		virtual void		AdjustMenusBusy (void);
		virtual void		AdjustMenusIdle (void);
		virtual void		ReadPrefs (void);
};

Dragon *CreateGDragon (void)
{
	// This is a function, not a method.  It's declared in Dragon.h and is NOT defined in Dragon.c
	
	return (Dragon *) new DPrefsTmpl;
}

DPrefsTmpl::DPrefsTmpl (void)
{
	// You should initialize any added instance variables here Ñ this method will be called whenever an object of
	//	this class is created, immediately after Dragon::Dragon
	
	// Be especially careful to supply default values for all preferences here, in case the ReadPrefs method fails
	//	(in low memory circumstances, for example)
	
	optionsMenu = NULL;
	templatePrefs = NULL;
	autoQuit = FALSE;		// This will be overridden by whatever value for autoQuit is specified in the 'DrPr' 128 resource in
						//	your project resource file Ñ so just removing this line won't necessarily be enough to make
						//	it auto-quit.  The file "PrefsTmpl.¹.rsrc" has a 'DrPr' 128 that sets autoQuit = FALSE
}

void DPrefsTmpl::ProcessFile (void)
{
	// This is the heart of your dragon.  You can open the file, change its file attributes, or whatever É
	// Simple changes can be simplified by using the macros defined in Dragon.h (curDocIsFile, curFileType, etc.)
	
	// Note that this method will not be called if you open a preferences file created by this dragon Ñ that's
	//	all taken care of for you by Dragon (see ProcessOwnedFile)
}

void DPrefsTmpl::ProcessDirectory (void)
{
	// Use this method to process folders and disks instead of just ignoring them or looking inside them for files
}

void DPrefsTmpl::SetUpMenus (void)
{
	// Delete this method if your dragon doesn't add any menus (but then how will the user change the preferences settings?)
	
	inherited::SetUpMenus ();			// Add the Apple, File, and Edit menus

	optionsMenu = GetMenu (mOptions);
	InsertMenu (optionsMenu, 0);
	
	DrawMenuBar ();
}

void DPrefsTmpl::DoMenu (long menuItemCode)
{
	// Delete this method if your dragon doesn't add any menus
	
	short	menuID, itemNum;

	menuID = menuItemCode >> 16;
	itemNum = menuItemCode & 0xFFFF;

	if (menuID == mOptions)
		DoOptionsMenu (itemNum);
	else
		inherited::DoMenu (menuItemCode);
}

void DPrefsTmpl::DoOptionsMenu (short itemNum)
{
	// Delete this method if your dragon doesn't add any menus
	
	switch (itemNum) {
		// Put your case statements hereÉ
		default:
			break;
	}
	preferences->SavePrefResource (prefPrefsTmplPrefs);	// Delete this statement if you call SavePref elsewhere instead
}

void DPrefsTmpl::AdjustMenusBusy (void)
{
	// Delete this method if your dragon doesn't add any menus
	
	inherited::AdjustMenusBusy ();
	DisableItem (optionsMenu, 0);	// Disable the entire Options menu
}

void DPrefsTmpl::AdjustMenusIdle (void)
{
	// Delete this method if your dragon doesn't add any menus
	
	inherited::AdjustMenusIdle ();
	EnableItem (optionsMenu, 0);		// Enable what we disabled in AdjustMenusBusy Ñ this will NOT enable items that
								//	were disabled to begin with
}

void DPrefsTmpl::ReadPrefs (void)
{
	// Read in any needed prefs resources Ñ delete this method if your dragon doesn't add any preferences

	inherited::ReadPrefs ();
	
	// Note that you should NOT reference any preferences resource handles that may have been in memory
	//	Ñ they will not be valid handles now because the file they were from has by this time been closed
	
	templatePrefs = (Handle) preferences->GetPrefResource (prefPrefsTmplPrefs);
	if (templatePrefs != NULL) {
		// Read data from the templatePrefs resource into instance variables
	}
}

