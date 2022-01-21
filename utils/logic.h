#ifndef __LOGIC_H__
#define __LOGIC_H__

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

void addToken(char *str, SignalState signal);

#endif