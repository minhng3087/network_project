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

void *client_handler(void *arg){
    int clientfd, n, flag = 1;
    char buff[BUFFER_SIZE], response[BUFFER_SIZE], username[MAX_CHAR], password[MAX_CHAR];
    pthread_detach(pthread_self());
    clientfd = (intptr_t) arg;
    while(1) {
        n = recv(clientfd, buff, BUFFER_SIZE, 0);
        buff[n] = '\0';
        if (buff[strlen(buff) - 1] == '\n')
            buff[strlen(buff) - 1] = 0;
        printf("String received from client: %s\n", buff);
        
        switch(flag) {
            case 1:
                strcpy(username, buff);
                if(has_account(username) == TRUE) {
                    flag++;
                    strcpy(response, "Password");
                }else {
                    strcpy(response, USERNAME_WRONG);
                }
                break;
            case 2:
                strcpy(password, buff);
                int status = sign_in(username, password);
                if(status == 1) {
                    strcpy(response, LOGIN_SUCCESS);
                }else if(status == 3) {
                    strcpy(response, PASSWORD_WRONG);
                    flag = 1;
                }else if(status == 2) {
                    strcpy(response, ACCOUNT_BLOCK);
                    flag = 1;
                }
                break;
        } 
        send(clientfd, response, strlen(response), 0);
        memset(response, 0, strlen(response));
    }
    close(clientfd);
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
