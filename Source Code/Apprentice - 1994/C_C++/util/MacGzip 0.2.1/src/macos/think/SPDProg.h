#define _SPD_PROG_

#define	kCalCursorRes	128

#define kTicksCursor	20

#define	kNiceBack	256


extern Boolean InitAnimatedCursors(short acurID);
extern void ReleaseAnimatedCursors();

extern void InitMovableModal(unsigned long int);
extern void ReleaseMovableModal(void);

extern Boolean	SPDSystemTask();

extern Boolean CanBreak;

extern unsigned long int	SPDBackCycles;
extern unsigned long int	SPDEnd, SPDNow;

extern	char*	SPDpstr;

#define	SPDBackSysTask(a,b) { if ((SPDBackCycles=(SPDBackCycles+1)%(a))==1) (b)=SPDSystemTask(); }