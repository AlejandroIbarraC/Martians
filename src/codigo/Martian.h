
#include <gtk/gtk.h>
#include <SDL_rect.h>
#ifndef MARTIAN_H
#define MARTIAN_H


typedef struct {
    int id;
    int x;
    int y;
    int row;
    int col;
    int type;
    int priority;
    int energy;
    int executedtime;
    int executiontime;//totalEnergy
    int period;
    int ready;
    int arrivalTime;
    int destX;
    int destY;
    int currentDirection;
    int movDir;
    SDL_Rect spriteRect;
    SDL_Rect rect;
    GtkWidget *pBar;
} Martian;
#endif //MARTIAN_H
