// NewRotation 1.0.1
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

// ------------------------------------------------------------------- //
// Another public domain C example source demo, brought back from      //
// the dead at:  itty bitty bytesª - by Kenneth A. Long!               //
// Made to run in Think Cª on 2 June 1994.                             //
// Uses no resource file - just add MacTraps and ANSI libraries.       //
// The original of this file was found on ftp.cso.uiuc.edu in mac/MUG. //
// ------------------------------------------------------------------- //
// Update, 22 September 1994, to include Offscreen GWorld, key
// controls and eliminate flicker.
// kenlong@netcom.com
// ------------------------------------------------------------------- //

#include <stdio.h>    
#include <math.h>
#include <qdoffscreen.h>

#define kLeft				0    
#define kTop				0    
#define kRight				qd.screenBits.bounds.right    
#define kBottom				qd.screenBits.bounds.bottom   
#define kNumberOfFaces		9
#define kNumberOfVertices	9
#define kNumberOfSets		20
#define kSolid				1
#define kSolidLight			0
#define kWireFrame			2

#define X					0    
#define Y					1
#define Z					2 
#define LX					80
#define LY					40
#define LZ					60    

short vertex [kNumberOfVertices] [3] = 
{            
    {            0,         0,       LZ / 2 }, 
    {       LX / 2,         0,           LZ },     
    {           LX,         0,       LZ / 2 }, 
    {       LX / 2,         0,            0 }, 
    {      LX / 2,        LY,       LZ / 4 },     
    { 3 * LX / 4,        LY,       LZ / 2 },     
    {      LX / 2,        LY, 3 * LZ / 4},     
    {      LX / 4,        LY,       LZ / 2 }, 
    {      LX / 2, -LY / 2,       LZ / 2 }
};    

short  face [kNumberOfFaces] [5] = 
{     
    {0, 1, 6, 7, 0}, 
    {1, 2, 5, 6, 1}, 
    {2, 3, 4, 5, 2}, 
    {7, 4, 3, 0, 7}, 
    {7, 6, 5, 4, 7}, 
    {0, 3, 8, 0}, 
    {8, 1, 0, 8}, 
    {8, 2, 1, 8}, 
    {8, 3, 2, 8}
};
        
short  xr = 200, yr = 160, zr = 0, dxo = 50, dyo = 30, dzo = -50;    
PolyHandle  face_poly [kNumberOfSets] [kNumberOfFaces];
short visible [kNumberOfSets] [kNumberOfFaces];    
short light [kNumberOfSets] [kNumberOfFaces];    
short drawflag;

Rect option_rect, display_rect;

WindowPtr    newWindow;                           
Rect        windowBounds;

//¥ GWorld goodies.
GWorldPtr        gOffscreenGWorld;    
Rect            gOnscreenRect;    // The "projection screen" area in the window
Rect            gOffscreenRect;

//¥ MBar hide/show stuff.
RgnHandle		mBarRgn, GrayRgn;
short			*mBarHeightPtr;
short			oldMBarHeight;

GWorldPtr Create_A_GWorld (Rect *theRect);
void New_Window (void);
void main (void);
void Compute_Rotation_Data (short axis);
void Rotate_Object (void);

void HideMenuBar (void) 
{
	Rect	mBarRect;

	GrayRgn = GetGrayRgn ();
	mBarHeightPtr = (short *)  0x0BAA;
	oldMBarHeight = *mBarHeightPtr;
	*mBarHeightPtr = 0;
	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top + oldMBarHeight;
	mBarRgn = NewRgn ();
	RectRgn (mBarRgn, &mBarRect);
	UnionRgn (GrayRgn, mBarRgn, GrayRgn);
	PaintOne (0L, mBarRgn);
}

void ShowMenuBar (void) 
{
	*mBarHeightPtr = oldMBarHeight;
	DiffRgn (GrayRgn, mBarRgn, GrayRgn);
	DisposeRgn (mBarRgn);
}

GWorldPtr Create_A_GWorld (Rect *theRect)
{
    CGrafPtr    currentPort;
    GDHandle    currentGDevice;
    GWorldPtr   offScreen;
    QDErr       result;

    GetGWorld (&currentPort, &currentGDevice);
    
    if ((result = NewGWorld (&offScreen, 0, theRect, nil, nil, 0L)) != noErr)
        if ((result = NewGWorld (&offScreen, 0, theRect, nil, nil, useTempMem)) != noErr)
            return (nil);
            
    SetGWorld (offScreen, nil);

    ClipRect (&offScreen->portRect);    
    
    if (LockPixels (GetGWorldPixMap (offScreen)))
    {
        ForeColor (blackColor);
        BackColor (whiteColor);
        EraseRect (&offScreen->portRect);
        UnlockPixels (GetGWorldPixMap (offScreen));
    }

    SetGWorld (currentPort, currentGDevice);
    return offScreen;
}

