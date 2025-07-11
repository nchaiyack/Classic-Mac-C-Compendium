/* QD32Exists.c
Apple suggests, and I agree, that all new programs should use the new Gestalt
manager, that is documented in Inside Mac VI. It's very easy to use, as
illustrated below.

The only subtlety in the code below is that if Gestalt() is NOT present--because
the program is being run under a System before 6.04--then the compiler's glue
code is supposed to cause Gestalt() to return with an error code, rather than
crash because of a missing trap. Same goes for the call to SysEnvirons(), which
should be present on all Systems after 4.1.

The answers are cached, so subsequent calls after the first will be quick.

HISTORY:
4/21/90	dgp	added statics to avoid wasting time recomputing this.
2/27/91 dgp	added QD8Exists().
4/15/91 dgp	added NewPaletteManager() to determine whether the calls documented
			in the Palette Manager chapter of Inside Mac VI are available.
8/24/91	dgp	Made compatible with THINK C 5.0.
3/27/92	dgp	Use Gestalt() instead of SysEnvirons(), as recommended by Apple. 
9/15/92 dgp Changed NewPaletteManager() to check only the system version. Apple
			documents had indicated to me that the new palette manager calls
			would be available in any system if QD32 were present, so I had
			ORed that in. However, I find that the HasDepth() call fails on
			my Mac IIci (which has QD32 in ROM) under System 6.04. So the new
			test is slightly more conservative than the former.
4/17/93	dgp	Corrected capitalization of "VideoToolbox.h" so that
			THINK C will use the pre-compiled header.
*/

#include "VideoToolbox.h"
static void Exists(void);		/* Internal routine to do the work, once.  */
static void OldExists(void);	/* Handle Systems 4.1 to 6.03  */

static Boolean firstTime=1,qD8Exists=0,qD32Exists=0,newPaletteManager=0;

Boolean QD32Exists(void)		/* Does 32-bit quickdraw exist? */
{
	if(firstTime)Exists();
	return qD32Exists;
}

Boolean QD8Exists(void)			/* Does color quickdraw exist? */
{
	if(firstTime)Exists();
	return qD8Exists;
}

Boolean NewPaletteManager(void)	/* Does it exist? */
{
	if(firstTime)Exists();
	return newPaletteManager;
}

static void Exists(void)		/* Internal routine to do the work, once.  */
{
	OSErr error;
	long version;

	firstTime=0;
	error=Gestalt(gestaltVersion,&version);	/* Is Gestalt available? */
	if(error){
		OldExists();						/* No. Try SysEnvirons(). */
		return;
	}
	Gestalt(gestaltQuickdrawVersion,&version);
	qD8Exists = version>=gestalt8BitQD;
	qD32Exists = version>=gestalt32BitQD;
	Gestalt(gestaltSystemVersion,&version);
	newPaletteManager = version>=0x605;
}

#define missingTrap 0xA89F
#define QD32Trap	0xAB03

static void OldExists(void)	/* Handle Systems 4.1 to 6.03  */
{
	OSErr error;
	SysEnvRec theWorld;
	
	firstTime=0;
	error=SysEnvirons(1,&theWorld);		/* Is SysEnvirons available? */
	if(error)return;					/* No. */
	qD8Exists=theWorld.hasColorQD;
	if(qD8Exists){
		if(NGetTrapAddress(QD32Trap,ToolTrap) != NGetTrapAddress(missingTrap,ToolTrap))
			qD32Exists=1;
		newPaletteManager=theWorld.systemVersion>=0x605;
	}
}
