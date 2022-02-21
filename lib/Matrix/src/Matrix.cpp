#include "Matrix.h"
MatrixButton::MatrixButton() {}

void MatrixButton::begin(int id)
{
    _id = id;
}

void MatrixButton::setPressedHandler(CallbackFunction f)
{
    pressed_cb = f;
}

void MatrixButton::down()
{
    if (!_state)
    {
        _state = true;
        down_ms = millis();
    }
    else
    {
        if (!_signaled)
        {
            if ((millis() - down_ms) >= DEBOUNCE_TIME)
            {
                _signaled = true;
                pressed_cb(*this);
            }
        }
    }
}

void MatrixButton::up()
{
    _state = false;
    _signaled = false;
}

Matrix::Matrix(byte *x_pins, byte x_pincount, byte *y_pins, byte y_pincount, MatrixButton *buttons)
{
    _buttons = buttons;
    _x_pins = x_pins;
    _y_pins = y_pins;
    _x_pincount = x_pincount;
    _y_pincount = y_pincount;
}

void Matrix::begin()
{
    for (int i = 0; i < _x_pincount; i++)
    {
        pinMode(_x_pins[i], OUTPUT);
        digitalWrite(_x_pins[i], HIGH);
    }
    for (int i = 0; i < _y_pincount; i++)
    {
        pinMode(_y_pins[i], INPUT);
    }
}

void Matrix::loop()
{
    for (int i = 0; i < _x_pincount; i++)
    {
        digitalWrite(_x_pins[i], LOW);
        for (int j = 0; j < _y_pincount; j++)
        {
            if (digitalRead(_y_pins[j]) == 0)
            {
                _buttons[(i * 4) + j].down();
            }
            else
            {
                _buttons[(i * 4) + j].up();
            }
        }
        digitalWrite(_x_pins[i], HIGH);
    }
}
