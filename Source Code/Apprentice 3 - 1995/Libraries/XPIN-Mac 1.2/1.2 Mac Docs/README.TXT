                          The Cross-Platform Plug-In Toolkit
                                               Version1.2
                                          December 1992.
                          (c) 1992, Ramin Firoozye - rp&A Inc.


You have in your possession, the Cross-Platform Plug-in Toolkit (XPIN). 

No, we're not talking about men who wear dresses, though I admit, the term "plug-in" can be misconstrued. But, I didn't invent it, I just stole it.

Plug-ins are small oblivious chunks of code that sit in their own little world and can not be directly invoked. There's one function per file and you can copy them around just like any other file. By themselves, they are pretty useless. But if you have a program that is "plug-in aware", you'll see some wondrous things happen. The plug-in aware application can "plug in" the little chunks of code into itself at runtime. Usually at startup, it looks for all the plug-ins it can find. Any plug-ins found can be loaded into the application at runtime and run as if they were built into the application in the first place. They act like little application helpers that can be added on long after a program has shipped. To the end-user, they appear to be not unlike a six-in-one screw-driver for which they can buy extra custom tips a few months later. The cool thing about them is that just as easily as they come, they go away. They can be unloaded from memory to free up valuable RAM for other cool things.

There are many benefits to plug-ins. Developers get to put optional functions into plug-ins and slim down their main applications. They can update and add functions incrementally rather than having to pull last-minute all-nighters adding features to  already large girth programs that will only fit on CD-ROM's. Users like them because it gives them control over which features they need with their application. Plug-ins are also a sign of "power user-hood" to some. Like INIT's on the Mac, a true power user has a folder bulging with plug-ins that would take a week to scroll through. I'm pretty sure it's a hormonal thing. Marketing types like them because they can sell whole disks full of plug-ins as "goodies disks". And during a trade show, they can spy on their competitors, then call the developers at home in the evening. They hack together a quick plug-in and modem it over so the next day, the Marketing types can show off the cool new feature.

The important property of a plug-in is that its installation involves the simple and familiar act of copying a file into a directory. Even end users and marketing types can do it. Removing them is just as easy. Just take them out of the directory. No special installers, configuration files, or pliers needed.

But wait! Did I hear you say "Is that all? What about that extra steak-knife set?" 

I did say, Cross Platform, didn't I.

Let's close our eyes and imagine together. What if you wrote your killer app on the Mac and saw this huge nebulous blob of a Windows market out there. It had large green tentacles, wore a circus tent, and didn't comb its hair. Well, if you were smart, you would have used some sort of a portable GUI toolkit like XVT or StarView so you wouldn't have to rewrite everything from scratch. If you were a stupid and short-sighted developer, obviously you wouldn't be here reading this note (or actually closing your eyes. Jeez...)

Even if you had used these portable GUI toolkits, your application would not have been able to use plug-ins very easily. Plug-ins delve into the bowels of an operating system and tickle its DNA (this isn't really true, but since writing plug-ins usually involves assembly language programming , most people really want to believe it). Plug-ins, as it turns out, fall into that murky "system dependent" region that would ordinarily go under the "to be implemented" category on the porting list... Obviously, the reverse Windows to Mac version of this scenario applies too.

Well, tarry no longer, that's where this toolkit comes in. The Caller API, those routines used in the main appliction to find and run plug-ins are completely portable between Windows and Macintosh. They are even spelled the same way. This means if your source code calls "XPINInit" in your Macintosh code, it also calls "XPINInit" the same way in the Windows version. What's more, the parameters are the same too. (The only place it sort of falls apart is when you have to specify a path to look for plug-ins. The toolkit tries to help out by letting you use $HOME as a shorthand for the application's home directory and $SYSTEM. On a Mac, $SYSTEM is the System Folder. On a PC, it's the Windows directory).

What about the plug-ins themselves. Well, all the behind-the-scenes-fat-men-with-cigars-behind-closed-doors action behind writing  plug-ins are taken care of by this toolkit. The simplest fully functionl plug-in you can write is seven lines long (I'm not kidding) and three of them lines are braces and comments. 

In fact, here it is:
-----------------
#include "XPIN.H"
XPIN(xBlock)
{
      XPINDescribe("Test", "Description of this thing", 1, 0);
      /* You would usually do something here */
      XPINEnd();
}
-----------------

Of course, it doesn't do anything, but you see the point.  Compile this baby and throw the binary file into the proper folder. Restart the plug-in aware appliction and your little plug-in is available for use. What's more, the exact seven lines copied over to the PC will compile with no changes and turn into a Windows-compatible plug-in! Copy it into the proper directory and... well, you know the story.

FTC regulations require truth in advertising. 

Remember, we're not selling binary compatibility. Last time I looked, we were looking at Intel vs. Motorola type differences there. You can copy your binary plug-in over to your IBM-PC and watch it do nothing. Next thing I know the Feds are kicking down the doors. 

What I mean is: your source code, if properly written, is portable. 

Those eagle-eyed ones among you might have noticed the fine print there: "if properly written". No you can't have your money back. It's very simple. If you call "Alert" in the Mac version of your plug-in, it won't compile very well when you rebuild the plug-in under Windows. They don't have "Alert" there in the Windows world. Same thing if you use "MessageBox" under Windows and pop it back over to the Mac. 

To be able to move the source code back and forth, you basically have three choices: 1) Limit your plug-ins to portable activity (like the above example); 2) Use a portability toolkit (although you should check the toolkit specs to see if it can be used inside a Macintosh stand-alone code resource or a Windows DLL); or 3) Use #ifdef's to segment your C program into code for the Mac and code for Windows. 

