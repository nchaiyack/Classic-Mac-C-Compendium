#include <sys/param.h>

char *getwd(char *path)
	{
	return (char *)getcwd(path, MAXPATHLEN);
	}
