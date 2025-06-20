
class touchsettings
{
	public:
		int whichGrain;
		int whichDate;
};

typedef preferences<touchsettings> touchprefs;

#pragma template preferences<touchsettings>;

class touchshell : public boxmaker, public touchprefs
{
	public:
		touchshell( Str255 prefsFileName, touchsettings &defaultsettings);
		
		enum
		{
			kSecondButton = 1,
			kMinuteButton,
			kHourButton,
			kHalfDayButton,
			kDayButton,
			kGroundZeroButton,

			kCreatDate,
			kModDate,
			kBothDates
		};

	protected:
		virtual void OpenDoc( Boolean opening);

		virtual void HandleDialogEvent( short itemHit, DialogPtr theDialog);

	private:
		void ChangeGrain( int newGrain);
		void ChangeDate( int newDate);

		unsigned long starting_time;
		unsigned long time_to_set;
		
		const long *theGrains;
};
