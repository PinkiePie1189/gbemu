#include "Display.h"

void create_display(Display *disp, char *title, int width, int height) {
    disp->window = SDL_CreateWindow(title,
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height, SDL_WINDOW_SHOWN);
    if (!disp->window) {
        fprintf(stderr, "Window creation failed! %s\n", SDL_GetError());
        return;
    }
    disp->renderer = SDL_CreateRenderer(disp->window, -1,
            SDL_RENDERER_ACCELERATED);
    if (!disp->renderer) {
        fprintf(stderr, "Renderer creation failed! %s\n", SDL_GetError());
        return;
    }
    disp->screen_texture = SDL_CreateTexture(disp->renderer,
        SDL_PIXELFORMAT_RGBA4444, SDL_TEXTUREACCESS_STREAMING,
        width, height);
}

void draw_pixel(Display *disp, int x, int y, int color) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = (color) & 0xFF;
    SDL_SetRenderDrawColor(disp->renderer, r, g, b, SDL_ALPHA_OPAQUE);
    SDL_Rect pixel = {x, y, 1, 1};
    SDL_RenderFillRect(disp->renderer, &pixel);
}

void clear_display(Display *disp) {
    SDL_SetRenderDrawColor(disp->renderer, 0xFF, 0xFF, 0xFF, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(disp->renderer);
}

void present_display(Display *disp) {
    SDL_RenderPresent(disp->renderer);
}
