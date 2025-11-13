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

#include <stddef.h>
#include <string.h>

#include <SDL3/SDL.h>

#define YAM__CHECK_WINDOW_WAS_CREATED()                         \
        do {                                                    \
                if (state.window_created == YAM_FALSE) {        \
                        yam__error(YAM__WINDOW_NOT_CREATED);    \
                        return YAM_FALSE;                       \
                }                                               \
        } while (YAM_FALSE)

#ifdef YAM_DEBUG_FUNCALLS
#define YAM__DEBUG_FUNCALL() SDL_Log("%s", __func__)
#else
#define YAM__DEBUG_FUNCALL()
#endif


enum yam__error_type {
        YAM__NO_ERROR = 0,
        YAM__WINDOW_ALREADY_CREATED,
        YAM__WINDOW_NOT_CREATED,
        YAM__PIXEL_OUT_OF_BOUNDS,
        YAM__SDL
};

static const char *error_messages[] = {
        [YAM__NO_ERROR] = "",
        [YAM__WINDOW_ALREADY_CREATED] = "The window has already been created",
        [YAM__WINDOW_NOT_CREATED] = "The window has not been created",
        [YAM__PIXEL_OUT_OF_BOUNDS] = "Trying to set a pixel outside the allowed range"
};

static enum yam__error_type current_error;

struct yam__internal_state {
        yam_bool window_created;
        SDL_Window *window;
        SDL_Renderer *renderer;
        SDL_Texture *texture;
        Uint32 *back_buffer;
        size_t back_buffer_bytes;
        int width;
        int height;
};

static struct yam__internal_state state;

static void yam__reset_state(void)
{
        memset(&state, 0, sizeof(state));
        current_error = YAM__NO_ERROR;
}

static void yam__error(enum yam__error_type type)
{
        YAM__DEBUG_FUNCALL();

        current_error = type;
}

static void yam__sdl_error(void)
{
        YAM__DEBUG_FUNCALL();
        current_error = YAM__SDL;
}

const char *yam_get_error(void)
{
        YAM__DEBUG_FUNCALL();

        const char *message = (current_error == YAM__SDL)
                ? SDL_GetError()
                : error_messages[current_error];

        current_error = YAM__NO_ERROR;

        return message;
}

int yam_get_error_code(void)
{
        enum yam__error_type error_code = current_error;
        current_error = YAM__NO_ERROR;
        return error_code;
}

yam_bool yam_create_window(const char *title, int width, int height)
{
        YAM__DEBUG_FUNCALL();

        if (state.window_created == YAM_TRUE) {
                yam__error(YAM__WINDOW_ALREADY_CREATED);
                return YAM_FALSE;
        }

        yam__reset_state();

        if (!SDL_Init(SDL_INIT_VIDEO)) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        state.window = SDL_CreateWindow(title, width, height, 0);
        if (state.window == NULL) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        state.width = width;
        state.height = height;

        state.renderer = SDL_CreateRenderer(state.window, NULL);
        if (state.renderer == NULL) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        state.texture = SDL_CreateTexture(state.renderer,
                                          SDL_PIXELFORMAT_ARGB8888,
                                          SDL_TEXTUREACCESS_STREAMING,
                                          width,
                                          height);
        if (state.texture == NULL) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        state.back_buffer_bytes = sizeof(*state.back_buffer) * width * height;
        state.back_buffer = SDL_malloc(state.back_buffer_bytes);
        if (state.back_buffer == NULL) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        /* TODO: How should vsync be handled? Probably as a
           setting for the user */
        SDL_SetRenderVSync(state.renderer, 1);

        state.window_created = YAM_TRUE;
        return YAM_TRUE;
}

yam_bool yam_destroy_window(void)
{
        YAM__DEBUG_FUNCALL();

        YAM__CHECK_WINDOW_WAS_CREATED();

        SDL_free(state.back_buffer);
        SDL_DestroyTexture(state.texture);
        SDL_DestroyRenderer(state.renderer);
        SDL_DestroyWindow(state.window);
        SDL_Quit();

        yam__reset_state();
        return YAM_TRUE;
}

yam_bool yam_clear_window(void)
{
        YAM__DEBUG_FUNCALL();

        YAM__CHECK_WINDOW_WAS_CREATED();

        memset(state.back_buffer, 0, state.back_buffer_bytes);
        return YAM_TRUE;
}

yam_bool yam_present_window(void)
{
        YAM__DEBUG_FUNCALL();

        YAM__CHECK_WINDOW_WAS_CREATED();

        int pitch;
        void *pixels;
        if (!SDL_LockTexture(state.texture,
                             NULL,
                             &pixels,
                             &pitch))
        {
                yam__sdl_error();
                return YAM_FALSE;
        }

        const size_t row_bytes = state.width * sizeof(*state.back_buffer);
        Uint8 *destination = (Uint8 *)pixels;
        Uint8 *source = (Uint8 *)state.back_buffer;
        for (int y = 0; y < state.height; ++y) {
                memcpy(destination + (y * pitch), source + (y * row_bytes), row_bytes);
        }

        SDL_UnlockTexture(state.texture);

        if (!SDL_RenderClear(state.renderer)) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        if (!SDL_RenderTexture(state.renderer, state.texture, NULL, NULL)) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        if (!SDL_RenderPresent(state.renderer)) {
                yam__sdl_error();
                return YAM_FALSE;
        }

        return YAM_TRUE;
}

yam_bool yam_set_pixel_color(int x, int y, yam_byte r, yam_byte g, yam_byte b, yam_byte a)
{
#ifdef YAM_YES_REALLY_DEBUG_SET_PIXEL_FUNCALL
        YAM__DEBUG_FUNCALL();
#endif

        YAM__CHECK_WINDOW_WAS_CREATED();

#ifndef YAM_NO_BOUNDS_CHECK
        if (x < 0 || y < 0 || x >= state.width || y >= state.height) {
                // TODO: Consider adding string formatting to this error message
                yam__error(YAM__PIXEL_OUT_OF_BOUNDS);
                return YAM_FALSE;
        }
#endif

        state.back_buffer[y * state.width + x]
                = (a << 24)
                | (r << 16)
                | (g << 8)
                | b;

        return YAM_TRUE;
}

int yam_poll_event(void)
{
#ifdef YAM_YES_REALLY_DEBUG_POLL_EVENT_FUNCALL
        YAM__DEBUG_FUNCALL();
#endif

        if (state.window_created == YAM_FALSE) {
                yam__error(YAM__WINDOW_NOT_CREATED);
                return -1;
        }

start:
        ;

        SDL_Event event;
        if (!SDL_PollEvent(&event)) {
                return YAM_EVENT_NONE;
        }

        switch (event.type) {
        case SDL_EVENT_QUIT:
                return YAM_EVENT_QUIT;
        default:
                goto start;
                break;
        }
}
