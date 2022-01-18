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
#include <sys/ioctl.h>
#include <fcntl.h>
#include "file.h"
#include "action.h"

extern l_user *head_user;
extern l_stock *head_stock;
extern l_oversold *head_oversold;
extern l_overbought *head_overbought;

int buy_flag = 0;
int flag_delete = FALSE;
int temp_balance = 0;
int temp_amount = 0;
int sell_flag = 0;
int count_same = 0;


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
            int amount2 = amount;
            l_user *info = get_account(username); //current_user
            if(info != NULL && info->balance < amount * price) {
                strcpy(response, "Balance not enough");
                strcpy(response, "Input price again");
                buy_flag = 1;
            }else {
                // case stock not exist in overbought.txt and price < price of stock => save to file overbought.txt
                if(has_stock_in_oversold(name_stock) == FALSE) {
                    write_node_to_overfile("file/overbought.txt", username, name_stock, price, amount);
                    strcpy(response, "Order Match Success (1) !!!\n");
                    strcat(response, "Press q to quit\n");
                }
                // case stock exist in overbought.txt
                else {
                    l_oversold *tmp = head_oversold;
                    while(tmp != NULL) {
                        if (strcmp(tmp->name_stock, name_stock) == 0 && price >= tmp->price && strcmp(tmp->username, info->username) != 0) {
                            l_user* seller = get_account(tmp->username);
                            l_stock *stock_sell =  search_stock_of_user(seller, name_stock, tmp->price);
                            l_stock *search_stock = search_stock_of_user(info, name_stock, tmp->price);
                            if(tmp->amount <= amount) {
                                temp_amount = tmp->amount;
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
                                temp_amount = tmp->amount;
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
                    strcpy(response, "Order Match Success!!!\n");
                    strcat(response, "Press q to quit");
                    // reset
                    count_same = 0;
                    buy_flag = 1;
                    flag_delete = FALSE;
                }
            }
            break;
    }
    send(clientfd, response, strlen(response), 0);
}

// void sell_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]) {
//     char name_stock[MAX_CHAR], response[BUFFER_SIZE];
//     int price, amount;
//      switch(sell_flag) {
//         case 0:
//             strcpy(response, "\nInput name stock");
//             sell_flag++;
//             break;
//         case 1:
//             strcpy(name_stock, request);
//             strcpy(response, "Input price");
//             sell_flag++;
//             break;
//         case 2:
//             price = atoi(request);
//             strcpy(response, "Input amount");
//             sell_flag++;
//             break;
//         case 3: 
//             amount = atoi(request);
//             int amount2 = amount;
//             l_user *info = get_account(username); //current_user
//             if(amount > get_amount_from_stock(&info, name_stock)) {
//                 strcpy(response, "Amount not enough");
//                 strcpy(response, "Input amount again");
//                 sell_flag = 3;
//             }else {
//                 if(has_stock_in_overbought(name_stock) == FALSE) {
//                     write_node_to_overfile("file/oversold.txt", username, name_stock, price, amount);
//                     strcpy(response, "Order Match Success!!!\n");
//                     strcat(response, "Press q to quit");
//                 }else {
//                     l_overbought *tmp = head_overbought;
//                     while(tmp != NULL) {
//                         if (strcmp(tmp->name_stock, name_stock) == 0 && price <= tmp->price && strcmp(tmp->username, info->username) != 0) {
//                             l_user* buyer = get_account(tmp->username);
//                             l_stock *stock_buy =  search_stock_of_user(&buyer, name_stock, tmp->price);
//                             l_stock *search_stock = search_stock_of_user(&info, name_stock, tmp->price);
//                             if(tmp->amount <= amount) {
//                                 temp_amount = tmp->amount;
//                                 tmp->key = TRUE;
//                                 temp_balance += tmp->price * tmp->amount;
//                                 buyer->balance -= tmp->price * tmp->amount;
//                                 stock_buy->amount += tmp->amount;
//                                 flag_delete = TRUE;
//                                 if(search_stock == NULL) {
//                                     add_stock(&(info->stock), create_stock(name_stock, tmp->amount, tmp->price));
//                                 }else {
//                                     search_stock->amount += tmp->amount;
//                                 }
//                             }else if(tmp->amount > amount) {
//                                 temp_amount = tmp->amount;
//                                 tmp->amount = abs(tmp->amount - amount);
//                                 temp_balance += tmp->price * amount;
//                                 buyer->balance -= tmp->price * amount;
//                                 // update amount stock of buyer
//                                 stock_buy->amount += amount;
//                                 // L15 100 5 => L15 100 4 -> add L15 100 1 to LL
                                
//                                 l_stock* temp = create_stock(tmp->name_stock, amount, tmp->price);
//                                 add_stock(&head_stock, temp);

//                                 if(search_stock == NULL) {
//                                     add_stock(&(info->stock), temp);
//                                 }else {
//                                     search_stock->amount += amount;
//                                 }
//                             }
                            
//                             if(stock_buy->amount == 0) {
//                                 delete_node_stock(buyer->stock,stock_buy);
//                             }
//                             amount -= temp_amount;
//                             count_same++;
//                         }
//                         tmp = tmp->next;
//                     }
                    
//                     if(amount > 0) {
//                         write_node_to_overfile("file/oversold.txt", username, name_stock, price, amount);
//                     }
//                     if(count_same == 1) {
//                        append_one_stock_to_order_match(name_stock, price, amount2);
//                     }
//                     if(flag_delete == TRUE) {
//                         delete_all_by_key_overbought(TRUE);
//                         append_file_order_match();
//                     }
//                     write_file_overbought();
//                     info->balance += temp_balance;
//                     // add new stock to buyer 
//                     write_file("file/users.txt");
//                     strcpy(response, "Order Match Success!!!\n");
//                     strcat(response, "Press q to quit");
//                     // reset
//                     count_same = 0;
//                     sell_flag = 1;
//                     flag_delete = FALSE; 
//                 }
//             }

//     }
//     send(clientfd, response, strlen(response), 0);
// }


void order(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR], int check_action) {
    switch(check_action) {
        case 1:
            buy_stock(clientfd, request, username);
            break;
        case 2: 
            // sell_stock(clientfd, request, username);
            break;
        case 3:
            send(clientfd, "Bye", strlen("Bye"), 0);
            buy_flag = 0;
            sell_flag = 0;
            break;
    }
}

