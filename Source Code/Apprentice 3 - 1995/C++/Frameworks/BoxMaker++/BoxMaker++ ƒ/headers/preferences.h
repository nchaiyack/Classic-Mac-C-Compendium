//
// preferences is a template for a preferences file
//
template<class C> class preferences : public C
{
	public:
		enum
		{
			kFourQuestionMarks = 0x3F3F3F3F
		};
		preferences( Str255 fName, const C &defaults,
					OSType fileCreator = kFourQuestionMarks);

		~preferences();

	private:
		FSSpec theFileSpec;
		const OSType iFileCreator;
		//
		// theDefaults holds a copy of the default settings, as found in the
		// preferences file or in the default settings. The destructor compares
		// it to the current settings. If they differ the current settings are
		// written to the preferences file.
		//
		C theDefaults;

		enum
		{
			kPreferencesInfo = -16397
		};
		//
		// defaultsChanged returns true when the default settings
		// have changed from the ones past to the constructor.
		//
		Boolean defaultsChanged() const;
};
