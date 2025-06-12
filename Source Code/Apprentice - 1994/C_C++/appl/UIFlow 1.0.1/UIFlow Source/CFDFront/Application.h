// **********************************************************************
//	the application class.
// **********************************************************************
class TTestApplication : public TApplication
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal struct TDocument *DoMakeDocument(CmdNumber itsCmdNumber);			// make a new document
		pascal void ITestApplication(OSType itsMainFileType);										// initialize the application
		pascal void DoSetupMenus (void);																		// set up the menus for this application
	};


// *********************************************************************************
// 	the geometry document class.
// *********************************************************************************
class TTestDocument : public TDocument 
	{
	public:
// ----------------------------------------------------------------------------------------------------
//	the methods
// ----------------------------------------------------------------------------------------------------
		pascal void DoMakeViews (Boolean);																	// make the views
		pascal struct TDocument * DoMakeDocument(CmdNumber);
		Boolean ITestDocument(Boolean);																		// set up the point fields.
		pascal void DoSetupMenus (void);
	};
	
// *********************************************************************************
// 	the geometry document class.
// *********************************************************************************
class TFilterDialog :  public TObject
	{
	public:
		void GetMachineInfo(char *, char *, char *);
		Boolean DoMachineDialog(void);
	};
	
