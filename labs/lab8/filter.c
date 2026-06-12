#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#define BUFFER_SIZE 1024
#define MAX_FILTER_SIZE 9

typedef struct {
    int size;
    double coeff[MAX_FILTER_SIZE];
    double norm;
} SavitzkyGolayFilter;

const SavitzkyGolayFilter filters[] = {
    {5, {-3, 12, 17, 12, -3}, 35},
    {7, {-2, 3, 6, 7, 6, 3, -2}, 21},
    {9, {-21, 14, 39, 54, 59, 54, 39, 14, -21}, 231}
};

SavitzkyGolayFilter select_filter(int points) {
    switch(points) {
        case 5: return filters[0];
        case 7: return filters[1];
        case 9: return filters[2];
        default: return filters[0];
    }
}

double smooth_value(const double *buffer, const SavitzkyGolayFilter *filter, int position) {
    double result = 0.0;
    int half_size = filter->size / 2;
    int i;
    
    for (i = 0; i < filter->size; i++) {
        int idx = position - half_size + i;
        result += buffer[idx] * filter->coeff[i];
    }
    
    return result / filter->norm;
}

int main(int argc, char **argv) {
    double y1[BUFFER_SIZE];
    double y2[BUFFER_SIZE];
    int buffer_pos = 0;
    int points = 5;
    int i;
    double input1, input2;
    SavitzkyGolayFilter filter;
    
    if (argc > 1) {
        points = atoi(argv[1]);
        if (points != 5 && points != 7 && points != 9) points = 5;
    }
    
    filter = select_filter(points);
    
    for (i = 0; i < BUFFER_SIZE; i++) {
        y1[i] = y2[i] = 0.0;
    }
    
    while (scanf("%lf %lf", &input1, &input2) == 2) {
        y1[buffer_pos] = input1;
        y2[buffer_pos] = input2;
        
        if (buffer_pos >= filter.size - 1) {
            double smoothed1 = smooth_value(y1, &filter, buffer_pos);
            double smoothed2 = smooth_value(y2, &filter, buffer_pos);
            printf("%lf %lf\n", smoothed1, smoothed2);
        } else {
            printf("%lf %lf\n", input1, input2);
        }
        fflush(stdout);
        
        buffer_pos++;
        if (buffer_pos >= BUFFER_SIZE) {
            for (i = 0; i < BUFFER_SIZE - filter.size; i++) {
                y1[i] = y1[i + filter.size];
                y2[i] = y2[i + filter.size];
            }
            buffer_pos = BUFFER_SIZE - filter.size;
        }
    }
    
    return 0;
}