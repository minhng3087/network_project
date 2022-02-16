#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <stdint.h>
#include "file.h"
#include "action.h"
#include "utils/handle.h"


extern l_user *head_user;
extern l_stock *head_stock;
extern l_oversold *head_oversold;
extern l_overbought *head_overbought;
extern l_order *head_order;

int flag_delete = FALSE;
int temp_balance = 0;
int temp_amount = 0;
int sell_flag = 0;
int count_same = 0;
int list_trade[2];
int direct_buy_flag = 0;
int direct_sell_flag = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void answer(int confd, char *message, SignalState signal) {
    char string[1000] = "";
    strcat(string, message);
    addToken(string, signal);
    if (send(confd, string, strlen(string), 0) <= 0) {
        printf("Error by send function\n");
    };
}

void manage_profile_account(int clientfd, char username[MAX_CHAR]) {
    l_user *user =  get_account(username);
    char target[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    l_stock *stock = user->stock;
    // strcpy(response, "ID NAME BLANCE STATUS\n");
    // sprintf(target, "%d %s %d %d\n", user->id, user->username, user->balance, user->status);
    // strcat(response, target);
    // strcat(response, "List stock\n");
    // while(stock != NULL) {
    //     sprintf(target, "%s %d\n", stock->name, stock->price);
    //     strcat(response, target);
    //     stock = stock->next;
    // }
    strcpy(response, username);
   // strcat(response, "Press q to quit");
    answer(clientfd, response, SUCCESS_SIGNAL);
    //send(clientfd, response, strlen(response), 0);  
}

void buy_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]) {
    // pthread_mutex_lock(&lock);
    char name_stock[MAX_CHAR], response[BUFFER_SIZE];
    char price_str[MAX_CHAR];
    char amount_str[MAX_CHAR];
    int price, amount, amount2;
    int tokenCount;
    char **data = words(request, &tokenCount, "|\n");
    strcpy(name_stock, data[0]);
    strcpy(price_str, data[1]);
    strcpy(amount_str, data[2]);
    amount = atoi(amount_str);
    price = atoi(price_str);
    amount2 = amount;
    int signal;
    l_user *info = get_account(username); //current_user
    if(info != NULL && info->balance < amount * price) {
        strcpy(response, "Balance not enough! ");
        strcat(response, "Input price again");
    }else {
        // case stock not exist in overbought.txt and price < price of stock => save to file overbought.txt
        if(has_stock_in_oversold(name_stock) == FALSE) {
            write_node_to_overfile("file/overbought.txt", username, name_stock, price, amount);
            add_overbought(&head_overbought, create_overbought(username, name_stock, price, amount));
            sprintf(response, "%s đã đặt lệnh mua thành công %s giá %d số lượng %d", username, name_stock, price, amount);
            signal = REQUEST_BUY_SUCCESS_SIGNAL;
            //answer(clientfd, response, REQUEST_BUY_SUCCESS_SIGNAL);
           // return;
        }
        // case stock exist in overbought.txt
        else {
            l_oversold *tmp = head_oversold;
            while(tmp != NULL) {
                if (strcmp(tmp->name_stock, name_stock) == 0 && price >= tmp->price && strcmp(tmp->username, info->username) != 0) {
                    l_user* seller = get_account(tmp->username);
                    l_stock *stock_sell =  search_stock_of_user(&seller, name_stock, tmp->price);
                    l_stock *search_stock = search_stock_of_user(&info, name_stock, tmp->price);
                    char *str = (char *) malloc(sizeof(char)*1024);
                    if(tmp->amount <= amount) {
                        temp_amount = tmp->amount;
                        tmp->key = TRUE;
                        temp_balance += tmp->price * tmp->amount;
                        seller->balance += tmp->price * tmp->amount;
                        if (stock_sell != NULL) {
                            stock_sell->amount -= tmp->amount;
                        }
                        flag_delete = TRUE;
                        if(search_stock == NULL) {
                            add_stock(&(info->stock), create_stock(name_stock, tmp->amount, tmp->price));
                        }else {
                            search_stock->amount += tmp->amount;
                        }
                        sprintf(str, "Bán thành công %s giá %d số lương %d", tmp->name_stock, tmp->price, tmp->amount);
                        signal = SUCCESS_SIGNAL;
                        answer(seller->clientfd, str, SUCCESS_SIGNAL);
                       // send(seller->clientfd, str, strlen(str), 0);
                        free(str);
                    //    return;
                    }else if(tmp->amount > amount) {
                        
                        temp_amount = tmp->amount;
                        tmp->amount = abs(tmp->amount - amount);
                        temp_balance += tmp->price * amount;
                        seller->balance += tmp->price * amount;
                        // update amount stock of seller
                        if (stock_sell->amount != NULL) {
                            stock_sell->amount -= amount;
                        }
                        // L15 100 5 => L15 100 4 -> add L15 100 1 to LL
                        
                        l_stock* temp = create_stock(tmp->name_stock, amount, tmp->price);
                        add_stock(&head_stock, temp);

                        if(search_stock == NULL) {
                            add_stock(&(info->stock), temp);
                        }else {
                            search_stock->amount += amount;
                        }
                        sprintf(str, "Bán thành công %s giá %d số lượng %d", tmp->name_stock, tmp->price, tmp->amount);
                        signal = SUCCESS_SIGNAL;
                        answer(seller->clientfd, str, SUCCESS_SIGNAL);
                      //  send(seller->clientfd, str, strlen(str), 0);
                        free(str);
                        //return;
                    }
                    if(stock_sell->amount == 0) {
                        delete_node_stock(seller->stock,stock_sell);
                    }
                    amount -= temp_amount;
                    // count_same++;
                }
                tmp = tmp->next;
            }
            
            if(amount > 0) {
                write_node_to_overfile("file/overbought.txt", username, name_stock, price, amount);
            }
            if(count_same == 1) {
                append_one_stock_to_order_match(name_stock, price, amount2);
            }
            if(flag_delete == TRUE) {
                delete_all_by_key(TRUE);
                append_file_order_match();
            }
            write_file_oversold();
            info->balance -= temp_balance;
            // add new stock to buyer 
            write_file("file/users.txt");
            strcpy(response, "Mua thành công");
            signal = SUCCESS_SIGNAL;
            // strcat(response, "Press q to quit");
            // reset
            count_same = 0;
            flag_delete = FALSE;
        }
    }
    //send(clientfd, response, strlen(response), 0);
    answer(clientfd, response, signal);
    //  pthread_mutex_unlock(&lock);
}

