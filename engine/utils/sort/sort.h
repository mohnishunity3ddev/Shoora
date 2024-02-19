#if !defined (SHOORA_SORT_H)

#include <defines.h>
#include <containers/stack/stack.h>

template <typename T>
i32
Partition(T *Arr, i32 Low, i32 High, b32 (*LessEqCmp)(const T &, const T &))
{
    T pivot = Arr[Low];
    i32 i = Low, j = High;

    do
    {
        do { i++; } while (LessEqCmp(Arr[i], pivot) && i < High);
        do { j--; } while (!LessEqCmp(Arr[j], pivot) && j >= Low);

        if(i < j) {
            SWAP(Arr[i], Arr[j]);
        }
    } while(i < j);

    SWAP(Arr[Low], Arr[j]);
    return j;
}

template <typename T>
void
QuicksortIterative(T *Items, i32 Size, b32 (*Cmp)(const T &, const T &) = DefaultLessEqualComparator)
{
    i32 low = 0;
    i32 high = Size;

    if(high <= low) return;

    interval<i32> Range{low, high};
    interval<i32> InvalidInterval{-1, -1};
    stack<interval<i32>> Stack{Size, InvalidInterval};
    Stack.push(Range);

    while(!Stack.isEmpty()) {
        interval<i32> CurrentRange = Stack.pop();
        if(CurrentRange == InvalidInterval) {
            ASSERT(!"Received invalid interval! please check!");
            break;
        }

        i32 PartitionIndex = Partition(Items, CurrentRange.low, CurrentRange.high, Cmp);

        if(PartitionIndex > CurrentRange.low) {
            Stack.push({CurrentRange.low, PartitionIndex});
        }
        if(PartitionIndex + 1 < CurrentRange.high) {
            Stack.push({PartitionIndex + 1, CurrentRange.high});
        }
    }
}

template <typename T>
void
QuicksortRecursive(T *Items, i32 Low, i32 High, b32 (*Cmp)(const T &, const T &) = DefaultLessEqualComparator)
{
    if(High <= Low)
        return;

    i32 p = Partition(Items, Low, High, Cmp);
    QuicksortRecursive(Items, Low, p, Cmp);
    QuicksortRecursive(Items, p+1, High, Cmp);
}

// TODO: Add other sorting algorithms here if needed: Radix sort for integers and merge sort

#define SHOORA_SORT_H
#endif