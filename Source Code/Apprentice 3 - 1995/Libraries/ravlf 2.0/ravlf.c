/*******************************************************************************
 * ravlf.c
 *
 * Random-access, variable-length files
 * C version 2.0
 * See header file for more info.
 * 
 *******************************************************************************/

#include "ravlf.h"
#include <Packages.h>

#define accessPerm	fsRdWrPerm		// open files for read/write access
#define bBlockSize	2048L			// b-tree block size, currently must be a multiple of dBlockSize
#define dBlockSize	2048L			// data block size
#define iBlockSize	2048L			// index block size, currently must be a multiple of dBlockSize
#define bBlockId	1				// block ids written as first 2 bytes of new block
#define dBlockId	2
#define iBlockId	3
#define Maxkeys		12				// b-tree keys per record, must be even; this value may affect performance
#define Minkeys		Maxkeys / 2
#define Maxchild	Maxkeys + 1
#define	NIL			-1L
#define nokey		-1L
#define nofile		-1

#define CacheSize	5				// for index records


#define abs(a) (((long)(a) < 0L) ? (-(a)) : (a))
#define max(a,b) (((a) > (b)) ? (a) : (b))


typedef short block_idType;			// type written at start of block for block id
typedef long bnode_iType;			// type of field used to index a record's children
typedef long data_iType;			// type of field used to store a record's data, index to data


typedef struct {
	short			keyCount;
	Key_Type		key[Maxkeys];
	data_iType		data[Maxkeys];
	bnode_iType		child[Maxchild];
} bRecType;							// data index record


typedef struct {
	short			pLength;		// physical length
	short			aLength;		// actual length
	data_iType		next;
	union {
		data_iType		back;		// used by free record
		TimeStamp_Type	timestamp;	// used by record in use
	} parm;
	// actual following data is variable length
} dRecType;


typedef struct {
	long			next;
	long			previous;
	ID_Type			id;
	Index_Type		index;
} iRecType;


typedef struct {
	short			fileNum;
	ID_Type			id;
	long			elem;
} iCacheType;


typedef struct {
	long			bRoot;
	bnode_iType		free_bList;
	data_iType		free_dList;
	long			iRoot;
	long			free_iList;
	short			version;
	short			maxkeys;
	long			iCount;
	long			checksum;
} HeaderType;


static IOParam paramBlock;			// for low level async. calls
static short fileNum;				// current file reference number
static HeaderType header;
static OSErr err;
static iCacheType cache[CacheSize];
#ifdef B_TREE_DEBUG
static SFReply sfReply;
#endif


void InitCache(void);
void WriteCache(ID_Type id, long elem);
long ReadCache(ID_Type id);
void Flush_File(void);
void Read_Header(void);
void Write_Header(void);
void Write_BlockId(block_idType id);
void Init_bRec(bRecType *bRec);
void Read_bRec(bnode_iType index, bRecType *bRec);
void Write_bRec(bnode_iType index, bRecType *bRec);
bnode_iType Grow_bList(void);
bnode_iType New_bRec(bRecType *bRec);
void Dispose_bRec(bnode_iType index);
data_iType Search1_BTree(bnode_iType root1, Key_Type *key);
data_iType Search_BTree(Key_Type key);
Key_Type Max1_Key(bnode_iType root1);
Key_Type Unique1_Key(bnode_iType root1, Key_Type *prevKey);
void Insert_Key(Key_Type *oKey, data_iType *oData, bnode_iType *oChild, Key_Type key,
				data_iType data, bnode_iType child, Boolean atRight, short keypos, short keyCount);
void Split_bRec(bRecType *bRec, short pos, Key_Type *promo_key, data_iType *promo_data,
				bnode_iType *promo_rchild);
Boolean Insert1_BTree(bnode_iType root1, Key_Type key, data_iType data, Key_Type *promo_key,
						data_iType *promo_data, bnode_iType *promo_rchild);
void Insert_BTree(Key_Type key, data_iType data);
void Delete_Key(Key_Type *oKey, data_iType *oData, bnode_iType *oChild, short keypos, short keyCount);
short Find_Predessor(bnode_iType root1, Key_Type *key, data_iType *data);
short Find_Successor(bnode_iType root1, Key_Type *key, data_iType *data);
Key_Type Find_Swap_Key(bRecType *bRec, short *pos);
Boolean Redistribute(bRecType *bRec, short pos);
Boolean Concatenate(bRecType *bRec, short pos);
Boolean Delete1_BTree(bnode_iType root1, Key_Type key, data_iType *data);
data_iType Delete_BTree(Key_Type key);
#ifdef B_TREE_DEBUG
short Tree_Depth(bnode_iType root1, short depth);
void Dump1_Tree(bnode_iType root1, short fRefNum, char *buffer, short cur_depth, short max_depth);
#endif
void Read_dRec(data_iType dIndex, dRecType *dRec);
void Write_dRec(data_iType dIndex, dRecType *dRec);
void Concat_Free_dRec(data_iType dIndex, dRecType *dRec);
void Free_dRec(data_iType dIndex, dRecType *dRec);
data_iType Grow_dList(void);
data_iType New_dRec(dRecType *dRec);
void Dispose1_Data(data_iType dIndex);
data_iType Save1_Data(Ptr data, short dataLen);
void Init_iRec(iRecType *iRec);
void Read_iRec(long elem, iRecType *iRec);
void Write_iRec(long elem, iRecType *iRec);
long Grow_iList(void);
long New_iRec(iRecType *iRec);
void Dispose_iRec(long elem);
long Find_Elem(ID_Type id);


/*******************************************************************************
 * InitCache
 *
 *******************************************************************************/
 
void InitCache(void)
{
	short		loop;
	
	for(loop = 0; loop < CacheSize; loop++)
	{
		cache[loop].fileNum = nofile;
		cache[loop].id = NoID;
		cache[loop].elem = NIL;
	}
} // InitCache



/*******************************************************************************
 * WriteCache
 *
 *******************************************************************************/

void WriteCache(ID_Type id, long elem)
{
	short		cacheElem;
	short		loop;
	
	loop = 0;
	while ((loop < CacheSize) && (cache[loop].fileNum != nofile))
		loop++;

	if (loop == CacheSize)
	{
		loop = Random();
		loop = (abs(loop) % CacheSize);
	}
	
	cache[loop].fileNum = fileNum;
	cache[loop].id = id;
	cache[loop].elem = elem;
} // WriteCache



/*******************************************************************************
 * ReadCache
 *
 *******************************************************************************/

long ReadCache(ID_Type id)
{
	short		loop;
	
	loop = 0;
	while (loop < CacheSize)
	{
		if ((fileNum == cache[loop].fileNum) && ((id == NoID) || (id == cache[loop].id)))
		{
			cache[loop].fileNum = nofile;
			cache[loop].id = NoID;
			cache[loop].elem = NIL;
			if (id != NoID)
				return (cache[loop].elem);
		}
		loop++;
	}
	return (NIL);
} // ReadCache



/*******************************************************************************
 * Init_Disk
 *
 *******************************************************************************/

pascal void Init_Disk(void)
{
	paramBlock.ioResult = 0;
	fileNum = nofile;

	header.bRoot = NIL;
	header.free_bList = NIL;
	header.free_dList = NIL;

	InitCache();
	header.iRoot = NIL;
	header.free_iList = NIL;

	#ifdef B_TREE_DEBUG
	sfReply.fName[0] = '\0';
	sfReply.vRefNum = 0;
	sfReply.good = 1;
	#endif
	
	err = noErr;
} // Init_Disk



