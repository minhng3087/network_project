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
#define BUFFER 200
char username[20], password[20];

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

    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION);
    printf("LOGIN");
    gotoxy(X_POSITION, Y_POSITION + 5);
    printf("SIGN UP");
    gotoxy(X_POSITION, Y_POSITION + 10);
    printf("QUIT");

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
    if (key == 127)
    {
        // del 1 ky tu
        if ((*index) > 0)
        {
            str[--(*index)] = '\0';
        }
    }
    if ((*index) < maxlen)
    {

        if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9'))
        {
            str[(*index)++] = key;
            str[(*index)] = '\0';

            printf(KWHT);
            gotoxy(X_POSITION + 10, Y_POSITION);
            printf("%s", str);
        }
    }
}

void drawPointerBuyMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION){
    gotoxy(0, 0);
    printf("choice: %d", (*choice));
    if ((*choice) == 0 || (*choice) == 1 || (*choice) == 2)
    {
        gotoxy(X_POSITION - 2, Y_POSITION + 2 * (*choice));
        printf("  ");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 2, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION - 2, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 8, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 17, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 8, Y_POSITION + 4);
            for (int i = 0; i < 10; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 8, Y_POSITION + 6);
            for (int i = 0; i < 10; i++)
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
        gotoxy(X_POSITION - 4, Y_POSITION + 2 * (*choice));
        printf("\u27A4");
    }
    else
    {
        if ((*choice) == 3)
        {
            gotoxy(X_POSITION - 2, Y_POSITION + 7);
            putchar('|');
            gotoxy(X_POSITION + 6, Y_POSITION + 7);
            putchar('|');
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 8);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
        else
        {
            gotoxy(X_POSITION + 8, Y_POSITION + 7);
            putchar('|');
            gotoxy(X_POSITION + 17, Y_POSITION + 7);
            putchar('|');
            gotoxy(X_POSITION + 8, Y_POSITION + 6);
            for (int i = 0; i < 10; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 8, Y_POSITION + 8);
            for (int i = 0; i < 10; i++)
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

    printf(KYEL);
    gotoxy(X_POSITION-2, Y_POSITION);
    printf("Stock name: ");
    gotoxy(X_POSITION, Y_POSITION + 2);
    printf("Price: ");
    gotoxy(X_POSITION, Y_POSITION + 4);
    printf("Amount: ");
    printf(KMAG);
    gotoxy(X_POSITION, Y_POSITION + 7);
    printf("Submit");
    gotoxy(X_POSITION + 10, Y_POSITION + 7);
    printf("Back");
    gotoxy(X_POSITION, Y_POSITION - 2);
    printf(KRED);
    printf("Press up/down to switch your choice");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 4, Y_POSITION);
    printf("\u27A4");

    char name_stock[40], price[40], amount[40], mesg[100];
    int nameStockLen = 0, priceLen = 0, amountLen = 0;

    while (1)
    {
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
                            printf("Please enter stock name!");
                        }
                        else if (priceLen == 0)
                        {
                            printf("Please enter price !");
                        }
                        else if (amountLen == 0)
                        {
                            printf("Please enter amount !");
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
                        // gotoxy(10, 10);
                        // printf("%s", mesg);

                        if (send(sockfd, (void *)mesg, strlen(mesg), 0) < 0)
                        {
                            gotoxy(0, 0);
                            printf("Error");
                        };
                        strcpy(mesg, "");
                            gotoxy(0, 0);
                            printf("Check");
                        recv(sockfd, mesg, 1000, 0);
                        //int tokenTotal;
                        //char **data = words(mesg, &tokenTotal, "|");
                        //SignalState signalState = data[tokenTotal - 1][0] - '0';

                        if (5 == SUCCESS_SIGNAL)
                        {
                            gotoxy(X_POSITION - 12, Y_POSITION + 15);
                            printf("Order Match Success (1) !!! Press Enter to continue");
                            while (1)
                            {
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
                             //   gotoxy(0, 0);
                            }
                        }
                    }
                }
                else if (choice == 4) 
                {
                    state = MENU;
                    break;
                }
            }
            else
            {
                if (choice == 0)
                {
                    getInput(key, name_stock, 20, &nameStockLen, X_POSITION, Y_POSITION);
                }
                else if (choice == 1)
                {
                    getInput(key, price, 20, &priceLen, X_POSITION, Y_POSITION + 2);
                }
                else if (choice == 2)
                {
                    getInput(key, amount, 20, &amountLen, X_POSITION, Y_POSITION + 4);
                }
                
            }
        }

        printf(KWHT);
        if (choice == 0)
        {
            gotoxy(X_POSITION + 10 + nameStockLen, Y_POSITION);
            printf("           ");
            gotoxy(X_POSITION + 10 + nameStockLen, Y_POSITION);
        }
        else if (choice == 1)
        {
            gotoxy(X_POSITION + 10 + priceLen, Y_POSITION + 2);
            printf("           ");
            gotoxy(X_POSITION + 10 + priceLen, Y_POSITION + 2);
        }
        else if (choice == 2)
        {
            gotoxy(X_POSITION + 10 + amountLen, Y_POSITION + 4);
            printf("           ");
            gotoxy(X_POSITION + 10 + amountLen, Y_POSITION + 4);
        }
        else
        {
            gotoxy(0, 0);
            printf("          ");
            gotoxy(0, 0);
        }
    }
}

