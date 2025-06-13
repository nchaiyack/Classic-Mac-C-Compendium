 /*
 
 
For version 3.0:
  * better memory management
  * better integration of AppleScript (scriptable)
  * sort of disc list view
  * list of all groups as real document (to reduce memory usage)
  * save settings option instead of submenus
  * offline reading officially supported
  * Drag and Drop support.
  * Searching in subjects and authors.
  * Maybe filters and total search.


**************** SEARCHING **************************************************************
 
> The only feature request for Nuntius that I have had since day one is the
> ability to search the message headers for a keyword. 

That would be nice -- to give a word and to see all the articles whose
subjects contain it become hilighted. I'm not sure how often I'd use it.

On a similar note, being able to search the List of All Groups for any
newsgroup names containing a certain keyword would be nice; the results
would come up in a new grouplist. Then you could find all "mac" groups,
all "job" groups, that-group-to-do-with-your-hobby which may be either
alt or rec, and so on...

 Anyone from this list able to get ahold of a PC may have a look at
how Trumpet does that. Starting from an empty group list, one types
characters and the list dynamically adjusts with any group containing
the string typed so far. It even seems that new typing interrupts the
ongoing algorithm to start a new list matching the new string, quite
welcome after the first character is typed.
Well, I'm not asking for Nuntius to do that, but suggesting that if
anything changes in "subscribe", this way of doing it be given a
close look.
The hierarchies of Netnews are not ideal to reach a subject indeed.

Searching: I keep on getting things in my mind like - auto-select all
threads with the selected word - and things like that.  Keep in mind that
if you implement some sort of 'interesting' article feature that it could
work much like 'searching'... as far as searching for name/subject/both, I
don't really have a preference.

**************** FILTERS **************************************************************

 >The other major thing for this, is how to mark it. I think Eudora's
>approach with simple filtering is good, but not enough. I would like to use
>some external programming language, so the users are not limited to what I
>implement. In other words: I would like Nuntius to become very involved
>with AppleScript or the alike. I would like to be able to program by saying
>"when this event happends, execute this script".

    Although I've had very little first-hand experience with it, Intercon's
TCP/Connect is said to have the best method for marking/killing a thread.
>From what I understand, there is a text file which the user may type lines
like

    Mark with "X" articles by "John Norstad" in "comp.sys.mac.comm"
    Highlight articles concerning "Nuntius" in Green.
    Boldface articles concerning "Nuntius" and by "Peter Speck".

    And so on.  And the news client does it.

    The text processing may be difficult to program and could be
substituted with, say, a well-designed series of popup menus.  But this
method is very open.  The same interface is used to mark items as
interesting and uninteresting, and any sort of semi-interesting in between.
And the user may pick the colors, styles, and marks as she or he wishes.

    If you want scripts, just put in the line "Execute ... with all
articles by..."  (Interfaces are always so much easier to discuss than to
code :-)

**************** THREAD LIST **************************************************************

Date threading: no single option is perfect - I'd suggest just going by
whatever is easiest - and if that doesn't work, go from there.

Reverse threading: allow the user some way of reversing the sort (i.e.
earliest date first/earliest date last - could also be applied to the other
types of sorting, but wouldn't be quite as useful.)

Selecting sorting criteria: allow the user to set it similiar to your
article and thread opening selections.  Also, allow users to simply click
on the current header items (subject, author) to change the sorting
criteria - think of the finder 'view by' windows when implementing.  You
could include a little switchable 'sorting direction' icon up on the
header, too.  Keep it simple though - this is the hallmark of your program.

Trumpet for Windows shows a line count.

I do find myself occasionally wishing for the ability to collapse threads;
some groups get dominated by huge, huge threads which take up screens and
screens. A Finder-like twist-triangle would work well here... The problem
is, how to indicate how many articles there are in a collapsed thread? How
to indicate how many are new or unread?

I don't particularly like adding a number at the start of the line -- extra
clutter that's redundant when the thread is expanded.

One thing does spring to mind: when a thread is collapsed, the "posters
name" on the left could be replaced with a "xx unread" or "xx new" text --
"new" being used in preference to "unread" in the same way the "new" bullet
is used in preference to the "unread" dash. Maybe in italics to
differentiate it from the other items...

How about the number of articles (unread/total) in a group?

**************** PRIVATE/PUBLIC **************************************************************

>Why is it that Nuntius stores the List of All Groups in the personal folder
>rather than the public folder? 
>
>Seems to me that everyone refers to the same master list of groups so the
>logical thing would be to have one copy accesible by everyone rather than
>everyone having a copy of the same list, especially when it takes  300-500k
>of disk space.

I made it so, as it then would be much easier to find out the new groups. If it 
is public, I have to ask the newsserver about which groups that have been created
since <date>. And I would have a problem figuring out which groups is missng 
(an unimplemented feature, as I never really missed it.
It is my experience (from all the bug reports), that asking the newsserver is 
working for most people, but some servers are not able to do it. They return no 
new groups, allways!.

It seems, however, that the list of groups should be public, and thus relying on 
the newsserver for asking about new groups.
It seems to, that the files containing the information about which articles that 
are new/seen/read should be in one single file. I have, however, some trouble 
figuring out the format of this file.

**************** SMALL ONES **************************************************************
progress ved close

As Nuntius does an extraordinary good job in opening on demand multiple
connections "on the quiet", I suggest that it accept that those connections
be closed from server's site when they're idle "on the quiet" too, that is
whithout worry-generating error message to the user.
This can also happen when the server is restared and I suppose the user
shouldn't be worried in that case either.
Unless data is affected (connection active), of course.

twice open af same prefs, just be silent

I don't think the problem is modific. date change with WriteNow, for I tried it
at first.  With WriteNow >=3.0, when it reads in a text file it appends 
"(WN3.0)" to the file name when it saves it.  Then when Nuntius looks at the
file it hasn't been changed, for it doesn't know about the name change.  Since 
most need Eudora anyway for e-mail responses
it seems just as easy to specify it as your editor (sure wish it has a spell
checker, yeh, yeh, I know, get the commmercial version).

More time for closing connections, spin off list of MacTCPs waiting for ack.

Mark articles as unread for a whole group without opening it.


This experience shows up two problems. One is that Nuntius (which is in
every other respect infinitely superior to the 'postnews'/'trn'
combination) could do a better job of reporting newshost responses. I'm
copying this message to Nuntius's author in case he wants to tidy up this
loose end in some future release.

forward menu item

queue for extract of binaries instead of spawning several threads

**************** BUGS **************************************************************

Well, I, for one, would rather have 'scrollbar orgasm' than the current
solution, which is to lose the cursor position when you mark more than
five messages...

Does Nuntius honour the "Followup-To:" header when posting a followup? I
haven't done an explicit test, but I have found myself correcting
newsgroups lines.

Second comment: When posting a followup in a long thread, Nuntius by
default supplies the Subject line from the earliest post. This is wrong;
subject lines get changed as the conversation wanders, and Nuntius should
pick up it's Subject line from the header of the article being followed up
to.

* Another one: Nuntius is in the background and is opening up a new thread
window.  The process finishes and the thread window is displayed, but the
window it brings up gives the impression that it is the top most window.
(i.e. both the foreground's application window, and the Nuntius window have
striped title bars.)  Could this be a MacApp problem?

* Our news server runs at capacity most of the day.  Therefore, many
requests to expand a thread are not met.  Have a problem here.  If Nuntius
is trying to expand the thread it will, for each new/unread article, try to
get the article.  If it can't get it, it 'unexpands' the article and marks
it as 'read' in the process.  I have to manually go through and expand each
article after this happens.  Previous versions instead reported gave a
dialog box saying that the server is unavailable, but didn't mark the
article as read.

And I would like to add my own "polite request" to Nuntius features in the
next version: when you read a newsgroup, you can use the space-bar to read
sequentially inside a thread both in Nuntius and in Newswatcher. However
when you have read the last article in a thread and hit the space-bar, in
Nuntius it takes you to the top of the group, whereas in NewsWatcher it
takes you to the *next* thread. I would very much like to see this latter
feature implemented in Nuntius: it's a real bother to play around with the
mouse or hit the space-bar a great number of times till you get to the next
thread after the one you have gone through.

I agree.  Losing the cursor position after marking messages as read
is really annoying.  Why can't Nuntius behave like Eudora?  There,
if you delete a message, the following one is selected.


**************** LIST OF ALL GROUPS **************************************************************
..as the "All Groups" list is modelled on the Finder view-by-name, it
would be nice if it did these two things:

"Expanded" ie turned-triangle hierarchies collapse if a window for that
hierarchy is opened (cf expanding a folder in view-by-name and then
double-clicking the folder).

Option-double-clicking a hierarchy opens a window for that hierarchy and
closes the parent window (as in Finder, sometimes I want to navigate to a
group without leaving a clutter of windows behind).
**************** BINARIES **************************************************************
gammel renames til extract from selection,
og kommer altid med liste som kan laves om p} r{kkef|lge.

skan alle artikler for kendte suffix/filnavne
Gem fundne filnavne i dict, og lave liste af
  liste af sammenh{ngende artikler

Frav{lg dem hvor ingen artikler ville blive vist med nuv{rende thread-menu ops{tning

Analyser dem for part-numre, og list dem |verst som giver mening.

Lav triangel, s} man umiddelbart ser dem der giver mening cmpacted.


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
BUGS:-----------------------------------------------------------------

Prefs for ListOfAllGroups Shared. (In public folder!)

gray cQuit check efter cancel af gruppe update

check for new groups virker ikke altid

Den besked der er til NNTP error 400 skal vises i fejl dialogen.

New important features:--------------------------------------------------

change newsserver.

paragraph select click in left margin


Append article to file

click i text nŒr alt er valgt -> frav¾lg


--------------------------------------------------------------------------------

>  Oh yeah, one small bugette I found.  I moved my 'status' window for
>extracting binaries to the far right side of my 2 page display.  When I just
>extract binaries from a list its fine but if I open a thread to read it, the
>location seems to be reset back to the original location on the left side of
>the monitor.  I played w/this a bit and found that after a while of extracting
>some binaries the window moved back but only 2 out of 3 lists and only after a
>number of binaries had been extracted.  Just a small thing for your to-do
>list...  I think you've done a great job w/Nuntius and hope you keep it going!

save/extract binaries THREADS skal m¾rke dem som l¾ste

At the moment you have the option to auto-run a BinHex decoder when a
BinHex file is downloaded. How about extending this to include an auto-run
application for GIFf and JFIF files after they are successfully uudecoded?

--------------------------------------------------------------------------------
Discipline + EvenBetterBusError
Heap scramble (MacsBug)

Delete inactive threads time-out preference for hver gruppe

/////////////////////////////////////////////////////////
gray drag select

Now, here's what I think the ideal situation should be, based on personal
use. (Usually I want to save either the whole article or just a few lines
-- in both cases I want it to go to the notes with a divider from the
snippet above and an attribution line if necessary.)
1) In _all_ cases a dividing line should be written above the selection.
2) If _all_ of the headers are included in the selection, don't
   add an attribution line -- it's redundant. If only some or none of
   the headers are selected, write an attribution line
3) Write the selection.

  option for
    no or user-defined path header
    generate date header
    generate message-id header

forward af artikler via SMTP

>i like being able to put all the files outside the preferences folder, but
>it could be even better: if I click on a group list file you should scan
>the folder the group list is in to try to locate the prefs file, so I can
>also start by clicking a group list. this is the way Eudora works.

RFC-CHAR

When I read newsgroups this morning a desirable feature came in my mind 
which I think is not too difficult to implement.
I often read a posting containing some questions and I was very curious 
reading answers. So I tried to keep in mind the threads title. But the 
next days the thread was only one in a million (exaggeration) and I 
missed the reply.
So if I could mark a thread for "special interest" in which way ever...
May be such marked threads could appear on top of the list....
->On my list for 1.2, I have a "mark as interesting" (This is a separate flag, 
article will still have the new/seen/read), and a "show interesting threads".
The interesting threads will never be deleted, unless marked as 
"uninteresting".
Does this cover your needs?


I use it from different Macs, some of which have portrait monitors, some
have 640x480. It remembers the window size from the portrait when I switch
to the smaller screen, but it should automatically make the window smaller
so it all fits on the screen.


What I would like is a menu with checkable items, for the following:
Path, From, Newsgroups, Subject, Message-ID, Date, Article-I.D, References,
Sender, Organization, Nntp-Posting-Host and anything else you think
appropriate,
plus... a menu item called "Show Full Headers" which when selected overrides
the user preferences set with the other menu items, and shows everything,
much as your "Show article headers" does now. I think that it should leave
all the other menu items as they are (perhaps greyed out) so that when the
user unchecks "Show Full Headers" the display returns to what they had before.


autodelete of obsolote public/private files

3) Mark group(s) as read. I want to select one or several groups and mark
them as read. This is much faster than opening a bunch of groups one at a
time and waiting for the stuff to transfer.


3. On a small machine Nuntius cannot be given much memory, so extracting
binary articles in parallel uses up the memory too quickly. It might be
better to extract each binary in turn so that only one extraction is
running at a time, and the user can go and have a cup of coffee or
something while the extraction is running.


find text:
	subject/author in list of threads
	body of displayed/all articles in list of threads (slow slow slow)
	body of displayed articles in article window


separate font (view) for printing

* When viewing a group window (or is it called a thread list window?), I find
myself wanting to type the first few characters of a thread's subject to scroll
down to it. Does this sound like a useful feature? Perhaps you could compare
typed characters, ignoring beginning "Re:"'s. You might also want to handle tab
(next alphabetically) and shift tab (previous alphabetically), like you do in
group list windows.


uudecode filename -> type/creator map list

*/