/*******************************************************************************
 * Dispose_Disk
 *
 *******************************************************************************/

pascal void Dispose_Disk(void)
{
	// make sure async file flushing is done!
	while (paramBlock.ioResult == 1)
		;
} // Dispose_Disk



/*******************************************************************************
 * Disk_Error
 *
 *******************************************************************************/

pascal short Disk_Error(void)
{
	return err;
} // Disk_Error



/*******************************************************************************
 * Cur_File
 *
 *******************************************************************************/

pascal short Cur_File(void)
{
	return fileNum;
} // Cur_File



/*******************************************************************************
 * Flush_File
 *
 *******************************************************************************/

void Flush_File(void)
{
	// make sure its done flushing from last time!
	while (paramBlock.ioResult == 1)
		;

	paramBlock.ioCompletion = NULL;
	paramBlock.ioRefNum = fileNum;
	err = PBFlushFile((ParmBlkPtr)&paramBlock, TRUE);
} // Flush_File



/*******************************************************************************
 * Flush_File
 *
 *******************************************************************************/

pascal void Flush_Vol(short volNum, Boolean async)
{
	// make sure its done flushing from last time!
	while (paramBlock.ioResult == 1)
		;

	paramBlock.ioCompletion = NULL;
	paramBlock.ioNamePtr = NULL;
	paramBlock.ioVRefNum = volNum;
	err = PBFlushVol((ParmBlkPtr)&paramBlock, async);
} // Flush_Vol



/*******************************************************************************
 * Read_Header
 *
 *******************************************************************************/

void Read_Header(void)
{
	long		size;

	if (fileNum != nofile)
	{
		err = SetFPos(fileNum, fsFromStart, 0L);
		size = sizeof(HeaderType);
		err = FSRead(fileNum, &size, (Ptr)&header);
	}
} // Read_Header



/*******************************************************************************
 * Write_Header
 *
 *******************************************************************************/

void Write_Header(void)	// this should be called following any data or index changes
{
	long		size;
	
	if (fileNum != nofile)
	{
		err = SetFPos(fileNum, fsFromStart, 0L);
		size = sizeof(HeaderType);
		err = FSWrite(fileNum, &size, (Ptr)&header);
		Flush_File();
	}
} // Write_Header



/*******************************************************************************
 * Write_BlockId
 *
 *******************************************************************************/

void Write_BlockId(block_idType id)	// call before allocating a new index or data block
{
	long		size;

	err = SetFPos(fileNum, fsFromLEOF, 0L);
	size = sizeof(block_idType);
	err = FSWrite(fileNum, &size, (Ptr)&id);
} // Write_BlockId



/*******************************************************************************
 * Init_bRec
 *
 *******************************************************************************/

void Init_bRec(bRecType *bRec)
{
	short		loop;
	
	bRec->keyCount = 0;
	for(loop = 0; loop < Maxkeys; loop++)
	{
		bRec->key[loop] = nokey;
		bRec->data[loop] = NIL;
		bRec->child[loop] = NIL;
	}
	bRec->child[Maxchild - 1] = NIL;
} // Init_bRec



/*******************************************************************************
 * Read_bRec
 *
 *******************************************************************************/

void Read_bRec(bnode_iType index, bRecType *bRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, index);
	size = sizeof(bRecType);
	err = FSRead(fileNum, &size, (Ptr)bRec);
} // Read_bRec



/*******************************************************************************
 * Write_bRec
 *
 *******************************************************************************/

void Write_bRec(bnode_iType index, bRecType *bRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, index);
	size = sizeof(bRecType);
	err = FSWrite(fileNum, &size, (Ptr)bRec);
} // Write_bRec



/*******************************************************************************
 * Grow_bList
 *
 *******************************************************************************/

bnode_iType Grow_bList(void)
{
	long		startEOF;
	long		curEOF;
	bRecType	bRec;
	long		size;
	
	err = GetEOF(fileNum, &startEOF);
	Write_BlockId(bBlockId);
	curEOF = startEOF + sizeof(block_idType);
	size = sizeof(bRecType);
	
	while (((curEOF + size - 1L) / bBlockSize) == (startEOF / bBlockSize))
	{
		Init_bRec(&bRec);
		if (((curEOF + size * 2L - 1L) / bBlockSize) == (startEOF / bBlockSize))
			bRec.child[0] = curEOF + size;				// link free list
		Write_bRec(curEOF, &bRec);
		curEOF += size;
	}
	
	size = bBlockSize - (curEOF % bBlockSize);
	if (size < bBlockSize)
		err = SetEOF(fileNum, curEOF + size);
	if (err == noErr)
		return (startEOF + sizeof(block_idType));
	else
		return (NIL);
} // Grow_bList



/*******************************************************************************
 * New_bRec
 *
 *******************************************************************************/

bnode_iType New_bRec(bRecType *bRec)
{
	bnode_iType		new_bRec;
	
	if (header.free_bList == NIL)
		header.free_bList = Grow_bList();
	new_bRec = header.free_bList;
	Read_bRec(header.free_bList, bRec);
	header.free_bList = bRec->child[0];
	Init_bRec(bRec);
	// Write_Header(); called by calling routines
	return new_bRec;
} // New_bRec



/*******************************************************************************
 * Dispose_bRec
 *
 *******************************************************************************/

void Dispose_bRec(bnode_iType index)
{
	bRecType		bRec;
	
	Init_bRec(&bRec);
	bRec.child[0] = header.free_bList;
	header.free_bList = index;
	Write_bRec(index, &bRec);
} // Dispose_bRec



/*******************************************************************************
 * Search1_BTree
 *
 *******************************************************************************/

data_iType Search1_BTree(bnode_iType root1, Key_Type *key)
{
	bRecType		bRec;
	short			loop;
	data_iType		result;
	
	if (root1 == NIL)
	{
		result = NIL;
		*key = nokey;
	}
	else
	{
		Read_bRec(root1, &bRec);
		loop = 0;
		while ((loop < bRec.keyCount) && (*key > bRec.key[loop]))
			loop++;
		if ((loop < bRec.keyCount) && (*key == bRec.key[loop]))
			result = bRec.data[loop];
		else
		{
			result = Search1_BTree(bRec.child[loop], key);
			if ((*key == nokey) && (loop < bRec.keyCount))
				*key = bRec.key[loop];
		}
	}
	return result;
} // Search1_BTree



/*******************************************************************************
 * Search_BTree
 *
 *******************************************************************************/

data_iType Search_BTree(Key_Type key)
{
	return Search1_BTree(header.bRoot, &key);
} // Search_BTree



/*******************************************************************************
 * Valid_Key
 *
 *******************************************************************************/

pascal Boolean Valid_Key(Key_Type *key)
{
	return (Search1_BTree(header.bRoot, key) != NIL);
} // Valid_Key



/*******************************************************************************
 * Max1_Key
 *
 *******************************************************************************/
 
Key_Type Max1_Key(bnode_iType root1)
{
	bRecType		bRec;
	
	Read_bRec(root1, &bRec);
	if (bRec.child[bRec.keyCount] == NIL)
		return (bRec.key[bRec.keyCount - 1]);
	else
		return (Max1_Key(bRec.child[bRec.keyCount]));
} // Max1_Key



