#define UseExtendedThingResource 1

#include "Types.r"
#include "SysTypes.r"

resource 'thng' (128, sysHeap) {
	'osa ', 'SMPL', 'appl', 0x0000004E, 0x00,	/* ComponentDescription */
	'PROC', 128,								/* resource where Component code is found */
	'STR ', 128,								/* name string resource */
	'STR ', 129,								/* info string resource */
	'ICON', 0,									/* icon resource */
	0,
	componentHasMultiplePlatforms,
	0,
	{0x0000004E, 'PROC', 128, platformPowerPC}
};


resource 'STR ' (128) {
	"SampleScript"
};

resource 'STR ' (129) {
	"Sample OSA Component"
};
