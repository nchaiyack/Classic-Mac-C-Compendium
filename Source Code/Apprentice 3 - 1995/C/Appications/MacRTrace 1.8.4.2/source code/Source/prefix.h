#ifdef THINK_C
	#include <MacHeaders>
#endif

#ifdef __MWERKS__
	#if __powerc
		#include <MacHeadersPPC>
	#else
		#include <MacHeaders68K>
	#endif
#endif

#define _NOERRORCHECK_ 1
#define __MAC_RTRACE__
