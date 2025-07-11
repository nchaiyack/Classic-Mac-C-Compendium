#include <Files.h>

OSErr	PBCatSearchIn(CSParamPtr pb, long parID);
OSErr	CatSearch(long* found, ...);

enum {
	csVRefNum,					// vRefNum (short)
	csVNamePtr,					// volume name (StringPtr)
	csMatchPtr,					// buffer (Ptr), size (long)
	csSearchTime,				// time (long) [defaults to zero]
	csOptBuffer,				// buffer (Ptr), size (long) [defaults to 1K buffer]
	
	csPartialName,				// name (StringPtr)
	csFullName,					// name (StringPtr)
	csFlAttrib,					// bits, mask (long)
	csFlFndrInfo,				// type (OSType), creator (OSType), flags (unsigned short), location (Point), fldr (short)
		csFInfoFDType,			// (OSType)
		csFInfoFDCreator,		// (OSType)
		csFInfoFDFlags,			// (unsigned short)
		csFInfoFDLocation,		// (Point)
		csFInfoFDFldr,			// (short) (not used in HFS)
	csFlLgLen,					// min, max (long)
	csFlPyLen,					// min, max (long)
	csFlRLgLen,					// min, max (long)
	csFlRPyLen,					// min, max (long)
	csFlCrDat,					// min, max (unsigned long)
	csFlMdDat,					// min, max (unsigned long)
	csFlBkDat,					// min, max (unsigned long)
	csFlXFndrInfo,				// iconID (short), script (short), XFlags (short), comment (short), putAway (long)
		csFXInfoFDIconID,		// (short)
		csFXInfoFDScript,		// (short)
		csFXInfoFDXFlags,		// (short)
		csFXInfoFDComment,		// (short)
		csFXInfoFDPutAway,		// (long)
	csFlParID,					// dir ID (long)
	csNegate,
	csDrUsrWds,					// rect (Rect), flags (short), location (Point), view (short)
		csDInfoFRRect,			// (Rect)
		csDInfoFRFlags,			// (short)
		csDInfoFRLocation,		// (Point)
		csDInfoFRView,			// (short)
	csDrNmFls,					// min, max (short)
	csDrCrDat,					// min, max (unsigned long)
	csDrMdDat,					// min, max (unsigned long)
	csDrBkDat,					// min, max (unsigned long)
	csDrFndrInfo,				// scroll (Point), openChain (long), comment (short), putAway (long)
		csDXInfoFRScroll,		// (Point)
		csDXInfoFROpenChain,	// (long)
		csDXInfoFRComment,		// (short)
		csDXInfoFRPutAway,		// (long)
	csDrParID,					// dir ID (long)
	
	csSearchInDirectory,		// dir ID (long)

	csContinue,					// Pass this to continue previous search
	csInitOnly,					// Pass this to set up parameters only; don't call PBCatSearch
	csEndList					// Pass this to end the list of search criteria
};

