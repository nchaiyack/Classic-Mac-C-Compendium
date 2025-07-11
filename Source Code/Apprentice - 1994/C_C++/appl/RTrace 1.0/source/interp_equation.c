/*****************************************************************************\
* interp_equation.c                                                           *
*                                                                             *
* This file contains code which implements a general-purpose equation         *
* interpreter.  Given an equation string, it can return a floating point      *
* which is the evaluated numeric value of the equation.                       *
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "interp_equation.h"


/* error codes */
enum
	{
	NO_ERROR = 0,
	INVALID_NUMBER,
	UNKNOWN_FUNCTION,
	MISSING_RIGHT_PARENTHESIS,
	INVALID_VARIABLE
	};


/* error messages */
char *error_messages[] = 
	{
	"No error",						/* NO_ERROR */
	"Invalid number format",		/* INVALID_NUMBER */
	"Unknown function",				/* UNKNOWN_FUNCTION */
	"Missing right parenthesis",	/* MISSING_RIGHT_PARENTHESIS */
	"Unknown variable"				/* INVALID_VARIABLE */
	};


/* built-in functions */
char *built_in_functions[] =
	{
	"sin",
	"cos",
	"tan",
	"exp",
	"sqrt",
	"abs",
	"ln",
	"log",
	"int",
	"atan",
	"acos",
	"asin",
	"cosh",
	"sinh",
	"tanh"
	};

enum
	{
	SIN = 0,
	COS,
	TAN,
	EXP,
	SQRT,
	ABS,
	LN,
	LOG,
	INT,
	ATAN,
	ACOS,
	ASIN,
	COSH,
	SINH,
	TANH
	};

/* Globals */
variable_value_proc var_proc;

/* Prototypes */
short get_expression_value(char **expression, double *sum);
short get_addend_value(char **addend, double *product);
short get_factor_value(char **factor, double *power_fact);
short get_power_factor_value(char **power_factor, double *value);
short get_variable_or_function_value(char **name, double *value);
short get_number(char **number, double *value);


/*****************************************************************************\
*                                                                             *
* Terminology:                                                                *
*                                                                             *
*   An expression is a string of characters which make up a mathematical      *
*   expression.  Expressions may contain any valid mathematical construction. *
*                                                                             *
*   An addend is like an expression, but it may not contain + or -            *
*                                                                             *
*   A factor is like an addend, but it may not contain * or /                 *                                                                          *
*                                                                             *
*   A power factor is like a factor, but it may not contain ^.  Anything in   *
*   parentheses is considered a power factor, even if it contains ^.  Any     *
*   standard function like sin or tan is also a power factor.                 *
*                                                                             *
* Examples:                                                                   *
*                                                                             *
*   Expressions:   a+2, b*7, (2+5)*67, x^2-2*x+5, y                           *
*   Addends:       a*2, 23/7, x^2+5, 3                                        *
*   Factors:       5, a, x^y                                                  *
*   Power Factors: 8, x, (x-3), ((65-y)*x)), (x-2)^12, (x^2), sin(x), cos(4)  *
*                                                                             *
\*****************************************************************************/



/*****************************************************************************\
* procedure evaluate_equation                                                 *
*                                                                             *
* Purpose: This procedure takes a string containing a mathematical expression *
*          and evaluates it, returning the floating point value.              *
*                                                                             *
* Parameters: expression: the string expression to evaluate                   *
*             value:      receives the value of the expression                *
*             proc:       called when a variable is encountered.  This        *
*                         procedure should set its second parameter (a        *
*                         double) to the value of the variable whose name is  *
*                         the first parameter (a string).  It should return a *
*                         zero if there were no problems, or non-zero if the  *
*                         variable name was invalid.                          *
*             returns error code.                                             *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short evaluate_equation(char *expression, double *value, variable_value_proc proc)
{

	short	i;
	short	error;
	char	temp_string[255];
	char	*expression_ptr = expression;
	
	/* Save the variable value procedure pointer in a global */
	var_proc = proc;
	
	/* Save the expression string */
	strcpy(temp_string, expression);	

	/* Convert the equation to lower case, and strip all spaces */
	i = 0;
	while (expression[i])
		{
		
		/* Convert this character to lower case */
		expression[i] = tolower(expression[i]);
		
		if (expression[i] == ' ')
			strcpy (expression + i, expression + i + 1);
		else
			i++;
		}

	/* Find the value of this equation, catch error if any */
	error = get_expression_value(&expression_ptr, value);

	/* Restore the original expression string */
	strcpy(expression, temp_string);	

	return error;
	
}	/* evaluate_equation() */



/*****************************************************************************\
* procedure get_error_message                                                 *
*                                                                             *
* Purpose: This procedure takes an error code, as returned by                 *
*          evaluate_equation, and returns a error message string.             *
*                                                                             *
* Parameters: error: the error code.                                          *
*             returns error message string.                                   *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

char *get_error_message(short error)
{

	/* return the error message */
	return (error_messages[error]);

}	/* get_error_message() */


