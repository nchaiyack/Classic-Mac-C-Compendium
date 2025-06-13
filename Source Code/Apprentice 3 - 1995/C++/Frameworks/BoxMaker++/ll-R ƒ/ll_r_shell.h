
class ll_r_settings
{
	public:
		enum
		{
			kType = 1,
			kCreator,
			kSize,
			kBytes,
			kKiloBytes,
			kCreatDate,
			kModDate,
			kBakDate,
			kTimes,

			kTabs,
			kSpaces,
			kAskFileName,
			kIndentFolders,
			kNumButtons
		};
		Boolean myPrefs[ kNumButtons];
};

typedef preferences<ll_r_settings> ll_r_prefs;

#pragma template preferences<ll_r_settings>;

class ll_r_shell : public boxmaker, public ll_r_prefs
{
	public:
		ll_r_shell( Str255 prefsFileName, ll_r_settings defaultsettings);
		~ll_r_shell();

		virtual void EnterFolder( Boolean opening);
		virtual void OpenDoc( Boolean opening);
		virtual void CantEnterFolder( Boolean opening);
		virtual void ExitFolder( Boolean opening);

		virtual void StartABunch( long numTopLevelItems, Boolean opening);

		virtual void HandleDialogEvent( short itemHit, DialogPtr theDialog);

	private:
		short outFileNo;
		long  outVRefNum;
				
		void SetupDialog( DialogPtr theDialog);
		void FlipButton( DialogPtr theDialog, short theItem);

		long indentationLevel;
		unsigned char plentySpace[ 1000];
		unsigned char delimiter;
		//
		// 'append' appends the specified string to 'plentySpace'
		// without checking for running out of space. It uses and
		// updates 'currentEnd', and also appends a delimiter as
		// specified by the preferences.
		//
		unsigned char *currentEnd;
		void append( char letter);
		void append( char *item, int len);
		void append( unsigned char *item);		// appends a Pascal string
		void append( char *item);				// appends a C string
		void appendDate( unsigned long seconds);
		void append2Digits( short number);
		void RemoveLastChar();
		void ChangeLastToNewLine();
		void StartNewLine();
		long KiloBytes( long bytes);
};

inline void ll_r_shell::EnterFolder( Boolean opening)
{
	if( myPrefs[ kIndentFolders])
	{
		//
		// Note: we do not use 'delimiter' for the first characters.
		// Instead we always use a tab there. I think this looks nicer.
		//
		plentySpace[ indentationLevel] = '\t';
		indentationLevel += 1;
	}
}

inline void ll_r_shell::ExitFolder( Boolean opening)
{
	if( myPrefs[ kIndentFolders])
	{
		indentationLevel -= 1;
	}
}

inline void ll_r_shell::append( char letter)
{
	*currentEnd++ = letter;
}

inline void ll_r_shell::append( unsigned char *item)
{
	const int len = *item;
	append( (char *)item + 1, len);
}

inline void ll_r_shell::append( char *item)
{
	while( *item != 0)
	{
		*currentEnd++ = *item++;
	}
}

inline void ll_r_shell::append2Digits( short number)
{
	const short lastdigit  = number % 10;
	const short firstdigit = (number / 10) % 10;
	append( firstdigit + '0');
	append( lastdigit  + '0');
}

inline void ll_r_shell::RemoveLastChar()
{
	currentEnd -= 1;
}

inline void ll_r_shell::ChangeLastToNewLine()
{
	currentEnd[ -1] = '\r';
}

inline void ll_r_shell::StartNewLine()
{
	currentEnd = &plentySpace[ indentationLevel];
}

inline long ll_r_shell::KiloBytes( long bytes)
{
	return ((bytes + 1023) >> 10);
}
