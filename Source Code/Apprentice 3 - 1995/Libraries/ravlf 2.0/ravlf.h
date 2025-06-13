/*******************************************************************************
 * ravlf.h
 *
 * Random-access, variable-length files
 * C version 2.0, converted from Pascal to C April 1994, Modified February 1995
 * By Tad Woods
 * Postal mail: P.O. Box 1210, Salem, VA 24153
 * CompuServe: 70312,3552   America Online: "Tad Woods"
 * Internet         mail: tandtsw@roanoke.infi.net
 *    anonymous ftp host: ftp.infi.net
 *             directory: /pub/customer/roanoke/tandtsw
 *
 * Random-access, variable-length files (ravlf) implements direct access
 * to variable-length records in a file's data fork.
 *
 * DISCLAIMER
 * ----------
 * You may compile this code (ravlf.c and ravlf.h) into your own 
 * applications at no charge, as long as those applications are used
 * in-house or distributed for free. To compile this code in shareware or
 * commercial applications please contact the author for written permission.
 * This software has been tested but comes with no guarantees or technical
 * support. DO NOT SELL OR DISTRIBUTE THIS SOURCE CODE without the author's
 * written permission.
 *
 * GENERAL INTERFACE
 * -----------------
 * The functionality is similar to, but simpler than, the Mac's Resource Manager.
 * Call Init_Disk at the start of your program. Call Create_File to create
 * a new ravlf. Call Open_File to open a ravlf once it has been created.
 * Call Use_File to make a file the active ravlf, after it has been opened.
 * Call Save_Data to store a handle's data in the active ravlf. Call
 * Read_Data to read data from the active ravlf into a handle. Call
 * Call Dispose_Data to delete a record. Call Close_File to close a ravlf.
 * Call Dispose_Disk when your program Quits.
 *
 * RECORD KEYS
 * -----------
 * Records are identified by a key value, defined as a long by Key_Type,
 * and may be any value in the valid range of a long, except -1 which
 * is reserved for "record not found." You must provide a key value when
 * calling Save_Data and Read_Data. You may generate the key value yourself
 * or call Max_Key (and add 1) or Unique_Key to obtain a unique key value.
 * Note that records must have unique keys. If you call Save_Data with a
 * key value that already exists, the existing record will be deleted and
 * replaced with the new data.
 *
 * MAXIMUM RECORD SIZE
 * -------------------
 * The maximum size of handle's data that can be saved, which is the same
 * size of one ravlf record, is limited by the type of the pLength and 
 * aLength fields of the dRecType typedef in "ravlf.c." They are currently
 * defined as shorts, which limits the record size to 32K. If you need to
 * store larger records change these two fields to long types. If you do
 * this, files created under the old types will not be readable and may
 * cause a crash if you try to read them.
 *
 * TIMESTAMP (optional parameter)
 * ------------------------------
 * Read_Data, Save_Data, and Dispose_Data also take a timestamp parameter.
 * Save_Data and Dispose_Data will do nothing and return FALSE if the
 * timestamp value does not match the timestamp value of the currently
 * saved record with the same key value. Pass NULL for timestamp to ignore
 * this check. (This can be used to prevent users from updating the same
 * record at the same time in multi-user situations.)
 *
 * INDEXING (optional functions)
 * --------------------------
 * By default ravlf records are only indexed by their key values (described
 * above). You may add one additional index in the form of a string, defined
 * by Index_Type. Call Insert_Index to associate a string index with a key
 * value (pass the key value as the id parameter). Call Find_Index to obtain
 * the key value associated with a given string. Call Read_Next and 
 * Read_Previous to obtain the id's of sequential indexed records. Pass -1
 * to obtain the first or last indexed record id, respectively. Call 
 * Delete_Index to remove an index.
 *
 * INDEX LIMITATIONS
 * -----------------
 * One record can have only one index associated with it, and one index can
 * have only one record associated with it. If you use the optional indexing,
 * you should call Insert_Index just after calling Save_Data, and Delete_Index
 * just after calling Dispose_Data. This is because deleting an index does
 * not automatically delete an associated record, or deleting a record does
 * not automatically delete an associated index.
 *
 * INDEX FIELD SIZE
 * ----------------
 * You may change the type of Index_Type in order to change its fixed length,
 * but it must remain a pascal string. If you do change Index_Type, files
 * created under the old type will not be readable and may cause a crash if
 * you try to read them.
 *
 *******************************************************************************/

