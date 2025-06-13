#pragma once

#include "TControlPanel.h"

struct myCDEV : TControlPanel {
			myCDEV(short numItems,DialogPtr cp);
	virtual	~myCDEV(void);
};
