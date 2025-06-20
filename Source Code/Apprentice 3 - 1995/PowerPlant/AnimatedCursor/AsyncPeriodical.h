// AsyncPeriodical.h
// Copyright �1994 David Kreindler. All rights reserved.

// declarations of class AsyncPeriodical, VBLPeriodical and TMPeriodical

// theory of operation
//	these are abstract classes for implementing recurring periodic tasks
//	subclasses of class VBLPeriodical use the vertical retrace interrupt (VBL) queue for 60Hz period resolution
//	subclasses of class TMPeriodical use the Time Manager queue for 1kHz period resolution
//	subclasses of these classes must override the DoTask function, and might have to override the Start and Stop functions as well
//	subclasses' overrides of Start and Stop should call their superclasses' version

// warning
//	subclasses' overrides of VBLPeriodical's and TMPeriodical's DoTask function are called at interrupt time,
//	so they cannot use the MemoryManager, either directly or indirectly, and cannot rely on unlocked handles

// revision history
//	940212 DK: initial version
//	940219 DK: added class TMPeriodical

// to do
//	make a driver-based class (called by SystemTask), for tasks that need to allocate or move memory

#ifndef ASYNCPERIODICAL_H
#	define ASYNCPERIODICAL_H

#	ifdef __MWERKS__
#		pragma options align=mac68k
#	endif

#	include <retrace.h>
#	include <timer.h>

#	ifdef __MWERKS__
#		pragma options align=reset
#	endif

//
// class AsyncPeriodical
// abstract recurring task class
//

class AsyncPeriodical {
	
	public:
	
	// period units in the subclass' underlying timebase (kTimerHz)
	// note that Ticks can be negative for special purposes, such as "reversing" a task
	
		typedef long Ticks;
	
	// control functions
	
		virtual void Start(Ticks taskInterval);
		virtual void Stop();
	
	protected:
	
		AsyncPeriodical();
		virtual ~AsyncPeriodical();
	
		virtual void DoTask() = 0;
		
	// access to private member variables
	
		Ticks Interval() const;
		Boolean& IsRunning();
		
	private:
	
		volatile Boolean	mTaskIsRunning;
		volatile Ticks		mInterval;
};

//
// class VBLPeriodical
// abstract vertical retrace interrupt (VBL) recurring task class
//

class VBLPeriodical: public AsyncPeriodical {
	
	public:
	
		VBLPeriodical();
		virtual ~VBLPeriodical();
	
		void Start(Ticks taskInterval);	// virtual from AsyncPeriodical
		void Stop();					// virtual from AsyncPeriodical
	
	// the actual underlying timebase (1/sec)
	
		enum {
			kTimerHz = 60UL
		};
	
	protected:
	
		void DoTask() = 0;				// virtual from AsyncPeriodical; called by HandleVBLTask after the globals pointer has been set up
	
	private:
	
	// the extended ::VBLTask structure that we use to allow the static *VBLProcPtr to access an instance pointer (this) and the value of our globals register

		typedef struct {
			VBLTask			task;		// ::VBLTask, defined in <retrace.h>
			VBLPeriodical*	thisP;		// the *VBLProcPtr must be a static member function, so we pass it 'this' explicitly
			long			a5;			// the value of 68K register A5 (for applications) or A4 (for non-application code)
		} VBLTask, *VBLTaskPtr;

	// the VBL callback function (*VBLProcPtr)
	// sets up the globals pointer, calls the virtual member function 'DoTask', resets the VBLTask and restores the globals

#	ifdef __powerc
		static pascal void HandleVBLTask(VBLTaskPtr taskP);
#	else
	// this declaration uses the Metrowerks function parameter 68K register declaration syntax;
	// other compilers might require a #pragma parameter directive instead

		static pascal void HandleVBLTask(VBLTaskPtr taskP: __A0);
#	endif

		static VBLUPP	sProc;
		VBLTask			mTask;
};

//
// class TMPeriodical
// abstract Time Manager recurring task class
//

class TMPeriodical: public AsyncPeriodical {

	public:
	
		TMPeriodical();
		virtual ~TMPeriodical();
		
		void Start(Ticks taskInterval);		// virtual from AsyncPeriodical
		void Stop();						// virtual from AsyncPeriodical
	
	// the actual underlying timebase (1/sec)
	
		enum {
			kTimerHz = 1000UL
		};
	
	protected:
	
		void DoTask() = 0;					// virtual from AsyncPeriodical; called by HandleTMTask after the globals pointer has been set up
	
	private:
	
	// the extended ::TMTask structure that we use to allow the static *TimerProcPtr to access an instance pointer (this) and the value of our globals register

		typedef struct {
			TMTask			task;			// ::TMTask, defined in <timer.h>
			TMPeriodical*	thisP;			// the *TimerProcPtr must be a static member function, so we pass it 'this' explicitly
			long			a5;				// the value of 68K register A5 (for applications) or A4 (for non-application code)
		} TMTask, *TMTaskPtr;

	// the TM callback function (*TimerProcPtr)
	// sets up the globals pointer, calls the virtual member function 'DoTask', resets the TMTask and restores the globals

#	ifdef __powerc
		static pascal void HandleTimerTask(TMTaskPtr taskP);
#	else
	// this declaration uses the Metrowerks function parameter 68K register declaration syntax;
	// other compilers might require a #pragma parameter directive instead

		static pascal void HandleTimerTask(TMTaskPtr taskP: __A1);
#	endif

		static TimerUPP	sProc;
		TMTask			mTask;
};

//
// inline function definitions
//

#	ifndef OUTLINE_INLINES
#		include "asyncperiodical.i"
#	endif

#endif // !defined (ASYNCPERIODICAL_H)