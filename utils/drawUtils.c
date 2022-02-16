#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>

#include "define.h"
#include "drawUtils.h"
#include "handle.h"
#include "../action.h"
#include "../file.h"
#include <pthread.h>
#define FILENAME "file/users.txt"
#define BUFFER 200
char username[20], password[20];
extern l_user *head_user;
extern l_stock *head_stock;
char respone_msg[1024];
pthread_t recv_msg_thread;
char message[1024];
int check_out = 0;
int connfd = 0;

void recv_msg_handler() {
    while (1) {
        if (check_out == 1) {
            break;
        }
        int receive = recv(connfd, message, BUFFER, 0);
        message[receive] = '\0';
        if (message[strlen(message) - 1] == '\n')
        message[strlen(message) - 1] = 0;
        if (receive > 0) {
            if (strcmp(message,"end") == 0) {
                memset(message, 0, sizeof(message));
                continue;
            }
            if(strlen(message)>0){
                strcpy(respone_msg,"");
                //memset(respone_msg, 0, sizeof(respone_msg));
                strcpy(respone_msg,message);
                // gotoxy(20,20);
                // printf("responeEEE:%s",respone_msg);
            }
        } else if (receive == 0) {
            printf("connection lost!");
            break;
        } else {
            // -1
        }
        // gotoxy(25,25);
        // printf("respone:%s",respone_msg);
        // gotoxy(25,25);
        // printf("respone:%s",respone_msg);
       // printf("%s",respone_msg);
        memset(message, 0, sizeof(message));
    }
}


int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

void delay(int seconds)
{
    clock_t start_time = clock();
    while (clock() < start_time + seconds * 100)
        ;
}

void gotoxy(int x, int y)
{
    printf("%c[%d;%df", 0x1B, y, x);
}

void clearRect(int start_x, int start_y, int width, int height)
{
    for (int j = start_y; j < start_y + height; j++)
    {
        gotoxy(start_x, j);
        for (int i = start_x; i <= start_x + width; i++)
        {
            printf(" ");
        }
    }
}

void clearRectReverse(int start_x, int start_y, int width, int height)
{
    for (int i = start_x - width - 1; i <= start_x; i++)
    {
        for (int j = start_y - 1; j < start_y + height; j++)
        {
            gotoxy(i, j);
            printf(" ");
        }
    }
}

void clearScreen()
{
    clearRect(0, 0, WIDTH, HEIGHT);
}

void drawBorder()
{
    clearScreen();
    printf(KYEL);
    gotoxy(1, 2);
    for (int i = 1; i < WIDTH; i++)
    {
        putchar('=');
    }
    gotoxy(1, HEIGHT);
    for (int i = 1; i < WIDTH; i++)
    {
        putchar('=');
    }
    for (int i = 3; i < HEIGHT; i++)
    {
        gotoxy(0, i);
        putchar('x');
    }
    for (int i = 3; i < HEIGHT; i++)
    {
        gotoxy(WIDTH - 1, i);
        putchar('x');
    }
}

void drawSubBorder(int x, int y, int width, int height, char *title)
{
    gotoxy(x, y);
    for (int i = 0; i < width; i++)
    {
        putchar('_');
    }
    gotoxy(x, y + height - 1);
    for (int i = 0; i < width; i++)
    {
        putchar('_');
    }

    for (int i = 1; i < height; i++)
    {
        gotoxy(x, i + y);
        putchar(220);
    }
    for (int i = 1; i < height; i++)
    {
        gotoxy(x + width, i + y);
        putchar(220);
    }

    gotoxy(x + width / 2 - strlen(title) / 2 - 1, y);
    printf(" %s ", title);
}

void printText(char *str, int top, int left, int right)
{
    int c = 0, line = 0;
    while (str[c] != '\0')
    {
        gotoxy(left, top + line++);
        for (int i = left; i < right; i++)
        {
            if (str[c] != '\0')
            {
                printf("%c", str[c++]);
            }
            else
            {
                break;
            }
        }
    }
}

void returnMenu()
{
    gotoxy(20, 1);
    printf("Press Enter to return to the menu");
    // while(getchar() != '\n');
    // nhan phin bat ky de chung trinh chay tiep
    getchar();
    state = MENU;
}

void quit()
{
    drawBorder();
    gotoxy(MARGIN_LEFT + 3 * WIDTH / 10 + 6, TOP + HEIGHT / 3);
    printf("Bye! See you again");

    // dua con tro den duoi cua so de sao khi ket thuc phien lam viec
    // xuat hien dong lenh cua linux se ko lam hong cua so
    gotoxy(0, HEIGHT + 1);
}

void testDraw()
{
    system("clear");
    drawBorder();

    int X_POSITION = WIDTH / 2 - 16,
        Y_POSITION = TOP + 5 * HEIGHT / 12;

    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION);
    putchar('[');
    gotoxy(X_POSITION + 31, Y_POSITION);
    putchar(']');

    int currentChar = 'a';

    int checkPos = 1, checkVector = 0;

    while (1)
    {
        gotoxy(X_POSITION, Y_POSITION - 2);
        printf(KYEL);
        printf("Press up/down to start, press Enter to quit");
        gotoxy(0, 0);
        printf(KWHT);
        printf("        ");
        gotoxy(0, 0);

        sleep(0.3);
        // phat hien nhan phim
        if (kbhit())
        {
            // lay ma ascii cu phim vua nhan
            char key = getch();

            if (key == 65 || key == 66)
            {
                gotoxy(X_POSITION, Y_POSITION - 2);
                printf(KYEL);
                printf("Press Enter to stop                        ");
                gotoxy(0, 0);
                printf(KWHT);
                printf("        ");
                gotoxy(0, 0);

                while (1)
                {
                    delay(100);
                    if (checkVector == 0) // dang tang
                    {
                        gotoxy(X_POSITION + checkPos, Y_POSITION);
                        printf(KRED);
                        putchar('/');
                        if (checkPos == 30)
                        {
                            checkVector = -1;
                        }
                        else
                        {
                            checkPos++;
                        }
                    }
                    else // dang giam
                    {
                        gotoxy(X_POSITION + checkPos, Y_POSITION);
                        putchar(' ');
                        if (checkPos == 1)
                        {
                            checkVector = 0;
                        }
                        else
                        {
                            checkPos--;
                        }
                    }

                    gotoxy(X_POSITION, Y_POSITION + 2);
                    printf(KYEL);
                    printf("Current char: ");
                    putchar(currentChar);
                    if (currentChar == 'z') // chay tu a->z
                    {
                        currentChar = 'a';
                    }
                    else
                    {
                        currentChar++;
                    }
                    gotoxy(0, 0);
                    printf(KWHT);
                    printf("        ");
                    gotoxy(0, 0);

                    if (kbhit())
                    {
                        char key2 = getch();
                        if (key2 == 10)
                        {
                            break;
                        }
                    }
                }
            }
            // Enter
            else if (key == 10)
            {
                state = MENU;
                break;
            }
        }
    }
}

char *readFile(char *filename)
{
    FILE *fp;
    fp = fopen(filename, "r+");

    if (fp == NULL)
    {
        printf("\nCan't open file!\n");
        exit(1);
    }

    char *str = malloc(10000 * sizeof(char));
    int index = 0;
    while (1)
    {
        char c = getc(fp);
        if (c == EOF)
        {
            str[index] = '\0';
            break;
        }
        str[index++] = c;
    }

    str = realloc(str, index);
    return str;
}

void drawCharacter(char *str, int x_position, int y_position)
{
    int lineNum = 0, colNum = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\n')
        {
            gotoxy(x_position, y_position + ++lineNum);
            colNum = 0;
        }
        else
        {
            if (str[i] == ' ')
            {
                gotoxy(x_position + colNum + 1, y_position + lineNum);
            }
            else
            {
                printf("%c", str[i]);
            }
            colNum++;
        }
    }
}

void drawCharacterReverse(char *str, int x_position, int y_position)
{
    int lineNum = 0, colNum = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == '\n')
        {
            gotoxy(x_position, y_position + ++lineNum);
            colNum = 0;
        }
        else
        {
            if (str[i] != ' ')
            {
                gotoxy(x_position - colNum, y_position + lineNum);
                printf("%c", str[i]);
            }
            colNum++;
        }
    }
}

void displayMenuWindow()
{
    system("clear");
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;

    printf(KGRN);
    gotoxy(X_POSITION, Y_POSITION - 15);
    printf("🌟 LOGIN 🌟");
    gotoxy(X_POSITION - 1, Y_POSITION - 10);
    printf("🌟 SIGN UP 🌟");
    printf(KRED);
    gotoxy(X_POSITION, Y_POSITION - 5);
    printf("🌟 QUIT 🌟");

    int pointer = Y_POSITION;
    printf(KGRN);

    while (state == MENU)
    {
        sleep(0.3);
        // phat hien nhan phim
        if (kbhit())
        {
            // xoa con tro o vi tri cu
            gotoxy(X_POSITION - 10, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 15, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 10, pointer - 1);
            for (int i = 0; i < 26; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 10, pointer + 1);
            for (int i = 0; i < 26; i++)
            {
                putchar(' ');
            }
            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION + 4;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION + 4)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                state = (pointer - Y_POSITION) / 2 + 1;
            }
        }

        printf(KBLU);

        gotoxy(X_POSITION - 10, pointer);
        putchar(' ');
        gotoxy(X_POSITION + 15, pointer);
        putchar(' ');
        gotoxy(X_POSITION - 10, pointer - 1);
        for (int i = 0; i < 26; i++)
        {
            putchar(' ');
        }
        gotoxy(X_POSITION - 10, pointer + 1);
        for (int i = 0; i < 26; i++)
        {
            putchar(' ');
        }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);
    }
}

