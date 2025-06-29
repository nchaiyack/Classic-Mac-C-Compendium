
# PROGRAMMING THE MACINTOSH WITH THINK C

Why would anyone want to become a Macintosh programmer? The answer is more complex than you might think. Several circumstances make a familiarity with programming the Macintosh helpful if not downright necessary.

- Mac users are becoming more sophisticated and are demanding more from their machines. Many state-of-the-art application programs allow user programming through either macros or a built-in control language such as Basic.
- Apple has privately indicated that a scripting language might eventually become part of the Macintosh operating system.
- HyperCard and SuperCard give Mac users a taste of programming but lead them on. To accomplish meaningful work, HyperTalk and SuperTalk scripts usually have to resort to external commands (XCMDs) written in a traditional programming language such as C or Pascal.
- The way programs are built dictates the way programs are used. A user familiar with Mac programming restrictions won’t be surprised by the behavior of a Mac application.
- Corporate Mac use inevitably leads to custom applications and the need to tie the Macintosh into existing database and entry applications running on other machines.

But more to the point, we’ve seen over time that virtually all Mac users eventually express an interest in learning how the Mac does what it does. Apple’s documentation doesn’t always explain underlying concepts, and the curious Mac user consequently senses a wall between him or her and the inner confines of the Toolbox built into the Mac ROM. We’ve written this book to help you break through that wall.

**No Degree Necessary**

You won’t need a degree in computer science to become a Mac programmer, but you will need some preparation. We know that the main hurdle in writing a first Mac application is the volume of code it takes to put up a single window. That’s where we can help. Our charter for this book is to help the casual programmer explore the wealth of system software that comes with the Mac.

**How This Book Will Proceed**

Before we get down to writing applications, we’ll take care of a few important preliminaries. We’ll introduce you to the THINK C compiler in Chapter 2, and in Chapter 3, we’ll take a look at C language fundamentals.

Then in Chapter 4, we’ll develop a simple application. We’ll step back in Chapter 5 to see how the Mac manages memory, and then in Chapters 6 through 8 we’ll develop a generic application that will form the basis of all your future programming projects. In later chapters, we’ll use this generic application to create applications that explore the file system, that demonstrate the graphics capabilities of the Mac, and that show how the Mac handles text. We’ll look at a host of other examples of using the Mac’s Toolbox. Every project will be a complete working application.

**If You’re New to C**

For the projects in this book we’ll use the C programming language. If you’re new to C, look at the short C primer in Chapter 3. It’s only a primer. It is not an introduction to programming. We assume that you already know about fundamental programming concepts such as variables, subroutines, and assignments. We don’t expect you to know C itself.

**Know Thy Mac Interface**

You do need to be familiar with the conventions of the Mac interface. We’ll assume that you know what a menu, a dialog box, a button, and a window are, and we’ll focus on how to put them together in an application program.

If you study a Macintosh application carefully, you’ll notice how the menus lead to dialog boxes, how button and command names refer to actions that the user takes, and how well-organized dialog boxes lead the user from the most significant choices to the less important ones. Compare two similar applications—two word processors, say—and notice which elements of the user interface are the same in both programs and which differ. Which interface do you like better? Why? If you can answer these questions, you’re well on your way to understanding how to design a user interface for your programs.

One advantage of programming the Mac over programming other machines is that much of the Mac’s user interface is well defined and directly supported by the operating system. You don’t have to invent a window, a menu, a scroll bar, or any of the other features of the Macintosh interface.

The small price you pay for having a standard interface is conformity. All Mac applications have an Apple, a File, and an Edit menu, and these menus have standard sets of basic commands and standard command-key equivalents for their menu commands. That’s because Mac programmers have agreed to cooperate with Apple and make their applications conform to the Macintosh user interface guidelines. This set of “suggestions” about what a Macintosh application should look like helps users know where things are when they first sit down to use a program.