This all was a trick. Answer #3 is the right one. But, you may ask, what #ifdef's should you use?

The file XCONFIG.H does a bunch of looking around at compile time and defines a common set of #define's that can be used by these #ifdef's. I've tried to test this against as many compiler as I had access to. If your favorite compiler/development system isn't there, please send me a copy of the whole compiler and I'll fix it (just kidding, relax! I can hear the SPA dogs barking). The #define's  are divided into OS, COMPILER, and LANGUAGE groupings. Check XCONFIG.H for details. What this does is let you separate your program into common and system dependent code.

NOTE: In order to avoid the label of one who preaches but doesn't practice, you should be aware that the entire source for the toolkit itself is portable between the Mac and Windows platforms. Obviously, the project, library, and makefiles aren't, but the *.C and *.H files are all the same... They use the #ifdef approach too. You would be surprised how much common code there is.

The toolkit supports multiple plug-in "types" per application. So you can have one set of plug-ins to (for example) "export", another for "import" and a third for "nostrils". You can specify which directory to look into for each plug-in type (or throw them all in the same directory, I don't care). The library code has no built-in limit on how many plug-in's per type it can handle (other than available memory). However, to avoid runaway searches, it artificially stops at100. This is a #define setting. If you don't like it, bump it up to some huge number -- the index used to access plug-ins is a longword, so you can have something like 4 billion entries per plug-in type. Go nuts.

A memory management note: We don't actually load all these plug-ins into memory all at once when we start up. We just locate them on the disk, then run through each one, load them one at a time and send them a message to identify themselves. We build an in-memory list of them and toss them back out onto the street like petty little drunkards. At runtime, if the user asks that the plug-in function be executed, we call them back, apologize and reload just the one plug-in back into memory in order to let it do its thing. After it's done, we pop it back again. Yes, I know you performance freaks are cringing. But the whole point of plug-ins are to add secondary external functions. If you have a performance flea in your shorts, by all means, hardcode it into the program in machine language. All of this, by the way, is taken care of by the XPIN toolkit. You just call XPINInit, then call XPINCall whenever you want to run a specific plug-in. Before quitting, you call XPINDone to cleanup. That's all.

Since menus are so easy to modify at runtime, most applications add plug-in names to an "add-on" menu of some sort. The tooklit associates a "label" and a "description" with each plug-in. The main routine gets access to them by calling XPINLabel and XPINDesc. You're free to do whatever you want with these items (here's an original idea: add the label to a menu!) You invoke plug-ins by their "index", a number between 0 and the number returned by XPINCount.

Cleverly embedded in the last two paragraphs is the entire XPIN caller API. It don't get much simpler. There are basically six functions you need to know about to make your application plug-in aware.

The toolkit doesn't take care of everything though. The developer's got to do something too. You get to decide how plug-ins are shown to the end-user (if at all). Obviously, menus are a good choice. Pop-ups, lists, and button-bars are some other interface elements you can use. Since you don't know what (or how many) plug-ins the user will have on their system, the interface element you use should be expandable. For example, a button bar should be scrollable. When you design for plug-ins, remember, the sky's the limit.