void getInput(char key, char *str, int maxlen, int *index, int X_POSITION, int Y_POSITION)
{
    int check_pwd = 0;
    if (key == 127)
    {
        // del 1 ky tu
        if ((*index) > 0)
        {
            str[--(*index)] = '\0';
        }
    }
    if (str[0] == '1') {
        check_pwd = 1;
    }
    if ((*index) < maxlen)
    {

        if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
        {
            str[(*index)++] = key;
            str[(*index)] = '\0';

            printf(KWHT);
            gotoxy(X_POSITION + 2, Y_POSITION);
            if (check_pwd == 1){
                for (int i = 0; i < strlen(str); i++){
                    printf("\u2022");
                }
            } else {
                printf("%s", str);
            }
        }
    }
}

void getInputForMethod(char key, char *str, int maxlen, int *index, int X_POSITION, int Y_POSITION)
{
    int check_pwd = 0;
    if (key == 127)
    {
        // del 1 ky tu
        if ((*index) > 0)
        {
            str[--(*index)] = '\0';
        }
    }
    if (str[0] == '1') {
        check_pwd = 1;
    }
    if ((*index) < maxlen)
    {

        if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
        {
            str[(*index)++] = key;
            str[(*index)] = '\0';

            printf(KWHT);
            gotoxy(X_POSITION + 4, Y_POSITION);
            if (check_pwd == 1){
                for (int i = 0; i < strlen(str); i++){
                    printf("\u2022");
                }
            } else {
                printf("%s", str);
            }
        }
    }
}

void drawPointerBuyMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION){
    gotoxy(0, 0);
    printf("choice: %d", (*choice));
    if ((*choice) == 0 || (*choice) == 1 || (*choice) == 2)
    {
        gotoxy(X_POSITION - 15, Y_POSITION + 2 * (*choice));
        printf("  ");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 7);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 6, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 14, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 6, Y_POSITION + 7);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
    }
    // up left
    if (key == 65 || key == 68)
    {
        if ((*choice) == 0)
        {
            (*choice) = 4;
        }
        else
        {
            (*choice)--;
        }
    }
    // down right enter
    else if (key == 66 || key == 67 || key == 10)
    {
        if ((*choice) == 4)
        {
            (*choice) = 0;
        }
        else
        {
            (*choice)++;
        }
    }
    printf(KYEL);
    if ((*choice) == 0 || (*choice) == 1 || (*choice) == 2)
    {
        gotoxy(X_POSITION - 15, Y_POSITION + 2 * (*choice));
        printf("\u27A4");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 7);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
        }
        else
        {
            gotoxy(X_POSITION + 6, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 14, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 6, Y_POSITION + 7);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
    }

    gotoxy(0, Y_POSITION + 3);
    printf("X        ");
}

void displayBuyMethodWindow(int sockfd){
    clearScreen();
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;
    

    printf(KYEL); // corlor-font: yellow
    gotoxy(X_POSITION-12, Y_POSITION);
    printf("Mã chứng khoán: ");
    gotoxy(X_POSITION-12, Y_POSITION + 2);
    printf("           Giá: ");
    gotoxy(X_POSITION-12, Y_POSITION + 4);
    printf("      Số lượng: ");
    printf(KMAG);// corlor-font: purple
    gotoxy(X_POSITION - 6, Y_POSITION + 6);
    printf("Đặt lệnh");
    gotoxy(X_POSITION + 8, Y_POSITION + 6);
    printf("Back");
    gotoxy(X_POSITION-12, Y_POSITION - 2);
    printf(KRED);// corlor-font: red
    printf("👉 Press up/down to switch your choice 👈");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 15, Y_POSITION);
    printf("\u27A4");
    char name_stock[40], price[40], amount[40], mesg[100];
    int nameStockLen = 0, priceLen = 0, amountLen = 0;

    while (1)
    {
        strcpy(mesg, "");
        strcpy(mesg, respone_msg);
        if(strlen(mesg) > 0){
            int tokenTotal;
            char **data = words(mesg, &tokenTotal, "|");
            SignalState signalState = data[tokenTotal - 1][0] - '0';

            if (signalState == REQUEST_BUY_SUCCESS_SIGNAL)
            {
                gotoxy(X_POSITION - 20, Y_POSITION + 10);
                printf(KGRN);
                printf("                                                                                                     ");
                gotoxy(X_POSITION - 20, Y_POSITION + 10);
                printf(KGRN);
                printf("🎉  %s  🎉 ", mesg);
                strcpy(mesg, name_stock);
                    strcat(mesg, "|");
                    strcat(mesg, price);
                    strcat(mesg, "|");
                    strcat(mesg, amount);
                    addToken(mesg, ORDER_BUY_SIGNAL);
                // while (1)
                // {
                //     if (kbhit())
                //     {
                //         char key3 = getch();
                //         if (key3 == 10)
                //         {
                //             clearScreen();
                //             displayMainMenuWindow(sockfd);
                //             return;
                //         }
                //     }
                // }
                
            }
            if (signalState == SUCCESS_SIGNAL)
                {
                    gotoxy(X_POSITION - 20, Y_POSITION + 10);
                    printf(KGRN);
                    printf("                                                                                                     ");
                    gotoxy(X_POSITION - 5, Y_POSITION + 10);
                    printf(KGRN);
                    printf("🎉  %s  🎉 ", mesg);
                    // while (1)
                    // {
                    //     if (kbhit())
                    //     {
                    //         char key3 = getch();
                    //         if (key3 == 10)
                    //         {
                    //             clearScreen();
                    //             displayMainMenuWindow(sockfd);
                    //             return;
                    //         }
                    //     }
                    // }
                }
        }
        // phat hien nhan phim
        if (kbhit())
        {
            // xoa con tro o vi tri cu

            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65 || key == 66 || key == 67 || key == 68)
            {
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
                printf("                                                ");
                drawPointerBuyMethodWindow(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1 || choice == 2)
                {
                    drawPointerBuyMethodWindow(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 3)  // submit
                {
                    // gotoxy(0, 0);
                    gotoxy(X_POSITION - 12, Y_POSITION + 15);
                    printf(KRED);
                    if (nameStockLen == 0 || priceLen == 0 || amountLen == 0)
                    {
                        if (nameStockLen == 0)
                        {
                            printf("❗️ Vui lòng nhập mã chứng khoán ❗️");
                        }
                        else if (priceLen == 0)
                        {
                            printf("❗️ Vui lòng nhập giá ❗️");
                        }
                        else if (amountLen == 0)
                        {
                            printf("❗️ Vui lòng nhập số lượng ❗️");
                        }
                    }
                    else
                    {
                        strcpy(mesg, name_stock);
                        strcat(mesg, "|");
                        strcat(mesg, price);
                        strcat(mesg, "|");
                        strcat(mesg, amount);
                        addToken(mesg, ORDER_BUY_SIGNAL);

                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                        
                        //recv(sockfd, mesg, 1000, 0);
                    }
                }
                else if (choice == 4) 
                {
                    displayMainMenuWindow(sockfd);
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInputForMethod(key, name_stock, 20, &nameStockLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInputForMethod(key, price, 20, &priceLen, X_POSITION, Y_POSITION + 2);
                }
                else if (choice == 2)
                {
                    getInputForMethod(key, amount, 20, &amountLen, X_POSITION, Y_POSITION + 4);
                }
                
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
            printf("           ");
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
            printf("           ");
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
        }
        else if (choice == 2)
        {
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
            printf("           ");
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
        }
        else
        {
            gotoxy(0, 0);
            printf("          ");
            gotoxy(0, 0);
        }
    }
}

void drawPointerSellMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION){
    // gotoxy(0, 0);
    // printf("choice: %d", (*choice));
    if ((*choice) == 0 || (*choice) == 1 || (*choice) == 2)
    {
        gotoxy(X_POSITION - 15, Y_POSITION + 2 * (*choice));
        printf("  ");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 7);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 6, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 14, Y_POSITION + 6);
            putchar(' ');
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 6, Y_POSITION + 7);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
    }
    // up left
    if (key == 65 || key == 68)
    {
        if ((*choice) == 0)
        {
            (*choice) = 4;
        }
        else
        {
            (*choice)--;
        }
    }
    // down right enter
    else if (key == 66 || key == 67 || key == 10)
    {
        if ((*choice) == 4)
        {
            (*choice) = 0;
        }
        else
        {
            (*choice)++;
        }
    }
    printf(KYEL);
    if ((*choice) == 0 || (*choice) == 1 || (*choice) == 2)
    {
        gotoxy(X_POSITION - 15, Y_POSITION + 2 * (*choice));
        printf("\u27A4");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 7);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
        }
        else
        {
            gotoxy(X_POSITION + 6, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 14, Y_POSITION + 6);
            putchar('|');
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 6, Y_POSITION + 7);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
    }

    gotoxy(0, Y_POSITION + 3);
    printf("        ");
}