void New_Window (void)
{
	SetRect (&windowBounds, kLeft, kTop, kRight, kBottom);
	
    newWindow = NewWindow (0L, &windowBounds, "\p3D Display", true, noGrowDocProc, (WindowPtr) - 1L, true, 0);
    SetPort (newWindow);

    SetRect (&gOnscreenRect, kLeft, kTop + 20, kRight, kBottom + 20);
    SetRect (&gOffscreenRect, kLeft, kTop + 20, kRight, kBottom + 20);

    gOffscreenGWorld = (GWorldPtr) Create_A_GWorld (&gOffscreenRect);
}

void main ()                        
{    
    EventRecord the_event;
    Point mousepoint;
    GrafPtr the_port;
    short i, j;
    register short  this_set, next_set;
    Rect temp_rect;
    
    InitGraf (&qd.thePort);     
    InitFonts ();    
    InitWindows ();    
    InitCursor ();
    
    HideMenuBar ();
    HideCursor ();
    New_Window ();
    
    Compute_Rotation_Data (X);
      drawflag = kSolidLight;
    

    for (;;)
    {    
        GetNextEvent (everyEvent, &the_event);
        if (the_event.what == keyDown)
        {    
            switch (the_event.message & charCodeMask)
            {
                case 'Q':
                case 'q':
                    ExitToShell ();
                break;

                case 'x':
                case 'X':
                    Compute_Rotation_Data (0);
                break;

                case 'y':
                case 'Y':
                    Compute_Rotation_Data (1);
                break;

                case 'z':
                case 'Z':
                    Compute_Rotation_Data (2);
                break;

                case 'f':
                case 'F':
                        if (++drawflag == 3)     
                            drawflag = 0;
                break;
            }
        }
        Rotate_Object ();
    }
    ShowCursor ();
    ShowMenuBar ();
}

