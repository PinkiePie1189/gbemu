#ifndef DISPLAY_H
#define DISPLAY_H

#include <SDL2/SDL.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *screen_texture;
    char *title;
    int width;
    int height;
} Display;

void create_display(Display *disp, char *title, int width, int height);
void clear_display(Display *disp);
void draw_pixel(Display *disp, int x, int y, int color);
void present_display(Display *disp);

#endif // DISPLAY_H
