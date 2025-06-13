//
// standardgetfile is a base class to interface to CustomGetFile
//
// Note for CodeWarriors: if your compiler complains that it can not
// use a struct as a base class for standardgetfile you probably are using
// a precompiled header file generated with the C compiler with this C++
// code. You should recompile them with the C++ compiler, and all should
// be fine.
//
class standardgetfile : protected StandardFileReply
{
	public:
		standardgetfile( short dlogID = sfGetDialogID,
			OSType *thetypes = 0L, long numtypes = -1, short *activelist = nil);

		~standardgetfile();

		void changeTypes( OSType *thetypes, long numtypes);

		Boolean doIt();

		const StandardFileReply &operator()();

		//
		// Functions to read the list of types accepted by this application,
		// as read from the 'typs' resource #128. Present for use by subclasses.
		//
		const OSType *GetTheTypes() const;
		short GetNumTypes() const;

	protected:
		//
		// override these four virtual members when appropriate
		//
		virtual Boolean filterThisItem( const CInfoPBPtr myPB)
		{
			//
			// default: filter out invisible files
			//
			return ((myPB->hFileInfo.ioFlFndrInfo.fdFlags & fInvisible) != 0);
		};

		virtual short handleItemPress(
								short item, DialogPtr theDialog)
		{
			return item;			
		};

		virtual Boolean filterEvent( DialogPtr theDialog,
						const EventRecord *theEvent, short *itemHit)
		{
			return false;
		}

		virtual void handleActivation(
				DialogPtr theDialog, short item, Boolean activating)
		{
		
		}

		OSType		*theTypes;
		long		numTypes;

	private:
		//
		// The four functions to pass to CustomGetFile and
		// their UPP's:
		//
		friend static pascal Boolean theFileFilter(
											CInfoPBPtr myPB, Ptr myDataPtr);

		friend static pascal short theDlogHook(
						short item, DialogPtr theDialog, Ptr myDataPtr);


		friend static pascal Boolean theModalFilter(
											DialogPtr theDialog,
											const EventRecord *theEvent,
											short *itemHit, Ptr myDataPtr);

		friend static pascal void theActivate(
											DialogPtr theDialog,
											short item,
											Boolean activating, Ptr myDataPtr);

		static FileFilterYDUPP		theFileFilterUPP;
		static DlgHookYDUPP			theDlogHookUPP;
		static ModalFilterYDUPP		theModalFilterUPP;
		static ActivateYDUPP		theActivateUPP;

		void setTypes( OSType *thetypes, long numtypes);

		const short	DLOG_ID;
		short		*activeList;
};

inline const StandardFileReply &standardgetfile::operator()()
{
	return (const StandardFileReply &)(*this);
}

inline const OSType *standardgetfile::GetTheTypes() const
{
	return (OSType * const)theTypes;
}

inline short standardgetfile::GetNumTypes() const
{
	return numTypes;
}

