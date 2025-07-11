// =================================================================================
//	UFutures.cp
// =================================================================================
//	Copyright:
//	� 1993 by Apple Computer, Inc., all rights reserved.
//	� 1993-1994 by Steve Sisak (sgs@gnu.ai.mit.edu), all rights reserved.
//	� 1994 by Harold Ekstrom (ekstrom@aggroup.com), all rights reserved.

#include <UEnvironment.h>
#include "UFutures.h"

AESpecialHandlerUPP	UFutures::sDoThreadBlockUPP = nil;
AESpecialHandlerUPP	UFutures::sDoThreadUnblockUPP = nil;
AESpecialHandlerUPP	UFutures::sIsFutureBlockUPP = nil;


// ---------------------------------------------------------------------------------
//		� InitFutures
// ---------------------------------------------------------------------------------

pascal OSErr
UFutures::InitFutures()
{
	OSErr	err = noErr;

	Try_ {

		// Check for the presence of the thread manager.
		if ( !UEnvironment::HasFeature( env_HasThreadsManager ) ) {
			ThrowOSErr_( threadProtocolErr );
		}

		// Create UPPs.
		sDoThreadBlockUPP = NewAESpecialHandlerProc( DoThreadBlock );
		sDoThreadUnblockUPP = NewAESpecialHandlerProc( DoThreadUnblock );
		sIsFutureBlockUPP = NewAESpecialHandlerProc( DoThreadBlock );
	
		// Install the blocking handler.
		err = ::AEInstallSpecialHandler( keyBlockFunctionClass,
			sDoThreadBlockUPP, false );
		ThrowIfOSErr_( err );

		// Install the unblocking handler.
		err = ::AEInstallSpecialHandler( keyUnblockFunctionClass,
			sDoThreadUnblockUPP, false );
		ThrowIfOSErr_( err );

	} Catch_( inErr ) {
	
		// Nothing here.
	
	} EndCatch_

	return err;
}


// ---------------------------------------------------------------------------------
//		� Ask
// ---------------------------------------------------------------------------------

pascal OSErr
UFutures::Ask( AppleEvent *inEvent, AppleEvent *outReply )
{
	//	Send the question with an immediate timeout.
	OSErr err = ::AESend( inEvent, outReply, kAEWaitReply,
		kAENormalPriority, kImmediateTimeout, nil, nil );
	
	// Timeout isn't really an error (in fact, it's expected).
	if ( err == errAETimeout ) err = noErr;

	return err;
}


// ---------------------------------------------------------------------------------
//		� BlockUntilReal
// ---------------------------------------------------------------------------------

pascal OSErr
UFutures::BlockUntilReal( const AppleEvent *inEvent )
{
	OSErr	err = noErr;
	
	// Attempt to extract a non-existant attribute.
	// This will block the current thread if the event is a future.
	AERecord	theParam;
	err = ::AEGetParamDesc( inEvent, kAENonexistantAttribute,
		typeAERecord, &theParam);

	// Descriptor not found isn't really an error in this case.
	if ( err == errAEDescNotFound ) {
	
		err = noErr;
	
	}
	
	return err;
}


// ---------------------------------------------------------------------------------
//		� IsFuture
// ---------------------------------------------------------------------------------

pascal Boolean
UFutures::IsFuture( const AppleEvent *inEvent )
{
	Boolean				isFuture = false;
	OSErr				err;
	AESpecialHandlerUPP	theBlockProc;
	
	// Get the blocking handler.
	err = ::AEGetSpecialHandler( keyBlockFunctionClass, &theBlockProc, false );
	
	if ( err == noErr ) {
	
		// Install a temporary handler that does nothing.
		err = ::AEInstallSpecialHandler( keyBlockFunctionClass,
			sIsFutureBlockUPP, false );
	
		if ( err == noErr ) {
		
			if ( BlockUntilReal( inEvent ) == errAEReplyNotArrived ) {
			
				// It's a future!
				isFuture = true;
				
			}		
			
			// Re-install the blocking handler.
			err = ::AEInstallSpecialHandler( keyBlockFunctionClass,
				theBlockProc, false );
		
		}
	
	}
	
	return isFuture;
}


// ---------------------------------------------------------------------------------
//		� IsFutureBlock
// ---------------------------------------------------------------------------------

pascal OSErr
UFutures::IsFutureBlock( AppleEvent *inEvent )
{
	return noErr;
}


