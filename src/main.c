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
static const int width = 1300;
static const int height = 840;
static const int sdl_width = 840;
static const int squareLength = 40;
static const int FPS = 60;
static const int martianLimit = 15;
#define AUTO 0
#define MANUAL 1

// VARIABLES
int simType=MANUAL;
// SDL variables
static SDL_Window *sdlWindow;
static SDL_Renderer *sdlRenderer;

// GTK variables
static GtkWindow *gtkWindow;
static GtkWidget *gtkHbox;
static GtkWidget *gtkDA;

static GtkWidget *gtkButtonStartStop;
static GtkWidget *gtkButtonAddMartian;

static GtkWidget *radio_sim_type1;
static GtkWidget *radio_sim_type2;
static GtkWidget *sim_type_hbox;

static GtkWidget *radio_os1;
static GtkWidget *radio_os2;
static GtkWidget *radio_os_hbox;

static GtkWidget *radio_cal1;
static GtkWidget *radio_cal2;
static GtkWidget *radio_cal3;
static GtkWidget *radio_cal_hbox;

static GtkWidget *radio_rtos_cal1;
static GtkWidget *radio_rtos_cal2;
static GtkWidget *radio_rtos_cal_hbox;

static GtkWidget *radio_green;
static GtkWidget *radio_blue;
static GtkWidget *radio_red;
static GtkWidget *radio_color_hbox;

static GtkWidget *entryEnergy;
static GtkWidget *entryPeriod;
static GtkWidget *entryArrivalTime;

static GtkWidget *time_value_label;

static GtkWidget *pBarVBox;
static void *gdk_window;
static void *window_id;

// Control variables
static int sprite = 0;
static int f_time = 0;
static int mCount = 0;
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
void deleteMartian(Martian *martian) ;
void destroy(GtkWidget* widget, gpointer data);
void destroy_aux();
static gboolean idle(void *ud);
SDL_Texture* loadImage(const char* file, SDL_Renderer *renderer);
void onChangeCalInteractive(GtkWidget* widget, gpointer data);
void onChangeCalRTOS(GtkWidget* widget, gpointer data);
void onChangeSimType(GtkWidget* widget, gpointer data);
gboolean onKeyPress(GtkWidget *widget, GdkEventKey *event, gpointer data);
void renderControlPanel(GtkWidget* widget, gpointer data);
void renderControlPanelAux();
void renderMartian(Martian *martian);

