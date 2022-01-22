#ifndef __HANDLE_H__
#define __HANDLE_H__

typedef enum {
  LOGIN_SIGNAL,
  LOGOUT_SIGNAL,
  REGISTER_SIGNAL,
  DISCONNECT_SIGNAL,

  MENU_SIGNAL,
  SUCCESS_SIGNAL,
  ORDER_BUY_SIGNAL,
  FAILED_SIGNAL,
  BET
} SignalState;

char **words(char *line, int *total, char *strCut);
int isNumber(char *str);
int isIpV4(char *str);
void clearBuffer();
void addToken(char *str, SignalState signal);

#endif