#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#include "handle.h"
#include "logic.h"

void addToken(char *str, SignalState signal) {
    int len = strlen(str);
    str[len] = '|';
    str[len+1] = '0' + signal;
    str[len+2] = '\0';
}
