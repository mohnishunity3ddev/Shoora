#include "double_linked_list.h"
#include <platform/platform.h>
#include <utils/utils.h>
template <typename T>
dll_node<T>::dll_node(T value) : value(value), next(nullptr), prev(nullptr) {}

template <typename T>
dll_node<T>::dll_node(T val, dll_node *nextPtr, dll_node *prevPtr) : value(val), next(nextPtr), prev(prevPtr)
{
}

template <typename T>
dll_node<T>::~dll_node()
{
    // LogInfo("destructor for dnode called!\n");
}

template <typename T>
double_linked_list<T>::~double_linked_list()
{
    LogInfo("destructor for double linked list called!\n");
    this->Clear();
}

template <typename T>
b32
double_linked_list<T>::IsEmpty()
{
    bool result = ((head == nullptr) && (size == 0));
    return result;
}

template <typename T>
void
double_linked_list<T>::Add(T val)
{
    dnode *n = new dnode(val);
    if (IsEmpty())
    {
        head = n;
        size = 1;
        tail = head;
    }
    else
    {
        ASSERT(tail != nullptr && tail->next == nullptr);
        n->prev = tail;
        tail->next = n;
        tail = n;
        size++;
    }
}

template <typename T>
void
double_linked_list<T>::AddAt(i32 index, T val)
{
    if (index >= size || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << size << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), size, index);
#endif
        return;
    }

    dnode *n = new dnode(val);
    if (index == 0)
    {
        head->prev = n;
        n->next = head;
        head = n;
        return;
    }

    dnode *c = head;
    for (i32 i = 0; i < index - 1; ++i)
    {
        c = c->next;
    }

    c->prev->next = n;
    n->prev = c->prev;

    n->next = c;
    c->prev = n;
}

template <typename T>
void
double_linked_list<T>::Update(i32 index, T val)
{
    if (index >= size || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << size << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), size, index);
#endif
        return;
    }

    dnode *c = head;
    for (i32 i = 0; i < index; ++i)
    {
        c = c->next;
    }
    ASSERT(c != nullptr);
    c->value = val;
}

template <typename T>
void
double_linked_list<T>::RemoveAt(i32 index)
{
    if (index >= size || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << size << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), size, index);
#endif
        return;
    }

    // special case for advancing head.
    if (index == 0)
    {
        dnode *n = head;
        head = head->next;
        head->prev = nullptr;
        T value = n->value;
        delete n;
        size--;
        if (head == nullptr)
        {
            tail = nullptr;
        }
        return;
    }

    dnode *c = head;
    for (i32 i = 0; i < index - 1; ++i)
    {
        c = c->next;
    }

    T val = c->value;
    dnode *n = c;
    c->prev->next = c->next;
    c->next->prev = c->prev;
    delete n;
    size--;
}

template <typename T>
void
double_linked_list<T>::Remove(T val)
{
    dnode *c = head;
    dnode *p = c;

    while (c != nullptr && c->value != val)
    {
        p = c;
        c = c->next;
    }

    if (c == nullptr)
    {
#if _SHU_DEBUG
        Tls << val << " not found.\n";
        LogInfo(Tls.ToString(), val);
#endif
        return;
    }

    dnode *n = c;
    p->next = c->next;
    c->next->prev = p;
    delete n;
    size--;

    // LogInfo << "removed " << val << "\n";
    // display();
}

template <typename T>
dnode *
double_linked_list<T>::GetNode(int index)
{
    if (index < 0 || index >= size)
    {
        return nullptr;
    }

    if (index == 0)
    {
        return head;
    }
    if (index == size - 1)
    {
        return tail;
    }
    dnode *curr = head;
    int count = 0;
    while (curr != nullptr && count++ < index)
    {
        curr = curr->next;
    }
    return curr;
}

template <typename T>
dnode *
double_linked_list<T>::GetMidNode(int start, int end, int *mid)
{
    if (start < 0 || end >= size)
    {
        LogInfo("Cannot get Mid node since the range was wrong!\n");
        return nullptr;
    }

    int midIndex = start + ((end - start) / 2);
    dnode *result = GetNode(midIndex);
    *mid = midIndex;
    return result;
}

template <typename T>
i32
double_linked_list<T>::GetCount()
{
    return size;
}

template <typename T>
void
double_linked_list<T>::Display()
{
    if (IsEmpty())
    {
        LogInfo("the list is empty!\n");
        return;
    }
    // LogInfo << "list with size " << size << " is ";
    LogInfo("(head)");
    dnode *c = head;
    while (c != nullptr)
    {
#if _SHU_DEBUG
        Tls << c->value << " ==> ";
        LogInfo(Tls.ToString(), c->value);
#endif
        c = c->next;
    }
    LogInfo("(tail).\n");
}

template <typename T>
void
double_linked_list<T>::DisplayReverse()
{
    LogInfo("the doubly linked list in reverse: \n");
    dnode *ptr = this->tail;
    LogInfo("(tail)");
    while (ptr != nullptr)
    {
#if _SHU_DEBUG
        Tls << ptr->value << " ==> ";
        LogInfo(Tls.ToString(), ptr->value);
#endif
        ptr = ptr->prev;
    }
    LogInfo("(head).\n");
}

template <typename T>
void
double_linked_list<T>::Clear()
{
    dnode *c = head;
    while (c != nullptr)
    {
        dnode *n = c;
        c = c->next;
        delete n;
    }
    size = 0;
    head = nullptr;
}
