#include "linked_list.h"
#include <platform/platform.h>
#include <utils/utils.h>

template <typename T>
singly_linked_list_node<T>::singly_linked_list_node(T data)
    : data(data), next(nullptr)
{
}

template<typename T>
singly_linked_list_node<T>::singly_linked_list_node(T data, singly_linked_list_node<T> *nextPtr)
    : data(data), next(nextPtr)
{
}

template <typename T>
singly_linked_list_node<T>::~singly_linked_list_node()
{
    LogInfo("ll_node destructor called!\n");
}

template<typename T>
singly_linked_list<T>::singly_linked_list()
    : head(nullptr), tail(nullptr), numItems(0)
{
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
    bool result = ((head == nullptr) && (numItems == 0));
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
        numItems = 1;
        tail = head;
    }
    else
    {
        ASSERT(tail != nullptr && tail->next == nullptr);
        tail->next = n;
        tail = n;
        numItems++;
    }
}

template <typename T>
void
singly_linked_list<T>::Insert(node *PreviousNode, node *NewNode)
{
    if(PreviousNode == nullptr) {
        // new node is the new head of the linked list.
        if(head != nullptr) {
            NewNode->next = head;
        } else {
            NewNode->next = nullptr;
        }
        head = NewNode;
    }
    else {
        if(PreviousNode->next == nullptr) {
            PreviousNode->next = NewNode;
            NewNode->next = nullptr;
        } else {
            NewNode->next = PreviousNode->next;
            PreviousNode->next = NewNode;
        }
    }

    ++this->numItems;
}


template <typename T>
void
singly_linked_list<T>::AddAt(i32 index, T val)
{
    if (index >= numItems || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << numItems << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), numItems, index);
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
    if (index >= numItems || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << numItems << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), numItems, index);
#endif

        return;
    }

    node *c = head;
    for (i32 i = 0; i < index; ++i)
    {
        c = c->next;
    }
    ASSERT(c != nullptr);
    c->data = val;
}

template <typename T>
void
singly_linked_list<T>::RemoveAt(i32 index)
{
#if _SHU_DEBUG
    Tls << "Trying to Remove at index " << index << ".\n";
    LogInfo(Tls.ToString(), index);
#endif

    if (index >= numItems || index < 0)
    {
#if _SHU_DEBUG
        Tls << "Out of bounds access. The list has only " << numItems << " items and you are trying to access "
            << index << " index.\n";
        LogInfo(Tls.ToString(), numItems, index);
#endif
        return;
    }

    // special case for advancing head.
    if (index == 0)
    {
        node *n = head;
        head = head->next;
        T data = n->data;
        delete n;
        numItems--;
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

    T val = c->data;
    node *n = c;
    p->next = c->next;
    delete n;
    numItems--;
}

template <typename T>
void
singly_linked_list<T>::Remove(node *PreviousNode, node *ToDeleteNode)
{
    if (PreviousNode == nullptr)
    {
        if (ToDeleteNode->next == nullptr) {
            head = nullptr;
        } else {
            head = ToDeleteNode->next;
        }
    }
    else
    {
        PreviousNode->next = ToDeleteNode->next;
    }

    --this->numItems;
}

template <typename T>
void
singly_linked_list<T>::Remove(T val)
{
    node *c = head;
    node *p = c;

    while (c != nullptr && c->data != val)
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
    numItems--;

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
    return numItems;
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
    // LogInfo "list with numItems " << numItems << " is ";
    LogInfo("Head");
    node *c = head;
    while (c != nullptr)
    {
#if _SHU_DEBUG
        Tls << " ==> " << c->data;
        LogInfo(Tls.ToString(), c->data);
#endif

        c = c->next;
    }
    LogInfo(" ==> end.\n\n");
}

template <typename T>
void
singly_linked_list<T>::Clear()
{
    // TODO: Clear does not work with a custom allocator right now.
#if 0
    node *c = head;
    while (c != nullptr)
    {
        node *n = c;
        c = c->next;
        // delete n;
    }
#endif
    head = nullptr;
    numItems = 0;
    head = nullptr;
}
