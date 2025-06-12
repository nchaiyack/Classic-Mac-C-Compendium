Boolean FindHeader (Handle text, const char *hdr, char *contents, 
	short maxLength);

short PrettifyName (char *from);

void GetAuthorFromHeader (Handle text, char *author);
long FindBody (Handle text);
void DoShowHideHeader (WindowPtr wind);
