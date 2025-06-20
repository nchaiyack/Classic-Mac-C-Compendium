/* Interface for the equation interpreter */

typedef Boolean (*variable_value_proc)(char *, double *);

short evaluate_equation(char *expression, double *value, variable_value_proc proc);
char *get_error_message(short error);