Your application designs should follow the guidelines—unless, of course, you have discovered a much better way of doing things; that’s progress, after all. If your modifications to the standard interface don’t work, if they don’t feel right, your users will let you know about it. One of the first applications for the Macintosh was a document outliner very Mac-like in all respects but one: The way the user scrolled text differed from Apple’s specification. Users complained directly to the company, in letters to the editor in Macintosh magazines, in online comments on services like CompuServe, and in many other ways. The program was redesigned, and later versions worked according to the guidelines.

The sample applications in this book follow Apple’s guidelines, and if you’ve used your Mac for some time, you probably have a pretty good idea of how a Macintosh application should look and feel. If you need information about a particular interface issue, you’ll find the complete user interface guidelines in two books: *The Programmer’s Introduction to the Macintosh* and *The Macintosh Human Interface Guidelines*, both published by Addison-Wesley. As a casual programmer, you’re not likely to need these books, but they do deserve a browse. A professional programmer should have both.

### Know Thy Toolbox

You also need to know something about how Apple divided the Macintosh’s resident software into Toolboxes and Managers—roughly, Toolbox routines for the interface goodies and Manager routines for operating system chores like file handling. The six-volume *Inside Macintosh*, also from Addison-Wesley, is the definitive source for Macintosh programming information. Known tersely as *IM* by those of us who frequent the Apple Macintosh Developer Technical Support electronic mail facility, *Inside Macintosh* is the basic reference for system routine call syntax and for data structures. You don’t need to rush out and buy all six volumes—the whole set represents a sizable investment. Start by buying Volume I. As you progress, you’ll know when it’s time to get the other books.

Volumes I and II contain the original Macintosh programming information. Volume I focuses on the user interface, and Volume II on the behind-the-scenes operating system activity. You’ll use Volume I a lot and Volume II hardly at all. Volume III describes the Macintosh hardware and fills the reader in on changes to the Toolbox and Manager routines between the initial Mac (128K) and the Lisa XL. Volume IV covers changes that resulted from the introduction of the Plus and 512E models. Volume V describes changes that resulted from the addition of color and other hardware innovations with the Mac II. Volume VI describes changes that accompanied the additional Mac models introduced in late 1990 and System 7.0, the new Mac operating system introduced in 1991.

You might think of the volumes after I and II as “delta documents,” documents that describe only changes or additions but that do not recap the original information. This makes *IM* somewhat hard to use. For casual programming, we recommend that you try a third party reference. A professional programmer will need all the &IM* volumes at some point in his or her career because their detailed information will eventually come in handy for troubleshooting.

A good third party resource is *Encyclopedia Mac ROM*, by Mathews and Friedland, from Brady Books. A software utility Kurt finds useful is the *Inside Mac Desk Accessory*, a shareware utility written by Bernard Gallet. This DA, available directly from its author, is first-rate—better than anything available from commercial publishers. It contains a database of the Toolbox calls and the data structures found in *IM*. It really helps to have the information online and quickly accessible. A commercial product of this type is available from Addison-Wesley, but we find the shareware utility much more useful.

# Know Thy Programming Language

Picking a programming language is an emotionally charged decision. Everyone has his or her own ideas about which one is best. For the Mac, five general choices were available to us: HyperTalk, Basic, Pascal, C, and assembly language.

We ruled out HyperTalk for many reasons. Although it comes free with every Mac, it simply doesn’t have the power to control all aspects of the Macintosh by itself. XCMDs are available, but they must be programmed in one of the other languages. Moreover, the disjointed nature of HyperTalk scripts, scattered as they are within HyperCard stacks, makes it difficult to present finished solutions. HyperTalk also brings along a large overhead because it is interpreted by HyperCard, which eats up a lion’s share of the available memory in a 1-megabyte Mac. Finally, HyperCard is mostly object oriented, which we feel makes it less suited for procedural operations such as scientific calculations or for carefully controlled sequences of events.

