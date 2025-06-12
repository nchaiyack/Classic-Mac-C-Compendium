/*
 * Subclass of CApplication to allow memory debugging
 */

#pragma once

#include "CApplication.h"

class MemApplication : public CApplication {
public:
	virtual void SetUpMenus(void);
	virtual void RemovePatches(void);
	virtual void DoCommand(long theCommand);
	virtual void UpdateMenus(void);
	virtual void InstallPatches(void);
};
