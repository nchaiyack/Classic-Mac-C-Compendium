//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefs.h
//|
//| This is the interface to the Preferences class.
//|_________________________________________________________

#pragma once

class CPrefs
	{
	
	Str255	filename;				//  Name of preferences file
	short	size;					//  Size of preferences structure
	short	current_version;		//  Version number of current preferences
	
  protected:

	virtual short	*GetPrefsPointer(void);
	virtual void	Update(void);
	
  public:

	void			IPrefs(Str255 the_filename, short prefs_size, short version);
	virtual void	SetDefaults(void);
	void			Load(void);
	void			Save(void);

	};
