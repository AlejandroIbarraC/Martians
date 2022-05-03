#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>

#include "martian.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// CONSTANTS
// Window dimensions
static const int width = 1200;
static const int height = 840;
static const int sdl_width = 840;
static const int squareLength = 40;
static const int FPS = 60;

// VARIABLES
// SDL variables
static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

// GTK variables
static GtkWindow *gtkWindow;
static GtkWidget *gtkHbox;
static GtkWidget *gtkDA;
static GtkWidget *radio_green;
static GtkWidget *radio_blue;
static GtkWidget *radio_red;
static GtkWidget *pBarVBox;
static void *gdk_window;
static void *window_id;

GtkWidget *entry;

// Control variables
static int sprite = 0;
static int f_time = 0;

// SDL variables
static SDL_Texture *background;
static SDL_Texture *martianSS_green;
static SDL_Texture *martianSS_blue;
static SDL_Texture *martianSS_red;
static SDL_Rect backgroundRect;

// Data variables
martian_t *martians;

// Function declaration
martian_t* addMartian(int x, int y, int type, int totalEnergy);
void buttonAddMartian(GtkWidget* widget, gpointer data);
void destroy(GtkWidget* widget, gpointer data);
static gboolean idle(void *ud);
SDL_Texture* loadImage(const char* file, SDL_Renderer *renderer);
void moveMartian(int direction, martian_t *martian);
gboolean onKeyPress(GtkWidget *widget, GdkEventKey *event, gpointer data);
void renderMartian(martian_t *martian);

static int counter = 0;

int main(int argc, char** argv) {
    // Start SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Main GTK window
    gtk_init(&argc, &argv);
    gtkWindow = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(gtkWindow, "Martians");
    gtk_window_set_default_size(gtkWindow, width, height);

    // GTK main boxes
    gtkHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 740);
    GtkWidget *controlPanel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(controlPanel, 10);

    // GTK graphic elements
    gtkDA = gtk_drawing_area_new();
    GtkWidget *mainTitle = gtk_label_new("Martians");
    entry = gtk_entry_new();

    // Martian color radio selectors
    radio_green = gtk_radio_button_new_with_label(NULL, "Green");
    radio_blue = gtk_radio_button_new_with_label(NULL, "Blue");
    radio_red = gtk_radio_button_new_with_label(NULL, "Red");
    gtk_radio_button_join_group((GtkRadioButton *) radio_blue, (GtkRadioButton *) radio_green);
    gtk_radio_button_join_group((GtkRadioButton *) radio_red, (GtkRadioButton *) radio_green);
    GtkWidget *radio_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_hbox), radio_green, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(radio_hbox), radio_blue, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(radio_hbox), radio_red, FALSE, FALSE, 10);

    GtkWidget *button = gtk_button_new_with_label ("Add Martian");

    // Progress bar stuff
    pBarVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    PangoAttrList *mainTitleAttrs = pango_attr_list_new();
    pango_attr_list_insert(mainTitleAttrs, pango_attr_size_new(28 * PANGO_SCALE));
    gtk_label_set_attributes((GtkLabel*)mainTitle, mainTitleAttrs);
    gtk_widget_set_halign(mainTitle, GTK_ALIGN_CENTER);


    gtk_entry_set_placeholder_text((GtkEntry*) entry, "Enter text here");
    g_signal_connect(button, "clicked",G_CALLBACK (buttonAddMartian), "button");


    // Add elements in boxes
    gtk_container_add (GTK_CONTAINER(gtkWindow), gtkHbox);
    gtk_box_pack_start (GTK_BOX(gtkHbox), gtkDA, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (gtkHbox), controlPanel, TRUE, TRUE, 0);

    // Control panel elements
    gtk_box_pack_start(GTK_BOX(controlPanel), mainTitle, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entry, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), button, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), pBarVBox, FALSE, FALSE, 10);

    gtk_widget_show_all ((GtkWidget*) gtkWindow);

    // Get window ID for SDL window creation
    gdk_window = gtk_widget_get_window(GTK_WIDGET(gtkDA));
    window_id = (void*)(intptr_t)GDK_WINDOW_XID(gdk_window);

    // GTK idle function control SDL loop
    g_idle_add(&idle, 0);

    // Handle program exit
    g_signal_connect(gtkWindow, "destroy", G_CALLBACK(destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(gtkWindow), 0);

    gtk_widget_add_events((GtkWidget *) gtkWindow, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(gtkWindow), "key_press_event", G_CALLBACK(onKeyPress), NULL);

    // Start GTK
    gtk_main();

    return 0;
}