#pragma once

#define Version	2
#define NoID	-1L

typedef long			Key_Type;
typedef long			ID_Type;
typedef Str32			Index_Type;
typedef unsigned long	TimeStamp_Type;



/*******************************************************************************
 * Init_Disk
 *
 * Call once at the start of your program before any other ravlf function.
 *******************************************************************************/
pascal void Init_Disk(void);



/*******************************************************************************
 * Init_Disk
 *
 * Call once just before your program Quits.
 *******************************************************************************/
pascal void Dispose_Disk(void);


/*******************************************************************************
 * Disk_Error
 *
 * Returns an error code, or zero if no error occurred, from the last call to
 * a ravlf function.
 *******************************************************************************/
pascal short Disk_Error(void);



/*******************************************************************************
 * Cur_File
 *
 * Returns the file reference id of the currently active ravlf, or -1 if
 * no file is active.
 *******************************************************************************/
pascal short Cur_File(void);



/*******************************************************************************
 * Flush_Vol
 *
 *  -> volNum		volume reference number
 *  -> async		TRUE to return from Flush_Vol immediately
 *
 * Flushes the volume referenced by volNum.
 *******************************************************************************/
pascal void Flush_Vol(short volNum, Boolean async);



/*******************************************************************************
 * Use_File
 *
 *  -> newFileNum	ravlf file reference number (previously returned by
 * 					Open_File)
 *
 * Makes newFileNum the active ravlf. Other ravlf functions operate on the
 * active file.
 *******************************************************************************/
pascal void Use_File(short newFileNum);



/*******************************************************************************
 * Open_File
 *
 *  -> volNum		volume reference number
 * 	-> dirID		directory id on volume (or zero if volNum is a working
 *					directory reference number)
 *  -> name			name of file
 *
 * Opens a previously created file and makes it the active ravlf.
 *******************************************************************************/
pascal short Open_File(short volNum, long dirID, Str255 name);



/*******************************************************************************
 * Close_File
 *
 *  -> closeFileNum		ravlf file reference number
 *
 * Closes a previously opened ravlf.
 *******************************************************************************/
pascal void Close_File(short closeFileNum);



/*******************************************************************************
 * Create_File
 *
 *  -> volNum		volume reference number
 * 	-> dirID		directory id on volume (or zero if volNum is a working
 *					directory reference number)
 *  -> name			name of file
 *  -> creator		creator ID (application's signature)
 *  -> fileType		file type (custom file type)
 *  -> allowReplace	TRUE to allow the newly created file to replace an
 *					existing file of the same name
 *
 * Creates a ravlf.
 *******************************************************************************/
pascal void Create_File(short volNum, long dirID, Str255 name, OSType creator, OSType fileType, Boolean allowReplace);



/*******************************************************************************
 * Remove_File
 *
 *  -> volNum		volume reference number
 * 	-> dirID		directory id on volume (or zero if volNum is a working
 *					directory reference number)
 *  -> name			name of file
 *
 * Deletes a ravlf.
 *******************************************************************************/
pascal void Remove_File(short volNum, long dirID, Str255 name);



/*******************************************************************************
 * Valid_Key
 *
 * <-> key		pointer to key value
 *				on entry, key value to test
 *				on exit, same value, next existing sequential key value, or -1
 *
 * Returns TRUE if a record exists for the value passed in key, or FALSE
 * if it does not exist.
 *******************************************************************************/
pascal Boolean Valid_Key(Key_Type *key);



/*******************************************************************************
 * Max_Key
 *
 * Returns the maxmium existing key value.
 *******************************************************************************/
pascal Key_Type Max_Key(void);



/*******************************************************************************
 * Max_Key
 *
 * Returns the next non-existing, minimum, unique key value.
 *******************************************************************************/
pascal Key_Type Unique_Key(void);



/*******************************************************************************
 * Dump_Tree
 *
 * An internal debugging function.
 *******************************************************************************/
#ifdef B_TREE_DEBUG
pascal void Dump_Tree(void);
#endif



/*******************************************************************************
 * Read_TimeStamp
 *
 *  -> key		a key value
 *
 * Returns the timestamp value associated with the record associated with
 * the key value.
 *******************************************************************************/
