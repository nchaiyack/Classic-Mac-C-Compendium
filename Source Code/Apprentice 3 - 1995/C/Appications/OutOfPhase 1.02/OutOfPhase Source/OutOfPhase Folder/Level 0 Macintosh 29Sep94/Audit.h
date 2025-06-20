/* Audit.h */

#ifndef Included_Audit_h
#define Included_Audit_h

/* Audit module depends on: */
/* MiscInfo.h */
/* Definitions */
/* Debug */

/* to use this audit-trail generator, include this macro in the prefix */
/* #define AUDIT (1)  enables auditing */
/* #define AUDIT (0)  eliminates auditing */
/* in addition, if your program is very crash-prone, you can cause the audit */
/* file to be flushed to disk after every APRINT. */
/* #define AUDITFLUSHING (1)  enables audit flushing */
/* #define AUDITFLUSHING (0)  disables audit flushing */

/* APRINT prints a string in the same way that printf does.  it accepts these options: */
/* %s = decimal signed short */
/* $xs = hexadecimal short */
/* %l = decimal signed long */
/* %xl = hexadecimal long */
/* %b = boolean from short */
/* %c = decimal signed char */
/* %xc = hexadecimal char */
/* %t = C String (text) */
/* %p = Pascal string */
/* %r = Reference (a pointer) */

/* since it's a macro, remember to use DOUBLE parens with APRINT! */

#if AUDIT
	void	ENDAUDIT(void);
	void	AuditPrint(char* Str,...); /* Internal routine only */
	void	AHEXDUMP(char* Ptr, long NumBytes);
	#define APRINT(param) AuditPrint param
#else
	#define ENDAUDIT() ((void)0)
	#define AHEXDUMP(param1,param2) ((void)0)
	#define APRINT(param) ((void)0)
#endif

#endif
