#include "PStrings.h"

/*
*
*		PStrings
*   hueras@world.std.com (Jon Hueras)
*		
*			This is a set of routines for manipulating Pascal strings. The routines are
*			designed not only to be (reasonably) fast, but also robust when presented
*			with unusual arguments.
*			
*			In those routines that reference substrings by index (indices begin at one,
*			by the way) and/or length, there is the possibility that the specified
*			substring is not wholly contained in the target string. Take, for example,
*			the string S = "abc". DeleteSubStr(S, 2, 3) is not literally possible here.
*			Likewise, DeleteSubStr(S, -5, 10). However, both of these calls are valid.
*			
*			What happens is that the target string is treated as though it were extended
*			indefinitely from either end for the purpose of determining the start and
*			end points of the substring. Then the arguments are "pinned" to values that
*			actually fall within the target string's boundaries.
*			
*			Thus, DeleteSubStr(S, 2, 3) would behave the same as DeleteSubStr(S, 2, 2)
*			and DeleteSubStr(S, -5, 10) would behave the same as DeleteSubStr(S, 1, 3).
*			More interesting is that DeleteSubStr(S, -5, 7) would behave the same as
*			DeleteSubStr(S, 1, 1) (why this is true is left as an exercise to the
*			reader).
*			
*			All operations assume that any string variable that is modified is capable
*			of containing 255 characters (i.e., Str255). If the result of an operation
*			would result in overflow, the result is truncated. If you wish to modify a
*			smaller string but cannot be certain the result will not overflow, you
*			should work with a Str255 temporary variable (if necessary) and use
*			CopySubStr to truncate the final result.
*		
*		CopyStr(SrcStr, DstStr)
*		
*			A simple string to string copy from SrcSrt to DstStr.
*		
*		AppendStr(SrcStr, DstStr)
*		
*			A copy of SrcStr is appended to the end of DstStr.
*		
*		EqualStr(Str1, Str2, CaseSens)
*		
*			Tests whether Str1 and Str2 are the same length and contain the same sequence
*			of characters. CaseSens determines whether upper- and lower-case alphabetics
*			are considered equivalent.
*		
*		FindSubStr(Pat, Str)
*		
*			Searches Str for an occurrence of Pat. If found, the index of the first
*			matching character in Str is returned, otherwise zero is returned. Note that
*			if Pat is a null string or has a length greater than that of Str, then zero is
*			necessarily returned.
*		
*		CopySubStr(SrcStr, Index, Count, DstStr)
*		
*			Copies the specified substring within SrcStr to DstStr.
*		
*		EqualSubStr(SrcStr, Index, Count, CompStr, CaseSens)
*		
*			Tests whether the specified substring in SrcStr and CompStr are the same
*			length and contain the same sequence of characters. CaseSens determines
*			whether upper- and lower-case alphabetics are considered equivalent.
*		
*		InsertSubStr(SrcStr, DstStr, Index)
*		
*			Inserts SrcStr into DstStr at the given index position in DstStr.
*		
*		DeleteSubStr(Str, Index, Count)
*		
*			Deletes the specifies substring within Str.
*		
*		ReplaceSubStr(Pat, SrcStr, DstStr)
*		
*			Literally does a FindSubStr(Pat, DstStr) and, if Pat is found, deletes Pat
*			from DstStr and inserts SrcStr at the same position.
*
*/

pascal void CopyStr(StringPtr	SrcStr,
										StringPtr	DstStr)
	{
		BlockMove(SrcStr, DstStr, *SrcStr+1);
	}

pascal void AppendStr(register StringPtr	SrcStr,
											register StringPtr	DstStr)
	{
		register int	SrcStrLen = *SrcStr;
		register int	DstStrLen = *DstStr;
		
		if (DstStrLen + SrcStrLen > 255)
			SrcStrLen = 255 - DstStrLen;
		
		if (SrcStrLen) {
			BlockMove(SrcStr+1, DstStr+1+DstStrLen, SrcStrLen);
			*DstStr += SrcStrLen;
		}
	}

