#ifndef _PStrings_
#define _PStrings_

extern pascal void		CopyStr(StringPtr SrcStr, StringPtr DstStr);
extern pascal void		AppendStr(StringPtr SrcStr, StringPtr DstStr);
extern pascal Boolean	EqualStr(StringPtr Str1, StringPtr Str2, Boolean CaseSens);
extern pascal int			FindSubStr(StringPtr Pat, StringPtr Str);
extern pascal void		CopySubStr(StringPtr SrcStr, int Index, int Count, StringPtr DstStr);
extern pascal Boolean	EqualSubStr(StringPtr SrcStr, int Index, int Count, StringPtr CompStr, Boolean CaseSens);
extern pascal void		InsertSubStr(StringPtr SrcStr, StringPtr DstStr, int Index);
extern pascal void		DeleteSubStr(StringPtr Str, int Index, int Count);
extern pascal int			ReplaceSubStr(StringPtr Pat, StringPtr SrcStr, StringPtr DstStr);

#endif