#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

static const char *translit_table[256] = {0};

void init_translit_table(void) {
    translit_table[0xC0] = "A";
    translit_table[0xC1] = "B";
    translit_table[0xC2] = "V";
    translit_table[0xC3] = "G";
    translit_table[0xC4] = "D";
    translit_table[0xC5] = "E";
    translit_table[0xA8] = "Yo";
    translit_table[0xC6] = "Zh";
    translit_table[0xC7] = "Z";
    translit_table[0xC8] = "I";
    translit_table[0xC9] = "J";
    translit_table[0xCA] = "K";
    translit_table[0xCB] = "L";
    translit_table[0xCC] = "M";
    translit_table[0xCD] = "N";
    translit_table[0xCE] = "O";
    translit_table[0xCF] = "P";
    translit_table[0xD0] = "R";
    translit_table[0xD1] = "S";
    translit_table[0xD2] = "T";
    translit_table[0xD3] = "U";
    translit_table[0xD4] = "F";
    translit_table[0xD5] = "Kh";
    translit_table[0xD6] = "Cz";
    translit_table[0xD7] = "Ch";
    translit_table[0xD8] = "Sh";
    translit_table[0xD9] = "Shh";
    translit_table[0xDA] = "``";
    translit_table[0xDB] = "Y`";
    translit_table[0xDC] = "`";
    translit_table[0xDD] = "E`";
    translit_table[0xDE] = "Yu";
    translit_table[0xDF] = "Ya";
    translit_table[0xE0] = "a";
    translit_table[0xE1] = "b";
    translit_table[0xE2] = "v";
    translit_table[0xE3] = "g";
    translit_table[0xE4] = "d";
    translit_table[0xE5] = "e";
    translit_table[0xB8] = "yo";
    translit_table[0xE6] = "zh";
    translit_table[0xE7] = "z";
    translit_table[0xE8] = "i";
    translit_table[0xE9] = "j";
    translit_table[0xEA] = "k";
    translit_table[0xEB] = "l";
    translit_table[0xEC] = "m";
    translit_table[0xED] = "n";
    translit_table[0xEE] = "o";
    translit_table[0xEF] = "p";
    translit_table[0xF0] = "r";
    translit_table[0xF1] = "s";
    translit_table[0xF2] = "t";
    translit_table[0xF3] = "u";
    translit_table[0xF4] = "f";
    translit_table[0xF5] = "kh";
    translit_table[0xF6] = "cz";
    translit_table[0xF7] = "ch";
    translit_table[0xF8] = "sh";
    translit_table[0xF9] = "shh";
    translit_table[0xFA] = "``";
    translit_table[0xFB] = "y`";
    translit_table[0xFC] = "`";
    translit_table[0xFD] = "e`";
    translit_table[0xFE] = "yu";
    translit_table[0xFF] = "ya";
}

int is_rus_letter(unsigned char c) {
    if (c >= 0xC0 && c <= 0xDF) return 1;
    if (c >= 0xE0 && c <= 0xFF) return 1;
    if (c == 0xA8 || c == 0xB8) return 1;
    return 0;
}

char *translit_word(const unsigned char *word, int len) {
    int out_len = 0;
    for (int i = 0; i < len; i++) {
        const char *tr = translit_table[word[i]];
        if (tr) out_len += strlen(tr);
        else out_len += 1;
    }
    char *out = malloc(out_len + 1);
    if (!out) return NULL;
    int pos = 0;
    for (int i = 0; i < len; i++) {
        const char *tr = translit_table[word[i]];
        if (tr) {
            strcpy(out + pos, tr);
            pos += strlen(tr);
        } else {
            out[pos++] = word[i];
        }
    }
    out[pos] = '\0';
    return out;
}

unsigned char **split_words(const unsigned char *buf, size_t buf_len, int **lens, int *word_cnt) {
    unsigned char **words = NULL;
    int *lengths = NULL;
    int cnt = 0;
    int i = 0;
    while (i < buf_len) {
        while (i < buf_len && !is_rus_letter(buf[i])) i++;
        if (i >= buf_len) break;
        int start = i;
        while (i < buf_len && is_rus_letter(buf[i])) i++;
        int word_len = i - start;
        unsigned char *word = malloc(word_len + 1);
        if (!word) goto error;
        memcpy(word, buf + start, word_len);
        word[word_len] = '\0';
        words = realloc(words, (cnt + 1) * sizeof(unsigned char*));
        lengths = realloc(lengths, (cnt + 1) * sizeof(int));
        if (!words || !lengths) goto error;
        words[cnt] = word;
        lengths[cnt] = word_len;
        cnt++;
    }
    *lens = lengths;
    *word_cnt = cnt;
    return words;
error:
    for (int j = 0; j < cnt; j++) free(words[j]);
    free(words);
    free(lengths);
    return NULL;
}

void process_file(const char *fname) {
    FILE *f = fopen(fname, "rb");
    if (!f) {
        fprintf(stderr, "Cannot open %s\n", fname);
        return;
    }
    struct stat st;
    if (stat(fname, &st) != 0) {
        fclose(f);
        return;
    }
    size_t fsize = st.st_size;
    unsigned char *buf = malloc(fsize);
    if (!buf) {
        fclose(f);
        return;
    }
    if (fread(buf, 1, fsize, f) != fsize) {
        free(buf);
        fclose(f);
        return;
    }
    fclose(f);

    int *lens = NULL;
    int word_cnt = 0;
    unsigned char **words = split_words(buf, fsize, &lens, &word_cnt);
    free(buf);
    if (!words || word_cnt == 0) {
        free(lens);
        return;
    }

    int max_len = 0;
    for (int i = 0; i < word_cnt; i++) {
        if (lens[i] > max_len) max_len = lens[i];
    }

    char outname[1024];
    snprintf(outname, sizeof(outname), "%s_translit.txt", fname);
    FILE *out = fopen(outname, "w");
    if (!out) {
        perror("Cannot create output file");
        for (int i = 0; i < word_cnt; i++) free(words[i]);
        free(words);
        free(lens);
        return;
    }

    for (int i = 0; i < word_cnt; i++) {
        char *trans = translit_word(words[i], lens[i]);
        if (!trans) continue;
        int spaces = max_len - lens[i];
        for (int s = 0; s < spaces; s++) fputc(' ', out);
        fputs(trans, out);
        fputc('\n', out);
        free(trans);
        free(words[i]);
    }
    fclose(out);
    free(words);
    free(lens);
    printf("Processed: %s -> %s\n", fname, outname);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file1 file2 ...\n", argv[0]);
        return 1;
    }
    init_translit_table();
    for (int i = 1; i < argc && i <= 1000; i++) {
        process_file(argv[i]);
    }
    return 0;
}