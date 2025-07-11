
/* Values used in mouse tracking. */

#define trackPress			1
#define trackMove			2
#define trackRelease		3

#define kMouseMoved			true
#define kTurnItOn			true
#define kTurnItOff			!kTurnItOn


typedef void (*ConstrainProcPtr)(Point anchorPoint, Point previousPoint,
								Point *nextPoint);

typedef void (*FeedbackProcPtr)(Point anchorPoint, Point currentPoint,
								Boolean turnItOn, Boolean mouseDidMove);

typedef void (*TrackMouseProcPtr)(short aTrackPhase, Point *anchorPoint,
								Point *previousPoint, Point *nextPoint,
								Boolean mouseDidMove);



Rect	SketchNewRect(Boolean fromCenter);
void	RectFeedback(Point anchorPoint, Point nextPoint,
					Boolean turnItOn, Boolean mouseDidMove);

Rect	SketchNewLine(Boolean fromCenter);
void	LineFeedback(Point anchorPoint, Point nextPoint,
					Boolean turnItOn, Boolean mouseDidMove);

Rect	SketchNewOval(Boolean fromCenter);
void	OvalFeedback(Point anchorPoint, Point nextPoint,
					Boolean turnItOn, Boolean mouseDidMove);

Rect	SketchNewRoundRect(Boolean fromCenter);
void	RoundRectFeedback(Point anchorPoint, Point nextPoint,
					Boolean turnItOn, Boolean mouseDidMove);

void	TrackMouse(ConstrainProcPtr constrainProc,
					FeedbackProcPtr feedbackProc,
					TrackMouseProcPtr trackMouseProc,
					Point *anchorPoint,
					Point *endPoint);

void	AdjustForTrackingFromCenter(Point *anchorPoint, Point endPoint);