/*******************************************************************************
 * Max_Key
 *
 *******************************************************************************/

pascal Key_Type Max_Key(void)
{
	if (header.bRoot == NIL)
		return (nokey);
	else
		return (Max1_Key(header.bRoot));
} // Max_Key



/*******************************************************************************
 * Unique1_Key
 *
 *******************************************************************************/

Key_Type Unique1_Key(bnode_iType root1, Key_Type *prevKey)
{
	bRecType		bRec;
	short			loop;
	Key_Type		result;
	
	if (root1 == NIL)
		return (nokey);
	else
	{
		Read_bRec(root1, &bRec);
		loop = 0;
		while (loop < bRec.keyCount)
		{
			result = Unique1_Key(bRec.child[loop], prevKey);
			if (result != nokey)
				return (result);
			if (*prevKey == nokey)
				*prevKey = bRec.key[loop];
			if (bRec.key[loop] - *prevKey > 1L)
			{
				result = *prevKey + 1L;
				return (result);
			}
			*prevKey = bRec.key[loop];
			loop++;
		}
		return (Unique1_Key(bRec.child[loop], prevKey));
	}
} // Unique1_Key



/*******************************************************************************
 * Unique_Key
 *
 *******************************************************************************/

pascal Key_Type Unique_Key(void)
{
	Key_Type		prevKey;
	Key_Type		result;
	
	if (header.bRoot == NIL)
		return (nokey);
	else
	{
		prevKey = nokey;
		result = Unique1_Key(header.bRoot, &prevKey);
		if (result == nokey)
			result = prevKey + 1L;
		return (result);
	}
} // Unique_Key


#ifdef B_TREE_DEBUG

#include <string.h>
#include <stdio.h>

/*******************************************************************************
 * Tree_Depth
 *
 *******************************************************************************/

short Tree_Depth(bnode_iType root1, short depth)
{
	bRecType		bRec;
	
	if (root1 == NIL)
		return depth;
	Read_bRec(root1, &bRec);
	if (bRec.child[bRec.keyCount] == NIL)
		return depth;
	else
		return (Tree_Depth(bRec.child[bRec.keyCount], depth + 1));
} // Tree_Depth


/*******************************************************************************
 * Dump1_Tree
 *
 *******************************************************************************/

void Dump1_Tree(bnode_iType root1, short fRefNum, char *buffer, short cur_depth, short max_depth)
{
	bRecType		bRec;
	short			loop, loop2;
	long			inOutCount;
	short			tab = 22;
	
	if (root1 == NIL)
		return;
	else
	{
		Read_bRec(root1, &bRec);
		
		for(loop2 = 0; loop2 < (max_depth - cur_depth) * tab; loop2++)
			buffer[loop2] = ' ';
		inOutCount = (max_depth - cur_depth) * tab;
		FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
		sprintf(buffer, "* 0x%lX *\r", root1);
		inOutCount = strlen(buffer);
		FSWrite(fRefNum, &inOutCount, (Ptr)buffer);

		loop = 0;
		while (loop < bRec.keyCount)
		{
			Dump1_Tree(bRec.child[loop], fRefNum, buffer, cur_depth + 1, max_depth);
			
			for(loop2 = 0; loop2 < (max_depth - cur_depth) * tab; loop2++)
				buffer[loop2] = ' ';
			inOutCount = (max_depth - cur_depth) * tab;
			FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
			sprintf(buffer, "[%5ld : 0x%lX]\r", bRec.key[loop], bRec.child[loop]);
			inOutCount = strlen(buffer);
			FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
			
			loop++;
		}
		Dump1_Tree(bRec.child[loop], fRefNum, buffer, cur_depth + 1, max_depth);

		for(loop2 = 0; loop2 < (max_depth - cur_depth) * tab; loop2++)
			buffer[loop2] = ' ';
		inOutCount = (max_depth - cur_depth) * tab;
		FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
		sprintf(buffer, "[      : 0x%lX]\r", bRec.child[loop]);
		inOutCount = strlen(buffer);
		FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
	}
} // Dump1_Tree


/*******************************************************************************
 * Dump_Tree
 *
 *******************************************************************************/

pascal void Dump_Tree(void)
{
	short			iErr;
	Point			where;
	SFTypeList		typeList;
	short			fRefNum;
	long			inOutCount;
	char			buffer[256];
	
	if (!(sfReply.good))
		return;

	iErr = FSOpen(sfReply.fName, sfReply.vRefNum, &fRefNum);
	if (iErr != 0)
	{
		/* 
		where.h = 0; where.v = 0;
		typeList[0] = 'TEXT';
		SFGetFile(where, "\p", NULL, 1, typeList, NULL, &sfReply);
		if (!(sfReply.good))
		{
		}
		*/
		SFPutFile(where, "\pSave B Tree Dump As:", "\pB-Tree Dump", NULL, &sfReply);
		if (!(sfReply.good))
			return;
			
		iErr = Create(sfReply.fName, sfReply.vRefNum, 'MSWD', 'TEXT');
		if (iErr == -48)
		{
			FSDelete(sfReply.fName, sfReply.vRefNum);
			iErr = Create(sfReply.fName, sfReply.vRefNum, 'MSWD', 'TEXT');
		}
		if (iErr != 0) { sfReply.good = 0; SysBeep(1); return; }
		iErr = FSOpen(sfReply.fName, sfReply.vRefNum, &fRefNum);
		if (iErr != 0) { sfReply.good = 0; SysBeep(1); return; }
	}
	SetFPos(fRefNum, fsFromLEOF, 0L);
	
	Dump1_Tree(header.bRoot, fRefNum, buffer, 0, Tree_Depth(header.bRoot, 0));
	
	strcpy(buffer, "\r-------------------------------------------------------------------------------\r\r");
	inOutCount = strlen(buffer);
	FSWrite(fRefNum, &inOutCount, (Ptr)buffer);
	
	iErr = FSClose(fRefNum);
} // Dump_Tree

#endif


/*******************************************************************************
 * Insert_Key
 *
 *******************************************************************************/

void Insert_Key(Key_Type *oKey, data_iType *oData, bnode_iType *oChild, Key_Type key,
				data_iType data, bnode_iType child, Boolean atRight, short keypos, short keyCount)
{
	while (keyCount > keypos)							// shift larger keys right
	{
		oKey[keyCount] = oKey[keyCount - 1];
		oData[keyCount] = oData[keyCount - 1];
		oChild[keyCount + 1] = oChild[keyCount];
		keyCount--;
	}
	oKey[keypos] = key;
	oData[keypos] = data;
	if (atRight)
		oChild[keypos + 1] = child;
	else
	{
		oChild[keypos + 1] = oChild[keypos];
		oChild[keypos] = child;
	}
} // Insert_Key



/*******************************************************************************
 * Split_bRec
 *
 *******************************************************************************/

