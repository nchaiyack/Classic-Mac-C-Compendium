/*  Notes to Nuntius.

Feel free to send me email if you're compiling Nuntius,
or otherwise work with the source.


I'm missing stack crawl on PPC.


++++ How to compile: +++++++++++++++++++++++++++
This version of Nuntius compiles with CodeWarrior 5
and MacApp 3.1.1
Standard installation, refer to Mark Anderson's guidelines.
You might want to fix the bugs in MacApp that I have posted
to comp.sys.mac.programming.oop.macapp3. See below.

A lot of the resources are compiled with MPW.
Run the script make_rsrc after you have modified any of 
the Å.r files.

I have had a really hard time with MacApp's management of
CODE segments: Version 2.0 of Nuntius unloads one of its segments
in the call chain when it unloads. Everything was OK on our macs,
but a lot of people had problems.
I have now substituted MacApps memory management with my own,
which keeps fingers off the segments, and don't use any reserves.
You should not include UMemory.cp and UMemory.more.cp in
the projects/libraries.

To get the segments load correctly, I had to outline the two
constructors in UBusyCursor.cp, and specify a #pragma segment MAMain
for them.
++++++++++++++++ MODS to MacApp ++++++++++++++++
When making the MacApp libraries, I change them
  to compile with 68000 (and not 68020) code generation,
  and full MacsBugs names.

Remember to comment out the ASLM check in DefineConfiguration()
in UMacAppGlobals.cp, as when ASLM is just taken out of the system folder,
the mac will crash when you ask for the ASLM gestalt.

+++++++++++++++ TCP debug window +++++++++++++++
For how to setup MacTCP on a single machine without
connection to the Internet, see TCPfprintf.cp
(I'm using Telnet instead of the SIOX).
Leave out CWDebug.c and SIOX.Lib files from the project.






Why you might prefer Nuntius:
	1) No 32K limits. You can view articles of any length.
	   You are not limited to 16383 groups.
	2) You can do several things at once: 
      e.g. while extracting binaries
	    you can go on and read other articles, open other groups, 
	    change the preferences, etc.
	    You are only limited by available memory.
	3) URLs are highlighed just like in Mosaic/NetScape.
	4) Hierachical list of all groups, much like the Finder

Why you might avoid Nuntius:
	1) Nuntius takes up more memory and more diskspace than any other
		newsreader for the mac.
	2) Nuntius has not been tuned for small macs
	3) It's not able to get your name from InternetConfig...
		(I wish they had made an URL resolver instead, we need that much more)
	4) Nuntius is written for the fun of it, I am not employed by
		anyone to do it, so development happends when I have time.
		(I have friends, study and real work besides Nuntius...)

You can get the source to Nuntius from ftp.ruc.dk in /pub/nuntius. Nuntius is
written in C++, MacApp 3.1.1 and CodeWarrior 5.

Why you might obtain the sourcecode for Nuntius:
  You want to add the feature, I didn't add...
  You want to have some MacApp sample code...
    (It's the most complete framework, not the newest...)
  You want to get code snippets to:
    use AppleScript, have preferences stored in a mini-database,
    get code to redirect fprintf(stderr...) to a TCP stream (Telnet...),
    a MacTCP C++ class,  Finder window code, BinHex/uudecode,
    and ...

*/