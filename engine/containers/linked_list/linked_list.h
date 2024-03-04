#pragma once

#include "defines.h"

template <typename T>
struct singly_linked_list_node
{
    T data;
    singly_linked_list_node *next;

    singly_linked_list_node(T data);
    singly_linked_list_node(T data, singly_linked_list_node *nextPtr);

    ~singly_linked_list_node();
};
#define node singly_linked_list_node<T>

template <typename T>
struct singly_linked_list
{
    node *head;
    node *tail;
    i32 numItems;

    explicit singly_linked_list();
    ~singly_linked_list();

    b32 IsEmpty();
    void Add(T val);
    void Insert(node *previousNode, node *newNode);
    void Remove(node *previousNode, node *toDeleteNode);
    void AddAt(i32 index, T val);
    void Update(i32 index, T val);
    void RemoveAt(i32 index);
    void Remove(T val);
    i32 GetCount();
    void Display();
    void Clear();
};

#include "linked_list_impl.h"