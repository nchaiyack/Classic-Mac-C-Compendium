extern	long			oldStartPtr;
extern	long			startPtr;
extern	long			listPtr;
extern	Str255			thisWord;

void DoComputerPlayer(void);
char RandomChoice(void);
char RandomChoiceButRealWord(void);
char SmartChoice(void);
void FindStartPtr(Boolean);
Boolean FindOneOccurrence(void);
void SearchList(void);
void GetNextWord(Boolean);
Boolean MatchWord(void);
