// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_LIST_DEFINED
#define KP_LIST_DEFINED

#include <iostream.h>
#include <stdlib.h>
#include "KPbasic.h"

// To insure that KPList isn't defined before KPString, so that
// KPList<KPString> will be considered a complete type in the KPString.h
// header file.  This is probably a g++-specific problem.
#include "KPString.h"

// Assumes Element has a default constructor and operator=().

template <class Element>
class KPList {
        friend class KPReadOnlyIterator<Element>;
        friend class KPIterator<Element>;
    public:
        KPList();
        KPList(const KPList<Element> &list);
        KPList(const Element &element);
        virtual ~KPList();
        KPList<Element> operator+(const KPList<Element> &list);
        KPList<Element> operator+(const Element &element);
        void operator+=(const KPList<Element> &list);
        void operator+=(const Element &element);
        KPList<Element> &operator=(const KPList<Element> &list);
        KPList<Element> &operator=(const Element &element);
        Element &head();
        Element &tail();
        const Element &head() const;
        const Element &tail() const;
        void add_to_head(const Element &element);
        void add_to_tail(const Element &element);
        void remove_head();
        void remove_tail();
        Element &operator[](int index);
        const Element &operator[](int index) const;
        int size() const;
        bool is_empty() const;
        void clear();
        KPList<Element> all_such_that(bool (*f)(const Element &)) const;

    protected:
        struct Node {
            Element element;
            Node *previous;
            Node *next;
        };
        virtual void error(const char *msg) const;
        int my_size;
        unsigned int my_iterator_count;
        Node *my_head;
        Node *my_tail;
};

// Assumes Element has the above plus operator==().

template <class Element>
class KPComparableList: public KPList<Element> {
    public:
        KPComparableList();
        KPComparableList(const KPComparableList<Element> &list);
        KPComparableList(const KPList<Element> &list);
        KPComparableList(const Element &element);
        virtual ~KPComparableList();
        KPComparableList<Element> &operator=(const KPList<Element> &list);
        KPComparableList<Element> &operator=(const Element &element);
        KPComparableList<Element> operator+(const KPList<Element> &list);
        KPComparableList<Element> operator+(const Element &element);
        KPComparableList<Element>
                            operator-(const KPComparableList<Element> &list);
        KPComparableList<Element> operator-(const Element &element);
        void operator-=(const KPComparableList<Element> &list);
        void operator-=(const Element &element);
        bool operator==(const KPComparableList<Element> &list) const;
        bool operator!=(const KPComparableList<Element> &list) const;
        bool contains(const KPComparableList<Element> &list) const;
        bool contains(const Element &element) const;
        int occurrences_of(const Element &element) const;
        void clear();
        void clear(const Element &element);
        void clear(const KPComparableList<Element> &list);
    protected:
        virtual void error(const char *msg) const;
};

// Assumes Element has the above plus operator<().  Note that this operator
// must place a total ordering on the set of Elements.

template <class Element>
class KPSortableList: public KPComparableList<Element> {
    public:
        KPSortableList();
        KPSortableList(const KPSortableList<Element> &list);
        KPSortableList(const KPList<Element> &list);
        KPSortableList(const Element &element);
        virtual ~KPSortableList();
        KPSortableList<Element> &operator=(const KPList<Element> &list);
        KPSortableList<Element> &operator=(const Element &element);
        bool operator<(const KPSortableList<Element> &list) const;
        void sort();
        Element &min();
        const Element &min() const;
        Element &max();
        const Element &max() const;
    protected:
        virtual void error(const char *msg) const;
        static int findpivot(int i, int j, Element **elementlist);
        static int partition(int l, int r, const Element &pivot,
                                                    Element **elementlist);
        static void quicksort(int i, int j, Element **elementlist);
};

// A list keeps track of the number of iterators iterating over it.  Rules:
//
//    - If a list has no iterators, one can add to and remove from that
//      list without restriction.
//
//    - If a list has one iterator, one can add to and remove elements from
//      the list through the iterator without restriction, although one
//      cannot remove elements from the list directly.
//
//    - If a list has more than one iterator, one cannot remove elements
//      from the list at all.
//
// As a result, if a list goes out of scope while there is still an
// iterator attached to it, this is an error.  Usually the declaration of
// an iterator appears after the declaration of the list it is to iterate
// over (or it is in a more local scope), so this is usually not a problem
// (since it's destructor is called first, detaching it implicitly).  To
// detach an iterator from a list explicitly, call iterate_over().

