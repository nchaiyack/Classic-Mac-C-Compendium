/*******************
** logging.h
**
** header file for msg logging module.
********************/

#ifndef LOGGING_H
#define LOGGING_H

#if LOGGING
#define LOGENTRY(x)		LogEntry(x)
#else
#define LOGENTRY(x)
#endif

void LogInit(const char *name);
void LogClose(void);
void LogStop(void);
void LogStart(void);
void LogEntry( const char *s);
void LogError(OSErr, const char *s);

#endif

