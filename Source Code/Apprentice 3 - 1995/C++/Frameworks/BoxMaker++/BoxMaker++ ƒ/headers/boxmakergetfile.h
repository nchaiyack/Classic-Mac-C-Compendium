//
// boxmakergetfile assumes that, if the dialog given to it has a tenth
// item, it is a button called 'Select highlighted item'
//
typedef struct OSTypePair
{
	OSType type;
	OSType creator;
} OSTypePair;

class boxmakergetfile : public standardgetfile
{
	public:
		boxmakergetfile( short dlogID = sfGetDialogID);
		
//		~boxmakergetfile();

	protected:

		Boolean matchesTypeList( OSType ourType) const;
		Boolean matchesTypeCreatorPairs( OSType theType, OSType theCreator) const;
		//
		// Functions to read the list of types accepted by this application,
		// as read from the 'tycr' resource #128. Present for use by subclasses.
		//
		const OSTypePair *GetTheTypeCreatorPairs() const;
		short GetNumTypeCreatorPairs() const;

		virtual Boolean filterThisItem( const CInfoPBPtr myPB);
		virtual short handleItemPress( short item, DialogPtr theDialog);
		//
		// Four booleans, read from 'flgs' resource #128:
		//
		Boolean enterFolders;
		Boolean passFolders;
		Boolean enterInvisibles;
		Boolean passInvisibles;
		//
		// A boolean, derived from the above flags, cached for efficiency:
		//
		Boolean allowFolderSelection;

		static Boolean matches( OSType original, OSType pattern);

		ControlHandle selectThisItemControl;
		//
		// the list of file types to look at, as read from the 'typs' resource #128
		// (950531: these are stored as part of the 'standardgetfile' superclass)
		//
		// the list which further restricts the type/creator pairs.
		// Read from 'tycr' resource #128
		//
		OSTypePair *theTypeCreatorPairs;
		short   numTypeCreatorPairs;
};

inline Boolean boxmakergetfile::matches( OSType original, OSType pattern)
{
	return ((original == pattern) || (pattern == '****'));
}

inline const OSTypePair *boxmakergetfile::GetTheTypeCreatorPairs() const
{
	return (const OSTypePair *)theTypeCreatorPairs;
}

inline short boxmakergetfile::GetNumTypeCreatorPairs() const
{
	return numTypeCreatorPairs;
}