void displaySellMethodWindow(int sockfd){
    clearScreen();
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;
    
    char msg[1024];
    printf(KYEL); // corlor-font: yellow
    gotoxy(X_POSITION-12, Y_POSITION);
    printf("Mã chứng khoán: ");
    gotoxy(X_POSITION-12, Y_POSITION + 2);
    printf("           Giá: ");
    gotoxy(X_POSITION-12, Y_POSITION + 4);
    printf("      Số lượng: ");
    printf(KMAG);// corlor-font: purple
    gotoxy(X_POSITION - 6, Y_POSITION + 6);
    printf("Bán");
    gotoxy(X_POSITION + 8, Y_POSITION + 6);
    printf("Back");
    gotoxy(X_POSITION-12, Y_POSITION - 2);
    printf(KRED);// corlor-font: red
    printf("👉 Press up/down to switch your choice 👈");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 15, Y_POSITION);
    printf("\u27A4");

    char name_stock[40], price[40], amount[40], mesg[100];
    int nameStockLen = 0, priceLen = 0, amountLen = 0;

    while (1)
    {
         strcpy(mesg, "");
        strcpy(mesg, respone_msg);
        if(strlen(mesg) > 0){
            int tokenTotal;
            char **data = words(mesg, &tokenTotal, "|");
            SignalState signalState = data[tokenTotal - 1][0] - '0';

            if (signalState == REQUEST_SELL_SUCCESS_SIGNAL)
            {
                gotoxy(X_POSITION - 20, Y_POSITION + 10);
                printf(KGRN);
                printf("                                                                                               ");
                gotoxy(X_POSITION - 20, Y_POSITION + 10);
                printf(KGRN);
                printf("🎉  %s  🎉 ", mesg);
        
            }
            if (signalState == SUCCESS_SIGNAL)
                {
                    gotoxy(X_POSITION - 20, Y_POSITION + 10);
                    printf(KGRN);
                    printf("                                                                                              ");
                    gotoxy(X_POSITION - 5, Y_POSITION + 10);
                    printf(KGRN);
                    printf("🎉  %s  🎉 ", mesg);
                }
        }
        // phat hien nhan phim
        if (kbhit())
        {
            // xoa con tro o vi tri cu

            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65 || key == 66 || key == 67 || key == 68)
            {
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
                printf("                                                ");
                drawPointerSellMethodWindow(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1 || choice == 2)
                {
                    drawPointerSellMethodWindow(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 3)  // submit
                {
                    // gotoxy(0, 0);
                    gotoxy(X_POSITION - 12, Y_POSITION + 15);
                    printf(KRED);
                    if (nameStockLen == 0 || priceLen == 0 || amountLen == 0)
                    {
                        if (nameStockLen == 0)
                        {
                            printf("❗️ Vui lòng nhập mã chứng khoán ❗️");
                        }
                        else if (priceLen == 0)
                        {
                            printf("❗️ Vui lòng nhập giá ❗️");
                        }
                        else if (amountLen == 0)
                        {
                            printf("❗️ Vui lòng nhập số lượng ❗️");
                        }
                    }
                    else
                    {

                        strcpy(mesg, name_stock);
                        strcat(mesg, "|");
                        strcat(mesg, price);
                        strcat(mesg, "|");
                        strcat(mesg, amount);
                        addToken(mesg, ORDER_SELL_SIGNAL);
                        // gotoxy(10, 10);
                        // printf("%s", mesg);

                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                    }
                }
                else if (choice == 4) 
                {
                    displayMainMenuWindow(sockfd);
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInputForMethod(key, name_stock, 20, &nameStockLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInputForMethod(key, price, 20, &priceLen, X_POSITION, Y_POSITION + 2);
                }
                else if (choice == 2)
                {
                    getInputForMethod(key, amount, 20, &amountLen, X_POSITION, Y_POSITION + 4);
                }
                
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
            printf("           ");
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
            printf("           ");
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
        }
        else if (choice == 2)
        {
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
            printf("           ");
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
        }
        else
        {
            gotoxy(0, 0);
            printf("          ");
            gotoxy(0, 0);
        }
    }
}

void displayOnlineUserWindow(int sockfd){
    char users[1024];
    strcpy(users, "Online Users List");
    addToken(users, TRANSACTION_SIGNAL);
    if (send(sockfd, (void *)users, strlen(users), 0) < 0)
    {
        gotoxy(0, 0);
        printf("Error");
    };
        // recv(sockfd, users, 1000, 0);
        clearScreen();
        drawBorder();
        int list_trade[2];
       // gotoxy(25,25);
        int i = 0;
    while(1){
        // if(strlen(respone_msg) > 0){
        //     i++;
        // }
        // gotoxy(21,21);
        // printf("%d",i);
        strcpy(users, respone_msg);
        // gotoxy(20,20);
        // printf("responee:%s",respone_msg);
        //strcpy(respone_msg,"");
        // strcpy(respone_msg, "");
        // memset(respone_msg, 0, sizeof(respone_msg));
        char mesg[100];
        int tokenTotal;
        char **data = words(users, &tokenTotal, "|");
        SignalState signalState = data[tokenTotal - 1][0] - '0';
        int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,

        Y_POSITION = TOP + 4 * HEIGHT / 12;

        printf(KMAG);// corlor-font: purple
        gotoxy(X_POSITION - 12, Y_POSITION);
        printf("💈    Danh sách người dùng online    💈");

        printf(KYEL); // corlor-font: yellow
        gotoxy(X_POSITION - 12, Y_POSITION + 2);
        printf("👉 Nhấn lên/xuống để chọn người dùng 👈");

        int tmp = 0;
        int count = 0;
        // gotoxy(20,20);
        // printf("%lu ",tokenTotal);
        // printf("data 1: %s",data[1]);
        if(signalState == REQUEST_BUY_NOTIFY_SIGNAL){
            displayRequestNotification(sockfd, data[0]);
            return;
        }else if (signalState == REQUEST_SELL_NOTIFY_SIGNAL) {
            displayRequestNotification(sockfd, data[0]);
            return;
        }else {
        if (tokenTotal == 2){
            for (int i = 0; i < tokenTotal - 1 ; i++){
                gotoxy(X_POSITION - 12, Y_POSITION + 4 + tmp);
                printf("                                           ");
                gotoxy(X_POSITION - 12, Y_POSITION + 4 + tmp);
                printf("%s", data[i]);
                tmp = tmp + 2;
                count++;
            }
        }
        if (tokenTotal == 3){
            for (int i = 0; i < tokenTotal - 1 ; i++){
                if(strcmp(data[i],"8Users List")!= 0){
                    gotoxy(X_POSITION - 12, Y_POSITION + 4 + tmp);
                    printf("                                           ");
                    gotoxy(X_POSITION - 12, Y_POSITION + 4 + tmp);
                    printf("%s", data[i]);
                    tmp = tmp + 2;
                    count++;
                }
            }
        }
        int pointer = Y_POSITION + 4, choice;
        printf(KYEL);
            if (kbhit())
            {
                // clear border
                gotoxy(X_POSITION - 17, pointer);
                putchar(' ');
                gotoxy(X_POSITION + 30, pointer);
                putchar(' ');
                gotoxy(X_POSITION - 17, pointer - 1);
                for (int i = 0; i < 48; i++)
                {
                    putchar(' ');
                }
                gotoxy(X_POSITION - 17, pointer + 1);
                for (int i = 0; i < 48; i++)
                {
                    putchar(' ');
                }
                char key = getch();
                // up
                if (key == 65)
                {
                    if (pointer == Y_POSITION + 4)
                    {
                        // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                        pointer = Y_POSITION + 4 + tmp;
                    }
                    else
                    {
                        pointer -= 2;
                    }
                }
                // down
                else if (key == 66)
                {
                    if (pointer == Y_POSITION + 4 + tmp)
                    {
                        // neu dang o duoi cung thi chuyen len tren
                        pointer = Y_POSITION + 4;
                    }
                    else
                    {
                        pointer += 2;
                    }
                }
                // Enter
                else if (key == 10)
                {
                    choice = (pointer - Y_POSITION - 4) / 2;
                    // gotoxy(0,0);
                    // printf("choice: %d", choice);
                    if (choice == 0) {
                        // char mesg[1024];
                        l_user *tmp_account =  get_account(data[0]);
                        if(tmp_account != NULL){
                            displayTransactionWindow(sockfd, tmp_account->id);
                        }
                        break;
                    }
                    else if (choice == 1)
                    {
                        strcpy(mesg, "1");
                        addToken(mesg, CHOICE_USER_OPTION_SIGNAL);
                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                       // clearScreen();
                       // displayTransactionWindow(sockfd);
                        break;
                    }
                    else
                    {
                        clearScreen();
                        displayMainMenuWindow(sockfd);
                        state = MENU;
                    }
                }
            }
            printf(KYEL);
            // draw border of select option
            gotoxy(X_POSITION - 17, pointer);
            putchar('|');
            gotoxy(X_POSITION + 30, pointer);
            putchar('|');
            gotoxy(X_POSITION - 17, pointer - 1);
            for (int i = 0; i < 48; i++)
            {
                putchar('-');
            }
            gotoxy(X_POSITION - 17, pointer + 1);
            for (int i = 0; i < 48; i++)
            {
                putchar('-');
            }
            gotoxy(0, 0);
            printf(KWHT);
            printf("      ");
            gotoxy(0, 0);
        }
    }
}

void displayRequestNotification(int sockfd, char msg[1024]){
    system("clear");
    drawBorder();
    // char mesg[1024];
    // char user_id[20];
    // snprintf(user_id, 20,"%d", id);
    // strcpy(mesg, user_id);
    // addToken(mesg, CHOICE_USER_OPTION_SIGNAL);
    // if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
    // {
    //     gotoxy(0, 0);
    //     printf("Error");
    // };
    char result[1024];
    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    printf(KYEL);
    gotoxy(X_POSITION - 20, Y_POSITION - 13);
    printf("🔔 %s 🔔", msg);
    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION - 10);
    printf("💈    Có     💈");
    gotoxy(X_POSITION, Y_POSITION - 8);
    printf("💈   Không   💈");
    gotoxy(X_POSITION, Y_POSITION - 6);
    printf("💈   Back    💈");
    printf(KRED);
    gotoxy(X_POSITION-5, Y_POSITION - 4);
    printf("👉 Vui lòng chọn Có/Không 👈");

    int pointer = Y_POSITION - 10, choice;
    printf(KYEL);

    while (1)
    {
        int tokenTotal;
        char mesg[1024];
        strcpy(result, respone_msg);
        char **data = words(result, &tokenTotal, "|");
        SignalState signalState = data[tokenTotal - 1][0] - '0';
        if(signalState == YES_SIGNAL){
            gotoxy(X_POSITION - 20, Y_POSITION - 13);
            printf("                                                                         ");
            gotoxy(X_POSITION - 10, Y_POSITION - 13);
            printf("🔔 %s 🔔", data[0]);
            gotoxy(X_POSITION, Y_POSITION - 10);
            printf("                                                                             ");
            printf(KRED);
            gotoxy(X_POSITION-10, Y_POSITION - 10);
            printf("👉 Nhấn enter để hoàn tất  👈");
            gotoxy(X_POSITION, Y_POSITION - 8);
            printf("                                                                             ");
            gotoxy(X_POSITION, Y_POSITION - 6);
            printf("                                                                            ");
            gotoxy(X_POSITION-5, Y_POSITION - 4);   
            printf("                                                                          ");
            if (kbhit())
            {
                char key3 = getch();
                if (key3 == 10)
                {
                    clearScreen();
                    displayMainMenuWindow(sockfd);
                    return;
                }
            }
            // printf(KRED);
            // gotoxy(X_POSITION-5, Y_POSITION - 4);
            // printf("👉 Nhấn enter để hoàn tất  👈");
        } else if(signalState == NO_SIGNAL){
            gotoxy(X_POSITION - 20, Y_POSITION - 13);
            printf("                                                                                        ");
            gotoxy(X_POSITION - 10, Y_POSITION - 13);
            printf("🔔 %s 🔔", data[0]);
            gotoxy(X_POSITION, Y_POSITION - 10);
            printf("                                                                                         ");
            printf(KRED);
            gotoxy(X_POSITION, Y_POSITION - 10);
            printf("👉 Nhấn enter để hoàn tất  👈");
            gotoxy(X_POSITION, Y_POSITION - 8);
            printf("                                                                                         ");
            gotoxy(X_POSITION, Y_POSITION - 6);
            printf("                                                                                    ");
            if (kbhit())
            {
                char key3 = getch();
                if (key3 == 10)
                {
                    clearScreen();
                    displayMainMenuWindow(sockfd);
                    return;
                }
            }
            // printf(KRED);
            // gotoxy(X_POSITION-5, Y_POSITION - 4);
            // printf("👉 Nhấn enter để hoàn tất  👈");
        }
        // sleep(0.3);
        if (kbhit())
        {
            gotoxy(X_POSITION - 10, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 20, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 10, pointer - 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 10, pointer + 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION - 10)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION - 6;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION - 6)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION - 10;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                choice = (pointer - Y_POSITION + 10) / 2;
                // gotoxy(0,0);
                // printf("choice: %d ", choice);
                // printf("pointer: %d ", pointer);
                // printf("y: %d ", Y_POSITION);
                if (choice == 0) {
                    strcpy(mesg, "yes");
                    strcat(mesg, "|");
                    strcpy(mesg, "yes");
                    addToken(mesg, YES_SIGNAL);
                    // gotoxy(40,40);
                    // printf("%s",mesg);
                    if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                    {
                        gotoxy(0, 0);
                        printf("Error");
                    };
                    // gotoxy(X_POSITION - 20, Y_POSITION - 13);
                    // printf("                                                                         ");
                    // gotoxy(X_POSITION - 20, Y_POSITION - 13);
                    // printf("        🔔 Giao dịch thành công 🔔");
                    // printf(KWHT);
                    // displayBuyMethodWindow(sockfd);
                }
                else if (choice == 1)
                {
                    strcpy(mesg, "no");
                    strcat(mesg, "|");
                    strcpy(mesg, "no");
                    addToken(mesg, NO_SIGNAL);
                    if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                    {
                        gotoxy(0, 0);
                        printf("Error");
                    };
                    // gotoxy(X_POSITION - 20, Y_POSITION - 13);
                    // printf("                                                                       ");
                    // gotoxy(X_POSITION - 20, Y_POSITION - 13);
                    // printf("       🔔 Giao dịch thất bại 🔔");
                    // printf(KWHT);
                    // gotoxy(X_POSITION, Y_POSITION - 10);
                    // printf("                 ");
                    // gotoxy(X_POSITION, Y_POSITION - 8);
                    // printf("                 ");
                    // gotoxy(X_POSITION, Y_POSITION - 6);
                    // printf("                  ");
                    // printf(KRED);
                    // gotoxy(X_POSITION-5, Y_POSITION - 4);
                    // printf("👉 Nhấn enter để hoàn tất  👈");
                    // displaySellMethodWindow(sockfd);
                }
                else
                {
                    displayMainMenuWindow(sockfd);
                    //state = MENU;
                    //return;
                }
            }
        }

        printf(KYEL);

        gotoxy(X_POSITION - 10, pointer);
        putchar('|');
        gotoxy(X_POSITION + 20, pointer);
        putchar('|');
        gotoxy(X_POSITION - 10, pointer - 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }
        gotoxy(X_POSITION - 10, pointer + 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);
    }
}

