
int kbhit(void);
int getch(void);

void gotoxy(int x, int y);
void delay(int seconds); 
char *readFile(char *filename);

void clearRect(int start_x, int start_y, int end_x, int end_y); 
void clearRectReverse(int start_x, int start_y, int width, int height);
void clearScreen();                                             
void drawBorder();                                                                                
void testDraw();


void displayMenuWindow(); 
void displayLoginWindow(int sockfd); 
void displaySignUpWindow(int sockfd); 
void displayMainMenuWindow(int sockfd);
void displayOnlineUserWindow(int sockfd);
void drawOnlineUserWindow();
void displayTransactionWindow(int sockfd, int id);
void displayBuyTransactionWindow(int sockfd, int id);
void displayRequestNotification(int sockfd, char msg[1024]);
void displaySellTransactionMethodWindow(int sockfd, int id);
void displayOrderWindow(int sockfd);
void displayAccountInfoWindow(int sockfd, char username[1024]);
void displayBuyMethodWindow(int sockfd);
void displaySellMethodWindow(int sockfd);
void drawPointerLoginWindow(char key, int *choice, int X_POSITION, int Y_POSITION);
void drawPointerBuyMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION);
void drawPointerSellMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION);
void drawPointerSignUpPage(char key, int *choice, int X_POSITION, int Y_POSITION);

void returnMenu();

void quit();