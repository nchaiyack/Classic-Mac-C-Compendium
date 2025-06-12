/*	2a. variables */


extern MenuHandle DeskMenu;
extern MenuHandle FileMenu;
extern MenuHandle EditMenu;
extern MenuHandle FindMenu;
extern MenuHandle MarkMenu;
extern MenuHandle WindowMenu;
extern MenuHandle ConvertMenu;

extern Str255 gFindBuffer ;
extern Str255 gReplaceBuffer ;
extern Str255 gCommandLine ;

extern Handle AppScratch;

extern short gAppResRefNum;
extern short gPreferencesFile;
extern short gSearchMethod ;

extern Boolean gAppIsRunning ;
extern Boolean gSearchBackwards ;
extern Boolean gCaseSensitive ;
extern Boolean gWrapSearch ;
extern Boolean gUseWorksheet ;

extern DocumentRecord Documents[];
extern DocumentPeek CurrentScrollDocument;
extern DocumentPeek gWorkSheetDoc;
extern DocumentPeek gClipboardDoc;
extern WindowPtr gTextWindowPosition;

extern DialogRecord DlogStor;
extern DialogRecord AuxDlogStor;

extern CTabHandle MyCMHandle;
extern PaletteHandle DefaultPalette;

extern Rect DragBoundsRect;
extern short NumWindows;

extern short NumFileTypes ;
extern SFTypeList MyFileTypes ;

extern Boolean gHasColorQD;
extern Boolean gMacPlusKBD;
extern Boolean gInBackground;
extern Boolean gDAonTop;
extern short gLastScrapCount;
/* OSType gClipType; */
extern FSSpec DefaultSpecs;
extern FSSpec HomeSpecs;

/* to pass an event record into ScrollAction */
extern EventRecord *PassEvent;

extern unsigned long gSleepTime;

