#ifndef UART_LOGGER_H
#define UART_LOGGER_H

void loggerInit();
void loggerWrite(const unsigned char *message, char count);
void loggerWriteToMarker(const unsigned char *message, char end_marker);
void loggerWriteByteInBit(const unsigned char byte);
unsigned char getData(char * buffer, unsigned char buf_len);
#define LOGGER_END_MARKER '*'
typedef const unsigned char * LogMesT;

#endif
