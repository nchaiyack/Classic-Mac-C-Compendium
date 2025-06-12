/*

		DynamoArrayTest.c++
		
		Test file using the DynamoArray class...
		Shows the usage of the class, though minimally, using the standard C++
		input/output. You can use the class in any app, actually, not just
		standard C++ i/o applications (read: use in your Mac apps too).

		This is public domain.

		by: Hiep Dam, 3G Software
		Bug reports, improvements, witty remarks, are all welcomed and
		encouraged.
		Contacting the author: AOL:      starlabs
							   Delphi:   starlabs
							   Internet: starlabs@aol.com, starlabs@delphi.com

		Last update: 5/4/94
			
*/


#include <iostream.h>
#include "DynamoArray.h"	// Currently the array holds *longs*. See the header file...


void main() {
	DynamoArray theList(20);	// Create a dynamoarray with 20 spaces pre-allocated
	DynamoArray aCopy;
	long j;

	cout << "Test" << endl << endl;
	
	// Add 10 numbers, from 0 to 9, to the array
	for (short i = 0; i < 10; i++)
		theList.Append(i);
	
	// Add another 10 numbers, from -1 to -9
	for (i = 0; i < 10; i++)
		theList.Append(-(i+1));
	
	// Make a copy, well, actually assignment...
	aCopy = theList;

	// Remove elements of array, in order
	cout << "Removal in order:" << endl;
	while (theList.SizeOf() != 0) {
		theList.Remove(j, 0);
		cout << j << ", ";
	}
	cout << endl << endl;
	
	// Remove elements of array, randomly
	cout << "Removal randomly:" << endl;
	while (aCopy.SizeOf() != 0) {
		aCopy.RandomRemove(j);
		cout << j << ", ";
	}
	cout << endl;
}