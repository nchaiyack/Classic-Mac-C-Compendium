THIS CODE IS UNFINISHED AND UNTESTED

#pragma once

class rubberband
{
	public:
	
		rubberband( const Point &startpoint);
		~rubberband();
		
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
