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

#ifndef NULL
#define NULL 0
#endif

typedef int yam_bool;
#define YAM_TRUE 1
#define YAM_FALSE 0

typedef void (*yam_error_callback)(int code, const char *message);
typedef void (*yam_close_window_callback)(void);
typedef void (*yam_resize_window_callback)(int new_width, int new_height);

extern void yam_register_error_callback(yam_error_callback callback);
extern void yam_register_close_window_callback(yam_close_window_callback callback);
extern void yam_register_resize_window_callback(yam_resize_window_callback callback);

extern void yam_create_window(const char *title, int width, int height);
extern void yam_destroy_window(void);

/* Functions which must be called after yam_create_window
   and before yam_destroy_window */

extern void yam_poll_events(void);

#endif /* YAM_H */
