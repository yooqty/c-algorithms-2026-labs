#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

typedef struct {
    const char *latin;
    unsigned char rus;
    int len;
} BackTab;

static BackTab back_table[] = {
    {"Shh", 0xF9}, {"shh", 0xF9}, {"SHH", 0xF9},
    {"Sch", 0xF9}, {"sch", 0xF9},
    {"Zh",  0xC6}, {"zh",  0xE6},
    {"Kh",  0xD5}, {"kh",  0xF5},
    {"Cz",  0xD6}, {"cz",  0xF6},
    {"Ch",  0xD7}, {"ch",  0xF7},
    {"Sh",  0xD8}, {"sh",  0xF8},
    {"Ya",  0xDF}, {"ya",  0xFF},
    {"Yu",  0xDE}, {"yu",  0xFE},
    {"Yo",  0xA8}, {"yo",  0xB8},
    {"Y`",  0xDB}, {"y`",  0xFB},
    {"E`",  0xDD}, {"e`",  0xFD},
    {"``",  0xDA}, {"``",  0xFA},
    {"`",   0xDC}, {"`",   0xFC},
    {"A",   0xC0}, {"a",   0xE0},
    {"B",   0xC1}, {"b",   0xE1},
    {"V",   0xC2}, {"v",   0xE2},
    {"G",   0xC3}, {"g",   0xE3},
    {"D",   0xC4}, {"d",   0xE4},
    {"E",   0xC5}, {"e",   0xE5},
    {"Z",   0xC7}, {"z",   0xE7},
    {"I",   0xC8}, {"i",   0xE8},
    {"J",   0xC9}, {"j",   0xE9},
    {"K",   0xCA}, {"k",   0xEA},
    {"L",   0xCB}, {"l",   0xEB},
    {"M",   0xCC}, {"m",   0xEC},
    {"N",   0xCD}, {"n",   0xED},
    {"O",   0xCE}, {"o",   0xEE},
    {"P",   0xCF}, {"p",   0xEF},
    {"R",   0xD0}, {"r",   0xF0},
    {"S",   0xD1}, {"s",   0xF1},
    {"T",   0xD2}, {"t",   0xF2},
    {"U",   0xD3}, {"u",   0xF3},
    {"F",   0xD4}, {"f",   0xF4},
    {NULL, 0, 0}
};

void init_back_table(void) {
    for (int i = 0; back_table[i].latin; i++) {
        back_table[i].len = strlen(back_table[i].latin);
    }
}

int is_latin_letter(unsigned char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

unsigned char *detranslit_word(const char *word, int len) {
    unsigned char *out = malloc(len + 1);
    if (!out) return NULL;
    int out_pos = 0;
    int i = 0;
    while (i < len) {
        int matched = 0;
        for (int t = 0; back_table[t].latin; t++) {
            int l = back_table[t].len;
            if (i + l <= len && strncmp(word + i, back_table[t].latin, l) == 0) {
                out[out_pos++] = back_table[t].rus;
                i += l;
                matched = 1;
                break;
            }
        }
        if (!matched) {
            out[out_pos++] = word[i];
            i++;
        }
    }
    out[out_pos] = '\0';
    return out;
}

char **split_latin_words(const unsigned char *buf, size_t buf_len, int **lens, int *word_cnt) {
    char **words = NULL;
    int *lengths = NULL;
    int cnt = 0;
    int i = 0;
    while (i < buf_len) {
        while (i < buf_len && !is_latin_letter(buf[i])) i++;
        if (i >= buf_len) break;
        int start = i;
        while (i < buf_len && is_latin_letter(buf[i])) i++;
        int word_len = i - start;
        char *word = malloc(word_len + 1);
        if (!word) goto error;
        memcpy(word, buf + start, word_len);
        word[word_len] = '\0';
        words = realloc(words, (cnt + 1) * sizeof(char*));
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
    char **words = split_latin_words(buf, fsize, &lens, &word_cnt);
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
    snprintf(outname, sizeof(outname), "%s_rus.txt", fname);
    FILE *out = fopen(outname, "wb");
    if (!out) {
        perror("Cannot create output file");
        for (int i = 0; i < word_cnt; i++) free(words[i]);
        free(words);
        free(lens);
        return;
    }

    for (int i = 0; i < word_cnt; i++) {
        unsigned char *rus = detranslit_word(words[i], lens[i]);
        if (!rus) continue;
        int spaces = max_len - lens[i];
        for (int s = 0; s < spaces; s++) fputc(' ', out);
        fwrite(rus, 1, strlen((char*)rus), out);
        fputc('\n', out);
        free(rus);
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
    init_back_table();
    for (int i = 1; i < argc && i <= 1000; i++) {
        process_file(argv[i]);
    }
    return 0;
}