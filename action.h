#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#include <math.h>

typedef struct stock{
    char name[MAX_CHAR];
    int price;
    int amount;
    struct stock *next; 
}l_stock;

typedef struct user{
    int clientfd; // thread_id
    int id;
    char username[MAX_CHAR];
    char password[MAX_CHAR];
    int balance;
    int status;
    int is_online;
    int pass_incorrect;
    l_stock *stock;
    struct user *next;
}l_user;

typedef struct oversold{
    int key;
    char username[MAX_CHAR];
    char name_stock[MAX_CHAR];
    int price;
    int amount;
    struct oversold *next; 
}l_oversold;

typedef struct overbought{
    int key;
    char username[MAX_CHAR];
    char name_stock[MAX_CHAR];
    int price;
    int amount;
    struct overbought *next; 
}l_overbought;

l_user *create_user(int id, char username[MAX_CHAR], char password[MAX_CHAR], int balance, int status, l_stock* stock);
l_stock *create_stock(char name[MAX_CHAR], int amount, int price);
void add_stock(l_stock **stock, l_stock *node);
void add_user(l_user **head, l_user *node);

void print_list();
void error(char *s);

l_user *current_user;
l_user *get_account(char username[MAX_CHAR]);
int sign_in(char username[MAX_CHAR], char password[MAX_CHAR], int clientfd);
int check_pass(l_user *account, char password[MAX_CHAR]);
int has_account(char username[MAX_CHAR]);
void log_out(char username[MAX_CHAR]);
l_user *trade_user(char id[MAX_CHAR]);
char* user_stock_list(char id[MAX_CHAR]);
int direct_trade(l_user *current_user, l_user *trader, char stock_name[MAX_CHAR], int price, int type);
char* online_users(l_user *current_user);
char* get_list_stock_of_user(char username[MAX_CHAR]);

// order 
// buy stock
l_oversold *create_oversold(char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount);
void add_oversold(l_oversold **head, l_oversold *node);
l_overbought *create_overbought(char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount);
void add_overbought(l_overbought **head, l_overbought *node);
int has_stock_in_oversold(char name_stock[MAX_CHAR]);

void print_list_overbought();
void delete_list_stock(l_stock** head_ref);
int delete_all_by_key(int key);

l_stock* search_stock_of_user(l_user *head_ref, char name_stock[MAX_CHAR], int price);
void delete_node_stock(l_stock *head, l_stock *n);

// end buy_stock

// sell stock
int has_stock_in_overbought(char name_stock[MAX_CHAR]);
int get_amount_from_stock(l_user **head_ref, char name_stock[MAX_CHAR]);
int delete_all_by_key_overbought(int key);
// end sell stock

l_user* online_user;