int main(int argc, char** argv) {
    // Start SDL
    SDL_Init(SDL_INIT_VIDEO);
    initializeFile(ARCHIVOMARTIANS);
    initializeFile(ARCHIVOTIME);
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

    // Sim type selectors
    GtkWidget *sim_type_label = gtk_label_new("Sim Type:");
    radio_sim_type1 = gtk_radio_button_new_with_label(NULL, "Manual");
    radio_sim_type2 = gtk_radio_button_new_with_label(NULL, "Auto");
    gtk_radio_button_join_group((GtkRadioButton *) radio_sim_type2, (GtkRadioButton *) radio_sim_type1);
    sim_type_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(sim_type_hbox), sim_type_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sim_type_hbox), radio_sim_type1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(sim_type_hbox), radio_sim_type2, FALSE, FALSE, 0);

    g_signal_connect(radio_sim_type1, "clicked", G_CALLBACK(onChangeSimType), "button");

    // OS radio selectors
    GtkWidget *os_label = gtk_label_new("OS Type:");
    radio_os1 = gtk_radio_button_new_with_label(NULL, "Interactive");
    radio_os2 = gtk_radio_button_new_with_label(NULL, "RTOS");
    gtk_radio_button_join_group((GtkRadioButton *) radio_os2, (GtkRadioButton *) radio_os1);
    radio_os_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_os_hbox), os_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_os_hbox), radio_os1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_os_hbox), radio_os2, FALSE, FALSE, 0);

    g_signal_connect(radio_os1, "clicked", G_CALLBACK(renderControlPanel), "button");

    // Calendarizer radio selectors
    GtkWidget *cal_label = gtk_label_new("Scheduling:");
    radio_cal1 = gtk_radio_button_new_with_label(NULL, "FCFS");
    radio_cal2 = gtk_radio_button_new_with_label(NULL, "Priority");
    radio_cal3 = gtk_radio_button_new_with_label(NULL, "SRTN");
    gtk_radio_button_join_group((GtkRadioButton *) radio_cal2, (GtkRadioButton *) radio_cal1);
    gtk_radio_button_join_group((GtkRadioButton *) radio_cal3, (GtkRadioButton *) radio_cal1);
    radio_cal_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), cal_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_cal_hbox), radio_cal3, FALSE, FALSE, 0);

    g_signal_connect(radio_cal1, "clicked", G_CALLBACK(onChangeCalInteractive), "button");

    // RTOS
    GtkWidget *cal_label_rtos = gtk_label_new("Scheduling:");
    radio_rtos_cal1 = gtk_radio_button_new_with_label(NULL, "EDF");
    radio_rtos_cal2 = gtk_radio_button_new_with_label(NULL, "RM");
    gtk_radio_button_join_group((GtkRadioButton *) radio_rtos_cal2, (GtkRadioButton *) radio_rtos_cal1);
    radio_rtos_cal_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_rtos_cal_hbox), cal_label_rtos, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_rtos_cal_hbox), radio_rtos_cal1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_rtos_cal_hbox), radio_rtos_cal2, FALSE, FALSE, 0);

    g_signal_connect(radio_rtos_cal1, "clicked", G_CALLBACK(onChangeCalRTOS), "button");

    // Martian color radio selectors
    GtkWidget *color_label = gtk_label_new("Priority:");
    radio_green = gtk_radio_button_new_with_label(NULL, "Green (High)");
    radio_blue = gtk_radio_button_new_with_label(NULL, "Blue (Medium)");
    radio_red = gtk_radio_button_new_with_label(NULL, "Red (Low)");
    gtk_radio_button_join_group((GtkRadioButton *) radio_blue, (GtkRadioButton *) radio_green);
    gtk_radio_button_join_group((GtkRadioButton *) radio_red, (GtkRadioButton *) radio_green);
    radio_color_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

    gtk_box_pack_start(GTK_BOX(radio_color_hbox), color_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_green, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_blue, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(radio_color_hbox), radio_red, FALSE, FALSE, 0);

    // Buttons
    gtkButtonAddMartian = gtk_button_new_with_label ("Add Martian");
    gtkButtonStartStop = gtk_button_new_with_label ("Start");

    // Time stuff
    GtkWidget *time_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *time_label = gtk_label_new("Time (s) :");
    time_value_label = gtk_label_new("0");

    gtk_box_pack_start (GTK_BOX(time_hbox), time_label, TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX(time_hbox), time_value_label, TRUE, TRUE, 0);

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
    gtk_box_pack_start(GTK_BOX(controlPanel), sim_type_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_os_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_cal_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_rtos_cal_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryEnergy, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryPeriod, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), entryArrivalTime, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(controlPanel), radio_color_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), gtkButtonAddMartian, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), gtkButtonStartStop, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), time_hbox, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), pBar_label, FALSE, FALSE, 10);
    gtk_box_pack_start (GTK_BOX(controlPanel), pBarVBox, FALSE, FALSE, 10);

    gtk_widget_show_all((GtkWidget*) gtkWindow);

    // Default hidden elements
    gtk_widget_hide(entryPeriod);
    gtk_widget_hide(radio_rtos_cal_hbox);

    // Get window ID for SDL window creation
    gdk_window = gtk_widget_get_window(GTK_WIDGET(gtkDA));
    window_id = (void*)(intptr_t)GDK_WINDOW_XID(gdk_window);

    // GTK idle function control SDL loop
    g_idle_add(&idle, 0);

    // Init logic
    initialize();

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

    int totalEnergy = (int) strtol(entryEnergyText, NULL, 10);;
    int period;
    int arrivalTime;

    // Variable verification
    bool clear = false;
    if (totalEnergy <= 0) {
        printf("Energy error\n");
    } else {
        gtk_entry_set_text((GtkEntry *) entryEnergy, "");
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os2)) == TRUE) {
            // RTOS
            period = (int) strtol(entryPeriodText, NULL, 10);
            if (period <= 0) {
                // Error
                printf("Period error\n");
            } else {
                // Period is correct. Default arrival time to 1
                clear = true;
                gtk_entry_set_text((GtkEntry *) entryPeriod, "");
                arrivalTime = 1;
            }
        } else {
            // Interactive
            arrivalTime = (int) strtol(entryArrivalTimeText, NULL, 10);
            if (arrivalTime < lastTime) {
                printf("Arrival time error\n");
            } else {
                // Arrival time is correct. Default period to 1;
                clear = true;
                gtk_entry_set_text((GtkEntry *) entryArrivalTime, "");
                period = 1;
            }
        }
    }

    // Check clear status
    if (clear) {
        // Check that martian limit hasn't been reached
        if (mCount <= martianLimit) {
            // All clear to create martian
            printf("Energy %d, Period %d, Arrival time %d\n", totalEnergy, period, arrivalTime);
            Martian *martian = addMartian(0, squareLength * 11, type, totalEnergy, period, arrivalTime);

            // Draw energy progress bar
            GtkWidget *pBar = gtk_progress_bar_new();
            gtk_progress_bar_set_show_text((GtkProgressBar *) pBar, TRUE);
            martian->pBar = pBar;

            gtk_box_pack_start(GTK_BOX(pBarVBox), pBar, FALSE, FALSE, 0);
            renderControlPanelAux();

            mCount++;
            printf("Added martian type %d\n", type);
        } else {
            printf("Martian limit reached. Couldn't create new one\n");
        }
    } else {
        // Error introducing data
        printf("Error with introducing data\n");
    }
}