Basic has many appealing features. You can run it interactively, so you don’t have to wait until compile time to see whether a statement is going to execute the way you think it will. Basic is easy to understand. It’s inexpensive and readily available. Unfortunately, Basic belongs to the old sequential world of computing, in which instructions are always executed in a particular order. (In the original Basic, that order was dictated by line numbers.) The event-driven Mac interface isn’t well suited to this sequential control. Moreover, the Macintosh ROM expects to deal with special groupings of data called data structures, and Basic has no way of dealing with data structures directly, which means that Mac programs would have to be more complicated than they might otherwise have to be. Pascal has a reputation as a fine teaching language. Better still for our purposes, it was the language of choice internally at Apple as the Macintosh was developed. The data structures that the Toolbox and the operating system expect to deal with are forms Pascal directly understands. Two problems made us avoid Pascal: Coding pointers and handles—two basic types of Macintosh data—is somewhat cumbersome in Pascal; and accessing low-level data such as individual bits is possible only through Toolbox macros.

Assembly language, of course, lets you access the bits and bytes in memory, but we ruled it out because creating data and control structures tends to become quite complex. Assembly language has no data structures or structured loops, leaving you to invent them yourself. And good assembly language code is difficult to read quickly.

That left us with C, the language we have used for most of our programming projects in the last five years. We feel strongly that C does a better job of showing the control and manipulation of data structures for Mac programming than any other language does. The only real drawback to using C is that the Macintosh’s native language is Pascal, and C data types and calling conventions differ from their Pascal counterparts. Another drawback is that C lets you do some very stupid things. C is a laissez-faire language: It usually lets you do what you want to do as far as assignments and pointer arithmetic are concerned, but, because it’s so lenient about checking for compatible data types across assignment operations, it will let you do nonsensical things in your code without much warning or complaint from the compiler.

C assumes that you know what you’re doing. This is the Pascal aficionado’s major complaint against C, but it’s a feature that we enjoy. Be forewarned, though, that novice C programmers are virtually guaranteed to fall into one of C’s traps at some point early in their careers. Guard against C’s traps by double-checking every change you make to a program and by constantly double-checking your data types.

As a prospective Mac C programmer, you need to be familiar with the syntax and semantics of the C programming language. Sure, we’ll supply you with working code, but you’ll need to understand the language if you want to write your own code and get the most benefit from this book. You’ll need to learn the fundamentals of C—variable declarations, assignments, function definitions, function calls. Don’t worry if you don’t have any experience with the language—we’ll meet you halfway. Chapter 3 is a brief C primer you can use as a reference. We do suggest that you pick up a copy of Kernighan and Ritchie’s *The C Programming Language*, published by Prentice Hall, if you’re serious about learning C. This classic, known by the blue C on its cover, was the first book on the language. Recently revised, K&R is still our favorite, despite competition from dozens of other introductory books on C.

Although the Kernighan and Ritchie book is excellent for learning how to use C, it doesn’t teach you a thing about programming the Mac. As you read along in this book, we’ll alert you to common pitfalls that await the new C programmer of the Mac, and we’ll beef up our efforts when we get to more advanced topics like data structures, pointers, and dynamic memory allocation—and their relationship to the Mac. Of course, we think the best way to learn a language is by example. In each chapter, study the examples and read the code. We can’t emphasize that enough. The code will teach you more about how to put a Macintosh application together than any description can.

