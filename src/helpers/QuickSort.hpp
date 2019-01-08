/*
FILE:               QuickSort.hpp
DESCRIPTION:        Implements a templated version of QuickSort which can sort any type
                    with <= operators declared

MODIFICATION HISTORY:
Author             Date               Version
---------------    ----------         --------------
GeeksforGeeks	   2019-06-01		  0.0 - Code from: https://www.geeksforgeeks.org/quick-sort/
Joshua Dahl        2019-06-01         0.1 - Implemented templating and vector support
*/
#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include <vector>

/*
NAME:           swap(T* a, T* b)
DESCRIPTION:    Utility function to swap two elements
*/
template <typename T>
void swap(T* a, T* b)
{
    T t = *a;
    *a = *b;
    *b = t;
}

/*
NAME:           partition (std::vector<T>& arr, int low, int high)
DESCRIPTION:    This function takes last element as pivot, places the pivot element
                 at its correct position in sorted array, and places all smaller (smaller
                 than pivot) to left of pivot and all greater elements to right of pivot
RETURNS:        Index of pivot
*/
template <typename T>
int partition (T arr[], int low, int high)
{
    T pivot = arr[high];    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (arr[j] <= pivot)
        {
            i++;    // increment index of smaller element
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

/*
NAME:           quickSort(std::vector<T>& arr, int low, int high)
DESCRIPTION:    The main function that implements QuickSort
*/
template <typename T>
void quickSort(T arr[], int low, int high)
{
    if (low < high)
    {
        /* pi is partitioning index, arr[p] is now
           at right place */
        int pi = partition(arr, low, high);

        // Separately sort elements before
        // partition and after partition
        quickSort(arr, low, pi - 1);
        quickSort(arr, pi + 1, high);
    }
}

/*
NAME:           quickSort(std::vector<T> arr)
DESCRIPTION:    Preforms a quicksort on the entire provided array
*/
template <typename T>
inline void quickSort(T arr[], int size){
    // Sort the interval [0, size-1]
    quickSort(arr, 0, size - 1);
}

/*
NAME:           quickSort(std::vector<T> arr)
DESCRIPTION:    Preforms a quicksort on the entire provided vector
*/
template <typename T>
inline void quickSort(std::vector<T>& vec){
    // Sort the interval [0, array.size()-1] passing a pointer to the first element
    // in the vector as the array
    quickSort(&vec[0], vec.size());
}

#endif
