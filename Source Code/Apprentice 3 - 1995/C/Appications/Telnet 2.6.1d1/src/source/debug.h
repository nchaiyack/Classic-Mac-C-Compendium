extern void putln(char *DebugString);
void InitDebug(void);
Boolean	DebugKeys(Boolean cmddwn, unsigned char ascii, short s);
void	ShowDebugWindow(void);

// Don't undef this if compiling for PPC!!!!!!!! Yeah, sure Audit works w/PPC! ;)
//#define SUPPORT_AUDIT
