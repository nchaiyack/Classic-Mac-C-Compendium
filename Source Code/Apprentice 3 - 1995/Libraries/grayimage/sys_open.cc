// This may look like C code, but it is really -*- C++ -*-
/*
 ************************************************************************
 *
 *		   The lowest but one level file opener
 *
 * The program is intended to serve as an interface between the system
 * call for opening the file and the filebuf layer of the I/O G++ streams.
 * The function sys_open pretends to be <sys/file.h>'s open()
 *    int open(const char * filename, const int mode, const int mask)
 *
 * and extends functionality of the latter by the following
 * - if the file cannot be opened, the program writes a diagnostic
 *   message (through report_error()), indicating the problem and
 *   the name of the file of question.
 *
 * - the name of the file to open may be specified now as "| command"
 *   or "command |" i.e. as a pipe. The command is launched in a
 *   subprocess through '/bin/sh' with its standard input/output
 *   hooked to the file through pipe().
 *
 * Installation
 * modify filebuf.C file in the libg++ as follows
 * add the declaration at the beginning of the file
 *  extern int sys_open(const char * filename, const int mode, const int mask);
 * and then replace all occurences of " ::open(" for " ::sys_open("
 *
 * Implementation of _popen is due to
 * 	Copyright (C) 1991, 1992 Per Bothner. (bothner@cygnus.com)
 *
 ************************************************************************
 */ 

#include <std.h>
#include <sys/file.h>
#include <errno.h>

#define report_error _error
#include "myenv.h"			// Declares "error.h"

/*
 *------------------------------------------------------------------------
 * 
 * Launch the 'command' through 'sh' and hook its standard input/output
 *				to a file.
 * The file handler is returned, or -1 in case of error
 *
 * Important !
 * First argument, 'command' should point to a string buffer that lives
 * long enough (it'd better be static). This is because the command
 * pointer is to be passed to the subprocess that definitely won't
 * complete before the function _popen() exits. So the care should
 * be taken to make sure that 'command' still points to a valid,
 * undisposed buffer even after the function exits. The static
 * buffer will do it.
 * The second argument, mode is assumed to take only two values,
 * either O_RDONLY or O_WRONLY.
 */

static int _popen(const char * command, int mode)
{
  struct { int read_fd, write_fd; } pipe_fds;

  int parent_end, child_end;		// ends of the pipe
  int child_std_end;			// File handlrs for stdin/out

  if( ::pipe((int *)&pipe_fds) < 0 )	// Creating a pipe
    return -1;

  if( mode == O_RDONLY )		// We're reading, command is writing
    parent_end = pipe_fds.read_fd, child_end  = pipe_fds.write_fd,
    child_std_end = 1;			// stdout for command
  else					// Command is reading, we're writing
    parent_end = pipe_fds.write_fd, child_end  = pipe_fds.read_fd,
    child_std_end = 0;			// stdin for command

    int kid_id = ::fork();
    if( kid_id == 0 )
    {					// We're in kid's process
      ::close(parent_end);		// which is to execute the command
      if( child_end != child_std_end )
      {
	::dup2(child_end, child_std_end);
	::close(child_end);
      }
      ::execl("/bin/sh", "sh", "-c", command, 0);
      ::_exit(127);			// Executed only if execl failed!
    }

    ::close(child_end);			// We're in the parent process
    if( kid_id < 0 )
      ::close(parent_end), parent_end = -1;	// if fork failed

  return parent_end;
}

/*
 *------------------------------------------------------------------------
 *			  Phony 'open'
 */

int sys_open(const char *filename, const int mode, const int mask)
{
  int fd = -1;			// File handler of the file being opened

  register char *p = strchr(filename,'|');
  if( p == 0 )
    fd = ::open(filename,mode,mask);	// Regular open(), without pipes

  else if( p == strspn(filename," ") + filename )
  {					// if '|' is the 1st non-blank of fname
					// create the pipe to write to
    if( (mode & (O_WRONLY | O_RDONLY)) != O_WRONLY )
      report_error
	("File name '%s' looks like the pipe to write to,"
	 "\nbut the open mode is not WRITE_ONLY",filename);
    fd = _popen(p+1,O_WRONLY);		// p+1 points to the char right after |
  }
  else if( *(p+1) == '\0' )		// '|' is the last char of the filename
  {					// Create the pipe to read from
    static char tempname[200];		// tempname MUST be static
    if( p-filename >= (signed)sizeof(tempname) )
      report_error ("Pipe name is way too long!" ); // Copy the file name up to
    strncpy(tempname,filename,p-filename);	    // (but not including) '|'
    if( (mode & (O_WRONLY | O_RDONLY)) != O_RDONLY )
      report_error
	("File name '%s' looks like the pipe to read from,"
	 "\nbut the open mode is not READ_ONLY", filename);
    fd = _popen(tempname,O_RDONLY);
  }
  else
    report_error
      ("Pipe char in the file name occured in the wrong place");

  if( fd < 0 )
    report_error ("Can't open the file '%s' due to the reason above",
		  filename, (perror(""),0));
  return fd;
}