void sell_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]) {
    char name_stock[MAX_CHAR], response[BUFFER_SIZE];
    char price_str[MAX_CHAR];
    char amount_str[MAX_CHAR];
    int price, amount, amount2;
    int tokenCount;
    char **data = words(request, &tokenCount, "|\n");
    strcpy(name_stock, data[0]);
    strcpy(price_str, data[1]);
    strcpy(amount_str, data[2]);
    amount = atoi(amount_str);
    price = atoi(price_str);
    amount2 = amount;
    int signal;
    l_user *info = get_account(username); //current_user
    if(amount > get_amount_from_stock(&info, name_stock)) {
        strcpy(response, "Amount not enough");
        strcpy(response, "Input amount again");
        signal = FAILED_SIGNAL;
        sell_flag = 3;
    }else {
        if(has_stock_in_overbought(name_stock) == FALSE) {
            write_node_to_overfile("file/oversold.txt", username, name_stock, price, amount);
            add_oversold(&head_oversold, create_oversold(username, name_stock, price, amount));
            sprintf(response, "%s đã đặt lệnh bán thành công %s giá %d số lượng %d", username, name_stock, price, amount);
            signal = REQUEST_SELL_SUCCESS_SIGNAL;
           // answer(clientfd, response, REQUEST_SELL_SUCCESS_SIGNAL);
            // strcpy(response, "Order Match Success!!!\n");
            // strcat(response, "Press q to quit");
           // return;
        }else {
            l_overbought *tmp = head_overbought;
            while(tmp != NULL) {
                if (strcmp(tmp->name_stock, name_stock) == 0 && price <= tmp->price && strcmp(tmp->username, info->username) != 0) {
                    l_user* buyer = get_account(tmp->username);
                    l_stock *stock_buy =  search_stock_of_user(&buyer, name_stock, tmp->price);
                    l_stock *search_stock = search_stock_of_user(&info, name_stock, tmp->price);
                    char *str = (char *) malloc(sizeof(char) * 1024);
                    if(tmp->amount <= amount) {
                        temp_amount = tmp->amount;
                        tmp->key = TRUE;
                        temp_balance += tmp->price * tmp->amount;
                        buyer->balance -= tmp->price * tmp->amount;
                        flag_delete = TRUE;
                        if (stock_buy != NULL) {
                            stock_buy->amount += tmp->amount;
                        }else {
                            add_stock(&(buyer->stock), create_stock(name_stock,  tmp->amount, tmp->price));
                        }
                        // if(search_stock == NULL) {
                        //     add_stock(&(info->stock), create_stock(name_stock, tmp->amount, tmp->price));
                        // }else {
                        //     search_stock->amount += tmp->amount;
                        // }
                        search_stock->amount -= tmp->amount;
                        sprintf(str, "Mua thành công %s giá %d số lượng %d", tmp->name_stock, tmp->price, tmp->amount);
                        signal = SUCCESS_SIGNAL;
                        answer(buyer->clientfd, str, SUCCESS_SIGNAL);
                       // send(buyer->clientfd, str, strlen(str), 0);
                        free(str);
                      //  return;
                    }else if(tmp->amount > amount) {
                        temp_amount = tmp->amount;
                        tmp->amount = abs(tmp->amount - amount);
                        temp_balance += tmp->price * amount;
                        buyer->balance -= tmp->price * amount;
                        // update amount stock of buyer
                        if (stock_buy != NULL) {
                            stock_buy->amount += amount;
                        }else {
                            add_stock(&(buyer->stock), create_stock(name_stock, tmp->amount, tmp->price));
                        }
                        // L15 100 5 => L15 100 4 -> add L15 100 1 to LL
                        
                        add_stock(&head_stock, create_stock(tmp->name_stock, amount, tmp->price));
                        search_stock->amount -= amount;
                        sprintf(str, "Mua thành công %s giá %d số lượng %d", tmp->name_stock, tmp->price, tmp->amount);
                        signal = SUCCESS_SIGNAL;
                        answer(buyer->clientfd, str, SUCCESS_SIGNAL);
                       // send(buyer->clientfd, str, strlen(str), 0);
                        free(str);
                       // return;
                    }
                    
                    amount -= temp_amount;
                    // count_same++;
                   // send(buyer->clientfd, str, strlen(str), 0);
                   // free(str);
                }
                tmp = tmp->next;
            }
            
            if(amount > 0) {
                write_node_to_overfile("file/oversold.txt", username, name_stock, price, amount);
            }
            if(count_same == 1) {
                append_one_stock_to_order_match(name_stock, price, amount2);
            }
            if(flag_delete == TRUE) {
                delete_all_by_key_overbought(TRUE);
                append_file_order_match();
            }
            write_file_overbought();
            info->balance += temp_balance;
            // add new stock to buyer 
            write_file("file/users.txt");
            strcpy(response, "Bán thành công");
            signal = SUCCESS_SIGNAL;
            // reset
            count_same = 0;
            sell_flag = 1;
            flag_delete = FALSE; 
        }
    }
    answer(clientfd, response, signal);
    // send(clientfd, response, strlen(response), 0);
    // pthread_mutex_unlock(&lock);
}

