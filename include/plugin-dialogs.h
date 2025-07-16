#ifndef _H_PLUGIN_DIALOGS
#define _H_PLUGIN_DIALOGS

#include <libxfce4panel/xfce-panel-plugin.h>

#include "calculate-times.h"

#define PACKAGE_VERSION "1.0"
#define PACKAGE_NAME "PrayerTimes"
#define PLUGIN_WEBSITE "https://github.com/enfyna/xfce-prayer-times"

#define not_interval(args) (args)[0]
#define aggressive_mode(args) (args)[1]
#define show_seconds(args) (args)[2]

typedef struct {
    XfcePanelPlugin* plugin;

    GApplication* app;

    /* panel widgets */
    GtkWidget* ebox;
    GtkWidget* hvbox;
    GtkWidget* label;
    GtkWidget* check;

    /*
     * 0: isha_angle
     * 1: fajr_angle;
     * 2: latitude
     * 3: longitude;
     * 4: shadow_factor
     * 5: elevation
     * 6: descend_correction
     */
    pt_args pt_args;
    /*
     * 0: not_interval;
     * 1: aggressive_mode;
     * 2: show_seconds;
     */
    gdouble pl_args[3];

    gint timeout;
    pt_list pt_list;
} pt_plugin;

void set_tooltip_text(pt_plugin* pt);

void pt_about(void);
void pt_configure(XfcePanelPlugin* plugin, pt_plugin* pt);

void pt_configure_response(GtkWidget* dialog, gint response, pt_plugin* pt);
void pt_save(XfcePanelPlugin* plugin, pt_plugin* pt);
void pt_read(pt_plugin* pt);

void pt_free(XfcePanelPlugin* plugin, pt_plugin* pt);
#endif // _H_PLUGIN_DIALOGS
