/*										Audit.h									*/
/*
 * Audit.h
 * Copyright � 1992-93 Apple Computer Inc. All Rights Reserved.
 * Programmed by Martin Minow,
 *	Internet:	minow@apple.com
 *	AppleLink:	MINOW
 * Version of January 14, 1993
 *
 * Edit History
 *	93.01.09 MM		First public release
 *	93.01.14 MM		Think and MPW generate different record sizes; a disaster if
 *					you create an Audit Record under Think and call Audit compiled
 *					under MPW. Also added a test for record sizes and included
 *					record size information in the AuditRecord.
 *	93.07.09 MM		Conversion to Think C 6.0, text reformatted for standard
 *					page layout, no substantive changes 
 *	93.08.14 MM		Added "logicalRAMSize to the AuditRecord. This protects
 *					Audit (somewhat) against bad data forcing address errors.
 *
 * This file contains the unique definitions used for driver and code-segment
 * logging.
 *
 * Note: because Pascal does not support variable-length calling sequences,
 * the functions are C-format only.
 */
#ifndef __Audit__
#define __Audit__

#include <stdarg.h>
#include <OSUtils.h>
#include <Processes.h>

/*
 * To create an audit record, a driver, application, or other code segment calls
 * InitAudit(). This is the only function that accesses the memory manager. The
 * audit record is created on the System Heap.
 *
 * To obtain a reference to an audit record, a code segment calls GetAuditPtr().
 * This may be called at any time.
 *
 * Both InitAudit and GetAuditPtr are moderately inefficient (they call Gestalt()).
 * Thus, a driver (etc.) should store the value in a private variable. For example,
 * a driver would typically store the value in a variable in its driver control
 * block. The Audit Record never moves in memory.
 *
 * Once an audit area has been created for a particular Gestalt selector,
 * it persists in its current form until the computer is re-booted.
 *
 * The display application processes log entries as follows. Note
 * that all functions may be safely called with a NULL auditPtr:
 *
 *		gAuditPtr = GetAuditPtr(kMyAuditSelector);
 *		GetCurrentProcess(&gOldLogProcess);
 *		WakeUpAudit(gAuditPtr, &gOldProcess);
 *		wasLogging = EnableAudit(gAuditPtr, TRUE);
 *		while (gQuitNow == FALSE) {
 *			ProcessOneEvent();
 *			for (i = 0; i < 100; i++) {
 *				if (ReadAudit(gAuditPtr, &missed, &logEntry) == FALSE)
 *					break;
 *				else {
 *					Str255		timeText, dataText;
 *
 *					FormatAuditEntryTimestamp(gAuditPtr, &logEntry, timeText);
 *					FormatAuditEntryData(&ENTRY, dataText);
 *					DisplayString(timeText);
 *					DisplayString(dataText);
 *				}
 *			}
 *		}
 *		EnableAudit(gAuditPtr, wasLogging);
 *		WakeUpCurrentProcess(&gOldLogProcess);
 *		ExitToShell();
 */

/*
 * Each audit record entry contains the following information:
 *		tickCount		The Ticks value at the time the data was collected.
 *		lostData		The number of audit records that were not stored before
 *						this one because Audit was called when there were no free
 *						records available.
 *		idCode			A longword that uniquely identifies the log entry (i.e.,
 *						who logged it). This is provided by the Audit caller.
 *		format			A longword that describes the format of the log data.
 *		data[8]			eight longwords that contain the entry-unique information.
 *						The actual conte is defined by the format longword.
 * TickCount and lostData are maintained by the Audit library, while the other
 * parameters are copied from the Audit parameters. Each entry is time-stamped by
 * the following algorithm:
 *		elapsedTicks = RECORD.tickCount - LOG.ticksAtStart;
 *		SecsToDate(
 *			LOG.timeAtStart + elapsedTicks / 60,
 *			&logEntryDateString
 *		);
 *		printf(, ..., date.second, elapsedTicks % 60);
 * AuditRead returns a copy of the current audit record. See AuditEntryFormat.c
 * and AuditDCMD.c for record formatting examples. Note that, because of the way
 * formatting data is stored, changing the size of data[] is very difficult --
 * and not recommended.
 */
