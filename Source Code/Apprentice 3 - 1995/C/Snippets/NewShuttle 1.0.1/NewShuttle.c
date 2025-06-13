// NewShuttle 1.0.1
// inspired by code from Eddy Vasile (70451.3333@compuserve.com)
// ported to CodeWarrior by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

//¥ Plot points collected from a fortran bbs in 86

//¥ A note from Ken Long:
//¥ This was my first successfull Pascal to C port!  I did have a bit of
//¥ help from Mark Hanrek, of AOL fame (he likes to help novice programmers).
//¥ I guess I'm going to have to stop calling myself a "novice" because I'm
//¥ learning too much!  But I this to help other beginning  C programmers.
//¥ There's nothing special about this source code, so it's public domain.
//¥ I added a 'BNDL' and some color icons, for effect.
//¥ I hope you have as much fun fiddling with this program as I did!
//¥ I'm a self-taught C programmer and Pascal porter.


//¥ program ShuttleViewer;

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int 	main (void);
void 	Do_Init_Managers (void);
void 	Set_Up_Window (void);
int 	Init_Variables (void);
int		Main_Event_Loop (void);

int 	Calculate_1 (void);
int 	Calculate_2 (void);
int 	Calculate_3 (void);
int 	Set_Data_Array (void);

//************************************************************************

#define Max_A	300
#define Max_B	124
#define Max_C	259
#define Max_D	631
#define r1		400

short o_X_angle, o_Y_angle, o_Z_angle, Pc, Ec;

float 	CH, SH, CP, SP, CB, SB, 
		XV, YV, ZV, 
		X, Y, Z, 
		X3, Y3, Z3, 
		AM, BM, CM, DM, EM, FM, GM, HM, IM, 
		D, P, B, H, U, 
		Vc, 
		U1, V1;

double V[Max_A + 1][3];
double E[Max_A + 1];
double data[Max_D];

WindowPtr 	window;
long 		ticks;

main (void)
{
	Do_Init_Managers ();
	Set_Data_Array ();
	Init_Variables ();
	Set_Up_Window ();
	Main_Event_Loop ();
	ExitToShell ();
}

void Do_Init_Managers (void)
{
	MaxApplZone ();

	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
	InitCursor ();
}

int Init_Variables ()
{
	short position, j, Ec, Pc;

	position = 0;
	for (Pc = 1; Pc <= Max_B; Pc++)
	{
		for (j = 0; j < 3; j++)
		{
			position = position + 1;
			V [Pc] [j] = data [position] * 0.12; 	//¥ This number controls
		}											//¥ scale.  Try "0.2"
	}
	for (Ec = 0; Ec < Max_C; Ec++)
	{
		position = position + 1;
		E [Ec] = data [position];
	}
}

void Set_Up_Window (void)
{
	window = GetNewWindow (r1, nil, (WindowPtr)-1L);
	ShowWindow (window);
	SetPort (window);
	
	MoveTo(200, 170);
	TextSize(10);
	DrawString("\pWait for a couple of views before clicking.");
	
	MoveTo (10, 250);
	TextSize (14);
	TextFace (shadow);
	DrawString ("\pEddy and Kenny's Shuttle Viewer ");
	
	MoveTo (20, 270);
	TextSize (12);
	TextFace (bold);
	DrawString ("\pClick and hold button down to STOP (wait a couple of views).");

	MoveTo (30, 290);
	TextSize (10);
	TextFace (italic);

	DrawString ("\pThink Pascal Code by Eddy Vasile. Plot Points from anonymous source.");
	MoveTo (40, 310);
	TextSize (12);
	TextFace (bold);

	DrawString ("\pPorted to Code Warrior by Ken Long, 20 Nov 1994");
	PenNormal ();
	TextMode (srcCopy); //¥ black text with white background
	TextSize (9);
	TextFont (monaco);
	TextFace (0);
	Delay(100, &ticks);		//¥ Delay between views.
}

//************************************************************************

int Main_Event_Loop ()
{
	Rect frameRect, shuttleRect;
	
	SetRect (&frameRect, 160, 2, 505, 211);
	FrameRect (&frameRect);
	SetRect (&shuttleRect, 163, 5, 502, 208);
	while (! Button ())
	{
		BackColor (blackColor);
		ForeColor (whiteColor);		//¥ Try redColor, if you have color.
		EraseRect(&shuttleRect);
		Calculate_3 ();
		Delay (50, &ticks);
		o_X_angle = o_X_angle + 10;
		o_Y_angle = o_Y_angle + 10;
		o_Z_angle = o_Z_angle + 10;
	}
}



