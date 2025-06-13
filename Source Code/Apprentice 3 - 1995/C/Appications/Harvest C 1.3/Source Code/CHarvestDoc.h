/****
 * CHarvestDoc.h
 *
 *	Document class for a typical application.
 *
 ****/

#define	_H_CHarvestDoc			/* Include this file only once */
#include <CDocument.h>
#include <CApplication.h>
#include <Processes.h>

class CHarvestPane;
class CList;
class CHarvestOptions;
class CDataFile;
class CWarningsArray;

struct CHarvestDoc : CDocument {
	public:
	CHarvestPane *itsTable;
	CHarvestOptions *itsOptions;
	CWarningsArray *itsWarnings;
	CList	*itsSourceFiles;
	OSType	itsSignature;
	unsigned long itsPartition;
	short itsSizeFlags;
	long StdAppVol;
	short StdAppDir;
	Str255 StdAppName;
	char *itsPrefix;

	virtual ProcessSerialNumber RunApp(void);
	virtual void AddLibraryFileHFS(Str63 aName,short aVolNum,long aDirID);
	virtual void AddResourceFileHFS(Str63 aName,short aVolNum,long aDirID);
	virtual void AddSourceFileHFS(Str63 aName,short aVolNum,long aDirID);
	virtual void AddResourceFile(SFReply *theFile);
	virtual void AddSourceFile(SFReply *theFile);
	virtual void AddLibFile(SFReply *theFile);
	virtual Boolean BringUpToDate(void);
	virtual Boolean PickAppName(void);
	virtual Boolean CheckLink(void);
	virtual Boolean DoLink(void);
	virtual Boolean Link(void);
	virtual void Clean(void);

									/** Construction/Destruction **/
    
    void        IHarvestDoc(CApplication *aSupervisor, Boolean printable);
    void        Dispose(void);
    
	void		DoOptions(CHarvestOptions *);
	void		DoWarnings(CHarvestOptions *,CWarningsArray *theWarnings);
	void		DoSetProjectInfo(void);
	void		DoCommand(long theCommand);
	Handle			WriteToHandle(void);
	void		ReadFromHandle(Handle);

    void        UpdateMenus(void);
    
    void        ResetGopher(void);
    
    void		Activate(void);

	void		NewFile(void);
	void		FSNewFile(char *,short,long);
	void		FSOpenFile(char *,short,long);
	void		OpenFile(SFReply *macSFReply);
	void		BuildWindow(Handle theData);
	
									/** Filing **/
	Boolean		DoSave(void);
	Boolean		DoSaveAs(SFReply *macSFReply);
	void		DoRevert(void);
	Boolean		Close(Boolean);
	int FindFile(char *,short,long);
};