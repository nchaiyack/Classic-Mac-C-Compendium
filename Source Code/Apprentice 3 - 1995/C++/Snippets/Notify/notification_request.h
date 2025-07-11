//
// A bare-bones interface to the Notification manager
//
// To make it generally usable we maintain the Notification Manager Record,
// a small memory block containing the number of live instances, and the
// reply procedure in the system heap.
//
// #define DEBUG
//
// We have the following equalities:
//
//		num_destructed <= num_constructed;
//		num_answered   <= num_constructed;
//
// num_answered and num_destructed are not related in any way, however.
// They are kept to determine who is responsible of freeing memory allocated
// in the system heap. The destructor tests whether all notifications have
// been handled by comparing 'num_constructed' to 'num_answered'. If they
// are equal and this is the last instance to go (i.e. 'num_constructed'
// equals 'num_destructed' the system heap blocks are freed. This way the
// system heap blocks are _not_ freed when the last instance 'dies' while
// not all replies have been answered yet. In that case the NMResp function
// frees the blocks (including the one it is in itself!). The function determines
// that it has to do the job itself by looking at the field 'do_it_yourself'.
//
// the structure of the information record created in the system heap:
//
class notify_info
{
	friend class notification_request;
	
	friend pascal void main( struct NMRec *theNMRec);

	private:
		notify_info();
		~notify_info();

		void *operator new( size_t sz);
		void operator delete( void *me);

		unsigned int num_constructed;
		unsigned int num_answered;
		unsigned int num_destructed;
		Handle response_handle;	// to be freed by the last one to leave
		NMProcPtr    response_proc;
};

inline void *notify_info::operator new( size_t sz)
{
	return (void *)NewPtrSysClear( (long)sz);
}

inline void notify_info::operator delete( void *me)
{
	DisposePtr( (Ptr)me);
}

inline notify_info::~notify_info()
{
	DisposeHandle( response_handle);
}

class notification_request
{
	friend void notify_user( const char * const message, int audible);
	friend pascal void main( struct NMRec *theNMRec);

	private:
		notification_request( const char * const message, int audible = false);
		~notification_request();

		void *operator new( size_t sz);
		void operator delete( void *me);

		int num_instances() const;
		int num_to_be_answered() const;
		//
		// We maintain a pointer to a 'theInfo' record. This way we can
		// purposefully forget to deallocate the record on exit when
		// there are still outstanding notification requests.
		//
		NMRec theNMRec;
		//
		// theInfo always equals 'staticInfo'. We maintain a copy since the Notification
		// managers response procedure can not access 'staticInfo', which is an application
		// global (which even may no longer be around since the application may already
		// have quit)
		// We need a static variable since we only want to allocate one 'notify_info' record.
		// The notify_info record is deleted either by the application or by a nmResponse
		// procedure.
		// The application can't always do this since there may be unanswered response procs
		// after the app has quit, the response procs could do it when the last nmResponse proc
		// is executed, but this could lead to extra processing (repeated allocation of the
		// 'notify_info' record when notifications are posted and answered in sequence)
		//
		notify_info *theInfo;
		static notify_info *staticInfo;
				
		static const int an_error_occurred;
		//
		// strlen_255 returns the smaller of the length of the C string and 255
		//
		static int strlen_255( const char * const string);
};

inline void *notification_request::operator new( size_t sz)
{
	return (void *)NewPtrSysClear( (long)sz);
}

inline void notification_request::operator delete( void *me)
{
	DisposePtr( (Ptr)me);
}

inline int notification_request::num_instances() const
{
	int result = an_error_occurred;
	if( theInfo != 0)
	{
		result = theInfo->num_constructed - theInfo->num_destructed;
	}
	return result;
}

inline int notification_request::num_to_be_answered() const
{
	int result = an_error_occurred;
	if( theInfo != 0)
	{
		result = theInfo->num_constructed - theInfo->num_answered;
	}
	return result;
}

void notify_user( const char * const message, int audible = false);
