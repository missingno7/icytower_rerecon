#ifndef ICYTOWER_BETA_H
#define ICYTOWER_BETA_H
#include <stdio.h>
typedef struct node {
    char email[128];
    char name[128];
    char code[16];
    struct node *next;
} Tbeta;
Tbeta *create_post(void);
void destroy_all(Tbeta *b);
void read_line(char *dst, FILE *fp);
Tbeta *load_plain_data(char *filename);
Tbeta *load_garbled_data(char *filename);
void garble_string(char *str, int n);
int save_garbled_data(Tbeta *b, char *filename);
#endif
