#ifndef MY_LEDS_H
#define MY_LEDS_H

enum LedsName {LEDGREEN = 0, LEDRED = 1, LEDTOTAL = 2};

void ledInit();
void ledOn(char led_name);
void ledOff(char led_name);
void ledTaggle(char led_name);

#endif
