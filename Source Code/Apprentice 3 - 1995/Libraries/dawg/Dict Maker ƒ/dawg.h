//
//	Description of the dawg data structure
//
//	The dawg is stored as an array of edges, each edge stored in an
//	unsigned longword. Each node is represented by the index into
//	this array of the first edge leaving that node; subsequent edges
//	leaving the same node occupy successive locations in the array.
//	The last edge leaving a node is flagged by a bit. Edges leading
//	to terminal nodes (those which are completed words) are marked
//	with another bit.  The edges are labelled with character numbers
//	from 1:26 (1=a) and occupy one 32 bit word each.  The node with
//	index 0 is the special node with no edges leaving it.
//
//	Bits in format of edges in unsigned 32-bit word:
//
//	| . . . : . . . | . . . : . . . | . . . : . . . | . . . : . . . |
//	 <-------------------------------------> ! ! !   <------------->
//		Index of edge pointed to in array	 F L W		label
//
//	F: this is the FIRST edge out of this node
//	L: this is the LAST  edge out of this node
//	W: this edge points to a node that completes a WORD
//
enum
{
	FIRST = 0x0800,
	LAST  = 0x0400,
	WORD  = 0x0200
};
//
// INDEX is the number of bits the unsigned long has to be shifted to
// obtain the index of the next node 'down' the stream.
//
enum
{
	INDEX = 12
};
