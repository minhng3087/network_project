
#include "client.h"
#include "menu.h"

void send_request(int sockfd, char sendline[BUFFER], char recvline[BUFFER]) {
    fgets(sendline, BUFFER, stdin);
    send(sockfd, sendline, strlen(sendline), 0);
    int n = recv(sockfd, recvline, BUFFER, 0);
    recvline[n] = '\0';
    if (recvline[strlen(recvline) - 1] == '\n')
        recvline[strlen(recvline) - 1] = 0;
    printf("%ld\n", strlen(recvline));
}

int login(int sockfd) {
    char sendline[BUFFER], recvline[BUFFER];
    int sign_in = 0;
    int choice;
    while(1) {
        if (sign_in == 0) {
            menu_login();
        }else if (sign_in == 1) {
            menu_main();
        }
        __fpurge(stdin);
        scanf("%d", &choice);
        switch (choice) {
            case 1: 
                printf(" ________________Đăng nhập__________________\n");
                printf("Username: ");
                __fpurge(stdin);
                send_request(sockfd, sendline, recvline);
                printf("%s\n", recvline);
                if (strcmp(recvline, USERNAME_WRONG) != 0) {
                    __fpurge(stdin);
                    send_request(sockfd, sendline, recvline);
                    printf("%s\n", recvline);
                    sign_in = strcmp(recvline, LOGIN_SUCCESS) == 0 ? 1 : 0;
                }
              
                break;
            case 2:
                printf(" ________________Đăng ký__________________\n");
                break;
        }
    }
    
}

int main(int argc, char **argv) {
    char ip[MAX_CHAR];
    int sockfd;
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
    
    login(sockfd);
    close(sockfd);
}