/**
 * Called when button start/stop is clicked
 */
void buttonStartStop(GtkWidget* widget, gpointer data) {
    // Disable UI controls
    if (simType==MANUAL) {
        gtk_widget_set_sensitive(radio_os_hbox, FALSE);
        gtk_widget_set_sensitive(sim_type_hbox, FALSE);
        gtk_widget_set_sensitive(gtkButtonStartStop, FALSE);

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os1)) == TRUE) {
            // Interactive
            gtk_widget_set_sensitive(radio_cal_hbox, FALSE);
        } else {
            // RTOS
            gtk_widget_set_sensitive(radio_rtos_cal_hbox, FALSE);
        }
    } else if (simType== AUTO){
        gtk_widget_set_sensitive(radio_os_hbox, FALSE);
        gtk_widget_set_sensitive(entryEnergy, FALSE);
        gtk_widget_set_sensitive(radio_color_hbox, FALSE);
        gtk_widget_set_sensitive(gtkButtonAddMartian, FALSE);

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os1)) == TRUE) {
            // Interactive
            gtk_widget_set_sensitive(radio_cal_hbox, FALSE);
            gtk_widget_set_sensitive(entryArrivalTime, FALSE);
        } else {
            // RTOS
            gtk_widget_set_sensitive(radio_rtos_cal_hbox, FALSE);
            gtk_widget_set_sensitive(entryPeriod, FALSE);
        }
    }
    // Call logic to start
    sem_post(startSemaphore);
    running = true;
    printf("Started simulation\n");
}


/**
 * Clears main GTK and SDL memory on program exit
 */
void destroy(GtkWidget* widget, gpointer data) {
    destroy_aux();
}

void destroy_aux() {
    finish=1;
    if (get_size()==0){
        pthread_mutex_unlock(&mutexFinalMain);
    }

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

    pthread_mutex_lock(&mutexFinal);
    // Close GTK
    gtk_main_quit();
    pthread_mutex_unlock(&mutexFinal);
    freedata();
}


