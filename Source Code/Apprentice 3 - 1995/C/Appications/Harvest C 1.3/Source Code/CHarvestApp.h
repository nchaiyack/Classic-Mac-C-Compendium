/*****
 * CHarvestApp.h
 *
 *	Application class for Harvest C
 *
 *****/

#define	_H_CHarvestApp		/* Include this file only once */
#include <CApplication.h>

struct CHarvestApp : CApplication {

	CursHandle			BeachBalls[4];
	int					BeachIndex;
	
	FSSpec	HarvestCTree;

	long	StdIncludeDir;
	short	StdIncludeVol;
	long	StdLibDir;
	short	StdLibVol;

	void	IHarvestApp(void);
	void	SetUpFileParameters(void);
    
    void    SetUpMenus(void); 
    void    UpdateMenus(void); 

	void	ChooseProjectFile(SFReply *);
	void	ChooseSourceFile(SFReply *);
	void	ChooseLibFile(SFReply *);
	void	DoCommand(long theCommand);
    
	void	Exit(void);

	void	CreateProject(void);
	void 	OpenProject(SFReply *macSFReply);
	void	StartUpAction(short);
	void	DoAppleEvent( CAppleEvent *anAppleEvent);
	void	CreateDocument(void);
	void	OpenDocument(SFReply *macSFReply);
	void	DoAbout(void);
	void	DoRegistration(void);
	void	SpinCursor(void);
};
