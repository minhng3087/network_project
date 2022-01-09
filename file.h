#include "define.h"

int read_file(char *filename);
void write_file(char *filename);

// order

int read_file_overbought();
int read_file_oversold();
void write_file_overbought();
void write_file_oversold();
void append_file_order_match();

void write_node_to_overfile(char* filename, char username[MAX_CHAR], char name_stock[MAX_CHAR], int price, int amount);
void append_one_stock_to_order_match(char name_stock[MAX_CHAR], int price, int amount);