In any case, as the developer, you need to come up with a way to associate each of these interface elements with the index for each plug-in. If they push a button in the button-bar, you need to be able to know which plug-in they want you to run. For example, if you are adding plug-ins to a menu, if the user picks the first item, you need to somehow translate that first selection to (say) index 0 and pass it to XPINCall. If they pick the second item, you use index 1 and so forth. So you need to come up with a clever scheme that matches interface items (menus, buttons, list items, etc...) to each plug-in's index.

The second thing you need to take care of is the way the caller program exchanges information with the plug-in. Remember, your main application has no idea what plug-in it may encounter down the road. You need to come up with some sort of a protocol between the two. Whatever incredibly brilliant scheme you come up with, it better be flexible enough to allow future cool things to happen without having to mangle and rebuild your main application. 

As a public service, what I do provide you with is enough rope to hang yourself and your machine with. What you get is a general purpose parameter block (called "XBlock"). The caller application loads it up with stuff it wants the plug-in to get and passes it when it calls XPINCall. The plug-in can do whatever it wants with the information. It can also return a status value which the caller can check to see if things worked OK.

The XBlock has a number of generic fields. It has an "action-code", a fixed number of "args", and a "status" field. The number of args are set in a #define and you can change it if you want--remember to rebuild the XPIN libraries though. Currently it supports five args. The action code is also something you can define. Check XPINSHR.H for those reserved for internal use. You can add global actions every plug-in has to respond to in the file XPINOP.C. Current XOP_INIT and XOP_DONE are the only two. Each arg field and the status field is a union and can be accessed as either a byte, word, long, or pointer value. A set of C macros, XSET#, XGET#, XGETSTAT#, and XSETSTAT#  (where # is one of 'b', 'w', 'l', or 'p') are provided to simplify access to the XBlock values. They can be used by both the caller and plug-in. On the PC, the "p" value is a far pointer.

Remember, it is your responsibility as the writer of the application to define what gets passed in args[0..4] and what action codes and status returns are supported. And after you decide, you should publish them so others can write plug-ins for your application too. I toyed with a lot of ways to save you this step. But short of a central parameter registry (i.e. me), or runtime parameter parsing (slow and not necessarily cross-platform portable), there is no easy way to do it and remain flexible. Remember though, plan ahead!

At this point, some of you may be wondering if these plug-ins are compatible with other plug-ins (like Photoshop or HyperCard). Well, they're not. I looked carefully at the way these other ones were designed and decided they were each intended for very specific applications. What I wanted was much more general purpose. However, if you think about it, each application defines a way to associate plug-ins with an interface element (i.e. the Filters menu in PhotoShop). And there is a well-defined protocol between the caller and the plug-in and it's published. So the two main ingredients for plug-in nirvana are right here ready for you to use with very little thinking...

The other important thing you should know about is that standlone CODE resources (Mac) and DLL's (Windows) each have restrictions on what can be done inside them. These aren't actually restrictions. More like rules. Or intense suggestions.  They have nothing to do with the way the toolkit is implemented. They just come with the territory. The restrictions usually don't get in the way, but when you step on them, your application will mysteriously barf and you will think of me as an enemy (rather than your best friend and someone who would never trash your house when you're out of town). 

The restrictions are further described in the Programmer's Guide. If you try doing something non-trivial, you should know about these restrictions. The truly daring will also read the References to further reading at the end. If you do, you'll be so much more impressed that some poor schmuck sat down and made plug-ins portable. Maybe you'll buy me a beer next time.

What you should be getting as part of this toolkit is the documentation, source, and libraries for the toolkit, sample callers for Mac (or Windows) versions, and a couple of plug-ins that do pretty much nothing, but sufficiently illustrate that this isn't vaporware. This release of the Macintosh version is compatible with Think C 5.0, and MPW 3.2.x.  You get Think C libraries and project files and MPW makefiles. Just note that Think C uses A4-relative techniques for making standalone code resources, and MPW uses the A5-relative ones. Take your pick.

The Windows version works with Microsoft C/C++ 7.0 and Borland C++ 3.1. Makefiles for both MSC and Borland, and Borland Windows IDE project files are included. I haven't tested against Visual C++ and Borland 4.0 but there should be no reason why they shouldn't work.

