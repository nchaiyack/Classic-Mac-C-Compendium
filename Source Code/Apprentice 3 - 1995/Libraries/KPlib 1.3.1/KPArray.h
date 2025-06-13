// A module of KPLib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_ARRAY_DEFINED
#define KP_ARRAY_DEFINED

#include <iostream.h>
#include <stdlib.h>
#include "KPbasic.h"

// Assumes Element has a default constructor and operator=().

template <class Element>
class KPArray {
    public:
        KPArray();
        KPArray(int size);
        KPArray(int size, const Element &init_element);
        KPArray(const KPArray<Element> &array);
        virtual ~KPArray();
        KPArray<Element> &operator=(const KPArray<Element> &array);
        Element &operator[](int index);
        const Element &operator[](int index) const;
        int size() const;
        bool is_empty() const;
        void resize(int new_size);
        void resize(int new_size, const Element &init_element);
        void set_all_to(const Element &element);
    protected:
        virtual void error(const char *msg) const;
        int my_size;
        Element *my_elements;
};

// Assumes Element has the above plus operator==().

template <class Element>
class KPComparableArray: public KPArray<Element> {
    public:
        KPComparableArray();
        KPComparableArray(int size);
        KPComparableArray(int size, const Element &init_element);
        KPComparableArray(const KPComparableArray<Element> &array);
        KPComparableArray(const KPArray<Element> &array);
        virtual ~KPComparableArray();
        KPComparableArray<Element> &operator=(const KPArray<Element> &array);
        bool operator==(const KPComparableArray<Element> &array) const;
        bool operator!=(const KPComparableArray<Element> &array) const;
        bool contains(const Element &element) const;
        int occurrences_of(const Element &element) const;
        int index_of(const Element &element) const;
    protected:
        virtual void error(const char *msg) const;
};

// Assumes Element has the above plus operator<().  Note that this operator
// must place a total ordering on the set of Elements.

template <class Element>
class KPSortableArray: public KPComparableArray<Element> {
    public:
        KPSortableArray();
        KPSortableArray(int size);
        KPSortableArray(int size, const Element &init_element);
        KPSortableArray(const KPSortableArray<Element> &array);
        KPSortableArray(const KPArray<Element> &array);
        virtual ~KPSortableArray();
        KPSortableArray<Element> &operator=(const KPArray<Element> &array);
        void sort();
        Element &min();
        const Element &min() const;
        Element &max();
        const Element &max() const;
    protected:
        virtual void error(const char *msg) const;
        int findpivot(int i, int j);
        int partition(int l, int r, const Element &pivot);
        void quicksort(int i, int j);
};

/****************************************************************************/

template <class Element>
inline
KPArray<Element>::KPArray(): my_size(0)
{
    /* do nothing */
}

/****************************************************************************/

template <class Element>
KPArray<Element>::KPArray(int size)
{
    if (size < 0)
        error("KPArray() - size of KPArray must be non-negative");

    my_size = size;
    my_elements = new Element[size];
    check_mem(my_elements);
}

/****************************************************************************/

template <class Element>
KPArray<Element>::KPArray(int size, const Element &init_element)
{
    if (size < 0)
        error("KPArray() - size of KPArray must be non-negative");

    my_size = size;
    if (my_size > 0) {
        my_elements = new Element[size];
        check_mem(my_elements);
        for (int i=0; i<size; i++)
            my_elements[i] = init_element;
    }
}

/****************************************************************************/

template <class Element>
KPArray<Element>::KPArray(const KPArray<Element> &array)
{
    my_size = array.my_size;
    if (my_size > 0) {
        my_elements = new Element[my_size];
        check_mem(my_elements);
        for (int i=0; i<my_size; i++)
            my_elements[i] = array.my_elements[i];
    }
}

/****************************************************************************/

template <class Element>
inline
KPArray<Element>::~KPArray()
{
    if (my_size > 0)
        delete [] my_elements;
}

/****************************************************************************/

