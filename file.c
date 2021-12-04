#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "action.h"
#include "define.h"

l_user *head_user = NULL;

int read_file(char *filename) {
    FILE *fo;
    fo = fopen(filename, "r");
    if (fo == NULL) {
        printf("Could not open file %s\n",filename);
        return 0;
    }
    char username[MAX_CHAR], password[MAX_CHAR];
    int id, balance, amount, price, status;
    char name_stock[MAX_CHAR];
    while(fscanf(fo, "%d %s %s %d %d", &id ,username, password, &balance, &status) != EOF ) {
        l_user *tmp = create_user(id, username, password, balance, status, NULL);
        char target[BUFFER];
        sprintf(target, "%s%s%s", "file/", username, ".txt");
        FILE *stock = fopen(target, "r");
        if(stock == NULL) {
            printf("Could not open file %s\n",target);
            return 0;
        }
        while(fscanf(stock, "%s %d %d", name_stock, &amount, &price) != EOF) {
            l_stock *stock1 = create_stock(name_stock, amount, price);
            add_stock(&(tmp->stock), stock1);
        }
        fclose(stock);
        add_user(&head_user, tmp);
    }
    fclose(fo);
    return 1;
}