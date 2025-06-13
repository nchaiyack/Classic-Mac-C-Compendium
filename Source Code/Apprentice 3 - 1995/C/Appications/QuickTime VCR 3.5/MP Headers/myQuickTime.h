/*---------------------------------------------------------------------
	File: myQuickTime.h
	Purpose: Contains class and define stuff to manage quicktime stuff 
	such as openning files, setting rates, and such.
	
	Created by: Geoffrey Slinker
	Date: 10:58:03 AM  6/13/92
	Modified: 
---------------------------------------------------------------------*/
#define MOVIE_WIND	700
#define MOVIE_POSTER_WIND	701


class QT : direct {
	public:
		Movie myMovie;
		Rect origBounds;
		Rect dispBounds;
		CWindowPtr movieWindow;
		CWindowPtr moviePosterWindow;
		
		QT(void);
		~QT(void);
		
		
		virtual void MovieCheck(void);
		virtual void LoadResources(void);
		virtual int OpenMovie(void);
		virtual int OpenMovieFSSpec(FSSpec mySpec);
		virtual void PlayMovie(void);
		virtual void CloseMovie(void);
		virtual void MyMoviesTask(void);
		virtual void StopMyMovie(void);
		virtual void FFMyMovie(void);
		virtual void RWMyMovie(void);
		virtual void GoStartMyMovie(void);
		virtual void GoEndMyMovie(void);
		virtual void RedrawPoster(void);
		virtual void HandleVolume(int direction);

};