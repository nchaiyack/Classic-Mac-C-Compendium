//	GW-cdev4.2.1.h

#include	<OSUtils.h>
#include	"GW-Common4.2.1.h"

enum	dialogItems {
			buttonAbout = 1,
			checkDragOn,
			checkGrowOn,
			checkPushOn,
			checkPushSound,
			checkPushCtrl,
			checkPushShift,
			checkPushOption,
			checkPushCommand,
			buttonPushCtrl,
			buttonPushShift,
			buttonPushOption,
			buttonPushCommand,
			checkPopOn,
			checkPopSound,
			checkPopCtrl,
			checkPopShift,
			checkPopOption,
			checkPopCommand,
			buttonPopCtrl,
			buttonPopShift,
			buttonPopOption,
			buttonPopCommand,
			checkShowIcon,
			checkNoMarquee,
			checkDragCtrl,
			checkDragShift,
			checkDragOption,
			checkDragCommand,
			buttonDragCtrl,
			buttonDragShift,
			buttonDragOption,
			buttonDragCommand,
			checkGlueOn,
			checkGlueCtrl,
			checkGlueShift,
			checkGlueOption,
			checkGlueCommand,
			buttonGlueCtrl,
			buttonGlueShift,
			buttonGlueOption,
			buttonGlueCommand
		};

#define		itemActive		0
#define		itemInactive	255
		
//	Prototypes

pascal long		main(short msg, short item, short numItems, short panelID,
						EventRecord *eventP, long val, DialogPtr dp);
Boolean			isSystem7(void);
Boolean			trapAvailable(short theTrap);
TrapType		getTrapType(short theTrap);
short			numToolboxTrap(void);
pascal Handle	get1Resource(OSType rsrcType, short id);
initDataHandle	load420Data(void);
OSErr			openPrefsFile(prefsFileSpec *pfSpecP);
void			saveSetting(initDataHandle dH);
long			checkMachine(void);
long			initPanel(DialogPtr dp, short numItems);
initDataHandle	getSetting(void);
void			hitItems(initDataHandle ih, DialogPtr dp, short item,
							EventRecord *eventP, short numItems);
void			setupDialogItems(initDataHandle ih,
									DialogPtr dp, short numItems);
void			setHilite(initDataHandle ih, DialogPtr dp, short numItems);
void			setActive(initDataHandle ih, DialogPtr dp, short numItems, short item);
void			setInactive(initDataHandle ih, DialogPtr dp, short numItems, short item);
void			setCheckButton(DialogPtr dp, short item, Boolean isOn);
void			checkKeyCombination(initDataHandle ih, short key);
Boolean			trackButton(DialogPtr dp, short numItems, short item, short key);
void			closePanel(initDataHandle ih);
Boolean			checkSameKeys(Boolean *key1, Boolean *key2);
Boolean			trackAbout(DialogPtr dp, short numItems);
void			about(DialogPtr dp);