int create_server(int argc, char **argv) {
    int sockfd;
    // 
    int rc, on = 1;
    int max_sd, new_sd;
    int desc_ready, end_server = FALSE;
    int close_conn;
    fd_set master_set, working_set;
    struct timeval timeout;
     int flag = 1, check = 0, buy = 0, sell = 0, flag_main = 0,
    check_order = 0, check_action ;
    char buff[BUFFER_SIZE], response[BUFFER_SIZE], username[MAX_CHAR], password[MAX_CHAR];
    char stock_name[MAX_CHAR], price[MAX_CHAR], trader_id[MAX_CHAR], amount[MAX_CHAR], type[MAX_CHAR];
    l_user *trader;
    //
    struct sockaddr_in servaddr;
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

    //
    rc = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char * ) & on, sizeof(on));
    if (rc < 0) {
        perror("setsockopt() failed");
        close(sockfd);
        exit(-1);
    }

    rc = ioctl(sockfd, FIONBIO, (char * ) & on);
    if (rc < 0) {
        perror("ioctl() failed");
        close(sockfd);
        exit(-1);
    }

    //
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

    //
    FD_ZERO(&master_set);
    max_sd = sockfd;
    FD_SET(sockfd, &master_set);
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    //

    printf("Server started on %s:%d\n", LOCALHOST, PORT);

    do {
        memcpy( & working_set, & master_set, sizeof(master_set));
        printf("Waiting on select()...\n");
        rc = select(max_sd + 1, & working_set, NULL, NULL, & timeout);
        if (rc < 0) {
            perror("  select() failed");
            break;
        }
        if (rc == 0) {
            printf("  select() timed out.  End program.\n");
            break;
        }
        desc_ready = rc;
        for (int i = 0; i <= max_sd && desc_ready > 0; ++i) {
            if (FD_ISSET(i, & working_set)) {
                desc_ready -= 1;
            if (i == sockfd) {
                printf("  Listening socket is readable\n");
            do {
                new_sd = accept(sockfd, NULL, NULL);
                if (new_sd < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("  accept() failed");
                        end_server = TRUE;
                    }
                    break;
                }
                printf("  New incoming connection - %d\n", new_sd);
                FD_SET(new_sd, & master_set);
                if (new_sd > max_sd)
                    max_sd = new_sd;
            } while (new_sd != -1);
            } else {
            printf("  Descriptor %d is readable\n", i);
            close_conn = FALSE;
            do {
                // 
                rc = recv(i, buff, BUFFER_SIZE, 0);
                buff[rc] = '\0';
                if (buff[strlen(buff) - 1] == '\n')
                    buff[strlen(buff) - 1] = 0;
                printf("String received from client: %s\n", buff);
                if (rc < 0) {
                    if (errno != EWOULDBLOCK) {
                        perror("  recv() failed");
                        close_conn = TRUE;
                    }
                    break;
                }

                if (rc == 0) {
                    printf("  Connection closed\n");
                    close_conn = TRUE;
                    break;
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
                        rc = send(i, response, strlen(response), 0);
                        break;
                    case 2:
                        strcpy(password, buff);
                        int status = sign_in(username, password, i);
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
                        rc = send(i, response, strlen(response), 0);
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
                            rc = send(i, response, strlen(response), 0);
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
                                    rc = send(i, response, strlen(response), 0);
                                    check_action = 2;
                                }
                                else if (strcmp(buff, "q") == 0) {
                                    check_action = 3;
                                    flag_main = 0;
                                    check_order = 0;
                                }
                                order(i, buff, username, check_action);
                            }
                        }
                        if(strcmp(buff, "manage") == 0) {
                            MANAGE: manage_profile_account(i, username);
                        }
                        if(strcmp(buff, "logout") == 0) {
                            log_out(username);
                            strcpy(response, "Goobye ");
                            strcat(response, username);
                            strcpy(username, "");
                            strcpy(password, "");
                            flag = 1;
                            send(i, response, strlen(response), 0);
                        }
                        break;   
                }         
                memset(response, 0, strlen(response));
                //
                if (rc < 0) {
                   perror("  send() failed");
                   close_conn = TRUE;
                   break;
                }

            } while (TRUE);
            if (close_conn) {
                close(i);
                FD_CLR(i, & master_set);
                if (i == max_sd) {
                while (FD_ISSET(max_sd, & master_set) == FALSE)
                    max_sd -= 1;
                }
            }
            } /* End of existing connection is readable */
        } /* End of if (FD_ISSET(i, &working_set)) */
        } /* End of loop through selectable descriptors */

    } while (end_server == FALSE);
    for (int i = 0; i <= max_sd; ++i) {
        if (FD_ISSET(i, & master_set))
      close(i);
    }

    return 0;
}

int main(int argc, char **argv) {
    create_server(argc, argv);
}
