This is a package of objects I used to interface to the thread manager package.  As
far as things go, it is pretty simple.  It is also strictly "as-is".  This is not a
complete project (rather, it is source ripped out of another project).  In fact, there
are references to objects that are not in this upload (like the ErrorRecorder).  Sorry.
Deal with it. :)

Basiclly, it shoulds how to use THINK C's TRY-CATCH exception constructs in Threads
without trashing your stack. :)  Someone, a while back, had asked me for this source.
I'm afraid I've lost that mail message.

You can tear this appart and stick it in the apropriate place in your project.
Have fun.

The application I built with this has been running for over two months on my SE/30
with system 7.1 and no bugs have been found.  I think this code is fairly robust, at
least, in my environment.  If you find some bugs let me know!

	Cheers,
			Gordon.

For an example, I've included the main routine (and some support stuff) for
a thread stresser routine (I built this thing when I suspected I was having
some trouble with the threads objects -- it just works out my objects).  Use
that to get an example of what needs to be done to get this up and running.