/**
 * GTK loop function on idle
 */
static gboolean idle(void *ud) {
    (void)ud;
    if (get_size()==0 && simType==AUTO && running==true){

        destroy_aux();
    } else if (finish==1){
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Schedule error!", "ERROR: Could not schedule martians", NULL);
        destroy_aux();
    }
    else {
        if (!sdlWindow) {
            // Create window
            printf("Creating SDL window for window id %p\n", window_id);
            sdlWindow = SDL_CreateWindowFrom(window_id);
            SDL_SetWindowSize(sdlWindow, sdl_width, height);
            printf("sdl_window=%p\n", sdlWindow);
            if (!sdlWindow) {
                printf("%s\n", SDL_GetError());
            }

            // Create renderer
            sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            printf("sdl_renderer=%p\n", sdlRenderer);
            if (!sdlRenderer) {
                printf("%s\n", SDL_GetError());
            }
            IMG_Init(IMG_INIT_PNG);

            // Load images
            background = loadImage("../src/img/wework.png", sdlRenderer);
            martianSS_green = loadImage("../src/img/mmsheet_green.png", sdlRenderer);
            martianSS_blue = loadImage("../src/img/mmsheet_blue.png", sdlRenderer);
            martianSS_red = loadImage("../src/img/mmsheet_red.png", sdlRenderer);

            backgroundRect = (SDL_Rect) {0, 0, sdl_width, height};
        } else {
            // Main SDL loop
            SDL_RenderClear(sdlRenderer);
            SDL_RenderCopy(sdlRenderer, background, NULL, &backgroundRect);

            // Update every martian
            for (int i = 0; i < get_size(); i++) {
                Martian *martian = find(i);
                if (martian->finish == 1) {
                    deleteMartian(martian);
                } else {
                    renderMartian(martian);
                }
            }

            // Render stuff
            SDL_RenderPresent(sdlRenderer);

            char tiempo[33];
            sprintf(tiempo,"%d",lastTime);
            // Update time label
            gtk_label_set_text((GtkLabel *) time_value_label, tiempo);

            // Handle frame on loop
            f_time++;
            if (FPS / f_time == 4) {
                f_time = 0;
                sprite++;
            }
            SDL_Delay(1000 / 120);
        }

        // Refresh GTK widgets
        gtk_widget_show((GtkWidget *) gtkWindow);
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
 * Activates when calendarization in interactive radio is changed
 */
void onChangeCalInteractive(GtkWidget* widget, gpointer data) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_cal1)) == TRUE) {
        scheduler=FCFS;
        printf("FCFS\n");
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_cal2)) == TRUE) {
        scheduler=PRIORITY;
        printf("Priority\n");
    } else {
        scheduler=SRTN;
        printf("SRTN\n");
    }
}


/**
 * Activates when calendarization in RTOS radio is changed
 */
void onChangeCalRTOS(GtkWidget* widget, gpointer data) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_rtos_cal1)) == TRUE) {
        mode=EDF;
        printf("EDF\n");
    } else {
        mode=RM;
        printf("RM\n");
    }
}


/**
 * Activates when sim type radio is changed
 */
void onChangeSimType(GtkWidget* widget, gpointer data) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sim_type1)) == TRUE) {
        simType=MANUAL;
        printf("manual\n");
        gtk_widget_set_sensitive(radio_os_hbox, TRUE);
        gtk_widget_set_sensitive(entryEnergy, TRUE);
        gtk_widget_set_sensitive(radio_color_hbox, TRUE);
        gtk_widget_set_sensitive(gtkButtonAddMartian, TRUE);

        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os1)) == TRUE) {
            // Interactive
            gtk_widget_set_sensitive(radio_cal_hbox, TRUE);
            gtk_widget_set_sensitive(entryArrivalTime, TRUE);
        } else {
            // RTOS
            gtk_widget_set_sensitive(radio_rtos_cal_hbox, TRUE);
            gtk_widget_set_sensitive(entryPeriod, TRUE);
        }
    } else {
        simType=AUTO;
        printf("auto\n");

    }
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
    renderControlPanelAux();
}

