/* ==========================================

	str_utl.proto.h
	
	Copyright (c) 1993,1994 Newport Software Development
	
	You may distribute unmodified copies of this file for
	noncommercial purposes.  You may use this file as a
	reference when writing your own nShell(tm) commands.
	
	All other rights are reserved.
	
   ========================================== */

int	 cStrEqual( char *p, char *q );
void cStrFromNum( char *p, int width, long num );
int	 cStrLen( char *p );
void pStrAppendC( StringPtr dst, char *src );
void pStrAppend(StringPtr dst, StringPtr new);
void pStrCopy(StringPtr dst, StringPtr src);
int  pStrEqual( StringPtr one, StringPtr two );
void pStrFromC(StringPtr dst, char *src);
