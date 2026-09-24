/* Historical beta.c recovered from DWARF and disassembly. */
#include <stdio.h>
#include <allegro.h>
#include "beta.h"

Tbeta *create_post()
{
    Tbeta *b = malloc(sizeof(Tbeta));
    int i;
    for (i = 0; i < 128; i++) {
        b->name[i] = rand() % 256;
        b->email[i] = rand() % 256;
        b->code[i % 16] = rand() % 256;
    }
    b->next = NULL;
    return b;
}

void destroy_all(Tbeta *b)
{
    Tbeta *c = b->next;
    free(b);
    while (c) {
        b = c->next;
        free(c);
        c = b;
    }
}

void read_line(char *dst, FILE *fp)
{
    char b = ' ';
    int i = 0;
    while (b != '\n') {
        fread(&b, 1, 1, fp);
        dst[i++] = b;
    }
    dst[i] = 0;
}

Tbeta *load_plain_data(char *filename)
{
    Tbeta *b = create_post();
    Tbeta *head = b;
    int i;
    FILE *fp = fopen(filename, "rb");
    for (i=0; i<9; i++) {
        read_line(b->name, fp);
        read_line(b->email, fp);
        read_line(b->code, fp);
        if (i < 8) { b = b->next = create_post(); }
        
    }
    fclose(fp);
    return head;
}

Tbeta *load_garbled_data(char *filename)
{
    Tbeta *head;
    Tbeta *b;
    int i, j;
    int true_check, check;
    head = b = create_post();
    FILE *fp = fopen(filename, "rb");
    true_check = 0;
    for (j = 0; j < 9; j++) {
        fread(b->name, 128, 1, fp);
        fread(b->email, 128, 1, fp);
        fread(b->code, 16, 1, fp);
        for (i = 0; i < 128; i++)
            true_check = true_check * (i + 1) + b->email[i] + b->name[i] + b->code[i % 16];
        if (j != 8) {
            b = b->next = create_post();
        }
    }
    fread(&check, 1, 4, fp);
    fclose(fp);
    if (check != true_check) {
        destroy_all(head);
        head = NULL;
    }
    return head;
}

void garble_string(char *str, int n)
{
    int i;
    for (i = 0; i < n; i++)
        str[i] ^= (959 + i * 137) % 256;
}

int save_garbled_data(Tbeta *b, char *filename)
{
    FILE *fp = fopen(filename, "wb");
    int check = 0;
    int i;
    if (!fp) return 0;
    do {
        fwrite(b->name, 128, 1, fp);
        fwrite(b->email, 128, 1, fp);
        fwrite(b->code, 16, 1, fp);
        for (i = 0; i < 128; i++)
            check = check * (i + 1) + b->email[i] + b->name[i] + b->code[i % 16];
        b = b->next;
    } while (b);
    fwrite(&check, 1, 4, fp);
    fclose(fp);
    return 1;
}