template <class Element>
KPArray<Element> &
KPArray<Element>::operator=(const KPArray<Element> &array)
{
    if (this == &array)
        return *this;

    if (my_size > 0)
        delete [] my_elements;

    my_size = array.my_size;
    if (my_size > 0) {
        my_elements = new Element[my_size];
        check_mem(my_elements);
        for (int i=0; i<my_size; i++)
            my_elements[i] = array.my_elements[i];
    }
    return *this;
}

/****************************************************************************/

template <class Element>
inline Element &
KPArray<Element>::operator[](int index)
{
    if (index < 0 || index >= my_size)
        error("operator[] - invalid index");

    return my_elements[index];
}

/****************************************************************************/

template <class Element>
inline const Element &
KPArray<Element>::operator[](int index) const
{
    if (index < 0 || index >= my_size)
        error("operator[] - invalid index");

    return my_elements[index];
}

/****************************************************************************/

template <class Element>
inline int
KPArray<Element>::size() const
{
    return my_size;
}

/****************************************************************************/

template <class Element>
inline bool
KPArray<Element>::is_empty() const
{
    return (my_size == 0);
}

/****************************************************************************/

template <class Element>
void
KPArray<Element>::resize(int new_size)
{
    if (my_size == new_size)
        return;

    if (new_size < 0)
        error("resize() - size of KPArray must be non-negative");

    if (new_size > 0) {
        Element *new_space = new Element[new_size];
        check_mem(new_space);

        const int savable_elements = ::min(my_size, new_size);
        for (int i=0; i<savable_elements; i++)
            new_space[i] = my_elements[i];

        if (my_size > 0)
            delete [] my_elements;
        my_elements = new_space;
    }
    else
        delete [] my_elements;

    my_size = new_size;
}

/****************************************************************************/

template <class Element>
void
KPArray<Element>::resize(int new_size, const Element &init_element)
{
    if (my_size == new_size)
        return;

    if (new_size < 0)
        error("resize() - size of KPArray must be non-negative");

    if (new_size > 0) {
        Element *new_space = new Element[new_size];
        check_mem(new_space);

        int i;
        const int savable_elements = ::min(my_size, new_size);
        for (i=0; i<savable_elements; i++)
            new_space[i] = my_elements[i];

        if (my_size > 0)
            delete [] my_elements;
        my_elements = new_space;

        for (i=my_size; i<new_size; i++)
            my_elements[i] = init_element;
    }
    else
        delete [] my_elements;

    my_size = new_size;
}

/****************************************************************************/

template <class Element>
void
KPArray<Element>::set_all_to(const Element &element)
{
    for (int i=0; i<my_size; i++)
        my_elements[i] = element;
}

/****************************************************************************/

template <class Element>
void
KPArray<Element>::error(const char *msg) const
{
    cerr << "KPArray: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::KPComparableArray(): KPArray<Element>()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::KPComparableArray(int size): KPArray<Element>(size)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::KPComparableArray(int size,
            const Element &init_element): KPArray<Element>(size, init_element)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::KPComparableArray(
            const KPComparableArray<Element> &array): KPArray<Element>(array)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::KPComparableArray(const KPArray<Element> &array):
                                                KPArray<Element>(array)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPComparableArray<Element>::~KPComparableArray()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPComparableArray<Element> &
KPComparableArray<Element>::operator=(const KPArray<Element> &array)
{
    KPArray<Element>::operator=(array);
    return *this;
}

/****************************************************************************/

template <class Element>
bool
KPComparableArray<Element>::operator==(const KPComparableArray<Element> &array)
                                                                        const
{
    if (this == &array)
        return true;
    
    if (my_size != array.my_size)
        return false;

    for (int i=0; i<my_size; i++)
        if (!(my_elements[i] == array.my_elements[i]))
            return false;
    
    return true;
}

/****************************************************************************/

template <class Element>
inline bool
KPComparableArray<Element>::operator!=(const KPComparableArray<Element> &array)
                                                                        const
{
    return !(*this == array);
}

/****************************************************************************/

template <class Element>
inline bool
KPComparableArray<Element>::contains(const Element &element) const
{
    return (index_of(element) != -1);
}

/****************************************************************************/

