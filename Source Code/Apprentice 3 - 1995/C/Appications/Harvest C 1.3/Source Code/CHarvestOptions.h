/****
 * CHarvestOptions.h
 *
 *	Document class for a typical application.
 *
 ****/

#define	_H_CHarvestOptions			/* Include this file only once */
#include <CObject.h>

struct CHarvestOptions : CObject {
	public:
	Boolean trigraphs;
	Boolean requireProtos;
	Boolean useMC68020;
	Boolean useMC68881;
	Boolean signedChars;
	Boolean MacsBugSymbols;
	Boolean int2byte;
	Boolean progress;
	Boolean bigGlobals;
	Boolean allWarnings;
	Boolean noWarnings;
	Boolean warnings[64];

	void IHarvestOptions(void);
};