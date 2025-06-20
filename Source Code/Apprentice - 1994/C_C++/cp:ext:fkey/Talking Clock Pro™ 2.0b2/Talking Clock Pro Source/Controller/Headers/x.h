/*
 * x.h �1993 Jon W�tte (h+@nada.kth.se) All Rights Reserved.
 * Please read the paragraph marked $$$ below before using this
 * source file.
 *
 * v1.0 930826 - Initial Release
 */

/*
 * This try/fail system was inspired by how the Think Class Library
 * exception mechanism works. No code was taken from the TCL, and I have
 * not even studied the source of the TCL exception mechanism.
 *
 * You use it by encapsulating code that might fail in
 *
 * TRY {
 *	<code>
 * } CATCH {
 *	<error handler>
 * } DONE ;
 *
 * If your error handler does not want to keep doing exceptions
 * that this exception might be encapsulated in, it should look
 * something like this:
 * ...
 * } CATCH {
 *	<error handler>
 *	NO_PROPAGATE ;
 * } DONE ;
 *
 * There are cases where you want to do something like restore the state
 * of a handle both when catching failures and when done with the critical
 * section normally - in that case, you can use CLEANUP instead of CATCH:
 *
 *	void
 *	Function ( Handle h ) {
 *
 *	char state = HGetState ( h ) ;
 *
 *		HLock ( h ) ;
 *		TRY {
 *			DoSomethingTo ( * h ) ;
 *		} CLEANUP {
 *			HSetState ( h , state ) ;
 *		} DONE ;
 *
 * The code between CLEANUP and DONE will be executed for both exception
 * cases and normal execution - HOWEVER, an exception will jump to the next
 * upward exception handler at DONE while the normal code will continue.
 * You can modify this behaviour with NO_PROPAGATE ;
 *
 * To trigger an exception, you call FailErr(e) with a non-0 e, or you call
 * FailNil(p) with a NULL p (in which case a -108 error is generated) If you
 * want to know what error you are handling in an error handler, it is stored
 * in the __err variable that is in scope in the whole TRY { } CATCH { } DONE
 * construct. NO_PROPAGATE ; sets this variable to 0.
 *
 * If you #define FAILINFO below, you will additionally have a C string with
 * the name of the offending file in __err_file and an int with the offending
 * line number in __err_line. This option will generate more string data in
 * your program (one string per call of FailErr or FailNil) so you may want to
 * turn it off before release (and write your failure handling routine conditional
 * on FAILINFO)
 *
 * If the outermost failure layer reaches DONE, bad things happen. This is because
 * you are supposed to provide a failure handler there and call NO_PROPAGATE. My
 * application typically looks like:
 *
 *	void
 *	main ( void ) {
 *
 *		InitToolbox ( ) ;
 *		TRY {
 *			InitApp ( ) ;
 *			while ( ! quit ) {
 *				TRY {
 *					ProcessEvent ( ) ;
 *				} CATCH {
 *					ReportError ( __err ) ;
 *					NO_PROPAGATE ;
 *				} DONE ;
 *			}
 *		} CLEANUP {
 *			QuitApp ( ) ;
 *			if ( __err ) {
 *				ReportStartQuitErr ( __err ) ;
 *				ExitToShell ( ) ;
 *			}
 *		} DONE ;
 *	}
 *
 * ReportError might look something like: (NOTE: The use of ParamText is NOT
 * WorldScript compatible!)
 *
 *	static void
 *	ReportError ( short errCode , Boolean toQuit ) {
 *
 *	StringHandle h = GetString ( errCode ) ;
 *	Str15 code ;
 *	StringHandle num ;
 *	Str31 fileStr ;
 *	Str15 lineStr ;
 *
 *		DisposeHandle ( err_safeguard ) ; /* Handle allocated in InitApp to give us some room * /
 *		if ( ! h ) {
 *			h = GetString ( 129 ) ; /* What do we do if this fails !? * /
 *		}
 *		HLock ( ( Handle ) h ) ;
 *		NumToString ( errCode , code ) ;
 *		num = GetString ( 130 ) ;
 *		if ( num ) {
 *			PtrAndHand ( ( Ptr ) & code [ 1 ] , ( Handle ) num , code [ 0 ] ) ;
 *			* * num += code [ 0 ] ;
 *			HLock ( ( Handle ) num ) ;
 *		}
 *
 *	#ifdef FAILINFO
 *		NumToString ( __err_line , lineStr ) ;
 *		CopyC2P ( __err_file , fileStr ) ; /* Make fileStr the Pascal version of __err_file * /
 *	#else
 *		lineStr [ 0 ] = fileStr [ 0 ] = 0 ;
 *	#endif
 *
 *		ParamText ( h ? * h : NULL , num ? * num : NULL , fileStr , lineStr ) ;
 *		if ( num ) {
 *			ReleaseResource ( ( Handle ) num ) ;
 *		}
 *		if ( h ) {
 *			ReleaseResource ( ( Handle ) h ) ;
 *		}
 *		if ( ! InFront ( ) ) {
 *			Notify ( ) ; /* Post notification since we're in the background * /
 *		}
 *		Alert ( 128 , NULL ) ; /* Could use a filterProc here * /
 *		HiliteMenu ( 0L ) ; /* For failures during menu selections * /
 *		ParamText ( NULL , NULL , NULL , NULL ) ;
 *		err_safeguard = NewHandle ( 1024 ) ;
 *	}
 *
 * This exception system is written COMPLETELY in ANSI C. It is also FreeWare
 * in the sense that you may use it, modified or not, in any product you
 * make, as long as you send the author (me, Jon W�tte, h+@nada.kth.se) a
 * copy of any product you make using this exception system. You may *NOT*
 * distribute copies of these files (x.c and x.h) that are changed, renamed
 * or in any other way modified from the originals as released by me.
 *
 * $$$ Also, please note, that I neither CAN nor WILL take any responsibility
 * for your use of this source code; you use it solely AT YOUR OWN RISK, and
 * if your use of this code or any derivative thereof results in damages to
 * you or a third party, you are solely responsible for covering all such
 * damages and costs related thereto. This code is provided by the author free
 * of charge. IF THE ABOVE LIMITATION OF JON W�TTE'S RESPONSIBILITY IS NOT LEGAL
 * WHERE YOU USE OR PUBLISH THIS CODE OR THE RESULTS THEREOF, you are STRICTLY
 * FORBIDDEN to use this code, ON THE PENALTY OF USD $1,000,000,000 (one
 * billion US dollars) or SEK 10,000,000,000 (ten billion Swedish Crowns)
 * whichever is greater, in damages for breach of contract and copyright
 * infringement. I MEAN IT.
 *
 * If you want to fix bugs, CONTACT ME and I will incorporate the bug fix in
 * the next general release.
 *
 */

