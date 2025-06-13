//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CHyperCuberApp.h
//|
//| This file contains the interface to the HyperCuber Application class.
//|___________________________________________________________________________

#define	_H_CHyperCuberApp		/* Include this file only once */
#include <CApplication.h>

class CControlsDirector;
class CHyperCuberDoc;

struct CHyperCuberApp : CApplication {

	CHyperCuberDoc		*itsDocument;		//  The document (which controls the graphics window)

public:

	void	IHyperCuberApp(void);
	void	SetUpFileParameters(void);

	void	InspectSystem( void);

    void    UpdateMenus(void); 

	void	MakeDesktop(void);

	void	DoCommand(long theCommand);

	void	Exit(void);

	void	CreateDocument(void);
	void	OpenDocument(SFReply *macSFReply);

};