/*****************************************************************************\
* procedure get_expression_value                                              *
*                                                                             *
* Purpose: This procedure takes a handle to a string containing an expression *
*          and evaluates it, returning the floating point value.              *
*                                                                             *
* Parameters: expression: handle to expression, after execution handle to     *
*                         character after expression.                         *
*             returns floating point value of expression                      *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_expression_value(char **expression, double *sum)
{

	char	operator;
	double	addend;
	short	error;
	Boolean	done = FALSE;
	
	/* Find the value of the first addend, trap error */
	if (error = get_addend_value(expression, sum))
		return error;

	while (!done)
		{
		
		/* Get the operator */
		operator = **expression;
		
		/* Perform the operation */
		switch (operator)
			{
			case '+':
			
				/* Move pointer to token after * */
				(*expression)++;
				
				/* Get next factor, pass along error if any */
				if (error = get_addend_value(expression, &addend))
					return error;
				
				/* Add it to the partial sum */
				*sum += addend;
				
				break;
			
			case '-':
			
				/* Move pointer to token after * */
				(*expression)++;
				
				/* Get next factor, pass along error if any */
				if (error = get_addend_value(expression, &addend))
					return error;
				
				/* Subtract it from the partial sum */
				*sum -= addend;
				
				break;
			
			default:
				done = TRUE;
				
			}
		}

	/* no error */
	return NO_ERROR;

}	/* get_expression_value() */



/*****************************************************************************\
* procedure get_addend_value                                                  *
*                                                                             *
* Purpose: This procedure takes a handle to a string containing an addend and *
*          evaluates it, returning the floating point value.                  *
*                                                                             *
* Parameters: addend: handle to addend, after execution handle to             *
*                     character after addend.                                 *
*             returns floating point value of addend                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_addend_value(char **addend, double *product)
{

	char	operator;
	double	factor;
	short	error;
	Boolean	done = FALSE;
	
	/* Find the value of the first factor, trap error */
	if (error = get_factor_value(addend, product))
		return error;

	while (!done)
		{
		
		/* Get the operator */
		operator = **addend;
		
		/* Perform the operation */
		switch (operator)
			{
			case '*':
				
				/* Move pointer to token after * */
				(*addend)++;
				
				/* Get next factor, pass along error if any */
				if (error = get_factor_value(addend, &factor))
					return error;
				
				/* Multiply it with the partial product */
				*product *= factor;
				
				break;
			
			case '/':
				
				/* Move pointer to token after * */
				(*addend)++;
				
				/* Get next factor, pass along error if any */
				if (error = get_factor_value(addend, &factor))
					return error;
				
				/* Divide the partial product by the factor */
				*product /= factor;
				
				break;
			
			default:
				done = TRUE;
				
			}
		}
	
	return (NO_ERROR);

}	/* get_addend_value() */



/*****************************************************************************\
* procedure get_factor_value                                                  *
*                                                                             *
* Purpose: This procedure takes a pointer to a containing a factor and        *
*          evaluates it, returning the floating point value.                  *
*                                                                             *
* Parameters: factor: handle to factor, after execution handle to             *
*                     character after factor.                                 *
*             returns floating point value of factor                          *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_factor_value(char **factor, double *total)
{

	double	power_fact;
	char	operator;
	short	error;
	Boolean	done = FALSE;
	
	/* Find the value of the first power factor, trap error */
	if (error = get_power_factor_value(factor, total))
		return error;

	while (!done)
		{
		
		/* Get the operator */
		operator = **factor;
		
		/* Perform the operation */
		switch (operator)
			{
			case '^':
				
				/* Move pointer to token after * */
				(*factor)++;
				
				/* Get next factor, pass along error if any */
				if (error = get_power_factor_value(factor, &power_fact))
					return error;
				
				/* Raise the partial result to the power_fact power */
				*total = pow(*total, power_fact);
				
				break;
			
			default:
				
				done = TRUE;
				
			}
		}
		
	return (NO_ERROR);

}	/* get_factor_value() */



/*****************************************************************************\
* procedure get_power_factor_value                                            *
*                                                                             *
* Purpose: This procedure takes a pointer to a containing a power factor and  *
*          evaluates it, returning the floating point value.                  *
*                                                                             *
* Parameters: base: handle to power factor, after execution handle to         *
*                   character after power factor.                             *
*             returns floating point value of power factor                    *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_power_factor_value(char **power_factor, double *value)
{
	
	short	error;
	
	/* If the first token is a parenthesis, evaluate the expression inside */
	if (**power_factor == '(')
		{
		
		/* evaluate the expression right after the left parenthesis */
		(*power_factor)++;
		if (error = get_expression_value(power_factor, value))
			return error;
		
		/* Skip the ) */
		if (**power_factor != ')')
			return MISSING_RIGHT_PARENTHESIS;
		else
			(*power_factor)++;

		
		}

	/* Not a parenthesis-- is it a variable or a function? */
	else if (isalpha(**power_factor) || (**power_factor == '�'))
		{
		if (error = get_variable_or_function_value(power_factor, value))
			return error;
		}
		
	/* It must be a number */
	else
		{
		
		/* Convert it to a floating point number, trap error */
		if (error = get_number(power_factor, value))
			return error;
	
		}	

	return (NO_ERROR);

}	/* get_power_factor_value() */