void Compute_Rotation_Data (short axis)
{
    register short this_set, i, j;
    short  basev [kNumberOfVertices] [3];
    short v [kNumberOfVertices] [3];
    short p_vx [kNumberOfVertices], p_vy [kNumberOfVertices];
    short ax, ay, az, bx, by, bz;
    
    Rect count_rect;                
    short v0, v1, v2;
    float sin_table, cos_table; 
    #define D_THETA  6.283185 / kNumberOfSets
    
    SetRect (&count_rect, 0, 21, 512, 342);    
    EraseRect (&count_rect);
    TextSize (18); 
    TextFace (bold); 
    TextFont (geneva); 
    PenNormal ();
    MoveTo (30, 50);    
    DrawString ("\pComputation of Rotation Data");
    MoveTo (30, 80);    
    DrawString ("\pCounting Down...");
        
    for (i = 0; i < kNumberOfVertices; ++i)    
    {    
        basev [i] [X] = vertex [i] [X]  + dxo;    
        basev [i] [Y] = vertex [i] [Y]  + dyo;    
        basev [i] [Z] = vertex [i] [Z]    + dzo;
    }
    
    for (this_set = 0; this_set < kNumberOfSets; ++this_set)    
    { 
        sin_table  =  sin (D_THETA * this_set);    
        cos_table = cos (D_THETA * this_set);    
        
        for (i = 0; i < kNumberOfVertices; ++i)    
        {    
            switch (axis)    
            {    
                case Z:    
                    v [i] [X] = basev [i] [X] * 
                              cos_table - basev [i] [Y] *
                              sin_table;
                              
                    v [i] [Y] = basev [i] [X] *
                              sin_table + basev [i] [Y] *
                              cos_table;
                              
                    v [i] [Z] = basev [i] [Z];
                break;
                
                case Y:    
                    v [i] [X] = basev [i] [X] * 
                              cos_table + basev [i] [Z] * sin_table;
                              
                    v [i] [Y] = basev [i] [Y];
                    v [i] [Z] = -basev [i] [X] * 
                              sin_table + basev [i] [Z] * cos_table;
                break;
                
                case X:    
                    v [i] [X] = basev [i] [X];
                    v [i] [Y] = basev [i] [Y] * 
                              sin_table +  basev [i] [Z] * cos_table;
                    v [i] [Z] = -basev [i] [Y] * 
                              cos_table +  basev [i] [Z] * sin_table;
                    break;
            }
        }
        for (i = 0; i < kNumberOfFaces; ++i)
        {    
            v0 = face [i] [0];        
            v1 = face [i] [1];    
            v2 = face [i] [2];    
            ay = v [v2] [Y] -  v [v1] [Y];    
            by = v [v0] [Y] -  v [v1] [Y];
            az =  v [v2] [Z] -  v [v1] [Z];    
            bz =  v [v0] [Z] -  v [v1] [Z];
        
            light [this_set] [i] = (ay * bz - az * by) > 0 ?0 :1;    
        }        
        for (i = 0; i < kNumberOfVertices; ++i)
        {    
            p_vx [i] = xr + .86603 * (v [i] [X] + v [i] [Z]);
            p_vy [i] = yr +  v [i] [Y] + (v [i] [X] - v [i] [Z])/2;
        }    
        for (i = 0; i < kNumberOfFaces; ++i)
        {    
            v0 = face [i] [0];        
            v1 = face [i] [1];    
            v2 = face [i] [2];    
            ax = p_vx [v2] -  p_vx [v1];    
            bx = p_vx [v0] -  p_vx [v1];
            ay = p_vy [v2] -  p_vy [v1];    
            by = p_vy [v0] -  p_vy [v1];
        
            visible [this_set] [i] =     (ax * by - ay * bx) < 0 ?1 :0;
        }
        for (i = 0; i < kNumberOfFaces; ++i)
        {    
            face_poly [this_set] [i] = OpenPoly ();
                MoveTo (p_vx [face [i] [0]], p_vy [face [i] [0]] );
                for (j = 1; j < kNumberOfVertices; ++j)
                {    
                    LineTo (p_vx [face [i] [j]], p_vy [face [i] [j]] );
                    if (face [i] [j] == face [i] [0] )    
                        break;
                }
                ClosePoly ();
        }
        SetRect (&count_rect, 200, 50, 250, 80);
        EraseRect (&count_rect);
        MoveTo (200, 80);
        i = (kNumberOfSets-this_set); 
        if (i > 99)    
            DrawChar (48 + i / 100);    
        else    
            DrawChar (' '); 
        
        if (i > 99)  
        { 
            i %= 100;      
            DrawChar (48 + i / 10);  
            i %= 10 ;  
        }    
        else
            {    
                if (i>9)    
                {
                    DrawChar (48 + i / 10);    
                    i %= 10;  
                } 
                else  
                    DrawChar (' '); 
        }    
        DrawChar (48 + i);
    }
    Draw_Option_Bar ();
}    

Draw_Option_Bar ()
{
    TextSize (12);    
    SetRect (&option_rect, 0, 0, 20, 420);    
    
    MoveTo (5, 16);    
    DrawString ("\p Q - Quits, X - 'X' axis, Y - 'Y' axis, Z - 'Z' axis and F is Frame.");
}
    
void Rotate_Object ()
{    
    long                ticksNow;
    GWorldPtr            windowGW;
    GDHandle            windowGD;
    PixMapHandle        thePixMap;

    register             short i;    
    static short             set = 0;
    register PolyHandle *poly;
    
    poly = &face_poly [set] [0];
    
    GetGWorld (&windowGW, &windowGD);     // Save whatever we had before

    SetGWorld (gOffscreenGWorld, nil);

    if (LockPixels (thePixMap = GetGWorldPixMap (gOffscreenGWorld)))
    {
        EraseRect (&gOffscreenGWorld->portRect);
        if (drawflag == kSolidLight)  
        {    
            for (i=0;i<kNumberOfFaces;++i)    
            {    
                if (visible [set] [i] ==1) 
                {    
                    if (light [set] [i] ==1)     
                        FillPoly (* (poly + i), &qd.ltGray);
                        FramePoly (* (poly + i)); 
                }    
            }
        }
        else 
            if (drawflag == kSolid)  
            {    
                for (i = 0; i < kNumberOfFaces; ++i)    
                    if (visible [set] [i] ==1)    
                        FramePoly (* (poly + i)); 
            }
            else     
                for (i = 0; i < kNumberOfFaces; ++i)    
                    FramePoly (* (poly + i)); 
        
        if (++set == kNumberOfSets)    
            set = 0;

        SetGWorld (windowGW, windowGD);

        CopyBits ((BitMap*) *thePixMap, 
                & ((GrafPtr) newWindow)->portBits, 
                  &gOffscreenRect, 
                  &gOnscreenRect, srcCopy, nil);
        
        UnlockPixels (thePixMap);
    }
    SetGWorld (windowGW, windowGD);
}
