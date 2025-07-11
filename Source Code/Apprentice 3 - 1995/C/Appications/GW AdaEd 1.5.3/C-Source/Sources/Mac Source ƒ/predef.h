/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */
/*
 * Copyright (C) 1985-1992  New York University
 * 
 * This file is part of the Ada/Ed-C system.  See the Ada/Ed README file for
 * warranty (none) and distribution info and also the GNU General Public
 * License for more details.

 */

/* Constants For PRDEF Operations */

/*  The values of these constants are arranged to make the tests in the routine
 *  get_file_argument_or_default work correctly as follows for the cases where
 *  there are pairs of routines, one with a file argument and one using the
 *  default file:
 *
 *    For all ops where a file is given, 0 <= operation <= P_P_FILE
 *    For all ops using the default input file, P_P_FILE < operation <= P_P_IN
 *    For all ops using the default output file, P_P_IN < operation <= P_P_OUT
 *    For all ops not in this category, P_P_OUT < operation
 */

/* Operations where a file argument is given */

#define P_SET_LINE_LENGTH_FILE	  1
#define P_SET_PAGE_LENGTH_FILE	  2
#define P_LINE_LENGTH_FILE	  3
#define P_PAGE_LENGTH_FILE	  4
#define P_NEW_LINE_FILE 	  5
#define P_SKIP_LINE_FILE	  6
#define P_END_OF_LINE_FILE	  7
#define P_NEW_PAGE_FILE 	  8
#define P_SKIP_PAGE_FILE	  9
#define P_END_OF_PAGE_FILE	 10
#define P_TIO_END_OF_FILE_FILE	 11
#define P_SET_COL_FILE		 12
#define P_SET_LINE_FILE 	 13
#define P_COL_FILE		 14
#define P_LINE_FILE		 15
#define P_PAGE_FILE		 16
#define P_GET_CHAR_FILE_ITEM	 17
#define P_PUT_CHAR_FILE_ITEM	 18
#define P_GET_STRING_FILE_ITEM	 19
#define P_PUT_STRING_FILE_ITEM	 20
#define P_GET_LINE_FILE 	 21
#define P_PUT_LINE_FILE 	 22
#define P_GET_INTEGER_FILE_ITEM  23
#define P_PUT_INTEGER_FILE_ITEM  24
#define P_PUT_INTEGER_STRING	 25
#define P_GET_FLOAT_FILE_ITEM	 26
#define P_PUT_FLOAT_FILE_ITEM	 27
#define P_GET_FIXED_FILE_ITEM	 28
#define P_PUT_FIXED_FILE_ITEM	 29
#define P_GET_ENUM_FILE_ITEM	 30
#define P_PUT_ENUM_FILE_ITEM	 31
#define P_P_FILE		 31

/* Operations using default input file */

#define P_GET_CHAR_ITEM 	 32
#define P_GET_STRING_ITEM	 33
#define P_GET_LINE		 34
#define P_GET_INTEGER_ITEM	 35
#define P_GET_INTEGER_STRING	 36
#define P_GET_FLOAT_ITEM	 37
#define P_GET_FLOAT_STRING	 38
#define P_GET_FIXED_ITEM	 39
#define P_GET_FIXED_STRING	 40
#define P_GET_ENUM_ITEM 	 41
#define P_GET_ENUM_STRING	 42
#define P_SKIP_LINE		 43
#define P_END_OF_LINE		 44
#define P_SKIP_PAGE		 45
#define P_END_OF_PAGE		 46
#define P_TIO_END_OF_FILE	 47
#define P_P_IN			 47

/* Operations using default output file */

#define P_SET_LINE_LENGTH	 48
#define P_SET_PAGE_LENGTH	 49
#define P_LINE_LENGTH		 50
#define P_PAGE_LENGTH		 51
#define P_NEW_LINE		 52
#define P_NEW_PAGE		 53
#define P_SET_COL		 54
#define P_SET_LINE		 55
#define P_COL			 56
#define P_LINE			 57
#define P_PAGE			 58
#define P_PUT_CHAR_ITEM 	 59
#define P_PUT_STRING_ITEM	 60
#define P_PUT_LINE		 61
#define P_PUT_INTEGER_ITEM	 62
#define P_PUT_FLOAT_ITEM	 63
#define P_PUT_FLOAT_STRING	 64
#define P_PUT_FIXED_ITEM	 65
#define P_PUT_FIXED_STRING	 66
#define P_PUT_ENUM_ITEM 	 67
#define P_PUT_ENUM_STRING	 68
#define P_P_OUT 		 68

