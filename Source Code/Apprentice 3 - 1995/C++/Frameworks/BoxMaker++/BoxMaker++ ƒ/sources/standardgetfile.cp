
#include <Finder.h>
#include <StandardFile.h>

#include "standardgetfile.h"

standardgetfile::standardgetfile( short dlogID,
	OSType *thetypes, long numtypes, short *activelist)
	: StandardFileReply()
	, DLOG_ID( dlogID)
	, activeList( activelist)
{
	setTypes( thetypes, numtypes);
}

standardgetfile::~standardgetfile()
{
	delete theTypes;
}

void standardgetfile::changeTypes( OSType *thetypes, long numtypes)
{
	delete theTypes;
	setTypes( thetypes, numtypes);
}

Boolean standardgetfile::doIt()
{
	Point where = { -1, -1};

	CustomGetFile(
			theFileFilterUPP,
			numTypes,
			theTypes,
			(StandardFileReply *)this,
			DLOG_ID,
			where,
			theDlogHookUPP,
			theModalFilterUPP,
			activeList,
			theActivateUPP,
			(void *)this	// Ptr yourDataPtr
	);
	return sfGood;
}

pascal Boolean theFileFilter( CInfoPBPtr myPB, Ptr myDataPtr)
{
	standardgetfile *it = (standardgetfile *)myDataPtr;
	return it->filterThisItem( myPB);
}

pascal short theDlogHook( short item, DialogPtr theDialog, Ptr myDataPtr)
{
	standardgetfile *it = (standardgetfile *)myDataPtr;
	return it->handleItemPress( item, theDialog);
}

pascal Boolean theModalFilter( DialogPtr theDialog,
								const EventRecord *theEvent,
								short *itemHit, Ptr myDataPtr)
{
	standardgetfile *it = (standardgetfile *)myDataPtr;
	return it->filterEvent( theDialog, theEvent, itemHit);
}

pascal void theActivate(
	DialogPtr theDialog, short item, Boolean activating, Ptr myDataPtr)
{
	standardgetfile *it = (standardgetfile *)myDataPtr;
	it->handleActivation( theDialog, item, activating);
}

FileFilterYDUPP standardgetfile::theFileFilterUPP =
				NewFileFilterYDProc( theFileFilter);

DlgHookYDUPP standardgetfile::theDlogHookUPP =
				NewDlgHookYDProc( theDlogHook);

ModalFilterYDUPP standardgetfile::theModalFilterUPP =
				NewModalFilterYDProc( theModalFilter);

ActivateYDUPP standardgetfile::theActivateUPP =
				NewActivateYDProc( theActivate);

void standardgetfile::setTypes( OSType *thetypes, long numtypes)
{
	theTypes = new OSType[ numtypes];
	numTypes = numtypes;
	for( int i = 0; i < numtypes; i++)
	{
		theTypes[ i] = thetypes[ i];
	}
}
