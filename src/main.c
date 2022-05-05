#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>
#include "codigo/logic.c"
#include "codigo/Martian.h"
#include "codigo/Martian_node.h"

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

static GtkWidget *radio_os1;
static GtkWidget *radio_os2;

static GtkWidget *radio_cal1;
static GtkWidget *radio_cal2;
static GtkWidget *radio_cal3;

static GtkWidget *radio_green;
static GtkWidget *radio_blue;
static GtkWidget *radio_red;
static GtkWidget *radio_color_hbox;

static GtkWidget *entryEnergy;
static GtkWidget *entryPeriod;
static GtkWidget *entryArrivalTime;

static GtkWidget *pBarVBox;
static void *gdk_window;
static void *window_id;

// Control variables
static int sprite = 0;
static int f_time = 0;
static bool running = false;

// SDL variables
static SDL_Texture *background;
static SDL_Texture *martianSS_green;
static SDL_Texture *martianSS_blue;
static SDL_Texture *martianSS_red;
static SDL_Rect backgroundRect;



// Function declaration
Martian* addMartian(int x, int y, int type, int totalEnergy, int period, int arrivalTime);
void buttonAddMartian(GtkWidget* widget, gpointer data);
void buttonStartStop(GtkWidget* widget, gpointer data);
void deleteMartian(struct Martian_node *martian) ;
void destroy(GtkWidget* widget, gpointer data);
void destroy_aux();
static gboolean idle(void *ud);
SDL_Texture* loadImage(const char* file, SDL_Renderer *renderer);

gboolean onKeyPress(GtkWidget *widget, GdkEventKey *event, gpointer data);
void renderControlPanel(GtkWidget* widget, gpointer data);
void renderMartian(Martian *martian);

int main(int argc, char** argv) {
    // Start SDL

    SDL_Init(SDL_INIT_VIDEO);

    // Main GTK window
    gtk_init(&argc, &argv);
    gtkWindow = (GtkWindow*)gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(gtkWindow, "Martians");
    gtk_window_set_default_size(gtkWindow, width, height);
    gtk_window_set_resizable(gtkWindow, FALSE);

    // GTK main boxes
    gtkHbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 820);
    GtkWidget *controlPanel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_set_margin_end(controlPanel, 10);

    // GTK graphic elements
    gtkDA = gtk_drawing_area_new();
    GtkWidget *mainTitle = gtk_label_new("Martians");
    entryEnergy = gtk_entry_new();
    entryPeriod = gtk_entry_new();
    entryArrivalTime = gtk_entry_new();

    // OS radio selectors
    GtkWidget *os_label = gtk_label_new("Sim Type:");
    radio_os1 = gtk_radio_button_new_with_label(NULL, "Interactive");
    radio_os2 = gtk_radio_button_new_with_label(NULL, "RTOS");
    gtk_radio_button_join_group((GtkRadioButton *) radio_os2, (GtkRadioButton *) radio_os1);
    GtkWidget *radio_os_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_os_hbox), os_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_os_hbox), radio_os1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_os_hbox), radio_os2, FALSE, FALSE, 0);

    g_signal_connect(radio_os1, "clicked", G_CALLBACK(renderControlPanel), "button");

    // Calendarizer radio selectors
    GtkWidget *cal_label = gtk_label_new("Calendarization:");
    radio_cal1 = gtk_radio_button_new_with_label(NULL, "FCFS");
    radio_cal2 = gtk_radio_button_new_with_label(NULL, "Priority");
    radio_cal3 = gtk_radio_button_new_with_label(NULL, "SRTN");
    gtk_radio_button_join_group((GtkRadioButton *) radio_cal2, (GtkRadioButton *) radio_cal1);
    gtk_radio_button_join_group((GtkRadioButton *) radio_cal3, (GtkRadioButton *) radio_cal1);
    GtkWidget *radio_cal_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), cal_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal3, FALSE, FALSE, 0);

    // Martian color radio selectors
    GtkWidget *color_label = gtk_label_new("Priority:");
    radio_green = gtk_radio_button_new_with_label(NULL, "Green");
    radio_blue = gtk_radio_button_new_with_label(NULL, "Blue");
    radio_red = gtk_radio_button_new_with_label(NULL, "Red");
    gtk_radio_button_join_group((GtkRadioButton *) radio_blue, (GtkRadioButton *) radio_green);
    gtk_radio_button_join_group((GtkRadioButton *) radio_red, (GtkRadioButton *) radio_green);
    radio_color_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_color_hbox), color_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_green, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_blue, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_red, FALSE, FALSE, 0);

    // Buttons
    GtkWidget *gtkButtonAddMartian = gtk_button_new_with_label ("Add Martian");
    GtkWidget *gtkButtonStartStop = gtk_button_new_with_label ("Start/Stop");

    // Progress bar stuff
    GtkWidget *pBar_label = gtk_label_new("Energy bars:");
    pBarVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    PangoAttrList *mainTitleAttrs = pango_attr_list_new();
    pango_attr_list_insert(mainTitleAttrs, pango_attr_size_new(20 * PANGO_SCALE));
    gtk_label_set_attributes((GtkLabel*)mainTitle, mainTitleAttrs);
    gtk_widget_set_halign(mainTitle, GTK_ALIGN_CENTER);


    gtk_entry_set_placeholder_text((GtkEntry*) entryEnergy, "Energy");
    gtk_entry_set_placeholder_text((GtkEntry*) entryPeriod, "Period");
    gtk_entry_set_placeholder_text((GtkEntry*) entryArrivalTime, "Arrival Time");

    g_signal_connect(gtkButtonAddMartian, "clicked", G_CALLBACK (buttonAddMartian), "button");
    g_signal_connect(gtkButtonStartStop, "clicked", G_CALLBACK (buttonStartStop), "button");

    // Add elements in boxes
    gtk_container_add (GTK_CONTAINER(gtkWindow), gtkHbox);
    gtk_box_pack_start (GTK_BOX(gtkHbox), gtkDA, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX (gtkHbox), controlPanel, TRUE, TRUE, 0);

    // Control panel elements
    gtk_box_pack_start(GTK_BOX(controlPanel), mainTitle, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_os_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_cal_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryEnergy, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryPeriod, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryArrivalTime, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_color_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), gtkButtonAddMartian, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), gtkButtonStartStop, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), pBar_label, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), pBarVBox, FALSE, FALSE, 10);

    gtk_widget_show_all((GtkWidget*) gtkWindow);

    // Default hidden elements
    // gtk_widget_hide(entryEnergy);
    // gtk_widget_hide(entryPeriod);

    // Get window ID for SDL window creation
    gdk_window = gtk_widget_get_window(GTK_WIDGET(gtkDA));
    window_id = (void*)(intptr_t)GDK_WINDOW_XID(gdk_window);

    // GTK idle function control SDL loop
    g_idle_add(&idle, 0);
    initialize();
    // Handle program exit
    g_signal_connect(gtkWindow, "destroy", G_CALLBACK(destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(gtkWindow), 0);

    gtk_widget_add_events((GtkWidget *) gtkWindow, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(gtkWindow), "key_press_event", G_CALLBACK(onKeyPress), NULL);

    // Start GTK
    gtk_main();
    freedata();

    return 0;
}