void accept_trade()
{
    char response[BUFFER_SIZE];
    strcpy(response, "Giao dịch thành công");
    // strcat(response, "Press q to quit");
    if (head_order->type == 0) {
        direct_buy(head_order, list_trade[0]);
        direct_sell(head_order, list_trade[1]);
    } else {
        direct_buy(head_order, list_trade[1]);
        direct_sell(head_order, list_trade[0]);
    }
    answer(list_trade[0], response, YES_SIGNAL);
    answer(list_trade[1], response, YES_SIGNAL);
}
void refuse_trade()
{
    char response[BUFFER_SIZE];
    strcpy(response, "Giao dịch thất bại");
    // strcat(response, "Press q to quit");
    answer(list_trade[0], response, NO_SIGNAL);
    answer(list_trade[1], response, NO_SIGNAL);
    // send(list_trade[0], response, strlen(response), 0);
    // send(list_trade[1], response, strlen(response), 0);
}

void direct_sell_stock(int clientfd, char request[BUFFER_SIZE], l_user *current, l_user *trader) {
    // pthread_mutex_lock(&lock);
    l_order *order_direct = malloc(sizeof(l_order));
    char name_stock[MAX_CHAR], response[BUFFER_SIZE];
    char price_str[MAX_CHAR];
    char user_id_tmp[MAX_CHAR];
    int user_id = 0;
    char amount_str[MAX_CHAR];
    int price, stock_amount, amount;
    int tokenCount;
    char **data = words(request, &tokenCount, "|\n");
    strcpy(user_id_tmp, data[0]);
    user_id = atoi(user_id_tmp);
    strcpy(name_stock, data[1]);
    strcpy(price_str, data[2]);
    strcpy(amount_str, data[3]);
    price = atoi(price_str);
    stock_amount = direct_trade(current, trader, name_stock, price, 2);
    amount = atoi(amount_str);
    char direct_order[BUFFER_SIZE];
    char target[BUFFER];
    int signal;
    // strcpy(request,current->username);
    // strcat(request,trader->username);
    // strcat(request,name_stock);
    if (direct_trade(current, trader, name_stock, 0, 2) == FALSE) {
        strcpy(response, "Stock not found\n");
    } else if (amount > stock_amount) {
        strcpy(response, "You don't have enough amount of stock \n");
    } else if (amount < 0) {
        strcpy(response, "Invalid amount \n");
    } else {
        strcpy(response, "Vui lòng chờ...");
        strcat(direct_order, current->username);
        strcat(direct_order, " muốn bán: ");
        sprintf(target, "cổ phiếu %s với số lượng %d kèm giá: %d VND", name_stock, amount, price);
        strcpy(order_direct->stock_name, name_stock);
        order_direct->user_id = current->id;
        order_direct->amount = amount;
        order_direct->price = price;
        order_direct->type = 1;
        head_order = order_direct;
        strcat(direct_order, target);
        answer(trader->clientfd, direct_order, REQUEST_SELL_NOTIFY_SIGNAL);
        //send(trader->clientfd, direct_order, strlen(direct_order), 0);
    }

    //send(clientfd, response, strlen(response), 0);
    // pthread_mutex_unlock(&lock);
}

