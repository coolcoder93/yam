/* yam interface.

Copyright (C) 2025 Simon Rehn.

This file is part of yam.

yam is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

yam is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with yam.  If not, see <https://www.gnu.org/licenses/>.  */

#ifndef YAM_H
#define YAM_H

/* The following macros can be defined to change the behaviour of yam:
 * YAM_NO_BOUNDS_CHECK:
    Disables bounds checking for yam_set_pixel_color.
 * YAM_DEBUG_FUNCALLS:
    Calling (most) yam functions will print the function names to stdout.
    The exceptions to this are: [yam_set_pixel_color, yam_poll_event]
    Example:
     Code:
      int main(void)
      {
              yam_create_window("test", 1280, 720);
              yam_set_pixel_color(45, 45, 255, 0, 0, 255);n
              yam_destroy_window();
      }
     Stdout:
      yam_create_window()
      yam_destroy_window()
 * YAM_YES_REALLY_DEBUG_SET_PIXEL_FUNCALL:
    Will print "yam_set_pixel_color" when yam_set_pixel_color is called.
    YAM_DEBUG_FUNCALLS has to be enabled.
 * YAM_YES_REALLY_DEBUG_POLL_EVENT_FUNCALL:
    Will print "yam_poll_event" when yam_poll_event is called.
    YAM_DEBUG_FUNCALLS has to be enabled.
 */

typedef int yam_bool;
#define YAM_TRUE 1
#define YAM_FALSE 0

typedef unsigned char yam_byte;

/* Events that can be returned by yam_poll_event */
#define YAM_EVENT_NONE 0
#define YAM_EVENT_QUIT 1

extern const char *yam_get_error(void);
extern int yam_get_error_code(void);

extern yam_bool yam_create_window(const char *title, int width, int height);
extern yam_bool yam_destroy_window(void);

/* Functions which must be called after yam_create_window
   and before yam_destroy_window */

extern yam_bool yam_clear_window(void);
extern yam_bool yam_present_window(void);
extern yam_bool yam_set_pixel_color(int x, int y, yam_byte r, yam_byte g, yam_byte b, yam_byte a);
// Returns -1 on error
extern int yam_poll_event(void);

#endif /* YAM_H */
