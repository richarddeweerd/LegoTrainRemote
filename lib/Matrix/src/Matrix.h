#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "Arduino.h"
#include <functional>
#define DEBOUNCE_TIME 50

class MatrixButton
{
public:
    typedef std::function<void(MatrixButton &btn)> CallbackFunction;

    MatrixButton();
    void begin(int id);
    void down();
    void up();
    int getId() { return _id; }
    void setPressedHandler(CallbackFunction f);

private:
    int _id;
    bool _state = false;
    bool _signaled = false;
    unsigned long down_ms;

    CallbackFunction pressed_cb = NULL;
};

class Matrix
{
public:
    Matrix(byte *x_pins, byte x_pincount, byte *y_pins, byte y_pincount, MatrixButton *buttons);

    void begin();
    void loop();

private:
    byte *_x_pins;
    byte *_y_pins;
    byte _x_pincount;
    byte _y_pincount;
    MatrixButton *_buttons;
};
#endif