/*
	DMenusTmpl.c
	
	Template for a simple Dragon sub-species with an added menu
	
	Please note that a dragon based on DMenusTmpl.c and MenusTmpl.π.rsrc will use a preferences file even though,
		technically, it doesn't need one (since the user won't have any way of changing any preferences).  I did it this
		way for safety's sake, given the possibility that one might later add preferences to a dragon based on it.  To
		prevent it from using a preferences file, put a null string in the 'STR ' 128 resource in MenusTmpl.π.rsrc
	
	To base a simple dragon (let's call it "Fafnir") on the code in this file —
	
	1.	Make a copy of this file and rename it "DFafnir.c"
	2.	Make a copy of the file "Template.π" and name it "Fafnir.π"
	3.	Add DFafnir.c to the Fafnir.π project
	4.	Change your project "creator" value to your dragon's signature
	5.	Make a copy of the file "MenusTmpl.π.rsrc" and name it "Fafnir.π.rsrc"
	6.	Change all instances in this file of the string "MenusTmpl" to "Fafnir"
	7.	Move Fafnir.π.rsrc up so it's in the same folder as Fafnir.π
	8.	Make the following changes in Fafnir.π.rsrc —
		¶ Use ResEdit 2.1's 'BNDL' editor to change your dragon's signature and add an 'FREF' resource for each document
			type the user can drag-and-drop on your dragon ('****' is all files, 'fold' is folders, and 'disk' is volumes)
		¶ Use ResEdit's icon family editor to change your dragon's icon — a boring icon is provided for you to fiddle with
		¶ Change the values in 'DrPr' 128 (resolveAliases, dirDepthLimit, etc.), if you wish — do NOT delete this resource
		¶ Edit the 'TEXT'/'styl' 128 resources — they provide the Finder's help text for your dragon's icon (or remove these
			two resources, as well as 'hfdr' –5696, if you'd rather not have this feature)
		¶ Change the value in 'MoMa' 128 if your dragon needs to call MoreMasters when it starts up
		¶ Edit the two 'vers' resources
		¶ Change the string in 'STR ' 128 to the name of your dragon's preferences file
	9.	Rewrite the method DFafnir::ProcessFile (or ProcessDirectory, or both) so your dragon actually does something
	10.	Add other code as necessary

	Created	01 Oct 1992	Framework with Options menu
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

class DMenusTmpl: public Dragon {
	
	protected:
		MenuHandle		optionsMenu;			// Delete this instance variable if your dragon doesn't add any menus

	public:
						DMenusTmpl (void);
		virtual void		ProcessFile (void);
		virtual void		ProcessDirectory (void);
	
	protected:
		virtual void		SetUpMenus (void);
		virtual void		DoMenu (long menuItemCode);
		virtual void		DoOptionsMenu (short itemNum);
		virtual void		AdjustMenusBusy (void);
		virtual void		AdjustMenusIdle (void);
};

Dragon *CreateGDragon (void)
{
	// This is a function, not a method.  It's declared in Dragon.h and is NOT defined in Dragon.c
	
	return (Dragon *) new DMenusTmpl;
}

DMenusTmpl::DMenusTmpl (void)
{
	// You should initialize any added instance variables here — this method will be called whenever an object of
	//	this class is created, immediately after Dragon::Dragon
	
	optionsMenu = NULL;
	autoQuit = FALSE;		// This will be overridden by whatever value for autoQuit is specified in the 'DrPr' 128 resource in
						//	your project resource file — so just removing this line won't necessarily be enough to make
						//	it auto-quit.  The file "MenusTmpl.π.rsrc" has a 'DrPr' 128 that sets autoQuit = FALSE
}

void DMenusTmpl::ProcessFile (void)
{
	// This is the heart of your dragon.  You can open the file, change its file attributes, or whatever …
	// Simple changes can be simplified by using the macros defined in Dragon.h (curDocIsFile, curFileType, etc.)
	
	// Note that this method will not be called if you open a preferences file created by this dragon — that's
	//	all taken care of for you by Dragon (see ProcessOwnedFile)
}

void DMenusTmpl::ProcessDirectory (void)
{
	// Use this method to process folders and disks instead of just ignoring them or looking inside them for files
	// If you don't use this method, you can just delete it (and its declaration above) — I put it here merely for convenience
}

void DMenusTmpl::SetUpMenus (void)
{
	// Delete this method if your dragon doesn't add any menus
	
	inherited::SetUpMenus ();			// Add the Apple, File, and Edit menus

	optionsMenu = GetMenu (mOptions);
	InsertMenu (optionsMenu, 0);
	
	DrawMenuBar ();
}

void DMenusTmpl::DoMenu (long menuItemCode)
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

void DMenusTmpl::DoOptionsMenu (short itemNum)
{
	// Delete this method if your dragon doesn't add any menus
	
	switch (itemNum) {
		// Put your case statements here…
		default:
			break;
	}
}

void DMenusTmpl::AdjustMenusBusy (void)
{
	// Delete this method if your dragon doesn't add any menus
	
	inherited::AdjustMenusBusy ();
	DisableItem (optionsMenu, 0);	// Disable the entire Options menu
}

void DMenusTmpl::AdjustMenusIdle (void)
{
	// Delete this method if your dragon doesn't add any menus
	
	inherited::AdjustMenusIdle ();
	EnableItem (optionsMenu, 0);		// Enable what we disabled in AdjustMenusBusy — this will NOT enable items that
								//	were disabled to begin with
}

