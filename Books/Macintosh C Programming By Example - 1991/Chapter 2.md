# USING THINK C

THINK C is one of the best-integrated programming environments to come along in years. You edit, compile, link, and run your program without leaving the environment. And, under MultiFinder with at least 2 megabytes of memory, you can use the THINK C Debugger to trace the execution of your program, stepping statement by statement through your source code.

The editor in THINK C behaves as any Macintosh text editor or word processor you might be accustomed to does. As in most programming editors, text doesn’t wrap at the end of a line as it does in a word processor and the editor supports automatic indentation of subsequent lines, which is handy for writing structured code. You can cut and paste text, find and replace text strings, and take advantage of other features that are useful for programmers, such as the ability to find curly brace pairs.

THINK C’s built-in compiler converts your source code into machine readable instructions and stores this object code in the project file. You might be accustomed to development environments in which you have to keep track of the object files, the .o or .obj files that the compiler creates as a result of compilation. You won’t have to do that in THINK C.

Likewise, linking the code, the final step in creating a runnable application out of source code, is automatic in THINK C. Linkage proceeds as a result of running the application. Because the objects are maintained and kept hidden by THINK C, there’s no need for a script to control linkage.

## The Development Folder

If you haven’t installed THINK C on your hard disk yet, now’s your chance. You’ll find that THINK C works better if you follow the file system organization scheme we describe in this chapter.

Keep all your development projects and the compiler in subfolders within one main folder, the Development folder. (We usually put this folder at the top of the file system hierarchy, although you can put it anywhere.) Inside the Development folder is a folder named THINK C, in which the compiler, the debugger, and associated files will reside. Each programming project folder will reside at this level.

The most important files on the THINK C distribution disks are the THINK C integrated programming environment file and the THINK C symbolic debugger file. Put these two application files, named THINK C and THINK C Debugger, in the THINK C folder. You'll also need ResEdit, the Apple resource editor, if you're to follow some of the examples in this book, so copy ResEdit from the THINK C distribution disks into the THINK C folder.

Along with the compiler environment and the debugger come programming libraries, header files, the precompiled headers file, library sources, and the class library. Minimally, you'll need to put the programming libraries and the header files in the THINK C folder. Put the file MacHeaders, the Mac #includes folder, the Mac Libraries folder, and the C Libraries folder in the THINK C folder.

Although we don't use the class library for any of our programming projects in this book, you might want to play with some of the THINK C object-oriented programming (OOP) examples, so you might as well install those files now. Add the oops Libraries folder and the THINK C Class Library folder to your THINK C folder.

If you have a lot of disk space, copy the programming examples from the THINK C distribution disks. The best way to learn programming is to study actual programs that work. The more code you have access to, the more approaches you'll have to your particular programming problems. Some of the example projects are good for starting small applications as well as Control Panel utilities (CDEV) and desk accessories (DA). Instead of putting these code example folders in the THINK C folder, put them in the Development folder, at the same level as the THINK C folder in the file system organization.

Another likely candidate for copying onto your hard disk is the C Library sources folder. This folder contains all source code for the THINK C programming libraries. (We'll talk about these in detail in a moment.) Again, put this folder in the Development folder, at the same level as the THINK C folder. The other file folders at this level can be independent projects, other programming libraries, or other developer's tools.

Finally, put all the project folders from the source code disk for this book into your Development folder. Figure 2-1 illustrates a typical layout of the Development folder.

With all these files in the Development folder, you might wonder how THINK C finds a particular file. It makes use of two hierarchies: the THINK C tree and the project tree. The THINK C tree encompasses every folder and file in the THINK C folder; the project tree encompasses every file in a project folder. This is why we advised you to put project folders at the same level as the THINK C folder. If you put the project files in the THINK C folder, THINK C would search all your project files every time it looked for a file, and it would run into trouble if you had multiple source files of the same name.

```
Develop ƒ THINK C 5.0 Folder
THINK C 5.0
Mac Libraries
Mac #includes
cdev stuff
DA stuff
THINK C Debugger 5.0
oops Libraries
C Libraries
THINK Class Library 1.1
DA shell

THINK C 5.0 Utilities
Rez Utilities compare oConv Prototype Helper

THINK C 5.0 Demos
Bullseye Folder Hex Dump DA MiniEdit Folder sample cdev OOP Demos

TCL 1.1 Demos
Art Class Folder NewClassDemo Folder Starter Folder TinyEdit Folder cdev stuff DA stuff

Hello f hello mac .c Hello Project

GenApp f GenApp7 GenApp7.rsrc GenApp Src GenApp Hdr

C Library Sources f alloc.c arith.o assert.o atexit.c ...
```
**Figure 2-1.**
*Layout and organization of a generic development folder. The THINK C folder, Utilities folder, Demos folders, C Library Sources folder, and project folders are all at the same level.*

## Programming Libraries

The header files that come with THINK C and the programming libraries take up the bulk of the THINK C distribution disks. You can’t write a Macintosh application without the Macintosh header files and the Macintosh libraries. The Macintosh headers contain definitions of the Macintosh data structures, and the libraries contain the hooks into the Macintosh’s programming Toolbox, which includes the routines to display a window, read a menu selection, and get a mouse click.

A programming library is an organized collection of program pieces. These pieces, called functions, can be used by any program that connects, or links, the library with the program. The THINK C environment has a built-in linker for this purpose.

Library functions provide a software toolbox for your program. Included in the many libraries that are shipped with THINK C are routines to process strings, format numeric values, search a list, sort a table, and perform file I/O (input and output), along with a wealth of other routines.

The functions in a library are in compiled, or machine readable, form. The human readable source code for a library is not usually available or, in the case of a commercial library, is available only for a price. This is not the case with THINK C, whose library sources are included on the distribution disks. We salute Symantec for including these sources with the library functions.

Library code is (usually) thoroughly debugged. When you modify proven source code, you run the risk of introducing new bugs into the code. You can’t modify library code directly in binary format, so the use of compiled libraries can contribute to software reliability. This impenetrability of library code means, though, that each library function needs complete, descriptive documentation of its name, action, inputs, and outputs. The inputs to a function are called “parameters,” or, informally, “arguments.” The output of a function is known as its “return value.”

**The Standard Libraries Reference**

The THINK C library functions are documented in the **Standard Libraries Reference** manual that comes with THINK C. Symantec has borrowed the style for the entries in this book from the old **UNIX Programmer’s Manual**, the original source of C function library documentation. The entries appear one function per page, with four main sections for each entry. At the top of the page, the function name appears, followed by a one-line description of the function’s action. The syntax, or usage, section follows. Here’s an example, the syntax for the function `toupper()`:

```c
#include <stdio.h>
int toupper (char c);
```

The first line tells us that we need to include `stdio.h` in order to use this function. The `.h` indicates that `stdio` is a header file. (We’ll go into header files later in this chapter.) The second line tells us that `toupper()` accepts a character argument and returns an integer. This gives us enough information to use `toupper()` in an application:

```c
#include <stdio.h> /* placed at top of source file */
...
myFunction()
{
    int upperC; /* declared inside function */
    char c;
    ...
    upperC = toupper (c); /* converts the character */
}
```

The most important section of a library reference entry is the description, which tells you what the function does. The description for `toupper()` says that it returns the uppercase equivalent character of a lowercase letter c...,” so you know that `toupper()` converts lowercase letters to uppercase.

If the function returns a value, the return value section of the entry describes the range of data values or the error value you can expect when the function returns. The function `toupper()` returns the uppercase equivalent of a lowercase letter. (The manual doesn’t tell you what `toupper()` returns if the original character wasn’t in the lowercase letter range—you have to find that out for yourself.)

The reference manual is organized by library. The names of the several libraries that come with THINK C differ depending on the version of the compiler you are using (version 3.0; version 4.0; or version 5.0, the new System 7.0 compatible compiler). Here are some of the important THINK C libraries:

**MacTraps** This is the most important library that comes with THINK C because it contains all references for the Macintosh Toolbox routines. Any program that uses a Toolbox function needs to link with the MacTraps library. In other words, virtually any program you write needs to link with MacTraps.

**ANSI** The ANSI (American National Standards Institute) committee concerned with C has been active for years in an attempt to standardize the language. The functions in THINK C’s ANSI library support the new standard. The library contains all of the I/O functions, including `printf()`, file stream utilities, and character I/O primitives. It also contains floating point support. If your application includes floating point (non-integer) calculations, you need to use the ANSI library.

**ANSI-small** This library is similar to the ANSI library, but it doesn’t include the floating point routines. Use this library if your application does not use floating point calculations and you want to save some space.

**math** If you plan to use the C math functions, such as the square root, trigonometric, or logarithmic functions, you need to use the math library.

**unix** The unix library is provided to help you convert UNIX applications to Macintosh applications. Some of the unix library functions don’t do anything—`setpid()`, for instance, is provided simply for compatibility. We’ve never needed to use this library when programming the Macintosh, and unless you’re coming from the UNIX world, you won’t either.

## Header Files

THINK C comes with scores of header files, whose names characteristically end with the `.h` extension. The contents of these files are organized along the lines of *Inside Macintosh*, by Toolbox manager. The names of the files differ depending on which version of THINK C you’re using. If you’re using anything other than THINK C 5.0, for example, Event Manager constants and structures are defined in the file `EventMgr.h`, QuickDraw stuff in `QuickDraw.h`, and Window Manager structures in `WindowMgr.h`. In THINK C 5.0, the file names conform with those used in MPW C: `Events.h`, `QuickDraw.h`, and `Windows.h`.

C programmers use header files to define constants, macros, data types and structures, variables, and function prototypes. In programming, one header file is usually included by multiple source files, so the header file serves to fix a constant or data structure definition for all files in a project. Defining something, like a constant’s value, in one place is organizational good sense: If you need to change the value, you need to do it only once. As your programs become larger and more complex—perhaps encompassing dozens of source code files—the proper maintenance of header files becomes very important.

Header files are sometimes called “include” files because their contents are included in the compilation stream with the contents of other files. In a C source file, you include the contents of one file in the compilation stream of another by using the #include directive. Consider the following source file, in which we’ve used #include to include two header files:

```c
#include "AppConstants.h"
#include "FileUtil.h"

main ()
{
  ...
}
```

In Figure 2-2, you see a diagram of the result. The compiler reads the contents of AppConstants.h and FileUtil.h before it looks at the source file code.

![Contents of THINK C editor]

**Figure 2-2.**

*The compilation stream using the #include directive. The compiler sees AppConstants.h, then FileUtil.h, and then the main() code.*

A note about the syntax of #include statements: When the compiler sees double quotes around the header file’s name, as in

```c
#include "constants.h"
```

it searches the current project folder tree, looking for the file constants.h. Conversely, when the compiler finds angled brackets, as in

```c
#include <QuickDraw.h>
```

it looks for the header file in the THINK C folder tree. The angled brackets signify that the file is a compiler-supplied header file that resides in the THINK C hierarchy. If you’ve set up your Development folder as we’ve recommended, you must use the angled bracket form for THINK C header files.

**MacHeaders**

*Inside Macintosh* is the standard guide to which header files you’ll need to include in a particular source file. If you are using the Window Manager and accessing a *WindowRecord* data structure, you’ll need somewhere in your source file the statement

```c
#include <Windows.h>
```

if you’re using THINK C 5.0 or

```c
#include <WindowMgr.h>
```

if you’re using an earlier version of THINK C. Your program needs to know about the Window Manager data structures. (If you don’t know what a data structure definition is now, don’t worry—we discuss this in the next chapter.)

The problem with this organizational convention is that you need *Inside Macintosh* to get started. The number of files that you’ll need to include for most applications runs high. Beginners find that the compiler’s syntax-checking error messages can get to be pretty annoying before they come up with a combination of header files that includes all the structure definitions.

One solution to this problem is to use our Generic application, discussed in Chapters 6, 7, and 8, which already includes the necessary header files.

Beginning with THINK C version 3.0, Symantec came up with an elegant solution to this problem: precompiled headers. MacHeaders, the precompiled header file supplied with THINK C, contains definitions for most of the commonly used managers. The file loads more quickly during compilation because it is in binary form, unlike conventional text header files. And you never need to load a manager include file because the compiler includes the MacHeaders file automatically if you set the MacHeaders compiler option. (You’ll find compiler options in the THINK C editor’s Edit-Options dialog box.)

**Custom MacHeaders File**

If you really know what you’re doing with header files, you can build your own MacHeaders from the text file Mac #includes.c, using the Precompile command on THINK C’s Source menu. Just modify Mac #includes.c so that it will include the files you’re interested in, and precompile it. If THINK C is to recognize the new file, you have to name it MacHeaders, so you might want to rename the original MacHeaders to avoid duplication.Page 8 of 26
The Project Folder

A project folder holds all a project's files. Each of your programming projects should be in its own folder. A typical project folder contains four types of files:

- The project file
- C language source files, called ".c files," usually kept in a subfolder
- C language header files, usually kept in a subfolder
- The project resource file

**The Project File**

The project file is the master file for the project. Each programming project centers around the project file, which contains everything THINK C needs to construct the application from your source files. All project management is done from this file. When you open the project file in THINK C's integrated environment, a list of the source files appears in the project window, as shown in our example in Figure 2-3. To open an individual source file, double-click on its name in this window. You can use the arrow keys to move the selection bar up and down in the window. If you type the first few letters of a file's name while this window is active, the selection cursor jumps to that file's name. If there are multiple files with similar names (for example, FileBuf.c, FileMgr.c, and FileUtil.c), the Tab key will move you to the next file name that contains the matching first few letters.

| Name           | obj size |
|----------------|----------|
| AboutBox.c     | 0        |
| AppInit.c      | 0        |
| DialogUtil.c   | 0        |
| Display.c      | 0        |
| DocUtil.c      | 0        |
| FileUtil.c     | 0        |
| MenuUtil.c     | 0        |
| MiscUtil.c     | 0        |
| Shell.c        | 0        |
| WindowUtil.c   | 0        |
| ANSI-small     | 0        |
| MacTraps       | 0        |

**Figure 2-3.**
*The project file window for miniGenAppπ from Chapter 6. The file names are listed in this window with the file sizes.*

The Full Titles option (from the Windows menu) is useful when you have more than one version of the source code on disk. It displays the file's path in the window title.

For building programs, THINK C offers a built-in, UNIX-like make facility. (When we say UNIX-like, we mean in essence, but certainly, we hasten to add, not in appearance.) This facility keeps track of compilation dates and dependency information for your source files and stores the data in the project file. In UNIX and MS-DOS terms, this means there's no makefile. When you make a change to a file, the date and time are noted internally. When you try to run your program, THINK C reminds you that the project needs to be brought up to date. You can also configure the environment to automatically make the program before you run it, again with the Edit-Options dialog box.

There are no .o files with THINK C. The project file holds the object code (machine language instructions) that is compiled from the source files. The project file also contains debugging data and linking data, such as symbol and line numbers, and code resource segmentation data. As a result, the project file can grow to be very large. Using the Precompiled Headers option can help reduce the size of your project file, but you won't have to worry about the projects in this book taking up too much space.

**C Source Files**

The C language source files constitute your program code and end with the .c suffix. Generally, you'll have more than one source file to a project. To minimize the number of files in the project folder so that it doesn't become cluttered, put all your source files in a subfolder of the project folder. We organize all of our more extensive projects this way. For example, if the project file name is miniGenApp, we name the source subfolder miniGenApp Src.

**C Header Files**

Header files end with the .h suffix. They contain constants and the definitions of data types and structures, variables, and function prototypes.

**Source File Suffixes**

The .c or .h suffix in these file names is a holdover from command-line system days. Because the Macintosh system software designers chose a free-form file naming convention, we don't have to suffer with an abbreviated name such as ACCNTS09.DBF, as our MS-DOS counterparts do. The free-form convention means that file names can be more descriptive. We can change ACCNTS09.DBF to Sept. Accounts. (Periods can appear anywhere in the name.) On the Mac, there's really no need for a file name "extension" to classify the file. The Finder notes the file's origin internally, so we don't need the .DBF extension to tell us that this is a dBASE file; the icon tells us that.

Source file names in THINK C are a different story. C source file names must end with .c. That's how THINK C recognizes them as C language source files. Likewise, by convention, header file names always end with .h. It's a throwback, admittedly, but for now, that's the way it is.

**The Project Resource File**

The final item you might put in the project folder is the project resource file. This file contains the program’s resources—menu descriptions, dialog box item lists, PICTs, control definition functions, or other resources the application needs at runtime. We build all our resource files with Apple’s ResEdit, one of the so-called resource editors. Symantec ships ResEdit with THINK C.

If you come from a different programming environment, the resource file concept is probably new to you. The principle behind resource files is that it’s advisable to split program code from the user interface items, that any messages that a program displays to the user belong in the resource file, not in the source code. The idea is that if an application’s interface items, such as strings, dialog box contents, menu titles, and other items of text within a program, are accessible from an outside source, the program can be easily converted to another language system.

This principle works pretty well, and we follow it in our examples. Anything a user sees in a program we place in the program’s resource file. Each of our example programs has a resource file (except our first example, Hello Mac!, which doesn’t really count as a full-fledged application).

In a stand-alone application, the program’s resources are built into the program file, in its resource fork (a topic we’ll cover in Chapters 10 and 11). But in the THINK C environment, the application has to have access to the resource file. There are two ways to set this up.

If you name the resource file correctly, THINK C automatically opens the resource file when you run your program. You should name the resource file after the project file name and give it an .rsrc extension. If the project name were GenericAppπ, for example, the resource file should be named GenericAppπ.rsrc. The resource file has to be in the same folder as the project file.

The alternative is to use the **OpenResFile** call in your program. For example, the call

```c
OpenResFile ("GenericAppπ.rsrc");
```

will open the resource file for your program’s use. You must make this call before your program accesses any resources. A program will eventually bomb if it doesn’t have access to the resource file. There is no warning or safeguard against this in THINK C, and there shouldn’t be. C programmers are masters of their own destinies.

## Working with THINK C

Application development centers on the project file, and the THINK C environment won’t operate unless a project file is open. So, the best way to begin a THINK C session is by double-clicking on the project file name. If you open THINK C without a project file, the environment will ask for one by presenting the Open Project dialog box. You must either select a project file to open or create a new project file.

You’re going to become very familiar with the features and commands in the THINK C environment as you spend hours and hours getting your programs up and running. You might spend most of your time in the editor. The multiwindow editor supports just about everything you’ll need to edit your program’s code. It is fast, it is highly functional, and it works as most Macintosh editor applications do.

After the project file is open, you open the source code files by double-clicking on their names in the project file window or by choosing Open from the File menu. THINK C’s Windows menu is handy for managing these files—it lists each open file. You bring an open window to the top by selecting it. The first nine open files get Command-key equivalents, Command-1 through Command-9, which you can use to bring a window to the top. Command-0 selects the project window.

You save a file by using the File menu’s Save command. Save As works a little differently than you might expect. Save As not only creates a new file with a new name, but it also changes the file name stored in the project file. If you want to save a file with another file name without changing the name in the project file, use Save A Copy As.

You edit text in a THINK C editor window as you would in a Macintosh word processor. Text does not wrap in the editor; you must use the Return key to start a new line. The editor automatically indents (autoindents) each line of text, which means that the next line begins under the first character of the previous line.

You use the mouse to select text ranges. A double-click selects a word; a triple-click selects a line. There’s no overstrike mode as there is in WordStar-like editors. The editor is always in insert mode. You overstrike text by selecting it and then typing the replacement text. Typed text always replaces any selected text on the Mac.

You can use the arrow keys to move the text cursor around the screen. Alone, the arrow keys move the cursor character by character horizontally and line by line vertically. The Option-arrow key combinations move the cursor as far as it can go in a particular direction: Option-up moves the cursor to the top of the file; Option-down to the bottom of the file; Option-left to the beginning of the current line; Option-right to the end of the current line. The Shift-arrow key combinations extend the

**Changing the Default Font**

The THINK C editor uses a 9-point Monaco font as its default font. Some people don’t like this font for one reason or another. The default font number, size, and tab size reside in the second, third, and fourth words of the THINK C CNFG #0 resource. You can change these values with a little ResEdit-style surgery on your THINK C application. Remember to enter these values in their hexadecimal equivalents. These are nice features if you like to work without taking your hands from the keyboard. Pressing the Enter key scrolls the window contents so that the text cursor is in the middle of the window, which is handy for finding your place after scrolling around.

The editor’s Edit menu supports full cut, copy, and paste operations, which are all supported by their conventional keyboard shortcuts (Command-X, Command-C, and Command-V). You can shift blocks of text left or right by using the Shift Left and Shift Right commands on the Edit menu or by using the keyboard shortcuts Command-[ and Command-]. This feature is illustrated in Figure 2-4. The Balance command (Command-B) on the Edit menu is useful for finding syntax errors caused by your forgetting to close a block with a curly brace.

**Figure 2-4.**

*Shifting text right. To use the Shift Right command, select the lines you want to shift, as shown on the top screen, and then press Command-]. The text shifts right, as shown on the bottom screen.*

If you hold down the Option key or the Command key and hold down the mouse button while the cursor is in an editor window title bar, THINK C displays a pop-up menu that lists the names of the header files included by the project file. If you then select one of the header file names, THINK C opens that file in the editor. This feature uses some internal project file information, so the source file must be part of a project and must have already been compiled for this feature to work.

**Searching for Text Strings**

The Search menu supports full text search and replace capability. You can search for text strings in single or multiple files. The search mechanism finds strings that match the search string and can replace them with the replace string. You enter the search and replace information into the Find dialog box shown in Figure 2-5, which appears when you choose Find from the Search menu (or use the keyboard equivalent, Command-F).

**Figure 2-5.**
*The Find dialog box. Note the check box options in the lower left corner.*

After you specify a search string, you can find each occurrence of the string by using Find Again (or Command-A). You can replace the occurrence of the search string with the replace string by using Replace (Command-P) or Replace and Find Again (Command-W), which replaces the current selection and moves the cursor to the next occurrence of the string.

Say, for example, that you want to change the name of the global variable `gKill` to the more descriptive `gDeleteRefs`. A global variable can occur in any file, so you’ll have to search the entire project. Here is how you go about this in a project:

1. Find the first occurrence of `gKill`.
2. Select the word by double-clicking on it.
3. Choose Enter Selection from the Search menu (or press Command-E). This sets the string for the search to the selected text, `gKill`.
4. Type `gDeleteRefs`. Because `gKill` was selected, the typing replaces this string.
5. Double-click on `gDeleteRefs` to select the word.
6. Choose Copy from the Edit menu (or press Command-C) to copy this word to the Clipboard.
7. Choose Find from the Search menu (or press Command-F). Select the Replace With edit box, and press Command-V to paste the Clipboard contents into the dialog box. This sets the replace string to `gDeleteRefs`.

The Find dialog box reflects the search and replace options shown in Figure 2-6. If you were replacing the search text in this source file only, you’d be ready to go. But because you’re looking for all the instances of a global variable, you need to set up the search so that THINK C will scan all source files.


8. Click on the Multi-File Search check box. You’re presented with a dialog box that looks something like the one shown in Figure 2-7. Click the Check All button, and click OK.

You are ready to start the search. You can replace occurrences of `gKill` one at a time with Replace and Find Again (Command-W) or replace all occurrences in a file at once with Replace All (no keyboard equivalent). When you’re ready for the next file, choose Find In Next File from the Search menu (Command-T).

To find function and global variable definitions, hold down the Option key or the Command key and double-click on the function name or the variable name. THINK C will open the source file in which the function or variable is defined and find the first instance of the string. If you include all your global variables in a header file, THINK C will open the source file containing the definition of `main()`.


```
Search for:
gWindow

Replace with:

Whole Words Only
Wrap Around
Ignore Case

Grep
Multi-File Search

Find
Don't Find
Cancel
```

**Figure 2-6.**
*The Find dialog box. `gKill` is the search string and `gDeleteRefs` is the replace string.*

```
Search for:
gKill

Replace with:
gDeleteRefs

Whole Words Only
Wrap Around
Ignore Case

Grep
Multi-File Search

Find
Don't Find
Cancel
```

**Figure 2-7.**
*The Multi-File Search dialog box.*

## Using the grep Option

> ### grep
> THINK C’s grep feature is derived from a UNIX utility program of the same name. The name grep is an acronym that stands for (g)lobal (r)egular (e)xpression and (p)rint. When it comes to esoterica, UNIX excels. The names of its five string-processing utilities—awk, grep, sed, lex, and yacc—are classic examples of the jargon that permeates the computer sciences. The name awk is made up of the first letters of its authors’ names: (A)ho, (W)einberg, and (K)ernighan. The name sed is for a (s)tream (ed)itor. The lex utility builds (lex)ical analyzers, and the name yacc is an acronym for (y)et (a)nother (c)ompiler (c)ompiler, which isn’t an entirely accurate name because it’s only a parser generator. That’s some computer programming folklore. We just thought you’d like to know.

The search routine has a built-in grep facility that lets you use a regular expression instead of a literal string as your match string. Unlike a literal string, which specifies only one string to match, a regular expression specifies a set of strings to match. A regular expression contains both alphanumeric characters and operator characters, called “metacharacters”, that control comparisons, repetitions, and other features of the expression-matching facility. Some examples will illustrate what we mean.

Any single character matches itself. For example,

`a`

matches *a*. You can freely concatenate expressions, just as you concatenate letters to make words. Any string as a regular expression therefore matches itself. Thus, the regular expression

`hello`

matches any occurrence of the string *hello* in the source text. The dot (period character) matches any single character. Therefore, the regular expression

`.`

matches *a*, *1*, *K*, *.* (the period itself), or any other single character. By itself, the dot is useless—it matches everything. But used with other characters, it becomes very handy. The expression

`c.t`

matches *cat*, *cot*, *cut*, *cmt*, *c_t*, and any similar string. If you want to find the dot and only the dot, however, you need to tell grep to treat the dot as a dot, not as a metacharacter. You do this by “escaping” the metacharacter with the backslash. For example, if you wanted to find all periods, you’d enter

`\.`

The expression that matches the backslash itself is

`\\`


If you wanted to find a word with an embedded blank, you’d escape the blank, as in

`hello\ world`

The ^ operator specifies the beginning of a line. Therefore, the expression

`^c.t`

matches the word cat if it occurs at the beginning of a line, but not the second syllable of the word concatenate. The operator \$ matches the end of a line. The regular expression

`^...$`

matches all lines that contain a single three-character word.

The \* operator specifies zero or more occurrences of an expression. The expression

`ca*t`

matches *cat*, *caaaaaaaaaat*, or *ct*.

You specify a “character class” between square braces. A character class is a set of characters for grep to match against. The expression

`[bchm]`

matches a single character from the set b, c, b, or m. Again, this match might not be useful by itself, but its value becomes evident when you concatenate character class expressions, such as

`[bchm]at`

This expression matches the words *bat*, *cat*, *bat*, and *mat*.

You use the - operator in a character class to specify a range of characters. For example,

`[a-z]`

matches any lowercase character, and

`[A-Za-z]*`

matches all text words. If you know C language syntax, you can use regular expressions to find text words. (If you don’t know C syntax, you’ll learn about it in the next chapter.) For example, the expression

`^[A-Z_a-z][0-9A-Z_a-z]*[ ]*()[ ]*$`

works pretty well for finding your function declarations, and

`^[A-Z_a-z][0-9A-Z_a-z][ ]*=([ ]*^[A-Z_a-z][0-9A-Z_a-z][ ]*;$`

finds most assignment statements.

Within the bounds of a character class, the ^ operator matches all characters except the one that follows. For example,

`[^a]`

matches all characters except a, and

`[^A-Za-z]`

matches any character that is not a letter. Note that, inside a character class, the circumflex does not match the beginning of a line. An expression such as

`^[^A-Za-z]`

therefore matches any nonletter, but only at the beginning of a line. We apologize for this apparent contradiction in meaning, but regular expressions are inherently context dependent. Using grep takes a special mindset. Although grep might offer a little more power than you think you'll ever need, it's nice to know it's there. THINK C's grep supports other operators, and we refer you to the well-written THINK C user's manual for more detail.

> ### DLE

> The origin of the term “escape,” which means to remove any special context of a character, comes from data communications, in which programs send special control characters in a data stream to control communications. For example, in certain protocols, the end-of-text character (ETX) signals the end of a data block and the beginning of a checksum value. If binary data is being transmitted, however, it is likely that the ETX character (which has a value of 3) is part of the data. The transmitting software therefore prefixes any control character with the data-link escape character (DLE). The receiving program understands that any character following the escape is data, not control, and therefore places the ETX in its data buffer.

##unning with THINK C

After you've created your application's source code, you'll probably want to run it. THINK C's Project and Source menus control compilation, code generation, and program execution.

The Set Project Type command from the Project menu brings up a dialog box similar to the one shown in Figure 2-8.

**Figure 2-8.**
*The Set Project Type dialog box.*

Notice the radio buttons in the upper left corner of the dialog box in Figure 2-8. The Macintosh system software has different code configuration requirements for applications, desk accessories, and CDEVs. THINK C can create any kind of executable code on the Macintosh and therefore can create four types of projects: applications, desk accessories, device drivers, and code segments. All the examples in this book use the Application option. Each application has a file signature that consists of the file type and the creator. An application is always of type APPL. The creator defines how Finder

- Maps an icon to an application
- Associates an application’s documents with the application

The partition size defines how much memory MultiFinder will allocate to the application when it starts up. This is all the memory your application will get for both code and data, so it has to be enough. But it should not be so much that it hogs all the space on the user’s machine—your user might want to run a concurrent application.

You arrive at a reasonable value for the partition size by some initial guessing and trial and error. If you find that your application is running out of memory, you can bump up the value. It’s a good idea to run an application in the smallest partition possible and in the partition that maximizes the number of applications your user can open. Our sample applications all use small partitions, but larger applications require more memory.

To learn the size of your modules, use the Get Info command from the Source menu. Its dialog box displays the code size, data size, STR size, and jump table size for each module and segment and for the entire project. Code size is the size, in bytes, of the object code. The Macintosh system software requires that object code be grouped in segments (taken care of by the compiler’s “back end” code generator), and each segment is limited to 32K. The code size value gives you an idea of how large your segments are getting. See the sidebar on segmenting your code for more information on how to keep your code segments under the 32K limit.

There are other limits on Macintosh applications: 32K on data size and 32K on jump table size. If your data size is growing too large, which should occur only in some large projects, or if you’ve allocated memory for some large arrays on the stack, check the Separate STRS option in the Set Project Type dialog box to move your program’s string constants into a resource and free up some space. The jump table limit won’t be reached, again, except in large programs. If you’re careful about keeping your code modular and using static functions, you shouldn’t have any problem with the jump table size. But a vigilant programmer pays attention to organization. Just because you don’t need the space in small applications doesn’t mean that you can be careless with space. We discuss techniques for managing jump table size in Chapters 3 and 5.

**Segmenting Your Code**

If one of your code segments grows larger than 32K, you’ll observe some strange behavior. When a segment grows too large, you have to move one or more modules to another segment. The segments are separated visually in the project window with a dotted line. It’s easy to resegment your project by dragging a module name in the project window to another segment.

You add new source modules to the project file with either of two Add commands from the Source menu. You can’t compile a source file until it belongs to a project file because THINK C writes the resulting object code into the project file. The Add command without the ellipsis adds the file associated with the current editor window to the project file. The Add command with the ellipsis opens a standard file dialog box and lets you select files to add to the project file. This dialog box stays open, letting you add multiple files with a single command, until you select the Cancel button. You remove a module from the project file by selecting the module name in the project window and choosing Remove from the Source menu.

For most applications, you will need to add the MacTraps library to the project file. You do that by selecting Add (with the ellipsis) from the Source menu, navigating to the THINK C folder, selecting MacTraps, and clicking the Add button. THINK C will load the MacTraps library contents into the project file either automatically, when you run the program, or when you select the library name in the project window and choose Load Library from THINK C’s Source menu.

**Compile and Make**

To run a program, THINK C first compiles the source code into machine-readable object code. The combination of a source file and its associated object code is called a “module.” Whenever you change a file’s source code, you need to regenerate the file’s object code by compiling it. A source file needs compilation when

- You first create it
- You modify it
- You modify another file that it includes

Compilation occurs on a file-by-file basis. In THINK C, the source file name must end with the .c extension, and the compiler will not compile a file that doesn’t belong to a project. You manually invoke compilation of a particular source file by choosing Compile from the Source menu (Command-K) to compile either the file displayed in the currently open source file window of the editor or the file selected in the project window if no source file window is open.

If you’re accustomed to other C compilers—say UNIX’s cc, the Microsoft Optimizing compiler for MS-DOS, or the MPW C compiler—you’re in for a pleasant surprise.

**Checking Syntax**

You can check a source file’s syntax without invoking the compiler’s code generator by choosing the Check Syntax command from the Source menu (Command-Y). Because the code generator is not run, you can use this method to check the syntax of a nonproject file or a file that doesn’t end in .c. And because it doesn’t generate code, this feature proves to be slightly faster than using Compile from the Source menu (Command-K).

The THINK C compiler is very fast. (Early versions of THINK C were called Lightspeed C.) Because THINK C's compiler is so fast, some developers use the compiler to check their code's syntax. The compiler stops when it finds a syntax error, opens a window into the file containing the error, puts the text cursor on the offending line, and opens a message window that describes the error.

In a working environment, most programmers let THINK C's built-in make facility take care of remembering which source files need recompilation. There are no makefiles to create and maintain in THINK C, which uses the project file information and derives the dependency information directly from the source files.

Choosing Run from the Project menu (Command-R) executes the program from within the THINK C environment. The make facility is automatically invoked when you run the program, so you never need to worry about whether your program is in phase with the source code. You can set an environment option to run the program either by beginning the recompilation process automatically or by putting up a dialog box that gives you the choice of compiling or of running the program without changing the object code. You can also invoke the compilation process by choosing Bring Up To Date from the Project menu (Command-U). If the project is already up to date, the environment simply runs the program.

THINK C's make facility is almost always right about which files need compilation. When it's wrong, that's either because you've moved files into the project folder from backup disks and the modification times therefore don't apply to the current project or because you've manually manipulated the flags signaling that a file needs to be recompiled.

Choosing the Make command from the Source menu brings up the dialog box shown in Figure 2-9. You easily turn recompilation on or off by clicking next to the module name. There are also buttons to force compilation of all modules or none. When the make facility becomes confused, it is best to click the Use Disk button and turn off the Quick Scan option by clicking its check box. Then THINK C will reset the make flags according to the results of its search through each header and source file for dependency information and each file's time of modification.

```
Source files to compile: 1
Libraries to load: 0

AboutBox.c
☑ AppInit.c
DialogUtil.o
Display.c
DocUtil.c
FileUtil.c
MenuUtil.o
MiscUtil.o

Check All
Check All .c
Check None
Use Disk
☑ Quick Scan

Make
Don't Make
Cancel
```

**Figure 2-9.**

*The make dialog box. Click on module name to compile that module. Selection is indicated by check mark. Click on name a second time to clear mark.*

**The Debugger**

The symbolic debugger completes the THINK C development package. The debugger is a separate application that runs concurrently with the THINK C environment. You must therefore have enough memory to run both the environment and the debugger, and, in System 6, you must be running MultiFinder. (The System 7.0 Finder incorporates the concurrency features of MultiFinder.) You need only 2 megabytes for small projects like the examples in this book. But if you're serious about developing average-size applications, you'll need more memory—about 4 megabytes minimum.

You'll find a second monitor useful when you start to work with the debugger. You can use your primary screen for your program display and configure the THINK C Debugger to run on the second monitor. In our opinion, this is the only way to debug glitches in user-interface software. When we developed our Tycho Table Maker application, we ran into problems with our user interface modules when the debugger windows interacted with the Tycho windows they overlapped. As soon as we moved the debugger to a second screen, the problems disappeared.

To run your program with the THINK C Debugger, choose Debug from the Project menu. If you created your project without the debugging option, you'll need to recompile all your source code so that the symbol information gets generated. Don't worry—THINK C knows this and does it for you automatically the first time you try to run the program with the symbolic debugger.

When you have the THINK C Debugger up and running with your program, you have three applications running: the THINK C environment, the THINK C Debugger, and your program. This can get quite confusing, especially if you use keyboard shortcuts and consequently don't look at the menu bar to see which application is actually in the foreground.

The THINK C Debugger has two main windows: the source window and the data window shown in Figure 2-10 on the next page. If you have a two-monitor system, the source and data windows appear on the second screen. On a single-monitor system, these windows appear on the lower third of the screen.

**make and makefiles**

The make facility in THINK C is based on a UNIX program that drove compilation. In its day, make was a technological wonder, using a combination of file dependency data and times of file modification data to determine which files in a programming project needed recompilation. The project administrator or a programmer defined the dependency information in a text file called the "makefile." The makefile also contained information about how to generate the object code, how to link the object code, and what programming libraries to include to create the stand-alone program.


![Image of the debugger window with the source code displayed.]
**Figure 2-10.**
*The debugger windows.*
**The Source Window**

The source window’s name is that of the currently active source module (Shell.c in Figure 2-10). When the debugger starts up, the module in the window will be the one that contains the function `main()`. Execution will be at a halt at the first statement in your `main()` function. This statement might be an assignment statement. At the bottom left of the window you’ll see the current function’s name—when you first start the debugger, `main`. As you continue to run your program within the debugger source window, the name changes to that of the current function. If you hold down the mouse button when the pointer is in this region, a pop-up menu appears that contains the names in the chain of calls that got you to that function. This chain is sometimes called the “call stack.” (If the program burrows deeply into function after function, it might take longer than you expect to create this menu. Hang in there—it will show up.) This menu is a live menu: When you select one of the function names, the debugger source window displays that function’s source code.

The six buttons at the top of the source window correspond to the first six menu commands in the Debug menu. If you click one of the buttons or choose the corresponding command from the Debug menu, the button appears to be pressed, as shown in Figure 2-11. You can figure out where you are in the debugging process at any time by looking at these buttons.

![Image of the debugger window with the source code displayed.]

**Figure 2-11.**
*The Go button is highlighted when you click it.*

The Go button begins execution of your program and continues execution until a breakpoint or an error occurs.

The Step button executes a single statement and returns control to the debugger. In C, a statement can contain multiple function calls, so if the line contains any function calls, the functions are run as a single statement.

The Trace button works the way the Step button does in that it executes a single statement. But if the current statement is a function call, Trace traces control flow into the function and control stops at the first statement of the function.

The In button also steps into a function, but it executes any number of statements up to the first statement of the next function in the statement stream.

The Out button steps out of the current function. Like In, it executes any number of statements, but it stops at the statement after the current function returns.

The Stop button stops your program regardless of the part that is executing. You can use the Command-period equivalent for Stop.

**NOTE:** *Be careful when using Out around the main event loop. If you’re in the outermost level of your main() function and you select Out, the debugger will never return! You’ll have to quit and restart your debugging session.*

The arrow on the left side of the source window (visible in Figure 2-10) points to the current statement. The little diamonds to the left of this arrow, called statement markers, correspond to statements in your source file. Each statement marker is a potential “breakpoint” at which the debugger will stop your program, letting you examine variables and other elements.

To set a breakpoint at a particular statement, click on its statement marker. The diamond will turn black to indicate that the breakpoint is set. When you press the Go button and execution reaches the statement, the debugger will stop the program and place the current statement arrow at that line. To clear a breakpoint, click on its darkened statement marker or select the line in the source window and choose Clear Breakpoint from the debugger’s Source menu. You can remove all of a program’s breakpoints at the same time by choosing Clear All Breakpoints from the debugger’s Source menu.

## Setting a breakpoint in another module

The source window displays the source module associated with the current statement, and you can set breakpoints only in this module. The THINK C environment and debugger were designed to work together, however, and you can set a breakpoint in another module:

1. Switch out to THINK C by clicking in the project file window or selecting the project window from the debugger’s Window menu.Page 24 of 26
2. Open the file that contains the module in which you want to set the breakpoint.
3. Choose Debug from THINK C’s Source menu (Command-G).

