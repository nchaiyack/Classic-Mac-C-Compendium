THIS CODE IS UNFINISHED AND UNTESTED

#pragma once

class dotMover
{
	public:
	
		dotMover();
		dotMover( const Point &startpoint);
		~dotMover();
		
		void moveto( const Point &newposition);
		void followMouse();
		void hide();
		void show();

	private:
		Point startingpoint;
		Point current_endpoint;
		int visible;
		
		void draw_once() const;
};
