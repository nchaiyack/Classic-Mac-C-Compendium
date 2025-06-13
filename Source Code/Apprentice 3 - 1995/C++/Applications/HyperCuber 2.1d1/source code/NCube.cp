//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains a procedure which creates an n-dimensional cube
//|____________________________________________________________________

#include <fstream.h>


//================================ Prototypes ===============================\\

long FindPath(Boolean *has_visited, long start_vertex, long n, ofstream *ofs, Boolean write);



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure CreateNCube
//|
//| Purpose: this procedure creates an n-cube file for any n
//|
//| Parameters: n:        the dimension of the n-cube
//|             filename: name of file for n-cube descriptions
//|_______________________________________________________________

void CreateNCube(long n, char *filename)
{

	ofstream ofs;

	ofs.open(filename);									//  Open the output file
	
	ofs << 1 << '\n';									//  Write version number
	ofs << n << '\n';									//  Write dimension of n-cube
	ofs << 0  << '\n' << 0 << '\n';						//  Write reserved zeros
	
	long num_vertices = 1L << n;						//  Computer number of vertices
	
	ofs << num_vertices << '\n';						//  Write number of vertices
	
	long i;
	for (i = 0; i < num_vertices; i++)					//  Write the vertices' coordinates
		{
		ofs << '(';										//  Write the left parenthesis
		
		long j;
		for (j = 0; j < n; j++)							//  This loop slices up i (the current vertex number)
														//    bit by bit.  For each 1-bit in i, a 1 is
														//    written to the file; for each 0-bit, a
														//    -1 coordinate is written.
			{										
			if (i & (1 << j))
				ofs << 1;								//  This bit is a 1; write a 1
			else
				ofs << -1;								//  This bit is a 0; write a 0
			
			if (j != n-1)
				ofs << ", ";							//  Write comma between coordinates (but not
														//    after the last coordinate
			}
		
		ofs << ')' << '\n';								//  Write the right parenthesis
		}
	
	ofs << 1 << '\n';									//  Write the number of colors
	ofs << "65535, 65535, 65535\n";						//  Write the color

	long size_of_array = num_vertices*n*sizeof(Boolean);
	Boolean **has_visited_handle =
			(Boolean **) NewHandle(size_of_array);		//  Allocate space for the array
	Boolean **has_visited_handle_temp =
			(Boolean **) NewHandle(size_of_array);		//  Allocate space for the temporary array
	
	HLock((Handle) has_visited_handle);					//  Lock down and dereference the arrays
	HLock((Handle) has_visited_handle_temp);	
	Boolean *has_visited = *has_visited_handle;
	Boolean *has_visited_temp = *has_visited_handle_temp;

	for (i = 0; i < size_of_array; i++)					//  Clear the arrays
		has_visited[i] = FALSE;
	BlockMove(has_visited, has_visited_temp, size_of_array);

	//  PASS 1: in this pass we count the number of paths we need to draw this object

	long num_paths_found = 0;
	for (i = 0; i < num_vertices; i++)					//  This loops through the vertices in order
		while (FindPath(has_visited, i, n,
											&ofs, FALSE))
			num_paths_found++;							//  Keep making paths until there are no
														//    more to be found.  

	ofs << num_paths_found << '\n';						//  Write the number of paths
	
	//  PASS 2: in this pass we actually write the paths to disk
	
	for (i = 0; i < size_of_array; i++)					//  Clear the array
		has_visited[i] = FALSE;

	for (i = 0; i < num_vertices; i++)					//  This loops through the vertices in order
		{
		long path_length;
		do
			{
			BlockMove(has_visited, has_visited_temp,
										size_of_array);	//  Make a copy of the array
			path_length =
					FindPath(has_visited_temp,
									i, n, &ofs, FALSE);	//  Find the length of the path (using copy)
			
			if (path_length)
				{
				ofs << '\n';
				ofs << 2 << '\n';						//  Make this a path
				ofs << 1 << '\n';						//  Draw with color 1
				ofs << path_length << '\n';				//  Write the length of this path
				
				FindPath(has_visited, i, n, &ofs, TRUE);//  Find the path again, this time writing it
														//    to the file.
				}
			}
		while (path_length > 0);					//  Keep finding paths until there are none left

		}

	ofs.close();										//  Close the file
			
}	//==== CreateNCube() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| Procedure FindPath
//|
//| Purpose: this procedure finds a path along the edges of the
//|          n-cube, and writes the path to file.
//|
//| Parameters: has_visited:  the array of Booleans, indicating which vertices are already
//|                           connected together.
//|             start_vertex: the vertex to find a path from
//|             n:            the dimension of the n-cube
//|             write:        TRUE if the path should be written to file
//|             ofs:          the output file stream to write to
//|             returns length of path if a path was found, 0 otherwise
//|_______________________________________________________________________________________

long FindPath(Boolean *has_visited, long start_vertex, long n, ofstream *ofs, Boolean write)
{

	long current_vertex = start_vertex;				//  Start path with start_vertex

	long num_vertices = 1 << n;						//  Find the total number of vertices
	
	long path_length = 0;							//  Start with no vertices in path
	
	do
		{
		
		long i;
		for (i = 0; i < n; i++)
			{
			long next_vertex = current_vertex ^ (1 << i);//  Find adjacent vertex number by flipping
														//    one bit of the start vertex
	
			long to_index = current_vertex * n + i;		//  Find index of segment to start vertex
			long from_index = next_vertex * n + i;		//  Find index of segment from start vertex
			
			if (!has_visited[to_index])					//  Check if there is already a segment
														//    between current and next_vertex
				{
				
				has_visited[to_index] = TRUE;			//  Mark that there is now a connection
				has_visited[from_index] = TRUE;			//    both ways
				
				if (write)
					(*ofs) << current_vertex + 1 << '\n';//  Write the vertex
				
				path_length++;							//  Path is now one more vertex in length
				
				current_vertex = next_vertex;			//  Start looking for the next vertex
	
				break;									//  Get out of the for loop
							
				}	// end if
				
			}	// end for
		
		if (i == n)
			{
			if (path_length)
				{
				if (write)
					(*ofs) << current_vertex + 1 << '\n';//  Write the last vertex of a path
				path_length++;							//  Count the last vertex in the length
				}
			
			break;										//  We didn't find another node; path done
			}
		
		}
	while (TRUE);

	return path_length;									//  Return the path length

}	//==== FindPath() ====\\




























