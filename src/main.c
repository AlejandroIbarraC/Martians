#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

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
static void *gdk_window;
static void *window_id;

GtkWidget *entry;

// Control variables
static int sprite = 0;
static int f_time = 0;

// SDL variables
static SDL_Texture *background;
static SDL_Texture *martianSS;
static SDL_Rect backgroundRect;

// TEST
martian_t *martian;

// Function declaration
void destroy(GtkWidget* widget, gpointer data);
static gboolean idle(void *ud);
SDL_Texture* loadImage(const char* file, SDL_Renderer *renderer);
void moveMartian(int direction);
gboolean onKeyPress(GtkWidget *widget, GdkEventKey *event, gpointer data);

void initSDL();

static int counter = 0;
void greet(GtkWidget* widget, gpointer data) {
    g_print("%s clicked %d times\n", (char*)data, ++counter);
    const gchar *entryText = gtk_entry_get_text(GTK_ENTRY(entry));
    g_print("Entry text is: %s\n", entryText);
}

int main(int argc, char** argv) {
    // Start SDL
    SDL_Init(SDL_INIT_VIDEO);

    // Main GTK window
    gtk_init(&argc, &argv);
    gtkWindow = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(gtkWindow, "Martians");
    gtk_window_set_default_size(gtkWindow, width, height);

    // GTK main boxes
    gtkHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 680);
    GtkWidget *controlPanel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(controlPanel, 10);

    // GTK graphic elements
    gtkDA = gtk_drawing_area_new();
    GtkWidget *mainTitle = gtk_label_new("Martians");
    entry = gtk_entry_new();
    GtkWidget *button = gtk_button_new_with_label ("Button");

    PangoAttrList *mainTitleAttrs = pango_attr_list_new();
    pango_attr_list_insert(mainTitleAttrs, pango_attr_size_new(28 * PANGO_SCALE));
    gtk_label_set_attributes((GtkLabel*)mainTitle, mainTitleAttrs);
    gtk_widget_set_halign(mainTitle, GTK_ALIGN_CENTER);


    gtk_entry_set_placeholder_text((GtkEntry*) entry, "Enter text here");
    g_signal_connect(button, "clicked",G_CALLBACK (greet), "button");


    // Add elements in boxes
    gtk_container_add (GTK_CONTAINER(gtkWindow), gtkHbox);
    gtk_box_pack_start (GTK_BOX(gtkHbox), gtkDA, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (gtkHbox), controlPanel, TRUE, TRUE, 0);

    // Control panel elements
    gtk_box_pack_start(GTK_BOX(controlPanel), mainTitle, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entry, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), button, FALSE, FALSE, 10);

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
 * Clears main GTK and SDL memory on program exit
 */
void destroy(GtkWidget* widget, gpointer data) {
    // Safe memory exit. Delete stuff
    g_print("Deleting stuff for safe exit\n");

    // Delete SDL stuff
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(martianSS);
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
        martianSS = loadImage("../src/img/mmsheet.png", sdlRenderer);

        backgroundRect = (SDL_Rect){0, 0, sdl_width, height};

        // TEST MARTIAN
        martian = (martian_t*)malloc(sizeof(martian_t));
        martian->x = 0;
        martian->y = 440;
    } else {
        // Main SDL loop
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, background, NULL, &backgroundRect);

        // TEST MARTIAN
        martian->spriteRect = (SDL_Rect){(sprite % 7)*165, 0, 165, 165};
        martian->rect = (SDL_Rect){martian->x, martian->y, squareLength, squareLength};
        SDL_RenderCopyEx(sdlRenderer, martianSS, &martian->spriteRect, &martian->rect, 0, NULL, 0);

        SDL_RenderPresent(sdlRenderer);

        // Handle frame on loop
        f_time++;
        if (FPS / f_time == 4) {
            f_time = 0;
            sprite++;
        }
        SDL_Delay(1000/120);
    }
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
void moveMartian(int direction) {
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
            moveMartian(0);
            break;
        case GDK_KEY_Down:
            moveMartian(1);
            break;
        case GDK_KEY_Left:
            moveMartian(2);
            break;
        case GDK_KEY_Right:
            moveMartian(3);
            break;
        default:
            result = FALSE;
    }
    return result;
}

//void initSDL() {
//    // Initialize SDL things
//    SDL_Init(SDL_INIT_VIDEO);
//    SDL_Window* window = SDL_CreateWindow("Martians", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
//    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
//    IMG_Init(IMG_INIT_PNG);
//
//    // Load images
//    SDL_Texture* background = loadImage("../src/img/wework.png", renderer);
//    SDL_Texture* martianSS = loadImage("../src/img/mmsheet.png", renderer);
//
//    SDL_Rect backgroundRect = {0, 0, width, height};
//
//    // Variables
//    // GUI
//    bool quit = false;
//    int FPS = 60;
//    int f_time = 0;
//    int sprite = 0;
//    SDL_Event event;
//
//    // TEST MARTIAN
//    martian_t *martian = (martian_t*)malloc(sizeof(martian_t));
//    martian->x = 0;
//    martian->y = 440;
//
//    // SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Martians UI", "Welcome to Martians UI", NULL);
//
//    // Main UI loop
//    while (!quit) {
//        SDL_StartTextInput();
//
//        while (SDL_PollEvent(&event)) {
//            switch (event.type) {
//                case SDL_QUIT:
//                    quit = true;
//                    break;
//                case SDL_KEYDOWN:
//                    switch (event.key.keysym.sym) {
//                        case SDLK_x:
//                            // Exit and show report
//                            quit = true;
//                            break;
//                        case SDLK_UP:
//                            moveMartian(0, martian);
//                            break;
//                        case SDLK_DOWN:
//                            moveMartian(1, martian);
//                            break;
//                        case SDLK_LEFT:
//                            moveMartian(2, martian);
//                            break;
//                        case SDLK_RIGHT:
//                            moveMartian(3, martian);
//                            break;
//                    }
//            }
//        }
//
//        // Render actions
//        SDL_RenderClear(renderer);
//        SDL_RenderCopy(renderer, background, NULL, &backgroundRect);
//
//        // TEST MARTIAN
//        martian->spriteRect = (SDL_Rect){(sprite % 7)*165, 0, 165, 165};
//        martian->rect = (SDL_Rect){martian->x, martian->y, squareLength, squareLength};
//        SDL_RenderCopyEx(renderer, martianSS, &martian->spriteRect, &martian->rect, 0, NULL, 0);
//
//        // Render current frame
//        SDL_RenderPresent(renderer);
//
//        // Handle frame on loop
//        f_time++;
//        if (FPS / f_time == 4) {
//            f_time = 0;
//            sprite++;
//        }
//
//        SDL_Delay(1000/120);
//    }
//
//    // Safe memory exit. Delete stuff.
//    SDL_DestroyTexture(background);
//    IMG_Quit();
//    SDL_DestroyRenderer(renderer);
//    SDL_DestroyWindow(window);
//    SDL_Quit();
//}