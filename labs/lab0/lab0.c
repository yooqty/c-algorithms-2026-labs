#include <stdio.h>
#include <stdlib.h>

int main() {
    int n;
    printf("Enter n: ");
    scanf("%d", &n);
    
    int *P = (int*)malloc((n + 1) * sizeof(int));
    if (P == NULL) {
        printf("Memory allocation error.\n");
        return 1;
    }
    
    P[0] = 0;
    P[1] = 0;
    for (int i = 2; i <= n; i++) {
        P[i] = i;
    }
    
    int p = 2;
    
    while (p * p <= n) {
        for (int i = p * p; i <= n; i += p) {
            P[i] = 0;
        }
        p++;
        while (p <= n && P[p] == 0) {
            p++;
        }
    }
    
    int count = 0;
    for (int i = 2; i <= n; i++) {
        if (P[i] != 0) count++;
    }
    
    int *primes = (int*)malloc(count * sizeof(int));
    if (primes == NULL) {
        printf("Memory allocation error.\n");
        free(P);
        return 1;
    }
    
    int idx = 0;
    for (int i = 2; i <= n; i++) {
        if (P[i] != 0) {
            primes[idx++] = P[i];
        }
    }
    
    printf("Prime numbers up to %d:\n", n);
    for (int i = 0; i < count; i++) {
        printf("%d ", primes[i]);
    }
    printf("\n");
    
    free(P);
    free(primes);
    
    return 0;
}