/*****************************************************************************\
* procedure get_variable_or_function_value                                    *
*                                                                             *
* Purpose: This procedure takes a pointer to a variable name or a function    *
*          name, and returns the value of that variable or the function.      *
*                                                                             *
* Parameters: name: handle to variable/function name.                         *
*             returns floating point value of variable/function               *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_variable_or_function_value(char **name, double *value)
{

	char	*var_name = *name;	/* find start of the name */
	short	error;
	char	next_token;
	short	i;

	/* find the end of the name */
	while (isalnum(**name) || (**name == '�'))
		(*name)++;
		
	/* Remember the token after the name */
	next_token = **name;
	
	/* Change the token after the name to an end-of-string token */
	**name = '\0';

	/* If the next token is a (, this must be a function */
	if (next_token == '(')
		{
		
		/* Move to the token after the ( */
		(*name)++;

		/* Find which built-in function it is */
		i = SIN;
		for (i = SIN; i <= TANH; i++)
			if (!strcmp(var_name, built_in_functions[i]))
				break;

		/* If it's not a valid function, generate an error */
		if (i > TANH)
			return (UNKNOWN_FUNCTION);
		
		/* Get the value of the expression in the parentheses, check error */
		if (error = get_expression_value(name, value))
			return error;

		/* apply the function to the expression */
		switch (i)
			{
			case SIN:
				*value = sin(*value);
				break;

			case COS:
				*value = cos(*value);
				break;

			case TAN:
				*value = tan(*value);
				break;

			case EXP:
				*value = exp(*value);
				break;

			case SQRT:
				*value = sqrt(*value);
				break;

			case ABS:
				*value = abs(*value);
				break;

			case LN:
				*value = log(*value);
				break;

			case LOG:
				*value = log10(*value);
				break;

			case INT:
				*value = floor(*value);
				break;

			case ATAN:
				*value = atan(*value);
				break;

			case ACOS:
				*value = acos(*value);
				break;

			case ASIN:
				*value = asin(*value);
				break;

			case COSH:
				*value = cosh(*value);
				break;

			case SINH:
				*value = sinh(*value);
				break;

			case TANH:
				*value = tanh(*value);
				break;
			}

		/* Skip the ) */
		if (**name != ')')
			return MISSING_RIGHT_PARENTHESIS;
		else
			(*name)++;

		}
	
	else	/* it's a variable */
		{
		
		/* check for pi */
		if ((!strcmp(var_name, "pi")) || (!strcmp(var_name, "�")))
			*value = 3.14159265358979323846;
		
		/* check for e */
		else if (!strcmp(var_name, "e"))
			*value = 2.71828182846;
		
		/* otherwise, request the value from the caller */
		else if ((*var_proc)(var_name, value))
			return INVALID_VARIABLE;
		
		/* Restore the token after the name */
		**name = next_token;
	
		}

	/* Return error, if any */
	return NO_ERROR;

}	/* get_variable_or_function_value() */



/*****************************************************************************\
* procedure get_number                                                        *
*                                                                             *
* Purpose: This procedure gets the value of a number in the expression.       *
*                                                                             *
* Parameters: number_string: handle to number in the expression string.       *
*             value:         receives floating point value of number          *
*             returns error code                                              *
*                                                                             *
* Created by: Greg Ferrar                                                     *
* Created on: September 9, 1992                                               *
* Modified:                                                                   *
\*****************************************************************************/

short get_number(char **number, double *value)
{

	char	*number_string = *number;	/* find beginning of number */
	char	next_token;
	short	error;

	/* accept unary - at beginning */
	if (**number == '-')
		(*number)++;
	
	/* there should be a sequence of 0 or more digits next */
	while (isdigit(**number))
		(*number)++;
	
	/* next there could be a decimal point... */
	if (**number == '.')
		{
		
		/* Go to next token */
		(*number)++;
		
		/* Read 0 or more digits after the decimal point */
		while (isdigit(**number))
			(*number)++;
		
		}
	
	/* next there could be an exponent */
	if ((**number == 'e') || (**number == 'E'))
		{
		
		/* Go to next token */
		(*number)++;
		
		/* there could be an unary - before the exponent */
		if (**number == '-')
			(*number)++;

		/* Read 1 or more digits after the decimal point */
		if (!isdigit(**number))
			error = INVALID_NUMBER;
		do
			(*number)++;
		while (isdigit(**number));
		
		}
	
	/* Save the token right after the number */
	next_token = **number;
	
	/* Replace the token with end-of-string character */
	**number = 0;

	/* Convert the number to a floating point number */
	*value = atof(number_string);

	/* Restore the token */
	**number = next_token;

	/* no error encountered */
	return NO_ERROR;

}	/* get_number() */