/**
 * Adds martian to linked list
 * @param x x coordinate
 * @param y y coordinate
 */
martian_t* addMartian(int x, int y, int type, int totalEnergy) {
    martian_t *returnMartian;
    if (martians->x == -100) {
        // Head is uninitialized
        martians->id = 0;
        martians->x = x;
        martians->y = y;
        martians->type = type;
        martians->energy = totalEnergy;
        martians->totalEnergy = totalEnergy;
        martians->next = NULL;
        returnMartian = martians;
    } else {
        // Create martian
        martian_t *martian = (martian_t*)malloc(sizeof(martian_t));
        martian->x = x;
        martian->y = y;
        martian->type = type;
        martian->energy = totalEnergy;
        martians->totalEnergy = totalEnergy;
        martian->next = NULL;

        // Append to end of martian list
        martian_t *last = martians;
        while (last->next != NULL) {
            last = last->next;
        }

        // UPDATE ID
        martian->id = last->id;
        last->next = martian;

        returnMartian = martian;
        printf("Added martian on %d, %d, type %d\n", martian->x, martian->y, martian->type);
    }
    return returnMartian;
}

/*
 * Called on Add Martian button
 */
void buttonAddMartian(GtkWidget* widget, gpointer data) {
    g_print("%s clicked %d times\n", (char*)data, ++counter);
    const gchar *entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    g_print("Entry text is: %s\n", entryText);

    // Get selected martian type
    int type;
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_green)) == TRUE) {
        type = 0;
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_blue)) == TRUE) {
        type = 1;
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_red)) == TRUE) {
        type = 2;
    }
    // Create martian struct
    int totalEnergy = 24;
    martian_t *martian = addMartian(0, 440, type, totalEnergy);

    // Draw energy progress bar
    GtkWidget *pBar = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text((GtkProgressBar *) pBar, TRUE);
    martian->pBar = pBar;

    char idLabel[3];
    sprintf(idLabel, "%d", martian->id);
    char pBarLabel[11] = "Marciano ";
    strcat(pBarLabel, idLabel);

    gtk_progress_bar_set_text((GtkProgressBar *) pBar, pBarLabel);
    gtk_progress_bar_set_fraction((GtkProgressBar *) pBar, (gdouble) 10 / totalEnergy);
    gtk_box_pack_start(GTK_BOX(pBarVBox), pBar, FALSE, FALSE, 0);

    printf("Added martian type %d\n", type);
}

/**
 * Clears main GTK and SDL memory on program exit
 */
void destroy(GtkWidget* widget, gpointer data) {
    // Safe memory exit. Delete stuff
    g_print("Deleting stuff for safe exit\n");

    // Delete SDL stuff
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(martianSS_green);
    SDL_DestroyTexture(martianSS_blue);
    SDL_DestroyTexture(martianSS_red);
    IMG_Quit();
    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();

    // Close GTK
    gtk_main_quit();
}

/**
 * GTK loop function on idle
 */
