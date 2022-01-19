#include "action.h"

extern l_user *head_user;
extern l_oversold *head_oversold;
extern l_overbought *head_overbought;
extern l_stock *head_stock;

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
void remove_user(l_user **head, l_user *node) {
    l_user* temp = *head;
    l_user* prev = NULL;
    
    if (temp != NULL && temp == node)
    {
        *head = temp->next;
        temp = NULL;           
        return;
    } else {
        while (temp != NULL && temp != node)
        {
            prev = temp;
            temp = temp->next;
        }

        if (temp == NULL)
            return;

        prev->next = temp->next;

        temp = NULL;
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
        printf("%d %d %s %s %d %d\n", tmp->clientfd, tmp->id, tmp->username, tmp->password, tmp->balance, tmp->status);
        l_stock *temp = (l_stock *) malloc(sizeof(l_stock));
        temp = tmp->stock;
        while(temp != NULL) {
            printf("%s %d %d\n", temp->name, temp->price, temp->amount);
            temp = temp->next;
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

int sign_in(char username[MAX_CHAR], char password[MAX_CHAR], int clientfd) {
    l_user *tmp = get_account(username);
    if(check_pass(tmp, password) == TRUE) {
        tmp->status = TRUE;
        tmp->pass_incorrect = 0;
        tmp->is_online = TRUE;
        tmp->clientfd = clientfd;
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

void log_out(char username[MAX_CHAR]) {
    l_user *tmp = get_account(username);
    tmp->is_online = FALSE;
    tmp->clientfd = FALSE;
}

l_user* trade_user(char id[MAX_CHAR]) {
    l_user *tmp = head_user;
    char user_id[MAX_CHAR];
    while (tmp != NULL) {
        if (tmp->is_online == TRUE) {
            snprintf(user_id, MAX_CHAR,"%d", tmp->id);
            if (strcmp(user_id, id) == 0) {
                return tmp;
            } 
        }
        tmp = tmp->next;
    }
    return NULL;
}

char* user_stock_list(char id[MAX_CHAR]) {
    l_user *tmp = head_user;
    char user_id[MAX_CHAR], amount[MAX_CHAR];
    char* str = malloc(sizeof(char));
    while (tmp != NULL) {
        snprintf(user_id, MAX_CHAR,"%d", tmp->id);
        if (strcmp(user_id, id) == 0) {
            l_stock *temp = (l_stock *) malloc(sizeof(l_stock));
            temp = tmp->stock;
            while(temp != NULL) {
                strcat(str, "\n");
                strcat(str, temp->name);
                strcat(str, ": ");
                snprintf(amount, MAX_CHAR,"%d", temp->amount);
                strcat(str, amount);
                temp = temp->next;
            }
        }         
        tmp = tmp->next;
    }
    return str;
}

int direct_trade(l_user *current_user, l_user *trader,char stock_name[MAX_CHAR], int price, int type) {
    l_stock *temp = (l_stock *) malloc(sizeof(l_stock));
    if(type == 1) { //mua
        temp = trader->stock;
        while(temp != NULL) {
            if (strcmp(stock_name, temp->name) == 0) {
                if (price > 0) {
                    int amount = round(current_user->balance/price);
                    if (amount > temp->amount) {
                        return temp->amount;
                    }
                    return amount;
                }
                return TRUE;
            }
            temp = temp->next;
        }
    } else { //bán
        temp = current_user->stock;
        while(temp != NULL) {
            if (strcmp(stock_name, temp->name) == 0) {
                if (price > 0) {
                    return temp->amount;
                }
                return TRUE;
            }
            temp = temp->next;
        }
    }
    return FALSE;
}

char* online_users(l_user *current_user) {
    l_user *tmp = head_user;
    char *str = malloc(sizeof(char));
    char user_id[MAX_CHAR];
    while (tmp != NULL) {
        if (tmp->is_online == TRUE && tmp->id != current_user->id) {
            snprintf(user_id, MAX_CHAR,"%d", tmp->id);
            strcat(str, user_id);
            strcat(str, ". ");
            strcat(str, tmp->username);
            strcat(str, "\n");
        }
        tmp = tmp->next;
    }
    if (strlen(str) == 0) {
        str = "There is no online user, please wait ...\n";
    }
    return str;
}

l_oversold *create_oversold(char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount) {
    l_oversold *temp = (l_oversold *) malloc(sizeof(l_oversold));
    strcpy(temp->username, username);
    strcpy(temp->name_stock, name_stock);
    temp->price = price;
    temp->amount = amount;
    temp->next = NULL;
    return temp;
}

void add_oversold(l_oversold **head, l_oversold *node) {
    if( *head == NULL) {
        *head = node;
    } else {
        l_oversold *tmp = *head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = node;
    }
}

l_overbought *create_overbought(char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount) {
    l_overbought *temp = (l_overbought *) malloc(sizeof(l_overbought));
    strcpy(temp->username, username);
    strcpy(temp->name_stock, name_stock);
    temp->price = price;
    temp->amount = amount;
    temp->next = NULL;
    return temp;
}

void add_overbought(l_overbought **head, l_overbought *node) {
    if( *head == NULL) {
        *head = node;
    } else {
        l_overbought *tmp = *head;
        while (tmp->next != NULL) tmp = tmp->next;
        tmp->next = node;
    }
}

// check stock exist in oversold.txt 
int has_stock_in_oversold(char name_stock[MAX_CHAR]) { 
    l_oversold *tmp = head_oversold;
    while (tmp != NULL) {
        if (strcmp(tmp->name_stock, name_stock) == 0) {
            return TRUE;
        } 
        tmp = tmp->next;
    }
    return FALSE;
}

void print_list_oversold() {
    l_oversold *tmp = head_oversold;
    while (tmp != NULL) {
        printf("%s %s %d %d\n", tmp->username, tmp->name_stock, tmp->price, tmp->amount);
        tmp = tmp->next;
    }
}

void delete_list_stock(l_stock** head_ref){
   /* deref head_ref to get the real head */
   l_stock* current = *head_ref;
   l_stock* next;
 
   while (current != NULL)
   {
       next = current->next;
       free(current);
       current = next;
   }
   
   /* deref head_ref to affect the real head back
      in the caller. */
   *head_ref = NULL;
}

void delete_node_stock(l_stock *head, l_stock *n){
    // When node to be deleted is 
    // head node
    if(head == n){
        // Copy the data of next node to head
        head->price = head->next->price;
        head->amount = head->next->amount;
        strcpy(head->name, head->next->name);
        // store address of next node
        n = head->next;
        // Remove the link of next node
        head->next = head->next->next;
        // free memory
        free(n);
        return;
    }
    // When not first node, follow the 
    // normal deletion process
    // Find the previous node
    l_stock *prev = head;
    while(prev->next != NULL && 
          prev->next != n)
        prev = prev->next;
    // Check if node really exists in 
    // Linked List
    if(prev->next == NULL){
        printf("Given node is not present in Linked List");
        return;
    }
    // Remove node from Linked List
    prev->next = prev->next->next;
    // Free memory
    free(n);
    return; 
}

int delete_all_by_key(int key){
    int totalDeleted = 0;
    l_oversold *prev, *cur;

    /* Check if head node contains key */
    while (head_oversold != NULL && head_oversold->key == key)
    {
        // Get reference of head node
        l_stock *tmp = create_stock(head_oversold->name_stock, head_oversold->amount, head_oversold->price);
        add_stock(&head_stock, tmp);
        prev = head_oversold;

        // Adjust head node link
        head_oversold = head_oversold->next;

        // Delete prev since it contains reference to head node
        free(prev);

        totalDeleted++;
    }

    prev = NULL;
    cur  = head_oversold;

    /* For each node in the list */
    while (cur != NULL)
    {
        // Current node contains key
        if (cur->key == key)
        {
            // Adjust links for previous node
            l_stock *tmp = create_stock(cur->name_stock, cur->amount, cur->price);
            add_stock(&head_stock, tmp);
         
            if (prev != NULL) 
            {
                prev->next = cur->next;
            }

            // Delete current node
            free(cur);

            cur = prev->next;

            totalDeleted++;
        } 
        else
        {
            prev = cur;
            cur = cur->next;
        }        

    }

    return totalDeleted;
}

l_stock* search_stock_of_user(l_user **head_ref, char name_stock[MAX_CHAR], int price) {
    l_stock* temp = (*head_ref)->stock;
    while(temp != NULL) {
        if(strcmp(temp->name, name_stock) == 0 && temp->price == price) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

char* get_list_stock_of_user(char username[MAX_CHAR]) {
    l_user *tmp = head_user;
    char amount[MAX_CHAR];
    char* str = malloc(sizeof(char) * 1024);
    while (tmp != NULL) {
        if (strcmp(tmp->username, username) == 0) {
            l_stock *temp = (l_stock *) malloc(sizeof(tmp->stock));
            temp = tmp->stock;
            while(temp != NULL) {
                strcat(str, "\n");
                strcat(str, temp->name);
                strcat(str, ": ");
                snprintf(amount, MAX_CHAR,"%d", temp->amount);
                strcat(str, amount);
                temp = temp->next;
            }
        }         
        tmp = tmp->next;
    }
    return str;
}

// buy stock ( chuc nang 2 )

int has_stock_in_overbought(char name_stock[MAX_CHAR]) { 
    l_overbought *tmp = head_overbought;
    while (tmp != NULL) {
        if (strcmp(tmp->name_stock, name_stock) == 0) {
            return TRUE;
        } 
        tmp = tmp->next;
    }
    return FALSE;
}

int get_amount_from_stock(l_user **head_ref, char name_stock[MAX_CHAR]) {
    l_stock* temp = (*head_ref)->stock;
    while(temp != NULL) {
        if(strcmp(temp->name, name_stock) == 0) {
            return temp->amount;
        }
        temp = temp->next;
    }
    return 0; 
}

int delete_all_by_key_overbought(int key){
    int totalDeleted = 0;
    l_overbought *prev, *cur;

    /* Check if head node contains key */
    while (head_overbought != NULL && head_overbought->key == key)
    {
        // Get reference of head node
        l_stock *tmp = create_stock(head_overbought->name_stock, head_overbought->amount, head_overbought->price);
        add_stock(&head_stock, tmp);
        prev = head_overbought;

        // Adjust head node link
        head_overbought = head_overbought->next;

        // Delete prev since it contains reference to head node
        free(prev);

        totalDeleted++;
    }

    prev = NULL;
    cur  = head_overbought;

    /* For each node in the list */
    while (cur != NULL)
    {
        // Current node contains key
        if (cur->key == key)
        {
            // Adjust links for previous node
            l_stock *tmp = create_stock(cur->name_stock, cur->amount, cur->price);
            add_stock(&head_stock, tmp);
         
            if (prev != NULL) 
            {
                prev->next = cur->next;
            }

            // Delete current node
            free(cur);

            cur = prev->next;

            totalDeleted++;
        } 
        else
        {
            prev = cur;
            cur = cur->next;
        }        

    }

    return totalDeleted;
}

void direct_buy(l_order *order, int user_id) {
    l_user *tmp = head_user;
    while (tmp != NULL) {
        if (user_id == tmp->id) {
            l_stock *stock_buy =  search_stock_of_user(&tmp, order->stock_name, order->price);
            if(stock_buy == NULL) {
                add_stock(&(tmp->stock), create_stock(order->stock_name, order->amount, order->price));
            }else {
                stock_buy->amount += order->amount;
            }
            tmp->balance -= order->price * order->amount;
        }
        tmp = tmp->next;
    }
    write_file("file/users.txt");
}

void direct_sell(l_order *order, int user_id) {
    l_user *tmp = head_user;
    while (tmp != NULL) {
        if (user_id == tmp->id) {
            l_stock *stock_buy = search_stock_of_user(&tmp, order->stock_name, order->price);
            if(stock_buy->amount == 0) {
                delete_node_stock(tmp->stock,stock_buy);
            }
            tmp->balance += order->price * order->amount;
        }
        tmp = tmp->next;
    }
    write_file("file/users.txt");
}