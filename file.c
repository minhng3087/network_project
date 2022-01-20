#include <stdio.h>
#include <stdlib.h>
#include "file.h"
#include "action.h"

l_user *head_user = NULL;
l_overbought *head_overbought = NULL;
l_oversold *head_oversold = NULL;
l_stock *head_stock = NULL; // store stock to save file order_matching.txt
l_order *head_order = NULL;

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
        while(fscanf(stock, "%s %d %d", name_stock, &price, &amount) != EOF) {
            l_stock *stock1 = create_stock(name_stock, amount, price);
            add_stock(&(tmp->stock), stock1);
        }
        fclose(stock);
        add_user(&head_user, tmp);
    }
    fclose(fo);
    return 1;
}

void write_file(char *filename) {
    FILE* fo = fopen(filename, "w");
    if (fo == NULL) {
        printf("Could not open file %s\n",filename);
        return ;
    }
    l_user *tmp = head_user;
    while (tmp != NULL) {
        fprintf(fo, "%d %s %s %d %d\n", tmp->id, tmp->username, tmp->password, tmp->balance, tmp->status);
        char target[BUFFER];
        sprintf(target, "%s%s%s", "file/", tmp->username, ".txt");
        FILE *stock = fopen(target, "w");
        if(stock == NULL) {
            printf("Could not open file %s\n",target);
            return;
        }
        l_stock *info_stock = (l_stock *) malloc(sizeof(tmp->stock));
        info_stock = tmp->stock;
        while(info_stock != NULL) {
            fprintf(stock, "%s %d %d\n", info_stock->name, info_stock->price, info_stock->amount);
            info_stock = info_stock->next;
        }
        free(info_stock);
        fclose(stock);
        tmp = tmp->next;
    }
    fclose(fo);
}

// order 
int read_file_overbought() {
   FILE* fo = fopen("file/overbought.txt", "r");
   if (fo == NULL) {
       printf("Could not open file %s\n","overbought.txt");
       return 0;
   }
 
   char username[MAX_CHAR], name_stock[MAX_CHAR];
   int price, amount;
   while(fscanf(fo, "%s %s %d %d", username, name_stock, &price, &amount) != EOF) {
       l_overbought *tmp = create_overbought(username, name_stock, price, amount);
       add_overbought(&head_overbought, tmp);
   }
   fclose(fo);
   return 1;
}
 
int read_file_oversold() {
   FILE* fo = fopen("file/oversold.txt", "r");
   if (fo == NULL) {
       printf("Could not open file %s\n","oversold.txt");
       return 0;
   }
 
   char username[MAX_CHAR], name_stock[MAX_CHAR];
   int price, amount;
   while(fscanf(fo, "%s %s %d %d", username, name_stock, &price, &amount) != EOF) {
       l_oversold *tmp = create_oversold(username, name_stock, price, amount);
       add_oversold(&head_oversold, tmp);
   }
   fclose(fo);
   return 1;
}

void write_file_overbought() {
   FILE *fo;
   fo = fopen("file/overbought.txt", "w");
   if (fo == NULL) {
       printf("Could not open file %s\n","overbought.txt");
       return;
   }
  
   l_overbought *tmp = head_overbought;
   while (tmp != NULL) {
       fprintf(fo, "%s %s %d %d\n", tmp->username, tmp->name_stock, tmp->price, tmp->amount);
       tmp = tmp->next;
   }
   fclose(fo);
}
 
void write_file_oversold() {
   FILE *fo;
   fo = fopen("file/oversold.txt", "w");
   if (fo == NULL) {
       printf("Could not open file %s\n","oversold.txt");
       return;
   }
  
   l_oversold *tmp = head_oversold;
   while (tmp != NULL) {
       fprintf(fo, "%s %s %d %d\n", tmp->username, tmp->name_stock, tmp->price, tmp->amount);
       tmp = tmp->next;
   }
   fclose(fo);
}

void append_file_order_match() {
   FILE *fo = fopen("file/order_matching.txt", "a");
   l_stock *tmp = head_stock;
   while (tmp != NULL) {
       fprintf(fo, "%s %d %d\n", tmp->name, tmp->price, tmp->amount);
       tmp = tmp->next;
   }
   delete_list_stock(&head_stock);
   fclose(fo);
}

void write_node_to_overfile(char *filename, char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount) {
    FILE *fo = fopen(filename, "a");
   if (fo == NULL) {
       printf("Could not open file %s\n",filename);
       return;
   }
  
   fprintf(fo, "%s %s %d %d\n", username, name_stock, price, amount);
   fclose(fo);
}

void append_one_stock_to_order_match(char name_stock[MAX_CHAR], int price, int amount) {
    FILE *fo = fopen("file/order_matching.txt", "a");
   fprintf(fo, "%s %d %d\n", name_stock, price, amount);
   fclose(fo);

}




