//	GW-Common4.2.1.h

#include	<Gestalt.h>
#include	<Folders.h>
#include	<Traps.h>
#include	<Sound.h>

#define		myCreator		'GlWi'
#define		myDataType		'R&R!'
#define		prefsFileFlag	0

#define		trapMask		0x0800

enum	keyCode {
			commandKeyBit	= 48,
			controlKeyBit	= 60,
			optionKeyBit	= 61,
			shiftKeyBit		= 63
		};

enum	ALRT_ids {
			locked_ALRT_id = -4048,
			error_ALRT_id,
			restart_ALRT_id,
			system7_ALRT_id,
			prefs_ALRT_id,
			samekeys_ALRT_id
		};
				
enum	cicn_ids {
			OK_cicn_id = -4048,
			NG_cicn_id,
			active_cicn_id,
			down_cicn_id,
			inactive_cicn_id,
			aboutup_cicn_id,
			aboutdown_cicn_id,
			ctrl_cicn_id,
			shift_cicn_id,
			option_cicn_id,
			command_cicn_id,
			window_cicn_id = 128,
			NO_icon
		};
		
enum	CURS_ids {
			hand_CURS_id = 128,
			grip_CURS_id
		};

enum	DATA_ids {
			gw401_DATA_id = -4048,
			gw412_DATA_id = -4047,
			gw420_DATA_id = 128
		};
		
enum	INIT_ids {
			my_INIT_id = 0
		};
		
enum	sndx_ids {
			push_sndx_id = 128,
			pop_sndx_id
		};

enum	STRx_ids {
			about_STRx_id = -4048,
			numof_STRx_id,
			keys_STRx_id,
			prefs_STRx_id = 128
		};
		
enum	PICT_ids {
			title_PICT_id = -4048
		};
		
enum	WIND_ids {
			about_WIND_id = 128
		};
		
enum	prefsIndices {
			nameIndex = 1
		};

typedef struct initData {
	Boolean	dragOn;
	Boolean	noMarquee;
	Boolean glueOn;
	Boolean	growOn;
	Boolean	pushOn;
	Boolean	popOn;
	Boolean	pushSound;
	Boolean	popSound;
	Boolean	pushKey[4];
	Boolean	popKey[4];
	Boolean	dragKey[4];
	Boolean glueKey[4];
	Boolean	showIcon;	
} initData, *initDataPtr, **initDataHandle;

typedef struct gw412Data {
	Boolean	dragOn;
	Boolean	growOn;
	Boolean	pushOn;
	Boolean	popOn;
	Boolean	pushSound;
	Boolean	popSound;
	Boolean	showIcon;
	Boolean	pushKey[4];
	Boolean	popKey[4];
	Boolean	noMarquee;
	Boolean	dragKey[4];
} gw412Data, *gw412DataPtr, **gw412DataHandle;

typedef struct gw401Data {
	Boolean	dragOn;
	Boolean	growOn;
	Boolean	pushOn;
	Boolean	popOn;
	Boolean	pushSound;
	Boolean	popSound;
	Boolean	showIcon;
	Boolean	pushKey[4];
	Boolean	popKey[4];
} gw401Data, *gw401DataPtr, **gw401DataHandle;

typedef struct prefsFileSpec {
	short	RsrcRefNum;
	short	vRefNum;
	long	DirID;
} prefsFileSpec;

typedef struct windowList {
	WindowPeek	wPeek;
	Boolean		isPainted;
	Point		wPosition;
} windowList;

#define		defaultDragOn		true
#define		defaultNoMarquee	false
#define		defaultGlueOn		false
#define		defaultGrowOn		true
#define		defaultPushOn		false
#define		defaultPopOn		false
#define		defaultPushSound	true
#define		defaultPopSound		true
#define		defaultDragKey0		false
#define		defaultDragKey1		false
#define		defaultDragKey2		false
#define		defaultDragKey3		false
#define		defaultGlueKey0		false
#define		defaultGlueKey1		false
#define		defaultGlueKey2		true
#define		defaultGlueKey3		false
#define		defaultPushKey0		false
#define		defaultPushKey1		true
#define		defaultPushKey2		false
#define		defaultPushKey3		false
#define		defaultPopKey0		true
#define		defaultPopKey1		false
#define		defaultPopKey2		false
#define		defaultPopKey3		false
#define		defaultShowIcon		true

#define		MaxWindowList		20

short	iconLoc_h	: 0x092C;
short	iconLoc_cs	: 0x092E;
