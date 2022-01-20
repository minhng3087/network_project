
int kbhit(void);
int getch(void);

void gotoxy(int x, int y);
void delay(int seconds); // delay (tham so truyen vao la so ms delay)
char *readFile(char *filename);

void clearRect(int start_x, int start_y, int end_x, int end_y); 
void clearRectReverse(int start_x, int start_y, int width, int height);
void clearScreen();                                             
void drawBorder();                                         
void drawSelectMenu();                                         
void testDraw();


void displayMenuWindow(); 
void displayLoginWindow(int sockfd); 
void displaySignUpWindow(int sockfd); 
void drawReadyPage();

void returnMenu();

void quit();