#include "action.h"

extern l_user *head_user;

void error(char *s){
    perror(s);
    exit(1);
}

l_user *create_user(int id, char username[MAX_CHAR], char password[MAX_CHAR], int balance, int status, l_stock* stock) {
    l_user *temp = (l_user *) malloc(sizeof(l_user));
    strcpy(temp->username, username);
    strcpy(temp->password, password);
    temp->id = id;
    temp->balance = balance;
    temp->is_online = FALSE;
    temp->pass_incorrect = 0;
    temp->status = status;
    temp->next = NULL;
    return temp;
}

void add_user(l_user **head, l_user *node) {
    if( *head == NULL) {
        *head = node;
    } else {
        l_user *tmp = *head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = node;
    }
}

l_stock *create_stock(char name[MAX_CHAR], int amount, int price) {
    l_stock *temp = (l_stock *) malloc(sizeof(l_stock));
    strcpy(temp->name, name);
    temp->amount = amount;
    temp->price = price;
    temp->next = NULL;
    return temp;
}

void add_stock(l_stock **head, l_stock *node) {
    if( *head == NULL) {
        *head = node;
    } else {
        l_stock *tmp = *head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = node;
    }
}

void print_list() {
    l_user *tmp = head_user;
    while (tmp != NULL) {
        printf("%d %s %s %d %d\n", tmp->id, tmp->username, tmp->password, tmp->balance, tmp->status);
        while(tmp->stock != NULL) {
            printf("%s %d %d\n", tmp->stock->name, tmp->stock->price, tmp->stock->amount);
            tmp->stock = tmp->stock->next;
        }
        tmp = tmp->next;
    }
}

l_user *get_account(char username[MAX_CHAR]) {
    l_user *tmp = head_user;
    while (tmp != NULL) {
        if (strcmp(tmp->username, username) == 0) {
            return tmp;
        }
        tmp = tmp->next;
    }
    return NULL;
}

int check_pass(l_user *account, char password[MAX_CHAR]) {
    return strcmp(account->password, password) == 0 ? TRUE : FALSE;
}

int sign_in(char username[MAX_CHAR], char password[MAX_CHAR]) {
    l_user *tmp = get_account(username);
    
    if(check_pass(tmp, password) == TRUE) {
        tmp->status = TRUE;
        tmp->pass_incorrect = 0;
        // login success
        return 1;
    } else {
        tmp->pass_incorrect++;
        if (tmp->pass_incorrect >= LIMIT_INCORRECT) {
            tmp->status = BLOCK;
            // account blocked
            return 2;
        } else {
            // password wrong
            return 3;
        }
    }
}
int has_account(char username[MAX_CHAR]) {
    l_user *tmp = head_user;
    while (tmp != NULL) {
        if (strcmp(tmp->username, username) == 0) {
            return TRUE;
        } 
        tmp = tmp->next;
    }
    return FALSE;
}

