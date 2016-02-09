#ifndef UART_LOGGER_H
#define UART_LOGGER_H

void loggerInit();
void loggerWrite(const unsigned char *message);
#define LOGGER_END_MARKER '*'
typedef const unsigned char * LogMesT;

#endif
