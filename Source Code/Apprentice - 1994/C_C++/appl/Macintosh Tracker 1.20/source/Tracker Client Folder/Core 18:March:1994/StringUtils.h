/* StringUtils.h */

#pragma once

void				BeginStringOperation(void);
void				EndStringOperation(Handle FinalString);

Handle			ConStr(Handle LeftStr, Handle RightStr);
Handle			MidStr(Handle String, long Start, long NumChars);
Handle			ReplaceStr(Handle String, Handle Key, Handle Replacement);
Handle			CString(char* Start);
Handle			StringOf(char* Start, long Length);
Handle			RegisterString(Handle TheString);
