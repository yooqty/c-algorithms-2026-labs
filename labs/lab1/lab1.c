#include <stdio.h>
#include <ctype.h>

int isvowel(int c);

int main(void)
{
    FILE *F;
    int c;
    long total = 0;
    long lines = 0;
    long digits = 0;
    long punct = 0;
    long spaces = 0;
    long letters = 0;
    long lower = 0;
    long upper = 0;
    long vowels = 0;
    long consonants = 0; 

    char filename[256];
    printf("Enter the file name: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL) {
        printf("File name input error.\n");
        return 1;
    }
    size_t len = 0;
    while (filename[len] != '\0' && filename[len] != '\n')
        len++;
    filename[len] = '\0';

    F = fopen(filename, "r");
    if (F == NULL) {
        printf("Failed to open file '%s'.\n", filename);
        return 1;
    }

    while ((c = fgetc(F)) != EOF) {
        total++;

        if (c == '\n')
            lines++;

        if (isdigit(c))
            digits++;

        if (ispunct(c))
            punct++;

        if (isspace(c))
            spaces++;

        if (isalpha(c)) {
            letters++;
            if (islower(c))
                lower++;
            if (isupper(c))
                upper++;

            if (isvowel(c))
                vowels++;
            else
                consonants++;
        }
    }

    fclose(F);

    printf("\n--- File analysis results '%s' ---\n", filename);
    printf("Total number of characters:      %ld\n", total);
    printf("Number of lines:                 %ld\n", lines);
    printf("Number of digits:                %ld\n", digits);
    printf("Number of punctuation marks:     %ld\n", punct);
    printf("Number of space characters:      %ld\n", spaces);
    printf("Number of letters:               %ld\n", letters);
    printf("  of them lowercase:             %ld\n", lower);
    printf("  of them uppercase:             %ld\n", upper);
    printf("Number of vowels:                %ld\n", vowels);
    printf("Number of consonants:            %ld\n", consonants);

    return 0;
}

int isvowel(int c)
{
    char ch = tolower(c);
    return (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u');
}