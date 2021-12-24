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

void *client_handler(void *arg){
    int clientfd, n, flag = 1, check = 0, buy = 0, sell = 0;
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
        if (strcmp(buff,"3") == 0) {
            flag = 4;
        }
        if (strcmp(buff,"5") == 0) {
            flag = 5;
        }else if (strcmp(buff,"4") == 0) {
            flag = 4;
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
                int status = sign_in(username, password);
                if(status == 1) {
                    strcpy(response, LOGIN_SUCCESS);
                    flag++;
                }else if(status == 3) {
                    strcpy(response, PASSWORD_WRONG);
                    flag = 1;
                }else if(status == 2) {
                    strcpy(response, ACCOUNT_BLOCK);
                    flag = 1;
                }
                break;  
            case 3: 
                CHOOSE_USER: if(check == 0) {
                    char str[MAX_CHAR] = "Please choose user you want to transaction: ";
                    strcat(str, online_users(current_user));
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
                        goto CHOOSE_USER;
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
                break;   
            case 4:
                manage_profile_account(clientfd, username);
                break;
            case 5:
                log_out(username);
                strcpy(response, "Goobye ");
                strcat(response, username);
                strcpy(username, "");
                strcpy(password, "");
                flag = 1;
                send(clientfd, response, strlen(response), 0);
                break;
        }         
        send(clientfd, response, strlen(response), 0);
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