void displaySellMethodWindow(int sockfd){

}

void displayOrderWindow(int sockfd){
    system("clear");
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;

    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION);
    printf("Buy");
    gotoxy(X_POSITION, Y_POSITION + 2);
    printf("Sell");
    gotoxy(X_POSITION, Y_POSITION + 4);
    printf("Back");

    int pointer = Y_POSITION, choice;
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
                if (pointer == Y_POSITION)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION + 6;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION + 6)
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
                choice = (pointer - Y_POSITION) / 2;
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
                    state = MENU;
                    return;
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

void displayMainMenuWindow(int sockfd)
{
    system("clear");
    drawBorder();

    int X_POSITION = MARGIN_LEFT + 5 * WIDTH / 12,
        Y_POSITION = TOP + 5 * HEIGHT / 12;
    printf(KGRN);
    gotoxy(X_POSITION, Y_POSITION-5);
    printf("🐶🐶🐶🐶🐶Welcome %s🐶🐶🐶🐶🐶", username);
    printf(KWHT);
    gotoxy(X_POSITION, Y_POSITION);
    printf("Xem bảng điện");
    gotoxy(X_POSITION, Y_POSITION + 2);
    printf("Đặt lệnh");
    gotoxy(X_POSITION, Y_POSITION + 4);
    printf("Giao dịch trực tiếp");
    gotoxy(X_POSITION, Y_POSITION + 6);
    printf("Quản lí tài khoản");
    gotoxy(X_POSITION, Y_POSITION + 8);
    printf("Log out");

    int pointer = Y_POSITION, choice;
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
                if (pointer == Y_POSITION)
                {
                    // neu con tro dang o vi tri cao nhat thi chuyen xuong cuoi
                    pointer = Y_POSITION + 8;
                }
                else
                {
                    pointer -= 2;
                }
            }
            // down
            else if (key == 66)
            {
                if (pointer == Y_POSITION + 8)
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
                choice = (pointer - Y_POSITION) / 2;
                gotoxy(0,0);
                printf("choice: %d", choice);
                if (choice == 0) {
                    // return;
                }
                else if (choice == 1)
                {
                    displayOrderWindow(sockfd);
                }
                else
                {
                    state = MENU;
                    return;
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

void drawPointerLoginWindow(char key, int *choice, int X_POSITION, int Y_POSITION)
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
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION - 2, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar(' ');
            }
        }
        else
        {
            gotoxy(X_POSITION + 8, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 17, Y_POSITION + 5);
            putchar(' ');
            gotoxy(X_POSITION + 8, Y_POSITION + 4);
            for (int i = 0; i < 10; i++)
            {
                putchar(' ');
            }
            gotoxy(X_POSITION + 8, Y_POSITION + 6);
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
            gotoxy(X_POSITION + 6, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION - 2, Y_POSITION + 4);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION - 2, Y_POSITION + 6);
            for (int i = 0; i < 9; i++)
            {
                putchar('=');
            }
        }
        else
        {
            gotoxy(X_POSITION + 8, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 17, Y_POSITION + 5);
            putchar('|');
            gotoxy(X_POSITION + 8, Y_POSITION + 4);
            for (int i = 0; i < 10; i++)
            {
                putchar('=');
            }
            gotoxy(X_POSITION + 8, Y_POSITION + 6);
            for (int i = 0; i < 10; i++)
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

    printf(KYEL);
    gotoxy(X_POSITION, Y_POSITION);
    printf("Username: ");
    gotoxy(X_POSITION, Y_POSITION + 2);
    printf("Password: ");
    printf(KMAG);
    gotoxy(X_POSITION, Y_POSITION + 5);
    printf("Login");
    gotoxy(X_POSITION + 10, Y_POSITION + 5);
    printf("Back");
    gotoxy(X_POSITION, Y_POSITION - 2);
    printf(KRED);
    printf("Press up/down to switch your choice");

    int choice = 0;

    printf(KYEL);
    gotoxy(X_POSITION - 4, Y_POSITION);
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
                gotoxy(X_POSITION - 12, Y_POSITION + 10);
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
                            printf("Please enter username !");
                        }
                        else if (passwordLen == 0)
                        {
                            printf("Please enter password !");
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
                            printf("Success! Press Enter to continue");
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