template <class Element>
int
KPComparableArray<Element>::occurrences_of(const Element &element) const
{
    int occurrences = 0;

    for (int i=0; i<my_size; i++)
        if (my_elements[i] == element)
            occurrences++;

    return occurrences;
}

/****************************************************************************/

template <class Element>
int
KPComparableArray<Element>::index_of(const Element &element) const
{
    for (int i=0; i<my_size; i++)
        if (my_elements[i] == element)
            return i;

    return -1;
}

/****************************************************************************/

template <class Element>
void
KPComparableArray<Element>::error(const char *msg) const
{
    cerr << "KPComparableArray: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::KPSortableArray(): KPComparableArray<Element>()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::KPSortableArray(int size):
                                            KPComparableArray<Element>(size)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::KPSortableArray(int size,
    const Element &init_element): KPComparableArray<Element>(size, init_element)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::KPSortableArray(const KPSortableArray<Element>
                                    &array): KPComparableArray<Element>(array)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::KPSortableArray(const KPArray<Element> &array):
                                            KPComparableArray<Element>(array)
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline
KPSortableArray<Element>::~KPSortableArray()
{ /* do nothing new. */ }

/****************************************************************************/

template <class Element>
inline KPSortableArray<Element> &
KPSortableArray<Element>::operator=(const KPArray<Element> &array)
{
    KPComparableArray<Element>::operator=(array);
    return *this;
}

/****************************************************************************/

template <class Element>
int
KPSortableArray<Element>::findpivot(int i, int j)
{
    const Element &first = my_elements[i];
    for (int k=i+1; k<=j; k++) {
        if (first < my_elements[k])
            return k;
        else if (my_elements[k] < first)
            return i;
    }
    return -1;
}

template <class Element>
int
KPSortableArray<Element>::partition(int l, int r, const Element &pivot)
{
    do {
        swap(my_elements[l], my_elements[r]);
        while (my_elements[l] < pivot)
            l++;
        while (!(my_elements[r] < pivot))
            r--;
    } while (l <= r);
    return l;
}

template <class Element>
void
KPSortableArray<Element>::quicksort(int i, int j)
{
    Element pivot;
    int k, index;

    index = findpivot(i, j);
    if (index != -1) {
        pivot = my_elements[index];
        k = partition(i, j, pivot);
        quicksort(i, k-1);
        quicksort(k, j);
    }
}

template <class Element>
inline void
KPSortableArray<Element>::sort()
{
    quicksort(0, my_size-1);
}

/****************************************************************************/

template <class Element>
Element &
KPSortableArray<Element>::min()
{
    if (my_size < 1)
        error("min() - empty array");

    int min_index = 0;
    for (int i=1; i<my_size; i++)
        if (my_elements[i] < my_elements[min_index])
            min_index = i;

    return my_elements[min_index];
}

/****************************************************************************/

template <class Element>
const Element &
KPSortableArray<Element>::min() const
{
    if (my_size < 1)
        error("min() - empty array");

    int min_index = 0;
    for (int i=1; i<my_size; i++)
        if (my_elements[i] < my_elements[min_index])
            min_index = i;

    return my_elements[min_index];
}

/****************************************************************************/

template <class Element>
Element &
KPSortableArray<Element>::max()
{
    if (my_size < 1)
        error("max() - empty array");

    int max_index = 0;
    for (int i=1; i<my_size; i++)
        if (my_elements[max_index] < my_elements[i])
            max_index = i;

    return my_elements[max_index];
}

/****************************************************************************/

template <class Element>
const Element &
KPSortableArray<Element>::max() const
{
    if (my_size < 1)
        error("max() - empty array");

    int max_index = 0;
    for (int i=1; i<my_size; i++)
        if (my_elements[max_index] < my_elements[i])
            max_index = i;

    return my_elements[max_index];
}

/****************************************************************************/

template <class Element>
void
KPSortableArray<Element>::error(const char *msg) const
{
    cerr << "KPSortableArray: " << msg << '\n';
    exit(EXIT_FAILURE);
}

/****************************************************************************/

#endif /* KP_ARRAY_DEFINED */