static gboolean idle(void *ud) {
    (void)ud;
    if(!sdlWindow) {
        // Create window
        printf("Creating SDL window for window id %p\n", window_id);
        sdlWindow = SDL_CreateWindowFrom(window_id);
        SDL_SetWindowSize(sdlWindow, sdl_width, height);
        printf("sdl_window=%p\n", sdlWindow);
        if(!sdlWindow) {
            printf("%s\n", SDL_GetError());
        }

        // Create renderer
        sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        printf("sdl_renderer=%p\n", sdlRenderer);
        if(!sdlRenderer) {
            printf("%s\n", SDL_GetError());
        }
        IMG_Init(IMG_INIT_PNG);

        // Load images
        background = loadImage("../src/img/wework.png", sdlRenderer);
        martianSS_green = loadImage("../src/img/mmsheet_green.png", sdlRenderer);
        martianSS_blue = loadImage("../src/img/mmsheet_blue.png", sdlRenderer);
        martianSS_red = loadImage("../src/img/mmsheet_red.png", sdlRenderer);

        backgroundRect = (SDL_Rect){0, 0, sdl_width, height};

        // Initialize martian linked list
        martians = (martian_t*)malloc(sizeof(martian_t));
        martians->x = -100;
        martians->next = NULL;
    } else {
        // Main SDL loop
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, background, NULL, &backgroundRect);

        // Update every martian
        renderMartian(martians);
        martian_t *last = martians;
        while (last->next != NULL) {
            renderMartian(last);
            last = last->next;
        }
        renderMartian(last);

        SDL_RenderPresent(sdlRenderer);

        // Handle frame on loop
        f_time++;
        if (FPS / f_time == 4) {
            f_time = 0;
            sprite++;
        }
        SDL_Delay(1000/120);
    }

    // Refresh GTK widgets
    gtk_widget_show_all ((GtkWidget*) gtkWindow);
    return true;
}

/**
 * Create SDL texture from image file
 * @param file path to image JPG or PNG
 * @param renderer SDL renderer
 * @return SDL texture
 */
SDL_Texture* loadImage(const char* file, SDL_Renderer* renderer) {
    // Load image as surface
    SDL_Surface* image = IMG_Load(file);

    // Verify file is correctly found
    if (image == NULL ){
        printf( "Unable to load image. SDL_image Error: %s\n", IMG_GetError());
    }

    // Convert to texture and delete surface
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
    SDL_FreeSurface(image);
    return texture;
}

/**
 * Moves martian in a specific direction
 * @param direction 0 UP, 1 DOWN, 2 LEFT, 3 RIGHT
 * @param martian martian_t object to move
 */
void moveMartian(int direction, martian_t *martian) {
    switch (direction) {
        case 0:
            // Up
            if (martian->y > 0) {
                martian->y -= 40;
            }
            break;
        case 1:
            // Down
            if (martian->y < 800) {
                martian->y += 40;
            }
            break;
        case 2:
            // Left
            if (martian->x > 0) {
                martian->x -= 40;
            }
            break;
        case 3:
            // Right
            if (martian->x < 800) {
                martian->x += 40;
            }
            break;
        default:
            printf("Invalid direction for moving martian: %d\n", direction);
    }
}

/**
 * Handles key presses using GTK
 * @return Boolean if key detection was successful
 */
gboolean onKeyPress (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    gboolean result = TRUE;
    switch (event->keyval) {
        case GDK_KEY_Up:
            moveMartian(0, martians);
            break;
        case GDK_KEY_Down:
            moveMartian(1, martians);
            break;
        case GDK_KEY_Left:
            moveMartian(2, martians);
            break;
        case GDK_KEY_Right:
            moveMartian(3, martians);
            break;
        default:
            result = FALSE;
    }
    return result;
}

/**
 * Renders martian on SDL UI
 * @param martian to render
 */
void renderMartian(martian_t *martian) {
    // Calculate SDL variables
    martian->spriteRect = (SDL_Rect){(sprite % 7)*165, 0, 165, 165};
    martian->rect = (SDL_Rect){martian->x, martian->y, squareLength, squareLength};

    // Set textures
    SDL_Texture *texture = martianSS_green;
    if (martian->type == 1) {
        texture = martianSS_blue;
    } else if (martian->type == 2) {
        texture = martianSS_red;
    }

    // Render stuff
    SDL_RenderCopyEx(sdlRenderer, texture, &martian->spriteRect, &martian->rect, 0, NULL, 0);
}