void Split_bRec(bRecType *bRec, short pos, Key_Type *promo_key, data_iType *promo_data,
				bnode_iType *promo_rchild)
{
	short		loop;
	bRecType	bRec2;
	Key_Type	oKey[Maxkeys+1];
	data_iType	oData[Maxkeys+1];
	bnode_iType	oChild[Maxchild+1];
	
	for (loop = 0; loop < Maxkeys; loop++)			// copy record into oversized record
	{
		oKey[loop] = bRec->key[loop];
		oData[loop] = bRec->data[loop];
		oChild[loop] = bRec->child[loop];
	}
	oChild[Maxchild - 1] = bRec->child[Maxchild - 1];
	Insert_Key(oKey, oData, oChild, *promo_key, *promo_data, *promo_rchild, TRUE, pos, bRec->keyCount);	// insert into oversized record

	*promo_key = oKey[Minkeys];						// pass back middle key
	*promo_data = oData[Minkeys];					// pass back middle data
	*promo_rchild = New_bRec(&bRec2);				// get new right record
	Init_bRec(bRec);								// clear existing left record

	for (loop = 0; loop < Minkeys; loop++)			// copy oversized record into split records
	{
		bRec->key[loop] = oKey[loop];
		bRec->data[loop] = oData[loop];
		bRec->child[loop] = oChild[loop];
		bRec2.key[loop] = oKey[Minkeys + 1 + loop];
		bRec2.data[loop] = oData[Minkeys + 1 + loop];
		bRec2.child[loop] = oChild[Minkeys + 1 + loop];
	}
	bRec->child[Minkeys] = oChild[Minkeys];
	bRec2.child[Minkeys] = oChild[Maxkeys + 1];
	bRec->keyCount = Minkeys;
	bRec2.keyCount = Minkeys;
	
	Write_bRec(*promo_rchild, &bRec2);
} // Split_bRec



/*******************************************************************************
 * Insert1_BTree
 *
 *******************************************************************************/

Boolean Insert1_BTree(bnode_iType root1, Key_Type key, data_iType data, Key_Type *promo_key,
						data_iType *promo_data, bnode_iType *promo_rchild)
{
	bRecType		bRec;
	short			pos;					// position in bRec
	Boolean			promotion;
	Boolean			result;
	
	if (root1 == NIL)
	{										// past bottom of tree
		*promo_key = key;
		*promo_data = data;
		*promo_rchild = NIL;
		result = TRUE;
	}
	else									// search the record for key
	{
		Read_bRec(root1, &bRec);
		pos = 0;
		while ((pos < bRec.keyCount) && (key > bRec.key[pos]))
			pos++;							// find where key should be

		if ((pos < bRec.keyCount) && (key == bRec.key[pos]))
		{
			bRec.data[pos] = data;			// duplicate key, update data field only
			Write_bRec(root1, &bRec);
			result = FALSE;
		}
		else								// search child
		{
			promotion = Insert1_BTree(bRec.child[pos], key, data, promo_key, promo_data, promo_rchild);

			if (promotion)
			{
				if (bRec.keyCount < Maxkeys)
				{							// there is room in this record
					Insert_Key(bRec.key, bRec.data, bRec.child, *promo_key, *promo_data, *promo_rchild, TRUE, pos, bRec.keyCount);
					bRec.keyCount++;
					result = FALSE;
				}
				else						// split record
				{
					Split_bRec(&bRec, pos, promo_key, promo_data, promo_rchild);
					result = TRUE;
				}
				Write_bRec(root1, &bRec);
			}
			else							// promotion == false
				result = FALSE;
		}
	}
	return (result);
} // Insert1_BTree



/*******************************************************************************
 * Insert_BTree
 *
 *******************************************************************************/

void Insert_BTree(Key_Type key, data_iType data)
{
	bRecType		bRec;
	bnode_iType		tempRoot;
	Key_Type		promo_key;
	data_iType		promo_data;
	bnode_iType		promo_rchild;
	
	if (Insert1_BTree(header.bRoot, key, data, &promo_key, &promo_data, &promo_rchild))
	{
		tempRoot = New_bRec(&bRec);			// create a new header.bRoot
		bRec.keyCount = 1;
		bRec.key[0] = promo_key;
		bRec.data[0] = promo_data;
		bRec.child[0] = header.bRoot;
		bRec.child[1] = promo_rchild;
		header.bRoot = tempRoot;
		Write_bRec(header.bRoot, &bRec);
		// Write_Header(); called by Save_Data
	}
} // Insert_BTree



/*******************************************************************************
 * Delete_Key
 *
 *******************************************************************************/

void Delete_Key(Key_Type *oKey, data_iType *oData, bnode_iType *oChild, short keypos, short keyCount)
{
	short		loop;
	
	loop = keypos;
	while (loop < keyCount - 1)
	{
		oKey[loop] = oKey[loop + 1];
		oData[loop] = oData[loop + 1];
		oChild[loop] = oChild[loop + 1];
		loop++;
	}
	oKey[keyCount - 1] = nokey;
	if (keypos < keyCount - 1)				// shift rightmost child when left key deleted
		oChild[keyCount - 1] = oChild[keyCount];
	oChild[keyCount] = NIL;
} // Delete_Key



/*******************************************************************************
 * Find_Predessor
 *
 *******************************************************************************/

short Find_Predessor(bnode_iType root1, Key_Type *key, data_iType *data)
{
	bRecType		bRec;
	
	Read_bRec(root1, &bRec);
	if (bRec.child[0] == NIL)				// leaf record?
	{
		*key = bRec.key[bRec.keyCount - 1];
		*data = bRec.data[bRec.keyCount - 1];
		return (bRec.keyCount);
	}
	else
		return (Find_Predessor(bRec.child[bRec.keyCount], key, data));
} // Find_Predessor



/*******************************************************************************
 * Find_Successor
 *
 *******************************************************************************/

short Find_Successor(bnode_iType root1, Key_Type *key, data_iType *data)
{
	bRecType		bRec;
	
	Read_bRec(root1, &bRec);
	if (bRec.child[0] == NIL)				// leaf record?
	{
		*key = bRec.key[0];
		*data = bRec.data[0];
		return (bRec.keyCount);
	}
	else
		return (Find_Successor(bRec.child[0], key, data));
} // Find_Successor



/*******************************************************************************
 * Find_Swap_Key
 *
 *******************************************************************************/

Key_Type Find_Swap_Key(bRecType *bRec, short *pos)
{
	short		pred_keycount, succ_keycount;
	Key_Type	pred_key, succ_key;
	data_iType	pred_data, succ_data;
	
	pred_keycount = Find_Predessor(bRec->child[*pos], &pred_key, &pred_data);
	succ_keycount = Find_Successor(bRec->child[*pos + 1], &succ_key, &succ_data);
	
	if (pred_keycount > succ_keycount)
	{
		bRec->key[*pos] = pred_key;
		bRec->data[*pos] = pred_data;
		return (pred_key);
	}
	else
	{
		bRec->key[*pos] = succ_key;
		bRec->data[*pos] = succ_data;
		(*pos)++;
		return (succ_key);
	}
} // Find_Swap_Key



/*******************************************************************************
 * Redistribute
 *
 *******************************************************************************/