void renderControlPanelAux() {
    gtk_widget_show_all((GtkWidget *) gtkWindow);
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_os1)) == TRUE) {
        // Interactive OS
        gtk_widget_show(entryArrivalTime);
        gtk_widget_show(radio_cal_hbox);
        gtk_widget_hide(radio_rtos_cal_hbox);
        gtk_widget_hide(entryPeriod);
        gtk_entry_set_text((GtkEntry *) entryPeriod, "");
        systemType=INTERACTIVE;
        printf("Interactive\n");
    } else {
        // RTOS
        gtk_widget_hide(entryArrivalTime);
        gtk_widget_hide(radio_cal_hbox);
        gtk_widget_show(radio_rtos_cal_hbox);
        gtk_widget_show(entryPeriod);
        gtk_entry_set_text((GtkEntry *) entryArrivalTime, "");
        systemType=RTOS;
        printf("RTOS\n");
    }
}


/**
 * Renders martian on SDL UI
 * @param martian to render
 */
void renderMartian(Martian *martian) {
    // Animate movement on loop
    if (martian->finish==0){
        if (martian->movDir != -1) {
            int movspeed=2;
            switch (martian->movDir) {
                case 0:
                    // Up
                    if (martian->destY != martian->y) {
                        martian->y -= movspeed;
                    } else {
                        martian->destY = -1;
                        martian->movDir = -1;
                    }
                    break;
                case 1:
                    // Down
                    if (martian->destY != martian->y) {
                        martian->y += movspeed;
                    } else {
                        martian->destY = 0;
                        martian->movDir = -1;
                    }
                    break;
                case 2:
                    // Left
                    if (martian->destX != martian->x) {
                        martian->x -= movspeed;
                    } else {
                        martian->destX = 0;
                        martian->movDir = -1;
                    }
                    break;
                case 3:
                    // Right
                    if (martian->destX != martian->x) {
                        martian->x += movspeed;
                    } else {
                        martian->destX = 0;
                        martian->movDir = -1;
                    }
                    break;
            }
        }


        // Calculate SDL variables
        martian->spriteRect = (SDL_Rect){(sprite % 7)*165, 0, 165, 165};
        int cambio=1;
        if (martian->changex!=0){
            int var= 1- (martian->changex<0)*2;
            martian->currentChangex+=var;
            martian->changex-=var;
        }
        if (martian->changey!=0){
            int var= 1- (martian->changey<0)*2;
            martian->currentChangey+=var*cambio;
            martian->changey-=var*cambio;
        }
        if (martian->currentDirection==LEFT){
            martian->rect = (SDL_Rect) {martian->x + squareLength / 4, martian->y,
                                        squareLength / 2, squareLength/2};

        }
        else if (martian->currentDirection==RIGHT){
            martian->rect = (SDL_Rect) {martian->x + squareLength / 4, martian->y + squareLength / 2,
                                        squareLength / 2, squareLength/2};
        }
        else if (martian->currentDirection==DOWN){
            martian->rect = (SDL_Rect) {martian->x,  martian->y + squareLength / 4,
                                        squareLength / 2, squareLength/2};
        }
        else{
            martian->rect = (SDL_Rect) {martian->x + squareLength / 2, martian->y+ squareLength / 4,
                                        squareLength / 2, squareLength/2};
        }

        martian->rect = (SDL_Rect){martian->x+martian->currentChangex, martian->y+martian->currentChangey,
                                   squareLength/2, squareLength/2};
        // Draw two martians in the same square if logic determines it
       // drawDual();*/

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
}
/**
 * Deletes martian
 * @param martian to delete
 */
void deleteMartian(Martian *martian) {
    printf("eliminar marciano\n");
    gtk_container_remove((GtkContainer *) pBarVBox, martian->pBar);
    martian->pBar = NULL;
    removeMartian(martian);
    printf("eliminado\n");
    free(martian);
    mCount--;
}
