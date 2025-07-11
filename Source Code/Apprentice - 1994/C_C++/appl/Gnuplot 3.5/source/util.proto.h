
/* util.c */
int chr_in_str(int, char);
int equals(int, char *);
int almost_equals(int, char *);
int isstring(int);
int isnumber(int);
int isletter(int);
int is_definition(int);
int copy_str(char[], int);
int quote_str(char[], int);
int quotel_str(char[], int);
int capture(char[], int, int);
int m_capture(char **, int, int);
int m_quote_capture(char **, int, int);
int convert(struct value *, int);
int disp_value(FILE *, struct value *);
double real(struct value *);
double imag(struct value *);
double magnitude(struct value *);
double angle(struct value *);
struct value *Gcomplex(struct value *, double, double);
struct value *Ginteger(struct value *, int);
int os_error(char[], int);
int int_error(char[], int);
void lower_case(char *);
void squash_spaces(char *);
