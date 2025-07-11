/*	2a. variables */

extern MenuHandle DeskMenu;
extern MenuHandle FileMenu;
extern MenuHandle EditMenu;
extern MenuHandle FindMenu;
extern MenuHandle MarkMenu;
extern MenuHandle WindowMenu;
extern MenuHandle ViewMenu;
extern MenuHandle ReportsMenu;
extern MenuHandle DataMenu;
extern MenuHandle RotateMenu;
extern MenuHandle ScaleMenu;
extern MenuHandle DataFieldsMenu;
extern MenuHandle SelectTemplateMenu;

extern Handle AppScratch;

extern short gAppResRefNum;
extern short gPrefsResRefNum;
extern short gSearchMethod ;

extern Boolean gAppIsRunning ;
extern Boolean gSearchBackwards ;
extern Boolean gCaseSensitive ;
extern Boolean gWrapSearch ;

extern short gSelectedBooks ;
extern unsigned short gProcessingFlagBits ;
extern unsigned short gTextOutputFlagBits ;
extern short gNumRoseBins ;
extern short gCurrentTemplate ;
extern Handle gMapHandles ;

extern Str255 gCommandLine ;
extern Str255 gFindBuffer ;
extern Str255 gReplaceBuffer ;

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
extern short Scaling;
extern short NumWindows;
extern short ToolWindowVisible;
extern short SurveySelected;
extern short SelectedFSB;
extern char SelectedDesig[];

extern short NumFileTypes ;
extern SFTypeList MyFileTypes ;
extern Boolean gHasColorQD;
extern Boolean gMacPlusKBD;
extern Boolean gInBackground;
extern Boolean gDAonTop;
extern short gLastScrapCount;
extern OSType gClipType;
extern FSSpec DefaultSpecs;
extern FSSpec HomeSpecs;

/* to pass an event record into ScrollAction */
extern EventRecord *PassEvent;

extern unsigned long gSleepTime;
