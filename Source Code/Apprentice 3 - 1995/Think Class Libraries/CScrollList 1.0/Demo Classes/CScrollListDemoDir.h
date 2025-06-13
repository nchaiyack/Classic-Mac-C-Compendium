/*************************************************************************************

 CScrollListDemoDir.c
	
		Interface for CScrollListDemoDir.
	
	SUPERCLASS = CShowcaseDemoDir
	
	REQUIRES : CScrollList
	
		© 1992 Dave Harkness

*************************************************************************************/


#include "CSApplication.h"


#define kWINDScrollListDemo		2000
#define kScPnScrollListDemo		2000
#define kSTRScrollListDemo		2000

#define kCheckCursorID		2000
#define kHandCursorID		2001


class CStringArray;


class CScrollListDemoDir : public CShowcaseDemoDir
{
public:

	virtual void		INewDemo( CDirectorOwner *aSupervisor);
	
	virtual void		Dispose( void);

protected:

	CStringArray		*itsArray;

};
