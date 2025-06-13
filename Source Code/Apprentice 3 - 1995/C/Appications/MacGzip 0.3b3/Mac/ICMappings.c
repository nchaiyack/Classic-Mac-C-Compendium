#include "ICAPI.h"
#include "ICKeys.h"

#define BlockMoveData( s, d, l )	BlockMove( s, d, l )

#if defined (__MWERKS__) || defined (MPW)
/* ejo: this is to keep the compiler quiet */
#include "ICMappings.h"
/* ejo: this is to get error codes */
#include <Errors.h>
/* ejo: this is for Munger () */
#include <TextUtils.h>
#endif

static void UnpackCopyString (Ptr *p, StringPtr s)
{
	short	len;

	len = (**(char **)p) + 1;
	BlockMoveData(*p, s, len);
	*p = *p + len;
}

static void PackCopyString (ConstStr255Param s, Ptr p, short *length)
{
	BlockMoveData(s, (Ptr)(p + *length), s[0] + 1);
	*length += s[0] + 1;
}


// (* WARNING: Depends very much on the exact format of ICMapEntry! *)
static OSErr UnpackEntry (Handle entries, long pos, ICMapEntry *entry, long *user_length)
{	
	OSErr	err = noErr;
	Ptr		p, org;
	long	maxsize;
	
	if ((entries == nil) || (*entries == nil) || 
		(pos < 0) || (pos > GetHandleSize(entries) - 6))
		return(paramErr);

	p = (*entries) + pos;
	maxsize = GetHandleSize(entries);
	org = p;
	
	BlockMoveData(p, entry, 6);
	
	if  ((entry->fixed_length != ICmap_fixed_length) || 
		 (entry->fixed_length > entry->total_length) ||
		 (entry->total_length > maxsize)) {
		err = badExtResource;
	} else {
		BlockMoveData(p, entry, entry->fixed_length);
		p = p + entry->fixed_length;
		UnpackCopyString(&p, entry->extension);
		UnpackCopyString(&p, entry->creator_app_name);
		UnpackCopyString(&p, entry->post_app_name);
		UnpackCopyString(&p, entry->MIME_type);
		UnpackCopyString(&p, entry->entry_name);
		*user_length = entry->total_length - (p - org);
	}
	return(err);
}

static void PackEntry (ICMapEntry *entry, Ptr p, long user_length)
{
	entry->version = 0;
	entry->fixed_length = (short)((long)&entry->extension - (long)entry);
	entry->total_length = entry->fixed_length;
	PackCopyString(entry->extension, p, &entry->total_length);
	PackCopyString(entry->creator_app_name, p, &entry->total_length);
	PackCopyString(entry->post_app_name, p, &entry->total_length);
	PackCopyString(entry->MIME_type, p, &entry->total_length);
	PackCopyString(entry->entry_name, p, &entry->total_length);
	entry->total_length += user_length;
	BlockMoveData(&entry, p, entry->fixed_length);
}


ICError ICMDeleteEntry (Handle entries, long pos)
{
	ICMapEntry	entry;
	long		lerr;
	long		user_length;
	
	lerr = UnpackEntry(entries, pos, &entry, &user_length);
	if (lerr == noErr) {
		lerr = Munger(entries, pos, nil, entry.total_length, (Ptr)-1, 0);
		if (lerr >= 0)	lerr = noErr;
	}
	return(lerr);
}


ICError	ICMCountEntries (Handle entries, long *count)
{
	short	*p;
	long	pos = 0;
	
	p = (short *)(*entries);
	*count = 0;
	while (pos < GetHandleSize(entries)) {
		pos += *p;
		p = (short*)(p + *p);
		(*count)++;
	}
	return(noErr);
}

ICError ICMGetEntry (Handle entries, long pos, ICMapEntry *entry)
{
	long	user_length;
	
	return(UnpackEntry(entries, pos, entry, &user_length));
}

ICError	ICMGetIndEntry (Handle entries, long ndx,
						long *pos, ICMapEntry *entry)
{
	/*ICError	err; 22may95,ejo: this is not used */
	short	*p;
	/*long	i; 22may95;ejo: this is not used */
	
	p = (short *)(*entries);
	*pos = 0;
	while ((ndx > 1) && (*pos < GetHandleSize(entries))) {
		*pos += *p;
		p = (short*)(p + *p);
		ndx -= 1;
	}
	return(ICMGetEntry(entries, *pos, entry));
}

ICError ICMAddEntry (Handle entries, ICMapEntry *entry)
{
	ICMapEntry	e;
	
	PackEntry(entry, (Ptr)&e, 0);
	return(PtrAndHand(&e, entries, entry->total_length));
}

ICError ICMSetEntry (Handle entries, long pos, ICMapEntry *entry)
{
	ICError		err;
	ICMapEntry	e, oldentry;
	long		user_length, source_length;
	
	err = UnpackEntry(entries, pos, &oldentry, &user_length);
	if (err == noErr) {
		PackEntry(entry, (Ptr)&e, user_length);
		source_length = oldentry.total_length - user_length;
		if (user_length < 8) {
			// hack to remove alignment bytes from previous version
			source_length = oldentry.total_length;
			e.total_length = e.total_length - user_length;
			user_length = 0;
		}
		err = Munger(entries, pos, nil, source_length, &e, e.total_length - user_length);
		if (err >= 0)	err = noErr;
	}
	
	return(err);
}
