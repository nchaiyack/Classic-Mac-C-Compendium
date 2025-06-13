#ifdef MSDOS
#include <stdlib.h>
#include <io.h>
#endif /* MSDOS */

#ifdef USG
#include <string.h>
#else /* not USG */
#ifdef MSDOS
#include <string.h>
#else
#ifdef THINK_C
#include <string.h>
#else /* not THINK_C */
#include <strings.h>
#endif /* not THINK_C */
#endif /* not MSDOS */
#endif /* not USG */
