//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the equation interpreter.
//|________________________________________________________________



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This is a procedure type, accepted by Evaluate Equation.  The
//| procedure should accept a variable name as the first parameter
//| and should return the value of that variable as the second
//| parameter.  It should return a zero if there were no problems,
//| or a non-zero if the variable is unknown.
//|________________________________________________________________

typedef Boolean (*variable_value_proc)(char *, double *);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure EvaluateEquation
//|
//| Purpose: This procedure takes a string containing a mathematical expression
//|          and evaluates it, returning the floating point value.
//|
//| Parameters: expression: the string expression to evaluate
//|             value:      receives the value of the expression
//|             proc:       called when a variable is encountered.  This
//|                         procedure should set its second parameter (a
//|                         double) to the value of the variable whose name is
//|                         the first parameter (a string).  It should return a
//|                         zero if there were no problems, or non-zero if the
//|                         variable name was invalid.
//|             returns error code.
//|_____________________________________________________________________________

short EvaluateEquation(char *expression, double *value, variable_value_proc proc);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure GetErrorMessage
//|
//| Purpose: This procedure takes an error code, as returned by
//|          evaluate_equation, and returns a error message string.
//|
//| Parameters: error: the error code
//|             returns error message string
//|____________________________________________________________________________

char *GetErrorMessage(short error);