Boolean Redistribute(bRecType *bRec, short pos)
{
	bRecType		bRec2;
	Key_Type		borrowKey;
	data_iType		borrowData;
	bnode_iType		borrowChild;
	
	if (pos < bRec->keyCount)
	{
		Read_bRec(bRec->child[pos + 1], &bRec2);	// look in right subtree
		if (bRec2.keyCount > Minkeys)
		{
			borrowKey = bRec->key[pos];
			bRec->key[pos] = bRec2.key[0];
			borrowData = bRec->data[pos];
			bRec->data[pos] = bRec2.data[0];
			borrowChild = bRec2.child[0];
			Delete_Key(bRec2.key, bRec2.data, bRec2.child, 0, bRec2.keyCount);
			bRec2.keyCount--;
			Write_bRec(bRec->child[pos + 1], &bRec2);
			Read_bRec(bRec->child[pos], &bRec2);		// the invalid record
			Insert_Key(bRec2.key, bRec2.data, bRec2.child, borrowKey, borrowData, borrowChild, TRUE, bRec2.keyCount, bRec2.keyCount);
			bRec2.keyCount++;
			Write_bRec(bRec->child[pos], &bRec2);
			return (FALSE);
		}
	}
	if (pos > 0)
	{
		Read_bRec(bRec->child[pos - 1], &bRec2);		// look in left subtree
		if (bRec2.keyCount > Minkeys)
		{
			borrowKey = bRec->key[pos - 1];
			bRec->key[pos - 1] = bRec2.key[bRec2.keyCount - 1];
			borrowData = bRec->data[pos - 1];
			bRec->data[pos - 1] = bRec2.data[bRec2.keyCount - 1];
			borrowChild = bRec2.child[bRec2.keyCount];
			Delete_Key(bRec2.key, bRec2.data, bRec2.child, bRec2.keyCount - 1, bRec2.keyCount);
			bRec2.keyCount--;
			Write_bRec(bRec->child[pos - 1], &bRec2);
			Read_bRec(bRec->child[pos], &bRec2);		// the invalid record
			Insert_Key(bRec2.key, bRec2.data, bRec2.child, borrowKey, borrowData, borrowChild, FALSE, 0, bRec2.keyCount);
			bRec2.keyCount++;
			Write_bRec(bRec->child[pos], &bRec2);
			return (FALSE);
		}
	}
	return (TRUE);										// there is still underflow
} // Redistribute



/*******************************************************************************
 * Concatenate
 *
 *******************************************************************************/

Boolean Concatenate(bRecType *bRec, short pos)
{
	bRecType		bRec2, bRec3;
	short			loop;
	bnode_iType		concat_rec, eliminate_rec;
	
	Read_bRec(bRec->child[pos], &bRec2);				// the invalid record
	if (pos < bRec->keyCount)
	{													// try right subtree
		concat_rec = bRec->child[pos];
		eliminate_rec = bRec->child[pos + 1];
		Read_bRec(eliminate_rec, &bRec3);				// get key from successor
		Insert_Key(bRec2.key, bRec2.data, bRec2.child, bRec->key[pos], bRec->data[pos], bRec3.child[0], TRUE, bRec2.keyCount, bRec2.keyCount);
		bRec2.keyCount++;
		Delete_Key(bRec->key, bRec->data, bRec->child, pos, bRec->keyCount);
		bRec->keyCount--;
		bRec->child[pos] = concat_rec;
		for(loop = 0; loop < bRec3.keyCount; loop++)	// move keys from right subtree
		{
			Insert_Key(bRec2.key, bRec2.data, bRec2.child, bRec3.key[loop], bRec3.data[loop], bRec3.child[loop + 1], TRUE, bRec2.keyCount, bRec2.keyCount);
			bRec2.keyCount++;
		}
		Dispose_bRec(eliminate_rec);
		Write_bRec(concat_rec, &bRec2);
	}
	else												// left subtree
	{
		concat_rec = bRec->child[pos - 1];
		eliminate_rec = bRec->child[pos];
		Read_bRec(concat_rec, &bRec3);					// get key from successor
		Insert_Key(bRec3.key, bRec3.data, bRec3.child, bRec->key[pos - 1], bRec->data[pos - 1], bRec2.child[0], TRUE, bRec3.keyCount, bRec3.keyCount);
		bRec3.keyCount++;
		Delete_Key(bRec->key, bRec->data, bRec->child, pos - 1, bRec->keyCount);
		bRec->keyCount--;
		bRec->child[pos - 1] = concat_rec;
		for (loop = 0; loop < bRec2.keyCount; loop++)	// move keys from right subtree
		{
			Insert_Key(bRec3.key, bRec3.data, bRec3.child, bRec2.key[loop], bRec2.data[loop], bRec2.child[loop + 1], TRUE, bRec3.keyCount, bRec3.keyCount);
			bRec3.keyCount++;
		}
		Dispose_bRec(eliminate_rec);
		Write_bRec(concat_rec, &bRec3);
	}

	return (bRec->keyCount < Minkeys);					// underflow if true
		
} // Concatenate



/*******************************************************************************
 * Delete1_BTree
 *
 *******************************************************************************/

Boolean Delete1_BTree(bnode_iType root1, Key_Type key, data_iType *data)
{
	bRecType		bRec;
	short			pos;
	Key_Type		swap_key;
	Boolean			underflow;
	data_iType		ignore_data;
	Boolean			result;
	
	if (root1 == NIL)
	{													// key not found
		*data = NIL;
		result = FALSE;
	}
	else
	{
		pos = 0;
		Read_bRec(root1, &bRec);
		while ((pos < bRec.keyCount) && (key > bRec.key[pos]))
			pos++;
		if ((pos < bRec.keyCount) && (key == bRec.key[pos]))
		{
			*data = bRec.data[pos];						// pass back data of deleted key
			if (bRec.child[0] == NIL)
			{											// leaf record
				Delete_Key(bRec.key, bRec.data, bRec.child, pos, bRec.keyCount);
				bRec.keyCount--;
				Write_bRec(root1, &bRec);
				result = (bRec.keyCount < Minkeys);		// underflow if too few keys
				return (result);
			}
			else										// swap internal key with leaf key
			{
				swap_key = Find_Swap_Key(&bRec, &pos);
				Write_bRec(root1, &bRec);
				underflow = Delete1_BTree(bRec.child[pos], swap_key, &ignore_data);
			}
		}
		else											// keep searching
			underflow = Delete1_BTree(bRec.child[pos], key, data);

		if (underflow)
		{
			if (Redistribute(&bRec, pos) == FALSE)
			{											// redistribute worked, no underflow
				Write_bRec(root1, &bRec);
				result = FALSE;
			}
			else
			{
				underflow = Concatenate(&bRec, pos);
				Write_bRec(root1, &bRec);
				result = underflow;
			}
		}
		else
			result = FALSE;
	}
	return (result);
} // Delete1_BTree



/*******************************************************************************
 * Delete_BTree
 *
 *******************************************************************************/

data_iType Delete_BTree(Key_Type key)
{
	bRecType		bRec;
	data_iType		data;
	
	if (Delete1_BTree(header.bRoot, key, &data))
	{
		Read_bRec(header.bRoot, &bRec);
		if (bRec.keyCount == 0)
		{
			Dispose_bRec(header.bRoot);
			header.bRoot = bRec.child[0];
			// Write_Header(); called by Dispose_Data
		}
	}
	return (data);
} // Delete_BTree



/*******************************************************************************
 * Read_dRec
 *
 *******************************************************************************/

void Read_dRec(data_iType dIndex, dRecType *dRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, dIndex);
	size = sizeof(dRecType);
	err = FSRead(fileNum, &size, (Ptr)dRec);
} // Read_dRec



