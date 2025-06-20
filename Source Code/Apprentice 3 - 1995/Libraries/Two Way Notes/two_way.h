/********************************************************/
/*                                                      */
/*                 Two_Way.h                            */
/*                 Header File for: Two_Way_List        */
/*                 Copyright 4/24/94 Hasan Edain        */
/*                 All Reights Reserved Worldwide       */
/*                                                      */
/********************************************************/

//	Please feel free to modify and use this code for any purpose, as long as you
// send me
// a) source of the modified code.
// b) licenced version of product the code is used in.
// c) notes as to how the code could be made more usefull if you tried to use it, but
// ended up not using it for any reason.
// Hasan Edain
// HasanEdain@AOL.com
// box 667 Langley Wa 98260
// fax: 206-579-6456

class node{
		node 			*next;
		node 			*prev;
		void 			*data;
		unsigned long	dataLength;
		short			dataKey;
	public:
		void			fillNode(node *prev,node *next, void *data, 
								unsigned long dataLength, short dataKey);
		void			*getData(void);
		short			getDataKey(void);
		unsigned long 	getDataLength(void);
		node			*getNextNode(void);
		node			*getPrevNode(void);
		void			setNextNode(node *theNode);
		void			setPrevNode(node *theNode);
		void			deleteNode(void);
};

class list{
		node 	*first;
		node 	*last;
		node 	*curr;
		short	nodeCount;
	public:
		list	*initList(void *data, unsigned long dataLength, short dataKey);
		void 	deleteList(void);
		
		short	addFirstNode(void *data, unsigned long dataLength, short dataKey);
		short	addNodeBeforeCurrent(void *data, unsigned long dataLength, short dataKey);
		short	addNodeAfterCurrent(void *data, unsigned long dataLength, short dataKey);
		short	addLastNode(void *data, unsigned long dataLength, short dataKey);
		
		void	deleteFirstNode(void);
		
		void	*extractFirstNode(short *dataKey, unsigned long *dataLength);
		void	*extractCurrNode(short *dataKey, unsigned long *dataLength);
		void	*extractNthNode(short *dataKey, unsigned long *dataLength, short n);
		void	*extractLastNode(short *dataKey, unsigned long *dataLength);
		
		void	moveCurrNodeForward(void);
		void	moveCurrNodeToStart(void);
		void	moveCurrNodeToNth(short n);
		void	moveCurrNodeToEnd(void);
		void	moveCurrNodeBackward(void);
		
		short	getListLength(void);
		list	*insSortList(void);
};