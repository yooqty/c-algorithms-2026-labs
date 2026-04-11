#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024
#define MAX_FIELD 256

struct book {
    char author[MAX_FIELD];
    char title[MAX_FIELD];
    char publisher[MAX_FIELD];
};


struct tnode {
    struct book *data;
    int count;
    struct tnode *lchild, *rchild;
};


int strncmp_cp1251(const char *s1, const char *s2, size_t n) {
    if (n == 0) return 0;
    unsigned char c1, c2;
    for (size_t i = 0; i < n; i++) {
        c1 = (unsigned char)s1[i];
        c2 = (unsigned char)s2[i];
        if (c1 == 0 && c2 == 0) return 0;
        if (c1 == 0) return -1;
        if (c2 == 0) return 1;
        if (c1 == 168) c1 = 166;
        if (c2 == 168) c2 = 166;
        if (c1 == 184) c1 = 182;
        if (c2 == 184) c2 = 182;
        if (c1 != c2) return (int)c1 - (int)c2;
    }
    return 0;
}

int compare_books(const struct book *a, const struct book *b) {
    int cmp = strncmp_cp1251(a->author, b->author, MAX_FIELD);
    if (cmp != 0) return cmp;
    return strncmp_cp1251(a->title, b->title, MAX_FIELD);
}

struct book *create_book(const char *author, const char *title, const char *publisher) {
    struct book *b = (struct book*)malloc(sizeof(struct book));
    if (!b) { perror("malloc"); exit(1); }
    strncpy(b->author, author, MAX_FIELD-1); b->author[MAX_FIELD-1] = '\0';
    strncpy(b->title, title, MAX_FIELD-1);   b->title[MAX_FIELD-1] = '\0';
    strncpy(b->publisher, publisher, MAX_FIELD-1); b->publisher[MAX_FIELD-1] = '\0';
    return b;
}

struct tnode *tree_insert(struct tnode *p, struct book *newbook) {
    if (p == NULL) {
        p = (struct tnode*)malloc(sizeof(struct tnode));
        if (!p) { perror("malloc"); exit(1); }
        p->data = newbook;
        p->count = 1;
        p->lchild = p->rchild = NULL;
        return p;
    }
    int cmp = compare_books(newbook, p->data);
    if (cmp < 0)
        p->lchild = tree_insert(p->lchild, newbook);
    else if (cmp > 0)
        p->rchild = tree_insert(p->rchild, newbook);
    else {
        p->count++;
        free(newbook);
    }
    return p;
}

void tree_print_inorder(FILE *out, struct tnode *p) {
    if (p != NULL) {
        tree_print_inorder(out, p->lchild);
        for (int i = 0; i < p->count; i++) {
            fprintf(out, "Author: %s\nTitle: %s\nPublisher: %s\n---\n",
                    p->data->author, p->data->title, p->data->publisher);
        }
        tree_print_inorder(out, p->rchild);
    }
}

void tree_search_prefix(struct tnode *p, const char *prefix, int n, FILE *out) {
    if (p == NULL) return;
    tree_search_prefix(p->lchild, prefix, n, out);
    if (strncmp_cp1251(p->data->author, prefix, n) == 0 ||
        strncmp_cp1251(p->data->title, prefix, n) == 0) {
        for (int i = 0; i < p->count; i++) {
            fprintf(out, "Found: Author=%s, Title=%s\n", p->data->author, p->data->title);
        }
    }
    tree_search_prefix(p->rchild, prefix, n, out);
}

void tree_free(struct tnode *p) {
    if (p != NULL) {
        tree_free(p->lchild);
        tree_free(p->rchild);
        free(p->data);
        free(p);
    }
}


int parse_bib_entry(const char *entry, char *author, char *title, char *publisher) {
    author[0] = title[0] = publisher[0] = '\0';
    const char *p;
    p = strstr(entry, "author");
    if (p) { p = strchr(p, '{'); if (p) { p++; const char *end = strchr(p, '}'); if (end) { int len = (end-p)<MAX_FIELD-1 ? (end-p) : MAX_FIELD-1; strncpy(author, p, len); author[len]='\0'; } } }
    p = strstr(entry, "title");
    if (p) { p = strchr(p, '{'); if (p) { p++; const char *end = strchr(p, '}'); if (end) { int len = (end-p)<MAX_FIELD-1 ? (end-p) : MAX_FIELD-1; strncpy(title, p, len); title[len]='\0'; } } }
    p = strstr(entry, "publisher");
    if (p) { p = strchr(p, '{'); if (p) { p++; const char *end = strchr(p, '}'); if (end) { int len = (end-p)<MAX_FIELD-1 ? (end-p) : MAX_FIELD-1; strncpy(publisher, p, len); publisher[len]='\0'; } } }
    return (author[0] != '\0' && title[0] != '\0');
}

void process_file(const char *filename, struct tnode **root) {
    FILE *f = fopen(filename, "r");
    if (!f) { fprintf(stderr, "Cannot open: %s\n", filename); return; }
    char line[MAX_LINE];
    char buffer[65536];
    int in_entry = 0, pos = 0;
    buffer[0] = '\0';
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '@') {
            if (in_entry) {
                buffer[pos] = '\0';
                char author[MAX_FIELD], title[MAX_FIELD], publisher[MAX_FIELD];
                if (parse_bib_entry(buffer, author, title, publisher)) {
                    struct book *b = create_book(author, title, publisher);
                    *root = tree_insert(*root, b);
                }
                pos = 0; buffer[0] = '\0';
            }
            in_entry = 1;
        }
        if (in_entry) {
            int len = strlen(line);
            if (pos + len < sizeof(buffer)-1) {
                strcpy(buffer + pos, line);
                pos += len;
            } else { in_entry = 0; pos = 0; }
        }
    }
    if (in_entry && pos > 0) {
        buffer[pos] = '\0';
        char author[MAX_FIELD], title[MAX_FIELD], publisher[MAX_FIELD];
        if (parse_bib_entry(buffer, author, title, publisher)) {
            struct book *b = create_book(author, title, publisher);
            *root = tree_insert(*root, b);
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    const char *default_files[] = {"0000_0000.bib", "0000_0001.bib", "0000_0002.bib"};
    int n_default = 3;
    struct tnode *root = NULL;

    if (argc >= 2) {
        for (int i = 1; i < argc; i++) process_file(argv[i], &root);
    } else {
        for (int i = 0; i < n_default; i++) process_file(default_files[i], &root);
    }

    FILE *out = fopen("output.txt", "w");
    if (!out) { perror("fopen output.txt"); tree_free(root); return 1; }
    tree_print_inorder(out, root);
    fclose(out);
    printf("Sorted bibliography written to output.txt\n");

    char prefix[10];
    printf("Enter prefix (first 5 chars) to search in authors/titles: ");
    if (fgets(prefix, sizeof(prefix), stdin)) {
        size_t len = strlen(prefix);
        if (len > 0 && prefix[len-1] == '\n') prefix[len-1] = '\0';
        FILE *search_out = fopen("search_results.txt", "w");
        if (search_out) {
            tree_search_prefix(root, prefix, 5, search_out);
            fclose(search_out);
            printf("Search results saved to search_results.txt\n");
        } else {
            perror("fopen search_results.txt");
        }
    }

    tree_free(root);
    return 0;
}