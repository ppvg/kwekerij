/*
* Copyright (C) 2012  Peter-Paul van Gemerden
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __BUTTON_H__
#define __BUTTON_H__

// Behaviour configuration
#define REPEAT_DELAY    500
#define REPEAT_INTERVAL 180

// State constants
#define CURRENT  0
#define PREVIOUS 1
#define CHANGED  2
#define PRESSED  3
#define REPEATED 4

class Button {
  public:
    Button(unsigned char pin);
    void setRepeat(bool repeat);
    bool isPressed();
    bool wasPressed();
    bool wasUnique();
    bool wasRepeat();
  private:
    unsigned char pin;
    bool allow_repeat;
    unsigned char state;
    unsigned long last_pressed;
    bool get(unsigned char property);
    void set(unsigned char property, unsigned char value);
};

#endif

// vim:softtabstop=2:shiftwidth=2:expandtab:
