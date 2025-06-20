#if 0

How to use the Notifier self-disposing Notification Manager request.

Written by Brian Stern
<BrianS@pbcomputing.com>
This source code may be used freely.

Notifier is a code resource meant to be used from extensions at INIT time to report 
errors.  It is an example of a self-disposing Notification request.  After the 
notification is posted there is no trace of it left on the machine (or hardly any).

Notifier is completely self-contained.  You simply load the code resource
into memory from your extension and pass it the address of a string to be reported
in a Notification Manager alert.  Since Notifier is a separate code resource it
is only loaded into memory if it is needed, which should be rarely.  The extension
that calls it can let itself be removed from memory.

Notifier makes a copy of the string and sets up a Notification request block.  If
it successfully adds this to the Notification queue by calling NMInstall it detaches
itself.  The extension that has called Notifier should not detach itself if the
error is fatal.  

After the Notification Manager has shown the alert it calls the notification response
routine in the Notifier code resource.  This routine removes the nmRecord from the
queue by calling NMRemove and then it marks itself Purgeable, unlocks itself, 
and returns.  The result of being purgeable and unlocked is that the handle is quickly
purged from the system heap.  Thus Notifier leaves little or no trace of itself after 
it returns.

See the files NotifierUnlock.c and NotifierTesterINIT.c for the source to the 
Notifier and an example of how to use it.

This package includes source for two methods of creating self-disposing requests.

There are several methods that have been used for creating self-disposing 
requests.  One method involves putting a bit of code in a low memory scratch area
called ToolScratch and jumping to that code, which then disposes of the handle 
containing the nmRequest.  The other method puts the disposal code on the stack and
uses an RTD instruction to clean up the stack.  

Both of those methods use self-modifying code.  The Toolscratch method is 
vulnerable to interrupt code that might use that location for something 
and the RTD instruction is only available on 68020 and later computers.

The unlocking method leaves a single used master pointer in the system heap.  I have
included a second source file NotifierToolScratch.c that disposes of a notification
request by the ToolScratch method.  I recommend that you use the Unlocking method 
but both seem to work.

#endif