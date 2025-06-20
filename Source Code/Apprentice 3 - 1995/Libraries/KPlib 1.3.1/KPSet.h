// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_SET_DEFINED
#define KP_SET_DEFINED

#include <stdlib.h>
#include "KPbasic.h"
#include "KPList.h"

// Assumes Element has a default constructor, operator=(), operator==(),
// and operator<().  Note that operator<() must place a total ordering on
// the set of Elements.

// All union, intersection and difference operations are of order O(n).

template <class Element>
class KPSet {
    public:
        KPSet();
        KPSet(const KPSet<Element> &set);
        KPSet(const KPList<Element> &list);
        KPSet(const Element &element);
        ~KPSet();
        operator KPList<Element>() const;

        // Union
        KPSet<Element> operator+(const KPSet<Element> &set) const;
        KPSet<Element> operator+(const Element &element) const;
        KPSet<Element> &operator+=(const KPSet<Element> &set);
        KPSet<Element> &operator+=(const Element &element);
        KPSet<Element> &add(const KPSet<Element> &set);
        KPSet<Element> &add(const Element &element);

        // Intersection
        KPSet<Element> operator*(const KPSet<Element> &set) const;
        KPSet<Element> operator*(const Element &element) const;
        KPSet<Element> &operator*=(const KPSet<Element> &set);
        KPSet<Element> &operator*=(const Element &element);

        // Difference
        KPSet<Element> operator-(const KPSet<Element> &set) const;
        KPSet<Element> operator-(const Element &element) const;
        KPSet<Element> &operator-=(const KPSet<Element> &set);
        KPSet<Element> &operator-=(const Element &element);

