char	*Get_keyword( char *s, short *keytype );
char	*Get_filename( char *s, StringPtr filename );
char 	*Get_value( char *s, float *value );

void Get_scales( char *s, float *hscale, float *vscale,
					short ht, short wd );

enum {
	sp_nokeyword = 0,
	sp_pict,
	sp_scale,
	sp_hscale,
	sp_vscale,
	sp_width,
	sp_height
};