void displayTransactionWindow(int sockfd, int id){
    // char mesg[1024];
    // char user_id[20];
    // snprintf(user_id, 20,"%d", id);
    // strcpy(mesg, user_id);
    // addToken(mesg, CHOICE_USER_OPTION_SIGNAL);
    // if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
    // {
    //     gotoxy(0, 0);
    //     printf("Error");
    // };
   // system("clear");
    clearScreen();
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    
    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION - 10);
    printf("💈   Mua   💈");
    gotoxy(X_POSITION, Y_POSITION - 8);
    printf("💈   Bán   💈");
    gotoxy(X_POSITION, Y_POSITION - 6);
    printf("💈   Back  💈");
    printf(KGRN);
    gotoxy(X_POSITION - 15, Y_POSITION - 15);
    printf("⭐️⭐️⭐️⭐️ Vui lòng chọn hình thức giao dịch ⭐️⭐️⭐️⭐️");

    int pointer = Y_POSITION - 10, choice;
    printf(KYEL);

    while (1)
    {
        sleep(0.3);
        if (kbhit())
        {
            gotoxy(X_POSITION - 10, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 20, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 10, pointer - 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 10, pointer + 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION - 10)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION - 6;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION - 6)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION - 10;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                choice = (pointer - Y_POSITION + 10) / 2;
                // gotoxy(0,0);
                // printf("choice: %d ", choice);
                // printf("pointer: %d ", pointer);
                // printf("y: %d ", Y_POSITION);
                if (choice == 0) {
                    displayBuyTransactionWindow(sockfd, id);
                }
                else if (choice == 1)
                {
                    displaySellTransactionMethodWindow(sockfd, id);
                }
                else
                {
                    displayMainMenuWindow(sockfd);
                    //state = MENU;
                    //return;
                }
            }
        }

        printf(KYEL);

        gotoxy(X_POSITION - 10, pointer);
        putchar('|');
        gotoxy(X_POSITION + 20, pointer);
        putchar('|');
        gotoxy(X_POSITION - 10, pointer - 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }
        gotoxy(X_POSITION - 10, pointer + 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);
    }
}

