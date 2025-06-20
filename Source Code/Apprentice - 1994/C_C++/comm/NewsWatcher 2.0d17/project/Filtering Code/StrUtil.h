/*
 * StrUtil.h
 * String utility functions
 */

void	pstrncpy(StringPtr to, ConstStr255Param from, short maxLen);
void	pstrncat(StringPtr to, ConstStr255Param from, short maxLen);

OSErr	ReplaceStr(StringPtr str, ConstStr255Param old, 
						ConstStr255Param new, short maxLen);

OSErr	ParamString(unsigned char *str, ConstStr255Param r0, ConstStr255Param r1,
					ConstStr255Param r2, ConstStr255Param r3, short maxLen);