Please note that the toolkit has only been tested against the compilers and versions noted. If you have different compilers or versions, I don't make any promises (it might work though). The Mac version requires System 7. The toolkit supports Aliases to plug-ins, FSSpec's, and the new file and folder searching routines; things that make life bucket-loads easier. If there is a huge hue and cry for System 6 compatibility, I'll immediately add that in (NOT!). Everyone I associate with socially runs System 7 though, so I may have to find new friends. The Windows version has only been tested under version 3.1. It may work under 3.0 and I don't think there are any 3.1 specific routines there. Try it out if you must. Again, everyone I know moved up to 3.1 a long time ago. The tester program ran under a pre-release version of NT. However, these are 16-bit DLL's here. A future version will support the full 32-bit NT DLL's.

The toolkit was written in C and is mainly tested against it. If there's a demand, I'll make sure the next release works with C++ and/or Pascal as well. I don't make any promises though. I am willing to support a finite set of development environments. It's bad enough having to support multiple operating systems. Definitely online for an upcoming release is support for Unix (probably using the dld library and gnu C). Windows/NT and OS/2 support would also be nice. Since both support DLL's, it should be pretty simple. I just didn't have time to test them. And I'm still trying to get NT to fully coexist alongside DOS, BSD/Unix, Solaris/386, OS/2, and PenPoint on the same hard-disk. And no, I do not have a life...

There are a few additional features that real grown-up plug-ins with testosterone problems should support. The most important one is callbacks. Callback routines allow the plug-in to call back the main application and have a running dialog with it. You can achieve a similar effect through action-code/status-code combinations, but it's not the same. I didn't want to make things too complicated on the first pass. The other feature that I intentionally left out was the ability to associate icons and other multimedia datatypes (pictures, sound, video clips, etc...) directly with each plug-in. This was mainly due to portability considerations. Besides, the main application can easily associate these datatypes with each plug-in index internally. It would be cool though, if the plug-in carried its own baggage around with it.


Shipping Manifest:

README.TXT - This file.
Programmer's Guide 1.2 (Word) - The Programmer's Guide in MSWord 5 format.
Programmer's Guide 1.2 (Text) - The Programmer's Guide in text format. No pix.

[ common toolkit source code ]
XCONFIG.H - Sets the #define's for each environment/system/compiler.
XPINTYPE.H - Basic data type definitions used by all routines.
XPINSHR.H - Shared definitions between caller/plug-in.
XPINCALL.H - Include file for a calling application.
XPIN.H - Include file for a plug-in. NOTE: This routine actually generates code.
XPINGLUE.C - The source for all Caller routines.
XPINOP.C - The source for all plug-in routines.

[ Macintosh library - Think C 5.0.4 version]
These plug-ins are just for illustration. They pop up a simple dialog box with a message. Copy the plug-ins into the Caller's folder and restart the application. A new menu item should pop up. NOTE: The Think C projects should be set up to build CODE resources. It's probably easy enough to just copy the ones here for every new plug-in. The filetype should be modified for each plug-in type. The resource type should be left alone. The toolkit always looks for a single resource type (xPIN) in each plug-in file.

MCaller.c - Macintosh Sample Caller
MCaller.h - Include file for Sample Caller
MCaller.� - Think-C project for Sample Caller
MCaller.�.rsrc - Resources for ThC project
CalcXPIN.� - Think-C project for a sample plug-in.
CalcXPIN.�.rsrc - Resource file for the Calc plug-in project.
CALCPIN.C - Source code. NOTE: The filename is DOS legal. It's portable, after all.
DialogXPIN.� - Think-C project for a sample plug-in.
DialogXPIN.�.rsrc - Resource file, etc...
DLOGPIN.C - Source code. NOTE: Again, DOS legal.
Calc plug-in/Dialog plug-in - The actual plug-in. Run MCaller. Nothing out of the ordinary. Then copy these plug-ins into the same folder as MCaller and double-click again. Notice the new menu. Ta Da!
Bad plug-in - This is a sample plug-in that has the right filetype but is not really  plug-in. It's just here to show that the toolkit isn't blind and stupid. Copy it into the MCaller folder. Nothing exciting should happen.