void displaySellTransactionMethodWindow(int sockfd, int id){
    char user_id[20];
    clearScreen();
    drawBorder();
    char msg[1024];

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;
    

    printf(KYEL); // corlor-font: yellow
    gotoxy(X_POSITION-12, Y_POSITION);
    printf("Mã chứng khoán: ");
    gotoxy(X_POSITION-12, Y_POSITION + 2);
    printf("           Giá: ");
    gotoxy(X_POSITION-12, Y_POSITION + 4);
    printf("      Số lượng: ");
    printf(KMAG);// corlor-font: purple
    gotoxy(X_POSITION - 6, Y_POSITION + 6);
    printf("Gửi");
    gotoxy(X_POSITION + 8, Y_POSITION + 6);
    printf("Back");
    gotoxy(X_POSITION-12, Y_POSITION - 2);
    printf(KRED);// corlor-font: red
    printf("👉 Nhấn lên/xuống để thay đổi lựa chọn  👈");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 15, Y_POSITION);
    printf("\u27A4");

    char name_stock[40], price[40], amount[40], mesg[100];
    int nameStockLen = 0, priceLen = 0, amountLen = 0;

    while (1)
    {
        strcpy(msg, respone_msg);
        // phat hien nhan phim
        char mesg[100];
        int tokenTotal;
        char **data = words(msg, &tokenTotal, "|");
        SignalState signalState = data[tokenTotal - 1][0] - '0';
        if (signalState == REQUEST_SELL_NOTIFY_SIGNAL){
            displayRequestNotification(sockfd, data[0]);
            return;
        } else {
        if (kbhit())
        {
            // xoa con tro o vi tri cu

            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65 || key == 66 || key == 67 || key == 68)
            {
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
                printf("                                                ");
                drawPointerSellMethodWindow(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1 || choice == 2)
                {
                    drawPointerSellMethodWindow(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 3)  // submit
                {
                    // gotoxy(0, 0);
                    gotoxy(X_POSITION - 12, Y_POSITION + 15);
                    printf(KRED);
                    if (nameStockLen == 0 || priceLen == 0 || amountLen == 0)
                    {
                        if (nameStockLen == 0)
                        {
                            printf("❗️ Vui lòng nhập mã chứng khoán ❗️");
                        }
                        else if (priceLen == 0)
                        {
                            printf("❗️ Vui lòng nhập giá ❗️");
                        }
                        else if (amountLen == 0)
                        {
                            printf("❗️ Vui lòng nhập số lượng ❗️");
                        }
                    }
                    else
                    {
                        snprintf(user_id, 20,"%d", id);
                        strcpy(mesg, user_id);
                        // if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        // {
                        //     gotoxy(0, 0);
                        //     printf("Error");
                        // };
                        strcat(mesg, "|");
                        strcat(mesg, name_stock);
                        strcat(mesg, "|");
                        strcat(mesg, price);
                        strcat(mesg, "|");
                        strcat(mesg, amount);
                        addToken(mesg, REQUEST_SELL_SUCCESS_SIGNAL);
                        // gotoxy(10, 10);
                        // printf("%s", mesg);

                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                        //strcpy(mesg, "");
                        //recv(sockfd, mesg, 1000, 0);
                        // gotoxy(0, 0);
                        // printf("%s",mesg);
                        int tokenTotal;
                        char **data = words(mesg, &tokenTotal, "|");
                        SignalState signalState = data[tokenTotal - 1][0] - '0';
                        // if (signalState == REQUEST_SELL_SUCCESS_SIGNAL)
                        // {
                        //     gotoxy(X_POSITION - 20, Y_POSITION + 10);
                        //     printf(KGRN);
                        //     printf("🎉  %s  🎉 ", mesg);
                        //     while (1)
                        //     {
                        //         if (kbhit())
                        //         {
                        //             char key3 = getch();
                        //             if (key3 == 10)
                        //             {
                        //                 clearScreen();
                        //                 displayMainMenuWindow(sockfd);
                        //                 return;
                        //             }
                        //         }
                        //     }
                        // }
                        // if (signalState == SUCCESS_SIGNAL)
                        //         {
                        //             gotoxy(X_POSITION - 20, Y_POSITION + 10);
                        //             printf(KGRN);
                        //             printf("🎉  %s  🎉 ", mesg);
                        //             while (1)
                        //             {
                        //                 if (kbhit())
                        //                 {
                        //                     char key3 = getch();
                        //                     if (key3 == 10)
                        //                     {
                        //                         clearScreen();
                        //                         displayMainMenuWindow(sockfd);
                        //                         return;
                        //                     }
                        //                 }
                        //             }       
                        //         }
                    }
                }
                else if (choice == 4) 
                {
                    displayMainMenuWindow(sockfd);
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInputForMethod(key, name_stock, 20, &nameStockLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInputForMethod(key, price, 20, &priceLen, X_POSITION, Y_POSITION + 2);
                }
                else if (choice == 2)
                {
                    getInputForMethod(key, amount, 20, &amountLen, X_POSITION, Y_POSITION + 4);
                }
                
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
            printf("           ");
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
            printf("           ");
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
        }
        else if (choice == 2)
        {
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
            printf("           ");
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
        }
        else
        {
            gotoxy(0, 0);
            printf("          ");
            gotoxy(0, 0);
        }
        }
    }
}

void displayBuyTransactionWindow(int sockfd, int id){
    // char mesg[1024];
    char user_id[20];
    clearScreen();
    drawBorder();
    char msg[1024];

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;
    
    
    printf(KYEL); // corlor-font: yellow
    gotoxy(X_POSITION-12, Y_POSITION);
    printf("Mã chứng khoán: ");
    gotoxy(X_POSITION-12, Y_POSITION + 2);
    printf("           Giá: ");
    gotoxy(X_POSITION-12, Y_POSITION + 4);
    printf("      Số lượng: ");
    printf(KMAG);// corlor-font: purple
    gotoxy(X_POSITION - 6, Y_POSITION + 6);
    printf("Gửi");
    gotoxy(X_POSITION + 8, Y_POSITION + 6);
    printf("Back");
    gotoxy(X_POSITION-12, Y_POSITION - 2);
    printf(KRED);// corlor-font: red
    printf("👉 Nhấn lên/xuống để thay đổi lựa chọn  👈");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 15, Y_POSITION);
    printf("\u27A4");
    char name_stock[40], price[40], amount[40], mesg[100];
    int nameStockLen = 0, priceLen = 0, amountLen = 0;

    while (1)
    {
        strcpy(msg, respone_msg);
        // gotoxy(20,20);
        // printf("respone1:%s",respone_msg);
        // strcpy(respone_msg,"");
        // strcpy(respone_msg, "");
        // memset(respone_msg, 0, sizeof(respone_msg));
        char mesg[100];
        int tokenTotal;
        char **data = words(msg, &tokenTotal, "|");
        SignalState signalState = data[tokenTotal - 1][0] - '0';
        if (signalState == REQUEST_BUY_NOTIFY_SIGNAL){
            displayRequestNotification(sockfd, data[0]);
            return;
        } else {
        // phat hien nhan phim
        if (kbhit())
        {
            // xoa con tro o vi tri cu

            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65 || key == 66 || key == 67 || key == 68)
            {
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
                printf("                                                ");
                drawPointerBuyMethodWindow(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1 || choice == 2)
                {
                    drawPointerBuyMethodWindow(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 3)  // submit
                {
                    // gotoxy(0, 0);
                    gotoxy(X_POSITION - 12, Y_POSITION + 15);
                    printf(KRED);
                    if (nameStockLen == 0 || priceLen == 0 || amountLen == 0)
                    {
                        if (nameStockLen == 0)
                        {
                            printf("❗️ Vui lòng nhập mã chứng khoán ❗️");
                        }
                        else if (priceLen == 0)
                        {
                            printf("❗️ Vui lòng nhập giá ❗️");
                        }
                        else if (amountLen == 0)
                        {
                            printf("❗️ Vui lòng nhập số lượng ❗️");
                        }
                    }
                    else
                    {
                        snprintf(user_id, 20,"%d", id);
                        strcpy(mesg, user_id);
                        // if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        // {
                        //     gotoxy(0, 0);
                        //     printf("Error");
                        // };
                        strcat(mesg, "|");
                        strcat(mesg, name_stock);
                        strcat(mesg, "|");
                        strcat(mesg, price);
                        strcat(mesg, "|");
                        strcat(mesg, amount);
                        addToken(mesg, REQUEST_BUY_SUCCESS_SIGNAL);
                       // addToken(mesg, ORDER_BUY_SIGNAL);
                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                        // strcpy(mesg, "");
                        // recv(sockfd, mesg, 1000, 0);
                        int tokenTotal;
                        char **data = words(msg, &tokenTotal, "|");
                        SignalState signalState = data[tokenTotal - 1][0] - '0';
                       // gotoxy(0, 0);
                        //printf("_%d_",signalState);
                        // if (signalState == REQUEST_BUY_SUCCESS_SIGNAL)
                        // {
                        //     gotoxy(X_POSITION - 20, Y_POSITION + 10);
                        //     printf(KGRN);
                        //     printf("🎉  %s  🎉 ", mesg);
                        //     while (1)
                        //     {
                        //         if (kbhit())
                        //         {
                        //             char key3 = getch();
                        //             if (key3 == 10)
                        //             {
                        //                 clearScreen();
                        //                 displayMainMenuWindow(sockfd);
                        //                 return;
                        //             }
                        //         }
                        //     }
                        // }
                        // if (signalState == SUCCESS_SIGNAL)
                        //     {
                        //         gotoxy(X_POSITION - 20, Y_POSITION + 10);
                        //         printf(KGRN);
                        //         printf("🎉_ %s _🎉 ", mesg);
                        //         while (1)
                        //         {
                        //             if (kbhit())
                        //             {
                        //                 char key3 = getch();
                        //                 if (key3 == 10)
                        //                 {
                        //                     clearScreen();
                        //                     displayMainMenuWindow(sockfd);
                        //                     return;
                        //                 }
                        //             }
                        //         }
                        //     }
                    }
                }
                else if (choice == 4) 
                {
                    displayMainMenuWindow(sockfd);
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInputForMethod(key, name_stock, 20, &nameStockLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInputForMethod(key, price, 20, &priceLen, X_POSITION, Y_POSITION + 2);
                }
                else if (choice == 2)
                {
                    getInputForMethod(key, amount, 20, &amountLen, X_POSITION, Y_POSITION + 4);
                }
                
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
            printf("           ");
            gotoxy(X_POSITION + 4 + nameStockLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
            printf("           ");
            gotoxy(X_POSITION + 4 + priceLen, Y_POSITION + 2);
        }
        else if (choice == 2)
        {
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
            printf("           ");
            gotoxy(X_POSITION + 4 + amountLen, Y_POSITION + 4);
        }
        else
        {
            gotoxy(0, 0);
            printf("          ");
            gotoxy(0, 0);
        }
    }
    }
}

void displayOrderWindow(int sockfd){
    system("clear");
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    
    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION - 10);
    printf("💈   Mua   💈");
    gotoxy(X_POSITION, Y_POSITION - 8);
    printf("💈   Bán   💈");
    gotoxy(X_POSITION, Y_POSITION - 6);
    printf("💈   Back  💈");
    printf(KGRN);
    gotoxy(X_POSITION-5, Y_POSITION - 15);
    printf("⭐️⭐️⭐️⭐️ ĐẶT LỆNH ⭐️⭐️⭐️⭐️");

    int pointer = Y_POSITION - 10, choice;
    printf(KYEL);

    while (1)
    {
        sleep(0.3);
        if (kbhit())
        {
            gotoxy(X_POSITION - 10, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 20, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 10, pointer - 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 10, pointer + 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION - 10)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION - 6;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION - 6)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION - 10;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                choice = (pointer - Y_POSITION + 10) / 2;
                gotoxy(0,0);
                printf("choice: %d ", choice);
                printf("pointer: %d ", pointer);
                printf("y: %d ", Y_POSITION);
                if (choice == 0) {
                    displayBuyMethodWindow(sockfd);
                }
                else if (choice == 1)
                {
                    displaySellMethodWindow(sockfd);
                }
                else
                {
                    displayMainMenuWindow(sockfd);
                    //state = MENU;
                    //return;
                }
            }
        }

        printf(KYEL);

        gotoxy(X_POSITION - 10, pointer);
        putchar('|');
        gotoxy(X_POSITION + 20, pointer);
        putchar('|');
        gotoxy(X_POSITION - 10, pointer - 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }
        gotoxy(X_POSITION - 10, pointer + 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);
    }
}

