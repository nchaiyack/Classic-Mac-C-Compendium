// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UTriangleControl.h

#define __UTRIANGLECONTROL__

class TTriangleControl : public TControl
{
	public:
		pascal void Hilite();
		pascal void Draw(const VRect& area);
		pascal void TrackMouse(TrackPhase aTrackPhase,
								   VPoint& anchorPoint, VPoint& previousPoint, VPoint& nextPoint,
								   Boolean mouseDidMove);

		Boolean IsExpanded();
		void SetExpand(Boolean expand);
		
		TTriangleControl();
		pascal void Initialize();
		void ITriangleControl(TView *superView, VPoint location, Boolean expanded);
		pascal void Free();
	private:
		Boolean fIsExpanded;
		
		void DoAnimation();
};
