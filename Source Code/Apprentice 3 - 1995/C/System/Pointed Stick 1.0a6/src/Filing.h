#ifndef __FILING__
#define __FILING__

extern FSSpec*			gPrefsFile;

void					ReadPrefsFile(void);
pascal void				SaveOnShutDown(void);

#endif