void displayAccountInfoWindow(int sockfd, char username[1024]){
    system("clear");
    drawBorder();
    //read_file(FILENAME);
    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    l_user *user =  get_account(username);
    if (user == NULL){
        gotoxy(20,20);
        printf("Can not find user!");
    }else {
        printf(KWHT);
        gotoxy(X_POSITION, Y_POSITION - 10);
        printf("💈   Thông tin cá nhân   💈");
        printf(KYEL);
        gotoxy(X_POSITION, Y_POSITION - 8);
        printf("📍         ID:");
        printf(KWHT);
        gotoxy(X_POSITION + 20, Y_POSITION - 8);
        printf("%d", user->id);
        gotoxy(X_POSITION, Y_POSITION - 6);
        printf(KYEL);
        printf("📍        Tên:");
        printf(KWHT);
        gotoxy(X_POSITION + 20, Y_POSITION - 6);
        printf("%s", user->username);
        gotoxy(X_POSITION, Y_POSITION - 4);
        printf(KYEL);
        printf("📍      Số dư:");
        printf(KWHT);
        gotoxy(X_POSITION + 20, Y_POSITION - 4);
        printf("%d VND", user->balance);
        gotoxy(X_POSITION, Y_POSITION - 2);
        printf(KYEL);
        printf("📍  Trạng thái:");
        printf(KWHT);
        gotoxy(X_POSITION + 20, Y_POSITION - 2);
        if(user->status == 1){
            printf("ACTIVE");
        }
    }

    printf(KMAG);
    gotoxy(X_POSITION , Y_POSITION);
    printf("📈 Danh sách chứng khoán");
    l_stock *stock = user->stock;
    int i = 4;
    gotoxy(X_POSITION + 5, Y_POSITION + 2);
    printf("Mã CK ");
    gotoxy(X_POSITION + 10, Y_POSITION + 2);
    printf("Giá  ");
    gotoxy(X_POSITION + 15, Y_POSITION + 2);
    printf("Số lượng  ");
    while(stock != NULL) {
        gotoxy(X_POSITION + 5, Y_POSITION + i);
        printf("%s ", stock->name);
        gotoxy(X_POSITION + 10, Y_POSITION + i);
        printf("%d ", stock->price);
        gotoxy(X_POSITION + 15, Y_POSITION + i);
        printf("%d ", stock->amount);
        i = i + 2;
        stock = stock->next;
    }

    int pointer = Y_POSITION - 10, choice;
    printf(KYEL);

    while (1)
    {
        sleep(0.3);
        if (kbhit())
        {
            gotoxy(X_POSITION - 10, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 20, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 10, pointer - 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 10, pointer + 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION - 10)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION - 6;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION - 6)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION - 10;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                clearScreen();
                displayMainMenuWindow(sockfd);
            }
        }

        printf(KYEL);

        // gotoxy(X_POSITION - 10, pointer);
        // putchar('|');
        // gotoxy(X_POSITION + 20, pointer);
        // putchar('|');
        // gotoxy(X_POSITION - 10, pointer - 1);
        // for (int i = 0; i < 31; i++)
        // {
        //     putchar('-');
        // }
        // gotoxy(X_POSITION - 10, pointer + 1);
        // for (int i = 0; i < 31; i++)
        // {
        //     putchar('-');
        // }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);
    }
}
void displayBoardWindow(int sockfd){
    system("clear");
    drawBorder();
    char mesg[BUFFER];
    read_file(FILENAME);
    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    printf(KGRN);
    gotoxy(X_POSITION - 5, Y_POSITION - 10);
    printf("💈💈💈💈 BẢNG ĐIỆN  💈💈💈💈");
    printf(KWHT);
    gotoxy(X_POSITION - 40, Y_POSITION - 7);
    printf("Dư mua");
    gotoxy(X_POSITION, Y_POSITION - 7);
    printf("Khớp lệnh");
    gotoxy(X_POSITION + 40, Y_POSITION - 7);
    printf("Dư bán");
    int j = 0;
    for (int i = 0; i < 3; i++){
        gotoxy(X_POSITION - 48 + j, Y_POSITION - 5);
        printf("Mã CK");
        gotoxy(X_POSITION - 35 + j, Y_POSITION - 5);
        printf("Khối lượng");
        gotoxy(X_POSITION - 20 + j, Y_POSITION - 5);
        printf("Giá");
        j += 35;
    }
    
    // khop lenh 
    printf(KYEL);
    gotoxy(X_POSITION - 48 + 35, Y_POSITION - 3);
    printf("L02");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION - 3);
    printf("250");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION - 3);
    printf("50");
    gotoxy(X_POSITION - 48 + 35, Y_POSITION - 1);
    printf("L01");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION - 1);
    printf("200");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION - 1);
    printf("10");
    gotoxy(X_POSITION - 48 + 35, Y_POSITION + 1);
    printf("M1");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION + 1);
    printf("350");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION + 1);
    printf("20");
    gotoxy(X_POSITION - 48 + 35, Y_POSITION + 3);
    printf("M2");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION + 3);
    printf("150");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION + 3);
    printf("20");
    gotoxy(X_POSITION - 48 + 35, Y_POSITION + 5);
    printf("N02");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION + 5);
    printf("270");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION + 5);
    printf("100");
    gotoxy(X_POSITION - 48 + 35, Y_POSITION + 5);
    printf("N01");
    gotoxy(X_POSITION - 35 + 35, Y_POSITION + 5);
    printf("180");
    gotoxy(X_POSITION - 20 + 35, Y_POSITION + 5);
    printf("30");

    printf(KRED);
    gotoxy(X_POSITION - 48, Y_POSITION - 3);
    printf("L02");
    gotoxy(X_POSITION - 35, Y_POSITION - 3);
    printf("235");
    gotoxy(X_POSITION - 20, Y_POSITION - 3);
    printf("30");
    gotoxy(X_POSITION - 48, Y_POSITION - 1);
    printf("L01");
    gotoxy(X_POSITION - 35, Y_POSITION - 1);
    printf("190");
    gotoxy(X_POSITION - 20, Y_POSITION - 1);
    printf("20");
    gotoxy(X_POSITION - 48, Y_POSITION + 1);
    printf("M1");
    gotoxy(X_POSITION - 35, Y_POSITION + 1);
    printf("330");
    gotoxy(X_POSITION - 20, Y_POSITION + 1);
    printf("10");
    gotoxy(X_POSITION - 48, Y_POSITION + 3);
    printf("M2");
    gotoxy(X_POSITION - 35, Y_POSITION + 3);
    printf("140");
    gotoxy(X_POSITION - 20, Y_POSITION + 3);
    printf("20");
    gotoxy(X_POSITION - 48, Y_POSITION + 5);
    printf("N02");
    gotoxy(X_POSITION - 35, Y_POSITION + 5);
    printf("265");
    gotoxy(X_POSITION - 20, Y_POSITION + 5);
    printf("100");
    gotoxy(X_POSITION - 48, Y_POSITION + 5);
    printf("N01");
    gotoxy(X_POSITION - 35, Y_POSITION + 5);
    printf("165");
    gotoxy(X_POSITION - 20, Y_POSITION + 5);
    printf("40");


    printf(KGRN);
    gotoxy(X_POSITION - 48 + 70, Y_POSITION - 3);
    printf("L02");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION - 3);
    printf("280");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION - 3);
    printf("30");
    gotoxy(X_POSITION - 48 + 70, Y_POSITION - 1);
    printf("L01");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION - 1);
    printf("220");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION - 1);
    printf("100");

    gotoxy(X_POSITION - 48 + 70, Y_POSITION + 1);
    printf("M1");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION + 1);
    printf("380");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION + 1);
    printf("20");

    gotoxy(X_POSITION - 48 + 70, Y_POSITION + 3);
    printf("M2");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION + 3);
    printf("160");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION + 3);
    printf("20");
 
    gotoxy(X_POSITION - 48 + 70, Y_POSITION + 5);
    printf("N02");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION + 5);
    printf("280");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION + 5);
    printf("150");

    gotoxy(X_POSITION - 48 + 70, Y_POSITION + 5);
    printf("N01");
    gotoxy(X_POSITION - 35 + 70, Y_POSITION + 5);
    printf("200");
    gotoxy(X_POSITION - 20 + 70, Y_POSITION + 5);
    printf("50");
    // pointer just show the position of border respectsively
    int pointer = Y_POSITION - 7, choice;
    printf(KYEL);

    while (1)
    {   
        gotoxy(X_POSITION - 60, Y_POSITION - 7);
        putchar('|');
        gotoxy(X_POSITION - 15, Y_POSITION - 7);
        putchar('|');
        gotoxy(X_POSITION + 20, Y_POSITION - 7);
        putchar('|');
        gotoxy(X_POSITION + 60, Y_POSITION - 7);
        putchar('|');
        gotoxy(X_POSITION - 60, Y_POSITION - 8);
        for (int i = 0; i < 108; i++)
            {
                putchar('=');
            }
        gotoxy(X_POSITION - 60, Y_POSITION - 6);
            for (int i = 0; i < 108; i++)
            {
                putchar('=');
            }
        if (kbhit())
        {
            char key = getch();
            // Enter
            if (key == 10)
            {
                displayMainMenuWindow(sockfd);
            }
        }
    }

}

