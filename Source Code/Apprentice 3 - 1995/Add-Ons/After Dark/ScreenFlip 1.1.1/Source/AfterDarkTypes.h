/*
   GraphicsModule_Types.h

   Data types and structures used by After Dark graphics modules.

   For more information, consult the programmer's section of the manual.

   By Patrick Beard and Bruce Burkhalter

   Copyright �1989, 1990, 1991 Berkeley Systems, Inc.
 */

#ifndef __GRAPHICSMODULE_TYPES__
#define __GRAPHICSMODULE_TYPES__

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

/*
   #ifdef applec

   #ifndef __SOUND__
   #include <Sound.h>
   #endif

   #endif

   #ifdef THINK_C

   #include <SoundMgr.h>

   #endif
 */

#if     THINK_C == 1

#include <SoundMgr.h>

#else

#ifndef __SOUND__
#include <Sound.h>
#endif

#endif

/*
   Messages that are passed to main() by After Dark:

   Initialize -         Allocate module's storage and get started.
   Close -              Deallocate storage and shutdown.
   Blank -              Blank out the screen (make it black).
   DrawFrame -          Draw next frame of animation sequence.
   ModuleSelected -     The module has specific processing to do when selected.
   DoAbout -            Module can execute special code for an about message.
   ButtonMessage -      Module wants to put up a dialog when a button is pressed.
 */

typedef enum
{
    Initialize,
    Close,
    Blank,
    DrawFrame,
    ModuleSelected,
    DoAbout,
    ButtonMessage = 8
}
GMMessage;

/* Return messages */

/*
   The first three messages can be returned by DoInitialize(), DoClose(), DoBlank(), DoDrawFrame()
   RefreshResources is valid only after a "SetUp" message.
 */

enum
{
    ModuleError = -1,           /* After Dark will display the string params->errorMessage. */
    RestartMe = 1,              /* After Dark will call main() with an "Initialize" message. */
    ImDone,                     /* After Dark will not call main() again and take over drawing. */
    RefreshResources            /* After Dark will redraw all controls after "SetUp" message. */
};

/* bits in systemConfig that are special. */

#define MultiModuleRunning (1L << 10)   /* multimodule is present. */
#define ModuleMayNotAnimate (1L << 9)   /* you may not animate. */
#define SoundAvailable (1L << 15)       /* do we have sound? */
#define ExtensionsAvailable (1L << 14)  /* are there extensions? */

/* types for looking at the monitors on the system. */
struct MonitorData
{
    Rect bounds;                /* limiting rect of monitor (global coords) */
    Boolean synchFlag;          /* flag set by monitor vbl task */
    char curDepth;              /* current pixel depth */
};

typedef struct MonitorData MonitorData, *MonitorDataPtr;

struct MonitorsInfo
{
    short monitorCount;         /* number of monitors on system */
    MonitorData monitorList[1]; /* the monitors */
};

typedef struct MonitorsInfo MonitorsInfo, *MonitorsInfoPtr;

/* copy of quickdraw globals */
struct QDGlobals
{
    GrafPtr qdThePort;
    Pattern qdWhite;
    Pattern qdBlack;
    Pattern qdGray;
    Pattern qdLtGray;
    Pattern qdDkGray;
    Cursor qdArrow;
    BitMap qdScreenBits;
    long qdRandSeed;
};

typedef struct QDGlobals QDGlobals, *QDGlobalsPtr;

struct ExtensionElement
{
    OSType selector;
    Ptr entryPoints;
};

typedef struct ExtensionElement ExtensionElement;

struct ExtensionTable
{
    short extensionCount;
    ExtensionElement extensionList[1];
};

typedef struct ExtensionTable ExtensionTable, *ExtensionTablePtr;

/* the parameters passed in at every call to the graphics module */
struct GMParamBlock
{
    short controlValues[4];     /* the values of the user set sliders. */
    MonitorsInfoPtr monitors;   /* what monitors are connected and their depth. */
    Boolean colorQDAvail;       /* whether color is around. */
    short systemConfig;         /* bitmask of system configuration. */
    QDGlobalsPtr qdGlobalsCopy; /* read-only qd globals */
    short brightness;           /* message variable to tell NL to dim monitor */
    Rect demoRect;              /* rect of the Control Panel if in Demo Mode. */
    StringPtr errorMessage;     /* string to be displayed if error encountered. */
    SndChannelPtr sndChannel;   /* sound channel allocated for module's use. */
    short adVersion;            /* BCD After Dark version number. */
    ExtensionTablePtr extensions;       /* After Dark extensions table. */
};

typedef struct GMParamBlock GMParamBlock, *GMParamBlockPtr;

#endif
