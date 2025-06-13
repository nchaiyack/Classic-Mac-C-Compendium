/* Dialectic Filters.h
*/


	// Conversion functions from the Raw Dialects folder.
	
extern void ConvertChef();
extern void ConvertFudd();
extern void ConvertOlde();
extern void ConvertUbby(); 
extern void ConvertPig();
extern void ConvertRot13();
extern void ConvertMorse();
extern void ConvertWAREZ();
extern void ConvertOp();

	
	// Data structures
	
typedef struct
{
	short dialect;
}
TPreferences;


typedef struct
{
	Handle theHandle;
	char    *base;
	unsigned long length;
	unsigned long offset;
}
TCharacterStream;


	// Constants
	
typedef enum
{
	kApply = 1,
	kCancel,
	kAbout,
	kBalloonHelpToggle,
	kLine,
	
	kChef = 6,
	kFudd,
	kOlde,
	kWAREZ,
	kPig,
	kRot13,
	kMorse,
	kUbby,
	kOp
}
TDialogItemKind;

#define kFirstDialect (kChef)
#define kLastDialect (kOp)


typedef enum
{
	kAboutLine = 7
}
TAboutBoxItemKind;

	
	// Resource IDs

	// Dialogs
#define kOptionsDlg   128
#define kAboutDlg     129
#define kStopAlertDlg 130
#define kNoteAlertDlg 131

	// STR# Resources
#define kErrorStringsResID	129
#define kAboutStringsResID	130

	// Error Messages / return values
#define kSuccess						0
#define kErrBBEditVersionNotHighEnough	1
#define kErrWrongWindowKind				2
#define kErrNeedSelection				3
#define kErrBalloonHelpNotAvailable		4
#define kErrWindowContentsNil			5
#define kErrNotEnoughMemory				6
#define kErrInsaneDialectValue			7	

#define kPrefsResourceType 'DiFi'

	// Global Variables from Dialectic Filters.c
extern Boolean 			gAbort;
extern TCharacterStream gInput, gOutput;
