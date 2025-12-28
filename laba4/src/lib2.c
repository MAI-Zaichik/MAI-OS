#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/libs.h"

float sin_integral(float a, float b, float e) {
    if (e <= 0 || a >= b) return 0.0f;
    
    float integral = 0.0f;
    float x = a;
    float prev_value = sinf(a);
    
    while (x < b) {
        float step = (x + e <= b) ? e : (b - x);
        float next_x = x + step;
        float next_value = sinf(next_x);
        
        integral += (prev_value + next_value) * step / 2.0f;
        
        x = next_x;
        prev_value = next_value;
    }
    
    return integral;
}

static int partition(int *arr, int low, int high) {
    int pivot = arr[high];
    int i = low - 1;
    
    for (int j = low; j < high; ++j) {
        if (arr[j] <= pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    
    return i + 1;
}

static void quicksort(int *arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);
        quicksort(arr, low, pi - 1);
        quicksort(arr, pi + 1, high);
    }
}

int *sort(int *array, size_t n) {
    if (!array || n == 0) return NULL;
    
    int *sorted = (int *)malloc(n * sizeof(int));
    if (!sorted) return NULL;
    
    memcpy(sorted, array, n * sizeof(int));
    quicksort(sorted, 0, n - 1);
    
    return sorted;
}