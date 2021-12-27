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

extern l_user *head_user;
extern l_stock *head_stock;
extern l_oversold *head_oversold;

int buy_flag = 0;
int flag_delete = FALSE;
int temp_balance = 0;
 
void manage_profile_account(int clientfd, char username[MAX_CHAR]) {
    l_user *user =  get_account(username);
    char target[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    l_stock *stock = user->stock;
    strcpy(response, "ID NAME BLANCE STATUS\n");
    sprintf(target, "%d %s %d %d\n", user->id, user->username, user->balance, user->status);
    strcat(response, target);
    strcat(response, "List stock\n");
    while(stock != NULL) {
        sprintf(target, "%s %d\n", stock->name, stock->price);
        strcat(response, target);
        stock = stock->next;
    }
    send(clientfd, response, strlen(response), 0);
}

void buy_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]) {
    char name_stock[MAX_CHAR], response[BUFFER_SIZE];
    int price, amount;
    switch(buy_flag) {
        case 0:
            strcpy(response, "Input name stock");
            buy_flag++;
            break;
        case 1:
            strcpy(name_stock, request);
            strcpy(response, "Input price");
            buy_flag++;
            break;
        case 2:
            price = atoi(request);
            strcpy(response, "Input amount");
            buy_flag++;
            break;
        case 3:
            amount = atoi(request);
            l_user *info = get_account(username); //current_user
            if(info != NULL && info->balance < amount * price) {
                strcpy(response, "Balance not enough");
                strcpy(response, "Input price again");
                buy_flag = 1;
            }else {
                // case stock not exist in overbought.txt and price < price of stock => save to file overbought.txt
                if(has_stock_in_oversold(name_stock) == FALSE) {
                    // l_stock *tmp = create_stock(name_stock, amount, price);
                    // add_stock(&head_stock, tmp);
                    write_node_to_overbought(username, name_stock, price, amount);
                    strcpy(response, "Order Match Success!!!\n");
                    strcat(response, "Input name stock");
                    buy_flag = 1;
                }
                // case stock exist in overbought.txt
                else {
                    l_oversold *tmp = head_oversold;
                    while(tmp != NULL) {
                        if (strcmp(tmp->name_stock, name_stock) == 0 && price >= tmp->price && strcmp(tmp->username, info->username) != 0) {
                            l_user* seller = get_account(tmp->username);
                            l_stock *stock_sell =  search_stock_of_user(&seller, name_stock, tmp->price);
                            l_stock *search_stock = search_stock_of_user(&info, name_stock, tmp->price);
                            if(tmp->amount <= amount) {
                                tmp->key = TRUE;
                                temp_balance += tmp->price * tmp->amount;
                                seller->balance += tmp->price * tmp->amount;
                                stock_sell->amount -= tmp->amount;
                                flag_delete = TRUE;
                                if(search_stock == NULL) {
                                    add_stock(&(info->stock), create_stock(name_stock, tmp->amount, tmp->price));
                                }else {
                                    search_stock->amount += tmp->amount;
                                }
                            }else if(tmp->amount > amount) {
                                tmp->amount = abs(tmp->amount - amount);
                                temp_balance += tmp->price * amount;
                                seller->balance += tmp->price * amount;
                                // update amount stock of seller
                                stock_sell->amount -= amount;
                                // L15 100 5 => L15 100 4 -> add L15 100 1 to LL
                                l_stock* temp = create_stock(tmp->name_stock, amount, tmp->price);
                                add_stock(&head_stock, temp);

                                if(search_stock == NULL) {
                                    add_stock(&(info->stock), temp);
                                }else {
                                    search_stock->amount += amount;
                                }
                            }
                            
                            if(stock_sell->amount == 0) {
                                delete_node_stock(seller->stock,stock_sell);
                            }
                            amount -= tmp->amount;
                        }
                        tmp = tmp->next;
                    }
                    if(amount > 0) {
                        write_node_to_overbought(username, name_stock, price, amount);
                    }
                    if(flag_delete == TRUE) {
                        delete_all_by_key(TRUE);
                        append_file_order_match();
                    }
                    write_file_oversold();
                    info->balance -= temp_balance;
                    // add new stock to buyer 
                    write_file("file/users.txt");
                    strcpy(response, "Order Match Success!!!\n");
                    strcat(response, "Input name stock");
                    buy_flag = 1;
                }
            }
            break;
    }
    send(clientfd, response, strlen(response), 0);
}

void order(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR], int check_action) {
    switch(check_action) {
        case 1:
            buy_stock(clientfd, request, username);
            break;
        case 2: 
            break;
        case 3:
            send(clientfd, "Bye", strlen("Bye"), 0);
            buy_flag = 0;
            break;
    }
}