pascal TimeStamp_Type Read_TimeStamp(Key_Type key);



/*******************************************************************************
 * Dispose_Data
 *
 *  -> key				a key value
 *  -> old_timestamp	pointer to a timestamp value, or NULL to ignore
 *						on entry, timestamp value returned from previous
 *						          Read_Data or Save_Data call
 *
 * Returns TRUE if the record associated with key was deleted.
 * Returns FALSE and does nothing if old_timestamp ­ NULL, and old_timestamp
 * references a non-zero value, and old_timestamp does not match the timestamp
 * value now associated with the record associated with key.
 *******************************************************************************/
pascal Boolean Dispose_Data(Key_Type key, TimeStamp_Type *old_timestamp);



/*******************************************************************************
 * Save_Data
 *
 *  -> key				a key value
 *  -> data				a valid Macintosh Handle to data to save
 * <-> old_timestamp	pointer to a timestamp value, or NULL to ignore
 *						on entry, timestamp value returned from previous
 *						          Read_Data or Save_Data call
 *						on exit, timestamp value for newly saved record
 *
 * Returns TRUE if the record associated with key was saved.
 * Returns FALSE and does nothing if old_timestamp ­ NULL, and old_timestamp
 * references a non-zero value, and old_timestamp does not match the timestamp
 * value now associated with the record associated with key.
 * NOTES: data will be unlocked on exit. data must be valid, but if it is
 * zero length no record will be saved and Dispose_Data will be called for
 * the record associated with key instead.
 *******************************************************************************/
pascal Boolean Save_Data(Key_Type key, Handle data, TimeStamp_Type *old_timestamp);



/*******************************************************************************
 * Read_Data
 *
 *  -> key				a key value
 * <-  timestamp		pointer to a timestamp value, or NULL to ignore
 *						on exit, timestamp value for record read
 *
 * Returns a newly allocate Handle for the record associated with key,
 * or NULL if no record was found. It is up to the calling program to
 * call DisposHandle when it is ready to dispose of this new Handle.
 *******************************************************************************/
pascal Handle Read_Data(Key_Type key, TimeStamp_Type *timestamp);



/*******************************************************************************
 * Read_Index
 *
 *  -> id				a key value, or -1 to get the id of first item in
 *						the index
 *
 * Returns id if an index exists for the id, or -1 if no index exists for id.
 *******************************************************************************/
pascal ID_Type Read_Index(ID_Type id);



/*******************************************************************************
 * Read_Next
 *
 *  -> current_id		a key value, or -1 to get the id of first item in
 *						the index
 *
 * Returns the id of the item that is sequentially after current_id in the
 * index, or -1 if there are no more items.
 *******************************************************************************/
pascal ID_Type Read_Next(ID_Type current_id);



/*******************************************************************************
 * Read_Previous
 *
 *  -> current_id		a key value, or -1 to get the id of last item in
 *						the index
 *
 * Returns the id of the item that is sequentially before current_id in the
 * index, or -1 if there are no more items.
 *******************************************************************************/
pascal ID_Type Read_Previous(ID_Type current_id);



/*******************************************************************************
 * Insert_Index
 *
 *  -> index			an index value to associate with the record associated
 *						with id
 *  -> id				a key value
 *
 * Adds an index item for id.
 *******************************************************************************/
pascal void Insert_Index(Index_Type index, ID_Type id);



/*******************************************************************************
 * Delete_Index
 *
 *  -> id				a key value
 *
 * Deletes the index item associated with id.
 *******************************************************************************/
pascal void Delete_Index(ID_Type id);



/*******************************************************************************
 * Find_Index
 *
 *  -> index			an index value to search for
 *  -> id				a key value of item to start search on, or -1 to
 *						search from the first item in the index
 *
 * Returns the id of the item (the key value) that is associated with index.
 * Note: The strings must match exactly and case is significant; substitute
 * the IUMagString call in the Find_Index function to change this.
 *******************************************************************************/
pascal ID_Type Find_Index(Index_Type index, ID_Type id);



/*******************************************************************************
 * Count_Index
 *
 * Returns a count of the number of indexed items. If you Insert_Index for
 * each record you Save_Data, this will also be the record count.
 *******************************************************************************/
pascal long Count_Index(void);


