#ifndef MY_BUTTONS_H
#define MY_BUTTONS_H

enum ButtonNames {BUTTONDOOR = 0, BUTTONNEWNOCK = 1, BUTTONTOTAL = 2};

char buttonIsPressed(char button_name);
void buttonsInit();

#endif
