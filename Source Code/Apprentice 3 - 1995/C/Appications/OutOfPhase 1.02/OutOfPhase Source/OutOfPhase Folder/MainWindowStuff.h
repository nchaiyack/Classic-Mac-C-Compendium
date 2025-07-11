/* MainWindowStuff.h */

#ifndef Included_MainWindowStuff_h
#define Included_MainWindowStuff_h

/* MainWindowStuff module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Screen */
/* EventLoop */
/* Menus */
/* Files */
/* WindowDispatcher */
/* Array */
/* CodeCenter */
/* TextEdit */
/* SampleList */
/* FunctionList */
/* AlgoSampList */
/* WaveTableList */
/* AlgoWaveTableList */
/* InstrList */
/* TrackList */
/* Alert */
/* StartupOpen */
/* CalculatorWindow */
/* GrowIcon */
/* Main */
/* DisassemblyWindow */
/* PcodeSystem */
/* DataMunging */
/* FixedPoint */
/* SampleConsts */
/* NumberDialog */
/* Numbers */
/* GlobalWindowMenuList */
/* BinaryCodedDecimal */
/* PlayPrefsDialog */
/* ImportWAVSample */
/* ImportRAWSample */
/* ImportAIFFSample */
/* BufferedFileInput */
/* BufferedFileOutput */

#include "Screen.h"
#include "EventLoop.h"
#include "Menus.h"
#include "FixedPoint.h"
#include "SampleConsts.h"

struct MainWindowRec;
typedef struct MainWindowRec MainWindowRec;

/* forward declarations */
struct FileSpec;
struct FileType;
struct CalcWindowRec;
struct DisaWindowRec;
struct WaveTableObjectRec;
struct SampleObjectRec;
struct BufferedInputRec;
struct BufferedOutputRec;
struct SampleListRec;
struct AlgoSampListRec;
struct WaveTableListRec;
struct AlgoWaveTableListRec;
struct InstrListRec;

/* this enum is used for remembering what kind of final output we want */
typedef enum
	{
		eOutput8Bits EXECUTE(= -5135),
		eOutput16Bits,
		eOutput24Bits,
		eOutput32Bits
	} OutputNumBitsType;

/* errors that can occur when attempting to load or save a file */
typedef enum
	{
		eFileLoadNoError EXECUTE(= -31251),
		eFileLoadBadFormat,
		eFileLoadDiskError,
		eFileLoadOutOfMemory
	} FileLoadingErrors;


/* initialize internal data structures for documents */
MyBoolean						InitializeDocuments(void);

/* clean up any internal data structures used for documents */
void								ShutdownDocuments(void);


/* open a new document window and load the document in.  the function takes */
/* ownership of the file specification. */
void								OpenDocument(struct FileSpec* TheFile);

/* close all open documents, subject to the user's ok */
void								DoCloseAllQuitPending(void);

/* close a document.  If the user cancelled, then return False, otherwise True */
MyBoolean						CloseDocument(MainWindowRec* Window);

/* save the document into the current file.  if it hasn't been saved, then call SaveAs */
/* it returns False if it fails. */
MyBoolean						SaveDocument(MainWindowRec* Window);

/* save the document into a new file (don't disturb the current one).  returns False */
/* if it fails.  this function calls SaveDocument() to do the work. */
MyBoolean						SaveDocumentAs(MainWindowRec* Window);


/* return True if the document has been modified & should be saved. */
MyBoolean						HasDocumentBeenModified(MainWindowRec* Window);


/* main window event handling functions */
void								MainWindowDoIdle(MainWindowRec* Window,
											MyBoolean CheckCursorFlag, OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers);
void								MainWindowBecomeActive(MainWindowRec* Window);
void								MainWindowBecomeInactive(MainWindowRec* Window);
void								MainWindowJustResized(MainWindowRec* Window);
void								MainWindowDoMouseDown(OrdType XLoc, OrdType YLoc,
											ModifierFlags Modifiers, MainWindowRec* Window);
void								MainWindowDoKeyDown(unsigned char KeyCode, ModifierFlags Modifiers,
											MainWindowRec* Window);
void								MainWindowClose(MainWindowRec* Window);
void								MainWindowUpdator(MainWindowRec* Window);
void								MainWindowMenuSetup(MainWindowRec* Window);
void								MainWindowDoMenuCommand(MainWindowRec* Window,
											MenuItemType* MenuItem);


/* get the number of spaces per tab that editors should use */
long								MainWindowGetTabSize(MainWindowRec* Window);


