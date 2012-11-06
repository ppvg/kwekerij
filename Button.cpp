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

#include <Arduino.h>
#include "Button.h"

Button::Button(uint8_t pin) {
  this->pin = pin;
  this->state = 0;
  this->allow_repeat = true;
  this->last_pressed = 0;
  pinMode(pin, INPUT);
}

void Button::setRepeat(bool repeat) {
  this->allow_repeat = repeat;
}

bool Button::isPressed(void) {
  set(PREVIOUS, get(CURRENT));

  if (digitalRead(pin) == HIGH)
    set(CURRENT, true);
  else
    set(CURRENT, false);

  if (get(PREVIOUS) != get(CURRENT))
    set(CHANGED, true);
  else
    set(CHANGED, false);

  if (get(CURRENT)) {
    if (get(CHANGED)) {
      // freshly pressed
      set(PRESSED, true);
      last_pressed = millis();
      return true;
    } else if (allow_repeat) {
      if (get(REPEATED)) {
        if (millis()-last_pressed > REPEAT_INTERVAL) {
          // consequtive repeats
          last_pressed = millis();
          return true;
        }
      } else if (millis()-last_pressed > REPEAT_DELAY) {
        // first repeat
        set(REPEATED, true);
        last_pressed = millis();
        return true;
      }
    }
  } else if (get(CHANGED)) {
    // button released, reset state
    state = 0;
  }
  return false;
}

/* Returns whether the button was pressed the last time isPressed() was called.*/
bool Button::wasPressed(void) {
  return get(PRESSED);
}

/* Returns whether the button was pressed, and 'freshly' pressed, the last time
 * isPressed() was called. */
bool Button::wasUnique(void) {
  return wasPressed() && !wasRepeat();
}

/* Return whether the button was pressed, and whether that was a repeating press,
 * the last time isPressed() was called. */
bool Button::wasRepeat(void) {
  return get(REPEATED);
}

bool Button::get(unsigned char property) {
  return bitRead(this->state, property);
}

void Button::set(unsigned char property, unsigned char value) {
  bitWrite(this->state, property, value);
}

// vim:softtabstop=2:shiftwidth=2:expandtab:
