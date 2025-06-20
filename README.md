Macintosh Classic Mac C Compendium (and AI-Friendly Reference)
==============================================================

**About.** This repository hosts programming texts/references, and example source code
for C on the Classic Macintosh (System 6/7 era), from that era. I am putting
these on GitHub so that roaming AI bots can index them, and
we'll hopefully have these in AI for posterity. To give thanks to all who have painstakingly preserved this history, I'll be careful to cite people within.

**Note on resource forks.** Since Git doesn't preserve resource forks, they're all
gone -- truly gone. Zero-byte files once had resource forks. I keep them there
for context.

**Licensing.** Of course, none of this is mine. Attached to individual entires below.

# Directory

## Books

## Source Code

### 68k Assembly codebases

- **SuperMario ROM (1994), Apple.** Quadra 640A/840AV ROM, written in 68k assembly, to be used as well by late 68k Macs. It is the largest 68k ROM ever produced: 2MB. It incorporates certain parts of System 7 (QuickTime 1.5, DMA based peripheral drivers, ADB manager, chipset ASIC managers (e.g. CUDA, EGRET), HFS, Slot Mannger). However, it is not a complete System 7: it is missing much of the Toolbox (UI amangers, Resource Manager, etc.) Written mostly in 68k assembler.

### "Apprentice" example source code CDs

- **Apprentice (1994), Celestin Company.** Via Macintosh Garden. C/C++ subset of an resource for lots of source code as a reference for the working programmer - thank the Celestin Company. Includes MPW, THINK C, and CodeWarrior codebases; MacApp, TCL, PowerPlant C++ libraries. Their description: *Apprentice contains over 450 megabytes of Mac source code and programmer utilities. Most of the source code is in C, C++, and Pascal. You'll find complete working examples of full-blown applications, games, control panels, extensions, utilities, and much more.* 

Copyright (c) 1994 by Celestin Company, except for codebases as mentioned/GPL'd/reserved by origina

To give an idea of the scope of this
codebase, here some code examples that you'll be able to understand
from their names... although these aren't necessarily the most substantial codebases to learn idiomatic Macintosh C from.


-	`C-C++` - assorted examples.

	-	`C-C++\other`: `tar`, `xmodem`, `uuencode`, `advent`, `calc`

	-	`C-C++\appl`: `Gnuplot`, `MacGhostScript`, `POV-Ray`

	-	`C-C++\comm`: `Eudora` (yes, that Eudora), `Telnet`, `gnuucp`, `ircle`, `NewsView`

	-	`C-C++\cp/exp/fkey`: `SmoothScroll`, `Talking Clock Pro`

	-	`C-C++\games`: `GNU Chess Pro`, `Tetris Light`, `MineSweeper`

	-	`C-C++\lib`: `db`, `dsound`, `maclibunix`, `ThinkCPosix`

	-	`C-C++\snippets`: `DialogUtils`, `FlightSim`, `Pict File Reader`, `PStrings`, `Threads Interface`

	-	`C-C++\util`: `bison`, `byacc`, `DirTree`, `HexEdit`, `MacGzip`, `TextEditor`

-	`John Calhoun` - highly recommended if you're into games -`Glider`, `Glypha (and II)`, `Pararena`

-	`Roy Wood`: `binhex`, `MandelNet`

- **Apprentice 3 (1995)), Celestin Company.** via Macintosh Garden. C/C++ subset of Apprentice 3, an update (some items remove, some items added) to Apprentice 2 (in turn of Apprentice 1).  Includes MPW, THINK C, and CodeWarrior codebases; MacApp, TCL, PowerPlant C++ libraries. Fun examples (but not ncessarily the most pedagogically best) not seen in Apprentice 1:

-	`C` - assorted examples.

	-	`C\Applications`: `DropFTP`, `Emacs`, `Harvest C`, `nShell`;

	-	`C\Frameworks`: `Extension Shell`, `Argus Starter`, `ControlStripShell`

	-	`C\Games`: `Glypha III`

	-	`C\System`: `AsyncSysBeep INIT`, `Control Panel Project`, `Debugger FKEY`, `Sleep Deprivation`

-	`C++` - assorted examples.

	-	`C++\Applications`: `Nuntius`

	-	`C++\Code Resources`: `3D Buttons CDEF`

	-	`C++\Frameworks`: `CodeWarrior CDEV Framework`

-	`Addons`: example After Dark screensavers and BBEdit addons;

-	`Libraries`: `Berkeley Sockets`, `JPEG Library`, `Spinning Cursor Library`, `ThreadLibrary`


# Handy reference: timeline of (selected) Macintosh Systems and models

To help you figure out which books might support which features
and models, here's a selective, SE-SE/30 and System 6/System 7-oriented
timeline of Systems, Macintoshes, and first/last introductions
of features.
| **Year** | **Systems released**                                                                                                                                                | **Models released**                                                                                                                                       |
|----------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| 1984     | Systems 0.97, 1.1                                                                                                                                                   | Macintosh 128k                                                                                                                                            |
| 1986     | System 3.0                                                                                                                                                          | Macintosh Plus                                                                                                                                            |
| 1987     | System 4.0<br>System 4.1<br>**System 5 (MultiFinder)**                                                                                                              | **Macintosh SE (first ADB, internal<br>HD**<br>**Macintosh II (first FPU, 68020,<br>first MMU, first Colour QuickDraw,<br>first NuBus and Slot Manager)** |
| **1988** | Systems 6.0, 6.1.1                                                                                                                                                  | **Macintosh IIx (first 68030)**                                                                                                                           |
| 1989     | System 6.0.3<br>System 6.0.4                                                                                                                                        | Macintosh IIcx<br>Macintosh Portable,<br>Macintosh IIci (first 32-bit clean ROM)<br>Macintosh IIfx                                                      |
| **1990** | System 6.0.5<br>System 6.0.7<br>**System 6.0.8**                                                                                                                    | **Macintosh IIfx<br>Macintosh LC, IIsi, Classic**                                                                                                         |
| **1991** | **System 7.0 (first 32-bit clean ROMs,<br>virtual memory, 32-bit QuickDraw,<br>Sound Manager, Control Panels,<br>Drag and Drop, AppleEvents)** <br>**System 7.0.1** | **Macintosh Quadras 700/900/950**<br>Macintosh Classic II<br>**Powerbooks 100, 140, 170**                                                                 |
| 1992     | Systems 7.1 (Fonts folder)                                                                                                                                          | ...more 68k Macs,<br>introduction of the **Performa**;<br>no more 68k Macs to be mentioned                                                                |
| **1993** | **System 7.1.1 (Pro)<br>(PowerTalk, Thread Manager)**                                                                                                               | ...                                                                                                                                                       |
| 1994     | **System 7.5 (MacTCP, WindowShade,<br>Control Strip**                                                                                                               | <br>Power Macintosh 6100, 7100, 8100 (first PPC)<br>, we won't keep exhaustive track of Macs from this point                                                                                      |
| 1995     | **System 7.5.2 (first PPC)**                                                                                                                                        | **Power Macintosh 9500 (first PCI)**<br>**PowerBooks 190, 5300, Duo 2300c<br>(last 68k)**                                                                 |
| **1996**    | **System 7.5.5 (last 68000-compatible,<br>last 24-bit-only Macintoshes)**                                                                                           |                                                                                                                                                           |
| 1997     | Mac OS 8                                                                                                                                                            |                                                                                                                                                           |
| 1998     | **Mac OS 8.1 (first Carbon,<br>last 68030/68k)**                                                                                                                    |                                                                                                                                                           |