typedef struct AuditEntry {
	unsigned long			tickCount;	/* TickCount() at Audit call			*/
	unsigned long			lostData;	/* Missing records before this one		*/
	OSType					idCode;		/* Why are we logging -- set by caller	*/
	unsigned long			format;		/* Format of the data (see below)		*/
	unsigned long			data[8];	/* The data itself						*/
} AuditEntry, *AuditEntryPtr;
/*
 * Hand-compute the size to make sure that compiler quirks don't mess us up.
 * We check that kSizeofAuditEntry equals sizeof (AuditEntry). The ANSI C
 * Standard does not permit "sizeof" in a #define statement.
 */
#define kSizeofAuditEntry ((4 * 4) + (4 * 8))


/*
 * AuditQueueEntry is a private structure (only Audit.c and AuditDCMD.c reference
 * it). It contains the Audit entry record and a queue element pointer that
 * connects this record with other records in the free or to-be-processed queues.
 */
typedef struct AuditQueueEntry {
	QElemPtr				qLink;		/* Queue linkage		*/
	AuditEntry				theEntry;	/* User's data area		*/
} AuditQueueEntry, *AuditQueueEntryPtr;
#define kSizeofAuditQueueEntry (kSizeofAuditEntry + 4)

/*
 * The audit record contains a fixed header with the following information:
 *
 *		lowVersion		The earliest version of the AuditRecord that the library
 *						that created the record understands.
 *		highVersion		The latest version of the AuditRecord that the library
 *						that created the record understands.
 *		recordSize		This longword encodes the size of the AuditRecord and
 *						AuditQueueEntry to ensure that compiler alignment
 *						considerations do not cause code to crash.
 *		free.queue		An O.S. queue with available log records.
 *		data.queue		An O.S. queue with busy log records.
 *		lostData		The number of records not logged. This is cleared whenever
 *						Audit successfully stores a caller's request.
 *		PSN				The ProcessSerialNumber of the process to awaken when
 *						something is logged.
 *		refNum			A longword that is available for your use.
 *		logEnabled		TRUE if logging.
 *		timeAtStart		GetDateTime() when the log was created.
 *		ticksAtStart	TickCount() when the log was created.
 * User software accesses an audit record by calling Audit functions. This is
 * necessary so that the record can be accessed by interrupt-level routines
 * without risk of obtaining inconsistent data.
 *
 * Note: the QHdr structure is not longword aligned. Some compilers add an extra
 * 16-bit word padding. If an AuditRecord is created by code generated by one
 * compiler, and accessed by code generated by the other compiler, disaster ensues.
 * To prevent this, the QHdr structures are encapsulated in a dummy structure that
 * is longword aligned. This is ugly, but the alternative is uglier.
 */
typedef struct AuditRecord {
	union {
	  void				*unusedLong;	/* Force longword align	*/
	  struct {
		unsigned short	low;			/* Earliest lib version	*/
		unsigned short	high;			/* Latest lib version	*/
	  } u;
	} version;
	unsigned long		recordSize;		/* Compiler check		*/
	unsigned long		lostData;		/* Missed log counter	*/
	void				*refNum;		/* User-controlled long	*/
	unsigned long		flags;			/* Logging & lost data	*/
	unsigned long		timeAtStart;	/* GetDateTime()		*/
	unsigned long		ticksAtStart;	/* TickCount()			*/
	ProcessSerialNumber	PSN;			/* Wakeup this process	*/
	unsigned long		logicalRAMSize;	/* From Gestalt			*/
	union {
		QHdr			queue;			/* Free queue header	*/
		long			unused[
			(sizeof (QHdr) + (sizeof (long) - 1)) / sizeof (long)
		];
	} free;
		union {
		QHdr			queue;			/* Data queue header	*/
		long			unused[
			(sizeof (QHdr) + (sizeof (long) - 1)) / sizeof (long)
		];
	} data;
	AuditQueueEntry		entries[1];		/* Entries stored here	*/
} AuditRecord, *AuditPtr;
#define kSizeofAuditRecord (										\
		+ (8 * 4)					/* Various longwords		*/	\
		+ (12 * 2)					/* sizeof QHdr's			*/	\
		+ sizeof (ProcessSerialNumber)								\
		+ kSizeofAuditQueueEntry									\
	)
