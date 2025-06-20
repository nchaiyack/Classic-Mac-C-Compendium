/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

#ifndef __segment_h
#define __segment_h

/* header file for Segment structure and constants */
typedef struct Segment_s {
    short seg_id;		/* code to identify segment */
    short seg_kind;		/* segment kind, one of SEGMENT_KIND...*/
    short seg_size;		/* size of entry in bytes */
#ifndef powerc	// align long-sized fields under 68K [Fabrizio Oddone]
	short	M68KeffPad;
#endif
    unsigned int seg_pos;	/* index of next element */
    unsigned int seg_maxpos;	/* maximum position value */
    unsigned int seg_dim;  	/* dimension of data area */
    unsigned int seg_extend; 	/* extension count */
    int **seg_ptr;		/* pointer tracking field */
    char *seg_data; 		/* data string as char */
} Segment_s;

typedef struct Segment_s *Segment;

#define SEGMENT_KIND_CODE 0
#define SEGMENT_KIND_DATA 1

#define SEG_ID  1985

#endif

#ifdef MACHINE_CODE
void print_data_segment(void);
#endif
void segment_append(Segment, Segment);
void seg_check(Segment);
void segment_empty(Segment);
void segment_free(Segment);
int segment_get_int(Segment, int);
unsigned int segment_get_pos(Segment);
unsigned int segment_get_maxpos(Segment);
int segment_get_off(Segment, int);
void segment_put_byte(Segment, int);
#ifdef _hdr_h
void segment_put_const(Segment, Const);
#endif
void segment_put_int(Segment, int);
void segment_put_long(Segment, long);
void segment_put_off(Segment, int, int);
void segment_put_real(Segment, double);
void segment_put_ref(Segment, int, int);
void segment_put_word(Segment, int);
void segment_set_pos(Segment, unsigned, unsigned);
Segment template_new(int, int, int, int **);
unsigned int PC();
#ifdef DEBUG
void zpseg(Segment);
#endif
