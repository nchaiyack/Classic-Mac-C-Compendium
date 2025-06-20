/*  CSApplication.h
 *	
 *  Copyright 1992, tcl-talk@brown.edu
 *
 */

#define	_H_CSApplication
#include <CApplication.h>
#include <CDirector.h>
#include <CPane.h>
#include <CTable.h>


class CSDirector;


//***************************************************************************
													// C O N S T A N T S
#define		kExtraMasters		4
#define		kRainyDayFund		20480
#define		kCriticalBalance	20480
#define		kToolboxBalance		20480

#define		kInfoResType		'EXCs'

#define		kMENUShowcase		1024
#define 	cmdOpenShowcase		1024L
#define 	cmdLastShowcase		2000L

//***************************************************************************
													// A P P L I C A T I O N
struct CSApplication : CApplication {


	void	ISApplication(void);
	
	void	CreateDocument(void);
	void	RemoveDirector( CDirector *aDirector);
    
    void    SetUpMenus(void); 
    void    UpdateMenus(void); 

	void	DoCommand(long theCommand);
	
	void	ForceClassReferences( void);
    
	void	Exit(void);
	
	
	CSDirector		*itsNewDocWindow;

};


//***************************************************************************
													// D I R E C T O R

class CSPane;
class CSDirector : public CDirector {

public:

	CSPane		*itsPane;
	
	
    void        ISDirector( void);
	virtual void		Dispose(void);

	virtual void		DoCommand(long theCommand);
	virtual void		UpdateMenus(void);
	
};


//***************************************************************************
													// P A N E

class CSPane : public CPane {

public:	
	
    void        ISPane( CView *anEnclosure, CBureaucrat *aSupervisor);
	virtual void		Dispose(void);

};


//***************************************************************************
													// T A B L E

class CSTable : public CTable {

public:	
	
    void        ISTable( CView *anEnclosure, CBureaucrat *aSupervisor,
				short aWidth, short aHeight,
				short aHEncl, short aVEncl);
				
	virtual void		Dispose(void);

	virtual void		DoCommand( long aCmd);
	virtual void		DrawCell( Cell theCell, Rect *cellRect);

};


//***************************************************************************
												// D E M O   D I R E C T O R

class CShowcaseDemoDir : public CDirector {

public:

	virtual void	INewDemo( CDirectorOwner *aSupervisor);

};