The source code for the new module will appear in the debugger’s source window, so that you can set the new breakpoint by clicking on the appropriate statement’s diamond.

**Editing a source file while debugging**

The linkage between the THINK C environment and the debugger works both ways: You can invoke the THINK C editor on the source file displayed in the debugger window by choosing Edit from the debugger’s Source menu (Command-E). This is a handy feature when you discover a problem and want to make a quick fix in the source code without quitting your program (the program you’re debugging, not THINK C or the THINK C Debugger).

**Setting a temporary breakpoint**

You set a temporary breakpoint in your program by holding down the Command key or the Option key while you click on a statement marker. After you release the mouse button, the debugger will run your program up to that breakpoint and then clear the breakpoint. Two other commands from the Debug menu, Go Until Here and Skip Until Here, create something like temporary breakpoints. Both work with selections in the source window. After you select a statement (by double-clicking on the corresponding line in the source window), choosing Go Until Here (Command-H) will cause your program to execute up to the selected statement.

The Skip Until Here command “jumps” the current statement arrow to a statement selection without executing the code between the arrow’s old location and its new location. This feature can be useful for skipping over code that you know has bugs, when you want to test the various cases of a determinant expression, or even when you want to jump backward to re-execute some statements. But be smart about how you use Skip Until Here. Don’t skip over allocations and then try to use that memory, for example. And don’t skip from one stack frame to another; you’ll mess up the program stack.

**Stuck in Auto-Mode**

The debugger has what the documentation calls its “auto-mode.” A more descriptive name might be “sticky mode.” If you hold down the Option key or the Command key when you click on one of the buttons at the top of the debugger’s source window (Go, Step, In, Out, Trace, or Stop), the debugger will loop on each command as if the button were stuck. For example, if you’re in auto-Step mode, the debugger will execute the next instruction, stop, update the source and data windows, and then step again, as if you had clicked the Step button again. The auto-mode is useful when you would like to watch a variable’s value change as the program executes. You cancel auto-mode by pressing Command-Shift-Period.

**Coming to a screaming halt**

Sometimes programmers inadvertently create infinite loops in their code. If your program is running but isn’t responding to commands or if a breakpoint you set hasn’t been reached in a reasonable amount of time, you can halt program execution by pressing Command-Shift-Period—what Symantec calls “the panic button.” This key combination stops your program, invokes the debugger, and places the current statement arrow wherever the program was when you pressed the keys. The key combination works when the program itself is running in the foreground, but it won’t work if your program intercepts the panic button (Command-Shift-Period).

**Setting a conditional breakpoint**

The THINK C Debugger also supports conditional breakpoints, called “watchpoints” in some debuggers. A conditional breakpoint halts execution only when a condition fails. To set up a conditional breakpoint:

1. Click on the statement marker in the source window.
2. Double-click on the statement line to select the statement.
3. Click on an expression in the data window.
4. Choose Attach Condition from the debugger’s Source menu.

The statement marker turns gray to signify a conditional breakpoint.

You clear a conditional breakpoint just as you would a regular breakpoint—by clicking on the corresponding statement marker or by selecting the statement and choosing Clear Breakpoint from the Source menu.

If you want to check the condition associated with a conditional breakpoint, select the statement and choose Show Condition from the Source menu.

**The Data Window**

The condition governing a conditional breakpoint depends on an expression in the debugger’s data window. In the data window, you can examine the contents of your program’s variables. The data window has three parts. The upper part is an edit box in which you can enter variable names or C language expressions. Below the edit box are two columns: The left column contains the names of data objects; the right column contains the values of the objects. (Objects in this discussion have nothing to do with object-oriented programming. They’re basically variables, but they could be constants or enumeration types.)

To display a variable, either enter its name in the edit box at the top of the data window or double-click on the variable name in the source window to select it and then choose Copy To Data from the debugger’s Edit menu (Command-D).

The data window supports many of the fundamental C data types. The types it supports are listed in the debugger’s Data menu. When a data object’s value is displayed in the right column of the data window, you can format the value by selecting it and then choosing the appropriate format type from the Data menu. To change an object’s value, select the value in the right column to place the value in the edit box, enter the new value in the edit text box, and press Return or Enter. The value must be consistent with the object’s type and with the rules of the C language. You can’t reassign a constant value, for example.

If the data value is a pointer or a handle, double-clicking on it in the right column will create a new dereferenced value in the data window. This feature is handy for tracking through memory to look at objects on the heap. If the data object is a pointer to a structure, a subsequent double-click on the data object’s value will open a window on the structure values. The data window automatically uses data structure information from the project file and the header files, so formatting and field information in the data window match the source code in its (the source code’s) window.

Each data object in the data window has a specific context in which its value is valid. The rules that govern the validity of data object values in contexts follow the scope rules of the C language. (See Chapter 3 for more information on the scope of data objects in C.) For example, a local variable’s value is valid only within the context of the function in which the variable is defined. You can therefore have three local variables named `i` in the data window, each with a different context. And, because you’ll probably forget the context of a value, especially if you have three `i’s in your data window, you can see the context of a data window’s data object by selecting the data object’s name in the left column of the data window and choosing Show Context from the Data menu. The source window will show the function in the source file in which the object is defined.

Using the THINK C environment with the THINK C Debugger is like using any tool—it takes practice. Some of the features will become second nature to you. Others you’ll never get used to. All in all, we’re sure that you’ll find THINK C as comfortable a development environment as we do. As the product has evolved, Symantec has delivered more features, compatibility with new systems, and compatibility with other development environments, including their popular THINK Pascal and Apple’s MPW Pascal. If you use THINK C as your primary development environment, you can rest assured that your investment will be protected. This product is here to stay.

In this chapter, we’ve made a few assumptions about your knowledge of C. In the next chapter, we’ll survey the C programming language as it applies to the Macintosh. If you’re already pretty good with C, you might want to catch up with us in Chapter 4. Otherwise, turn the page.