#define kAuditRecordSize ((unsigned long) (							\
		((sizeof (AuditRecord) - sizeof (AuditQueueEntry)) << 16L)	\
		| sizeof (AuditQueueEntry)									\
	))

/*
 * Values for the flags variable in the AuditRecord. These are private to the
 * Audit library and dcmd display routine.
 */
#define kAuditEnabledMask		1
#define kAuditPreserveFirstMask	2

/*
 * These flags control data formatting. Note: because data is passed using
 * variable-length argument list conventions and we want the code to work
 * compatibly on both Think and MPW, all numeric parameters must be passed as
 * "long" or "unsigned long." For example, to pass an OSErr code, do
 *		Audit(
 *			auditPtr,
 *			'fubr',
 *			AuditFormat1(kAuditFormatSigned),
 *			(signed long) statusCode
 *		);
 */
enum {
	/*
	 * Parameters to the AuditFormat macro.
	 */
	kAuditFormatSigned			= 0,		/* Signed long			*/
	kAuditFormatUnsigned		= 1,		/* Unsigned (decimal)	*/
	kAuditFormatHex				= 2,		/* Unsigned (hex/char)	*/
	kAuditFormatAddress			= 3,		/* Unsigned hex only	*/
	kAuditFormatReserved		= 4,		/* Unused				*/
	kAuditFormatLocation		= 5,		/* Location (last fmt)	*/
	kAuditFormatString			= 6,		/* String (last format)	*/
	kAuditFormatEnd				= 7,		/* End signal (no data)	*/
	/*
	 * kAuditFormatShift must be large enough to shift all format codes. All data
	 * format codes must fit into a single longword.
	 */
	kAuditFormatShift		= 3,
	kAuditFormatMask		= (1 << kAuditFormatShift) - 1
};

/*
 * The AuditFormat macro stores the format word.
 */
#define AuditFormat(f0, f1, f2, f3, f4, f5, f6, f7)	\
	(  (f0)											\
	 | ((f1) << (kAuditFormatShift * 1))			\
	 | ((f2) << (kAuditFormatShift * 2))			\
	 | ((f3) << (kAuditFormatShift * 3))			\
	 | ((f4) << (kAuditFormatShift * 4))			\
	 | ((f5) << (kAuditFormatShift * 5))			\
	 | ((f6) << (kAuditFormatShift * 6))			\
	 | ((f7) << (kAuditFormatShift * 7))			\
	 | (kAuditFormatEnd << (kAuditFormatShift * 8))	\
	)
#define AuditFormat1(f0)													\
	AuditFormat(															\
		(f0),          kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd,	\
		kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd	\
	)
#define AuditFormat2(f0, f1)												\
	AuditFormat(															\
		(f0),          (f1),          kAuditFormatEnd, kAuditFormatEnd,		\
		kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd	\
	)
#define AuditFormat3(f0, f1, f2)											\
	AuditFormat(															\
		(f0),          (f1),          (f2),          kAuditFormatEnd,		\
		kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd	\
	)
#define AuditFormat4(f0, f1, f2, f3)										\
	AuditFormat(															\
		(f0),          (f1),          (f2),          (f3),					\
		kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd	\
	)
#define AuditFormat5(f0, f1, f2, f3, f4)									\
	AuditFormat(															\
		(f0),          (f1),          (f2),          (f3),					\
		(f4),          kAuditFormatEnd, kAuditFormatEnd, kAuditFormatEnd	\
	)