// ---------------------------------------------------------------------------------
//		� DoThreadBlock
// ---------------------------------------------------------------------------------
//	Apparently the current thread needs to access some information, which 
//	is really a future.  We need to see if there is already a list of threads
//	blocked on this message.  If there isn't, create an empty list.  Add
//	the current thread to the list.  Sleep the current thread.

pascal OSErr
UFutures::DoThreadBlock( AppleEvent *inEvent )
{
	OSErr			err = noErr;
	SThreadListHdl	theThreadList;
	LThread			*theCurrentThread;

	// Get the current thread.
	theCurrentThread = LThread::GetCurrentThread();

	if ( theCurrentThread != nil ) {
	
		// Get the thread list attribute.
		DescType	theActualType;
		Size		theActualSize;
		err = ::AEGetAttributePtr( inEvent, kAERefconAttribute,
			typeLongInteger, &theActualType, (Ptr) &theThreadList,
			sizeof(theThreadList), &theActualSize );

		if ( err == errAEDescNotFound || err == noErr && theThreadList == nil ) {
		
			// If we can't find a waiting thread list, then create
			// one containing just ourself and put it back in the event.
			theThreadList = (SThreadListHdl) ::NewHandle( sizeofThreadList(1) );
			
			// Check MemError.
			err = MemError();
			
			if ( err == noErr && theThreadList != nil ) {
			
				// Setup the new list with only the current thread.
				(**theThreadList).numThreads = 1;
				(**theThreadList).threads[0] = theCurrentThread;
		
				// Put the thread list in the event.
				err = ::AEPutAttributePtr( inEvent, kAERefconAttribute,
					typeLongInteger, (Ptr) &theThreadList, sizeof(theThreadList) );

			}
		
		} else if ( err == noErr ) {
		
			// Otherwise just append the current thread onto the existing list.
			Int16 theCount = (**theThreadList).numThreads;
		
			// Resize the thread list.
			::SetHandleSize( (Handle) theThreadList, sizeofThreadList(theCount+1) );
	
			// Check MemError.
			err = MemError();
			
			if (err == noErr) {
			
				// Add the current thread to the list.
				(**theThreadList).threads[theCount] = theCurrentThread;
				(**theThreadList).numThreads = theCount+1;
			
			}
		
		}
	
	}
	
	if ( err == noErr ) {
	
		// Suspend the current thread.
		theCurrentThread->Suspend();

	}

	return err;
}


// ---------------------------------------------------------------------------------
//		� DoThreadUnblock
// ---------------------------------------------------------------------------------
//	This message has just turned real.  If there is a list of threads blocked 
//	because they tried to access the data, walk through the list of blocked
//	threads waking and deallocating the list element as you go.

pascal OSErr
UFutures::DoThreadUnblock( AppleEvent* inEvent )
{
	OSErr			err;
	SThreadListHdl	theThreadList;

	// Get the thread list attribute.
	DescType	theActualType;
	Size		theActualSize;
	err = ::AEGetAttributePtr( inEvent, kAERefconAttribute,
		typeLongInteger, &theActualType, (Ptr) &theThreadList,
		sizeof(theThreadList), &theActualSize );

	if ( err == errAEDescNotFound ) {
	
		//	It's possible that this unblocking handler will get called
		//	for ALL replies to apple events, not just futures. If that's
		//	the case, then getting the above error is not really an error.
		//	Clear it and just return.
		err = noErr;
		
	} else if ( err == noErr && theThreadList != nil ) {
	
		//	If there's a waiting list, make all threads in it ready for execution.
		//	We won't report errors inside the loop because:
		//		1)	one of the threads might have been disposed of
		//		2)	there's nothing we can do to recover at this point
		//		3)	the important thing is to wake up all remaining threads.

		Int16	theCount = (**theThreadList).numThreads;
		
		while ( --theCount >= 0 ) {
		
			LThread	*theThread = (**theThreadList).threads[theCount];
			
			// Resume the thread.
			// Note: should make sure the thread is actually suspended first.
			// If this happens, LThread throws an error.
			theThread->Resume();

		}
		
		// Now free the list handle (and take it out
		// of the refCon just to be safe).
		::DisposeHandle( (Handle) theThreadList );
		theThreadList = nil;
					
		// Put the nil thread list handle in the event.
		err = ::AEPutAttributePtr( inEvent, kAERefconAttribute,
			typeLongInteger, (Ptr) &theThreadList, sizeof(theThreadList) );

	}	

	return err;
}
