// A module of KPlib v1.3.1.
// Written by Keith Pomakis during the summer of 1994.
// Released to the public domain on October 10, 1994.

#ifndef KP_STACK_DEFINED
#define KP_STACK_DEFINED

#include "KPbasic.h"
#include "KPList.h"
#include <stdlib.h>

// Assumes Element has a default constructor and operator=().

template <class Element>
class KPStack {
    public:
        KPStack();
        KPStack(const KPStack<Element> &stack);
        KPStack(const KPList<Element> &list);
        KPStack(const Element &element);
        ~KPStack();
        operator KPList<Element>() const;
        KPStack<Element> &operator=(const KPStack<Element> &stack);
        KPStack<Element> &operator=(const Element &element);
        KPStack<Element> &push(const Element &element);
        Element pop();
        Element &top();
        const Element &top() const;
        int size() const;
        bool is_empty() const;
        KPStack<Element> &clear();
    protected:
        static void stack_empty_error(const char *fname);
        KPList<Element> my_list;
};

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::KPStack(): my_list()
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::KPStack(const KPStack<Element> &stack): my_list(stack.my_list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::KPStack(const KPList<Element> &list): my_list(list)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::KPStack(const Element &element): my_list(element)
{ /* do nothing */ }

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::~KPStack()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
inline
KPStack<Element>::operator KPList<Element>() const
{
    return my_list;
}

/****************************************************************************/

template <class Element>
inline KPStack<Element> &
KPStack<Element>::operator=(const KPStack<Element> &stack)
{
    my_list = stack.my_list;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPStack<Element> &
KPStack<Element>::operator=(const Element &element)
{
    my_list = element;
    return *this;
}

/****************************************************************************/

template <class Element>
inline KPStack<Element> &
KPStack<Element>::push(const Element &element)
{
    my_list.add_to_head(element);
    return *this;
}

/****************************************************************************/

template <class Element>
inline Element
KPStack<Element>::pop()
{
    if (my_list.size() < 1)
        stack_empty_error("pop()");
    
    Element element = my_list.head();
    my_list.remove_head();
    return element;
}

/****************************************************************************/

template <class Element>
inline Element &
KPStack<Element>::top()
{
    if (my_list.size() < 1)
        stack_empty_error("top()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline const Element &
KPStack<Element>::top() const
{
    if (my_list.size() < 1)
        stack_empty_error("top()");

    return my_list.head();
}

/****************************************************************************/

template <class Element>
inline int
KPStack<Element>::size() const
{
    return my_list.size();
}

/****************************************************************************/

template <class Element>
inline bool
KPStack<Element>::is_empty() const
{
    return my_list.is_empty();
}

/****************************************************************************/

template <class Element>
inline KPStack<Element> &
KPStack<Element>::clear()
{
    my_list.clear();
}

/****************************************************************************/

template <class Element>
void
KPStack<Element>::stack_empty_error(const char *fname)
{
    cerr << "KPStack: " << fname << " - stack empty\n";
    exit(EXIT_FAILURE);
}

/****************************************************************************/

#endif /* KP_STACK_DEFINED */
