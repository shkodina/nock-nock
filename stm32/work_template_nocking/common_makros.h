#ifndef COMMON_MAKROS_H
#define COMMON_MAKROS_H

enum BOOLEAN {FALSE = 0, TRUE = 1};
#define INVBOOL(value) value = value == TRUE ? FALSE : TRUE;

#define INVBIT(port, bit) port = port ^ (1<<bit);
#define UPBIT(port, bit) port = port | (1<<bit);
#define DOWNBIT(port, bit) port = port & (~(1<<bit));


#endif