void displayMainMenuWindow(int sockfd)
{
    connfd  = sockfd;
    if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
                printf("ERROR: pthread\n");
    }
    system("clear");
    drawBorder();
    char mesg[BUFFER];
    read_file(FILENAME);
    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    printf(KGRN);
    gotoxy(X_POSITION - 5, Y_POSITION - 10);
    printf("🎊🎊🎊🎊 Welcome %s 🎊🎊🎊🎊", username);
    printf(KWHT);
    gotoxy(X_POSITION - 2, Y_POSITION - 7);
    printf("💈    Xem bảng điện    💈");
    gotoxy(X_POSITION - 2, Y_POSITION - 5);
    printf("💈      Đặt lệnh       💈");
    gotoxy(X_POSITION - 2, Y_POSITION - 3);
    printf("💈 Giao dịch trực tiếp 💈");
    gotoxy(X_POSITION - 2, Y_POSITION - 1);
    printf("💈  Quản lí tài khoản  💈");
    gotoxy(X_POSITION - 2, Y_POSITION + 1);
    printf("💈      Đăng xuất      💈");

    // pointer just show the position of border respectsively
    int pointer = Y_POSITION - 7, choice;
    printf(KYEL);

    while (1)
    {
        sleep(0.3);
        if (kbhit())
        {
            // clear border
            gotoxy(X_POSITION - 5, pointer);
            putchar(' ');
            gotoxy(X_POSITION + 25, pointer);
            putchar(' ');
            gotoxy(X_POSITION - 5, pointer - 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 5, pointer + 1);
            for (int i = 0; i < 31; i++)
            {
                putchar(' ');
            }
            char key = getch();
            // up
            if (key == 65)
            {
                if (pointer == Y_POSITION - 7)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION + 1;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION + 1)
                {
                    // neu dang o duoi cung thi chuyen len tren
                    pointer = Y_POSITION - 7;
                }
                else
                {
                    pointer += 2;
                }
            }
            // Enter
            else if (key == 10)
            {
                choice = (pointer - Y_POSITION + 7) / 2;
                // gotoxy(0,0);
                // printf("choice: %d", choice);
                if (choice == 0) {
                    displayBoardWindow(sockfd);
                }
                else if (choice == 1)
                {
                    displayOrderWindow(sockfd);
                }
                else if (choice == 2)
                {
                    displayOnlineUserWindow(sockfd);
                }
                else if (choice == 3)
                {;
                   // printf("_%d_",signalState);
                    displayAccountInfoWindow(sockfd, username);
                }
                else
                {
                    state = MENU;
                    return;
                }
            }
        }
        printf(KYEL);
        // draw border of select option
        gotoxy(X_POSITION - 5, pointer);
        putchar('|');
        gotoxy(X_POSITION + 25, pointer);
        putchar('|');
        gotoxy(X_POSITION - 5, pointer - 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }
        gotoxy(X_POSITION - 5, pointer + 1);
        for (int i = 0; i < 31; i++)
        {
            putchar('-');
        }

        gotoxy(0, 0);
        printf(KWHT);
        printf("      ");
        gotoxy(0, 0);


        // print notification message
        // strcpy(mesg, "");
        // recv(sockfd, mesg, 1000, 0);
        // // gotoxy(0, 0);
        // // printf("%s",mesg);
        // int tokenTotal;
        // char **data = words(mesg, &tokenTotal, "|");
        // SignalState signalState = data[tokenTotal - 1][0] - '0';
        // if (signalState == SUCCESS_SIGNAL)
        // {
        //     gotoxy(X_POSITION - 10, Y_POSITION - 15);
        //     printf(KGRN);
        //     printf("🔔  %s  🔔 ", mesg);
        //     while (1)
        //     {
        //         if (kbhit())
        //         {
        //             char key3 = getch();
        //             if (key3 == 10)
        //             {
        //                 clearScreen();
        //                 displayMainMenuWindow(sockfd);
        //                 return;
        //             }
        //         }
        //     }       
        // }
    }
}