void direct_buy_stock(int clientfd, char request[BUFFER_SIZE], l_user *current, l_user *trader) {
    // pthread_mutex_lock(&lock);
    // printf("hi:%s", request);
    l_order *order_direct = malloc(sizeof(l_order));
    char user_id_tmp[MAX_CHAR];
    int user_id = 0;
    char name_stock[MAX_CHAR], response[BUFFER_SIZE];
    char price_str[MAX_CHAR];
    char amount_str[MAX_CHAR];
    int price, stock_amount, amount;
    int tokenCount;
    char **data = words(request, &tokenCount, "|\n");
    strcpy(user_id_tmp, data[0]);
    user_id = atoi(user_id_tmp);
    strcpy(name_stock, data[1]);
    strcpy(price_str, data[2]);
    strcpy(amount_str, data[3]);
    price = atoi(price_str);
    stock_amount = direct_trade(current, trader, name_stock, price, 1);
    amount = atoi(amount_str);
    char direct_order[BUFFER_SIZE];
    char target[BUFFER];
    int signal;
    // strcpy(request,current->username);
    // strcat(request,trader->username);
    // strcat(request,name_stock);
    // send(clientfd, request, strlen(request), 0);
    if (direct_trade(current, trader, name_stock, 0, 1) == FALSE) {
        strcpy(response, "Stock not found\n");
    } else if (stock_amount == 0) {
        strcpy(response, "Your balance is not enough \n");
    } else if (amount > stock_amount) {
        strcpy(response, "Invalid amount \n");
    } else if (amount < 0) {
        strcpy(response, "Invalid amount \n");
    } else {
        strcpy(response, "Vui lòng chờ...");
        strcat(direct_order, current->username);
        strcat(direct_order, " muốn mua: ");
        sprintf(target, "cổ phiếu %s với số lượng %d kèm giá: %d VND", name_stock, amount, price);
        strcpy(order_direct->stock_name, name_stock);
        order_direct->user_id = current->id;
        order_direct->amount = amount;
        order_direct->price = price;
        order_direct->type = 0;
        head_order = order_direct;
        strcat(direct_order, target);
        printf("%s",direct_order);
        answer(trader->clientfd, direct_order, REQUEST_BUY_NOTIFY_SIGNAL);
    //    send(trader->clientfd, direct_order, strlen(direct_order), 0);
    }

    // send(clientfd, response, strlen(response), 0);
    // pthread_mutex_unlock(&lock);
}

