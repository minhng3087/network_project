
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
void displayOrderWindow(int sockfd);
void displayBuyMethodWindow(int sockfd);
void displaySellMethodWindow(int sockfd);
void drawPointerLoginWindow(char key, int *choice, int X_POSITION, int Y_POSITION);
void drawPointerBuyMethodWindow(char key, int *choice, int X_POSITION, int Y_POSITION);
void drawPointerSignUpPage(char key, int *choice, int X_POSITION, int Y_POSITION);

void returnMenu();

void quit();