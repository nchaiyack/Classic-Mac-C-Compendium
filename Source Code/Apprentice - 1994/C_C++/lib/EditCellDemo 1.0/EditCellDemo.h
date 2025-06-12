#pragma once


#include "CSApplication.h"


struct CEditCell : CEditText
{
	void IEditCell( CView *anEnclosure, CBureaucrat *aSupervisor );		
	virtual void PlaceCell( LongRect *aPlace );
	virtual Boolean BecomeGopher( Boolean isBecoming );
};


struct CEditTable : CTable
{
	CEditCell* itsEditor;
	
	void IEditTable
		( CView *anEnclosure
		, CBureaucrat *aSupervisor
		, short aWidth
		, short aHeight
		, short aHEncl
		, short aVEncl
		, SizingOption aHSizing
		, SizingOption aVSizing );
	
	virtual void DoClick( Point hitPt, short modifierKeys, long when );
		
	virtual void EditCell( Cell editCell );
};


struct CEditTableDemoDir : CShowcaseDemoDir
{
	virtual void INewDemo( CDirectorOwner *aSupervisor);
};