// Use KPReadOnlyIterator if you wish to iterate over a const KPList or if you
// do not intend to modify the list through the iterator.

template <class Element>
class KPReadOnlyIterator {
    public:
        KPReadOnlyIterator();
        KPReadOnlyIterator(const KPList<Element> &list);
        KPReadOnlyIterator(const KPReadOnlyIterator<Element> &iterator);
        ~KPReadOnlyIterator();
        KPReadOnlyIterator<Element> &iterate_over(const KPList<Element> &list);
        KPReadOnlyIterator<Element> &iterate_over();
        KPReadOnlyIterator<Element> &
                        operator=(const KPReadOnlyIterator<Element> &iterator);
        const Element &current() const;
        const Element *ptr() const;
        KPReadOnlyIterator<Element> &beginning();
        KPReadOnlyIterator<Element> &end();
        KPReadOnlyIterator<Element> &operator++();  // Prefix
        KPReadOnlyIterator<Element> &operator--();  // Prefix
        void operator++(int);                       // Postfix
        void operator--(int);                       // Postfix
        const Element &operator*() const;
        const Element *operator->() const;
        bool at_beginning() const;
        bool at_end() const;
        int size() const;
        bool is_empty() const;
        const KPList<Element> &list() const;
    protected:
        static void error(const char *msg);
        const KPList<Element> *my_list;
        const KPList<Element>::Node *my_current;
};

// The rules defining what happens when odd combinations of element
// removals and insertions are performed are fairly intuitive.  An element
// can be deleted while the list is being parsed, and the parse can
// continue.  "Previous" and "next" retain their meanings.  Just don't try
// to access an element after it has been deleted!

template <class Element>
class KPIterator {
    public:
        KPIterator();
        KPIterator(KPList<Element> &list);
        KPIterator(const KPIterator<Element> &iterator);
        ~KPIterator();
        KPIterator<Element> &iterate_over(KPList<Element> &list);
        KPIterator<Element> &iterate_over();
        KPIterator<Element> &operator=(const KPIterator<Element> &iterator);
        KPIterator<Element> &insert_before_current(const Element &element);
        KPIterator<Element> &insert_after_current(const Element &element);
        KPIterator<Element> &replace_current_with(const Element &element);
        Element &current();
        Element *KPIterator::ptr();
        void remove_current();
        KPIterator<Element> &beginning();
        KPIterator<Element> &end();
        KPIterator<Element> &operator++();  // Prefix
        KPIterator<Element> &operator--();  // Prefix
        void operator++(int);               // Postfix
        void operator--(int);               // Postfix
        Element &operator*();
        Element *operator->();
        bool at_beginning() const;
        bool at_end() const;
        int size() const;
        bool is_empty() const;
        KPList<Element> &list();
    protected:
        static void error(const char *msg);
        KPList<Element> *my_list;
        KPList<Element>::Node my_deleted;
        KPList<Element>::Node *my_current;
};

// The following macro is defined as a convenience.  Here is an example of
// its usage:
//
//     KPSortableList<int> intlist;
//     intlist += 42;
//     intlist += 11;
//     intlist += 76;
//     intlist += 9;
//     intlist.sort();
//
//     KPReadOnlyIterator<int> iter(intlist);
//     FOREACH(iter) cout << *iter << '\n';

#define FOREACH(iter) for (iter.beginning(); iter.ptr(); iter++)

/****************************************************************************/

template <class Element>
inline
KPList<Element>::KPList()
{
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
}

/****************************************************************************/

template <class Element>
inline
KPList<Element>::KPList(const KPList<Element> &list)
{
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
    *this = list;
}

/****************************************************************************/

template <class Element>
inline
KPList<Element>::KPList(const Element &element)
{
    my_size = my_iterator_count = 0;
    my_head = my_tail = NULL;
    *this = element;
}

/****************************************************************************/

template <class Element>
inline
KPList<Element>::~KPList()
{
    if (my_iterator_count > 0)
        error("~KPList() - cannot destroy, iterators present");

    clear();
}

/****************************************************************************/

template <class Element>
inline KPList<Element>
KPList<Element>::operator+(const KPList<Element> &list)
{
    KPList<Element> new_list(*this);
    new_list += list;
    return new_list;
}

/****************************************************************************/