/**
 * Called on Add Martian button
 */
void buttonAddMartian(GtkWidget* widget, gpointer data) {
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
    const gchar *entryEnergyText = gtk_entry_get_text(GTK_ENTRY(entryEnergy));
    const gchar *entryPeriodText = gtk_entry_get_text(GTK_ENTRY(entryPeriod));
    const gchar *entryArrivalTimeText = gtk_entry_get_text(GTK_ENTRY(entryArrivalTime));

    int totalEnergy = (int) strtol(entryEnergyText, NULL, 10);
    int period = (int) strtol(entryPeriodText, NULL, 10);
    int arrivalTime = (int) strtol(entryArrivalTimeText, NULL, 10);

    // Variable verification
    if (totalEnergy <= 0 || totalEnergy > 100) {
        printf("Energy error\n");
    } else if (period <= 0 || period > 100) {
        printf("Period error\n");
    } else if (arrivalTime <= 0 || arrivalTime > 100) {
        printf("Arrival time error\n");
    } else {
        printf("Energy %d, Period %d, Arrival time %d\n", totalEnergy, period, arrivalTime);
        Martian *martian = addMartian(0, squareLength * 11, type, totalEnergy, period, arrivalTime);

        // Draw energy progress bar
        GtkWidget *pBar = gtk_progress_bar_new();
        gtk_progress_bar_set_show_text((GtkProgressBar *) pBar, TRUE);
        martian->pBar = pBar;

        gtk_box_pack_start(GTK_BOX(pBarVBox), pBar, FALSE, FALSE, 0);

        printf("Added martian type %d\n", type);
    }
}

/**
 * Called when button start/stop is clicked
 */
void buttonStartStop(GtkWidget* widget, gpointer data) {
    if (running) {
        running = false;
        printf("Stopped Simulation\n");
    } else {
        running = true;
        printf("Started simulation\n");
    }
}

/**
 * Deletes martian
 * @param martian to delete
 */
void deleteMartian(struct Martian_node *martian) {
    struct Martian_node* martians=Head;
    if (martian == martians) {
        // Delete head of list
        printf("Here\n");
        gtk_container_remove((GtkContainer *) pBarVBox, martians->martian->pBar);
        martians->martian->pBar = NULL;
        if (martians->next != NULL) {
            // List has more than 1 item. Can delete head.
            martians = martians->next;
        } else {
            // List only has head. Disable it.
            martians->martian->x = -100;
        }
    } else {
        // Delete item in the middle or end of list
        struct Martian_node *tmp = martians;
        while (tmp->next != NULL) {
            if (martian == tmp->next) {
                // tmp has the previous value
                break;
            }
            tmp = tmp->next;
        }
        // Reassign pointers
        tmp->next = martian->next;
        gtk_container_remove((GtkContainer *) pBarVBox, martian->martian->pBar);
        free(martian);
    }
}