        // Miscellaneous
        bool operator==(const KPSet<Element> &set) const;
        bool operator!=(const KPSet<Element> &set) const;
        bool operator<(const KPSet<Element> &set) const;
        KPSet<Element> &operator=(const KPSet<Element> &set);
        KPSet<Element> &operator=(const Element &element);
        KPSet<Element> &operator=(const KPList<Element> &list);
        const KPList<Element> &list() const;
        int size() const;
        bool is_empty() const;
        bool contains(const KPSet<Element> &set) const;
        bool contains(const Element &element) const;
        KPSet<Element> all_such_that(bool (*f)(const Element &)) const;
		Element retrieve();  // Returns and removes an element.
        KPSet<Element> &clear();
    protected:
        KPSortableList<Element> my_list;
};

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::KPSet(): my_list()
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::KPSet(const KPSet<Element> &set): my_list(set.my_list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::KPSet(const KPList<Element> &list)
{
    *this = list;
}

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::KPSet(const Element &element): my_list(element)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::~KPSet()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
inline
KPSet<Element>::operator KPList<Element>() const
{
    return my_list;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::operator+(const KPSet<Element> &set) const
{
    KPSet<Element> new_set(*this);
    new_set += set;
    return new_set;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::operator+(const Element &element) const
{
    KPSet<Element> new_set(*this);
    new_set += element;
    return new_set;
}

/****************************************************************************/

template <class Element>
KPSet<Element> &
KPSet<Element>::operator+=(const KPSet<Element> &set)
{
    if (this == &set)
        return *this;

    KPIterator<Element> a(my_list);
    KPReadOnlyIterator<Element> b(set.my_list);

    while (b.ptr()) {
        while (a.ptr() && *a < *b)
            a++;
        if (!a.ptr()) {
            for (; b.ptr(); b++)
                my_list.add_to_tail(*b);
        }
        else {
            if (*a == *b)
                a++;
            else
                a.insert_before_current(*b);
            b++;
        }
    }

    return *this;
}

/****************************************************************************/

template <class Element>
KPSet<Element> &
KPSet<Element>::operator+=(const Element &element)
{
    KPIterator<Element> iter(my_list);

    // Start looking from the end of the list, in case the elements are
    // being added in alphabetical order.

    iter.end();
    while (iter.ptr() && element < *iter)
        --iter;
    if (!iter.ptr())
        my_list.add_to_head(element);
    else if (!(*iter == element))
        iter.insert_after_current(element);

    return *this;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::add(const KPSet<Element> &set)
{
    return *this += set;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::add(const Element &element)
{
    return *this += element;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::operator*(const KPSet<Element> &set) const
{
    if (this == &set)
        return *this;

    KPSet<Element> new_set;
    KPReadOnlyIterator<Element> a(my_list), b(set.my_list);

    while (b.ptr()) {
        while (a.ptr() && *a < *b)
            a++;
        if (!a.ptr()) {
            b.end();
            b++;
        }
        else {
            if (*a == *b) {
                new_set.my_list.add_to_tail(*a);
                a++;
            }
            b++;
        }
    }
    return new_set;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::operator*(const Element &element) const
{
    KPSet<Element> new_set;
    if (contains(element))
        new_set.my_list = element;
    return new_set;
}

/****************************************************************************/

template <class Element>
KPSet<Element> &
KPSet<Element>::operator*=(const KPSet<Element> &set)
{
    if (this == &set)
        return *this;

    KPIterator<Element> a(my_list);
    KPReadOnlyIterator<Element> b(set.my_list);

    while (b.ptr()) {
        while (a.ptr() && *a < *b)
            a++;
        if (!a.ptr()) {
            b.end();
            b++;
        }
        else {
            if (!(*a == *b)) {
                a.remove_current();
                a++;
            }
            b++;
        }
    }
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::operator*=(const Element &element)
{
    if (contains(element))
        my_list = element;
    else
        my_list.clear();

    return *this;
}

/****************************************************************************/

template <class Element>
KPSet<Element>
KPSet<Element>::operator-(const KPSet<Element> &set) const
{
    KPSet<Element> new_set;

    if (this == &set)
        return new_set;

    KPReadOnlyIterator<Element> a(my_list), b(set.my_list);

    while (a.ptr()) {
        while (b.ptr() && *b < *a)
            b++;
        if (!b.ptr()) {
            a.end();
            a++;
        }
        else {
            if (!(*a == *b))
                new_set.my_list.add_to_tail(*a);
            else
                b++;
            a++;
        }
    }
    return new_set;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::operator-(const Element &element) const
{
    KPSet<Element> new_set(*this);
    new_set -= element;
    return new_set;
}

/****************************************************************************/

template <class Element>
KPSet<Element> &
KPSet<Element>::operator-=(const KPSet<Element> &set)
{
    if (this == &set) {
        my_list.clear();
        return *this;
    }

    KPIterator<Element> a(my_list);
    KPReadOnlyIterator<Element> b(set.my_list);

    while (b.ptr()) {
        while (a.ptr() && *a < *b)
            a++;
        if (!a.ptr()) {
            b.end();
            b++;
        }
        else {
            if (*a == *b) {
                a.remove_current();
                a++;
            }
            b++;
        }
    }
    return *this;
}

/****************************************************************************/

template <class Element>
KPSet<Element> &
KPSet<Element>::operator-=(const Element &element)
{
    KPIterator<Element> iter(my_list);

    while (iter.ptr() && *iter < element)
        iter++;
    if (iter.ptr() && *iter == element)
        iter.remove_current();

    return *this;
}

/****************************************************************************/

template <class Element>
inline bool
KPSet<Element>::operator==(const KPSet<Element> &set) const
{
    return my_list == set.my_list;
}

/****************************************************************************/

template <class Element>
inline bool
KPSet<Element>::operator!=(const KPSet<Element> &set) const
{
    return my_list != set.my_list;
}

/****************************************************************************/

template <class Element>
inline bool
KPSet<Element>::operator<(const KPSet<Element> &set) const
{
    return my_list < set.my_list;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::operator=(const KPSet<Element> &set)
{
    my_list = set.my_list;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::operator=(const Element &element)
{
    my_list = element;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::operator=(const KPList<Element> &list)
{
    my_list = list;
    if (my_list.size() > 1) {
        my_list.sort();
        KPIterator<Element> iter(my_list); 
        register const Element *prev = iter.ptr();
        for (iter++; iter.ptr(); iter++)
            if (*iter == *prev)
                iter.remove_current();
            else
                prev = iter.ptr();
    }
    return *this;
}

/****************************************************************************/

template <class Element>
inline const KPList<Element> &
KPSet<Element>::list() const
{
    return my_list;
}

/****************************************************************************/

template <class Element>
inline int
KPSet<Element>::size() const
{
    return my_list.size();
}

/****************************************************************************/

template <class Element>
inline bool
KPSet<Element>::is_empty() const
{
    return my_list.is_empty();
}

/****************************************************************************/

template <class Element>
bool
KPSet<Element>::contains(const KPSet<Element> &set) const
{
    if (this == &set)
        return true;

    KPReadOnlyIterator<Element> a(my_list), b(set.my_list);

    while (b.ptr()) {
        while (a.ptr() && *a < *b)
            a++;
        if (!(a.ptr() && *a == *b))
            return false;
        a++;
        b++;
    }

    return true;
}

/****************************************************************************/

template <class Element>
bool
KPSet<Element>::contains(const Element &element) const
{
    KPReadOnlyIterator<Element> iter(my_list);

    while (iter.ptr() && *iter < element)
        iter++;
    return (iter.ptr() && *iter == element);
}

/****************************************************************************/

template <class Element>
inline KPSet<Element>
KPSet<Element>::all_such_that(bool (*f)(const Element &)) const
{
    KPSet<Element> new_set;
    new_set.my_list = my_list.all_such_that(f);
    return new_set;
}

/****************************************************************************/

template <class Element>
Element
KPSet<Element>::retrieve()
{
	if (my_list.is_empty()) {
		cerr << "KPSet: retreive() - set empty\n";
		exit(EXIT_FAILURE);
	}

	Element element = my_list.head();
	my_list.remove_head();
	return element;
}

/****************************************************************************/

template <class Element>
inline KPSet<Element> &
KPSet<Element>::clear()
{
    my_list.clear();
	return *this;
}

/****************************************************************************/

#endif /* KP_SET_DEFINED */
