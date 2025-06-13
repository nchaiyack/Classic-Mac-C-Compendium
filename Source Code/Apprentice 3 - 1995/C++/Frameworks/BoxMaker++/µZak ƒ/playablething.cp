#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <Packages.h>
#include <Traps.h>
#include <Files.h>
#include <Aliases.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include <Fonts.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Desk.h>
#include <Movies.h>

#include "playablething.h"

class movieplayer : public playablething
{
	public:
		movieplayer( FSSpec theFSSpec);
		~movieplayer();

		virtual void idleTime() {};
		virtual void SetVolume( short newVolume) {};
		virtual void Start() = 0;
		virtual void Stop() = 0;
		virtual Boolean isDone() const = 0;

	private:
		Movie theMovie;
};
