#pragma once
//
// 941006: testing this code for the first time in a long time. It appears
// that one needs an instance of a 'venster' or a 'fullscreen' before one
// can create a 'gworld'. If one does:
//
// gworld  earth;
// window broken;
//
// the Mac crashes inside a 'ShowWindow' call in window::window
// I don't have time to study this in full, but I suspect that 'gworld::gworld'
// somehow is buggy. Alternatively there is a bug in the System (possibly only
// in system 7.5) which precludes the use of GWorlds without the use of windows.
// It also might be a bug in the Think Debugger (7.0.4), since an application
// I wrote to test this hypothesis did not hang.
//
class gworld : public grafport
{
	public:	
		gworld( int breedte = 320, int hoogte = 200, int diepte = 8, CTabHandle cTable = nil);
		~gworld();

		int diepte() const;
		//
		// dump and load do not do extensive error checking
		//
		OSErr dump( short defile) const;
		OSErr load( short defile) const;

	private:
		int de_diepte;
};

inline int gworld::diepte() const
{
	return de_diepte;
}