/* build any function objects that need to be built.  returns True if all of them */
/* were built without a problem. */
MyBoolean						MainWindowMakeUpToDateFunctions(MainWindowRec* Window);

/* build any algorithmic samples that need to be built.  returns True if all of them */
/* were built without a problem. */
MyBoolean						MainWindowMakeUpToDateAlgoSamps(MainWindowRec* Window);

/* build any algorithmic wave tables that need to be built.  returns True if all of */
/* them were built without a problem. */
MyBoolean						MainWindowMakeUpToDateAlgoWaveTables(MainWindowRec* Window);

/* build any instrument specifications that need to be built.  returns True if all of */
/* them were built without a problem. */
MyBoolean						MainWindowMakeUpToDateInstrList(MainWindowRec* Window);


/* build everything.  returns True if everything built correctly */
MyBoolean						MainWindowMakeEverythingUpToDate(MainWindowRec* Window);


/* deselect any selection in a scrolling list other than the specified list. */
/* specified list may be NIL for unconditional deselect */
void								MainWindowDeselectAllOtherStringLists(MainWindowRec* Window,
											void* TheDontDeselectStringList);


/* create a new calculator window.  the main window keeps track of all calculator */
/* objects that it has created */
void								MainWindowNewCalculator(MainWindowRec* Window);

/* when a calculator window closes, it calls this to make sure the main window */
/* object knows that it no longer exists */
void								MainWindowCalculatorClosingNotify(MainWindowRec* Window,
											struct CalcWindowRec* Calc);


/* notify the main window that a new disassembly window has been created. */
MyBoolean						MainWindowNewDisassemblyNotify(MainWindowRec* Window,
											struct DisaWindowRec* DisassemblyWindow);

/* notify the main window that a disassembly window has been destroyed. */
void								MainWindowDisassemblyClosingNotify(MainWindowRec* Window,
											struct DisaWindowRec* DisassemblyWindow);


/* enable global menu items.  these are menu items that can be chosen regardless */
/* of which editor is open (such as New Object, Save...) */
void								MainWindowEnableGlobalMenus(MainWindowRec* Window);

/* this checks to see if the menu item is a global menu item.  if it is, the */
/* associated action is performed and it returns True.  if not, then it returns */
/* False and the specific editor window must handle the menu item. */
MyBoolean						MainWindowDoGlobalMenuItem(MainWindowRec* Window,
											MenuItemType* MenuItem);


/* get a copy of the left-channel array from a stereo sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleLeftCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut);

/* get a copy of the right-channel array from a stereo sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleRightCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut);

/* get a copy of the sample array from a mono sample.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetSampleMonoCopy(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut);

/* get the number of frames per wave period for the specified wave table.  an error */
/* code is returned indicating success or failure.  the caller is responsible for */
/* disposing of the name string. */
SampleErrors				MainWindowGetWaveTableFrameCount(void* Window,
											char* NullTerminatedName, long* FrameCountOut);

/* get the number of tables in the specified wave table.  an error */
/* code is returned indicating success or failure.  the caller is responsible for */
/* disposing of the name string. */
SampleErrors				MainWindowGetWaveTableTableCount(void* Window,
											char* NullTerminatedName, long* TableCountOut);

/* get a copy of the sample array from a wave table.  an error is returned */
/* indicating the success or failure of this call.  the caller is responsible for */
/* disposing both the returned array (*DataOut) and the name string */
SampleErrors				MainWindowGetWaveTableArray(void* Window,
											char* NullTerminatedName, largefixedsigned** DataOut);

/* open a new sample editor initialized with the parameters and install the data */
/* in the array RawData into it.  this array is NOT largefixedsigned, but rather is */
/* signed char or signed short, depending on the setting of NumBits.  the caller */
/* is responsible for disposing of RawData. */
struct SampleObjectRec*	MainWindowCopyRawSampleAndOpen(MainWindowRec* Window,
											char* RawData, NumBitsType NumBits, NumChannelsType NumChannels,
											long Origin, long LoopStart1, long LoopStart2, long LoopStart3,
											long LoopEnd1, long LoopEnd2, long LoopEnd3, long SamplingRate,
											double NaturalFrequency);

/* open a new wave table editor initialized with the parameters and install the data */
/* in the array RawData into it.  this array is not largefixedsigned, but rather is */
/* signed char or signed short, depending on the setting of NumBits.  the caller */
/* is responsible for disposing of RawData. */
struct WaveTableObjectRec*	MainWindowCopyRawWaveTableAndOpen(MainWindowRec* Window,
											char* RawData, NumBitsType NumBits, long NumTables,
											long FramesPerTable);