#define AuditFormat6(f0, f1, f2, f3, f4, f5)								\
	AuditFormat(															\
		(f0),          (f1),          (f2),          (f3),					\
		(f4),          (f5),          kAuditFormatEnd, kAuditFormatEnd		\
	)
#define AuditFormat7(f0, f1, f2, f3, f4, f5, f6)							\
	AuditFormat(															\
		(f0),          (f1),          (f2),          (f3),					\
		(f4),          (f5),          (f6),          kAuditFormatEnd		\
	)
#define AuditFormat8(f0, f1, f2, f3, f4, f5, f6, f7)						\
	AuditFormat(															\
		(f0),          (f1),          (f2),          (f3),					\
		(f4),          (f5),          (f6),          (f7)					\
	)

/*
 * The driver or code segment calls the following functions to access audit
 * records. InitAudit returns NULL if it couldn't create an audit record. It must
 * be called when memory allocation is permitted (i.e. from an application, init,
 * or a driver open routine).
 *
 * All functions that take an AuditPtr argument are "NULL-safe:" if called with a
 * null parameter, they do nothing (gracefully).
 *
 *		gestaltSelector		Used to identify this audit record to the display
 *							application.
 *		nEntries			The number of audit entries. Each audit entry consumes
 *							52 bytes of non-relocatable System Heap space.
 *		initiallyEnabled	TRUE to start logging upon creation. The EnableAudit
 *							function allows modifying this parameter.
 *		preserveFirst		If TRUE and the entry area fills, preserve the first
 *							nEntries, losing the newest entry. If FALSE and the
 *							entry area fills, delete the earliest entry, preserving
 *							the newest entry. The PreserveAudit function allows
 *							modifying this parameter.
 * If some other program has already created an audit record with this
 * gestaltSelector, the function returns a pointer to that log record. This is not
 * necessarily an error. For example, it allows a driver to locate a log record
 * that had been created by an init routine. If the init routine did not run,
 * the driver will create the record. This may be useful for developers who are
 * trying to track down obscure "happens only at the customer site" bugs.
 */
AuditPtr					InitAudit(
		OSType					gestaltSelector,	/* Gestalt to create		*/
		unsigned short			nEntries,			/* Number of audit entries	*/
		Boolean					initiallyEnabled,	/* Start auditing now?		*/
		Boolean					preserveFirst		/* Save earlier entries?	*/
	);

/*
 * This function writes an audit entry if auditing is enabled.
 *
 *		auditPtr			As returned by AuditInit. If NULL, nothing is logged.
 *		idCode				A user-controlled value, by convention an OSType
 *							(4-byte character) that identifies this entry. The
 *							display application prints it. Note that, on the
 *							Macintosh, an OSType can be coerced from/to any
 *							longword scalar (such as an address) without loss of
 *							data. This may be useful for user-written display
 *							applications.
 *		format				A longword that specifies the format of the remaining
 *							data. Use the value AuditFormat1(kAuditFormatString)
 *							if the only datum is a pascal string, otherwise, use
 *							the AuditFormat macro to create the value.
 *		...					Additional data as needed. Note that all data must be
 *							specified as longwords (StringPtr, address, or long).
 *							Naturally short integers such as Booleans or OSErr
 *							codes must be explicitly cast to long. This is needed
 *							because of differences between Think C and MPW
 *							compilers.
 */
void						Audit(
		AuditPtr				auditPtr,	/* Audit Record Pointer				*/
		OSType					idCode,		/* User-specified identifier		*/
		unsigned long			format,		/* Format bits						*/
		...									/* Additional data, if any			*/
	);

/*
 * AuditString calls Audit with a single Pascal string argument
 */
#define AuditString(auditPtr, idCode, string) (			\
		Audit(											\
			(auditPtr),									\
			(idCode),									\
			AuditFormat1(kAuditFormatString),			\
			string										\
		)												\
	)
 
/*
 * AuditStatusString calls Audit with the status and string values.
 */
