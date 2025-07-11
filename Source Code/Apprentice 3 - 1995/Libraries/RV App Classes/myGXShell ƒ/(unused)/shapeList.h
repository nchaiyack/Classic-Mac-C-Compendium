THIS CODE IS UNFINISHED AND UNTESTED

class shapeList
{
	friend class shapeList_iterator;
	
	public:
		shapeList( long maxItems = 20);
		~shapeList();
		
		void Clear();
		long CountItems() const;
		void Add( const gxShape item);
		void Remove( const gxShape item);
		Boolean Contains( const gxShape item) const;
		
	private:
		long numItems;
		long maxNumItems;
		gxShape *theItems;
		
		long numIterators;

		long indexFor( const gxShape item, Boolean *isPresent) const;
		
		void InsertItemAt( const gxShape item, long index);
		void RemoveItemAt( long index);
};

class shapeList_iterator
{
	public:
		shapeList_iterator( shapeList &items);
		~shapeList_iterator();

		Boolean operator()( gxShape *item);
		
	private:
		shapeList *the_shapeList;
		long currentItem;		// runs down!
};

inline void shapeList::Clear()
{
	numItems = 0;
}

inline long shapeList::CountItems() const
{
	return numItems;
}
