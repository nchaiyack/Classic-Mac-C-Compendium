/* Audit.h */

#pragma once

/* to use this audit-trail generator, include this macro in the prefix */
/* #define AUDIT  defines the audit macros */

#ifdef AUDIT
	void	INITAUDIT(void);
	void	ENDAUDIT(void);
	void	AuditPrint(char* Str,...);
	void	AHEXDUMP(char* Ptr, long NumBytes);
	#define APRINT(param) AuditPrint param
#else
	#define INITAUDIT(param)
	#define ENDAUDIT(param)
	#define APRINT(param)
	#define AHEXDUMP(param1,param2)
#endif
