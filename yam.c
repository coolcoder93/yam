/* yam implementation.

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

#include "yam.h"

#include <SDL3/SDL.h>

enum yam__error_type {
        YAM__WINDOW_ALREADY_CREATED,
        YAM__WINDOW_NOT_CREATED,
        YAM__SDL
};

static const char *error_messages[] = {
        [YAM__WINDOW_ALREADY_CREATED] = "The window has already been created",
        [YAM__WINDOW_NOT_CREATED] = "The window has not been created"
};

struct yam__internal_state {
        yam_bool window_created;
        SDL_Window *window;
        yam_error_callback error_callback;
        yam_close_window_callback close_callback;
        yam_resize_window_callback resize_callback;
};

static struct yam__internal_state state;

#define YAM__CHECK_WINDOW_WAS_CREATED()                         \
        do {                                                    \
                if (state.window_created == YAM_FALSE) {        \
                        yam__error(YAM__WINDOW_NOT_CREATED);    \
                        return;                                 \
                }                                               \
        } while (YAM_FALSE)

static void yam__error(enum yam__error_type type)
{
        if (state.error_callback != NULL) {
                state.error_callback(type, error_messages[type]);
        }
}

static void yam__sdl_error(void)
{
        if (state.error_callback != NULL) {
                state.error_callback(YAM__SDL, SDL_GetError());
        }
}

void yam_register_error_callback(yam_error_callback callback)
{
        state.error_callback = callback;
}

void yam_register_close_window_callback(yam_close_window_callback callback)
{
        state.close_callback = callback;
}

void yam_register_resize_window_callback(yam_resize_window_callback callback)
{
        state.resize_callback = callback;
}

void yam_create_window(const char *title, int width, int height)
{
        if (state.window_created == YAM_TRUE) {
                yam__error(YAM__WINDOW_ALREADY_CREATED);
                return;
        }

        if (!SDL_Init(SDL_INIT_VIDEO)) {
                yam__sdl_error();
                return;
        }

        state.window = SDL_CreateWindow(title, width, height, 0);
        if (state.window == NULL) {
                yam__sdl_error();
                return;
        }

        state.window_created = YAM_TRUE;
}

void yam_destroy_window(void)
{
        YAM__CHECK_WINDOW_WAS_CREATED();

        SDL_DestroyWindow(state.window);
        SDL_Quit();

        state.window_created = YAM_FALSE;
}

void yam_poll_events(void)
{
        YAM__CHECK_WINDOW_WAS_CREATED();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
                switch (event.type) {
                /* TODO: Callbacks !! :) */
                case SDL_EVENT_QUIT:
                        if (state.close_callback != NULL) {
                                state.close_callback();
                        }
                        break;
                case SDL_EVENT_WINDOW_RESIZED:
                        if (state.resize_callback != NULL) {
                                state.resize_callback(event.window.data1,
                                                      event.window.data2);
                        }
                        break;
                default:
                        break;
                }
        }
}
