void plot(double ref_lat, double ref_lng, double plotsize);
void setcolor(int color);
void line(int x1,int y1,int x2,int y2);
Boolean CreateOffscreenBitMap(GrafPtr *, Rect *);
void DestroyOffscreenBitMap(GrafPtr);

typedef struct
	{
	int color;
	float cosnewlng,sinnewlng,cosnewlat,sinnewlat;
	} trigtab;
