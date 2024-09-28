#include <libxfce4util/libxfce4util.h>
#include <libxfce4ui/libxfce4ui.h>

#include <libxfce4panel/libxfce4panel.h>
#include <gtk/gtk.h>

#include "calculate-times.h"

typedef struct
{
    XfcePanelPlugin *plugin;

    /* panel widgets */
    GtkWidget       *ebox;
    GtkWidget       *hvbox;
    GtkWidget       *label;

    /* sample settings */
    gchar           *setting1;
    gint             setting2;
    gboolean         setting3;
} PrayerTimes;

PrayerTimes* new_prayer_times_plugin(XfcePanelPlugin *plugin)
{
    PrayerTimes *pt = g_new0(PrayerTimes, 1);
    // xfce_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");
    pt->plugin = plugin;


    return pt;
}

static void
prayer_times_construct(XfcePanelPlugin *plugin)
{
    PrayerTimes *pt;
    pt = new_prayer_times_plugin(plugin);

    gtk_container_add(GTK_CONTAINER (plugin), pt->ebox);
}

#ifdef RELEASE
#define RELEASE_PLUGIN
#endif

#ifndef RELEASE_PLUGIN
int main(){
    const double elevation = 800;
    const double LONG = 29.0605;
    const double LAT = 37.7737;
    const double SF = 1;

    const double Y = 2024;
    const double M = 9;
    double D = 1;
    const double H = 11;
    const double m = 0;
    const double s = 0;
    const double Z = 3;

    struct tm time = {
        .tm_min = m,
        .tm_sec = s,
        .tm_mon = M,
        .tm_hour = H,
        .tm_year = Y,
        .tm_mday = D,
    };

   prayer_times_list* pt = get_prayer_times_list(&time, Z, LONG, elevation, LAT, SF);

    printf("%2.0f.%.0f.%.0f : ", D, M, Y);
    printf("%s : ", double_to_time(pt->FAJR));
    printf("%s : ", double_to_time(pt->SUNRISE));
    printf("%s : ", double_to_time(pt->ZUHR));
    printf("%s : ", double_to_time(pt->ASR));
    printf("%s : ", double_to_time(pt->MAGHRIB));
    printf("%s \n", double_to_time(pt->ISHA));
}
#endif

XFCE_PANEL_PLUGIN_REGISTER(prayer_times_construct)
