#ifndef MY_LEDS_H
#define MY_LEDS_H

enum LedsName {LEDGREEN1 = 0, LEDRED1 = 1, LEDGREEN2 = 2, LEDRED2 = 3, LEDTOTAL = 4};

void ledInit();
void ledOn(char led_name);
void ledOff(char led_name);
void ledTaggle(char led_name);

#endif
