#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct word {
    int count;
    char *Word;
} WORD;

WORD *words[100000];
int totalw = 0;

char *getword(FILE *F);
int cmpword_alpha(const void *a, const void *b);
int cmpword_quant(const void *a, const void *b);

int main(int ac, char *av[]) {
    FILE *OUT1, *OUT2, *IN;
    char *pword;

    OUT1 = fopen("out_alpha.txt", "w");
    OUT2 = fopen("out_freq.txt", "w");
    if (!OUT1 || !OUT2) {
        printf("Creating output files error!\n");
        exit(1);
    }

    if (ac < 2) {
        printf("A file list wasn't specified!\n");
        exit(0);
    }

    for (int i = 1; i < ac; i++) {
        char path[256];
        snprintf(path, sizeof(path), "texts/%s", av[i]);
        IN = fopen(path, "r");
        if (!IN) {
            printf("Failed to open file %s\n", path);
            continue;
        }
        printf("File processing %s\n", path);

        while ((pword = getword(IN)) != NULL) {
            int found = -1;
            for (int j = 0; j < totalw; j++) {
                if (strcmp(words[j]->Word, pword) == 0) {
                    found = j;
                    break;
                }
            }
            if (found != -1) {
                words[found]->count++;
                free(pword);
            } else {
                WORD *new_word = (WORD *)malloc(sizeof(WORD));
                new_word->Word = pword;
                new_word->count = 1;
                words[totalw++] = new_word;
            }
        }
        fclose(IN);
    }

    qsort(words, totalw, sizeof(WORD *), cmpword_alpha);
    for (int i = 0; i < totalw; i++) {
        fprintf(OUT1, "%s %d\n", words[i]->Word, words[i]->count);
    }
    fclose(OUT1);

    qsort(words, totalw, sizeof(WORD *), cmpword_quant);
    for (int i = 0; i < totalw; i++) {
        fprintf(OUT2, "%s %d\n", words[i]->Word, words[i]->count);
    }
    fclose(OUT2);

    for (int i = 0; i < totalw; i++) {
        free(words[i]->Word);
        free(words[i]);
    }

    return 0;
}

char *getword(FILE *F) {
    int c;
    while ((c = fgetc(F)) != EOF && !isalpha(c));
    if (c == EOF) return NULL;

    char buffer[1024];
    int pos = 0;
    do {
        c = tolower(c);
        if (pos < 1023) buffer[pos++] = c;
        c = fgetc(F);
    } while (c != EOF && (isalnum(c)));
    buffer[pos] = '\0';

    char *word = (char *)malloc(pos + 1);
    if (word) strcpy(word, buffer);
    return word;
}

int cmpword_alpha(const void *a, const void *b) {
    WORD *wa = *(WORD **)a;
    WORD *wb = *(WORD **)b;
    return strcmp(wa->Word, wb->Word);
}

int cmpword_quant(const void *a, const void *b) {
    WORD *wa = *(WORD **)a;
    WORD *wb = *(WORD **)b;
    if (wa->count != wb->count)
        return wb->count - wa->count;
    else
        return strcmp(wa->Word, wb->Word);
}