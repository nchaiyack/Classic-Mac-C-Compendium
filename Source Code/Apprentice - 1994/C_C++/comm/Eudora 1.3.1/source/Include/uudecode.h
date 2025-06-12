typedef enum {NotAb, AbHeader, AbFinfo, AbName, AbResFork, AbDataFork,
				AbSkip, AbExcess, AbDone, AbSLimit} AbStates;

short SaveAbomination(UPtr text, long size);
Boolean IsAbLine(UPtr text, long size);

