#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/libs.h"

float sin_integral(float a, float b, float e) {
    if (e <= 0 || a >= b) return 0.0f;
    
    float integral = 0.0f;
    float x = a;
    
    while (x < b) {
        float step = (x + e <= b) ? e : (b - x);
        float mid_x = x + step / 2.0f;
        integral += sinf(mid_x) * step;
        x += step;
    }
    
    return integral;
}

int *sort(int *array, size_t n) {
    if (!array || n == 0) return NULL;
    
    int *sorted = (int *)malloc(n * sizeof(int));
    if (!sorted) return NULL;
    
    memcpy(sorted, array, n * sizeof(int));
    
    for (size_t i = 0; i < n - 1; ++i) {
        for (size_t j = 0; j < n - i - 1; ++j) {
            if (sorted[j] > sorted[j + 1]) {
                int temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    return sorted;
}