template <class Element>
inline KPList<Element>
KPList<Element>::operator+(const Element &element)
{
    KPList<Element> new_list(*this);
    new_list += element;
    return new_list;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::operator+=(const KPList<Element> &list)
{
    register const Node *node;
    const int size = list.my_size;
    int i;

    // Must use size as stopping condition in case *this == list.
    for (node = list.my_head, i=0; i < size; node = node->next, i++)
        *this += node->element;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::operator+=(const Element &element)
{
    Node *newnode = new Node;
    check_mem(newnode);
    newnode->next = NULL;
    newnode->element = element;
    if (my_size++ == 0) {
        my_head = newnode;
        newnode->previous = NULL;
    }
    else {
        my_tail->next = newnode;
        newnode->previous = my_tail;
    }
    my_tail = newnode;
}

/****************************************************************************/

template <class Element>
KPList<Element> &
KPList<Element>::operator=(const KPList<Element> &list)
{
    if (this == &list)
        return *this;

    if (my_iterator_count > 0)
        error("operator=() - cannot reassign, iterators present");

    register Node *node;
    Node *newnode, *prevnode;

    clear();

    if (!(node = list.my_head))
        return *this;

    newnode = new Node;
    check_mem(newnode);
    newnode->previous = NULL;
    newnode->element = node->element;
    my_head = prevnode = newnode;

    for (node = node->next; node; node = node->next) {
        newnode = new Node;
        check_mem(newnode);
        newnode->element = node->element;
        prevnode->next = newnode;
        newnode->previous = prevnode;
        prevnode = newnode;
    }
    newnode->next = NULL;
    my_tail = newnode;
    my_size = list.my_size;

    return *this;
}

/****************************************************************************/

template <class Element>
KPList<Element> &
KPList<Element>::operator=(const Element &element)
{
    if (my_iterator_count > 0)
        error("operator=() - cannot reassign, iterators present");

    clear();

    Node *newnode = new Node;
    check_mem(newnode);
    newnode->element = element;
    newnode->previous = newnode->next = NULL;
    my_head = my_tail = newnode;
    my_size = 1;

    return *this;
}

/****************************************************************************/

template <class Element>
inline Element &
KPList<Element>::head()
{
    if (!my_head)
        error("head() - list is empty");

    return my_head->element;
}

/****************************************************************************/

template <class Element>
inline Element &
KPList<Element>::tail()
{
    if (!my_tail)
        error("tail() - list is empty");

    return my_tail->element;
}

/****************************************************************************/

template <class Element>
inline const Element &
KPList<Element>::head() const
{
    if (!my_head)
        error("head() - list is empty");

    return my_head->element;
}

/****************************************************************************/

template <class Element>
inline const Element &
KPList<Element>::tail() const
{
    if (!my_tail)
        error("tail() - list is empty");

    return my_tail->element;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::add_to_head(const Element &element)
{
    Node *newnode = new Node;
    check_mem(newnode);
    newnode->element = element;
    newnode->previous = NULL;
    newnode->next = my_head;

    if (my_size++)
        my_head->previous = newnode;
    else
        my_tail = newnode;
    my_head = newnode;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::add_to_tail(const Element &element)
{
    Node *newnode = new Node;
    check_mem(newnode);
    newnode->element = element;
    newnode->previous = my_tail;
    newnode->next = NULL;

    if (my_size++)
        my_tail->next = newnode;
    else
        my_head = newnode;
    my_tail = newnode;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::remove_head()
{
    Node *old_head;

    if (my_iterator_count > 0)
        error("remove_head() - cannot remove element, iterators present");

    if (old_head = my_head) {
        if (old_head->next) {
            old_head->next->previous = NULL;
            my_head = old_head->next;
        }
        else
            my_head = my_tail = NULL;

        delete old_head;
        my_size--;
    }
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::remove_tail()
{
    Node *old_tail;

    if (my_iterator_count > 0)
        error("remove_tail() - cannot remove element, iterators present");

    if (old_tail = my_tail) {
        if (old_tail->previous) {
            old_tail->previous->next = NULL;
            my_tail = old_tail->previous;
        }
        else
            my_head = my_tail = NULL;

        delete old_tail;
        my_size--;
    }
}

/****************************************************************************/

template <class Element>
Element &
KPList<Element>::operator[](int index)
{
    if (index < 0 || index >= my_size)
        error("operator[] - invalid index");

    register Node *node = my_head;
    for (register int i=0; i<index; i++)
        node = node->next;

    return node->element;
}

/****************************************************************************/

template <class Element>
const Element &
KPList<Element>::operator[](int index) const
{
    if (index < 0 || index >= my_size)
        error("operator[] - invalid index");

    register Node *node = my_head;
    for (register int i=0; i<index; i++)
        node = node->next;

    return node->element;
}

/****************************************************************************/

template <class Element>
inline int
KPList<Element>::size() const
{
    return my_size;
}

/****************************************************************************/

template <class Element>
inline bool
KPList<Element>::is_empty() const
{
    return (my_size == 0);
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::clear()
{
    register Node *nextnode;

    if (my_iterator_count > 0)
        error("clear() - cannot clear, iterators present");

    for (register Node *node = my_head; node; node = nextnode) {
        nextnode = node->next;
        delete node;
    }
    my_size = 0;
    my_head = my_tail = NULL;
}

/****************************************************************************/

template <class Element>
KPList<Element>
KPList<Element>::all_such_that(bool (*f)(const Element &)) const
{
    KPList<Element> new_list;

    for (register const Node *node = my_head; node; node = node->next)
        if (f(node->element))
            new_list += node->element;

    return new_list;
}

/****************************************************************************/

template <class Element>
void
KPList<Element>::error(const char *msg) const
{
    cerr << "KPList: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>::KPComparableList(): KPList<Element>()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>::KPComparableList(
                const KPComparableList<Element> &list): KPList<Element>(list)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>::KPComparableList(const KPList<Element> &list):
                                                        KPList<Element>(list)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableList<Element>::KPComparableList(const Element &element):
                                                    KPList<Element>(element)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>::~KPComparableList()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPComparableList<Element> &
KPComparableList<Element>::operator=(const KPList<Element> &list)
{
    KPList<Element>::operator=(list);
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element> &
KPComparableList<Element>::operator=(const Element &element)
{
    KPList<Element>::operator=(element);
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>
KPComparableList<Element>::operator+(const KPList<Element> &list)
{
    KPComparableList<Element> new_list(*this);
    new_list += list;
    return new_list;
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>
KPComparableList<Element>::operator+(const Element &element)
{
    KPComparableList<Element> new_list(*this);
    new_list += element;
    return new_list;
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>
KPComparableList<Element>::operator-(const KPComparableList<Element> &list)
{
    KPComparableList<Element> new_list(*this);
    new_list -= list;
    return new_list;
}

/****************************************************************************/

template <class Element>
inline KPComparableList<Element>
KPComparableList<Element>::operator-(const Element &element)
{
    KPComparableList<Element> new_list(*this);
    new_list -= element;
    return new_list;
}

/****************************************************************************/

template <class Element>
void
KPComparableList<Element>::operator-=(const KPComparableList<Element> &list)
{
    for (register const Node *node = list.my_head; node; node = node->next)
        *this -= node->element;
}

/****************************************************************************/

template <class Element>
void
KPComparableList<Element>::operator-=(const Element &element)
{
    if (my_iterator_count > 0)
        error("operator-=() - cannot remove elements, iterators present");

    for (register Node *node = my_tail; node; node = node->previous)
        if (node->element == element) {
            if (node->previous)
                node->previous->next = node->next;
            else
                my_head = node->next;
            if (node->next)
                node->next->previous = node->previous;
            else
                my_tail = node->previous;
            delete node;
            my_size--;
            break;
        }
}

/****************************************************************************/

template <class Element>
bool
KPComparableList<Element>::operator==(const KPComparableList<Element> &list)
                                                                        const
{
    if (this == &list)
        return true;
    
    if (my_size != list.my_size)
        return false;

    register const Node *node1, *node2;
    for (node1 = my_head, node2 = list.my_head; node1;
                                    node1 = node1->next, node2 = node2->next)
        if (!(node1->element == node2->element))
            return false;
    
    return true;
}

/****************************************************************************/

template <class Element>
inline bool
KPComparableList<Element>::operator!=(const KPComparableList<Element> &list)
                                                                        const
{
    return !(*this == list);
}

/****************************************************************************/

template <class Element>
bool
KPComparableList<Element>::contains(const KPComparableList<Element> &list) const
{
    for (register const Node *node = list.my_head; node; node = node->next)
        if (!contains(node->element))
            return false;

    return true;
}

/****************************************************************************/

template <class Element>
bool
KPComparableList<Element>::contains(const Element &element) const
{
    for (register const Node *node = my_head; node; node = node->next)
        if (node->element == element)
            return true;

    return false;
}

/****************************************************************************/

template <class Element>
int
KPComparableList<Element>::occurrences_of(const Element &element) const
{
    int occurrences = 0;

    for (register const Node *node = my_head; node; node = node->next)
        if (node->element == element)
            occurrences++;

    return occurrences;
}

/****************************************************************************/

// I don't know why I have to redeclare clear() here.  I think it's another
// one of those infamous g++ bugs.

template <class Element>
void
KPComparableList<Element>::clear()
{
    KPList<Element>::clear();
}

/****************************************************************************/

template <class Element>
void
KPComparableList<Element>::clear(const Element &element)
{
    register Node *prevnode;

    if (my_iterator_count > 0)
        error("clear() - cannot remove elements, iterators present");

    for (register Node *node = my_tail; node; node = prevnode) {
        prevnode = node->previous;
        if (node->element == element) {
            if (node->previous)
                node->previous->next = node->next;
            else
                my_head = node->next;
            if (node->next)
                node->next->previous = node->previous;
            else
                my_tail = node->previous;
            delete node;
            my_size--;
        }
    }
}

/****************************************************************************/

template <class Element>
void
KPComparableList<Element>::clear(const KPComparableList<Element> &list)
{
    if (my_iterator_count > 0)
        error("clear() - cannot remove elements, iterators present");

    for (register Node *node = list.my_head; node; node = node->next)
        clear(node->element);
}

/****************************************************************************/

template <class Element>
void
KPComparableList<Element>::error(const char *msg) const
{
    cerr << "KPComparableList: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline KPSortableList<Element>::KPSortableList(): KPComparableList<Element>()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPSortableList<Element>::KPSortableList(const KPSortableList<Element> &list):
                                                KPComparableList<Element>(list)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPSortableList<Element>::KPSortableList(const KPList<Element> &list):
                                                KPComparableList<Element>(list)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableList<Element>::KPSortableList(const Element &element):
                                            KPComparableList<Element>(element)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPSortableList<Element>::~KPSortableList()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPSortableList<Element> &
KPSortableList<Element>::operator=(const KPList<Element> &list)
{
    KPComparableList<Element>::operator=(list);
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPSortableList<Element> &
KPSortableList<Element>::operator=(const Element &element)
{
    KPComparableList<Element>::operator=(element);
    return *this;
}

/****************************************************************************/

template <class Element>
bool
KPSortableList<Element>::operator<(const KPSortableList<Element> &list) const
{
    if (my_size < list.my_size)
        return true;
    
    if (my_size > list.my_size || this == &list)
        return false;

    register const Node *node1, *node2;
    for (node1 = my_head, node2 = list.my_head; node1;
                                    node1 = node1->next, node2 = node2->next)
        if (node1->element < node2->element)
            return true;
        else if (node2->element < node1->element)
            return false;

    return false;
}

/****************************************************************************/

template <class Element>
int
KPSortableList<Element>::findpivot(int i, int j, Element **elementlist)
{
    register const Element *first = elementlist[i];
    for (register int k=i+1; k<=j; k++) {
        if (*first < *elementlist[k])
            return k;
        else if (*elementlist[k] < *first)
            return i;
    }
    return -1;
}

template <class Element>
int
KPSortableList<Element>::partition(int l, int r, const Element &pivot,
                                                    Element **elementlist)
{
    do {
        swap(*elementlist[l], *elementlist[r]);
        while (*elementlist[l] < pivot)
            l++;
        while (!(*elementlist[r] < pivot))
            r--;
    } while (l <= r);
    return l;
}

template <class Element>
void
KPSortableList<Element>::quicksort(int i, int j, Element **elementlist)
{
    Element pivot;
    int k, index;

    index = findpivot(i, j, elementlist);
    if (index != -1) {
        pivot = *elementlist[index];
        k = partition(i, j, pivot, elementlist);
        quicksort(i, k-1, elementlist);
        quicksort(k, j, elementlist);
    }
}

template <class Element>
void
KPSortableList<Element>::sort()
{
    register int i;
    register Node *node;
    register Element **elementlist = new Element *[my_size];
    
    for (node = my_head, i=0; node; node = node->next, i++)
        elementlist[i] = &node->element;

    quicksort(0, my_size-1, elementlist);

    delete [] elementlist;
}

/****************************************************************************/

template <class Element>
Element &
KPSortableList<Element>::min()
{
    if (my_size < 1)
        error("min() - empty list");

    register Element *min = &my_head->element;
    for (register Node *node = my_head->next; node; node = node->next)
        if (node->element < *min)
            min = &node->element;

    return *min;
}

/****************************************************************************/

template <class Element>
const Element &
KPSortableList<Element>::min() const
{
    if (my_size < 1)
        error("min() - empty list");

    register const Element *min = &my_head->element;
    for (register const Node *node = my_head->next; node; node = node->next)
        if (node->element < *min)
            min = &node->element;

    return *min;
}

/****************************************************************************/

template <class Element>
Element &
KPSortableList<Element>::max()
{
    if (my_size < 1)
        error("max() - empty list");

    register Element *max = &my_head->element;
    for (register Node *node = my_head->next; node; node = node->next)
        if (*max < node->element)
            max = &node->element;

    return *max;
}

/****************************************************************************/

template <class Element>
const Element &
KPSortableList<Element>::max() const
{
    if (my_size < 1)
        error("max() - empty list");

    register const Element *max = &my_head->element;
    for (register const Node *node = my_head->next; node; node = node->next)
        if (*max < node->element)
            max = &node->element;

    return *max;
}

/****************************************************************************/

template <class Element>
void
KPSortableList<Element>::error(const char *msg) const
{
    cerr << "KPSortableList: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline
KPReadOnlyIterator<Element>::KPReadOnlyIterator()
{
    my_list = NULL;
    my_current = NULL;
}

/****************************************************************************/

template <class Element>
inline
KPReadOnlyIterator<Element>::KPReadOnlyIterator(const KPList<Element> &list)
{
    my_list = &list;
    // Even though list is a const, update its iterator count.
    ((KPList<Element> &)list).my_iterator_count++;
    my_current = list.my_head;
}

/****************************************************************************/

template <class Element>
inline
KPReadOnlyIterator<Element>::KPReadOnlyIterator(
                                const KPReadOnlyIterator<Element> &iterator)
{
    if (my_list = iterator.my_list)
        ((KPList<Element> *)my_list)->my_iterator_count++;
    my_current = iterator.my_current;
}

/****************************************************************************/

template <class Element>
inline
KPReadOnlyIterator<Element>::~KPReadOnlyIterator()
{
    if (my_list)
        ((KPList<Element> *)my_list)->my_iterator_count--;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::iterate_over(const KPList<Element> &list)
{
    if (my_list)
        ((KPList<Element> *)my_list)->my_iterator_count--;
    my_list = &list;
    ((KPList<Element> *)my_list)->my_iterator_count++;
    my_current = my_list->my_head;

    return *this;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::iterate_over()
{
    if (my_list) {
        ((KPList<Element> *)my_list)->my_iterator_count--;
        my_list = NULL;
        my_current = NULL;
    }

    return *this;
}

/****************************************************************************/


template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::operator=(const KPReadOnlyIterator<Element>
                                                                    &iterator)
{
    if (my_list)
        ((KPList<Element> *)my_list)->my_iterator_count--;
    if (my_list = iterator.my_list)
        ((KPList<Element> *)my_list)->my_iterator_count++;
    my_current = iterator.my_current;

    return *this;
}

/****************************************************************************/

template <class Element>
inline const Element &
KPReadOnlyIterator<Element>::current() const
{
    if(!my_current)
        error("current() - no current element");
    return my_current->element;
}


/****************************************************************************/

template <class Element>
inline const Element *
KPReadOnlyIterator<Element>::ptr() const
{
    if (my_current)
        return &my_current->element;
    else
        return NULL;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::beginning()
{
    if (my_list)
        my_current = my_list->my_head;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::end()
{
    if (my_list)
        my_current = my_list->my_tail;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::operator++()  // Prefix
{
    if (!my_current)
        error("operator++() - no next element");

    my_current = my_current->next;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPReadOnlyIterator<Element> &
KPReadOnlyIterator<Element>::operator--()  // Prefix
{
    if (!my_current)
        error("operator--() - no previous element");

    my_current = my_current->previous;
    return *this;
}

/****************************************************************************/

template <class Element>
inline void
KPReadOnlyIterator<Element>::operator++(int)  // Postfix
{
    if (!my_current)
        error("operator++() - no next element");

    my_current = my_current->next;
}

/****************************************************************************/

template <class Element>
inline void
KPReadOnlyIterator<Element>::operator--(int)  // Postfix
{
    if (!my_current)
        error("operator--() - no previous element");

    my_current = my_current->previous;
}

/****************************************************************************/

template <class Element>
inline const Element &
KPReadOnlyIterator<Element>::operator*() const
{
    if (!my_current)
        error("operator*() - no current element");

    return my_current->element;
}

/****************************************************************************/

template <class Element>
inline const Element *
KPReadOnlyIterator<Element>::operator->() const
{
    if (!my_current)
        error("operator->() - no current element");

    return &my_current->element;
}

/****************************************************************************/

template <class Element>
inline bool
KPReadOnlyIterator<Element>::at_beginning() const
{
    return (my_current && !my_current->previous);
}

/****************************************************************************/

template <class Element>
inline bool
KPReadOnlyIterator<Element>::at_end() const
{
    return (my_current && !my_current->next);
}

/****************************************************************************/

template <class Element>
inline int
KPReadOnlyIterator<Element>::size() const
{
    return my_list->size();
}

/****************************************************************************/

template <class Element>
inline bool
KPReadOnlyIterator<Element>::is_empty() const
{
    return my_list->is_empty();
}

/****************************************************************************/

template <class Element>
inline const KPList<Element> &
KPReadOnlyIterator<Element>::list() const
{
    if (!my_list)
        error("list() - no list associated with iterator");

    return *my_list;
}

/****************************************************************************/

template <class Element>
void
KPReadOnlyIterator<Element>::error(const char *msg)
{
    cerr << "KPReadOnlyIterator: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline
KPIterator<Element>::KPIterator()
{
    my_list = NULL;
    my_current = NULL;
}

/****************************************************************************/

template <class Element>
inline
KPIterator<Element>::KPIterator(KPList<Element> &list)
{
    my_list = &list;
    list.my_iterator_count++;
    my_current = list.my_head;
}

/****************************************************************************/

template <class Element>
inline
KPIterator<Element>::KPIterator(const KPIterator<Element> &iterator)
{
    if (my_list = iterator.my_list)
        my_list->my_iterator_count++;
    my_current = iterator.my_current;
}

/****************************************************************************/

template <class Element>
inline
KPIterator<Element>::~KPIterator()
{
    if (my_list)
        my_list->my_iterator_count--;
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::iterate_over(KPList<Element> &list)
{
    if (my_list)
        my_list->my_iterator_count--;
    my_list = &list;
    my_list->my_iterator_count++;
    my_current = my_list->my_head;

    return *this;
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::iterate_over()
{
    if (my_list) {
        my_list->my_iterator_count--;
        my_list = NULL;
        my_current = NULL;
    }

    return *this;
}

/****************************************************************************/


template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::operator=(const KPIterator<Element> &iterator)
{
    if (my_list)
        my_list->my_iterator_count--;
    if (my_list = iterator.my_list)
        my_list->my_iterator_count++;
    my_current = iterator.my_current;

    return *this;
}

/****************************************************************************/

template <class Element>
KPIterator<Element> &
KPIterator<Element>::insert_before_current(const Element &element)
{
    if (!my_current)
        error("insert_before_current() - no current element");

    KPList<Element>::Node *newnode = new KPList<Element>::Node;
    check_mem(newnode);
    newnode->element = element;

    if (my_current->previous) {
        if (my_current == &my_deleted) {
            newnode->previous = my_current->previous;
            newnode->next = my_current->next;
            my_current->previous->next = newnode;
            if (my_current->next)
                my_current->next->previous = newnode;
            else
                my_list->my_tail = newnode;
            my_current->previous = newnode;
        }
        else {
            newnode->previous = my_current->previous;
            newnode->next = my_current;
            my_current->previous->next = newnode;
            my_current->previous = newnode;
        }
    }
    else {
        my_list->my_head->previous = newnode;
        my_current->previous = newnode; // in case my_current == my_deleted
        newnode->next = my_list->my_head;
        newnode->previous = NULL;
        my_list->my_head = newnode;
    }

    my_list->my_size++;
    return *this;
}

/****************************************************************************/

template <class Element>
KPIterator<Element> &
KPIterator<Element>::insert_after_current(const Element &element)
{
    if (!my_current)
        error("insert_after_current() - no current element");
    
    KPList<Element>::Node *newnode = new KPList<Element>::Node;
    check_mem(newnode);
    newnode->element = element;

    if (my_current->next) {
        if (my_current == &my_deleted) {
            newnode->next = my_current->next;
            newnode->previous = my_current->previous;
            my_current->next->previous = newnode;
            if (my_current->previous)
                my_current->previous->next = newnode;
            else
                my_list->my_head = newnode;
            my_current->next = newnode;
        }
        else {
            newnode->next = my_current->next;
            newnode->previous = my_current;
            my_current->next->previous = newnode;
            my_current->next = newnode;
        }
    }
    else {
        my_list->my_tail->next = newnode;
        my_current->next = newnode;  // in case my_current == my_deleted
        newnode->previous = my_list->my_tail;
        newnode->next = NULL;
        my_list->my_tail = newnode;
    }

    my_list->my_size++;
    return *this;
}

/****************************************************************************/

template <class Element>
KPIterator<Element> &
KPIterator<Element>::replace_current_with(const Element &element)
{
    if (!my_current)
        error("replace_current_with() - no current element");
    if (my_current == &my_deleted)
        error("replace_current_with() - current element has been deleted");
    my_current->element = element;

    return *this;
}

/****************************************************************************/

template <class Element>
inline Element &
KPIterator<Element>::current()
{
    if(!my_current)
        error("current() - no current element");
    return my_current->element;
}


/****************************************************************************/

template <class Element>
inline Element *
KPIterator<Element>::ptr()
{
    if (my_current)
        return &my_current->element;
    else
        return NULL;
}

/****************************************************************************/

template <class Element>
void
KPIterator<Element>::remove_current()
{
    if (!my_current)
        error("remove_current() - no current element");

    else if (my_current == &my_deleted)
        error("remove_current() - current element has already been removed");

    else if (my_list->my_iterator_count > 1)
        error("remove_current() - cannot remove element, "
              "more than one iterator");

    else {
        my_deleted.previous = my_current->previous;
        my_deleted.next = my_current->next;
        delete my_current;
        my_current = &my_deleted;

        if (my_deleted.previous)
            my_deleted.previous->next = my_deleted.next;
        else
            my_list->my_head = my_deleted.next;

        if (my_deleted.next)
            my_deleted.next->previous = my_deleted.previous;
        else
            my_list->my_tail = my_deleted.previous;

        my_list->my_size--;
    }
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::beginning()
{
    if (my_list)
        my_current = my_list->my_head;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::end()
{
    if (my_list)
        my_current = my_list->my_tail;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::operator++()  // Prefix
{
    if (!my_current)
        error("operator++() - no next element");

    my_current = my_current->next;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPIterator<Element> &
KPIterator<Element>::operator--()  // Prefix
{
    if (!my_current)
        error("operator--() - no previous element");

    my_current = my_current->previous;
    return *this;
}

/****************************************************************************/

template <class Element>
inline void
KPIterator<Element>::operator++(int)  // Postfix
{
    if (!my_current)
        error("operator++() - no next element");

    my_current = my_current->next;
}

/****************************************************************************/

template <class Element>
inline void
KPIterator<Element>::operator--(int)  // Postfix
{
    if (!my_current)
        error("operator--() - no previous element");

    my_current = my_current->previous;
}

/****************************************************************************/

template <class Element>
inline Element &
KPIterator<Element>::operator*()
{
    if (!my_current)
        error("operator*() - no current element");

    return my_current->element;
}

/****************************************************************************/

template <class Element>
inline Element *
KPIterator<Element>::operator->()
{
    if (!my_current)
        error("operator->() - no current element");

    return &my_current->element;
}

/****************************************************************************/

template <class Element>
inline bool
KPIterator<Element>::at_beginning() const
{
    return (my_current && !my_current->previous);
}

/****************************************************************************/

template <class Element>
inline bool
KPIterator<Element>::at_end() const
{
    return (my_current && !my_current->next);
}

/****************************************************************************/

template <class Element>
inline int
KPIterator<Element>::size() const
{
    return my_list->size();
}

/****************************************************************************/

template <class Element>
inline bool
KPIterator<Element>::is_empty() const
{
    return my_list->is_empty();
}

/****************************************************************************/

template <class Element>
inline KPList<Element> &
KPIterator<Element>::list()
{
    if (!my_list)
        error("list() - no list associated with iterator");

    return *my_list;
}

/****************************************************************************/

template <class Element>
void
KPIterator<Element>::error(const char *msg)
{
    cerr << "KPIterator: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

#endif /* KP_LIST_DEFINED */
