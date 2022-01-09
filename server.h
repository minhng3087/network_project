#include "define.h"

int create_server(int argc, char **argv);
void *client_handler(void *arg);
void manage_profile_account(int clientfd, char username[MAX_CHAR]);
void buy_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]);
void sell_stock(int clientfd, char request[BUFFER_SIZE], char username[MAX_CHAR]);