If you are a C programmer experienced in other development environments, forget everything you’ve learned about console-based systems. You’ll find that all the basic `stdio` library routines for console input and output—routines such as `getch`, `scanf`, and the ubiquitous `printf—are provided with the THINK C environment, but you’ll be hard-pressed to find any real use for them in a Mac application. The good news is that you already know a lot about C that you can put to immediate use on the Mac.

**Know Thy Development Environment**

But why Symantec’s THINK C? Why not Apple’s own Macintosh Programmer’s Workshop C (MPW C)? That’s an easy question to answer. MPW C is definitely a big-league compiler, but with roots in UNIX, it isn’t exactly the interactive, event-driven product Mac users are used to. MPW C comes with all kinds of special tools, but they’re all invoked with cryptic command-line instructions or macros. The THINK C environment follows the Mac Interface Guidelines; it’s easy to learn; it’s as full-featured as MPW C; it produces code that is as small, reliable, and fast as MPW C code; and it costs less.

We’re not the only ones who think so, either. Some of the largest development houses in the industry, companies like Aldus (PageMaker, Freehand, and Persuasion), Claris (MacWrite and others), and Quark (XPress), have selected THINK C as their primary development environment. Indeed, we would guess that if you were to poll all applications developers, you’d find that more commercial programs had been developed with THINK C than with any alternative.

Although THINK C is one of the easiest development environments to use, the next chapter is geared toward those who have little or no experience with THINK C. We might even have a thing or two to teach THINK C veterans.

**The Programming Process**

All of this brings us to the actual act of writing a program for the Mac. We’ll start with an oversimplification. A program usually starts with your idea for a computer-based tool, which you then break into smaller, logically oriented pieces. Until you’ve figured out what it is you’re trying to accomplish, you shouldn’t start to code. The first steps often take the form of notes, diagrams, samples of screens, or printouts. The more you refine your ideas before you sit down to code, the more likely you are to produce a useful program.

A case in point: We spent two man-years sketching out our ideas and designing before we started the programming that eventually became Tycho Table Maker, our commercial table-editing program. We spent much of that time looking at examples of tables and extrapolating the basic concepts our program had to treat. And we looked at how to put information into Tycho. (It doesn’t make any sense to retype something that already exists, does it?)

Our ruminations resulted in several paper designs for specific pieces of Tycho. In particular, we spent a great deal of time designing the underlying database the program uses. But we wouldn’t have even realized that a table editor needs an underlying database if we hadn’t done the preplanning. Had we simply jumped in and started to program, we probably would have spent a great deal of time inventing a database by trial and error—which, as you might suspect, is not the most efficient way to design software.

When you fully understand the product you want to create, it’s time to start programming. You take your ideas and your paper design, and you begin to type source code into an editor. Source code is a sequence of computer-specific instructions for performing the process that carries out your program idea. All a computer does is process a sequence of instructions in a tightly controlled fashion. The real key to successful programming, therefore, is to identify the right process to encode, to think of all potential exceptions to the process that your program might encounter, and to keep the sequence of execution correct.

A good computer language helps in these tasks. C is a structured language—which lets us create data structures and control structures that imitate the real-life elements we try to model in programs. C also provides the low-level access to data objects, such as pointers or the bits of a data word, that we need for writing efficient programs.

THINK C has an adequate editor for typing in and organizing your source code. It also has a compiler and a linker and a debugger for examining your program in detail as it executes. Until this point in the programming process, you have used only the editor as you typed in your first-pass source code. Next in our programming sequence, you use the compiler to parse the source code instructions you typed into machine code, usually called object code, that the computer can understand directly. On the Mac, that object code must be linked into a file that the operating system can understand and execute. Figure 1-1 on the next page illustrates the process.

Finally, you’ve got a runnable application. Of course, if you didn’t do a good job of designing it or if you put in illogical or nonsensical instructions, it might not run too well. Then you use the Debugger to explore your code. Rarely does the first pass at a program come even close to working. (You’ll have an advantage with the examples in this book, though, because we’ll provide source code listings that we know will work.) In real life, you often find yourself back at step 2 (entering and modifying source code) or even at step 1 (isolating and designing key modules of the program on paper). And so it goes. You edit, compile, link, and run your program, find the errors, and go back through the sequence again

**What to Do Next**

To get yourself ready:

■ If you’re a newcomer to programming, get some exposure to programming concepts and terms. We’ll explain advanced concepts, but you need to know what a bit, a byte, an assignment, a loop, and a conditional expression are.

■ Get familiar with the C language. Chapter 3 is a helpful introduction, but we also recommend that you read *The C Programming Language*, Second Edition, by Brian Kernighan and Dennis Ritchie.

■ Know why you want to program the Mac. Are you merely curious about what it takes? Do you have specific needs that aren’t met by existing programs? Are you looking for shortcuts? Do you want to be the next Andy Hertzfeld? If you understand why you want to know about programming, you’ll get more from this book.

So start up your editor. You’re about to tackle your first Macintosh program.