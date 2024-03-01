#if !defined(DOUBLE_LINKED_LIST_H)

#include "defines.h"
#include <iostream>

template <typename T>
struct dll_node
{
    T value;
    dll_node *next;
    dll_node *prev;

    dll_node(T value) : value(value), next(nullptr), prev(nullptr) {}

    dll_node(T val, dll_node *nextPtr, dll_node *prevPtr)
        : value(val), next(nextPtr), prev(prevPtr)
    {
    }

    ~dll_node()
    {
        // LogInfo("destructor for dnode called!\n");
    }
};
#define dnode dll_node<T>

template <typename T>
struct double_linked_list
{
    dnode *head = nullptr;
    dnode *tail = nullptr;
    i32 size = 0;

    double_linked_list() = default;
    ~double_linked_list()
    {
        std::cout << "destructor for double linked list called!\n";
        clear();
    }

    bool
    isEmpty()
    {
        bool result = ((head == nullptr) && (size == 0));
        return result;
    }

    void
    add(T val)
    {
        dnode *n = new dnode(val);
        if (isEmpty())
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

    void
    addAt(i32 index, T val)
    {
        if(index >= size || index < 0)
        {
            std::cout << "Out of bounds access. The list has only " << size
                      << " items and you are trying to access " << index
                      << " index.\n";
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
        for(i32 i = 0; i < index - 1; ++i)
        {
            c = c->next;
        }

        c->prev->next = n;
        n->prev = c->prev;

        n->next = c;
        c->prev = n;
    }

    void
    update(i32 index, T val)
    {
        if (index >= size || index < 0)
        {
            std::cout << "Out of bounds access. The list has only " << size
                      << " items and you are trying to access " << index
                      << " index.\n";
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

    void
    removeAt(i32 index)
    {
        if (index >= size || index < 0)
        {
            std::cout << "Out of bounds access. The list has only " << size
                      << " items and you are trying to access " << index
                      << " index.\n";
            return;
        }

        // special case for advancing head.
        if (index == 0)
        {
            dnode *n = head;
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

    void
    remove(T val)
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
            std::cout << val << " not found.\n";
            return;
        }

        dnode *n = c;
        p->next = c->next;
        c->next->prev = p;
        delete n;
        size--;

        // std::cout << "removed " << val << "\n";
        // display();
    }

    dnode *
    getNode(int index)
    {
        if(index < 0 || index >= size) {
            return nullptr;
        }

        if(index == 0) {
            return head;
        }
        if(index == size - 1) {
            return tail;
        }
        dnode *curr = head;
        int count = 0;
        while(curr != nullptr && count++ < index) {
            curr = curr->next;
        }
        return curr;
    }

    dnode *
    getMidNode(int start, int end, int *mid)
    {
        if(start < 0 || end >= size) {
            std::cout << "Cannot get Mid node since the range was wrong!\n";
            return nullptr;
        }

        int midIndex = start + ((end - start)/2);
        dnode *result = getNode(midIndex);
        *mid = midIndex;
        return result;
    }

    i32
    getCount()
    {
        return size;
    }

    void
    display()
    {
        if (isEmpty())
        {
            std::cout << "the list is empty!\n";
            return;
        }
        // std::cout << "list with size " << size << " is ";
        std::cout << "Head";
        dnode *c = head;
        while (c != nullptr)
        {
            std::cout << " ==> " << c->value;
            c = c->next;
        }
        std::cout << " ==> end.\n\n";
    }

    void
    displayReverse()
    {
        std::cout << "the doubly linked list in reverse: \n";
        auto *ptr = this->tail;
        while (ptr != nullptr) {
            std::cout << ptr->value << " ==> ";
            ptr = ptr->prev;
        }
        std::cout << "\n";
    }

    void
    clear()
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
};

#define DOUBLE_LINKED_LIST_H
#endif // DOUBLE_LINKED_LIST_H