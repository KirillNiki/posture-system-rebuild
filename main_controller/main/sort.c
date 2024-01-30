
#include <stdio.h>
#include <string.h>
#include "sort.h"

void quick_sort(int *array, size_t array_size)
{
    if (array_size <= 1)
    {
        return;
    }
    int middle_index = array_size / 2;
    int middle_number = array[middle_index];
    int first_part[array_size - 1];
    int second_part[array_size - 1];
    int first_index = 0;  // size of array
    int second_index = 0; // size of array

    for (int i = 0; i < array_size; i++)
    {
        if (array[i] <= array[middle_index] && i != middle_index)
        {
            first_part[first_index] = array[i];
            first_index++;
        }
        else if (array[i] > array[middle_index])
        {
            second_part[second_index] = array[i];
            second_index++;
        }
    }

    for (int i = 0; i < array_size; i++)
    {
        if (i < first_index)
        {
            array[i] = first_part[i];
        }
        else if (i == first_index)
        {
            array[i] = middle_number;
        }
        else
        {
            array[i] = second_part[i - first_index - 1];
        }
    }
    memset(first_part, 0, sizeof(first_part));
    memset(second_part, 0, sizeof(second_part));
    quick_sort(array, first_index);
    quick_sort(array + first_index + 1, second_index);
}

