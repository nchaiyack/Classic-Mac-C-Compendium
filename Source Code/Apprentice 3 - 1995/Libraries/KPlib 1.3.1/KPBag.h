// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_BAG_DEFINED
#define KP_BAG_DEFINED

#include <stdlib.h>
#include "KPbasic.h"
#include "KPList.h"

// Assumes Element has a default constructor, operator=(), operator==(),
// and operator<().  Note that operator<() must place a total ordering on
// the Elements.

// KPBag<T>::operator<() places a total ordering on all KPBag<T> elements,
// so that KPSet<KPBag<T> > is possible.

template <class Element>
class KPBag {
    public:
        KPBag();
        KPBag(const KPBag<Element> &bag);
        KPBag(const KPList<Element> &list);
        KPBag(const Element &element, int count=1);
        ~KPBag();
        operator KPList<Element>() const;

        KPBag<Element> operator+(const KPBag<Element> &bag) const;
        KPBag<Element> operator+(const Element &element) const;
        KPBag<Element> &operator+=(const KPBag<Element> &bag);
        KPBag<Element> &operator+=(const Element &element);
        KPBag<Element> &add(const KPBag<Element> &bag);
        KPBag<Element> &add(const Element &element, int num=1);

        // It is illegal to call the following functions unless the elements
        // are there to be removed!  If unsure, call contains() first.
        KPBag<Element> operator-(const KPBag<Element> &bag) const;
        KPBag<Element> operator-(const Element &element) const;
        KPBag<Element> &operator-=(const KPBag<Element> &bag);
        KPBag<Element> &operator-=(const Element &element);
        KPBag<Element> &remove(const KPBag<Element> &bag);
        KPBag<Element> &remove(const Element &element, int num=1);
        KPBag<Element> &remove_all(const Element &element);

        // Miscellaneous
        bool operator==(const KPBag<Element> &bag) const;
        bool operator!=(const KPBag<Element> &bag) const;
        bool operator<(const KPBag<Element> &bag) const;
        KPBag<Element> &operator=(const KPBag<Element> &bag);
        KPBag<Element> &operator=(const Element &element);
        KPBag<Element> &operator=(const KPList<Element> &list);
        KPList<Element> list() const;
        KPList<Element> unique_list() const;
        int size() const;        // Total number of elements.
        int unique_size() const; // Total number of unique elements.
        bool is_empty() const;
        bool contains(const KPBag<Element> &bag) const;
        bool contains(const Element &element, int num=1) const;
        int occurrences_of(const Element &element) const;
        Element retrieve();  // Returns and removes an element.
        KPBag<Element> &clear();
    protected:
        class Slot {
            public:
                Element my_element;
                int my_count;
            public:
                Slot(): my_count(0)
                    { /* do nothing */ }
                Slot(const Slot &slot): my_count(slot.my_count)
                    {  my_element = slot.my_element; }
                Slot(const Element &element, int num): my_count(num)
                    { my_element = element; }
                void operator=(const Slot &slot)
                    { my_element = slot.my_element; my_count = slot.my_count; }
        };
        KPList<KPBag<Element>::Slot> my_list;
};

/****************************************************************************/

