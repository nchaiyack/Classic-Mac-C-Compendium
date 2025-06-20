#define WINDOW1 128
#define WINDOW2 129
#define m2PI &255

Rect show_r;
WindowPtr work_w, show_w;
WindowRecord w_record1, w_record2;
int doneFlag = 0;

/* PI/2 =  64 PI = 128 3PI/2 = 196 2PI = 256
   PI/4 = 32 3PI/4 = 96 5PI/4 = 160 7PI/4 = 224
*/

/* 64 *  sin(i * 2pi / 256) */
int sinus[256] = {
       0,    2,    3,    5,    6,    8,    9,   11,   12,   14, 
      16,   17,   19,   20,   22,   23,   24,   26,   27,   29, 
      30,   32,   33,   34,   36,   37,   38,   39,   41,   42, 
      43,   44,   45,   46,   47,   48,   49,   50,   51,   52, 
      53,   54,   55,   56,   56,   57,   58,   59,   59,   60, 
      60,   61,   61,   62,   62,   62,   63,   63,   63,   64, 
      64,   64,   64,   64,   64,   64,   64,   64,   64,   64, 
      63,   63,   63,   62,   62,   62,   61,   61,   60,   60, 
      59,   59,   58,   57,   56,   56,   55,   54,   53,   52, 
      51,   50,   49,   48,   47,   46,   45,   44,   43,   42, 
      41,   39,   38,   37,   36,   34,   33,   32,   30,   29, 
      27,   26,   24,   23,   22,   20,   19,   17,   16,   14, 
      12,   11,    9,    8,    6,    5,    3,    2,    0,   -2, 
      -3,   -5,   -6,   -8,   -9,  -11,  -12,  -14,  -16,  -17, 
     -19,  -20,  -22,  -23,  -24,  -26,  -27,  -29,  -30,  -32, 
     -33,  -34,  -36,  -37,  -38,  -39,  -41,  -42,  -43,  -44, 
     -45,  -46,  -47,  -48,  -49,  -50,  -51,  -52,  -53,  -54, 
     -55,  -56,  -56,  -57,  -58,  -59,  -59,  -60,  -60,  -61, 
     -61,  -62,  -62,  -62,  -63,  -63,  -63,  -64,  -64,  -64, 
     -64,  -64,  -64,  -64,  -64,  -64,  -64,  -64,  -63,  -63, 
     -63,  -62,  -62,  -62,  -61,  -61,  -60,  -60,  -59,  -59, 
     -58,  -57,  -56,  -56,  -55,  -54,  -53,  -52,  -51,  -50, 
     -49,  -48,  -47,  -46,  -45,  -44,  -43,  -42,  -41,  -39, 
     -38,  -37,  -36,  -34,  -33,  -32,  -30,  -29,  -27,  -26, 
     -24,  -23,  -22,  -20,  -19,  -17,  -16,  -14,  -12,  -11, 
      -9,   -8,   -6,   -5,   -3,   -2
};

int a, b, c, fi1, fi2;
int p0h, p0v, p1h, p1v, p2h, p2v, p3h, p3v;
