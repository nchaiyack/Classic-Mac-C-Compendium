/*
___________________________________________________________

HOW TO GET HELP ABOUT NUNTIUS

*  Look in the "Help Menu" in Nuntius for the answers to
   Frequently Asked Questions (FAQ).

*  Read the newsgroup comp.sys.mac.comm, or ask a question
   there.


HOW TO GET INFORMATION ABOUT NEW NUNTIUS RELEASES

*  You can subscribe to the mailing list NUNTIUS-ANNOUNCE-L
   by sending the following one line in the body of a
   message to "listproc@cornell.edu":

   SUBSCRIBE NUNTIUS-ANNOUNCE-L your name

   For example, "SUBSCRIBE NUNTIUS-ANNOUNCE-L Hans Jensen".
   If you have problems with the list, please contact the
   list administrator, Aaron Freimark, at
   "aaron@med.cornell.edu".


HOW TO REPORT BUGS YOU FIND IN NUNTIUS

*  First, make sure you are using the latest version of
   Nuntius.

*  Second, make sure you can reproduce the bug, maybe even
   on someone else's Mac.

*  Third, read the FAQ to see if it is a common problem.

*  Fourth, read the newsgroup comp.sys.mac.comm to see if
   everyone else in the world already knows about the bug.

*  Finally, if you have gotten this far, send a nice message
   to Peter Speck at "speck@ruc.dk" with the following
   information:

   Your Macintosh model
   Your System version
   The version of Nuntius you are using
   Versions of any related software (Eudora, Netscape, etc.)
   As much information about the bug as possible

___________________________________________________________

Known bugs in latest version of Nuntius
      Sometimes the "Quit" menu item is grayed out. I would like
      to get feedback on this, as I can't reproduce it.

______________________________________________________________

# ignore this:  replace -c � /�t/ "  " "{active}"

Bugs fixed and improvements.

2.0.3:
  Fixed freeze in extract of GIF files.
  
2.0.2:
  Fixed linebreaks in Japanese text.
  More suffixes recognized in uudecoder: 
  Fixed a problem with tabs in lines with URLs.
  Fixed icon problem for extracted binhex files (creator/type was not set).
  
2.0.1:
  Took over the memory management from MacApp as I kept fighting its
  segment loading & unloading and memory reserves without any good.
  Recognises "bolo:" as an URL.
  URL recongnition is a bit better (demands slashes after some schemes).

1.3b32 alias 2.0 final:
  Fixed window highlight when Nuntius switched to Netscape or Eudora.
  Watch cursor when opening Netscape or Eudora.
  
1.3b31:
  Fixed "//" in beginning of mailto: URLs
  Fixed creator and type for notes files.
  Fixed very old activate-when-opening-in-background window bug in MacApp.
  Fixed bug in "Flush all caches" (had causes too if you were reading lot's articles).

1.3b30:
  The AppleScripts are now saved if they are changed while running them.
  Fixed problem with Anarchie (didn't like terminating null's)
  Added "Flush all caches" command. 
    Flushes:
       NntpConnectionCache, GroupDocCache, ArticleViewCache, ArticleManageViewCache
       ArticleWindowCache, ProgressCache, and ArticleTextCache.
    Tried to fix some "ReturnObject" fatal bugs for GroupDocs.
  AppleScripts are now provided as TEXT files too.

1.3b29:
  Fixed problems with urgent flag in TCP packets: a lot of newsservers didn't like it
    Thanks to Jim Madden for coming up with a extremly detailed report of the problem.
    (Bug in Universal Interfaces 2.0a3).
  Fixed menu balloons for edit menu.
  Fixed some 'SIZE' flags.

1.3b28:
  Fixed a problem with placement of article text (was way down in the scroller)
  Printing improved:
    Made margins a bit bigger when printing.
    Made it print page numbers.
  
1.3b27:
  Cleaned up the preferences dialogs and removed stupid options.
    (I think that tons of preferences is almost as bad as tons of 
    documentation you have to read before using an application)
    Made them use a smaller font, Chicago is just soo screaming.
  Fixed missing slash in "Nuntius home ftp site" URL.
  NetScape is used for ftp if the file is a .html file.
  Made the "Mail script" insert "Re:" as suggested by Anne Harwell
  Changes spaces in the distribution line to commas.
  Doesn't bring thread window to front when autoupdating and
    the group is already open.
  Doesn't break lines starting with '>' when posting.

1.3b26:
  In the 1.3b25 release, I forgot to remove some debug stuff from
    the URL code. Sorry.

1.3b25:
  Fixed a problem with purged cursors.
  URL parsing improved, e.g. handles <URL:ftp://ftp.ruc.dk/...>
  Redraw of article window improved.
  Handles pseudo info-mac URLs in comp.sys.mac.digest
  You're able to print articles again. Improved page breaks.
  Uses the email-script for "mailto:" URLs
  More uuencoded suffixes: mpg/mpeg, qt/mov
  Decodes %20 in ftp URLs.
  Better error message on PowerMac if Thread-Mgr or AppleScript 
    is not installed. (And AppleScript is no longer mandatory).
  Nuntius displayes the error message from the server in a special
    dialog (definitly not elegant) if the error is "interesting".
  
1.3b24:
  Fixed a serious bug in opening URLs.
  Redraw of article window improved.
  Fixed some problems with cursor in article view.
  
1.3b23:
  You have a real insertionpoint to select text with now
    (completely rewritten code for handling display of articles).
  The URL improvement made in 1.3b22 was disabled in non-debug mode, sorry.
  But, I have rewritten all of it, and now you have active hotspots.
    No more subtle clicking with the command/option key held down.
  Article seperators doesn't disappear somewhat down the articles.
  The article window is now cached (much faster in system 7.5)
  
1.3b22:
  Improved URL parsing (option click in text).
  Clear the "push flag" in the MacTCP code. It seems like the new headers
    or MacTCP 2.0.6 changes it into a "urgent" flag.
  Made a fix for "Returned object" fatal error.
  
1.3b21:
  The translate tables are now split out into seperate tables
    for input and output.
  Closes all files if it's being force quit'ed.
  Fixed more file icons problems.
  Fetch Latest Nuntius menu in balloon (ups, sorry) help menu.
  FAQ in help menu too.

1.3b20:
  Fixed some problems in uudecoder introduced in 1.3 beta.
    Made it better when trailing spaces have been stripped.
  Icons of created files are now ok (forgot to set type/creator).
  Doesn't reuse nntp connections if they are closed by the server.
  Speeded up database when having huge number of articles.
  
1.3b19:
  Closes preferences file when quitting
    true for non-debug versions of Nuntius too now.
  
1.3b18:
  Fixed bug in new async file stuff.
  URL button in "old beta version" dialog
  Closes preferences file when quitting.

1.3b17:
  URLs are now supported by option-click on the URL.
    Added "URL script" AppleScript.
    Improved error handling in mail script.
  Completely rewritten all async filemanager stuff. All filemanager
    calls should not be async. As the Resource Manager on the MacPlus
    has a bug that Apple does not intend to fix, I have to use the
    "d e v e l o p" style of threads which makes it too hard to use
    async i/o (as no param blocks can be put on the stack).
    Note that ResourceManager calls cannot be made async.
  Fixed bug in flushing nntp connections.
  AppleScripts can now be in TEXT format; they will be compiled.
  
1.3b16:
  Nuntius is now fat instead of having two seperate applications.
  Improved "mail script" so it checks for Eudora 1.4 which doesn't implement
    AppleEvents enough for doing reply-by-mail
  More progress when launching Nuntius.

1.3b15:
  "twice in gAllThreads" fixed
    (Showed up in 68K version on macs without the Thread Manager)

1.3b14:
  Nuntius is now native for PowerMacs, compiled with MacApp 3.1.1 
    and CodeWarrior 4.5. No modifications are needed of MacApp nor MW.
  Interface to Eudora and other mailers are now done by AppleScript.
    If AppleScript is not installed, the old hack is used.
    The AppleScript interface is independent of the actual mail 
    application (if the mail application isn't AppleScript compatible,
    an osax (== XCMD) can probably be hacked together.
  Fixed bug in translation of chars in list of threads.
  Threads are now expired when no of the articles in the thread is
    available at the newsserver (not bulletprof test).
  
1.3b13:
  Fixed bugs in translate tables.

1.3b12:
  ARA freeze when closing connection avoided.
  Fixed bug in progress indicator (never went the complete way 
    when updating groups).
  Fixed bug in sorting ("Redirected" was sorted as "Directed")
  MIME headers in posting when using ISO 8859-1
  Speeded up creating of article window when many articles.

1.3b11:
  Fixed multi "missing article" alert when saving articles.
  Fixed "no such volume" when saving database etc.
  
1.3d9:
  When marking >5 threads, scrollbar orgasm is avoided by redisplaying
    the list of threads instead of updating the list.
  MacTCP DNR errors are ignored when posting articles.
    (1.3d8 fix was not fixed at all).
  When quitting while having "Post article" window open, no
    fatal error is generated about nntp connections in use.
  Extraction of binaries slightly improved.
  No more "article not available" when doing a follow-up.

1.3d8:
  UU decode bug introduced in 1.3d7 fixed
  Threads menu selection fixed.
  The MacTCP error "out of memory" is ignored when posting.
    (the dotname of the mac is used to make the message-id header).
  
1.3d7:
  Fixed bug in selecting folders.
  UUdecode improved (just _had_ to see that gif)
  
1.3d6:
  Uses HFS cache in system 7.5 and newer 
    (as 7.5 fixes the long standing bug in the HFS cache)
  Improved uudecoder: more files extracted ok (this is an art!)
    Fixed filename mangling: colons only substituted if first char
    Fixed display of subjects
  Fixed bug in receiving articles: hang on some articles.
  Is better a figuring out when a nntp connection can't be reused.
  This version sorts threads by name (only). Later versions will
    have dynamic options for this.
  Fixed activation of Thread window when grouplists are updated.
  Fixed freeze in expire of articles.

1.3d5
  partial of 1.3d6, made only to check effects of async i/o
  
1.3d4:
  Able to read saved group list when opening from "Open..."
  Built with MacApp 3.1 final: should remove window problems.
  Threads are sorted by subject now. More advanced sorting will follow.

1.3d3:
  Expire implemented; new preference dialog. Comments, please!
  Database files are closed when the group window is closed.
  Fix for colons in filenames under A/UX (before only for HFS).
  New Extended Eudora trick: If you create a file named
    "Nuntius public files" in the preferences folder, Nuntius will
    not try load or save databases. (Much like local preferences)
  You will spare a lot of disk space, but update of the groups
    may take more time as a full rebuild is needed every time.
  Better linebreaking of headers.
  
1.3d2:
  Implemented new database from scratch. Much better threadding.
  Some changes to make it compileable under SymC++, not yet ok.
  The caching code for NNTP connections has been reworked, and the 
    connections are now only discarded when aborting an operation
    and the connection can't be reused.
  File writes are not cached by HFS anymore. This speeds extraction
    of binaries way up. Thanks for the INIT, Stuart Cheshire.
  A lot more information is written to the "Fatal Error" files.
  
1.3d1:
  Converted to MacApp 3.1
  Fixed a "progress indicator window" leak.

1.2 final.
  Removed extra "TCHR" resource (backup of "Latin1/DK-ASCII")
  Temporary preference files are now deleted.
  Submenu of grouplist documents should not be empty anymore.
  Save article uses NewsServer translate tables.

1.1.4b2:
  X-Date header is now 1994 instead of 94.
  X-UserAgent changed to X-Newsreader.
  Bug in (scan subject of) extracting binaries fixed.

1.1.4b1
  Thread Unit rewritten. Has now support for 3 implementations of threads:
    1) Apple's Thread Manager extension
    2) My own setjmp/longjmp implementation.
    3) My own copy stacks into handle (the old  "d e v e l o p"  type).
    #3 is always used on MacPlus, as they have a bug in the ResourceMgr
        (Apples ThreadMgr are not compatible with MacPlus either).
    #1 is used if present, otherwise #2
  -
  Better MacTCP code: issues read only when something *is* received.
  Reduced number of segments -> should load a little faster, or just for fun.
  Better handling of windows menu.
  Faster (network) read of preferences files.
  All windows are now edited in AdLib, making it easier for other
    appls to read the 'View' format.
  The new-user is now able to edit server-prefs before Nuntius tries to
    do anything with the server.
  NewOptions: authentication when openening a NNTP connection, and not
    postponed until the server asks for it.
  Able to close list of threads window, while extracting binaries.
  Alert if cancelling while Nuntius is posting an article, as it may
    be posted anyway. (No support in NNTP for aborting an operation).
  Better mapping of chars in US/DK-ASCII tables (quotes, accents, etc).
  If the specified translatetable is not available, Nuntius will just
    post an alert without quitting.
  ArticleWindow-update bug fixed.
  NewFeature: ROT13 implemented.
  NewOption: one can alter the rate at which Nuntius checks for new articles
  If some of the saved articles is not available, Nuntius will tell you, and
    save the available articles.
  Appends UNIX mailbox "From" header in Notes files: can be browsed by Eudora. 
  Fix for crash if local prefs was denied by the "Eudora document Trick".
  Warning if running an old pre-final version (I just got tired of getting 
   too many bug reports for old versions). Tells to upgrade.
  Checks the Preferences folder for changes much more seldom. Better GUI
    response when the folder is located at a server.
  NewFeature: can start check for new articles manually.
  NewFeature: assigns commandkeys 1-9 to items in the grouplist submenu.-
  -
  The code doing all the "post article/followup" windows has been reworked,
    but should be rewritten from scratch. Until then, it may be unstable.

1.1.3
  "Talk with system adm" bug fixed.
  New splash screen with the real version number (read from vers resource).

1.1.2 final:
      No more experimental.
      Reworked some code to avoid freeze crashes.
      It will not switch to ISO-2022 if you have only installed one script system.
        (To avoid displaying some articles in Chicago).
      All filemgr read/write calls are now async and buffered: 
        (AppleShare users _will_ notice it)
      Fixed bug in checking for blocking local prefs: if the preferences folder
        contains an alias to the real "Nuntius personal files", Nuntius will not
        abort anymore but silently resolve the alias.

1.1.1d27:
      *** Still experimental ***
      Support for ISO-2022 indication, but does not remember the indication
      across lines. Posted articles does always end in Roman state.
      (For US/European people: is able to encode Katakana/Kanji, but not flawless).
      You have to format the articles with newlines yourself, Nuntius doesn't
      try do to anything like that at all when it uses this encoding, 
      it doesn't even look for spaces. The character set translate table is not 
      used when Nuntius detects articles posted with ISO-2022 (or posts itself).      
      Look in the Prefs/Editor menu for using ISO-2022 when posting.
      Thanks to Shigeru Kanemoto for the ISO 2022 conversion code.
      
1.1.1d26:
      Some low-level code for memory handling have been improved. Rebuild the
      list of all groups is 5 times as fast on my mac. (no promise for your mac).
      This version may be unstable as it is not tested much, so 
      *** please don't distribute it to the rest of the world ***
      I'll let you know when I think I it's stable.
      As I don't need the same bug reported by 400 people, only 40 :-)

1.1.1d25:
      Option for specifying max number of articles in group database.
      !!! This should reduce the needed amount of memory needed !!!
      Clicking outside text in grouplists, will deselect. (I listen, I listen)
      Found & fixed bug causing "In TPeriodicThread::GoSleep, fThreadCmd was not gone".

1.1.1d24:
      Fixed charset translation in copy/reply etc. (double bug actually)

1.1.1d23:
      Rewrote code to close MacTCP connections if Nuntius is aborted (cmd-opt-esc),
        or a fatal error occurs.
      Added Transparent charset, look at Prefs/NewsServer.
      Lookups the servers IP at launch. Performs basic tests if it changes.

1.1.1d22:
      Tried to fix "1 in list of USED TProgress", but have disabled the report
        as the error doesn't matter.
      When trying to display unavailable articles, their status are set to read.
      If all text in an article is selected, it will be deselected if clicking
      wihtout modifierkeys.

1.1.1d21:
      Fixed a bug in choosing files: didn't show anything resonable.
      Fixed some bugs in the new uudecoder.

1.1.1d20:
      Fix for "TObjectCache::Free(), Has 1 object in list of USED TProgress"
        - but untested as I can't reproduce it.
      Kills MacTCP stuff in Nuntius heap if forced quit

1.1.1d19:
      If the shiftkey is down, Nuntius will not decode the BinHex
        (just save it). 
      More classes is pointer based.

1.1.1d18:
      BinHex decoder implemented. Will decode the binhex, not just save
      the text.
      The same appl for extracting the saved BinHex is used for 
      the decoded binhex.
      
1.1.1d17:
      Added new news server translate tables:
        "Latin1/US-ASCII (In/Out)"   (default for new users)
        "Latin1/DK-ASCII (In/Out)"   (for Danish users)
      Translates subject & from headers too
        balloon help in list of all groups too
      Fixed bug in translate of 8 bits chars when posting article
      Better at reporting fatal errors to me
      Fixed "In TPeriodicThread::GoSleep, fThreadCmd was not gone"
      Fixed "In TGroupListDoc::Free, had fPeriodicCheck"
      
1.1.1d16:
      Fixed bug in finding database file for groups with long names (>29 chars)
        this caused the mac to hang when Nuntius tried to open such a group
        (bug introduced in d14, and did trash a lot of disks, sorry)
      Fixed bug in errormessages.
      Expires threads when all of the articles have been expired at the server.
        - or no new articles have been added the last 3 days and the first
          article have been expired at the server

1.1.1d15:
      Fixed bug in killing processes for periodic task like updating groups
        (fixes "In TPeriodicThread::GoSleep, fThread was not killed")
      Reserved more memory for low memory situations
      Stops updating processes etc when in low memory situtation, but
        user can still abort updating etc.
      Fixed bug in disposing thread when it was not started
        (eg not enough memory to create it). This caused some
        "TThread::Free: 'this' is not gCurThread"
      Fixed bug in asking for password (cleared local copy unless it was 
        stored in the prefs)

1.1.1d14:
      Generates TeachText with description of fatal bugs instead
        of asking the user to take a snapshot. Reports available memory too now.
      Fixed bug in killing processes when quitting Nuntius
      Fixed bug in forking processes
      Fixed bug in killing a process (showed up when killing >1 processes)
      Quote char is in STR# (actually template)

1.1.1d13:
      Have written my own process manager (instead of using the
        library from "d e v e l o p" #6). This should fix the
        fatal errors "returned from EndThread(devThread)".
      Recognizes "too many users" strings for nntp error 400
      Expires threads only when the first article is not available at the
        server, and no new articles have been added the last 3 days
      Other fixes (lost this file due to bug in MPW script...)

1.1.1d12
      Max articles in XHDR command batches is preference
        (utilize news server disk cache feature)
      Reserves much more memory for threads: should avoid the fatal bug:
        "In TThread::Free, returned from EndThread(devThread);"
      Able to translate characters in articles:  Prefs/NewsServer
 
1.1.1d11:
      Remembers {which groups that is updated} between sessions
      Able to check/uncheck "Launch mailer with document" in Prefs/Mail dialog

1.1.1d10
      Tries only to save printinfo in group lists if it exist
      Close window-with-new-groups crash fixed
      Checks for new groups first, then updates database
      Long window names are smTruncMiddle in window menu

1.1.1d9
      Quit-group-while-update fatal freeze bug fixed.
        This bug sometimes got Nuntius to complain about fatal error
        "TObjectCache::Free(), Has 1 objects in list of in-use PRealNntp"
      Yet another "Mark as" bug fixed (will this ever end?)
      If the group is opened from the ListOfAllGroups, and the 
        public/private files does not exist, they will not be created/saved
      Max size of article cache *before* fecthing new articles is now
        a preference (menu Prefs/Misc)
      Able to handle bogous "ready" msg from ANU news server when your
        mac has no dot name
      Nuntius won't open 'PREF' files created by other appls.
      Progress bar does not work too fast/much when updating a group
      Macs without dotnames can post again, but path header etc are omitted.
      Long pathnames are smTruncMiddle in preferences dialogs
      You can choose the desktop folder when being asked for a folder
      Save article always saves the headers.

1.1.1d8
      Another "Mark as xxx" bug fixed and even better (faster) now.
      Can update database in background.
      You can always quit if you hold down the control key while
        clicking in the menubar (hack due to bug).
      Show xxx threads _preference_ is separated from the
        show xxx threads _command_
      Able to use port other than 119: write number after newserver name
        like telnet
      Can cancel posted articles
      Able to auto check for new groups at startup
      "Check for new groups" menu item changes to "Rebuild list of all groups"
        if option key is held down
      Upgraded to MacApp 3.0.1 from 3.0.0
      Will not post article if not edited
      Bug in getting own dotname fixed (showed up at random)

1.1.1d7
      Small arrows works again (d6 recompile bug)
      Fetches headers in small batches to utilize servers diskcache
      Date header in saved articles too

1.1.1d6
      You can choose your Eudora Settings file as signature file
      Expand xxx articles _preference_ is separated from the expand xxx
      articles _command_
      Can extract uuencoded binaries without starting separator
      
1.1.1d5
      Better errormessage when Nuntius can't get its own dot-name
      Better to scan subjects for uuencode binaries: 
        can skip past resolution codes
      Better detection of uuencoded blocks: first line must start with 'M'
      Shows Date header too in articles
      Can open preferences dialogs even if aliases cannot be resolved

1.1.1d4
      Fixed bug in optimized "Mark selection as..." when all
      threads are selected

1.1.1d3
      Mark as xxx is much faster when all threads are selected
      Able to create message-id and date headers
        Currently created as X-Date and X-Message-ID
      Bug in getting own dotname fixed

1.1.1d2
      Headergeneration much more flexible, 
        look at STR# named kPostArticleHeaderStrings
      Better at sorting multipart binaries:
        '|', ':' and '-' are separators too now
        bad subjects are ignored
        subjects starting with "Re:" are ignored
      Saving articles: long/short header and separator is added
      New feature: can save selection in note file 

1.1.1d1
      Remembers editor, extractor and mailer by alias instead of signature.
      Needed as the desktop manager can contain references to non-existing
        appls and will not work with appls on non-mounted volumes.
      Can launch application with "minimum memory", instead of requiring
        the normal partition.
      Error message is now corrected.
      "Path:" header is in STR#, specifies user name too for 
        maximum compability with old news server software


1.1 final
        Doesn't forget to ask "save changes?" when closing _new_ group lists.
        Closing article window while it's fetching the articles
        no longer crash

1.1b3: Bug fix release:
        Bug in list of articles when some of the articles are too old
        to be kept at the server (displayed with "n.a." is list of threads).
        Bug in error message when testing server for XHDR command is fixed
        (reported as "is missing basic groups").

1.1b2: Bug fix release:
        Launch-Nuntius-by-double-click-on-a-group-list bug is fixed

1.1b1: Bug fix release.
        More MacApp3/d e v e l o p   bugs fixed
        Better to remember window positions for group lists.

1.1a5: Major bug fix release:
        many MacApp 3.0 / d e v e l o p  threads incompability
        problems fixed
        Prints always only one page horiz.

1.1a4  New features version:
        support for authentication

1.1a3: New features version:
        triangles in the list of articles
        marking of articles
        saves window positions
        update of list of all groups
        change to MacApp 3.0 final, caused a lot of 
        problems and crash bugs.

1.1a2: first major release of version 1.1

Fixes earlier than 1.1a2 have not been recorded, sorry.
*/
