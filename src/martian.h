//
// Created by alejandro on 4/25/22.
//

#include <gtk/gtk.h>

#ifndef UI_MARTIANS_MARTIAN_H
#define UI_MARTIANS_MARTIAN_H

#endif //UI_MARTIANS_MARTIAN_H

typedef struct martian_t {
    int id;
    int x;
    int y;
    int type;
    int energy;
    int totalEnergy;
    SDL_Rect spriteRect;
    SDL_Rect rect;
    GtkWidget *pBar;
    struct martian_t *next;
} martian_t;