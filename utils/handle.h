#ifndef __HANDLE_H__
#define __HANDLE_H__

typedef enum {
  LOGIN_SIGNAL,
  MENU_SIGNAL,
  SUCCESS_SIGNAL,
  ORDER_BUY_SIGNAL,
  ORDER_SELL_SIGNAL,
  REQUEST_BUY_SUCCESS_SIGNAL,
  REQUEST_SELL_SUCCESS_SIGNAL,
  ACCOUNT_INFO_SIGNAL,
  TRANSACTION_SIGNAL,
  FAILED_SIGNAL,
  LOGOUT_SIGNAL,
  REGISTER_SIGNAL,
} SignalState;

char **words(char *line, int *total, char *strCut);
int isNumber(char *str);
int isIpV4(char *str);
void clearBuffer();
void addToken(char *str, SignalState signal);

#endif