//************************************************************************

int Calculate_1 (void)
{
	CH =  cos (H);
	SH =  sin (H);
	CP =  cos (P);
	SP =  sin (P);
	CB =  cos (B);
	SB =  sin (B);
	AM =  CB * CH - SH * SP * SB;
	BM = -CB * SH - SP * CH * SB;
	CM =  CP * SB;
	DM =  SH * CP;
	EM =  CP * CH;
	FM =  SP;
	GM = -CH * SB - SH * SP * CB;
	HM =  SH * SB - SP * CH * CB;
	IM =  CP * CB;
}

//************************************************************************

int Calculate_2 ()
{
	X  = X - XV;
	Y  = Y - YV;
	Z  = Z - ZV;
	X3 = AM * X + BM * Y + CM * Z;
	Y3 = DM * X + EM * Y + FM * Z;
	Z3 = GM * X + HM * Y + IM * Z;
	U  = 135 + 13.5 * D * X3 / Y3;	//¥ Shuttle stretch/shrink width.
	Vc = 80 - 12.5 * D * Z3 / Y3;	//¥ Shuttle stretch/shrink length.
}									//¥ (Use 1st or 2nd number.)

//************************************************************************

int Calculate_3 ()
{
	short 	Ec;
	short temp_X, temp_Y, current_line = 1, current_column = 0;
	Str255	number_string;

	X  = 0;
	Y  = 0;
	Z  = 0;
	X3 = 0;
	Y3 = 0;
	Z3 = 0;
	AM = 0;
	BM = 0;
	CM = 0;
	DM = 0;
	EM = 0;
	FM = 0;
	GM = 0;
	HM = 0;
	IM = 0;
	D  = 0;
	P  = 0;
	B  = 0;
	H  = 0;
	U  = 0;
	Vc = 0;
	U1 = 0;
	V1 = 0;
	D  = 120;
	P  = 6.28 * o_X_angle / 255 - 3.1416;	//¥ Remove "- 3.1416" to start
	B  = 6.28 * o_Z_angle / 255;			//¥ with nose-up view.
	H  = 6.28 * o_Y_angle / 255;

	Calculate_1 ();
	XV = - D * CP * SH;
	YV = - D * CP * CH;
	ZV = - D * SP;
	for (Ec = 0; Ec < Max_C; Ec++)		//¥ Starting point.
	{
		X = V [abs (ceil (E [Ec]))] [0];
		Y = V [abs (ceil (E [Ec]))] [1];
		Z = V [abs (ceil (E [Ec]))] [2];

		Calculate_2 ();

		if (E [Ec] > 0)
		{
			MoveTo (ceil (U1 * 2 + 60), ceil (V1 + 40));	//¥ Left and top.
			LineTo (ceil (U  * 2 + 60), ceil (Vc + 40));	//¥ Left and top.
		}
		U1 = U;
		V1 = Vc;
//¥		Delay (5, &ticks);	//¥ I left this out but uncomment for slowDraw.
	}
}

//************************************************************************

