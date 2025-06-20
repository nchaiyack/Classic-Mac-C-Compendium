#include	<StdIO.h>
#include	"config.h"
#include	"Progress.h"

#ifdef MAC_GWADA
#include	"ANSIFabLib.h"
#endif

#include	"adafront.h"

#ifdef MAC_GWADA
#include "ShellInterface.h"

// a faster, low-CPU progress routine [Fabrizio Oddone]

void Progress(void)
{
static unsigned long	oldticks = 0UL;

if (TickCount() > oldticks + 40) {
	ProgressBar((gSrcFileP->Current - gSrcFileP->MyBuffer) * kProgressBarWidth / gSrcFileP->FileSize);
	oldticks = TickCount();
	}
}
#endif