/*******************************************************************************
 * Write_dRec
 *
 *******************************************************************************/

void Write_dRec(data_iType dIndex, dRecType *dRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, dIndex);
	size = sizeof(dRecType);
	err = FSWrite(fileNum, &size, (Ptr)dRec);
} // Write_dRec



/*******************************************************************************
 * Concat_Free_dRec
 *
 *******************************************************************************/

void Concat_Free_dRec(data_iType dIndex, dRecType *dRec)	// dRec must already be made first in free list
{
	dRecType		dRec2, dRec3;
	long			size;
	
	size = sizeof(dRecType) + abs(dRec->pLength);
	if (((dIndex + size) / dBlockSize) == (dIndex / dBlockSize))	// next dRec must be in same block
	{
		Read_dRec(dIndex + size, &dRec2);
		if (dRec2.pLength < 0)
		{													// adjacent dRec is free, combine it
			if (dRec2.parm.back == dIndex)
				dRec->next = dRec2.next;					// adjacent dRec is also next in free list
			else if (dRec2.parm.back != NIL)				// this should never be NIL
			{												// update next free record
				Read_dRec(dRec2.parm.back, &dRec3);
				dRec3.next = dRec2.next;
				Write_dRec(dRec2.parm.back, &dRec3);
			}

			if (dRec2.next != NIL)
			{												// update previous free record
				Read_dRec(dRec2.next, &dRec3);
				dRec3.parm.back = dRec2.parm.back;
				Write_dRec(dRec2.next, &dRec3);
			}
			dRec->pLength = -1 * (size + abs(dRec2.pLength));
			Concat_Free_dRec(dIndex, dRec);
		}
	}
} // Concat_Free_dRec



/*******************************************************************************
 * Free_dRec
 *
 *******************************************************************************/
 
void Free_dRec(data_iType dIndex, dRecType *dRec)
{
 	dRecType		dRec2;
 	
	if (header.free_dList != NIL)
	{
		Read_dRec(header.free_dList, &dRec2);
		dRec2.parm.back = dIndex;
		Write_dRec(header.free_dList, &dRec2);
	}
	dRec->pLength = -1 * max(abs(dRec->pLength), sizeof(data_iType));	// free dRec has negative length
	dRec->next = header.free_dList;
	dRec->parm.back = NIL;
	header.free_dList = dIndex;
	Concat_Free_dRec(dIndex, dRec);
	Write_dRec(dIndex, dRec);
} // Free_dRec



/*******************************************************************************
 * Grow_dList
 *
 *******************************************************************************/

data_iType Grow_dList(void)
{
	long		startEOF, curEOF;
	dRecType	dRec;
	
	err = GetEOF(fileNum, &startEOF);
	Write_BlockId(dBlockId);
	curEOF = startEOF + sizeof(block_idType);
	dRec.pLength = dBlockSize - sizeof(block_idType) - sizeof(dRecType); // create new dRec
	Write_dRec(curEOF, &dRec);
	err = SetEOF(fileNum, startEOF + dBlockSize);
	Free_dRec(curEOF, &dRec);				// add dRec to header.free_dList
	if (err == noErr)
		return (curEOF);
	else
		return (NIL);
} // Grow_dList



/*******************************************************************************
 * New_dRec
 *
 *******************************************************************************/
 
data_iType New_dRec(dRecType *dRec)
{
	dRecType		dRec2;
	data_iType		dIndex;
	
	if (header.free_dList == NIL)
		header.free_dList = Grow_dList();
	Read_dRec(header.free_dList, dRec);
	Concat_Free_dRec(header.free_dList, dRec);
	dIndex = header.free_dList;
	header.free_dList = dRec->next;
	if (header.free_dList != NIL)
	{
		Read_dRec(header.free_dList, &dRec2);
		dRec2.parm.back = NIL;
		Write_dRec(header.free_dList, &dRec2);
	}
	dRec->pLength = abs(dRec->pLength);		// block is now in use
	dRec->next = NIL;
	dRec->parm.timestamp = 0L;
	Write_dRec(dIndex, dRec);				// so concat_free_drec doesn't think it's still free
	return (dIndex);
} // New_dRec
 


/*******************************************************************************
 * Read_TimeStamp
 *
 *******************************************************************************/

pascal TimeStamp_Type Read_TimeStamp(Key_Type key)
{
	data_iType		dIndex;
	dRecType		dRec;
	
	err = noErr;
	dIndex = Search_BTree(key);
	if (dIndex != NIL)
	{
		Read_dRec(dIndex, &dRec);
		return (dRec.parm.timestamp);
	}
	return 0L;
} // Read_TimeStamp


/*******************************************************************************
 * Dispose1_Data
 *
 *******************************************************************************/

void Dispose1_Data(data_iType dIndex)			// dispose of data but not index to data
{
	dRecType		dRec;
	
	if (dIndex != NIL)
	{
		Read_dRec(dIndex, &dRec);
		Dispose1_Data(dRec.next);
		Free_dRec(dIndex, &dRec);
	}
} // Dispose1_Data



/*******************************************************************************
 * Dispose_Data
 *
 *******************************************************************************/

pascal Boolean Dispose_Data(Key_Type key, TimeStamp_Type *old_timestamp)
{
	TimeStamp_Type	timestamp;
	
	err = noErr;
	
	if (old_timestamp)
	{
		timestamp = Read_TimeStamp(key);
		if ((*old_timestamp) && (timestamp != *old_timestamp))
		{
			*old_timestamp = timestamp;
			err = 1;
			return (FALSE);
		}
		*old_timestamp = timestamp;
	}
	
	Dispose1_Data(Delete_BTree(key));			// dispose of data & index to data
	Write_Header();
	
	return (err == noErr);
} // Dispose_Data



/*******************************************************************************
 * Save1_Data
 *
 *******************************************************************************/

data_iType Save1_Data(Ptr data, short dataLen)
{
	dRecType		dRec, dRec2;
	data_iType		dIndex;
	long			size;
	data_iType		result;
	
	dIndex = New_dRec(&dRec);
	if (dataLen >= dRec.pLength)
		size = dRec.pLength;						// fragment large data
	else
		size = dataLen;								// fragment record into used & free parts
		
	err = SetFPos(fileNum, fsFromStart, dIndex + sizeof(dRecType));
	err = FSWrite(fileNum, &size, data);

	if (dRec.pLength - size > sizeof(dRecType))
	{												// make unused portion of data record free
		dRec2.pLength = dRec.pLength - size - sizeof(dRecType);
		Free_dRec(dIndex + sizeof(dRecType) + size, &dRec2);
		dRec.pLength = size;
	}
		
	if (dataLen > size)
		dRec.next = Save1_Data((Ptr)((long)data + size), dataLen - size);	// fragment data
	else
		dRec.next = NIL;							// end of data
	dRec.aLength = size;
	GetDateTime(&(dRec.parm.timestamp));
	Write_dRec(dIndex, &dRec);
	return (dIndex);
} // Save1_Data



/*******************************************************************************
 * Save_Data
 *
 *******************************************************************************/