void direct(int clientfd, char request[BUFFER_SIZE], l_user *current, l_user *trader, int check_action) {
    switch(check_action) {
        case 1:
            direct_buy_stock(clientfd, request, current, trader);
            break;
        case 2: 
            direct_sell_stock(clientfd, request, current, trader);
            break;
        case 3:
            send(clientfd, "end", strlen("end"), 0);
            direct_buy_flag = 0;
            direct_sell_flag = 0;
            break;
    }
}


void order(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR], int check_action) {
    switch(check_action) {
        case 1:
            buy_stock(clientfd, request, username);
            break;
        case 2: 
            sell_stock(clientfd, request, username);
            break;
        case 3:
            send(clientfd, "end", strlen("end"), 0);
            sell_flag = 0;
            break;
    }
}

void *client_handler(void *arg){
    int clientfd, n, flag = 1, check = 0, buy = 0, sell = 0, flag_main,
        check_order = 0, check_action = 0, check_direct_action = 0 ;
    // l_order *order_direct = malloc(sizeof(l_order));

    char buff[BUFFER_SIZE], response[BUFFER_SIZE], username[MAX_CHAR], password[MAX_CHAR];
    char order_buff[BUFFER_SIZE];
    char stock_name[MAX_CHAR], price[MAX_CHAR], trader_id[MAX_CHAR], amount[MAX_CHAR], type[MAX_CHAR];
    l_user *trader;
    pthread_detach(pthread_self());
    clientfd = (intptr_t) arg;
    while(1) {
        n = recv(clientfd, buff, BUFFER_SIZE, 0);
        buff[n] = '\0';
        if (buff[strlen(buff) - 1] == '\n') buff[strlen(buff) - 1] = 0;
        printf("String received from client: %s\n", buff);
        if(strlen(buff) == 0) {
            printf("bye %d\n", clientfd);
            close(clientfd);
            return 0;
        }
        // copy order(name_stock, price, amount)
        strcpy(order_buff, buff);
        int tokenCount;
        char **data = words(buff, &tokenCount, "|\n");
        SignalState SIGNAL = data[tokenCount-1][0] - '0';
        switch(SIGNAL){ 
            case LOGIN_SIGNAL: {
                if (tokenCount == 3) {
                    strcpy(username, data[0]);
                    strcpy(password, data[1]);
                    if(has_account(username) == TRUE) {
                        int status = sign_in(username, password, clientfd);
                        if(status == 1) {
                            strcpy(response, LOGIN_SUCCESS);
                            flag = 3;
                            answer(clientfd, response, SUCCESS_SIGNAL);
                            break;
                        }else if(status == 3) {
                            strcpy(response, PASSWORD_WRONG);
                            flag = 1;
                            answer(clientfd, response, FAILED_SIGNAL);
                            break;
                        }else if(status == 2) {
                            strcpy(response, ACCOUNT_BLOCK);
                            flag = 1;
                            answer(clientfd, response, FAILED_SIGNAL);
                            break;
                        }
                    }else {
                        strcpy(response, USERNAME_WRONG);
                        answer(clientfd, response, FAILED_SIGNAL);
                        break;
                    }
                } else {
                    printf("Error here");
                }
                break;
			}
            case REGISTER_SIGNAL: {
                if (tokenCount == 3) {
                    //head = signUp(head, confd, data[0], data[1]);
                } else {
                    // error
                }
                break;
            }
            case ORDER_BUY_SIGNAL: {
                if (tokenCount == 4) {
                    check_action = 1;
                    order(clientfd, order_buff, username, check_action);
                    break;
                } else {
                    printf("Error here");
                }
                break;
            }
            case ORDER_SELL_SIGNAL: {          
                if (tokenCount == 4) {
                    check_action = 2;
                    order(clientfd, order_buff, username, check_action);
                    break;
                } else {
                    printf("Error here");
                }
                break;
            }
            case ACCOUNT_INFO_SIGNAL:
                if (tokenCount == 2) {
                    manage_profile_account(clientfd, username);
                } else {
                    printf("Error here");
                }
                break;
            case TRANSACTION_SIGNAL: 
                current_user = get_current_user(clientfd);
                current_user->is_trading = TRUE;
                strcpy(response,"");
                strcpy(response, online_users(current_user));
                answer(clientfd, response, TRANSACTION_SIGNAL);
                strcpy(response,"");
                char other_rep[1024];
                l_user *tmp = head_user;
                while (tmp != NULL) {
                    if (tmp->is_trading == TRUE && tmp->id != current_user->id) {
                        strcat(other_rep, online_users(tmp));
                        answer(tmp->clientfd, other_rep, TRANSACTION_SIGNAL);
                    }
                    tmp = tmp->next;
                }
                break;
            case CHOICE_USER_OPTION_SIGNAL:
                if (tokenCount == 2){
                    trader = trade_user(data[0]);
                    current_user = get_current_user(clientfd);
                    list_trade[0] = current_user->clientfd;
                    list_trade[1] = trader->clientfd;
                }
                break;
            case REQUEST_BUY_SUCCESS_SIGNAL:
                if (tokenCount == 5){
                    trader = trade_user(data[0]);
                    current_user = get_current_user(clientfd);
                    direct_buy_stock(clientfd, order_buff,  current_user, trader);
                }
                break;
            case REQUEST_SELL_SUCCESS_SIGNAL:
                if (tokenCount == 5){
                    trader = trade_user(data[0]);
                    current_user = get_current_user(clientfd);
                    direct_sell_stock(clientfd, order_buff,  current_user, trader);
                }
                break;
            case YES_SIGNAL:
                accept_trade();
                break;
            case NO_SIGNAL:
                refuse_trade();
                break;
            case LOGOUT_SIGNAL:
            case MENU_SIGNAL:
            case SUCCESS_SIGNAL:
            case FAILED_SIGNAL:
            case REQUEST_BUY_NOTIFY_SIGNAL:
            case REQUEST_SELL_NOTIFY_SIGNAL:
            break;
            }
        switch(flag) {
            case 1:
                break;
            case 2:
                break;  
            case 3: 
                if(strcmp(buff, "direct") == 0) {
                    flag_main = 3;
                    // current_user = get_current_user(clientfd);
                    // current_user->is_trading = TRUE;
                    l_user *current_one = get_current_user(clientfd);
                    current_one->is_trading = TRUE;
                    DIRECT: 
                    if (strcmp(buff, "y") == 0 || strcmp(buff, "yes") == 0) {
                        strcpy(response, "Transaction succesfully!\n");
                        strcat(response, "Press q to quit");
                        if (head_order->type == 0) {
                            direct_buy(head_order, list_trade[0]);
                            direct_sell(head_order, list_trade[1]);
                        } else {
                            direct_buy(head_order, list_trade[1]);
                            direct_sell(head_order, list_trade[0]);
                        }
                        send(list_trade[0], response, strlen(response), 0);
                        send(list_trade[1], response, strlen(response), 0);
                        continue;
                    } else if (strcmp(buff, "n") == 0 || strcmp(buff, "no") == 0) {
                        strcpy(response, "Transaction not succesfully!\n");
                        strcat(response, "Press q to quit");
                        send(list_trade[0], response, strlen(response), 0);
                        send(list_trade[1], response, strlen(response), 0);
                        continue;
                    } if (strcmp(buff, "q") == 0) {
                        flag_main = 0;
                        check = 0;
                        get_current_user(list_trade[0])->is_trading = FALSE;
                        get_current_user(list_trade[1])->is_trading = FALSE;
                        send(clientfd, "end", strlen("end"), 0);
                        continue;
                    } 
                    if(check == 0) {
                        char str[MAX_CHAR] = "Please choose user you want to transaction: \n";
                        strcpy(response, str);
                        strcat(response, online_users(current_one));
                        char other_rep[MAX_CHAR];
                        l_user *tmp = head_user;
                        while (tmp != NULL) {
                            if (tmp->is_trading == TRUE && tmp->id != current_one->id) {
                                strcpy(other_rep, str);
                                strcat(other_rep, online_users(tmp));
                                send(tmp->clientfd, other_rep, strlen(other_rep), 0);
                            }
                            tmp = tmp->next;
                        }
                        check++;
                    }else if (check == 1) {
                        strcpy(trader_id, buff);
                        trader = trade_user(trader_id);
                        list_trade[0] = current_one->clientfd;
                        list_trade[1] = trader->clientfd;
                        if(trader) {
                            strcpy(response, "Please choose action(1.Buy, 2.Sell): ");
                            check++;
                        } else {
                            strcpy(response, "User not online, please try again");
                            check--;
                        }
                        send(clientfd, response, strlen(response), 0);
                    }else if (check == 2) {
                        if (strcmp(buff, "1") == 0) {
                            strcpy(response, get_list_stock_of_user(trader->username));
                            // send to client list stock to sell
                            send(clientfd, response, strlen(response), 0);
                            check_direct_action = 1;
                            free(get_list_stock_of_user(username));
                            check++;
                        }
                        else if (strcmp(buff, "2") == 0)  {
                            strcpy(response, get_list_stock_of_user(current_one->username));
                            // send to client list stock to sell
                            send(clientfd, response, strlen(response), 0);
                            check_direct_action = 2;
                            free(get_list_stock_of_user(username));
                            check++;
                        }
                        else if (strcmp(buff, "q") == 0) {
                            check_direct_action = 3;
                            flag_main = 0;
                            check = 0;
                        }
                    } else if (check == 3) {
                        direct(clientfd, buff, current_one, trader, check_direct_action);
                        if (strcmp(buff, "q") == 0) {
                            check_direct_action = 3;
                            flag_main = 0;
                            check = 0;
                        }
                    }
                    if(strcmp(buff, "order") == 0) {
                    flag_main = 2;
                    ORDER: if (check_order == 1) {
                        if (strcmp(buff, "B") == 0) {
                            check_action = 1;
                        }
                        else if (strcmp(buff, "S") == 0)  {
                            strcpy(response, get_list_stock_of_user(username));
                            // send to client list stock to sell
                            send(clientfd, response, strlen(response), 0);
                            check_action = 2;
                            free(get_list_stock_of_user(username));
                        }
                        else if (strcmp(buff, "q") == 0) {
                            check_action = 3;
                            flag_main = 0;
                            check_order = 0;
                        }
                        order(clientfd, buff, username, check_action);
                    }
                }
                }
                if(strcmp(buff, "manage") == 0) {
                    flag_main = 4;
                    MANAGE: 
                    if (strcmp(buff, "q") == 0) {
                        flag_main = 0;
                        send(clientfd, "end", strlen("end"), 0);
                    } else {
                        manage_profile_account(clientfd, username);
                    }
                }
                if(strcmp(buff, "logout") == 0) {
                    log_out(username);
                    strcpy(response, "Goobye ");
                    strcat(response, username);
                    strcpy(username, "");
                    strcpy(password, "");
                    flag = 1;
                    send(clientfd, response, strlen(response), 0);
                }
                break;   
        }         
        memset(response, 0, strlen(response));
    }
}

int create_server(int argc, char **argv) {
    int sockfd, connfd;
    struct sockaddr_in servaddr, cliaddr;
    if (argc != 2) {
        printf("Input: %s <port>\n", argv[0]);
        return 0;
    }
    read_file(FILENAME);
    read_file_oversold();
    read_file_overbought();

    int PORT = atoi(argv[1]);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket creation failed");
        exit(0);
    };
    memset(&servaddr, 0, sizeof(servaddr));
    
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr(LOCALHOST);
    servaddr.sin_port = htons(PORT);
    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) != 0){
        perror("bind failed");
        exit(0);
    }
    if ((listen(sockfd, MAX_CLIENTS)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    socklen_t cli_addr_size = sizeof(cliaddr);
    printf("Server started on %s:%d\n", LOCALHOST, PORT);
    while(1) {
        if ((connfd = accept(sockfd, (struct sockaddr*)&cliaddr, &cli_addr_size)) < 0 ) {
            error("server accept failed...\n");
        }
        printf("Connection accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
        pthread_t tid;
        pthread_create(&tid, NULL, &client_handler,(void* )(intptr_t)connfd);
    }

    close(sockfd);
    return 0;
}

int main(int argc, char **argv) {
    create_server(argc, argv);
}
