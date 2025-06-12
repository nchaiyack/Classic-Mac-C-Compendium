/*
	DPlainTmpl.c
	
	Template for a very simple auto-quit Dragon sub-species
	
	Don't use this template file if your dragon has menus or any preferences (not counting those used by the Dragon class)
	You can get rid of the auto-quit behavior of this dragon by changing the constructor method AND the 'DrPr' 128 resource
		in your renamed copy of PlainTmpl.¹.rsrc (see below for more details)
	Please note that there is a null string in the 'STR ' 128 resource in PlainTmpl.¹.rsrc file, so a dragon based on this
		template won't find or create a preferences file Ñ why would it need one, if there's no interface for the user to use
		to change preferences settings?
	
	To base a simple dragon (let's call it "Smaug") on the code in this file Ñ
	
	1.	Make a copy of this file and rename it "DSmaug.c"
	2.	Make a copy of the file "Template.¹" and name it "Smaug.¹"
	3.	Add DSmaug.c to the Smaug.¹ project
	4.	Change your project "creator" value to your dragon's signature
	5.	Make a copy of the file "PlainTmpl.¹.rsrc" and name it "Smaug.¹.rsrc"
	6.	Change all instances in this file of the string "PrefsTmpl" to "Smaug"
	7.	Move Smaug.¹.rsrc up so it's in the same folder as Smaug.¹
	8.	Make the following changes in Smaug.¹.rsrc Ñ
		¦ Use ResEdit 2.1's 'BNDL' editor to change your dragon's signature and add an 'FREF' resource for each document
			type the user can drag-and-drop on your dragon ('****' is all files, 'fold' is folders, and 'disk' is volumes)
		¦ Use ResEdit's icon family editor to change your dragon's icon Ñ a boring icon is provided for you to fiddle with
		¦ Change the values in 'DrPr' 128 (resolveAliases, dirDepthLimit, etc.), if you wish Ñ do NOT delete this resource
		¦ Edit the 'TEXT'/'styl' 128 resources Ñ they provide the Finder's help text for your dragon's icon (or remove these
			two resources, as well as 'hfdr' Ð5696, if you'd rather not have this feature)
		¦ Change the value in 'MoMa' 128 if your dragon needs to call MoreMasters when it starts up
		¦ Edit the two 'vers' resources
	9.	Rewrite the method DSmaug::ProcessFile (or ProcessDirectory, or both) so your dragon actually does something
	10.	Add other code as necessary

	Created	28 Sep 1992	Class declaration, ProcessFile, ProcessDirectory
	Modified	15 Jan 1994	Changed default autoQuit value
	
*/
									
#include	"Dragon.h"

class DPlainTmpl: public Dragon {
	
	protected:
		// Add instance variables here

	public:
						DPlainTmpl (void);
		virtual void		ProcessFile (void);
		virtual void		ProcessDirectory (void);
	
	protected:
		// Add protected methods here
};

Dragon *CreateGDragon (void)
{
	// This is a function, not a method.  It's declared in Dragon.h and is NOT defined in Dragon.c
	
	return (Dragon *) new DPlainTmpl;
}

DPlainTmpl::DPlainTmpl (void)
{
	// You should initialize any added instance variables here Ñ this method will be called whenever an object of
	//	this class is created, immediately after Dragon::Dragon
// BEGIN bug-fix 1.1.1 Ñ default should be *not* to auto-quit
	autoQuit = FALSE;		// This will be overridden by whatever value for autoQuit is specified in the 'DrPr' 128 resource in
						//	your project resource file Ñ so just removing this line won't necessarily be enough to keep
						//	it from auto-quitting.  The file "PlainTmpl.¹.rsrc" has a 'DrPr' 128 that sets autoQuit = TRUE
// END bug-fix 1.1.1
}

void DPlainTmpl::ProcessFile (void)
{
	// This is the heart of your dragon.  You can open the file, change its file attributes, or whatever É
	// Simple changes can be simplified by using the macros defined in Dragon.h (curDocIsFile, curFileType, etc.)
	
	// Note that this method will not be called if you open a preferences file created by this dragon Ñ that's
	//	all taken care of for you by Dragon (see ProcessOwnedFile)
}

void DPlainTmpl::ProcessDirectory (void)
{
	// Use this method to process folders and disks instead of just ignoring them or looking inside them for files
	// If you don't use this method, you can just delete it (and its declaration above) Ñ I put it here merely for convenience
}