int Set_Data_Array (void)
{
	data[  1] =    0; data[  2] =   -2; data[  3] =   46; data[  4] =    2;
	data[  5] =   -3; data[  6] =   46; data[  7] =    2; data[  8] =   -5;
	data[  9] =   46; data[ 10] =    2; data[ 11] =   -7; data[ 12] =   46;
	data[ 13] =    0; data[ 14] =   -7; data[ 15] =   46; data[ 16] =   -2;
	data[ 17] =   -7; data[ 18] =   46; data[ 19] =   -2; data[ 20] =   -5;
	data[ 21] =   46; data[ 22] =   -2; data[ 23] =   -3; data[ 24] =   46;
	data[ 25] =    0; data[ 26] =   -1; data[ 27] =   43; data[ 28] =    3;
	data[ 29] =   -2; data[ 30] =   43; data[ 31] =    4; data[ 32] =   -5;
	data[ 33] =   43; data[ 34] =    3; data[ 35] =   -7; data[ 36] =   43;
	data[ 37] =    0; data[ 38] =   -8; data[ 39] =   43; data[ 40] =   -3;
	data[ 41] =   -7; data[ 42] =   43; data[ 43] =   -4; data[ 44] =   -5;
	data[ 45] =   43; data[ 46] =   -3; data[ 47] =   -2; data[ 48] =   43;
	data[ 49] =    0; data[ 50] =    2; data[ 51] =   38; data[ 52] =    5;
	data[ 53] =    0; data[ 54] =   38; data[ 55] =    6; data[ 56] =   -4;
	data[ 57] =   38; data[ 58] =    4; data[ 59] =   -8; data[ 60] =   38;
	data[ 61] =    0; data[ 62] =   -9; data[ 63] =   38; data[ 64] =   -4;
	data[ 65] =   -8; data[ 66] =   38; data[ 67] =   -6; data[ 68] =   -4;
	data[ 69] =   38; data[ 70] =   -5; data[ 71] =    0; data[ 72] =   38;
	data[ 73] =    0; data[ 74] =    4; data[ 75] =   33; data[ 76] =    5;
	data[ 77] =    1; data[ 78] =   33; data[ 79] =    6; data[ 80] =   -5;
	data[ 81] =   33; data[ 82] =    4; data[ 83] =   -9; data[ 84] =   33;
	data[ 85] =    0; data[ 86] =  -10; data[ 87] =   33; data[ 88] =   -4;
	data[ 89] =   -9; data[ 90] =   33; data[ 91] =   -6; data[ 92] =   -5;
	data[ 93] =   33; data[ 94] =   -5; data[ 95] =    1; data[ 96] =   33;
	data[ 97] =    0; data[ 98] =    8; data[ 99] =   26; data[100] =    4;
	data[101] =    7; data[102] =   26; data[103] =    8; data[104] =    2;
	data[105] =   26; data[106] =    8; data[107] =   -7; data[108] =   26;
	data[109] =    0; data[110] =  -10; data[111] =   26; data[112] =   -8;
	data[113] =   -7; data[114] =   26; data[115] =   -8; data[116] =    2;
	data[117] =   26; data[118] =   -4; data[119] =    7; data[120] =   26;
	data[121] =    0; data[122] =    8; data[123] =   22; data[124] =    4;
	data[125] =    8; data[126] =   22; data[127] =    8; data[128] =    3;
	data[129] =   22; data[130] =    8; data[131] =   -8; data[132] =   22;
	data[133] =    0; data[134] =  -10; data[135] =   22; data[136] =   -8;
	data[137] =   -8; data[138] =   22; data[139] =   -8; data[140] =    3;
	data[141] =   22; data[142] =   -4; data[143] =    8; data[144] =   22;
	data[145] =    0; data[146] =    8; data[147] =   14; data[148] =    5;
	data[149] =    7; data[150] =   14; data[151] =    8; data[152] =    4;
	data[153] =   14; data[154] =    8; data[155] =   -9; data[156] =   14;
	data[157] =    0; data[158] =  -10; data[159] =   14; data[160] =   -8;
	data[161] =   -9; data[162] =   14; data[163] =   -8; data[164] =    4;
	data[165] =   14; data[166] =   -5; data[167] =    7; data[168] =   14;
	data[169] =    0; data[170] =    8; data[171] =    4; data[172] =    5;
	data[173] =    7; data[174] =    4; data[175] =    8; data[176] =    4;
	data[177] =    4; data[178] =    8; data[179] =   -9; data[180] =    4;
	data[181] =    0; data[182] =  -10; data[183] =    4; data[184] =   -8;
	data[185] =   -9; data[186] =    4; data[187] =   -8; data[188] =    4;
	data[189] =    4; data[190] =   -5; data[191] =    7; data[192] =    4;
	data[193] =    0; data[194] =    8; data[195] =  -12; data[196] =    5;
	data[197] =    7; data[198] =  -12; data[199] =    8; data[200] =    4;
	data[201] =  -12; data[202] =    8; data[203] =   -9; data[204] =  -12;
	data[205] =    0; data[206] =  -10; data[207] =  -12; data[208] =   -8;
	data[209] =   -9; data[210] =  -12; data[211] =   -8; data[212] =    4;
	data[213] =  -12; data[214] =   -5; data[215] =    7; data[216] =  -12;
	data[217] =    0; data[218] =    8; data[219] =  -27; data[220] =    5;
	data[221] =    7; data[222] =  -27; data[223] =    8; data[224] =    4;
	data[225] =  -27; data[226] =    8; data[227] =   -9; data[228] =  -27;
	data[229] =    0; data[230] =  -10; data[231] =  -27; data[232] =   -8;
	data[233] =   -9; data[234] =  -27; data[235] =   -8; data[236] =    4;
	data[237] =  -27; data[238] =   -5; data[239] =    7; data[240] =  -27;
	data[241] =    0; data[242] =    8; data[243] =  -36; data[244] =    5;
	data[245] =    7; data[246] =  -36; data[247] =    8; data[248] =    4;
	data[249] =  -36; data[250] =    8; data[251] =   -9; data[252] =  -36;
	data[253] =    0; data[254] =  -10; data[255] =  -36; data[256] =   -8;
	data[257] =   -9; data[258] =  -36; data[259] =   -8; data[260] =    4;
	data[261] =  -36; data[262] =   -5; data[263] =    7; data[264] =  -36;
	data[265] =    0; data[266] =    9; data[267] =  -43; data[268] =    2;
	data[269] =    9; data[270] =  -43; data[271] =    9; data[272] =    2;
	data[273] =  -43; data[274] =    9; data[275] =  -10; data[276] =  -43;
	data[277] =    0; data[278] =  -11; data[279] =  -43; data[280] =   -9;
	data[281] =  -10; data[282] =  -43; data[283] =   -9; data[284] =    2;
	data[285] =  -43; data[286] =   -2; data[287] =    9; data[288] =  -43;
	data[289] =    0; data[290] =   10; data[291] =  -48; data[292] =    2;
	data[293] =    9; data[294] =  -48; data[295] =    9; data[296] =    2; 
	data[297] =  -48; data[298] =   10; data[299] =  -10; data[300] =  -48; 
	data[301] =    0; data[302] =  -10; data[303] =  -48; data[304] =  -10; 
	data[305] =  -10; data[306] =  -48; data[307] =   -9; data[308] =    2; 
	data[309] =  -48; data[310] =   -2; data[311] =    9; data[312] =  -48; 
	data[313] =    9; data[314] =   -9; data[315] =   21; data[316] =   15; 
	data[317] =   -9; data[318] =  -16; data[319] =   35; data[320] =  -10;
	data[321] =  -36; data[322] =   35; data[323] =  -10; data[324] =  -40;
	data[325] =   -9; data[326] =   -9; data[327] =   21; data[328] =  -15;
	data[329] =   -9; data[330] =  -16; data[331] =  -35; data[332] =  -10;
	data[333] =  -36; data[334] =  -35; data[335] =  -10; data[336] =  -40;
	data[337] =    0; data[338] =   13; data[339] =  -37; data[340] =    0;
	data[341] =   33; data[342] =  -60; data[343] =    0; data[344] =   33;
	data[345] =  -69; data[346] =    0; data[347] =   14; data[348] =  -60;
	data[349] =    6; data[350] =   11; data[351] =  -43; data[352] =    6;
	data[353] =   11; data[354] =  -48; data[355] =   11; data[356] =    5;
	data[357] =  -43; data[358] =   11; data[359] =    5; data[360] =  -48;
	data[361] =   -6; data[362] =   11; data[363] =  -43; data[364] =   -6;
	data[365] =   11; data[366] =  -48; data[367] =  -11; data[368] =    5;
	data[369] =  -43; data[370] =  -11; data[371] =    5; data[372] =  -48;
	data[373] =   -1; data[374] =    2; data[375] =    3; data[376] =    4;
	data[377] =    5; data[378] =    6; data[379] =    7; data[380] =    8;
	data[381] =    1; data[382] =   -9; data[383] =   10; data[384] =   11;
	data[385] =   12; data[386] =   13; data[387] =   14; data[388] =   15;
	data[389] =   16; data[390] =    9; data[391] =  -17; data[392] =   18;
	data[393] =   19; data[394] =   20; data[395] =   21; data[396] =   22;
	data[397] =   23; data[398] =   24; data[399] =   17; data[400] =  -25;
	data[401] =   26; data[402] =   27; data[403] =   28; data[404] =   29;
	data[405] =   30; data[406] =   31; data[407] =   32; data[408] =   25;
	data[409] =  -33; data[410] =   34; data[411] =   35; data[412] =   36;
	data[413] =   37; data[414] =   38; data[415] =   39; data[416] =   40;
	data[417] =   33; data[418] =  -41; data[419] =   42; data[420] =   43;
	data[421] =   44; data[422] =   45; data[423] =   46; data[424] =   47;
	data[425] =   48; data[426] =   41; data[427] =  -49; data[428] =   50;
	data[429] =   51; data[430] =   52; data[431] =   53; data[432] =   54;
	data[433] =   55; data[434] =   56; data[435] =   49; data[436] =  -57;
	data[437] =   58; data[438] =   59; data[439] =   60; data[440] =   61;
	data[441] =   62; data[442] =   63; data[443] =   64; data[444] =   57;
	data[445] =  -65; data[446] =   66; data[447] =   67; data[448] =   68;
	data[449] =   69; data[450] =   70; data[451] =   71; data[452] =   72;
	data[453] =   65; data[454] =  -73; data[455] =   74; data[456] =   75;
	data[457] =   76; data[458] =   77; data[459] =   78; data[460] =   79;
	data[461] =   80; data[462] =   73; data[463] =  -81; data[464] =   82;
	data[465] =   83; data[466] =   84; data[467] =   85; data[468] =   86;
	data[469] =   87; data[470] =   88; data[471] =   81; data[472] =  -89;
	data[473] =   90; data[474] =   91; data[475] =   92; data[476] =   93;
	data[477] =   94; data[478] =   95; data[479] =   96; data[480] =   89;
	data[481] =  -97; data[482] =   98; data[483] =   99; data[484] =  100;
	data[485] =  101; data[486] =  102; data[487] =  103; data[488] =  104;
	data[489] =   97; data[490] =   -1; data[491] =    9; data[492] =   17;
	data[493] =   25; data[494] =   33; data[495] =   41; data[496] =   49;
	data[497] =   57; data[498] =   65; data[499] =   73; data[500] =   81;
	data[501] =   89; data[502] =   97; data[503] =   -2; data[504] =   10;
	data[505] =   18; data[506] =   26; data[507] =   34; data[508] =   42;
	data[509] =   50; data[510] =   58; data[511] =   66; data[512] =   74;
	data[513] =   82; data[514] =   90; data[515] =   98; data[516] =   -3;
	data[517] =   11; data[518] =   19; data[519] =   27; data[520] =   35;
	data[521] =   43; data[522] =   51; data[523] =   59; data[524] =   67;
	data[525] =   75; data[526] =   83; data[527] =   91; data[528] =   99;
	data[529] =   -4; data[530] =   12; data[531] =   20; data[532] =   28;
	data[533] =   36; data[534] =   44; data[535] =   52; data[536] =   60;
	data[537] =   68; data[538] =   76; data[539] =   84; data[540] =   92;
	data[541] =  100; data[542] =   -5; data[543] =   13; data[544] =   21;
	data[545] =   29; data[546] =   37; data[547] =   45; data[548] =   53;
	data[549] =   61; data[550] =   69; data[551] =   77; data[552] =   85;
	data[553] =   93; data[554] =  101; data[555] =   -6; data[556] =   14;
	data[557] =   22; data[558] =   30; data[559] =   38; data[560] =   46;
	data[561] =   54; data[562] =   62; data[563] =   70; data[564] =   78;
	data[565] =   86; data[566] =   94; data[567] =  102; data[568] =   -7;
	data[569] =   15; data[570] =   23; data[571] =   31; data[572] =   39;
	data[573] =   47; data[574] =   55; data[575] =   63; data[576] =   71;
	data[577] =   79; data[578] =   87; data[579] =   95; data[580] =  103;
	data[581] =   -8; data[582] =   16; data[583] =   24; data[584] =   32;
	data[585] =   40; data[586] =   48; data[587] =   56; data[588] =   64;
	data[589] =   72; data[590] =   80; data[591] =   88; data[592] =   96;
	data[593] =  104; data[594] =  -44; data[595] =  105; data[596] =  106;
	data[597] =  107; data[598] =  108; data[599] =   92; data[600] =  -46;
	data[601] =  109; data[602] =  110; data[603] =  111; data[604] =  112;
	data[605] =   94; data[606] =  -81; data[607] =  113; data[608] =  114;
	data[609] =  115; data[610] =  116; data[611] =   89; data[612] =  -82;
	data[613] =  117; data[614] =  118; data[615] =  -83; data[616] =  119;
	data[617] =  120; data[618] =  -87; data[619] =  121; data[620] =  122;
	data[621] =  -88; data[622] =  123; data[623] =  124; data[624] = -117;
	data[625] =  119; data[626] = -121; data[627] =  123; data[628] = -118;
	data[629] =  120; data[630] = -122; data[631] =  124;
}

//¥ Replace the MoveTo and LineTo lines in the 'if' stanement in Calculate_3
//¥ with the following lines and instead of drawing the shuttle, it will
//¥ draw the point locations (left then top) over the entire window contents.

//			temp_X = ceil( U1 * 2 + 75 );
//			temp_Y = ceil( V1 + 20 );
//			
//			MoveTo((current_column * 100), current_line * 11);
//			NumToString((long) temp_X, number_string );			// show temp_X
//			DrawString( number_string );
//			
//			MoveTo((current_column * 100) + 50, current_line * 11);
//			NumToString((long) temp_Y, number_string );			// show temp_Y
//			DrawString( number_string );
//			
//			if ( current_line++ > 25 )
//			{
//				current_line = 1;
//				current_column++;
//			}
//			if ( current_column > 4 )
//			{
//				current_column = 0;
//			}
