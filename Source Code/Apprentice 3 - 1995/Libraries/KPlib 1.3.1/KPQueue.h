// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_QUEUE_DEFINED
#define KP_QUEUE_DEFINED

#include <stdlib.h>
#include "KPbasic.h"
#include "KPList.h"

// Assumes Element has a default constructor and operator=().

template <class Element>
class KPQueue {
    public:
        KPQueue();
        KPQueue(const KPQueue<Element> &queue);
        KPQueue(const KPList<Element> &list);
        KPQueue(const Element &element);
        ~KPQueue();
        operator KPList<Element>() const;
        KPQueue<Element> &operator=(const KPQueue<Element> &queue);
        KPQueue<Element> &operator=(const Element &element);
        KPQueue<Element> &enqueue(const Element &element);
        Element dequeue();
        Element &head();
        const Element &head() const;
        Element &tail();
        const Element &tail() const;
        int size() const;
        bool is_empty() const;
        KPQueue<Element> &clear();
    protected:
        static void queue_empty_error(const char *fname);
        KPList<Element> my_list;
};

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::KPQueue(): my_list()
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::KPQueue(const KPQueue<Element> &queue): my_list(queue.my_list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::KPQueue(const KPList<Element> &list): my_list(list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::KPQueue(const Element &element): my_list(element)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::~KPQueue()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
inline
KPQueue<Element>::operator KPList<Element>() const
{
    return my_list;
}

/****************************************************************************/

template <class Element>
inline KPQueue<Element> &
KPQueue<Element>::operator=(const KPQueue<Element> &queue)
{
    my_list = queue.my_list;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPQueue<Element> &
KPQueue<Element>::operator=(const Element &element)
{
    my_list = element;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPQueue<Element> &
KPQueue<Element>::enqueue(const Element &element)
{
    my_list.add_to_tail(element);
    return *this;
}

/****************************************************************************/

template <class Element>
inline Element
KPQueue<Element>::dequeue()
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
KPQueue<Element>::head()
{
    if (my_list.size() < 1)
        queue_empty_error("head()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline const Element &
KPQueue<Element>::head() const
{
    if (my_list.size() < 1)
        queue_empty_error("head()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline Element &
KPQueue<Element>::tail()
{
    if (my_list.size() < 1)
        queue_empty_error("tail()");

    return my_list.tail();
}

/****************************************************************************/

template <class Element>
inline const Element &
KPQueue<Element>::tail() const
{
    if (my_list.size() < 1)
        queue_empty_error("tail()");

    return my_list.tail();
}

/****************************************************************************/

template <class Element>
inline int
KPQueue<Element>::size() const
{
    return my_list.size();
}

/****************************************************************************/

template <class Element>
inline bool
KPQueue<Element>::is_empty() const
{
    return my_list.is_empty();
}

/****************************************************************************/

template <class Element>
inline KPQueue<Element> &
KPQueue<Element>::clear()
{
    my_list.clear();
	return *this;
}

/****************************************************************************/

template <class Element>
void
KPQueue<Element>::queue_empty_error(const char *fname)
{
    cerr << "KPQueue: " << fname << " - queue empty\n";
    exit(EXIT_FAILURE);
}

/****************************************************************************/

#endif /* KP_QUEUE_DEFINED */