/* get a copy of the name of the current document file.  the name is a heap-allocated */
/* non-null-terminated block. */
char*								GetCopyOfDocumentName(MainWindowRec* Window);

/* dispatch a name change event.  this tells all editors that the document title */
/* has changed, and the window titles should be updated accordingly. */
void								MainWindowDispatchNameChange(MainWindowRec* Window);


/* get various attributes for playing back the song */
MyBoolean						MainWindowGetStereo(MainWindowRec* Window);
MyBoolean						MainWindowGetSurround(MainWindowRec* Window);
long								MainWindowGetSamplingRate(MainWindowRec* Window);
long								MainWindowGetEnvelopeRate(MainWindowRec* Window);
double							MainWindowGetBeatsPerMinute(MainWindowRec* Window);
double							MainWindowGetVolumeScaling(MainWindowRec* Window);
OutputNumBitsType		MainWindowGetOutputNumBits(MainWindowRec* Window);
MyBoolean						MainWindowGetInterpolationOverTime(MainWindowRec* Window);
MyBoolean						MainWindowGetInterpolationAcrossWaves(MainWindowRec* Window);
double							MainWindowGetScanningGap(MainWindowRec* Window);
double							MainWindowGetBufferDuration(MainWindowRec* Window);
MyBoolean						MainWindowGetClipWarning(MainWindowRec* Window);
char*								MainWindowGetPostProcessing(MainWindowRec* Window);
MyBoolean						MainWindowGetPostProcessingEnable(MainWindowRec* Window);


/* store new values for song playback attributes */
void								PutMainWindowStereo(MainWindowRec* Window, MyBoolean NewStereoFlag);
void								PutMainWindowSurround(MainWindowRec* Window, MyBoolean NewSurround);
void								PutMainWindowSamplingRate(MainWindowRec* Window,
											long NewSamplingRate);
void								PutMainWindowEnvelopeRate(MainWindowRec* Window,
											long NewEnvelopeRate);
void								PutMainWindowBeatsPerMinute(MainWindowRec* Window,
											double NewBeatsPerMinute);
void								PutMainWindowVolumeScaling(MainWindowRec* Window,
											double NewVolumeScaling);
void								PutMainWindowOutputNumBits(MainWindowRec* Window,
											OutputNumBitsType NewOutputNumBits);
void								PutMainWindowInterpolationOverTime(MainWindowRec* Window,
											MyBoolean NewInterpOverTime);
void								PutMainWindowInterpolationAcrossWaves(MainWindowRec* Window,
											MyBoolean NewInterpAcrossWaves);
void								PutMainWindowScanningGap(MainWindowRec* Window,
											double NewScanningGap);
void								PutMainWindowBufferDuration(MainWindowRec* Window,
											double NewBufferDuration);
void								PutMainWindowClipWarning(MainWindowRec* Window,
											MyBoolean NewClipWarning);
void								PutMainWindowPostProcessing(MainWindowRec* Window,
											char* NewPostProcessing);
void								PutMainWindowPostProcessingEnable(MainWindowRec* Window,
											MyBoolean NewPostProcessingEnable);


/* read the general information subblock from the specified file. */
FileLoadingErrors		MainWindowReadData(MainWindowRec* Window,
											struct BufferedInputRec* Input);

/* write the general information subblock to the specified file. */
FileLoadingErrors		MainWindowWriteData(MainWindowRec* Window,
											struct BufferedOutputRec* Output);


/* get the sample list object for the specified document.  the actual thing */
/* is returned */
struct SampleListRec*	MainWindowGetSampleList(MainWindowRec* Window);

/* get the algorithmic sample list object for the specified document.  the actual */
/* thing is returned. */
struct AlgoSampListRec*	MainWindowGetAlgoSampList(MainWindowRec* Window);

/* get the wave table list object for the specified document.  the actual thing */
/* is returned */
struct WaveTableListRec*	MainWindowGetWaveTableList(MainWindowRec* Window);

/* get the algorithmic wave table list object for the specified document.  the */
/* actual thing is returned */
struct AlgoWaveTableListRec*	MainWindowGetAlgoWaveTableList(MainWindowRec* Window);

/* get the instrument list object for the specified document.  the actual thing */
/* is returned */
struct InstrListRec*	MainWindowGetInstrList(MainWindowRec* Window);


/* this updates the object deletion undo information */
void								MainWindowNewDeleteUndoInfo(MainWindowRec* Window,
											struct FileSpec* Location, struct FileType* File);

#endif
