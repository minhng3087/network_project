#include "client.h"
#include "menu.h"
#include "utils/drawUtils.h"
#include "utils/define.h"

int sockfd = 0;
char sendline[BUFFER] = {};
char recvline[BUFFER];
pthread_t recv_msg_thread;
int check_out = 0;

void send_request(char sendline[BUFFER], char recvline[BUFFER]) {
    fgets(sendline, BUFFER, stdin);
    send(sockfd, sendline, strlen(sendline), 0);
    int n = recv(sockfd, recvline, BUFFER, 0);
    recvline[n] = '\0';
    if (recvline[strlen(recvline) - 1] == '\n')
        recvline[strlen(recvline) - 1] = 0;
}
void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}

void add_token(char str[BUFFER], char header[BUFFER]) {
    strcat(str, "|");
    strcat(str, header);
}
int flag = 0;
void recv_msg_handler() {
    char message[BUFFER];
    while (1) {
        if (check_out == 1) {
            break;
        }
        int receive = recv(sockfd, message, BUFFER, 0);
        message[receive] = '\0';
        if (message[strlen(message) - 1] == '\n')
        message[strlen(message) - 1] = 0;
        if (receive > 0) {
            if (strcmp(message,"end") == 0) {
                memset(message, 0, sizeof(message));
                continue;
            }
            printf("%s\n", message);
            
            str_overwrite_stdout();
        } else if (receive == 0) {
            printf("connection lost!");
            break;
        } else {
            // -1
        }
        memset(message, 0, sizeof(message));
    }
}

int direct_transaction(){
    printf("_________________Giao dịch trực tiếp__________________\n");
    
    while(1){ 
        __fpurge(stdin);
        fgets(sendline, BUFFER, stdin);
        send(sockfd, sendline, strlen(sendline), 0);
        if (strcmp(sendline, "q\n") == 0) {
            break;
        }
    }
    return 0;
}

int board() {
    char sendline[BUFFER];
    printf("_________________Bảng điện__________________\n");
    // int n = recv(sockfd, recvline, BUFFER, 0);
    // recvline[n] = '\0';
    // if (recvline[strlen(recvline) - 1] == '\n')
    //     recvline[strlen(recvline) - 1] = 0;
    // printf("%s\n", recvline);
    // while(1){ 
    //     __fpurge(stdin);
    //     send_request(sendline, recvline);
    //     printf("%s\n", recvline);
    // }
    while(1){ 
        __fpurge(stdin);
        fgets(sendline, BUFFER, stdin);
        send(sockfd, sendline, strlen(sendline), 0);
    }
    return 0;
}

int order(){
    while(1){ 
        __fpurge(stdin);
        fgets(sendline, BUFFER, stdin);
        send(sockfd, sendline, strlen(sendline), 0);
        if (strcmp(sendline, "q\n") == 0) {
            break;
        }
    }
    return 1;
}

int manage_profile_account(){
    printf("_________________Quan ly tai khoan__________________\n");
    while(1){ 
        __fpurge(stdin);
        fgets(sendline, BUFFER, stdin);
        send(sockfd, sendline, strlen(sendline), 0);
        if (strcmp(sendline, "q\n") == 0) {
            break;
        }
    }
    return 1;
}

int program_main() {
    char choice_main[2];
    int n, choice_order;
    while (1) {
        // menu_main();
        displayMainMenuWindow(sockfd);
        if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
            printf("ERROR: pthread\n");
            return EXIT_FAILURE;
        }
       
        __fpurge(stdin);
        fgets(choice_main, 2, stdin);
        int check = choice_main[0] - '0';

        switch(check) {
            case 1:
                send(sockfd, "board", strlen("board"), 0);
                board(sockfd);
                return 1;
            case 2:
                send(sockfd, "order", strlen("order"), 0);
                printf("_________________Đặt lệnh__________________\n");
                menu_order();
                scanf("%d", &choice_order);
                switch(choice_order) {
                    case 1:
                        send(sockfd, "B", strlen("B"), 0);
                        break;
                    case 2:
                        send(sockfd, "S", strlen("S"), 0);
                        break;
                }
                order();
                return 1;
            case 3:
                send(sockfd, "direct", strlen("direct"), 0);
                // add_token(sendline, "direct");
                direct_transaction();
                return 1;
            case 4:
                send(sockfd, "manage", strlen("manage"), 0);
                manage_profile_account();
                return 1;
            case 5: 
                send(sockfd, "logout", strlen("logout"), 0);
                n = recv(sockfd, recvline, BUFFER, 0);
                recvline[n] = '\0';
                if (recvline[strlen(recvline) - 1] == '\n')
                    recvline[strlen(recvline) - 1] = 0;
                printf("%s\n", recvline);
                check_out = 1;
                return 0;
        }
    }
    
}

void login() {
    char sendline[BUFFER], recvline[BUFFER];
    int choice;
    state = 0;
    
        while (1){
            if (state == MENU){
                displayMenuWindow();
            }
            if (state == LOGIN){
                displayLoginWindow(sockfd);
            }
            if (state == SIGN_UP){
                displaySignUpWindow(sockfd);
            }
            if (state == MAIN_MENU){
                if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
                    printf("ERROR: pthread\n");
                }
                break;
            }
            if (state == QUIT){
                quit();
                break;
        }
    }
}


int main(int argc, char **argv) {
    char ip[MAX_CHAR];
    struct sockaddr_in serv_addr;
    
    if (argc < 3) {
        printf("Input: %s <ip> <port>\n", argv[0]);
        return 0;
    }
    
    strcpy(ip, argv[1]);
    int port = atoi(argv[2]);
    
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    
    login();
    close(sockfd);
}
