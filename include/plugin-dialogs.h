#include <libxfce4panel/xfce-panel-plugin.h>
#include <libxfce4util/libxfce4util.h>

#include <gtk/gtk.h>
#include <glib.h>

#include "calculate-times.h"

#define PACKAGE_VERSION "1.0"
#define PACKAGE_NAME "PrayerTimes"
#define PLUGIN_WEBSITE "https://github.com/enfyna/xfce-prayer-times"

typedef struct {
    XfcePanelPlugin* plugin;

    GApplication* app;

    /* panel widgets */
    GtkWidget* ebox;
    GtkWidget* hvbox;
    GtkWidget* label;
    GtkWidget* check;

    /* settings */
    gdouble fajr_angle;
    gdouble isha_angle;
    gdouble latitude;
    gdouble longitude;
    gdouble elevation;
    gdouble shadow_factor;
    gdouble not_interval;
    
    gint timeout;
    prayer_times_list* pt_list;
} pt_plugin;

void set_tooltip_text(pt_plugin* pt);

void pt_about();
void pt_configure(XfcePanelPlugin* plugin, pt_plugin* pt);

void pt_configure_response(GtkWidget* dialog, gint response, pt_plugin* pt);
void pt_save(XfcePanelPlugin* plugin, pt_plugin* pt);
void pt_read(pt_plugin* pt);

void pt_free(XfcePanelPlugin* plugin, pt_plugin* pt);