#pragma once
#ifndef x_h
# define x_h

/* Define FAILINFO to get information about where something went wrong
 * in failure alerts.
 * Comment it out to save global/string data space
 */
# define FAILINFO

# include <setjmp.h>
# include <Types.h>

# ifdef FAILINFO
void __FailNil ( void * , char * , int ) ;
void __FailErr ( OSErr , char * , int ) ;
#  define FailNil(x) __FailNil(x,__FILE__,__LINE__)
#  define FailErr(x) __FailErr(x,__FILE__,__LINE__)
extern char * __err_file ;
extern int __err_line ;
# else
void FailNil ( void * ) ;
void FailErr ( OSErr ) ;
# endif

#ifdef applec
#define JBT long *
#define JB2CB
#define CB2JB
#else
#define JBT jmp_buf
#define JB2CB &
#define CB2JB *
#endif

extern JBT * __cur_buf ;

# define TRY \
	{ short __err ; jmp_buf __jbuf ; JBT * __last_top = __cur_buf ; \
		if ( ! ( __err = setjmp ( __jbuf ) ) ) { __cur_buf = JB2CB __jbuf ;

# define CATCH \
		} else { __cur_buf = __last_top ;

# define CLEANUP \
		__err = 0 ; goto __cleanup ; } else { __cur_buf = __last_top ; __cleanup :

# define DONE \
			if ( __err ) { \
				if ( ! __cur_buf ) { DebugStr ( "\pException Underflow" ) ; } ; \
				longjmp ( CB2JB __cur_buf , __err ) ;\
			} \
		} \
		__cur_buf = __last_top ; \
	}

# define NO_PROPAGATE \
	__err = 0 ;

#endif /* x_h */