void *client_handler(void *arg){
    int clientfd, n, flag = 1, check = 0, buy = 0, sell = 0, flag_main = 0,
        check_order = 0, check_action ;
    char buff[BUFFER_SIZE], response[BUFFER_SIZE], username[MAX_CHAR], password[MAX_CHAR];
    char stock_name[MAX_CHAR], price[MAX_CHAR], trader_id[MAX_CHAR], amount[MAX_CHAR], type[MAX_CHAR];
    l_user *trader;
    pthread_detach(pthread_self());
    clientfd = (intptr_t) arg;
    while(1) {
        n = recv(clientfd, buff, BUFFER_SIZE, 0);
        buff[n] = '\0';
        if (buff[strlen(buff) - 1] == '\n')
            buff[strlen(buff) - 1] = 0;
        printf("String received from client: %s\n", buff);
        if(strlen(buff) == 0) {
            printf("bye %d\n", clientfd);
            close(clientfd);
            return 0;
        }
        if(flag_main == 3) {
            goto DIRECT;
        }else if(flag_main == 4) {
            goto MANAGE;
        }else if(flag_main == 2) {
            check_order = 1;
            goto ORDER;
        }

        switch(flag) {
            case 1:
                strcpy(username, buff);
                if(has_account(username) == TRUE) {
                    flag++;
                    strcpy(response, "Password");
                }else {
                    strcpy(response, USERNAME_WRONG);
                }
                send(clientfd, response, strlen(response), 0);
                break;
            case 2:
                strcpy(password, buff);
                int status = sign_in(username, password, clientfd);
                if(status == 1) {
                    strcpy(response, LOGIN_SUCCESS);
                    // current_user = get_account(username);
                    flag++;
                }else if(status == 3) {
                    strcpy(response, PASSWORD_WRONG);
                    flag = 1;
                }else if(status == 2) {
                    strcpy(response, ACCOUNT_BLOCK);
                    flag = 1;
                }
                send(clientfd, response, strlen(response), 0);
                break;  
            case 3: 
                if(strcmp(buff, "direct") == 0) {
                    flag_main = 3;
                    DIRECT: if(check == 0) {
                        char str[MAX_CHAR] = "Please choose user you want to transaction: ";
                        strcat(str, online_users(username));
                        strcpy(response, str);
                        check++;
                    }else if (check == 1) {
                        strcpy(trader_id, buff);
                        trader = trade_user(trader_id);
                        if(trader) {
                            strcpy(response, "Please choose action(1.Buy, 2.Sell): ");
                            check++;
                        } else {
                            strcpy(response, "User not online, please try again");
                            check--;
                        }
                    }else if (check == 2) {
                        if (strcmp(type,"1") == 0) {
                            if (buy == 1) {
                                strcpy(stock_name, buff);
                                if(direct_trade(current_user, trader, stock_name, 0, 1) == TRUE) {
                                    strcpy(response, "Input price: ");
                                    buy++;
                                } else {
                                    strcpy(response, "Stock not found\n");
                                    char str[MAX_CHAR] = "Please choose stock to buy:";
                                    strcat(str, user_stock_list(trader_id));
                                    strcat(str, "\nName: ");
                                    strcat(response, str);
                                }
                            } else if (buy == 2) {
                                strcpy(price, buff);
                                snprintf(amount, MAX_CHAR,"%d", direct_trade(current_user, trader, stock_name, atoi(price), 1));
                                strcpy(response, "Valid amount (");
                                strcat(response, amount);
                                strcat(response, ")");
                                if (atoi(amount) == 0) {
                                    strcat(response, "Your balance is not enough!\n");
                                    strcat(response, "Input price: ");
                                } else {
                                    strcat(response, "\nInput amount: ");
                                    buy++;
                                }
                            } else if (buy == 3) {
                                if (atoi(buff) > atoi(amount)) {
                                    strcpy(response, "Invalid amount, input again:");
                                } else {
                                    strcpy(response, "Please wait...");
                                }
                            }
                        } else if (strcmp(type,"2") == 0) {
                            if (sell == 1) {
                                strcpy(stock_name, buff);
                                if(direct_trade(current_user, trader, stock_name, 0, 2) == TRUE) {
                                    strcpy(response, "Price: ");
                                    sell++;
                                } else {
                                    strcpy(response, "Stock not found\n");
                                    char str[MAX_CHAR] = "Please choose stock to sell:";
                                    snprintf(trader_id, MAX_CHAR,"%d", current_user->id);
                                    strcat(str, user_stock_list(trader_id));
                                    strcat(str, "\nName: ");
                                    strcat(response, str);
                                }
                            } else if (sell == 2) {
                                strcpy(price, buff);
                                snprintf(amount, MAX_CHAR,"%d", direct_trade(current_user, trader, stock_name, atoi(price), 2));
                                strcpy(response, "Valid amount (");
                                strcat(response, amount);
                                strcat(response, ")\nInput amount: ");
                                if (atoi(amount) == 0) {
                                    char str[MAX_CHAR] = "Please choose stock to sell:";
                                    snprintf(trader_id, MAX_CHAR,"%d", current_user->id);
                                    strcat(str, user_stock_list(trader_id));
                                    strcat(str, "\nName: ");
                                    sell--;
                                }
                                sell++;
                            } else if (sell == 3) {
                                if (atoi(buff) > atoi(amount)) {
                                    strcpy(response, "Invalid amount, input again:");
                                } else {
                                    strcpy(response, "Please wait...");
                                    // flag_main = 4;
                                }
                            }
                        } else {
                            strcpy(type, buff);
                            if (strcmp(type,"1") == 0) {
                                char str[MAX_CHAR] = "Please choose stock to buy:";
                                strcat(str, user_stock_list(trader_id));
                                strcat(str, "\nName: ");
                                strcpy(response, str);
                                buy++;
                            } else {
                                char str[MAX_CHAR] = "Please choose stock to sell:";
                                snprintf(trader_id, MAX_CHAR,"%d", current_user->id);
                                strcat(str, user_stock_list(trader_id));
                                strcat(str, "\nName: ");
                                strcpy(response, str);
                                sell++;
                            }
                        }
                    }
                    send(clientfd, response, strlen(response), 0);
                }
                if(strcmp(buff, "order") == 0) {
                    flag_main = 2;
                    ORDER: if (check_order == 1) {
                        if (strcmp(buff, "B") == 0) {
                            check_action = 1;
                        }
                        else if (strcmp(buff, "S") == 0) check_action = 2;
                        else if (strcmp(buff, "q") == 0) {
                            check_action = 3;
                            flag_main = 0;
                            check_order = 0;
                        }
                        order(clientfd, buff, username, check_action);
                    }
                }
                if(strcmp(buff, "manage") == 0) {
                    MANAGE: manage_profile_account(clientfd, username);
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

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
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