template <class Element>
inline
KPBag<Element>::KPBag(): my_list()
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPBag<Element>::KPBag(const KPBag<Element> &bag): my_list(bag.my_list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPBag<Element>::KPBag(const KPList<Element> &list)
{
    *this = list;
}

/****************************************************************************/

template <class Element>
inline
KPBag<Element>::KPBag(const Element &element, int count): my_list()
{
    if (count < 0) {
        cerr << "KPBag - negative count\n";
        exit(EXIT_FAILURE);
    }
    else if (count > 0) {
        KPBag<Element>::Slot slot(element, count);
        my_list = slot;
    }
}

/****************************************************************************/

template <class Element>
inline
KPBag<Element>::~KPBag()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
KPBag<Element>::operator KPList<Element>() const
{
    KPList<Element> list;

    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);
    FOREACH (iter)
        for (register int i=0; i<iter->my_count; iter++)
            list.add_to_tail(iter->my_element);

    return list;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element>
KPBag<Element>::operator+(const KPBag<Element> &bag) const
{
    KPBag<Element> new_bag(*this);
    new_bag += bag;
    return new_bag;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element>
KPBag<Element>::operator+(const Element &element) const
{
    KPBag<Element> new_bag(*this);
    new_bag += element;
    return new_bag;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator+=(const KPBag<Element> &bag)
{
    return add(bag);
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator+=(const Element &element)
{
    return add(element, 1);
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::add(const KPBag<Element> &bag)
{
    KPIterator<KPBag<Element>::Slot> a(my_list);
    KPReadOnlyIterator<KPBag<Element>::Slot> b(bag.my_list);

    while (b.ptr()) {
        while (a.ptr() && a->my_element < b->my_element)
            a++;
        if (!a.ptr()) {
            for (; b.ptr(); b++)
                my_list.add_to_tail(*b);
        }
        else {
            if (a->my_element == b->my_element) {
                a->my_count += b->my_count;
                a++;
            }
            else
                a.insert_before_current(*b);
            b++;
        }
    }

    return *this;
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::add(const Element &element, int num)
{
    if (num < 0) {
        cerr << "KPBag - can't add a negative number of something\n";
        exit(EXIT_FAILURE);
    }

    else if (num > 0) {
        KPIterator<KPBag<Element>::Slot> iter(my_list);

        // Start looking from the end of the list, in case the elements are
        // being added in alphabetical order.

        iter.end();
        while (iter.ptr() && element < iter->my_element)
            --iter;
        if (!iter.ptr()) {
            KPBag<Element>::Slot slot(element, num);
            my_list.add_to_head(slot);
        }
        else if (iter->my_element == element)
            iter->my_count += num;
        else {
            KPBag<Element>::Slot slot(element, num);
            iter.insert_after_current(slot);
        }
    }

    return *this;
}

/****************************************************************************/

template <class Element>
KPBag<Element>
KPBag<Element>::operator-(const KPBag<Element> &bag) const
{
    KPBag<Element> new_bag;

    KPReadOnlyIterator<KPBag<Element>::Slot> a(my_list), b(bag.my_list);

    FOREACH(b) {
        while (a.ptr() && a->my_element < b->my_element)
            a++;
        if (!a.ptr() || b->my_element < a->my_element ||
                        b->my_count > a->my_count) {
            cerr << "KPBag - error subtracting more than exists\n";
            exit(EXIT_FAILURE);
        }
        else if (b->my_count < a->my_count) {
            KPBag<Element>::Slot slot(a->my_element, a->my_count - b->my_count);
            new_bag.my_list.add_to_tail(slot);
        }
        a++;
    }

    return new_bag;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element>
KPBag<Element>::operator-(const Element &element) const
{
    KPBag<Element> new_bag(*this);
    return new_bag.remove(element, 1);
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator-=(const KPBag<Element> &bag)
{
    return remove(bag);
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator-=(const Element &element)
{
    return remove(element, 1);
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::remove(const KPBag &bag)
{
    if (this == &bag)
        my_list.clear();

    else {
        KPIterator<KPBag<Element>::Slot> a(my_list);
        KPReadOnlyIterator<KPBag<Element>::Slot> b(bag.my_list);

        FOREACH(b) {
            while (a.ptr() && a->my_element < b->my_element)
                a++;
            if (!a.ptr() || b->my_element < a->my_element ||
                            b->my_count > a->my_count) {
                cerr << "KPBag - error subtracting more than exists\n";
                exit(EXIT_FAILURE);
            }
            else if (b->my_count < a->my_count)
                a->my_count -= b->my_count;
            else
                a.remove_current();
            a++;
        }
    }

    return *this;
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::remove(const Element &element, int num)
{
    KPIterator<KPBag<Element>::Slot> iter(my_list);

    while (iter.ptr() && iter->my_element < element)
        iter++;

    if (!iter.ptr() || element < iter->my_element || num > iter->my_count) {
        cerr << "KPBag - error subtracting more than exists\n";
        exit(EXIT_FAILURE);
    }
    else if (num < iter->my_count)
        iter->my_count -= num;
    else
        iter.remove_current();

    return *this;
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::remove_all(const Element &element)
{
    KPIterator<KPBag<Element>::Slot> iter(my_list);

    while (iter.ptr() && iter->my_element < element)
        iter++;
    if (iter.ptr() && iter->my_element == element)
        iter.remove_current();

    return *this;
}

/****************************************************************************/

template <class Element>
bool
KPBag<Element>::operator==(const KPBag<Element> &bag) const
{
    if (this == &bag)
        return true;
    
    if (my_list.size() != bag.my_list.size())
        return false;

    KPReadOnlyIterator<KPBag<Element>::Slot> a(my_list), b(bag.my_list);
    while (a.ptr()) {
        if (!(a->my_count == b->my_count && a->my_element == b->my_element))
            return false;
        a++, b++;
    }

    return true;
}

/****************************************************************************/

template <class Element>
inline bool
KPBag<Element>::operator!=(const KPBag<Element> &bag) const
{
    return !operator==(bag);
}

/****************************************************************************/

template <class Element>
bool
KPBag<Element>::operator<(const KPBag<Element> &bag) const
{
    if (bag.my_list.size() < my_list.size())
        return false;
    else if (my_list.size() < bag.my_list.size())
        return true;
    else {
        KPReadOnlyIterator<KPBag<Element>::Slot> a(my_list), b(bag.my_list);
        while (a.ptr()) {
            if (b->my_count < a->my_count)
                return false;
            else if (a->my_count < b->my_count)
                return true;
            else if (b->my_element < a->my_element)
                return false;
            else if (a->my_element < b->my_element)
                return true;
            a++, b++;
        }
    }

    return false;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator=(const KPBag<Element> &bag)
{
    my_list = bag.my_list;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::operator=(const Element &element)
{
    KPBag<Element>::Slot slot(element, 1);
    my_list += slot;
    return *this;
}

/****************************************************************************/

template <class Element>
KPBag<Element> &
KPBag<Element>::operator=(const KPList<Element> &list)
{
    clear();
    KPReadOnlyIterator<Element> iter(list);
    FOREACH(iter)
        add(*iter);

    return *this;
}

/****************************************************************************/

template <class Element>
KPList<Element>
KPBag<Element>::list() const
{
    KPList<Element> list;

    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);
    FOREACH(iter)
        for (register int i=0; i<iter->my_count; i++)
            list.add_to_tail(iter->my_element);

    return list;
}

/****************************************************************************/

template <class Element>
KPList<Element>
KPBag<Element>::unique_list() const
{
    KPList<Element> list;

    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);
    FOREACH(iter)
        list.add_to_tail(iter->my_element);

    return list;
}

/****************************************************************************/

template <class Element>
int
KPBag<Element>::size() const
{
    int total_count = 0;
    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);
    FOREACH(iter)
        total_count += iter->my_count;
    return total_count;
}

/****************************************************************************/

template <class Element>
inline int
KPBag<Element>::unique_size() const
{
    return my_list.size();
}

/****************************************************************************/

template <class Element>
inline bool
KPBag<Element>::is_empty() const
{
    return my_list.is_empty();
}

/****************************************************************************/

template <class Element>
bool
KPBag<Element>::contains(const KPBag<Element> &bag) const
{
    if (this == &bag)
        return true;

    KPReadOnlyIterator<KPBag<Element>::Slot> a(my_list), b(bag.my_list);

    while (b.ptr()) {
        while (a.ptr() && a->my_element < b->my_element)
            a++;
        if (!(a.ptr() && a->my_element == b->my_element &&
                         a->my_count >= b->my_count))
            return false;
        a++;
        b++;
    }

    return true;
}

/****************************************************************************/

template <class Element>
bool
KPBag<Element>::contains(const Element &element, int num) const
{
    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);

    while (iter.ptr() && iter->my_element < element)
        iter++;
    return (iter.ptr() && iter->my_element == element && iter->my_count >= num);
}

/****************************************************************************/

template <class Element>
int
KPBag<Element>::occurrences_of(const Element &element) const
{
    KPReadOnlyIterator<KPBag<Element>::Slot> iter(my_list);

    while (iter.ptr() && iter->my_element < element)
        iter++;
    if (iter.ptr() && iter->my_element == element)
        return iter->my_count;
    else
        return 0;
}

/****************************************************************************/

template <class Element>
Element
KPBag<Element>::retrieve()
{
    if (my_list.is_empty()) {
        cerr << "KPBag: retreive() - bag empty\n";
        exit(EXIT_FAILURE);
    }

    Element element = my_list.head().my_element;
    if (--my_list.head().my_count == 0)
        my_list.remove_head();
    return element;
}

/****************************************************************************/

template <class Element>
inline KPBag<Element> &
KPBag<Element>::clear()
{
    my_list.clear();
    return *this;
}

/****************************************************************************/

#endif /* KP_BAG_DEFINED */