pascal Boolean Save_Data(Key_Type key, Handle data, TimeStamp_Type *old_timestamp)
{
	data_iType		dIndex;
	dRecType		dRec;
	TimeStamp_Type	timestamp;
	
	if (GetHandleSize(data) > 0L)
	{
		err = noErr;
		dIndex = Search_BTree(key);
		if (dIndex != NIL)
		{
			if (old_timestamp)
			{
				Read_dRec(dIndex, &dRec);
				timestamp = dRec.parm.timestamp;
				if ((*old_timestamp) && (timestamp != *old_timestamp))
				{
					*old_timestamp = timestamp;
					err = 1;
					return (FALSE);
				}
				*old_timestamp = timestamp;
			}
			Dispose1_Data(dIndex);				// dispose of existing data
		}
		HLock(data);
		Insert_BTree(key, dIndex = Save1_Data(*data, GetHandleSize(data))); // insert data & update index's data
		HUnlock(data);
		Write_Header();
		if ((old_timestamp) && (err == noErr))
		{
			Read_dRec(dIndex, &dRec);
			*old_timestamp = dRec.parm.timestamp;
		}
		return (err == noErr);
	}
	else
		return (Dispose_Data(key, old_timestamp));	// dispose of existing data & index when new data is empty
} // Save_Data



/*******************************************************************************
 * Read_Data
 *
 *******************************************************************************/

pascal Handle Read_Data(Key_Type key, TimeStamp_Type *timestamp)
{
	data_iType		dIndex;
	dRecType		dRec;
	Handle			data = NULL;
	long			size, pos;
	
	err = noErr;
	dIndex = Search_BTree(key);
	if (dIndex != NIL)
	{
		do {
			Read_dRec(dIndex, &dRec);
			if (data == NULL)
			{										// make space for first fragment of data
				if (timestamp)
					*timestamp = dRec.parm.timestamp;
				pos = 0L;
				data = NewHandle(dRec.aLength);
				err = MemError();
				if (err != noErr)
					return (NULL);
			}
			else									// make space for additional data fragments
			{
				pos = GetHandleSize(data);
				SetHandleSize(data, pos + dRec.aLength);
				err = MemError();
				if (err != noErr)
					return (NULL);
			}
			err = SetFPos(fileNum, fsFromStart, dIndex + sizeof(dRecType));
			size = dRec.aLength;
			HLock(data);
			err = FSRead(fileNum, &size, (Ptr)((long)(*data) + pos));
			HUnlock(data);
			dIndex = dRec.next;
		} while (dIndex != NIL);
	}
	return (data);
} // Read_Data


/*******************************************************************************
 * Init_iRec
 *
 *******************************************************************************/

void Init_iRec(iRecType *iRec)
{
	iRec->next = NIL;
	iRec->previous = NIL;
	iRec->index[0] = '\0';
	iRec->id = NoID;
} // Init_iRec



/*******************************************************************************
 * Read_iRec
 *
 *******************************************************************************/

void Read_iRec(long elem, iRecType *iRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, elem);
	size = sizeof(iRecType);
	err = FSRead(fileNum, &size, iRec);
} // Read_iRec



/*******************************************************************************
 * Write_iRec
 *
 *******************************************************************************/
 
void Write_iRec(long elem, iRecType *iRec)
{
	long		size;
	
	err = SetFPos(fileNum, fsFromStart, elem);
	size = sizeof(iRecType);
	err = FSWrite(fileNum, &size, iRec);
} // Write_iRec
 


/*******************************************************************************
 * Grow_iList
 *
 *******************************************************************************/

long Grow_iList(void)
{
	long		startEOF, curEOF;
	iRecType	iRec;
	long		size;
	
	err = GetEOF(fileNum, &startEOF);
	Write_BlockId(iBlockId);
	curEOF = startEOF + sizeof(block_idType);
	size = sizeof(iRecType);
	while (((curEOF + size - 1L) / iBlockSize) == (startEOF / iBlockSize))
	{
		Init_iRec(&iRec);
		if (((curEOF + size * 2L - 1L) / iBlockSize) == (startEOF / iBlockSize))
			iRec.next = curEOF + size;			// link free list
		Write_iRec(curEOF, &iRec);
		curEOF += size;
	}
	
	size = iBlockSize - (curEOF % iBlockSize);
	if (size < iBlockSize)
		err = SetEOF(fileNum, curEOF + size);
	if (err == noErr)
		return (startEOF + sizeof(block_idType));
	else
		return (NIL);
} // Grow_iList



/*******************************************************************************
 * New_iRec
 *
 *******************************************************************************/

long New_iRec(iRecType *iRec)
{
	long		result;
	
	if (header.free_iList == NIL)
		header.free_iList = Grow_iList();
	result = header.free_iList;
	Read_iRec(header.free_iList, iRec);
	header.free_iList = iRec->next;
	header.iCount++;
	Init_iRec(iRec);
	return (result);
} // New_iRec



/*******************************************************************************
 * Dispose_iRec
 *
 *******************************************************************************/

void Dispose_iRec(long elem)
{
	iRecType		iRec;
	
	Init_iRec(&iRec);
	iRec.next = header.free_iList;
	header.free_iList = elem;
	header.iCount--;
	Write_iRec(elem, &iRec);
} // Dispose_iRec



/*******************************************************************************
 * Find_Elem
 *
 *******************************************************************************/

long Find_Elem(ID_Type id)
{
	long		elem;
	iRecType	iRec;
	
	elem = ReadCache(id);
	if (elem == NIL)
	{
		elem = header.iRoot;
		while (elem != NIL)
		{
			Read_iRec(elem, &iRec);
			if (iRec.id == id)
				break;
			elem = iRec.next;
		}
	}
	return (elem);
} // Find_Elem



/*******************************************************************************
 * Read_Index
 *
 *******************************************************************************/

pascal ID_Type Read_Index(ID_Type id)
{
	long		elem;
	iRecType	iRec;

	err = noErr;
	if (id == NoID)
	{									// return id of first element
		if (header.iRoot == NIL)
			return (NoID);
		Read_iRec(header.iRoot, &iRec);
		WriteCache(iRec.id, header.iRoot);
		return (iRec.id);
	}
	else
	{									// verify id of this element}
		elem = Find_Elem(id);
		if (elem == NIL)
			return (NoID);
		else
		{
			Read_iRec(elem, &iRec);
			WriteCache(id, elem);
			return (id);
		}
	}
} // Read_Index



/*******************************************************************************
 * Read_Next
 *
 *******************************************************************************/

pascal ID_Type Read_Next(ID_Type current_id)
{
	long		elem;
	iRecType	iRec;

	err = noErr;
	if (current_id == NoID)
	{										// return id of first element
		if (header.iRoot == NIL)
			return (NoID);
		Read_iRec(header.iRoot, &iRec);
		WriteCache(iRec.id, header.iRoot);
		return (iRec.id);
	}
	else
	{										// return id of next element
		elem = Find_Elem(current_id);
		if (elem == NIL)
			return (NoID);
		else
		{
			Read_iRec(elem, &iRec);
			elem = iRec.next;
			if (elem == NIL)
				return (NoID);
			else
			{
				Read_iRec(elem, &iRec);
				WriteCache(iRec.id, elem);
				return (iRec.id);
			}
		}
	}
} // Read_Next



/*******************************************************************************
 * Read_Previous
 *
 *******************************************************************************/

