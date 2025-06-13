//
// queue is a template for a queue
//
// This is the simplest of queues, taken directly from
// Knuth, Fundamental algorithms, page 240, but with some changes
//
// Note: this code _assumes_ that class C is a pointer type
// (well, at least that one can do 'C a_C = 0')
//
template<class C> class queue
{
	public:
		queue( unsigned int numItems);
		~queue();

		Boolean isEmpty() const;
		Boolean isFull() const;
		unsigned int getLength() const;
		
		void Add( C newItem);
		C Remove();

	private:
		const unsigned int size;
		unsigned int F;
		unsigned int R;
		unsigned int numInside;
		C *theItems;
};

template<class C> inline Boolean queue<C>::isEmpty() const
{
	return (numInside == 0);
}

template<class C> inline Boolean queue<C>::isFull() const
{
	return (numInside == size);
}

template<class C> inline unsigned int queue<C>::getLength() const
{
	return numInside;
}
