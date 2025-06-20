/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * Declarations for progress monitoring
 ************************************************************************/
typedef enum ProgressEnum {NoBar = -2, NoChange = -1};
int OpenProgress(void);
void Progress(short percent,char *message);
void CloseProgress(void);
void ByteProgress(UPtr message, int onLine, int totLines);
void DisableProgress(void);
void EnableProgress(void);
void PushProgress(void);
void PopProgress(Boolean messageOnly);
void SetProgressN(short n);
Boolean ProgressIsOpen(void);