static Boolean EqualChars(register unsigned char	*CharPtr1, 
													register unsigned char	*CharPtr2,
													register int						NumChars,
													Boolean									CaseSens)
	{
		register unsigned char Char1, Char2;
		
		if (!CaseSens)
			while (--NumChars >= 0) {
				
				Char1 = *CharPtr1++;
				
				if (Char1 >= 'a' && Char1 <= 'z')
					Char1 -= 'a' - 'A';
					
				Char2 = *CharPtr2++;
					
				if (Char2 >= 'a' && Char2 <= 'z')
					Char2 -= 'a' - 'A';
					
				if (Char1 != Char2)
					return(false);
			
			}
		else
			while (--NumChars >= 0)
				if (*CharPtr1++ != *CharPtr2++)
					return(false);
		
		return(true);
	}

pascal Boolean EqualStr(StringPtr	Str1,
												StringPtr	Str2,
												Boolean		CaseSens)
	{
		if (*Str1 != *Str2)
			return(false);
		
		return(EqualChars(Str1+1, Str2+1, *Str1, CaseSens));
	}

pascal int FindSubStr(StringPtr	Pat,
											StringPtr	Str)
	{
		register StringPtr	p, s;
		register int				pl, sl;
		register char				fc;
		
		p = Pat;
		s = Str;
		
		pl = *p++;
		sl = *s++;
		
		if ((sl -= pl - 1) <= 0)
			return(0);
		
		if (--pl < 0)
			return(0);
		
		fc = *p++;
		
		while (--sl >= 0) {
			if (*s++ != fc)
				continue;
			if (pl && !EqualChars(p, s, pl, true))
				continue;
			return(s - Str - 1);
		}
		
		return(0);
	}

pascal void CopySubStr(register StringPtr	SrcStr,
											 register int				Index1,
											 register int				Count,
											 register StringPtr	DstStr)
	{
		register int	Index2, SrcLen = *SrcStr;
		
		Index2 = Index1 + Count;
				
		if (Index1 < 1)
			Index1 = 1;
		
		if (Index2 > SrcLen + 1)
			Index2 = SrcLen + 1;
		
		if ((Count = Index2 - Index1) <= 0) {
			*DstStr = 0;
			return;
		}
		
		*DstStr++ = Count;
		
		BlockMove(SrcStr+Index1, DstStr, Count);
	}

pascal Boolean EqualSubStr(register StringPtr	SrcStr,
													 register int				Index1,
													 register int				Count,
													 register StringPtr	CompStr,
													 Boolean						CaseSens)
	{
		register int	Index2, SrcLen = *SrcStr;
		
		Index2 = Index1 + Count;
				
		if (Index1 < 1)
			Index1 = 1;
		
		if (Index2 > SrcLen + 1)
			Index2 = SrcLen + 1;
		
		if ((Count = Index2 - Index1) <= 0)
			return(!*CompStr);
		
		return(EqualChars(SrcStr+Index1, CompStr+1, Count, CaseSens));
	}

pascal void InsertSubStr(register StringPtr	SrcStr,
												 register StringPtr	DstStr,
												 register int				Index1)
	{
		register int	SrcLen = *SrcStr;
		register int	DstLen = *DstStr;
		register int	Index2, Index3;
		
		if (!SrcLen)
			return;
		
		if (Index1 < 1)
			Index1 = 1;
		
		if (Index1 > DstLen + 1)
			Index1 = DstLen + 1;
				
		if ((Index2 = Index1 + SrcLen) > 256)
			Index2 = 256;
		
		if ((Index3 = DstLen + SrcLen + 1) > 256)
			Index3 = 256;
		
		*DstStr = Index3 - 1;
		
		BlockMove(DstStr+Index1, DstStr+Index2, Index3-Index2);
		BlockMove(SrcStr+1, DstStr+Index1, Index2-Index1);
	}

pascal void DeleteSubStr(register StringPtr	Str,
												 register int				Index1,
												 register int				Count)
	{
		register int	Index2, StrLen = *Str;
		
		Index2 = Index1 + Count;
				
		if (Index1 < 1)
			Index1 = 1;
		
		if (Index2 > StrLen + 1)
			Index2 = StrLen + 1;
		
		if ((Count = Index2 - Index1) <= 0)
			return;
		
		*Str -= Count;
		
		BlockMove(Str+Index2, Str+Index1, StrLen-Index2+1);
	}

pascal int ReplaceSubStr(StringPtr	Pat,
												 StringPtr	SrcStr,
												 StringPtr	DstStr)
	{
		register int i;
		
		if (i = FindSubStr(Pat, DstStr)) {
			DeleteSubStr(DstStr, i, *Pat);
			InsertSubStr(SrcStr, DstStr, i);
		}
		
		return(i);
	}
