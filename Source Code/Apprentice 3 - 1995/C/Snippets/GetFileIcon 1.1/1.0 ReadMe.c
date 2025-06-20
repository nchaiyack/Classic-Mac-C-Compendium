/*
GetFileIcon.c
1/2/95
ver 1.0


�
GetFileIcon is based on the FindIcon.c code by James Walker.

Q: Why does GetFileIcon exist?  Why not use FindIcon?
A: There are several reasons:
	1.	Displaying a file's icon is one of the most intuitive things to do on a
		a Macintosh.  Unfortunately, it is also very difficult; I would estimate
		that there is at least one person a week on the comp.sys.mac.programmer
		newsgroup who asks how to display the icon of a file.  Most programmers
		are usually referred to the FindIcon code by James Walker, but the code only
		shows how to get a files' icon; it doesn�t demonstrate how to display the
		icon.  Several programmers have posted messages requesting further help...
		this snippet is intended to take care of that need.

	2.	The FindIcon routines are able to get a file or a folders' icon.
		Personally, I do not need to retrieve a folders' icon, so I took everything
		that pertained to folders out (thus, reducing the amount of code by
		approximately half).

	3.	The FindIcon routine is comprised of 13 source files and 14 header files.
		With GetFileIcon, there are only two files to keep track of (GetFileIcon.c
		and GetFileIcon.h).

	4.	When I compile my project, I always have strict type checking turned on.
		Since GetFileIcon uses forbid_action() and forbid() in conjunction with
		'gotos', it took quite a bit of tweaking on my part to get the code to work.



Q: How do I use GetFileIcon?
A: Simple, In your THINK C project, make sure you have MacTraps and MacTraps2
	added, then add GetFileIcon.c to the project.  For an example on how to call
	GetFileIcon, look in GetFileIconExample.c.  BTW: This code seems to compile
	fine under THINK C 6.0 -> 7.0.4... I do not know about earlier versions of
	THINK C.



Other useful information:  If you wish to contact me, my Internet address is
jbeeghly@u.washington.edu (at least it is until June '95).  I make no guarantees
over the stability of the code (in other words, use it at your own risk).  If
you want to find out more about getting & plotting a files' icon, I strongly suggest
you get the code to FindIcon (available at most mac development FTP sites) and THINK
Reference.
*/