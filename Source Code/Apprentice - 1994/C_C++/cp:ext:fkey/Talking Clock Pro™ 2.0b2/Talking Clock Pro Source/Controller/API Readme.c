#if 0

	To compile this clock, you need Think C 6.0 and a special version of MacHeaders
	where many files that are #if 0 in the original Mac #includes.c are set to #if 1
	If you do not know how to generate a new copy of MacHeaders, please consult your
	Think C manual. If you still do not know how to do this; I suggest that you have
	not reached the technical skill level needed to fully assimilate this code.

	Stockholm 93-08-27

	This simple clock can also serve as a starting place for your own applications.
	What do you need to do?

	1)	You will have to change the source as it is.

	2)	For each window you create, you will have to provide a callback-function record.

	3)	You will also have to provide application callbacks for all events (these cannot
		be NULL; the window callbacks can be NULL) The app callbacks are called when a
		window is not frontmost to handle an event.

	4)	Go from there. Use the message system already built-in, and store your data in
		the data part of the DefWindowRec. In your window's Prepare and DoCommand functions,
		call the application's callbacks as well.

	There are some restrictions on applications that you write using this code as a
	starting place:

	i)		You cannot blame me for any crashes or lost data even if they are due to bugs
		in my code and not yours. You use the code at your own risk.
	ii)		I would very much appreciate to get a copy of what you're writing, when you're
		done. Failing that, a postcard would be swell.
	iii)	Please send any bug fixes you make to the code to me, so I may incorporate
		them in the base code and others may benefit. If you do, I will keep you on a list
		of people to notify of other bug fixes made.
	iv)		Once again; I cannot take responsibility for your use of this code, but please
		re-distribute my code only in its unmodified state. If you distribute a program
		built using a modified version of this code, or distribute a modified copy of this
		code, include a statement saying you have responsibility for the results, but also
		include a line saying "Some parts ©1993 Jon WŠtte"
	v)		The comment in the file x.h about limitation of my liability and responsibility
		goes for all of this code: IF THE LAW WHERE YOU LIVE, WORK, OR DISTRIBUTE YOUR
		WORK DOES NOT ALLOW ME TO DISCLAIM ANY AND ALL RESPONSIBILITY FOR ANY DAMAGES CAUSED
		BY THIS CODE OR THE DERIVATES THEREOF, YOU MAY NOT USE THIS CODE, ON THE PENALTY OF
		USD $1,000,000,000 (one billion) OR SEK 10,000,000,000 Kr WHICHEVER IS GREATER.
	vi)		I may provide some limited feedback if you mail me with questions about this
		code; I may even help you with specific problems you may have. However, I am a very
		busy person; perhaps more so than I would wish to be, so please understand if I do
		not have the time or energy needed to fully support this code base.

	Good luck!

#endif
