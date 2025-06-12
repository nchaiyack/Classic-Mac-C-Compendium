typedef Boolean (*NameFunc)(const char *name);
typedef short (*HeaderMunge)(char *header);

Boolean StartNNTP (void);
void EndNNTP (void);
Boolean GetGroupNames (unsigned long lastTime, NameFunc func);
short GetArticle (char *groupName, char *article,
	Handle *text, long *textLength);
Boolean PostArticle (char *text, unsigned short textLength);
short GetGroupArticleRange (TGroup *theGroup);
Boolean GetGroupArrayArticleRanges (TGroup **groupArray, short numGroups);
short GetHeaders (char *groupName, char *headerName, long first, long last, 
	Handle strings, long *nextStringOffset, HeaderMunge func, short maxStringLen,
	THeader ***headers, short *numHeaders);
short SearchHeaders (char *groupName, char *headerName, long first, long last,
	char *pattern, THeader ***headers, short *numHeaders);
void CloseIdleNewsConnection (Boolean destroy);
void GetServerInfo (void);
