#if !defined(DOUBLE_LINKED_LIST_H)
#define DOUBLE_LINKED_LIST_H

#include "defines.h"

template <typename T>
struct dll_node
{
    T value;
    dll_node *next;
    dll_node *prev;

    dll_node(T value);
    dll_node(T val, dll_node *nextPtr, dll_node *prevPtr);

    ~dll_node();
};
#define dnode dll_node<T>

template <typename T>
struct double_linked_list
{
    dnode *head = nullptr;
    dnode *tail = nullptr;
    i32 size = 0;

    double_linked_list() = default;
    ~double_linked_list();

    b32 IsEmpty();
    void Add(T val);
    void AddAt(i32 index, T val);
    void Update(i32 index, T val);
    void RemoveAt(i32 index);
    void Remove(T val);
    dnode *GetNode(int index);
    dnode *GetMidNode(int start, int end, int *mid);
    i32 GetCount();
    void Display();
    void DisplayReverse();
    void Clear();
};

#include "double_linked_list_impl.h"

#endif // DOUBLE_LINKED_LIST_H