/**
 * Clears main GTK and SDL memory on program exit
 */
void destroy(GtkWidget* widget, gpointer data) {
    destroy_aux();
}

void destroy_aux() {
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


    } else {
        // Main SDL loop
        SDL_RenderClear(sdlRenderer);
        SDL_RenderCopy(sdlRenderer, background, NULL, &backgroundRect);

        // Update every martian
        int size=get_size();
        for (int i=0; i<size; i++) {
            renderMartian(find(i));
        }


        // Render stuff
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
    gtk_widget_show_all((GtkWidget*) gtkWindow);
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
 * Handles key presses using GTK
 * @return Boolean if key detection was successful
 */
gboolean onKeyPress (GtkWidget *widget, GdkEventKey *event, gpointer data) {
    gboolean result = TRUE;
    switch (event->keyval) {
        case GDK_KEY_x:
            destroy_aux();
            break;
        default:
            result = FALSE;
    }


    return result;
}

/**
 * Render control panel elements depending on selected OS type
 */
void renderControlPanel(GtkWidget* widget, gpointer data) {
    //gtk_widget_show_all((GtkWidget *) gtkWindow);
//    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os1)) == TRUE) {
//        // Interactive OS
//        gtk_widget_show(radio_color_hbox);
//        gtk_widget_show(entryArrivalTime);
//        gtk_widget_hide(entryEnergy);
//        gtk_widget_hide(entryPeriod);
//    } else {
//        // RTOS
//        gtk_widget_hide(radio_color_hbox);
//        gtk_widget_hide(entryArrivalTime);
//        gtk_widget_show(entryEnergy);
//        gtk_widget_show(entryPeriod);
//    }
    printf("Changed\n");
}

/**
 * Renders martian on SDL UI
 * @param martian to render
 */
void renderMartian(Martian *martian) {
    // Animate movement on loop
    if (martian->movDir != -1) {
        switch (martian->movDir) {
            case 0:
                // Up
                if (martian->destY != martian->y) {
                    martian->y -= 8;
                } else {
                    martian->destY = -1;
                    martian->movDir = -1;
                }
                break;
            case 1:
                // Down
                if (martian->destY != martian->y) {
                    martian->y += 8;
                } else {
                    martian->destY = -1;
                    martian->movDir = -1;
                }
                break;
            case 2:
                // Left
                if (martian->destX != martian->x) {
                    martian->x -= 8;
                } else {
                    martian->destX = -1;
                    martian->movDir = -1;
                }
                break;
            case 3:
                // Right
                if (martian->destX != martian->x) {
                    martian->x += 8;
                } else {
                    martian->destX = -1;
                    martian->movDir = -1;
                }
                break;
        }
    }

    // Calculate SDL variables
    martian->spriteRect = (SDL_Rect){(sprite % 7)*165, 0, 165, 165};
    martian->rect = (SDL_Rect){martian->x, martian->y, squareLength, squareLength};

    drawDual();

    // Set textures
    SDL_Texture *texture = martianSS_green;
    if (martian->type == 1) {
        texture = martianSS_blue;
    } else if (martian->type == 2) {
        texture = martianSS_red;
    }

    // Energy bar
    GtkWidget *pBar = martian->pBar;
    if (pBar != NULL) {
        // Build label
        char idLabel[5];
        char typeLabel[10];
        char energyStr[4];
        char totalEnergyStr[4];

        sprintf(idLabel, "%d", martian->id);
        sprintf(energyStr, "%d", martian->energy);
        sprintf(totalEnergyStr, "%d", martian->executiontime);

        int mType = martian->type;
        switch (mType) {
            case 0:
                strcpy(typeLabel, " (Green) ");
                break;
            case 1:
                strcpy(typeLabel, " (Blue) ");
                break;
            case 2:
                strcpy(typeLabel, " (Red) ");
                break;
            default:
                strcpy(typeLabel, " (Error) ");
                break;
        }

        // Build label
        char pBarLabel[27] = "Martian ";
        strcat(pBarLabel, idLabel);
        strcat(pBarLabel, typeLabel);
        strcat(pBarLabel, energyStr);
        strcat(pBarLabel, "/");
        strcat(pBarLabel, totalEnergyStr);

        // Render bar
        gtk_progress_bar_set_text((GtkProgressBar *) pBar, pBarLabel);
        gtk_progress_bar_set_fraction((GtkProgressBar *) pBar, (gdouble) martian->energy / martian->executiontime);
    }

    // Render stuff
    SDL_RenderCopyEx(sdlRenderer, texture, &martian->spriteRect, &martian->rect, 0, NULL, 0);
}
