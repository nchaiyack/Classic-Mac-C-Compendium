//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains a procedure which creates an n-dimensional graph.
//| The object is generated using a parametric equation.
//|_______________________________________________________________________

#include "CHyperCuberPrefs.h"
#include "InterpEquation.h"

#include <fstream.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


//================================== Globals ================================\\

extern CHyperCuberPrefs	*gPrefs;

double *param_values;					//  This is global so that GetParamValue can access it.


//================================ Prototypes ===============================\\

Boolean CreateGraph(char *filename);
Boolean GetParamValue(char *name, double *value);

extern short EvaluateEquation(char *expression, double *value, variable_value_proc proc);
void GeneralErrorByString(char *error_string);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure CreateGraph
//|
//| Purpose: this procedure creates a graph, using
//|          the defined parametric equation.  The graph is represented
//|          by a series of paths which form a mesh connecting all vertices
//|          on the object.
//|
//| Parameters: filename: name of file for object description
//|             returns TRUE if error is encountered
//|____________________________________________________________________________

Boolean CreateGraph(char *filename)
{

	PrefsStruct prefs = gPrefs->prefs;					//  Get a copy of the prefs
	
	ofstream ofs;

	ofs.open(filename);									//  Open the output file
	
	ofs << 1 << '\n';									//  Write version number
	ofs << prefs.equation_dimension << '\n';			//  Write dimension of graph
	ofs << 0  << '\n' << 0 << '\n';						//  Write reserved zeros

//
//  Here we compute the number of possible values of each parameter (we will use this
//  several times later).  This is simply the number of steps it takes to get from start
//  to end in steps of step.  It is 1+floor(abs((end-start)/step)).
//

	long *param_steps = (long *)
		malloc(prefs.num_parameters * sizeof(long));	//  Allocate space for the array

	long i;
	for (i = 0; i < prefs.num_parameters; i++)
		{
		Parameter parameter = prefs.parameters[i];		//  Get this parameter
		
		param_steps[i] =
			(1 + floor(fabs(
			(parameter.end - parameter.start)/
									parameter.step)));	//  Find how many steps this parameter will take
		}

//
//  The mesh contains a point for each possible combination of values of
//  the parameters.  So the number of points in the mesh is the product
//  of all param_steps[i].
//
	
	long num_vertices = 1;
	for (i = 0; i < prefs.num_parameters; i++)
		num_vertices *= param_steps[i];					//  Multiply this parameter into product

	ofs << num_vertices << '\n';						//  Write number of vertices

	param_values = (double *)
		malloc(prefs.num_parameters * sizeof(double));	//  Allocate space for array of parameter values
	
	for (i = 0; i < prefs.num_parameters; i++)
		param_values[i] = prefs.parameters[i].start;	//  Initialize all parameters to start values
	
//
//  The following code computes the vertices.  It runs through all possible combinations
//  of the parameters in order.  The operation of this code can be likened to a multi-digit
//  counter; the first digit is incremented repeatedly until it passes its end value, at which
//  point it is set to its start value and a "carry" is generated which causes the next digit
//  to be incremented, and possible to generate a carry into the next digit.
//

	Boolean carry;
	i = 0;
	do
		{												//  i indicates which parameter we are
														//  currently incrementing.  The loop
														//  terminates when i is greater than
														//  the total number of params (there is
														//  a "carry" out of the top parameter).
		
		ofs << '(';										//  Write the left parenthesis for this vertex

		long j;
		for (j = 0; j < prefs.equation_dimension; j++)
			{
			double function_value;
			short error = EvaluateEquation(
							prefs.functions[j],
							&function_value,
							GetParamValue);				//  Evaluate the function for this coordinate

			if (error)
				{
				
				char error_message[200];
				sprintf(error_message, "HyperCuber encountered the following error while evaluating function x%d: %s.",
										j+1, GetErrorMessage(error));
				
				GeneralErrorByString(error_message);	//  Put up the error message
				
				return TRUE;							//  Failed to graph equation
				
				}

			ofs << function_value;						//  Write the value.
			
			if (j == prefs.equation_dimension - 1)
				ofs << ")\n";							//  Write close parenthesis on last coordinate
			else
				ofs << ", ";							//  Write comma on all except last coordinate
			}

		
		carry = FALSE;
		do
			{
			param_values[i] += prefs.parameters[i].step;	//  Increment a parameter
			
			if (param_values[i] > prefs.parameters[i].end)	//  Check if we've passed end
				{
				param_values[i] = prefs.parameters[i].start;//  Passed end; start over again at start
				i++;										//  and increment the next parameter
				if (i == prefs.num_parameters) break;		//  Carry out of last parameter; exit loop 
				carry = TRUE;								//  We have a "carry" into the next param
				}				
		
			else
				{
				i = 0;										//  No carry; start again with param 0
				carry = FALSE;
				}
				
			}
		while (carry);
		
		}
	while (i != prefs.num_parameters);
	
	ofs << 1 << '\n';									//  Write the number of colors (one)
	ofs << "65535, 65535, 65535\n";						//  Write the color (white)

//
//  Now we need to know the number of paths we will use.  Each parameter has a number of paths
//  associated with it.  The exact number of paths depends on the other parameters (but not on
//  the parameter itself).  The number of paths associated with a given parameter can be
//  computed by dividing the total number of points by the number of points in a particular
//  path (the number of possible values of the parameter).  To find the total number of paths, 
//  we sum this values over all parameters:
//
//  Number of Paths = num_vertices/param_steps[0] + ... + num_vertices/param_steps[n]
//

	long num_paths = 0;
	for (i = 0; i < prefs.num_parameters; i++)
		num_paths += num_vertices/param_steps[i];		//  Add this parameters's paths into the sum

	ofs << num_paths << '\n';							//  Write the number of paths


//
//  It is useful to imagine the list of all the vertices (as we wrote them to disk) as an n-dimensional
//  matrix.  The most intuitive way to think of a vertex is in terms of its parameters-- for each
//  possible combination of parameters there is a single vertex, and vice versa.  The values of the
//  parameters for a particular vertex are what I call the indices of the vertex.  The index of a
//  vertex for a parameter is the number of steps we took to get to that vertex.  Viewing the vertices
//  as an n-dimensional matrix, the index corresponds to the index into the matrix.
//
//  However, HyperCuber will not accept ordered n-tuples as vertex specifiers, so we need to have a way
//  to convert from the indexed specifier of a vertex to the absolute vertex number.  This is done the
//  same as with any matrix-- each dimension takes up a certain amount of space, and we need to
//  multiply the index of each parameter by the amount of space that parameter's dimension takes up in
//  the matrix.  For instance, if we have a 10x5x4 matrix, then the first parameter's dimension would
//  take up 1 space, since it is just a single number.  The second parameter's dimension would take up
//  10 spaces, since each step corresponds to stepping through ten steps of the first parameter.  The
//  third parameter's dimension would take up 50, since it corrsponds to stepping through five steps of
//  the second parameter, and each of the second parameter's steps is ten steps of the first parameter.
//
//  The next code segment finds the size of a parameter's dimension.  Each size is simply the product
//  of all lower param_steps, so this can be easily accomplished.
//


	long *dim_size = (long *)
		malloc(prefs.num_parameters * sizeof(long));	//  Allocate space for the array

	long size = 1;
	for (i = 0; i < prefs.num_parameters; i++)
		{
		dim_size[i] = size;								//  Save this size
		size *= param_steps[i];							//  Multiply by another param_step
		}

//
//  The following code actually writes the paths to disk.  It is similar to the "counter" code above,
//  but it executes multiple times, once for each parameter.  For each parameters, the code creates
//  a path for each possible combination of the OTHER parameters.  The parameter itself it not changed,
//  so it is "taken out of the loop," and carrys pass right over it.
//


	Parameter parameter;
	long param_num;
	for (param_num = 0; param_num < prefs.num_parameters; param_num++)
		{

		for (i = 0; i < prefs.num_parameters; i++)
			param_values[i] = prefs.parameters[i].start;		//  Initialize all parameters to start values

		parameter = prefs.parameters[param_num];				//  Get this parameter

		i = 0;
		do
			{													//  i indicates which parameter we are
																//  currently incrementing.  The loop
																//  terminates when i is greater than
																//  the total number of params (there is
																//  a "carry" out of the top parameter).
	
			ofs << '\n';
			ofs << 2 << '\n';									//  Make this a path
			ofs << 1 << '\n';									//  Draw with color 1
			ofs << param_steps[param_num] << '\n';				//  Write the length of this path

			//
			//  This loop writes out all the points in this path.  Here we are treating the
			//  vertices as an n1 x n2 x n3 x ... x nm array.  To find a particular vertex number
			//  we need to multiply the index of each parameter by the block size for that parameter.
			//
			
			double pvalue;
			for (param_values[param_num] = parameter.start;
					param_values[param_num] <= parameter.end;
					param_values[param_num] += parameter.step)	//  Loop through points of this path,
																//  writing each point as we go.
				{
				
				long vertex_num = 1;
				long n;
				for (n = 0; n < prefs.num_parameters; n++)		//  This loop finds the vertex number by
																//  finding the product of the indices
																//  of the current parameters.  The index
																//  of a parameter is the number of steps
																//  we have taken with that parameter.
					{
					
					double temp =
						(param_values[n] - prefs.parameters[n].start) /
								prefs.parameters[n].step;		//  Find index as signed double
					temp = (temp > 0) ? temp : -temp;			//  Get absolute value
					short index = floor(temp + 0.5);			//  Round to an integer
					
					vertex_num += dim_size[n]*index;			//  Add this parameter's contribution to
																//   the vertex number
					
					}
				
				ofs << vertex_num << '\n';						//  Write the vertex

				}
	
			carry = FALSE;
			do
				{

				if (i == param_num)
					{
					i++;										//  The paramth parameter is "taken out
																//  of the loop."
					if (i == prefs.num_parameters) break;		//  Carry out of last parameter; exit loop 
					}
					
				param_values[i] += prefs.parameters[i].step;	//  Increment a parameter
				
				if (param_values[i] > prefs.parameters[i].end)	//  Check if we've passed end
					{
					param_values[i] = prefs.parameters[i].start;//  Passed end; start over again at start
					i++;										//  and increment the next parameter
					if (i == prefs.num_parameters) break;		//  Carry out of last parameter; exit loop 
					carry = TRUE;								//  We have a "carry" into the next param
					}				
			
				else
					{
					i = 0;										//  No more carry; start again with param 0
					carry = FALSE;
					}
			
				}
			while (carry);
			
			}
		while (i != prefs.num_parameters);

		}	//  for param

	free(param_steps);									//  Free memory
	free(param_values);
	free(param_steps);

	ofs.close();										//  Close the file
	
	return (FALSE);										//  Successfully graphed equation
			
}	//==== CreateGraph() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure GetParamValue
//|
//| Purpose: this procedure finds the value of a parameter, given its name.
//|
//| Parameters: name: name of a parameter
//|             value: receives the value of the parameter
//|             returns zero if name is valid, non-zero if name is not a
//|                     valid parameter name.
//|____________________________________________________________________________

Boolean GetParamValue(char *name, double *value)
{

	long i;
	for (i = 0; i < gPrefs->prefs.num_parameters; i++)			//  Loop through all parameters
		{
		if (!strcmp(gPrefs->prefs.parameters[i].name, name))
			{
			*value = param_values[i];							//  Found the name; get the value
			return 0;											//  Name found
			}
		}
	
	return -1;													//  Name not found.

}	//==== GetParamValue() =====\\