void drawPointerLoginWindow(char key, int *choice, int X_POSITION, int Y_POSITION)
{
    if ((*choice) == 0 || (*choice) == 1)
    {
        // clear pointer when select other field
        gotoxy(X_POSITION - 16, Y_POSITION + 2 * (*choice));
        printf("  ");
    }
    else
    {
        if ((*choice) == 2)
        {
            // delete border line for submit 
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION - 8, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 2, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 10, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 2, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
    }
    if (key == 65 || key == 68)
    {
        if ((*choice) == 0)
        {
            // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
            (*choice) = 3;
        }
        else
        {
            (*choice)--;
        }
    }
    // down
    else if (key == 66 || key == 67 || key == 10)
    {
        if ((*choice) == 3)
        {
            // neu dang o duoi cung thi chuyen len tren
            (*choice) = 0;
        }
        else
        {
            (*choice)++;
        }
    }
    printf(KYEL);
    if ((*choice) == 0 || (*choice) == 1)
    {
        // print pointer
        gotoxy(X_POSITION - 16, Y_POSITION + 2 * (*choice));
        printf("\u27A4");
    }
    else
    {
        // Border for Submit and Back Button
        if ((*choice) == 2)
        {
            gotoxy(X_POSITION - 8, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION - 8, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 8, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
        else
        {
            // for back button
            gotoxy(X_POSITION + 2, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 10, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 2, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
    }

    gotoxy(0, Y_POSITION + 3);
    printf("X        ");
}

void displayLoginWindow(int sockfd)
{
    clearScreen();
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;

    printf(KYEL); // corlor-font: yellow
    gotoxy(X_POSITION-12, Y_POSITION);
    printf("Username: ");
    gotoxy(X_POSITION-12, Y_POSITION + 2);
    printf("Password: ");
    printf(KMAG);// corlor-font: purple
    gotoxy(X_POSITION - 6, Y_POSITION + 5);
    printf("Login");
    gotoxy(X_POSITION + 4, Y_POSITION + 5);
    printf("Back");
    gotoxy(X_POSITION-12, Y_POSITION - 2);
    printf(KRED);// corlor-font: red
    printf("👉 Press up/down to switch your choice 👈");

    int choice = 0;

    printf(KYEL);// corlor-font: yellow
    gotoxy(X_POSITION - 16, Y_POSITION);
    // print first pointer
    printf("\u27A4");

    char mesg[40];
    int usernameLen = 0, passwordLen = 0;

    while (1)
    {
        if (kbhit())
        {
            char key = getch();

            if (key == 65 || key == 66 || key == 67 || key == 68)
            {
                gotoxy(X_POSITION , Y_POSITION + 10);
                printf("                                                ");
                drawPointerLoginWindow(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1)
                {
                    drawPointerLoginWindow(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 2)  
                {
                    // gotoxy(0, 0);
                    gotoxy(X_POSITION - 12, Y_POSITION + 10);
                    printf(KRED);
                    if (usernameLen == 0 || passwordLen == 0)
                    {
                        if (usernameLen == 0)
                        {
                            printf("❗️ Please enter username ❗️");
                        }
                        else if (passwordLen == 0)
                        {
                            printf("❗️ Please enter password ❗️");
                        }
                    }
                    else
                    {
                       // minh|123456|0
                        strcpy(mesg, username);
                        strcat(mesg, "|");
                        strcat(mesg, password);
                        addToken(mesg, LOGIN_SIGNAL);
                        // gotoxy(10, 10);
                        // printf("%s", mesg);
                        if (send(sockfd, mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("error");
                        };
                           // gotoxy(25, 25);
                           //printf("x=%d", x);
                           // printf("_%s_",respone_msg);
                            //strcpy(mesg, "");
                            // gotoxy(0, 0);
                            // printf("Check\n");
                             recv(sockfd, mesg, 1000, 0);
                            // gotoxy(0, 5);
                            // printf("Check2");
                        int tokenTotal;
                        char **data = words(mesg, &tokenTotal, "|");
                        SignalState signalState = data[tokenTotal - 1][0] - '0';
                        // gotoxy(0, 5);
                        // printf("state: %d", signalState);
                        if (signalState == SUCCESS_SIGNAL)
                        {
                            gotoxy(X_POSITION - 12, Y_POSITION + 10);
                            printf(KGRN);
                            printf("🎉 Success! Press Enter to continue 🎉");
                            while (1)
                            {
                                if (kbhit())
                                {
                                    char key3 = getch();
                                    if (key3 == 10)
                                    {
                                        //stage = MAIN_MENU;
                                        displayMainMenuWindow(sockfd);
                                        return;
                                    }
                                }
                             //   gotoxy(0, 0);
                            }
                        }
                    }
                }
                else if (choice == 3) 
                {
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInput(key, username, 20, &usernameLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInput(key, password, 20, &passwordLen, X_POSITION, Y_POSITION + 2);
                }
            }
        }

        printf(KWHT);
        // set position for input form 
        if (choice == 0)
        {
            gotoxy(X_POSITION + 2 + usernameLen, Y_POSITION);
            printf("      ");
            gotoxy(X_POSITION + 2 + usernameLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 2 + passwordLen, Y_POSITION + 2);
            printf("      ");
            gotoxy(X_POSITION + 2 + passwordLen, Y_POSITION + 2);
        }
        else
        {
            gotoxy(0, 0);
            printf("      ");
            gotoxy(0, 0);
        }
    }
}

void drawPointerSignUpPage(char key, int *choice, int X_POSITION, int Y_POSITION)
{
    if ((*choice) == 0 || (*choice) == 1)
    {
        gotoxy(X_POSITION - 4, Y_POSITION + 2 * (*choice));
        printf("  ");
    }
    else
    {
        if ((*choice) == 2)
        {
            gotoxy(X_POSITION - 2, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 8, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION - 2, Y_POSITION + 4);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 11; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 10, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 19, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 10, Y_POSITION + 4);
            for (int i = 0; i < 10; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 10, Y_POSITION + 6);
            for (int i = 0; i < 10; i++)
            {
                putchar(' ');
            }
        }
    }
    if (key == 65 || key == 68)
    {
        if ((*choice) == 0)
        {
            // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
            (*choice) = 3;
        }
        else
        {
            (*choice)--;
        }
    }
    // down
    else if (key == 66 || key == 67 || key == 10)
    {
        if ((*choice) == 3)
        {
            // neu dang o duoi cung thi chuyen len tren
            (*choice) = 0;
        }
        else
        {
            (*choice)++;
        }
    }
    printf(KYEL);
    if ((*choice) == 0 || (*choice) == 1)
    {
        gotoxy(X_POSITION - 4, Y_POSITION + 2 * (*choice));
        printf("\u27A4");
    }
    else
    {
        if ((*choice) == 2)
        {
            gotoxy(X_POSITION - 2, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 8, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION - 2, Y_POSITION + 4);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 11; i++)
            {
                putchar('=');
            }
        }
        else
        {
            gotoxy(X_POSITION + 10, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 19, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 10, Y_POSITION + 4);
            for (int i = 0; i < 10; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 10, Y_POSITION + 6);
            for (int i = 0; i < 10; i++)
            {
                putchar('=');
            }
        }
    }

    gotoxy(0, Y_POSITION + 3);
    printf("X        ");
}

void displaySignUpWindow(int sockfd)
{
    clearScreen();
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 4 * HEIGHT / 12;

    // danh sach cac lua chon
    printf(KYEL);
    gotoxy(X_POSITION, Y_POSITION);
    printf("Username: ");
    gotoxy(X_POSITION, Y_POSITION + 2);
    printf("Password: ");
    printf(KMAG);
    gotoxy(X_POSITION, Y_POSITION + 5);
    printf("Sign up");
    gotoxy(X_POSITION + 12, Y_POSITION + 5);
    printf("Return");
    gotoxy(X_POSITION, Y_POSITION - 2);
    printf(KCYN);
    printf("SIGN IN");
    gotoxy(X_POSITION - 12, Y_POSITION + 8);
    printf(KGRN);
    printf("--Press up/down to switch your choice--");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 4, Y_POSITION);
    printf("\u27A4");

    char username[20], password[20], mesg[40];
    int usernameLen = 0, passwordLen = 0;

    while (1)
    {
        // phat hien nhan phim
        if (kbhit())
        {
            // xoa con tro o vi tri cu

            // lay ma ascii cu phim vua nhan
            char key = getch();
            // up
            if (key == 65 || key == 66)
            {
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
                printf("                                                ");
                drawPointerSignUpPage(key, &choice, X_POSITION, Y_POSITION);
            }
            // Enter
            else if (key == 10)
            {
                if (choice == 0 || choice == 1)
                {
                    drawPointerSignUpPage(key, &choice, X_POSITION, Y_POSITION);
                }
                else if (choice == 2) // Click vao LOGIN
                {
                    // gotoxy(0, 0);
                    // printf("username: %s - password: %s", username, password);
                    gotoxy(X_POSITION - 12, Y_POSITION + 10);
                    printf(KRED);

                    if (usernameLen == 0 || passwordLen == 0)
                    {
                        if (usernameLen == 0)
                        {
                            printf("--Please enter username--");
                        }
                        else if (passwordLen == 0)
                        {
                            printf("--Please enter password--");
                        }
                    }
                    else
                    {
                        strcpy(mesg, username);
                        strcat(mesg, "|");
                        strcat(mesg, password);
                        addToken(mesg, REGISTER_SIGNAL);

                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            printf("error");
                        };
                        strcpy(mesg, "");

                        if (strcmp(username, "long") == 0)
                        {
                            printf("--Username already exist--");
                        }
                        else
                        {
                            printf(KGRN);
                            printf("--Sign up successful, press Enter to continue--");
                            while (1)
                            {
                                if (kbhit())
                                {
                                    char key3 = getch();
                                    if (key3 == 10)
                                    {
                                        state = MENU;
                                        return;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (choice == 3) // Click vao Return
                {
                    state = MENU;
                    break;
                }
            }
            // Nhap username/password tu ban phim
            else
            {
                if (choice == 0)
                {
                    getInput(key, username, 8, &usernameLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInput(key, password, 8, &passwordLen, X_POSITION, Y_POSITION + 2);
                }
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 10 + usernameLen, Y_POSITION);
            printf("      ");
            gotoxy(X_POSITION + 10 + usernameLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 10 + passwordLen, Y_POSITION + 2);
            printf("      ");
            gotoxy(X_POSITION + 10 + passwordLen, Y_POSITION + 2);
        }
        else
        {
            gotoxy(0, 0);
            printf("      ");
            gotoxy(0, 0);
        }
    }
}