#define AuditStatusString(auditPtr, idCode, status, string) ( \
		Audit(											\
			(auditPtr),									\
			(idCode),									\
			AuditFormat2(kAuditFormatSigned, kAuditFormatString), \
			(signed long) (status),						\
			string										\
		)												\
	)
/*
 * AuditStatusLocation calls Audit with the function location.
 */
#define AuditStatusLocation(auditPtr, idCode, status) (	\
		Audit(											\
			(auditPtr),									\
			(idCode),									\
			AuditFormat2(kAuditFormatSigned, kAuditFormatLocation), \
			(signed long) (status)						\
		)												\
	)

/*
 * Return a pointer to the log area, or NULL if there is none. Note: this can be
 * called at any time (even from a driver completion routine), so exception
 * logging can take place (somewhat inefficiently) even if the AuditPtr is not
 * stored permanently.
 */
AuditPtr					GetAuditPtr(
		OSType					gestaltSelector
	);

/*
 * Awaken a specified process when data is stored in the audit record. Call by the
 * following sequence:
 *		GetCurrentProcess(&oldPSN);		// process to awaken
 *		WakeUpAudit(auditPtr, &oldPSN);
 *	The previous process is now stored in oldPSN)
 *		... display the log ...
 *		WakeUpAudit(auditPtr, &oldPSN);	// restore old
 *		ExitToShell();
 * This is normally only called by the display application.
 */
void						WakeUpAudit(
		AuditPtr				auditPtr,
		ProcessSerialNumber		*oldPSN
	);

/*
 * Enable/disable audit logging. Returns the old logging state.
 */
Boolean						EnableAudit(
		AuditPtr				auditPtr,
		Boolean					enableLogging
	);

/*
 * Return the value of the Audit enable flag. Returns FALSE if auditPtr is NULL or
 * auditing is disabled.
 */
Boolean						IsAuditEnabled(
		AuditPtr				auditPtr
	);

/*
 * Set the preserveAudit flag. Returns the old flag value.
 */
Boolean						PreserveAudit(
		AuditPtr				auditPtr,
		Boolean					preserveFirst
	);

/*
 * Get the time that the log record was created. This is used to time-stamp log
 * entries.
 */
void						GetAuditStartTimes(
		AuditPtr				auditPtr,
		unsigned long			*timeAtStart,
		unsigned long			*ticksAtStart
	);

/*
 * Read the next audit entry. This returns a copy of the entry, if one is
 * available, and returns TRUE. This function manages all log queues. ReadAudit
 * returns FALSE if no entry is available or auditPtr is NULL.
 */
Boolean						ReadAudit(
		AuditPtr				auditPtr,
		AuditEntryPtr			thisLogEntry
	);

/*
 * Store a user-controlled reference value. This may be coerced to any scalar
 * value (such as a memory pointer or longword). SetAuditRefNum returns the
 * previous value of the refNum, or NULL if no value had been stored.
 */
void						*SetAuditRefNum(
		AuditPtr				auditPtr,
		void					*refNum
	);

/*
 * Return the current user-controlled reference value. This may be coerced to any
 * scalar value (such as a memory pointer or longword). This returns zero if
 * auditPtr is NULL or no value had been stored.
 */
void						*GetAuditRefNum(
		AuditPtr				auditPtr
	);

/*
 * This function is in AuditEntryFormat.c - it formats an entry into a single line
 * that is stored in the result. Note: only the data is formatted: the timestamp
 * is not processed.
 */
void						FormatAuditEntryData(
		register AuditEntryPtr	entryPtr,
		StringPtr				result
	);

/*
 * This function is in AuditEntryFormat.c - it formats the timestamp into
 * yyyy.mm.dd hh.mm.ss.msec format. Audit does not use the built-in date
 * formatting routines.
 */
void						FormatAuditEntryTimestamp(
		register AuditPtr		auditPtr,
		register AuditEntryPtr	entryPtr,
		StringPtr				result
	);


#endif	/* __Audit__	*/

	