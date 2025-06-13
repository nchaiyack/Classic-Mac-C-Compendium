/* 22may95,ejo: added this */
#include <Types.h>
/* --- ejo --- */

#define _SPD_PROG_

#define	kCalCursorRes	128

#define kTicksCursor	20

#define	kNiceBack	256


extern Boolean InitAnimatedCursors(short acurID);
extern void ReleaseAnimatedCursors(void); /* 22may95,ejo: added void */

extern void InitMovableModal(unsigned long int);
extern void ReleaseMovableModal(void);

extern Boolean	SPDSystemTask(void); /* 22may95,ejo: added void */

extern Boolean CanBreak;

extern unsigned long int	SPDBackCycles;
extern unsigned long int	SPDEnd, SPDNow;

extern	char*	SPDpstr;

#define	SPDBackSysTask(a,b) { if ((SPDBackCycles=(SPDBackCycles+1)%(a))==1) (b)=SPDSystemTask(); }