/* Other operations */

#define P_TIO_CREATE		 69
#define P_TIO_OPEN		 70
#define P_TIO_CLOSE		 71
#define P_TIO_DELETE		 72
#define P_TIO_RESET		 73
#define P_TIO_RESET_MODE	 74
#define P_TIO_MODE		 75
#define P_TIO_NAME		 76
#define P_TIO_FORM		 77
#define P_TIO_IS_OPEN		 78
#define P_SET_INPUT		 79
#define P_SET_OUTPUT		 80
#define P_STANDARD_INPUT	 81
#define P_STANDARD_OUTPUT	 82
#define P_CURRENT_INPUT 	 83
#define P_CURRENT_OUTPUT	 84
#define P_SIO_CREATE		 85
#define P_SIO_OPEN		 86
#define P_SIO_CLOSE		 87
#define P_SIO_DELETE		 88
#define P_SIO_RESET		 89
#define P_SIO_RESET_MODE	 90
#define P_SIO_MODE		 91
#define P_SIO_NAME		 92
#define P_SIO_FORM		 93
#define P_SIO_IS_OPEN		 94
#define P_SIO_READ		 95
#define P_SIO_WRITE		 96
#define P_SIO_END_OF_FILE	 97
#define P_DIO_CREATE		 98
#define P_DIO_OPEN		 99
#define P_DIO_CLOSE		100
#define P_DIO_DELETE		101
#define P_DIO_RESET		102
#define P_DIO_RESET_MODE	103
#define P_DIO_MODE		104
#define P_DIO_NAME		105
#define P_DIO_FORM		106
#define P_DIO_IS_OPEN		107
#define P_DIO_READ		108
#define P_DIO_READ_FROM 	109
#define P_DIO_WRITE		110
#define P_DIO_WRITE_TO		111
#define P_DIO_SET_INDEX 	112
#define P_DIO_INDEX		113
#define P_DIO_SIZE		114
#define P_DIO_END_OF_FILE	115
#define P_CLOCK 		116
#define P_YEAR			117
#define P_MONTH 		118
#define P_DAY			119
#define P_SECONDS		120
#define P_SPLIT 		121
#define P_TIME_OF		122
#define P_ADD_TIME_DUR		123
#define P_ADD_DUR_TIME		124
#define P_SUB_TIME_DUR		125
#define P_SUB_TIME_TIME 	126
#define P_LT_TIME		127
#define P_LE_TIME		128
#define P_GT_TIME		129
#define P_GE_TIME		130


/* predef1 */
void predef(void);
void predef_raise(int, char *);

/* predef2 */
void calendar(void);

/* predef3 */
int *get_argument_ptr(int);
void get_string_value(int);
char *make_string(void);
int get_argument_value(int);
float get_float_argument_value(int);
long get_long_argument_value(int);
void get_filenum(void);
void get_file_argument_or_default(void);
void return_string(char *, int);

/* predef4 */
void initialize_predef(void);
void check_opened_ok(void);
void check_file_open(void);
void check_status(int);
void open_textio(char);
void load_look_ahead(int);
void close_textio(void);
char get_char(void);
void skip_line(void);
void put_blanks(int);
void put_char(char);
void put_line(void);
void put_page(void);
void put_string(char *);
void put_buffer(char *, int, char);
void open_seq_io(int);
void open_dir_io(int);
void close_file(void);
void predef_term(void);
char *predef_alloc(int);
void predef_free(char *);
#ifdef MAC_AND_PC
FILE *fopen_bin(char *, char *);
FILE *fopen_txt(char *, char *);
#endif

/* predef5 */
void scan_enum(void);
void scan_enum_string(int *);
int scan_integer(int *, int);
int scan_integer_string(int *, int *);
long scan_fixed(int *, int);
long scan_fixed_string(int *, int *);
float scan_float(int *, int);
float scan_float_string(int *, int *);
int enum_ord(int *, int, int);

/* predef6 */

void image_integer(int, int);
void image_fixed (long, int *, int, int, int);
void image_float (float, int, int, int);
void image_enum(int, int *);