pascal ID_Type Read_Previous(ID_Type current_id)
{
	long		elem;
	iRecType	iRec;

	err = noErr;
	if (current_id == NoID)
	{											// return id of last element
		if (header.iRoot == NIL)
			return (NoID);
		elem = header.iRoot;
		while (elem != NIL)
		{
			Read_iRec(elem, &iRec);
			elem = iRec.next;
		}
		return (iRec.id);
	}
	else
	{											// return id of previous element
		elem = Find_Elem(current_id);
		if (elem == NIL)
			return (NoID);
		else
		{
			Read_iRec(elem, &iRec);
			elem = iRec.previous;
			if (elem == NIL)
				return (NoID);
			else
			{
				Read_iRec(elem, &iRec);
				WriteCache(iRec.id, elem);
				return (iRec.id);
			}
		}
	}
} // Read_Previous



/*******************************************************************************
 * Insert_Index
 *
 *******************************************************************************/

pascal void Insert_Index(Index_Type index, ID_Type id)
{
	iRecType		newRec;
	iRecType		iRec;
	long			newElem;
	long			elem;
	
	err = noErr;

	newElem = New_iRec(&newRec);
	BlockMove((Ptr)index, (Ptr)(newRec.index), (long)(index[0] + 1));
	newRec.id = id;

	if (header.iRoot == NIL)
		header.iRoot = newElem;
	else
	{
		elem = header.iRoot;
		Read_iRec(elem, &iRec);

		// special case, first element in list
		if (IUMagString((Ptr)&(index[1]), (Ptr)&(iRec.index[1]), index[0], iRec.index[0]) == -1)
		{
			newRec.next = elem;
			newRec.previous = NIL;
			iRec.previous = newElem;
			Write_iRec(elem, &iRec);
			header.iRoot = newElem;
		}
		else
		{
			// search until iRec is the element which comes after our new element
			while (IUMagString((Ptr)&(index[1]), (Ptr)&(iRec.index[1]), index[0], iRec.index[0]) != -1)
			{
				if (iRec.next == NIL)
				{
					// insert at the end of the list, create dummy iRec that follows our new element
					iRec.next = NIL;
					iRec.previous = elem;
					elem = NIL;
					break;
				}
				elem = iRec.next;
				Read_iRec(elem, &iRec);
			}

			// insert in middle of list, iRec is the element after our new element
			newRec.previous = iRec.previous;
			newRec.next = elem;
			iRec.previous = newElem;
			if (elem != NIL)
				Write_iRec(elem, &iRec);
				
			// read and update the element before our new element
			Read_iRec(newRec.previous, &iRec);
			iRec.next = newElem;
			Write_iRec(newRec.previous, &iRec);
		}

	} // if header.iRoot == NIL

	Write_iRec(newElem, &newRec);
	Write_Header();
} // Insert_Index



/*******************************************************************************
 * Delete_Index
 *
 *******************************************************************************/

pascal void Delete_Index(ID_Type id)
{
	iRecType		iRec, iRec2;
	long			elem;

	if (id != NoID)
	{
		elem = Find_Elem(id);
		if (elem == NIL)
			err = qErr;
		else
		{
			err = noErr;

			Read_iRec(elem, &iRec);

			if (iRec.previous == NIL)
				header.iRoot = iRec.next;
			else
			{
				Read_iRec(iRec.previous, &iRec2);
				iRec2.next = iRec.next;
				Write_iRec(iRec.previous, &iRec2);
			}

			if (iRec.next != NIL)
			{
				Read_iRec(iRec.next, &iRec2);
				iRec2.previous = iRec.previous;
				Write_iRec(iRec.next, &iRec2);
			}

			Dispose_iRec(elem);
			Write_Header();
		}
	}
} // Delete_Index


/*******************************************************************************
 * Find_Index
 *
 *******************************************************************************/

pascal ID_Type Find_Index(Index_Type index, ID_Type id)
{
	iRecType		iRec;
	long			elem;
	short			len;
	
	err = noErr;
	if (id == NoID)
		elem = header.iRoot;
	else
		elem = Find_Elem(id);
	len = index[0];
	while (elem != NIL)
	{
		Read_iRec(elem, &iRec);
		if (IUMagString((Ptr)&(index[1]), (Ptr)&(iRec.index[1]), len, len) == 0)
		{
			return (iRec.id);
		}
		elem = iRec.next;
	}
	return (NoID);
} // Find_Index



/*******************************************************************************
 * Count_Index
 *
 *******************************************************************************/

pascal long Count_Index(void)
{
	/*
	iRecType		iRec;
	long			elem;
	long			count;
	
	elem = header.iRoot;
	count = 0L;
	while (elem != NIL)
	{
		Read_iRec(elem, &iRec);
		elem = iRec.next;
		count++;
	}
	return (count);
	*/
	
	return (header.iCount);
} // Count_Index



/*******************************************************************************
 * Use_File
 *
 *******************************************************************************/

pascal void Use_File(short newFileNum)
{
	err = noErr;
	fileNum = newFileNum;
	Read_Header();							// in case another file was current or another access path updated this info!
} // Use_File



/*******************************************************************************
 * Open_File
 *
 *******************************************************************************/

pascal short Open_File(short volNum, long dirID, Str255 name)
{
	short		openFileNum;
	short		result = nofile;
	
	err = HOpen(volNum, dirID, name, accessPerm, &openFileNum);
	if (err == noErr)
	{
		Use_File(openFileNum);
		if (header.version != Version)
			err = ioErr;
		if (err == noErr)
			result = openFileNum;
		else										// error reading header
		{
			FSClose(openFileNum);
			fileNum = nofile;
		}
	}
	return (result);
} // Open_File



/*******************************************************************************
 * Close_File
 *
 *******************************************************************************/

pascal void Close_File(short closeFileNum)
{
	short		remUseFile;
	
	remUseFile = fileNum;
	Use_File(closeFileNum);
	ReadCache(NoID); 						// flush references to this file out of the cache
	if (closeFileNum == remUseFile)
		Use_File(nofile);
	else
		Use_File(remUseFile);
	err = FSClose(closeFileNum);
} // Close_File



/*******************************************************************************
 * Create_File
 *
 *******************************************************************************/

pascal void Create_File(short volNum, long dirID, Str255 name, OSType creator, OSType fileType, Boolean allowReplace)
{
	short		createFileNum;
	short		remFileNum;
	
	err = HCreate(volNum, dirID, name, creator, fileType);
	if ((err == dupFNErr) && (allowReplace))
	{
		err = HDelete(volNum, dirID, name);
		if (err == noErr)
			err = HCreate(volNum, dirID, name, creator, fileType);
	}
	if (err == noErr)
	{
		err = HOpen(volNum, dirID, name, accessPerm, &createFileNum);
		if (err == noErr)
		{
			remFileNum = fileNum;
			fileNum = createFileNum;
			Write_Header();
			header.version = Version;
			header.maxkeys = Maxkeys;
			header.bRoot = NIL;
			header.free_bList = Grow_bList();
			header.free_dList = NIL;
			header.iRoot = NIL;
			header.iCount = 0L;
			header.free_iList = NIL;
			header.checksum = 0L;
			Write_Header();
			Close_File(createFileNum);
			Use_File(remFileNum);
		}
	}
} // Create_File



/*******************************************************************************
 * Remove_File
 *
 *******************************************************************************/

pascal void Remove_File(short volNum, long dirID, Str255 name)
{
	err = HDelete(volNum, dirID, name);				// deletes a closed file
} // Remove_File