[ Macintosh library - MPW C 3.2 version ]
MCaller.c - Macintosh Sample Caller
MCaller.h - Include file for Sample Caller (used by MCaller.c and MCaller.r)
MCaller.make - MPW Makefile for Sample Caller
MCaller.r - Rez input file for Sample Caller (derez form of MCaller.�.rsrc)
CALCPIN.C - Demo 'Calc Plug-in' source
CALCPIN.make - MPW Makefile for 'Calc Plug-in'
CALCPIN.r - Rez input file for 'Calc Plug-in' (derez of CALCPIN.�.rsrc)
DLOGPIN.C - Demo 'Dialog Plug-in' source
DLOGPIN.make - MPW Makefile for 'Dialog Plug-in'
DLOGPIN.r - Rez input file for 'Dialog Plug-in' (derez of DLOGPIN.�.rsrc)
SAGlobals.h - Standalone A5-globals Include file for all Plug-ins. 
SAGlobals.o - Object library of Standalone A5-globals routines
[ NOTE: You can get the SAGlobals sources from online sources. They
usually accompany Macintosh Technical Note #256 ].

---------

Think C Notes:
The Caller application needs to include XPINGLUE.C and at least the ANSI-Small library. (full ANSI works fine too). To avoid recompilation, I would suggest including a library version of XPINGLUE instead of the full source code. To build a library, create a Think C project, add XPINGLUE.C and ANSI-Small then select "Build Library". You can now add this library to your Think C project.

The plug-ins require XPINOP.C and the ANSI-A4 library. To build a library, create a Think C project, add XPINOP.C and ANSI-A4. Make sure you Set Project Type to "Code Resources". You may have to give it a Resource Type and ID value. Use 'pin!' and 128. Set the File Type to xPIN. Build Library. Now you can add it to any generic XPIN source.

BIG IMPORTANT NOTE: The plug-in sources, XPINOP.C, and ANSI-A4 all need to be in the same code segment. You can only have one function per plug-in file. Think C does support multi-segment and large model code resources. I left it as an exercise to the reader to do this. I personally think of plug-ins as small single function objects. If they are larger than 64K, they probably should be standalone apps.


MPW C Notes:
To get access to the A5-globals, the XPIN.H macros uses the Stand-Alone
Globals package that usually seem to accompany TN #256. The files SAGlobals.h and SAGlobals.o are included with this package. I wasn't sure as to the legality of including the whole source library. You can find the sources on BMUG and any Apple Developer CD (that's where I got mine). The latest developer CD (December 1992) reorganized the TN's and 256 seems to have been reorg'ed to somewhere out of sight. If you absolutely MUST have them and can't find them, drop me a line and I'll Email them to you if I can.

The *.r files are derez versions of *.�.rsrc files. If you accidentally wipe them out, they are easy to recreate.


--------------------------------------------------------------------
Legal Stuff

The entire library and Cross-platform Plug-in Toolkit are Copyright (c) 1992, rp&A Inc. and the author.
 
The portable plug-in toolkit source and library code is presented without any warranty or assumption of responsibility from rp&A Inc. and the author. You can not sell this toolkit in source or library form as a standalone product without express permission of rp&A Inc. You are, however, encouraged to use this code in your personal or commercial application without any royalties. You can modify the sources as long as the license and credits are included with the source.

For bugs, updates, kudos, or suggestions you can contact the author at:

CompuServe: 70751,252
Internet: rpa@netcom.com
Phone: 415-826-3113
Fax: 415-826-0199
US Mail: rp&A Inc.
             P.O. Box 77067
             San Francisco, CA 94107

P.S. Send a postcard if you find this toolkit useful.

----------------------------------------------------------------------

Author's Background (so you don't think he's some kinda crackpot or something):

Ramin Firoozye' is a crackpot who has been messing around with computer stuff for, oh, 10 years or so. He hides behind the legal corporate shield of rp&A Inc. and considers a 20 hour work week a moral victory over forces of evil. Most of the time, he'll solve friendly problems in return for a pint of Redhook Ale.

One more reason to hate the author's guts:

The XPIN toolkit was primarily developed on portable equipment while yoyoing up and down Highway 1 along coastal Northern California in a now deceased VW Camper Bus (R.I.P). Desktop systems were used for telecommunication, testing, and playing color video games. If they invent a safe fusion-powered portable and world-wide wireless networks, I'm outta here...

--------------------------------------------------------------------

Free plug: rp&A Inc. has over a million combined flea years of software development experience (it could be a zillion, we lost count a long time ago). Macintosh, DOS/Windows, NT, OS/2, Unix (System V and BSD), VMS, and PenPoint (basically anything with a manual, a compiler, and a task scheduler) are tackled without fear. rp&A also does object-oriented analysis, design, and programming, distributed applications on TCP/IP networks, and multimedia productions under both Macintosh and Windows. Here at rp&A Central, we don't engage in religious OS diatribes. We drink coffee and consider computers our friends. Feel free to call.
