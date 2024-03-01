#include "linked_list.h"
#include <platform/platform.h>
#include <utils/utils.h>

#if _SHU_DEBUG
static templateString Tls;
#endif

template <typename T>
singly_linked_list_node<T>::singly_linked_list_node(T value)
{
    this->value = value;
    this->next = nullptr;
}

template<typename T>
singly_linked_list_node<T>::singly_linked_list_node(T val, singly_linked_list_node<T> *nextPtr)
{
    this->value = val;
    this->next = nextPtr;
}

template <typename T>
singly_linked_list_node<T>::~singly_linked_list_node()
{
    LogInfo("ll_node destructor called!\n");
}

template <typename T>
singly_linked_list<T>::~singly_linked_list()
{
    LogInfo("destructor for linked list called!\n");
    this->Clear();
}

template <typename T>
b32
singly_linked_list<T>::IsEmpty()
{
    bool result = ((head == nullptr) && (size == 0));
    return result;
}

template <typename T>
void
singly_linked_list<T>::Add(T val)
{
    node *n = new node(val);
    if (IsEmpty())
    {
        head = n;
        size = 1;
        tail = head;
    }
    else
    {
        ASSERT(tail != nullptr && tail->next == nullptr);
        tail->next = n;
        tail = n;
        size++;
    }
}

template <typename T>
void
singly_linked_list<T>::AddAt(i32 index, T val)
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
    node *n = new node(val);
    if (index == 0)
    {
        n->next = head;
        head = n;
        return;
    }

    node *c = head;
    node *p = c;
    for (i32 i = 0; i < index; ++i)
    {
        p = c;
        c = c->next;
    }

    p->next = n;
    n->next = c;
}

template <typename T>
void
singly_linked_list<T>::Update(i32 index, T val)
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

    node *c = head;
    for (i32 i = 0; i < index; ++i)
    {
        c = c->next;
    }
    ASSERT(c != nullptr);
    c->value = val;
}

template <typename T>
void
singly_linked_list<T>::RemoveAt(i32 index)
{
    if (index >= size || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Trying to Remove at index " << index << ".\n";
        LogInfo(Tls.ToString(), index);
        Tls << "Out of bounds access. The list has only " << size << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), size, index);
#endif

        return;
    }

    // special case for advancing head.
    if (index == 0)
    {
        node *n = head;
        head = head->next;
        T value = n->value;
        delete n;
        size--;
        if (head == nullptr)
        {
            tail = nullptr;
        }
        return;
    }

    node *c = head;
    node *p = c;
    for (i32 i = 0; i < index; ++i)
    {
        p = c;
        c = c->next;
    }

    T val = c->value;
    node *n = c;
    p->next = c->next;
    delete n;
    size--;
}

template <typename T>
void
singly_linked_list<T>::Remove(T val)
{
    node *c = head;
    node *p = c;

    while (c != nullptr && c->value != val)
    {
        p = c;
        c = c->next;
    }

    if (c == nullptr)
    {
#if _SHU_DEBUG
        Tls << val << "not found.\n";
        LogInfo(Tls.ToString(), val);
#endif
        return;
    }

    node *n = c;
    p->next = c->next;
    delete n;
    size--;

#if _SHU_DEBUG
    Tls << "removed " << val << ".\n";
    LogInfo(Tls.ToString(), val);
#endif
    // display();
}

template <typename T>
i32
singly_linked_list<T>::GetCount()
{
    return size;
}

template <typename T>
void
singly_linked_list<T>::Display()
{

    if (this->IsEmpty())
    {
        LogInfo("the list is empty!\n");
        return;
    }
    // LogInfo "list with size " << size << " is ";
    LogInfo("Head");
    node *c = head;
    while (c != nullptr)
    {
#if _SHU_DEBUG
        Tls << " ==> " << c->value;
        LogInfo(Tls.ToString(), c->value);
#endif

        c = c->next;
    }
    LogInfo(" ==> end.\n\n");
}

template <typename T>
void
singly_linked_list<T>::Clear()
{
    node *c = head;
    while (c != nullptr)
    {
        node *n = c;
        c = c->next;
        delete n;
    }
    size = 0;
    head = nullptr;
}
