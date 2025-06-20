/* ExecMain.h
 *
 * Global routines and variables available to
 * the sections of the compiler.  Note: do not
 * modify the variables that are marked as
 * read-only.
 */
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */


#pragma once

//void	WatchCursorOn(void);
//void	WatchCursorOff(void);

/* Call to handle user interface updates. */
void	HandleEvent(unsigned long sleep);
void	Do1Event(EventRecord *theEvent);
void	HandleKeys(EventRecord *theEvent);
void	HandleMouseDown (EventRecord *theEvent);
void	HandleWindowEvent(EventRecord *theEvent);

void	PauseApplication(void);
void	ResumeApplication(void);
void	FinishedRunning(void);
void	QuitApplication(void);

typedef enum {
	applRunning,
	applPaused,
	applFinished
} ApplState;

Boolean	ApplicationIsPaused(void);
ApplState ApplicationState(void);

