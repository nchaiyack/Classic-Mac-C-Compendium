/*
	The canonical Internet Config interface is defined in Pascal.  These headers have
	not been thoroughly tested.  If there is a conflict between these headers and the
	Pascal interfaces, the Pascal should take precedence.
*/

/* ///////////////////////////////////////////////////////////////////////////////// */

#ifndef __ICCAPI__
#define __ICCAPI__

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __FILES__
#include <Files.h>
#endif

#ifndef __COMPONENTS__
#include <Components.h>
#endif

#ifndef __ICTYPES__
#include <ICTypes.h>
#endif

/* //////////////////////////////////////////////////////////////////////////////// */

enum {
	internetConfigurationComponentType = 'PREF',										/* the component type */
	internetConfigurationComponentSubType = 'ICAp',				   /* the component subtype */
	internetConfigurationComponentInterfaceVersion = 0L			/* current version number */
};

typedef ComponentInstance internetConfigurationComponent;

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

	extern pascal ICError ICCStart(internetConfigurationComponent *inst, OSType creator);
	/* checks for the presence of the Component Manager and the Internet Configuration component */
	/* returns badComponentInstance if it can't find either*/
	/* inst is either nil or a valid component instance */
	/* this routine lets you access the component based implementation with only minimal*/
	/* yucky glue */
	extern pascal ICError ICCStop(internetConfigurationComponent inst);
	/* shut down the component */

	pascal ICError ICCFindConfigFile(internetConfigurationComponent inst, short count, ICDirSpecArrayPtr folders)
	FIVEWORDINLINE(0x2F3C, 0x06, 0x02, 0x7000, 0xA82A);
	pascal ICError ICCSpecifyConfigFile(internetConfigurationComponent inst, FSSpec config)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x03, 0x7000, 0xA82A);

	pascal ICError ICCGetSeed(internetConfigurationComponent inst, long *seed)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x04, 0x7000, 0xA82A);
	pascal ICError ICCGetPerm(internetConfigurationComponent inst, ICPerm *perm)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x0D, 0x7000, 0xA82A);

	pascal ICError ICCBegin(internetConfigurationComponent inst, ICPerm perm)
	FIVEWORDINLINE(0x2F3C, 0x02, 0x05, 0x7000, 0xA82A);
	pascal ICError ICCGetPref(internetConfigurationComponent inst, ConstStr255Param key, ICAttr *attr, Ptr buf, long *size)
	FIVEWORDINLINE(0x2F3C, 0x10, 0x06, 0x7000, 0xA82A);
	pascal ICError ICCSetPref(internetConfigurationComponent inst, ConstStr255Param key, ICAttr attr, Ptr buf, long size)
	FIVEWORDINLINE(0x2F3C, 0x10, 0x07, 0x7000, 0xA82A);
	pascal ICError ICCCountPref(internetConfigurationComponent inst, long *count)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x08, 0x7000, 0xA82A);
	pascal ICError ICCGetIndPref(internetConfigurationComponent inst, long n, Str255 key)
	FIVEWORDINLINE(0x2F3C, 0x08, 0x09, 0x7000, 0xA82A);
	pascal ICError ICCDeletePref(internetConfigurationComponent inst, ConstStr255Param key)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x0C, 0x7000, 0xA82A);
	pascal ICError ICCEnd(internetConfigurationComponent inst)
	FIVEWORDINLINE(0x2F3C, 0x00, 0x0A, 0x7000, 0xA82A);

	pascal ICError ICCDefaultFileName(internetConfigurationComponent inst, Str63 *name)
	FIVEWORDINLINE(0x2F3C, 0x04, 0x0B, 0x7000, 0xA82A);

#ifdef __cplusplus
}
#endif __cplusplus

#endif
