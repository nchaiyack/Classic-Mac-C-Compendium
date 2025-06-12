
/* binary.c */
char GPFAR *gpfaralloc(unsigned long, char *);
char GPFAR *gpfarrealloc(char GPFAR *, unsigned long);
void gpfarfree(char GPFAR *);
int is_binary_file(FILE *);
int fread_matrix(FILE *, float GPFAR *GPFAR *GPFAR *, int *, int *, float GPFAR GPFAR *GPFAR *, float GPFAR GPFAR *GPFAR *);
int fwrite_matrix(FILE *, float GPFAR *GPFAR *, int, int, int, int, float GPFAR GPFAR *, float GPFAR GPFAR *);
float GPFAR *vector(int, int);
void free_vector(float GPFAR *, int, int);
float GPFAR *extend_vector(float GPFAR *, int, int, int);
float GPFAR *retract_vector(float GPFAR *, int, int, int);
float GPFAR *GPFAR *matrix(int, int, int, int);
void free_matrix(float GPFAR *GPFAR *, unsigned, unsigned, unsigned, unsigned);
float GPFAR *GPFAR *extend_matrix(float GPFAR *GPFAR *, int, int, int, int, int, int);
float GPFAR *GPFAR *retract_matrix(float GPFAR *GPFAR *, int, int, int, int, int, int);
float GPFAR *GPFAR *convert_matrix(float GPFAR *, int, int, int, int);
void free_convert_matrix(float GPFAR *GPFAR *, int, int, int, int);
