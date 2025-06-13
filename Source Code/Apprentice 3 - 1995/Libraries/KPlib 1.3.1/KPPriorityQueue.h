// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_PRIORITY_QUEUE_DEFINED
#define KP_PRIORITY_QUEUE_DEFINED

#include <stdlib.h>
#include "KPbasic.h"
#include "KPList.h"

// Assumes Element has a default constructor, operator=(), operator==(),
// and operator<().  Note that operator<() must place a total ordering on
// the set of Elements.

template <class Element>
class KPPriorityQueue {
    public:
        KPPriorityQueue();
        KPPriorityQueue(const KPPriorityQueue<Element> &queue);
        KPPriorityQueue(const KPList<Element> &list);
        KPPriorityQueue(const Element &element);
        ~KPPriorityQueue();
        operator KPList<Element>() const;
        KPPriorityQueue<Element> &
							operator=(const KPPriorityQueue<Element> &queue);
        KPPriorityQueue<Element> &operator=(const Element &element);
        KPPriorityQueue<Element> &enqueue(const Element &element);
        Element dequeue();
        Element &head();
        const Element &head() const;
        Element &tail();
        const Element &tail() const;
        int size() const;
        bool is_empty() const;
        KPPriorityQueue<Element> &clear();
    protected:
        static void queue_empty_error(const char *fname);
        KPSortableList<Element> my_list;
};

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::KPPriorityQueue(): my_list()
{ /* do nothing. */ }

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::KPPriorityQueue(const KPPriorityQueue<Element>
                                            &queue): my_list(queue.my_list)
{ /* do nothing. */ }

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::KPPriorityQueue(const KPList<Element> &list):
                                                                my_list(list)
{
    my_list.sort();
}

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::KPPriorityQueue(const Element &element):
                                                            my_list(element)
{ /* do nothing. */ }

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::~KPPriorityQueue()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
inline
KPPriorityQueue<Element>::operator KPList<Element>() const
{
    return my_list;
}

/****************************************************************************/

template <class Element>
inline KPPriorityQueue<Element> &
KPPriorityQueue<Element>::operator=(const KPPriorityQueue<Element> &queue)
{
    my_list = queue.my_list;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPPriorityQueue<Element> &
KPPriorityQueue<Element>::operator=(const Element &element)
{
    my_list = element;
    return *this;
}

/****************************************************************************/

template <class Element>
KPPriorityQueue<Element> &
KPPriorityQueue<Element>::enqueue(const Element &element)
{
    KPIterator<Element> iter(my_list);

    iter.end();
    while (iter.ptr() && element < *iter)
        iter--;
    if (!iter.ptr())
        my_list.add_to_head(element);
    else
        iter.insert_after_current(element);

    return *this;
}

/****************************************************************************/

template <class Element>
inline Element
KPPriorityQueue<Element>::dequeue()
{
    if (my_list.size() < 1)
        queue_empty_error("dequeue()");
    
    Element element = my_list.head();
    my_list.remove_head();
    return element;
}

/****************************************************************************/

template <class Element>
inline Element &
KPPriorityQueue<Element>::head()
{
    if (my_list.size() < 1)
        queue_empty_error("head()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline const Element &
KPPriorityQueue<Element>::head() const
{
    if (my_list.size() < 1)
        queue_empty_error("head()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline Element &
KPPriorityQueue<Element>::tail()
{
    if (my_list.size() < 1)
        queue_empty_error("tail()");

    return my_list.tail();
}

/****************************************************************************/

template <class Element>
inline const Element &
KPPriorityQueue<Element>::tail() const
{
    if (my_list.size() < 1)
        queue_empty_error("tail()");

    return my_list.tail();
}

/****************************************************************************/

template <class Element>
inline int
KPPriorityQueue<Element>::size() const
{
    return my_list.size();
}

/****************************************************************************/

template <class Element>
inline bool
KPPriorityQueue<Element>::is_empty() const
{
    return my_list.is_empty();
}

/****************************************************************************/

template <class Element>
inline KPPriorityQueue<Element> &
KPPriorityQueue<Element>::clear()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
void
KPPriorityQueue<Element>::queue_empty_error(const char *fname)
{
    cerr << "KPPriorityQueue: " << fname << " - queue empty\n";
    exit(EXIT_FAILURE);
}

/****************************************************************************/

#endif /* KP_PRIORITY_QUEUE_DEFINED */
